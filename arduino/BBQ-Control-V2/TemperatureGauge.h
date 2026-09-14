#pragma once
#include "Widgets.h"
class TemperatureGauge {
 public:
 void draw(Widgets&w,float core,float progress,bool valid,bool fahrenheit,int trend,const String &age);
 void drawAge(Widgets&w,const String&age);
 static uint16_t color(float progress);
};
