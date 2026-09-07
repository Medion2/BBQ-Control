#include "Touch.h"
#include "Config.h"
#include <Wire.h>
namespace {
TouchState state;
uint32_t lastPoll = 0, lastReport = 0;
bool readRegister(uint16_t reg, uint8_t *data, uint8_t count) {
  Wire.beginTransmission(Config::TouchAddress);
  Wire.write(uint8_t(reg >> 8)); Wire.write(uint8_t(reg));
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(int(Config::TouchAddress), int(count)) != count) return false;
  for (uint8_t i = 0; i < count; ++i) data[i] = Wire.read();
  return true;
}
bool acknowledge() {
  Wire.beginTransmission(Config::TouchAddress);
  Wire.write(uint8_t(0x81)); Wire.write(uint8_t(0x4E)); Wire.write(uint8_t(0));
  return Wire.endTransmission() == 0;
}
}
void touchUpdate() {
  state.pressed = false;
  uint32_t now = millis();
  if (now - lastPoll < 15) return;
  lastPoll = now;
  if (state.down && now - lastReport > 250) { state.down = false; state.points = 0; }
  uint8_t status;
  if (!readRegister(0x814E, &status, 1)) { ++state.errors; return; }
  // No new report is not a release: retain the current contact state.
  if (!(status & 0x80)) return;
  uint8_t count = status & 15;
  bool valid = count <= 5;
  uint8_t data[4] = {};
  if (valid && count) valid = readRegister(0x8150, data, sizeof(data));
  if (!acknowledge()) valid = false;
  if (!valid) { ++state.errors; return; }
  lastReport = now;
  if (!count) { state.down = false; state.points = 0; return; }
  // GT911: 0x8150=X low, 0x8151=X high, 0x8152=Y low, 0x8153=Y high.
  int16_t x = uint16_t(data[0]) | (uint16_t(data[1]) << 8);
  int16_t y = uint16_t(data[2]) | (uint16_t(data[3]) << 8);
  if (x < 0 || y < 0 || x >= Config::Width || y >= Config::Height) { ++state.errors; return; }
  state.pressed = !state.down;
  state.down = true; state.points = count; state.x = x; state.y = y;
}
const TouchState &touchState() { return state; }
