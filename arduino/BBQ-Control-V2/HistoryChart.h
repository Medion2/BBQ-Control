#pragma once
#include "Widgets.h"
class HistoryChart {
 float samples[60];uint32_t last=0;unsigned head=0,count=0;
 public:
 HistoryChart();bool sample(uint32_t now,float value);void draw(Widgets&w,float target=NAN);
};
