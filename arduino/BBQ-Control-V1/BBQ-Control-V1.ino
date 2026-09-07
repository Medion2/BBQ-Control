#include "Display.h"
#include "Graphics.h"
#include "WiFiManager.h"
void setup() {
  Serial.begin(115200);
  delay(300);
  displayBegin();
  graphicsBegin();
  wifiBegin();
}
void loop() {
  wifiUpdate();
  graphicsUpdate();
  delay(1);
}
