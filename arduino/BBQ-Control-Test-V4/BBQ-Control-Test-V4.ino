#include "Display.h"
#include "Touch.h"
#include "Graphics.h"
#include "WiFiManager.h"
#include "Clock.h"
void setup() {
  Serial.begin(115200);
  delay(300);
  displayBegin();
  graphicsBegin();
  wifiBegin();
  Serial.println("BBQ-Control V4: n/p wechseln die Testseite.");
}
void loop() {
  wifiUpdate();
  clockUpdate();
  touchUpdate();
  graphicsUpdate();
  delay(1);
}
