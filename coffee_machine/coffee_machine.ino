// path to this library
// note: Arduino IDE  accepts custom libs in "src" dir only
#include "src/esp-phillips-3200/esp-phillips-3200.h"
//#include <Wire.h>
//#include <LiquidCrystal_I2C.h>

#include <WiFi.h>

Phillips3200 machine;
bool inited = false;

//LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 for a 16 chars and 2 line display

//button code
int button = 16; // push button is connected
int temp = 0; 	 // temporary variable for reading the button pin status
int cleared = 0;

//WIFI CODE/////////////////////////////////////////////////////////
// Load Wi-Fi library

// Replace with your network credentials
const char* ssid     = "<YOUR WIFI NAME>";
const char* password = "<YOUR WIFI PASS>";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 5000;

const char* wl_status_to_string(wl_status_t status) {
  switch (status) {
    case WL_NO_SHIELD: return "WL_NO_SHIELD";
    case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
    case WL_CONNECTED: return "WL_CONNECTED";
    case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED: return "WL_DISCONNECTED";
    default: return "WIFI_UNKNOWN_STATUS";
  }
}


void printLCD(String texts, int row=0, bool clear = true) {
  /*lcd.setCursor(0, row);
  if (clear)
    lcd.clear();
  lcd.print(texts);  */
}

void printSerial(String text) {
  //Serial.println(text);
}

bool power_status = false;
std::string machine_status = "";
std::string brew = "";
uint8_t strength_level = 0; // 0 - unavailable; values: 1 - 3
uint8_t grinder_type = 0; // 1 - "normal"; 0 - powder
uint8_t water_level = 0; // values: 1 - 3
uint8_t milk_level = 0; // 0 - unavailable; values: 1 - 3


void on_machine_state_changed() {
  // callback is fired when state of the coffee machine changes

  // false - off, true - on
  power_status = machine.current_power_status;
 
  // machine statuses:
  // "off"
  // "heating" - the machine is heating / initializing
  // "ready" - ready, nothing is selected
  // "ready_aqua_clean" - ready + aqua clean warning
  // "selected" - brew is selected
  // "brewing" - brewing
  // "error_no_water" - no water / water tank is ejected
  // "error_grounds_container" - grounds container light is on
  // "error" - other errors
  machine_status = machine.current_machine_status;
  /*if (machine_status == "selected") {
    blinkSlow();
    delay(1000);
    machine.send_cmd("start_pause");

  } else {
    blinkFast();
  }*/
  // brews, available for "selected" and "brewing" machine statuses:
  // "espresso", "2x_espresso", "coffee", "2x_coffee",
  // "americano", "2x_americano", "cappuccino", "latte",
  // "hot_water"
  // "none" - if not available
  brew = machine.current_brew;

  // 0 - unavailable; values: 1 - 3
  strength_level = machine.current_strength_level;

  // 1 - "normal"; 0 - powder
  grinder_type = machine.current_grinder_type;

  // values: 1 - 3
  water_level = machine.current_water_level;

  // 0 - unavailable; values: 1 - 3
  milk_level = machine.current_milk_level;

  printLCD(machine_status.c_str(), 1); 
}


void setup() {
  delay(1000);

  
  // pass callback function
  machine.setup(on_machine_state_changed);
  
  //LCD CODE///////////////////////////////////////////////////////////////////////
  /*lcd.init();                       // Initialize the LCD
  lcd.backlight();                  // Turn on the backlight
  lcd.clear();                      // Clear the LCD screen
*/

  
  //WIFI CODE///////////////////////////////////////////////////////////////////////
  WiFi.hostname("coffee");
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  
  printSerial("");
  printSerial("Connecting to:");
  printSerial(ssid);             
  printLCD("Connecting WiFi.");   
 
  IPAddress staticIP(192,168,50,227);
  IPAddress gateway(192,168,50,1);
  IPAddress dnss(192,168,50,1);
  IPAddress subnet(255,255,255,0);
  WiFi.config(staticIP, gateway, subnet, gateway, dnss);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    printSerial(wl_status_to_string(WiFi.status()));
  }
  printSerial("");
  printSerial("WiFi connected.");
  printSerial("IP address: ");
  printSerial(WiFi.localIP().toString());
  printLCD("WiFi connected !"); 
  printLCD(WiFi.localIP().toString(), 1); 

  server.begin();

  inited = true;

}

