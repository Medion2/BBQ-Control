#include "TemperatureGauge.h"
#include "ReferenceRing.h"
#include <algorithm>
uint16_t TemperatureGauge::color(float p){
 const uint16_t stops[]={Theme::Blue,Theme::Green,Theme::rgb(0xFFD000),Theme::Orange,Theme::Red};
 float t=std::max(0.f,std::min(3.999f,p*4));int i=int(t);float f=t-i;uint16_t a=stops[i],b=stops[i+1];
 return (int(((a>>11)&31)*(1-f)+((b>>11)&31)*f)<<11)|(int(((a>>5)&63)*(1-f)+((b>>5)&63)*f)<<5)|int((a&31)*(1-f)+(b&31)*f);
}
void TemperatureGauge::draw(Widgets&w,float core,float progress,bool valid,bool fahrenheit,int trend,const String&age){
 w.region(184,64,284,312);w.symbol(263,6,6,Theme::Grey);
 for(uint32_t p:ReferenceRing){float a=((p>>16)&255)/255.f;w.pixel(22+(p&255),36+((p>>8)&255),valid&&a<=progress?color(a):Theme::Track,(p>>24)/255.f);}
 for(int i=0;i<=60;++i){float q=(135+i*4.5f)*.0174532925f,r=i%5==0?100:103;w.line(146+cosf(q)*106,160+sinf(q)*106,146+cosf(q)*r,160+sinf(q)*r,1,valid&&i/60.f<=progress?color(i/60.f):Theme::Track);}
 w.symbol(146,88,1,Theme::Red);
 String v=valid?String(fahrenheit?core*1.8f+32:core,1):"--";int tw=w.textWidth(v,HeroFont);float scale=std::min(.85f,180.f/tw);int x=146-(tw*scale+26)/2;
 w.textScaled(x,109,v,HeroFont,Theme::White,scale);int unit=x+tw*scale+5;w.circle(unit,146,2,1,Theme::White);w.text(unit+5,143,fahrenheit?"F":"C",LabelFont,Theme::White);
 const char *label=!valid?"KEINE DATEN":trend==2?"TREND WARTET":trend>0?"STEIGT":trend<0?"FAELLT":"STABIL";
 w.centered(146,207,label,SmallFont,valid&&trend==1?Theme::Green:Theme::Grey);
 if(valid&&trend!=2){if(trend==0)w.line(140,238,152,238,2,Theme::Grey);else {int sign=trend>0?1:-1;w.line(140,237+sign*3,146,237-sign*3,2,Theme::Grey);w.line(146,237-sign*3,152,237+sign*3,2,Theme::Grey);}}
 w.centered(146,254,"LETZTES UPDATE",SmallFont,Theme::Grey);w.centered(146,274,age,SmallFont,Theme::White);w.present();
}

void TemperatureGauge::drawAge(Widgets&w,const String&age){
 // This rectangle sits below the ring and trend arrow; updating age must not redraw them.
 w.region(184,318,284,46);
 w.centered(146,0,"LETZTES UPDATE",SmallFont,Theme::Grey);
 w.centered(146,20,age,SmallFont,Theme::White);w.present();
}
