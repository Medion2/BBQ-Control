#pragma once
#include "Widgets.h"
class TemperatureGauge {
 public:
 void draw(Widgets &w,float core,float target,bool valid);
 static uint16_t color(float ratio);
};
