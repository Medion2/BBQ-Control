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
void HistoryChart::draw(Widgets&w,float target){
 if(!w.region(12,290,280,140))return;
 w.roundRect(0,0,280,140,7,Theme::Panel);
 w.text(12,8,"TEMPERATURVERLAUF",SmallFont,Theme::Blue);
 w.text(254,8,"C",SmallFont,Theme::Grey);
 float lo=INFINITY,hi=-INFINITY;
 for(float v:samples)if(isfinite(v)){lo=std::min(lo,v);hi=std::max(hi,v);}
 if(!isfinite(lo)){w.centered(140,62,"Warte auf Messwerte",SmallFont,Theme::Grey);w.present();return;}
 if(isfinite(target)){lo=std::min(lo,target);hi=std::max(hi,target);}
 if(hi-lo<4){lo-=2;hi+=2;}else{float pad=(hi-lo)*.1f;lo-=pad;hi+=pad;}
 for(int i=0;i<4;++i){float y=32+i*23.f;w.line(38,y,266,y,1,Theme::Background);w.text(4,int(y)-5,String(hi-(hi-lo)*i/3,0),SmallFont,Theme::Grey);}
 if(isfinite(target)){float y=101-(target-lo)/(hi-lo)*69;for(int x=38;x<264;x+=8)w.line(x,y,x+4,y,1,Theme::Grey);}
 for(unsigned n=1;n<count;++n){float a=samples[(head+60-count+n-1)%60],b=samples[(head+60-count+n)%60];
  if(isfinite(a)&&isfinite(b))w.line(38+(60-count+n-1)*228.f/59,101-(a-lo)/(hi-lo)*69,38+(60-count+n)*228.f/59,101-(b-lo)/(hi-lo)*69,2,Theme::Green);
 }
 float v=samples[(head+59)%60];if(isfinite(v))w.circle(266,101-(v-lo)/(hi-lo)*69,1,2,Theme::Green);
 w.text(30,111,"-60 min",SmallFont,Theme::Grey);w.text(128,111,"-30",SmallFont,Theme::Grey);w.text(236,111,"Jetzt",SmallFont,Theme::Grey);w.present();
}
