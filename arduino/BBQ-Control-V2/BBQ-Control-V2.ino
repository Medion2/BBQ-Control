#include "Display.h"
#include "Dashboard.h"
#include "WiFiManager.h"
#include "HomeAssistant.h"
Dashboard dashboard;
void setup(){Serial.begin(115200);delay(300);displayBegin();wifiBegin();haBegin();dashboard.begin();}
void loop(){wifiUpdate();haUpdate();dashboard.update();delay(1);}