//presets have problem - unable to set the water level at the desired value. Something with the current water level retrieval cannot be updated. 
//Needs further investigation. Disabling presets until then 
String startCoffee(String brewType, int desiredWaterLevel = 3, int desiredStrengthLevel = 3) {
  String _ERROR = "";
  if (machine_status == "ready" || machine_status == "ready_aqua_clean" || machine_status == "selected") {
     
    machine.send_cmd(brewType.c_str());
    machine.send_cmd("request_info");
    machine.machine_out_loop();
    delay(1000);
 
    if (machine_status == "selected" || machine_status == "ready") {
 
      int wl = 0;
      int c = 0;
      if (water_level != 0)  {
        if (water_level == 1) c = desiredWaterLevel == 3 ? 2 : desiredWaterLevel == 2 ? 1 : 0;
        if (water_level == 2) c = desiredWaterLevel == 3 ? 1 : desiredWaterLevel == 2 ? 0 : 1;
        if (water_level == 3) c = desiredWaterLevel == 3 ? 0 : desiredWaterLevel == 2 ? 1 : 2;

        for (int i=0; i<c; i++) {
          machine.send_cmd("coffee_water_level");
          machine.send_cmd("request_info");
          machine.machine_out_loop();
          delay(1000);
        }

      } else {
        _ERROR += " [ERR]Unable to set water level. Curr level is: "+String(strength_level);  
        String errLCD = "Str lvl fail:"+String(strength_level);         
        printLCD(errLCD);  
        delay(2000);   
      }

      delay(1500);

      int st = 0;
      if (brewType != "hot_water" && strength_level != desiredStrengthLevel) {
        while (strength_level != desiredStrengthLevel && st++ < 6) {
          machine.send_cmd("coffee_strength_level");
          //delay(50);
          machine.send_cmd("request_info");
          delay(1000);
        }
        
        delay(1500);
        if (strength_level != desiredStrengthLevel) {
          _ERROR += " [ERR]Unable to set strength level. Curr level is: "+String(strength_level);  
          String errLCD = "Str lvl fail:"+String(strength_level);         
          printLCD(errLCD);  
          delay(2000);
        }
      }   

      delay(1000);
      printLCD("Starting..      ", 1, false);  
      machine.send_cmd("start_pause");
    } else {
      _ERROR += " [ERR]Cannot Brew NOTselectedmake coffee. Brew not selected.";           
      printLCD("Brew NOT Selected", 0, false); 
      delay(2000); 
    }
  } else {
    _ERROR += " [ERR]Cannot make coffee. Machine is not ready";           
    printLCD("Machine NotReady");  
    delay(2000);
  }
  return _ERROR;
}




void loop() {
  // required:
  machine.loop();
 

  //HARDWARE BUTTON TEST CODE///////////////////////////////////////////////////////////////////////////////////////////////////
  /*temp = digitalRead(button);

  if (temp == LOW && cleared == 0) {
    //digitalWrite(led, HIGH);
    cleared = 1;
    printLCD("BTN PRESSED!!   ");     // Print some text
    printSerial("Button pressed");
  } else if (temp == HIGH && cleared == 1) {
    cleared = 0;
    //digitalWrite(led, LOW);
    printLCD("BTN RELEASED!!  ");     // Print some text
    printSerial("Button released");
  }*/


  //WIFI WEBSERVER CODE///////////////////////////////////////////////////////////////////////////////////////////////////////


  loopServer();
  
}

