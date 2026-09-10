#pragma once
#include "Widgets.h"
#include "TouchManager.h"
class SettingsPage {
 float targets[5]={NAN,NAN,NAN,NAN,NAN};float editing=60;unsigned probe=0;
 public:
 float target(unsigned i,float remote)const{return isfinite(targets[i])?targets[i]:remote;}
 bool local(unsigned i)const{return isfinite(targets[i]);}
 void open(unsigned i,float remote){probe=i;editing=target(i,remote);if(!isfinite(editing))editing=60;}
 void draw(Widgets&w);
 int touch(TouchEvent e); // 0 unchanged, 1 redraw, 2 close
};
