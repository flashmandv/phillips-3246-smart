#include <string>
#include "Arduino.h"
#include "esp-phillips-3200.h"

byte Phillips3200::_buf_power_on[12] = {
  0xD5, 0x55, 0x0A, 0x01, 0x03, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x2A, 0x10
  //0xD5, 0x55, 0x00, 0x01, 0x03, 0x00, 0x0E, 0x01, 0x00, 0x00, 0x39, 0x39
};
byte Phillips3200::_buf_power_on1[12] = {
  0xD5, 0x55, 0x02, 0x01, 0x03, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x1C, 0x17
  //0xD5, 0x55, 0x01, 0x01, 0x03, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x01, 0x25
};
byte Phillips3200::_buf_power_off[12] = {
  0xD5, 0x55, 0x00, 0x01, 0x03, 0x00, 0x0E, 0x01, 0x00, 0x00, 0x39, 0x39
};
byte Phillips3200::_buf_espresso[12] = {
  0xD5, 0x55, 0x00, 0x01, 0x03, 0x00, 0x0E, 0x02, 0x00, 0x00, 0x2D, 0x2F
};
byte Phillips3200::_buf_coffee[12] = {
  0xd5, 0x55, 0x00, 0x01, 0x03, 0x00, 0x0e, 0x08, 0x00, 0x00, 0x1d, 0x1e
};
byte Phillips3200::_buf_americano[12] = {
  0xD5, 0x55, 0x00, 0x01, 0x03, 0x00, 0x0E, 0x20, 0x00, 0x00, 0x04, 0x15
};
byte Phillips3200::_buf_cappuccino[12] = {
  0xD5, 0x55, 0x00, 0x01, 0x03, 0x00, 0x0E, 0x04, 0x00, 0x00, 0x05, 0x03
};
byte Phillips3200::_buf_latte[12] = {
  0xD5, 0x55, 0x00, 0x01, 0x03, 0x00, 0x0E, 0x10, 0x00, 0x00, 0x2D, 0x24
};
byte Phillips3200::_buf_hot_water[12] = {
  0xD5, 0x55, 0x00, 0x01, 0x03, 0x00, 0x0E, 0x00, 0x01, 0x00, 0x39, 0x38
};
byte Phillips3200::_buf_coffee_strength_level[12] = {
  0xD5, 0x55, 0x00, 0x01, 0x03, 0x00, 0x0E, 0x00, 0x02, 0x00, 0x2D, 0x2D
};
byte Phillips3200::_buf_coffee_water_level[12] = {
  0xD5, 0x55, 0x00, 0x01, 0x03, 0x00, 0x0E, 0x00, 0x04, 0x00, 0x04, 0x07
};//                                                           WL
byte Phillips3200::_buf_coffee_milk_level[12] = {
  0xD5, 0x55, 0x00, 0x01, 0x03, 0x00, 0x0E, 0x00, 0x08, 0x00, 0x1F, 0x16
};
byte Phillips3200::_buf_aqua_clean[12] = {
  0xD5, 0x55, 0x00, 0x01, 0x03, 0x00, 0x0E, 0x00, 0x10, 0x00, 0x29, 0x34
};
byte Phillips3200::_buf_calc_clean[12] = {
  0xD5, 0x55, 0x00, 0x01, 0x03, 0x00, 0x0E, 0x00, 0x20, 0x00, 0x0C, 0x35
};
byte Phillips3200::_buf_start_pause[12] = {
  0xD5, 0x55, 0x00, 0x01, 0x03, 0x00, 0x0E, 0x00, 0x00, 0x01, 0x3D, 0x30
};
byte Phillips3200::_buf_request_info[12] = {
  0xD5, 0x55, 0x00, 0x01, 0x03, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x35, 0x34
};

void Phillips3200::setup(std::function<void ()> on_machine_state_changed) {

  _on_machine_state_changed = on_machine_state_changed;

  MachineSerial.begin(115200);

  #if defined(ESP8266)
  DisplayOutSerial.begin(115200);
  #else
  DisplayOutSerial.begin(115200, SERIAL_8N1, RX2_PIN, TX2_PIN);
  #endif

  pinMode(NPN_E_PIN, OUTPUT);
  digitalWrite(NPN_E_PIN, HIGH);
}

