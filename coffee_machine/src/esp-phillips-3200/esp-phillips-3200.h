#ifndef PHILLIPS_3200
#define PHILLIPS_3200

#if defined(ESP8266)
#include <SoftwareSerial.h>
#else
#include <HardwareSerial.h>
#endif

#include <string>
#include "Arduino.h"

#if defined(ESP8266)

  #ifndef RX2_PIN
  #define RX2_PIN 14 // Rx from display
  #endif

  #ifndef TX2_PIN
  #define TX2_PIN 12 // Tx not used
  #endif

  #ifndef NPN_E_PIN
  #define NPN_E_PIN 13 // Gnd for display (to switch it on and off)
  #endif

#else

  #ifndef RX2_PIN
  #define RX2_PIN 16 // Rx from display
  #endif

  #ifndef TX2_PIN
  #define TX2_PIN 17 // Tx not used
  #endif
  #ifndef NPN_E_PIN
  #define NPN_E_PIN 23 // Gnd for display (to switch it on and off)
  #endif

#endif

#define MachineSerial Serial  // UART0

const size_t _machine_cmd_buf_size = 18;

class Phillips3200 {
  public:
    #if defined(ESP8266)
    SoftwareSerial DisplayOutSerial = SoftwareSerial(RX2_PIN, TX2_PIN);
    #else
    HardwareSerial DisplayOutSerial = HardwareSerial(2);  // UART2
    #endif

    /* curent state */
    bool current_power_status = false;
    std::string current_machine_status = "";
    std::string current_brew = "";
    uint8_t current_strength_level = 0;
    uint8_t current_grinder_type = 0;
    uint8_t current_water_level = 0;
    uint8_t current_milk_level = 0;

    /* hooks */
    void setup(std::function<void ()> on_machine_state_changed);
    void loop();
	void machine_out_loop();

    /* machine_cmd */
    void send_cmd(const std::string &command);
  private:
    /* machine_cmd */
    void _send_buf(const byte buf_command[]);
    void _send_power_on();

    /* display_out */
    void _display_out_loop();

    /* machine_out */
    std::function<void ()> _on_machine_state_changed;

    char _current_machine_cmd_buf[_machine_cmd_buf_size + 2];
    char _prev_machine_cmd_buf[_machine_cmd_buf_size + 2];
    size_t _machine_cmd_buf_idx = 0;
    void _on_machine_out_buffer_changed();
    

    /* machine_states */
    uint8_t machine_states_get_level_from_byte(byte value);
    uint8_t machine_states_get_grinder_from_byte(byte mode);
    std::string machine_states_get_machine_status_from_buf(char *buffer);
    std::string machine_states_get_brew_from_buf(char *buffer);

    static byte _buf_power_on[12];
    static byte _buf_power_on1[12];
    static byte _buf_power_off[12];
    static byte _buf_espresso[12];
    static byte _buf_coffee[12];
    static byte _buf_americano[12];
    static byte _buf_cappuccino[12];
    static byte _buf_latte[12];
    static byte _buf_hot_water[12];
    static byte _buf_coffee_strength_level[12];
    static byte _buf_coffee_water_level[12];
    static byte _buf_coffee_milk_level[12];
    static byte _buf_aqua_clean[12];
    static byte _buf_calc_clean[12];
    static byte _buf_start_pause[12];
    static byte _buf_request_info[12];
};

#endif
