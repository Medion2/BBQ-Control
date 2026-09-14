#pragma once
#include "Widgets.h"
#include "TemperatureGauge.h"
#include "HistoryChart.h"
#include "TouchManager.h"
#include "UiState.h"
class Dashboard {
 Widgets widgets; TemperatureGauge gauge; HistoryChart history[5],ambient[5];
 TouchManager touch; UiState ui;
 bool ready=false,force=true,ntpStarted=false,dimmed=false,pending[5]={},tripped[5]={};
 uint32_t last=0,lastInteraction=0,toastUntil=0;
 float arc=0,editing=60,alarmTargets[5]={NAN,NAN,NAN,NAN,NAN};
 int page=0,backPage=0,activeAlarm=-1; unsigned shown=99,plotMode=2,plotWindow=120;
 String headerKey,bodyKey,gaugeKey,gaugeAge,footerKey,rows[7],toast;
 void navigate(int); void header(); void nav(); void home(); void chart(bool);
 void probes(); void details(); void settings(); void info(); void targetPage();
 void handle(const TouchEvent&); void alarms(); void footer(); void save(); void openTarget();
 void row(int,const String&,const String&,uint16_t,bool toggle=false,bool on=false);
 public: void begin(); void update();
};