void Phillips3200::loop() {
  _display_out_loop();
  machine_out_loop();
}

/* machine_cmd */
void Phillips3200::_send_buf(const byte buf_command[]) {
  for (uint8_t i = 0; i < 5; i++) {
    MachineSerial.write(buf_command, 12);
  }
}

void Phillips3200::_send_power_on() {
  

  _send_buf(_buf_power_on);
  _send_buf(_buf_power_on1);
  // Workaround: NPN_E_PIN is connected to a NPN Transistor that cuts
  // the ground from the display and MC of the display reboots
  digitalWrite(NPN_E_PIN, LOW);
  delay(300);
  digitalWrite(NPN_E_PIN, HIGH);
}

void Phillips3200::send_cmd(const std::string &command) {
  if (command == "power_on") {
    return _send_power_on();
  }
  if (command == "power_off") {
    return _send_buf(_buf_power_off);
  }
  if (command == "espresso") {
    return _send_buf(_buf_espresso);
  }
  if (command == "coffee") {
    return _send_buf(_buf_coffee);
  }
  if (command == "americano") {
    return _send_buf(_buf_americano);
  }
  if (command == "cappuccino") {
    return _send_buf(_buf_cappuccino);
  }
  if (command == "latte") {
    return _send_buf(_buf_latte);
  }
  if (command == "hot_water") {
    return _send_buf(_buf_hot_water);
  }
  if (command == "coffee_strength_level") {
    return _send_buf(_buf_coffee_strength_level);
  }
  if (command == "coffee_water_level") {
    return _send_buf(_buf_coffee_water_level);
  }
  if (command == "coffee_milk_level") {
    return _send_buf(_buf_coffee_milk_level);
  }
  if (command == "aqua_clean") {
    return _send_buf(_buf_aqua_clean);
  }
  if (command == "calc_clean") {
    return _send_buf(_buf_calc_clean);
  }
  if (command == "start_pause") {
    return _send_buf(_buf_start_pause);
  }
  if (command == "request_info") {
    return _send_buf(_buf_request_info);
  }
}

/* display_out */
void Phillips3200::_display_out_loop() {
  while (DisplayOutSerial.available() > 0) {
    MachineSerial.write(DisplayOutSerial.read());
  }
}

/* machine_out */
void Phillips3200::_on_machine_out_buffer_changed() {
  bool props_changed = false;

  const bool power_status =
    _current_machine_cmd_buf[2] == 0x1 ? 0 : 1;
  if (power_status != current_power_status) {
    props_changed = true;
    current_power_status = power_status;
  }

  const std::string machine_status =
    machine_states_get_machine_status_from_buf(_current_machine_cmd_buf);
  if (machine_status != current_machine_status) {
    props_changed = true;
    current_machine_status = machine_status;
  }

  if (machine_status == "selected") {
    // can calculate brew setting on selected status only
    const std::string brew =
      machine_states_get_brew_from_buf(_current_machine_cmd_buf);
    if (brew != current_brew) {
      props_changed = true;
      current_brew = brew;
    }

    const uint8_t water_level =
      machine_states_get_level_from_byte(_current_machine_cmd_buf[10]);
    if (water_level != current_water_level) {
      props_changed = true;
      current_water_level = water_level;
    }

    const uint8_t grinder_type =
      brew == "hot_water" ?
        0 :
        machine_states_get_grinder_from_byte(_current_machine_cmd_buf[9]);
    if (grinder_type != current_grinder_type) {
      props_changed = true;
      current_grinder_type = grinder_type;
    }

    const uint8_t strength_level =
      grinder_type == 1 ?
        machine_states_get_level_from_byte(_current_machine_cmd_buf[8]) :
        0;
    if (strength_level != current_strength_level) {
      props_changed = true;
      current_strength_level = strength_level;
    }

    const uint8_t milk_level =
      brew == "cappuccino" || brew == "latte" ?
        machine_states_get_level_from_byte(_current_machine_cmd_buf[13]) :
        0;
    if (milk_level != current_milk_level) {
      props_changed = true;
      current_milk_level = milk_level;
    }
  }

  if (machine_status != "selected" &&
      machine_status != "brewing" &&
      machine_status != "unknown") {
    // reset brew settings on all but these statuses
    current_brew = "none";
    current_strength_level = 0;
    current_grinder_type = 0;
    current_water_level = 0;
    current_milk_level = 0;
  }

  if (machine_status == "unknown") {
    // do not report unknown status
    props_changed = false;
  }

  if (props_changed) {
    _on_machine_state_changed();
  }
}

