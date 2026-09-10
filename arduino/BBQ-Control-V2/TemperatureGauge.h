#pragma once
#include "Widgets.h"
class TemperatureGauge {
 public:
 void draw(Widgets &w,float core,float target,bool valid,float arc=-1,bool localTarget=false);
 static uint16_t color(float ratio);
};
