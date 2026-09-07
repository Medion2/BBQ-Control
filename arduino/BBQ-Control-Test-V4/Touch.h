#pragma once
#include <Arduino.h>
struct TouchState {
  bool down = false;
  bool pressed = false;
  int16_t x = 0, y = 0;
  uint8_t points = 0;
  uint32_t errors = 0;
};
void touchUpdate();
const TouchState &touchState();