void Phillips3200::machine_out_loop() {
  while (MachineSerial.available() > 0) {
    char b = MachineSerial.read();
    _current_machine_cmd_buf[_machine_cmd_buf_idx++] = b;

    // Skip input if it doesn't start with 0xd5
    if (_machine_cmd_buf_idx == 1 && b != 0xd5) {
      _machine_cmd_buf_idx = 0;
      continue;
    }

    if (_machine_cmd_buf_idx < _machine_cmd_buf_size) {
      continue;
    }

    _machine_cmd_buf_idx = 0;

    if (memcmp(_current_machine_cmd_buf,
               _prev_machine_cmd_buf,
               _machine_cmd_buf_size) == 0) {
      continue;
    }

    memcpy(
        _prev_machine_cmd_buf,
        _current_machine_cmd_buf,
        _machine_cmd_buf_size
    );

    _on_machine_out_buffer_changed();
  }
}

/* machine_states */
uint8_t Phillips3200::machine_states_get_level_from_byte(byte value) {
  switch (value) {
    case 0x0:
      return 1;
    case 0x38:
      return 2;
    case 0x3f:
      return 3;
    default:
      return 255;
  }
}

uint8_t Phillips3200::machine_states_get_grinder_from_byte(byte mode) {
  switch (mode) {
    case 0x7:
      return 1; // "normal";
    case 0x38:
      return 0; // "powder";
    default:
      return 255;
  }
}

std::string Phillips3200::machine_states_get_machine_status_from_buf(
    char *buffer
) {
  if (buffer[2] == 0x1) {
    return "off";
  }

  if (buffer[15] != 0x00) {
    if (buffer[15] == 0x07) {
      return "error_grounds_container";
    }

    return "error";
  }

  if (buffer[14] != 0x00 &&
      // 2 - 13 == 0x00
      // checking 4 & 6 seems to be enough
      buffer[4] == 0x00 &&
      buffer[6] == 0x00) {
    return "error_no_water";
  }

  if (buffer[11] == 0x7) {
    return "selected";
  }

  if ((buffer[2] == 0x00) &&
        (buffer[3] == 0x03 ||
        buffer[4] == 0x03 ||
        buffer[5] == 0x03 ||
        buffer[6] == 0x03 ||
        buffer[6] == 0x18 ||
        buffer[7] == 0x18)) {
    return "heating";
  }

  if (buffer[16] == 0x7) {
    return "brewing";
  }

  if (buffer[15] == 0x00 // no error
        && buffer[14] == 0x00 // water tank ok
        && buffer[13] == 0x00 // calcncclean
        && buffer[3] == 0x07
        && buffer[4] == 0x07
        && buffer[5] == 0x07) {
    // 12 == 0x07 - aqua_clean OK
    if (buffer[12] == 0x00) {
      return "ready_aqua_clean";
    }
    return "ready";
  }

  return "unknown";
}

std::string Phillips3200::machine_states_get_brew_from_buf(char *buffer) {
  std::string s = machine_states_get_machine_status_from_buf(buffer);
  if (s == "selected" || s == "brewing") {
      if (buffer[3] == 0x7) { return "espresso"; }
      if (buffer[3] == 0x38) { return "2x_espresso"; }
      if (buffer[5] == 0x7)  { return "coffee"; }
      if (buffer[5] == 0x38) { return "2x_coffee"; }
      if (buffer[6] == 0x38) { return "americano"; }
      if (buffer[7] == 0x7) { return "2x_americano"; }
      if (buffer[4] == 0x7) { return "cappuccino"; }
      if (buffer[6] == 0x7) { return "latte"; }
      if (buffer[7] == 0x38) { return "hot_water"; }
  }

  return "none";
}