void loopServer() {
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    printSerial("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        //Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            String ERROR = "";
            String STATUS = "";
            bool redirect = false;

            // turns the GPIOs on and off
            if (header.indexOf("GET /poweron") >= 0) {

              STATUS = "POWERING ON..";
              printLCD("  POWERING ON   "); 

              if (power_status == 0) {
                machine.send_cmd("power_on");
                redirect = true;
              } else {
                ERROR = "Cannot power on. Machine is already on";       
                printLCD("MachineAlreadyON");  
              }

            } else if (header.indexOf("GET /poweroff") >= 0) {

              STATUS = "POWERING OFF..";      
              printLCD("  POWERING OFF  ");   

              if (power_status == 1) {
                machine.send_cmd("power_off");
                redirect = true;
              } else {
                ERROR = "Cannot power off. Machine is already off";           
                printLCD("MachineAlreadyON");  
              }

            }  else if (header.indexOf("GET /espresso_full") >= 0) {

              STATUS = "STARTING FULL STRONG ESPRESSO..";      
              printLCD("START FULL ESPRE");   

              ERROR = startCoffee("espresso", 3, 3);
              redirect = (ERROR == "");
              
            } else if (header.indexOf("GET /espresso_small_strong") >= 0) {

              STATUS = "STARTING SMALL STRONG ESPRESSO..";      
              printLCD("START SMALL ESPR");   

              ERROR = startCoffee("espresso", 1, 3);
              redirect = (ERROR == "");
 
            }  else if (header.indexOf("GET /small_hot_water") >= 0) {

              STATUS = "STARTING SMALL HOT WATER..";      
              printLCD("START SHOT WATER");   

              ERROR = startCoffee("hot_water", 1, 0);
              redirect = (ERROR == "");
 
            }  else if (header.indexOf("GET /medium_hot_water") >= 0) {

              STATUS = "STARTING MEDIUM HOT WATER..";      
              printLCD("START MHOT WATER");   

              ERROR = startCoffee("hot_water", 2, 0);
              redirect = (ERROR == "");
 
            }  else if (header.indexOf("GET /big_hot_water") >= 0) {

              STATUS = "STARTING BIG HOT WATER..";      
              printLCD("START BHOT WATER");   

              ERROR = startCoffee("hot_water", 3, 0);
              redirect = (ERROR == "");
 
            } else if (header.indexOf("GET /set_strength_level") >= 0) {

              STATUS = "Set strength level..";      
              printLCD("SET STRENGTH LVL");   

              machine.send_cmd("coffee_strength_level");
              redirect = true;
 
            } else if (header.indexOf("GET /set_water_level") >= 0) {

              STATUS = "Set water level..";      
              printLCD("SET WATER LVL...");   

              machine.send_cmd("coffee_water_level");
              redirect = true;
 
            }  else if (header.indexOf("GET /start") >= 0) {

              STATUS = "Starting..      ";      
              printLCD(STATUS);   

              machine.send_cmd("start_pause");
              redirect = true;
 
            } else if (header.indexOf("GET /espresso") >= 0) {

              STATUS = "SELECTING ESPRESSO..";      
              printLCD("SELECT ESPRESSO");   

              machine.send_cmd("espresso");              
              redirect = true;
              
            } else if (header.indexOf("GET /americano") >= 0) {

              STATUS = "SELECTING AMERICANO..";      
              printLCD("SELECT AMERICANO");   

              machine.send_cmd("americano");              
              redirect = true;
              
            } else if (header.indexOf("GET /hot_water") >= 0) {

              STATUS = "SELECTING HOT WATER..";      
              printLCD("SELECT HOT WATER");   

              machine.send_cmd("hot_water");              
              redirect = true;
              
            }



            
            
            if (STATUS != "") 
              printSerial(STATUS);  
            if (ERROR != "") 
              printSerial(ERROR);      

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>");
            client.println("html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}");
            client.println("</style>");
            client.println("</head><body>");
            // Web Page Heading
            client.println("<h2>Coffee Machine (NodeMCU ESP32 v1)</h2>");

            if (STATUS != "") {
              String status = "<p>STATUS: "+STATUS+" </p>";
              client.println(status);
            }

            if (ERROR != "") {
              String err = "<p>ERROR: "+ERROR+" </p>";
              client.println(err);
            }

            client.println("<p style=\"text-align: left;\"><a href=\"/\"><button class=\"button\">HOME PAGE</button></a></p>");


            //std::to_string
            String powerStatus = "<p>POWER STATUS: " + String((power_status == 0) ? "OFF" : "ON") + " </p>";
            String machineStatus = "<p>MACHINE STATUS: " + String(machine_status.c_str()) + "</p>";
            String selectedBrew = "<p>Selected brew: " + String(brew.c_str()) + "</p>";
            String strengthLvl = "<p>Strength level: " + String(strength_level) + "</p>";
            String waterLvl = "<p>Water level: " + String(water_level) + "</p>";
            String milkLvl = "<p>Milk level: " + String(milk_level) + "</p>";
            client.println(powerStatus);
            client.println(machineStatus);
            client.println(selectedBrew);
            client.println(strengthLvl);
            client.println(waterLvl);
            client.println(milkLvl);
            //client.println(machine_status.c_str());
          
            
            // If the output5State is off, it displays the ON button       
            if (power_status == 0) {
              client.println("<p><a href=\"/poweron\"><button class=\"button\">POWER ON</button></a></p>");
            } else {
              if (machine_status == "ready" || machine_status == "ready_aqua_clean" || machine_status == "selected") {

                if (machine_status == "selected") {

                  client.println("<p><a href=\"/set_water_level\"><button class=\"button\">Change Water Level</button></a></p>");
                  client.println("<p><a href=\"/set_strength_level\"><button class=\"button\">Change Strength Level</button></a></p>");
                  client.println("<p><a href=\"/start\"><button class=\"button\">START</button></a></p>");

                }

                //Disabling preset buttons as the setting of water/strength level is very buggy (due to current level not being refreshed properly)
                /*client.println("<br><h5>-----PRESETS-----</h5>");
                client.println("<p><a href=\"/espresso_full\"><button class=\"button\">ESPRESSO FULL STRONG</button></a></p>");
                client.println("<p><a href=\"/espresso_small_strong\"><button class=\"button\">ESPRESSO SMALL STRONG</button></a></p>");
                client.println("<p><a href=\"/small_hot_water\"><button class=\"button\">SMALL HOT WATER</button></a></p>");
                client.println("<p><a href=\"/medium_hot_water\"><button class=\"button\">MEDIUM HOT WATER</button></a></p>");
                client.println("<p><a href=\"/big_hot_water\"><button class=\"button\">BIG HOT WATER</button></a></p>");*/

                client.println("<br><h5>----- SELECT -----</h5>");
                client.println("<p><a href=\"/espresso\"><button class=\"button\">Espresso</button></a>");
                client.println("<p><a href=\"/americano\"><button class=\"button\">Americano</button></a>");
                client.println("<p><a href=\"/hot_water\"><button class=\"button\">Hot Water</button></a>");
                
              } else if (machine_status == "brewing") {
                client.println("<p><a href=\"/start\"><button class=\"button\">STOP</button></a></p>");
              }

              client.println("<p><a href=\"/poweroff\"><button class=\"button\">POWER OFF</button></a></p>");
            } 

            if (redirect) {
               client.println("<script>function redirAfter () {document.location.href='/';} setTimeout(redirAfter, 2500);</script>");
            }
             
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    printSerial("Client disconnected.");
    printSerial("");
  }
}

// API to control the machine:

// available commands:
// power: "power_on", "power_off"
// select brew:
//   "espresso", "coffee", "americano", "cappuccino", "latte", "hot_water"
// levels:
//   "coffee_strength_level", "coffee_water_level", "coffee_milk_level"
// start: "start_pause"
// other: "aqua_clean", "calc_clean", "request_info"

//std::string command;
//machine.send_cmd(command);