#include "TemperatureGauge.h"
#include <math.h>
#include <algorithm>
#include "Config.h"
uint16_t TemperatureGauge::color(float p){return p<.5f?Theme::Blue:p<.8f?Theme::Green:p<1.f?Theme::Orange:Theme::Red;}
void TemperatureGauge::draw(Widgets&w,float core,float target,bool valid,float arc){
 if(!w.region(0,40,480,240))return;
 float progress=valid&&isfinite(target)&&target>0?std::max(0.f,std::min(1.f,core/target)):0;
 if(arc>=0)progress=arc;
 // 270 degree arc, clockwise from lower left. Four temperature-progress bands.
 for(int y=6;y<228;++y)for(int x=129;x<351;++x){
  float dx=x-240,dy=y-114,r=hypotf(dx,dy);float angle=atan2f(dy,dx)*57.2957795f;
  float a=angle-135;while(a<0)a+=360;
  if(a>270)continue;float alpha=std::min(1.f,5.f-fabsf(r-103));
  w.pixel(x,y,a<=progress*270?color(a/270):Theme::Panel,alpha);
 }
 w.centered(240,58,valid?String(core,1):"--",HeroFont,Theme::White);
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
