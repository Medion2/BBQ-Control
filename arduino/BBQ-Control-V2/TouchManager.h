#pragma once
#include <Arduino.h>
struct TouchEvent { enum Kind {None,Tap,Left,Right} kind=None; int x=0,y=0; };
class TouchManager {
 uint8_t address=0x5d;bool down=false;int sx=0,sy=0,x=0,y=0;uint32_t start=0,last=0,report=0;
 bool read(uint16_t reg,uint8_t *data,int count);
 public:void begin();TouchEvent update();
};
