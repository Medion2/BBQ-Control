#pragma once
#include "Widgets.h"
struct HistoryStats { float low=NAN,high=NAN,mean=NAN;unsigned count=0; };
class HistoryChart {
 static constexpr unsigned Capacity=120;
 float samples[Capacity];uint32_t last=0;unsigned head=0,count=0;
 public:
 HistoryChart();bool sample(uint32_t now,float value);
 float recent(unsigned ago)const{return ago<count?samples[(head+Capacity-1-ago)%Capacity]:NAN;}
 HistoryStats stats(unsigned window)const;
 int trend()const; // 2 unknown, -1 falling, 0 stable, 1 rising
 void drawPage(Widgets&w,const HistoryChart &ambient,float target,unsigned mode,unsigned window,bool fahrenheit)const;
};
