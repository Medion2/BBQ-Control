#include "Display.h"
#include "Touch.h"
#include "Graphics.h"
void setup() {
  Serial.begin(115200);
  delay(300);
  displayBegin();
  graphicsBegin();
  Serial.println("BBQ-Control V4: n/p wechseln die Testseite.");
}
void loop() {
  touchUpdate();
  graphicsUpdate();
  delay(1);
}
