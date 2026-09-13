#include "HistoryChart.h"
#include <algorithm>
HistoryChart::HistoryChart(){for(auto &v:samples)v=NAN;}
bool HistoryChart::sample(uint32_t now,float v){
 if(count&&uint32_t(now-last)<60000){float &current=samples[(head+Capacity-1)%Capacity];if((!isfinite(current)&&!isfinite(v))||current==v)return false;current=v;return true;}
 uint32_t steps=count?uint32_t(now-last)/60000:1;if(steps>Capacity)steps=Capacity;
 for(uint32_t i=0;i<steps;++i){samples[head]=i+1==steps?v:NAN;head=(head+1)%Capacity;if(count<Capacity)++count;}last=now;return true;
}
HistoryStats HistoryChart::stats(unsigned window)const{
 HistoryStats s;float sum=0;for(unsigned i=0;i<std::min(window,count);++i){float v=recent(i);if(!isfinite(v))continue;
  if(!s.count){s.low=s.high=v;}else{s.low=std::min(s.low,v);s.high=std::max(s.high,v);}sum+=v;++s.count;}
 if(s.count)s.mean=sum/s.count;return s;
}
int HistoryChart::trend()const{for(unsigned i=0;i<4;++i)if(!isfinite(recent(i)))return 2;float d=recent(0)-recent(3);return d>.3f?1:d<-.3f?-1:0;}
void HistoryChart::drawPage(Widgets&w,const HistoryChart&a,float target,unsigned mode,unsigned window,bool fahrenheit)const{
 auto cv=[&](float v){return fahrenheit?v*1.8f+32:v;};
 auto core=stats(window),air=a.stats(window);bool showCore=mode!=1,showAir=mode!=0;
 float lo=INFINITY,hi=-INFINITY;
 auto include=[&](float v){if(isfinite(v)){lo=std::min(lo,cv(v));hi=std::max(hi,cv(v));}};
 if(showCore){include(core.low);include(core.high);}if(showAir){include(air.low);include(air.high);}
 bool any=isfinite(lo);if(any&&showCore)include(target);
 w.region(12,100,456,216);w.roundRect(0,0,456,216,8,Theme::Panel);
 const char *labels[]={"Kern","Ziel","Garraum"};uint16_t colors[]={Theme::Red,Theme::Grey,Theme::Orange};int xs[]={12,118,245};
 for(int i=0;i<3;++i){w.line(xs[i],14,xs[i]+14,14,2,colors[i]);w.text(xs[i]+21,6,labels[i],SmallFont,colors[i]);}
 w.text(425,6,fahrenheit?"F":"C",SmallFont,Theme::Grey);
 if(!any){w.centered(228,92,"Noch keine Messwerte",LabelFont,Theme::Grey);w.present();}
 else {
  float pad=std::max(2.f,(hi-lo)*.1f);lo-=pad;hi+=pad;
  auto yy=[&](float v){return 166-(cv(v)-lo)/(hi-lo)*132;};
  for(int i=0;i<5;++i){int y=34+i*33;w.line(43,y,434,y,1,Theme::Track);w.text(4,y-7,String(hi-(hi-lo)*i/4,0),SmallFont,Theme::Grey);}
  if(showCore&&isfinite(target))for(int x=43;x<434;x+=10)w.line(x,yy(target),std::min(x+5,434),yy(target),1,Theme::Grey);
  auto curve=[&](const HistoryChart&h,uint16_t c){for(unsigned i=1;i<window;++i){float v=h.recent(i),next=h.recent(i-1);if(isfinite(v)&&isfinite(next))w.line(434-i*391.f/(window-1),yy(v),434-(i-1)*391.f/(window-1),yy(next),2,c);}if(isfinite(h.recent(0)))w.circle(434,yy(h.recent(0)),1,2,c);};
  if(showCore)curve(*this,Theme::Red);if(showAir)curve(a,Theme::Orange);
  w.text(43,186,"-"+String(window)+" min",SmallFont,Theme::Grey);w.centered(235,186,"-"+String(window/2)+" min",SmallFont,Theme::Grey);w.text(403,186,"Jetzt",SmallFont,Theme::Grey);w.present();
 }
 HistoryStats s=mode==1?air:core;float vals[]={s.low,s.high,s.mean};const char *names[]={"MIN","MAX","MITTEL"};
 for(int i=0;i<3;++i){w.region(12+i*154,330,148,76);w.roundRect(0,0,148,76,7,Theme::Panel);w.centered(74,10,String(names[i])+(mode==1?" GARRAUM":" KERN"),SmallFont,Theme::Grey);
  String value=isfinite(vals[i])?String(cv(vals[i]),1)+(fahrenheit?" F":" C"):"--";int tw=w.textWidth(value,ValueFont);float scale=std::min(1.f,136.f/tw);w.textScaled((148-tw*scale)/2,36,value,ValueFont,Theme::White,scale);w.present();}
}
