#include "HistoryChart.h"
#include <math.h>
#include <algorithm>
HistoryChart::HistoryChart(){for(auto &v:samples)v=NAN;}
bool HistoryChart::sample(uint32_t now,float v){
 if(count && uint32_t(now-last)<60000){
  float &current=samples[(head+59)%60];
  if((!isfinite(current)&&!isfinite(v))||current==v)return false;
  current=v;return true;
 }
 uint32_t steps=count?uint32_t(now-last)/60000:1;
 if(steps>60)steps=60;
 for(uint32_t i=0;i<steps;++i){samples[head]=i+1==steps?v:NAN;head=(head+1)%60;if(count<60)++count;}
 last=now;return true;
}
void HistoryChart::draw(Widgets&w){
 if(!w.region(0,400,480,48))return;
 float lo=INFINITY,hi=-INFINITY;
 for(float v:samples)if(isfinite(v)){lo=std::min(lo,v);hi=std::max(hi,v);}
 w.text(20,0,"60 min",SmallFont,Theme::Grey);
 if(!isfinite(lo)){w.centered(270,13,"Verlauf wartet auf Daten",SmallFont,Theme::Grey);w.present();return;}
 if(hi-lo<2){lo-=1;hi+=1;}
 w.text(20,20,String(lo,0)+"-"+String(hi,0)+" C",SmallFont,Theme::Grey);
 w.line(100,39,460,39,1,Theme::Panel);
 if(count==1&&isfinite(samples[(head+59)%60]))w.circle(460,36-(samples[(head+59)%60]-lo)/(hi-lo)*30,1,2,Theme::Orange);
 for(unsigned n=1;n<count;++n){
  float a=samples[(head+60-count+n-1)%60],b=samples[(head+60-count+n)%60];
  if(isfinite(a)&&isfinite(b))w.line(100+(60-count+n-1)*360.f/59,36-(a-lo)/(hi-lo)*30,100+(60-count+n)*360.f/59,36-(b-lo)/(hi-lo)*30,2,Theme::Orange);
 }w.present();
}
