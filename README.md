## ESP32/ESP8266 library to control Philips 3200 coffee machine

Based on:\
https://github.com/mkorenko/esp-phillips-3200 \
https://github.com/chris7topher/SmartPhilips2200 \
https://github.com/micki88/Philips-ep3200-testing \
https://github.com/walthowd/Philips-ep3200-ha \
https://github.com/veonua/SmartPhilips

A video for explanation can be found here:
https://youtu.be/jhzEMkL5Xek

Used these parts:
- [NodeMCU ESP32 v1](https://www.elektor.com/joy-it-nodemcu-esp32-development-board)
- [Molex 90325-0008 Connector](https://www.mouser.com/ProductDetail/Molex/90325-0008?qs=P41GyhEsKL7wtbj5ylImAA%3D%3D&countryCode=US&currencyCode=USD)
- [Molex 92315-0825 Cable](https://www.mouser.com/ProductDetail/Molex/92315-0825?qs=sfs0HZCnrVBO%252B%2Fha6s8VfA%3D%3D&countryCode=US&currencyCode=USD)
- [BC33725TFR NPN Transistor](https://www.mouser.com/ProductDetail/onsemi-Fairchild/BC33725TFR?qs=zGXwibyAaHYlHlvhRz3mQw%3D%3D&countryCode=US&currencyCode=USD)

For NodeMCU boards you don't need to convert voltage as they have onboard voltage regulators.

### Library defaults
```
ESP32:
- uses Serial / UART0 to connect to the coffee machine
- uses HardwareSerial(2) / UART2 - RX2_PIN / TX2_PIN to connect to the display:
  #define RX2_PIN 16 // Rx from display
  #define TX2_PIN 17 // Tx not used
  #define NPN_E_PIN 23 // Gnd for display (to switch it on and off)

ESP8266:
- uses Serial / UART0 to connect to connect to the coffee machine
- uses SoftwareSerial - RX2_PIN / TX2_PIN to the display:
  #define RX2_PIN 14 // Rx from display
  #define TX2_PIN 12 // Tx not used
  #define NPN_E_PIN 13 // Gnd for display (to switch it on and off)
```

### Library API / usage example
Consider the following Arduino IDE project example:
```
// path to this library
// note: Arduino IDE  accepts custom libs in "src" dir only
#include "src/esp-phillips-3200/esp-phillips-3200.h"

Phillips3200 machine;

void on_machine_state_changed() {
  // callback is fired when state of the coffee machine changes

  // available public properties:

  // false - off, true - on
  bool power_status = machine.current_power_status;

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
  std::string machine_status = machine.current_machine_status;

  // brews, available for "selected" and "brewing" machine statuses:
  // "espresso", "2x_espresso", "coffee", "2x_coffee",
  // "americano", "2x_americano", "cappuccino", "latte",
  // "hot_water"
  // "none" - if not available
  std::string brew = machine.current_brew;

  // 0 - unavailable; values: 1 - 3
  uint8_t strength_level = machine.current_strength_level;

  // 1 - "normal"; 0 - powder
  uint8_t grinder_type = machine.current_grinder_type;

  // values: 1 - 3
  uint8_t water_level = machine.current_water_level;

  // 0 - unavailable; values: 1 - 3
  uint8_t milk_level = machine.current_milk_level;
}

void setup() {
  // pass callback function
  machine.setup(on_machine_state_changed);
}

void loop() {
  // required:
  machine.loop();
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

std::string command;
machine.send_cmd(command);
```


### ESP32 wiring example

The wiring within the coffee machine is as shown in the picture:
![Wiring](https://github.com/flashmandv/phillips-3246-smart/blob/main/images/wiring.png)

*Warning!*  You need a voltage regulator if your ESP32 can't handle more then 3V. If you use Node MCU DevBoard - then you don't need voltage regulator (as shown here)

##### Molex Cable has black/red line on side for PIN1 (Shown going right to left above):

- PIN1 - 4-5V from Coffee Machine
    - Connect to ESP832 and PIN1 on Molex 90325-0008 Connector that goes to display
- PIN2 - Ground
    - Connect to Ground / GND on ESP32 and connect to [collector leg of NPN transistor](https://www.mouser.com/datasheet/2/308/1/BC338_D-1802398.pdf)
- PIN3 - Ground. Connect to the PIN2 ground (otherwise the display on some machines does not start)
- PIN4 - WakeUp - Not used
- PIN5 - RX
    - This is actually TX from the coffee machine, but connects to RX pin on ESP32
        - This pin is `RXD0` / `GPIO3` on most boards
    - Also connect to PIN5 on Molex 90325-0008 Connector that goes to display
- PIN6 - TX
    - This is actually RX from coffee machine, but connects to TX pin on ESP32
        - This pin is `TXD0` / `GPIO1` on most boards
        - This is the pin that we are stealing and routing through the ESP32
- PIN7 - Prog Rx - Not used
- PIN8 - Prog Tx - Not used

##### From the ESP32 connections not already referenced
- `GPIO16` for TX that connects to PIN6 on Molex 90325-0008 Connector that goes to display
- `D23` / `GPIO23` for GND to turn off display
    - Connects to [middle base leg of NPN transistor](https://www.mouser.com/datasheet/2/308/1/BC338_D-1802398.pdf)

##### From NPN transistor, connections not alread referenced
- [Emitter leg of NPN transistor](https://www.mouser.com/datasheet/2/308/1/BC338_D-1802398.pdf) to PIN2 on Molex 90325-0008 Connector that goes to display
