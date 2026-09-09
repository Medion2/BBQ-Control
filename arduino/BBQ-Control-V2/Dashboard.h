#pragma once
#include "Widgets.h"
#include "TemperatureGauge.h"
#include "HistoryChart.h"
#include "StatusBar.h"
class Dashboard {
 Widgets widgets;TemperatureGauge gauge;HistoryChart history;StatusBar bar;
 uint32_t last=0;String previous;int states=-1;
 public:void begin();void update();
};
