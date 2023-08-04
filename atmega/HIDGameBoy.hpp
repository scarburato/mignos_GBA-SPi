#pragma once

#include "HID.hpp"

class HIDGameBoy  {
  uint8_t hat = 0x0f;
  uint8_t buttons = 0;

public:
  enum Button {
    BUTTON_A = 0,
    BUTTON_B,
    BUTTON_SELECT,
    BUTTON_START
  };

  enum HatState {
    HATSWITCH_UP           = 0x00,
    HATSWITCH_UPRIGHT      = 0x01,
    HATSWITCH_RIGHT        = 0x02,
    HATSWITCH_DOWNRIGHT    = 0x03,
    HATSWITCH_DOWN         = 0x04,
    HATSWITCH_DOWNLEFT     = 0x05,
    HATSWITCH_LEFT         = 0x06,
    HATSWITCH_UPLEFT       = 0x07,
    HATSWITCH_NONE         = 0x0F
  };

  HIDGameBoy();

  void inline setButton(Button btn, bool pressed) {
    bitWrite(buttons, btn, pressed);
  };

  void inline setHat(HatState state) {
    hat = state;
  };

  void begin();
  void sendState();
};