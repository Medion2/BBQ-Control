#pragma once
#include <Arduino.h>
#include "Fonts.h"
namespace Theme {
constexpr uint16_t rgb(unsigned c) { return ((c>>8)&0xf800)|((c>>5)&0x07e0)|((c>>3)&31); }
constexpr uint16_t Background=rgb(0x101114),Panel=rgb(0x1B1D23),Orange=rgb(0xFF7A00),White=0xffff;
constexpr uint16_t Grey=rgb(0xADB2BE),Blue=rgb(0x409CFF),Green=rgb(0x45D391),Red=rgb(0xF45364);
}
class Widgets {
 public:
 bool begin();
 bool region(int x,int y,int w,int h);
 void present();
 void pixel(int x,int y,uint16_t color,float alpha=1);
 void rect(int x,int y,int w,int h,uint16_t color);
 void roundRect(int x,int y,int w,int h,int radius,uint16_t color);
 void line(float x,float y,float x2,float y2,float thickness,uint16_t color);
 void circle(float x,float y,float radius,float thickness,uint16_t color);
 int textWidth(const String &s,const SmoothFont &font);
 void text(int x,int y,const String &s,const SmoothFont &font,uint16_t color);
 void centered(int x,int y,const String &s,const SmoothFont &font,uint16_t color);
 void icon(int x,int y,int type,uint16_t color);
 bool doubled() const { return buffers[1]!=nullptr; }
 private:
 uint16_t *buffers[2]={},*pixels=nullptr; int active=0,rx=0,ry=0,width=0,height=0;
};
