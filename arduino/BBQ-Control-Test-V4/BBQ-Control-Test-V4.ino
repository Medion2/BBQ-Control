#include "Display.h"
void setup() {
  Serial.begin(115200);
  delay(300);
  displayBegin();
  gfx->fillScreen(0x0000);
  gfx->setTextColor(0xffff);
  gfx->setTextSize(3);
  gfx->setCursor(60, 220);
  gfx->print("BBQ-Control V4");
}
void loop() { delay(1); }
