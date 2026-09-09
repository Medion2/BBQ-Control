#include "Display.h"
#include "Graphics.h"
#include "WiFiManager.h"
#include "HomeAssistant.h"
void setup() {
  Serial.begin(115200);
  delay(300);
  displayBegin();
  graphicsBegin();
  wifiBegin();
  haBegin();
}
void loop() {
  wifiUpdate();
  haUpdate();
  graphicsUpdate();
  delay(1);
}
