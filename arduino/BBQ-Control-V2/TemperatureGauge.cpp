#include "TemperatureGauge.h"
#include <math.h>
#include <algorithm>
#include "Config.h"
#include "RingPixels.h"
uint16_t TemperatureGauge::color(float p){return p<.5f?Theme::Blue:p<.8f?Theme::Green:p<1.f?Theme::Orange:Theme::Red;}
void TemperatureGauge::draw(Widgets&w,float core,float target,bool valid,float arc){
 if(!w.region(0,40,480,240))return;
 float progress=valid&&isfinite(target)&&target>0?std::max(0.f,std::min(1.f,core/target)):0;
 if(arc>=0)progress=arc;
 // 270 degree arc, clockwise from lower left. Four temperature-progress bands.
 for(uint32_t p:RingPixels){
  unsigned angle=(p>>16)&255;
  w.pixel(129+(p&255),6+((p>>8)&255),valid&&angle<=progress*255?color(angle/255.f):Theme::Panel,(p>>24)/255.f);
 }
 String value=valid?String(core,1):"--";
 // Keep wide, three-digit readings inside the ring without shrinking the hero.
 if(valid&&w.textWidth(value,HeroFont)>185)value=String(core,0);
 w.centered(240,58,value,HeroFont,Theme::White);
 w.circle(229,150,2,1.5,Theme::Grey);w.text(236,140,"C",LabelFont,Theme::Grey);
 w.centered(240,167,"Kerntemperatur",LabelFont,Theme::Grey);
 const char *label="Warte auf Messung";uint16_t tone=Theme::Orange;
 if(valid){
  label=core<Config::RareMin?"Rind / Aufheizen":core<Config::MediumRareMin?"Rind / Rare":core<Config::MediumMin?"Rind / Medium Rare":core<Config::MediumWellMin?"Rind / Medium":core<Config::WellDoneMin?"Rind / Medium Well":"Rind / Well Done";
  tone=core<Config::MediumRareMin?Theme::Blue:core<Config::MediumMin?Theme::Green:core<Config::WellDoneMin?Theme::Orange:Theme::Red;
 }
 w.centered(240,216,label,SmallFont,tone);
 w.present();
}
