#pragma once
#include "Widgets.h"
#include "TemperatureGauge.h"
#include "HistoryChart.h"
#include "StatusBar.h"
#include "TouchManager.h"
#include "SettingsPage.h"
class Dashboard {
 Widgets widgets;TemperatureGauge gauge;HistoryChart history[5];StatusBar bar;
 TouchManager touch;SettingsPage settings;
 uint32_t last=0,perfAt=0,frames=0;String previous,cards[4],detailKey,probeKey,bannerKey,headerKey;int states=-1,page=0;
 bool ntpStarted=false;
 float arc=0;bool ready=false,force=true;unsigned shown=99;
 void navigate(int destination);void header();void details();
 public:void begin();void update();
};
