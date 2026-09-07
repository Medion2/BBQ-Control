#include "Display.h"
#include "Graphics.h"
#include "WiFiManager.h"
#include "MqttManager.h"
void setup() {
  Serial.begin(115200);
  delay(300);
  displayBegin();
  graphicsBegin();
  wifiBegin();
  mqttBegin();
}
void loop() {
  wifiUpdate();
  mqttUpdate();
  graphicsUpdate();
  delay(1);
}
