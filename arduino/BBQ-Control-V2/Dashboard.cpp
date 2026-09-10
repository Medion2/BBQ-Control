#include "Dashboard.h"
#include "ProbeData.h"
#include "HomeAssistant.h"
#include "WiFiManager.h"
#include "Config.h"
#include <time.h>
namespace {
bool fresh(const ProbeData&d,uint32_t now){return haLive()&&d.received&&d.probeOnline&&uint32_t(now-d.receivedAt)<Config::StaleMs;}
String temp(float v,bool ok){return ok&&isfinite(v)?String(v,1)+" C":"--";}
}
void Dashboard::header(){
 char clock[8]="--:--";time_t utc=time(nullptr);struct tm local;
 if(utc>1700000000&&localtime_r(&utc,&local))strftime(clock,sizeof(clock),"%H:%M",&local);
 String key=String(page)+":"+clock+":"+String(wifiConnected())+String(haLive());
 if(key==headerKey)return;headerKey=key;
 widgets.region(0,0,480,40);
 if(page)widgets.text(16,10,"< Zurueck",LabelFont,Theme::White);
 else {widgets.icon(24,13,3,Theme::Red);widgets.text(43,6,"MEATER",ValueFont,Theme::White);}
 widgets.icon(330,12,0,wifiConnected()?Theme::Blue:Theme::Red);
 widgets.icon(365,12,1,haLive()?Theme::Blue:Theme::Red);
 widgets.text(407,10,clock,LabelFont,Theme::White);widgets.present();
}
void Dashboard::navigate(int destination){
 page=destination;force=true;previous="";detailKey="";probeKey="";bannerKey="";headerKey="";states=-1;for(auto &c:cards)c="";
 // Clear the changing body once on navigation, including the lower cards.
 for(int y=40;y<480;y+=220){widgets.region(0,y,480,220);widgets.present();}
 header();if(page==2)settings.draw(widgets);
}
void Dashboard::begin(){
 ready=widgets.begin();if(!ready){Serial.println("UI: buffer allocation failed");return;}
 // Initial background in strips, never a full-screen framebuffer refresh in loop.
 for(int y=0;y<480;y+=16){widgets.region(0,y,480,16);widgets.present();}
 touch.begin();header();update();
}
void Dashboard::details(){
 const auto &d=probeData();bool ok=fresh(d,millis());
 String key=String(ok)+String(d.ambient,1)+String(d.peak,1)+String(d.elapsed,0)+String(d.cooking)+haStatusText()+String(page)+d.cookState;
 if(key==detailKey&&!force)return;detailKey=key;
 widgets.region(0,40,480,204);
 widgets.text(24,16,page==3?"Verbindungen":"Garraum / Details",ValueFont,Theme::White);
 if(page==3){widgets.text(24,72,wifiStatusText(),LabelFont,Theme::Grey);widgets.text(24,112,haStatusText(),LabelFont,Theme::Grey);widgets.text(24,152,"MQTT deaktiviert",LabelFont,Theme::Grey);}
 else {widgets.centered(240,62,ok?String(d.ambient,1):"--",HeroFont,Theme::White);widgets.centered(240,161,"Garraum / C",LabelFont,Theme::Grey);}
 widgets.present();widgets.region(0,244,480,204);
 if(page==3){widgets.text(24,20,ok?"Meater verbunden":"Meater ohne aktuelle Daten",LabelFont,Theme::Grey);widgets.text(24,62,"Akku: kein Sensor vorhanden",LabelFont,Theme::Orange);widgets.text(24,110,widgets.doubled()?"Zwei Widget-Puffer im PSRAM":"Ein Widget-Puffer",SmallFont,Theme::Grey);}
 else {widgets.text(24,16,"Spitze: "+temp(d.peak,ok),LabelFont,Theme::Grey);widgets.text(24,56,"Garzeit: "+(ok&&isfinite(d.elapsed)?String(d.elapsed/60,0)+" min":"--"),LabelFont,Theme::Grey);widgets.text(24,96,"Gericht: "+String(d.cooking).substring(0,28),LabelFont,Theme::Grey);widgets.text(24,136,"HA-Status: "+String(d.cookState),SmallFont,Theme::Grey);}
 widgets.present();force=false;
}
void Dashboard::update(){
 if(!ready)return;
 TouchEvent e=touch.update();
 if(page!=2&&(e.kind==TouchEvent::Left||e.kind==TouchEvent::Right)){haSelect((haSelected()+(e.kind==TouchEvent::Left?1:4))%5);arc=0;navigate(0);}
 if(page==2){int action=settings.touch(e);if(action==1)settings.draw(widgets);if(action==2)navigate(0);}
 else if(e.kind==TouchEvent::Tap){
  if(page&&e.y<40)navigate(0);
  else if(page&&e.y>=448)navigate(page==3?0:3);
  else if(!page){
   if(e.x>=300&&e.y>=42&&e.y<110)navigate(1);
   else if(e.x>=12&&e.x<292&&e.y>=42&&e.y<282){settings.open(haSelected(),probeData().target);navigate(2);}
   else if((e.x>=300&&e.y>=264&&e.y<332)||(e.x>=310&&e.x<390&&e.y<40))navigate(3);
  }
 }
 uint32_t now=millis();if(now-last<16)return;last=now;
 if(!ntpStarted&&wifiConnected()){configTzTime("CET-1CEST,M3.5.0,M10.5.0/3","pool.ntp.org","time.nist.gov");ntpStarted=true;}
 header();
 bool chartChanged=false;
 for(unsigned i=0;i<5;++i){const auto &p=haProbe(i);bool changed=history[i].sample(now,fresh(p,now)?p.core:NAN);if(i==haSelected())chartChanged=changed;}
 const ProbeData &d=probeData();bool valid=fresh(d,now);unsigned selected=haSelected();
 if(shown!=selected){shown=selected;header();force=true;}
 if(page==1||page==3)details();
 if(page==0){
  float target=settings.target(selected,d.target);
  float wanted=valid&&isfinite(target)&&target>0?fminf(1,fmaxf(0,d.core/target)):0;
  bool moving=fabsf(arc-wanted)>.002f;
  arc=moving?arc+(wanted-arc)*.2f:wanted;
  String key=String(valid)+":"+String(d.core,1)+":"+String(target,1)+":"+String(settings.local(selected));
  bool dataChanged=key!=previous;
  bool drawn=false;
  if(force||dataChanged||moving){previous=key;gauge.draw(widgets,d.core,target,valid,arc,settings.local(selected));drawn=true;}
  const char *labels[]={"GARRAUM","RESTZEIT (HA)","AKKUSTAND","VERBINDUNG"};
  float remaining=d.remaining-(now-d.receivedAt)/1000.f;
  char duration[16]="--:--";
  if(valid&&isfinite(d.remaining)){unsigned minutes=unsigned(ceilf(fmaxf(0,remaining)/60));snprintf(duration,sizeof(duration),"%02u:%02u",minutes/60,minutes%60);}
  String vals[]={temp(d.ambient,valid),String(duration),valid&&isfinite(d.battery)?String(d.battery,0)+" %":"--",haLive()?"HA online":"HA offline"};
  int icons[]={7,5,4,1};
  for(int i=0;i<4;++i){String k=String(labels[i])+vals[i];if(!force&&cards[i]==k)continue;cards[i]=k;
   widgets.region(300,42+i*74,168,68);widgets.roundRect(0,0,168,68,7,Theme::Panel);
   widgets.text(12,7,labels[i],SmallFont,Theme::Blue);
   uint16_t tone=i==0?Theme::Blue:i==3?(haLive()?Theme::Green:Theme::Red):vals[i].startsWith("--")?Theme::Orange:Theme::Green;
   widgets.icon(22,37,icons[i],tone);
   const SmoothFont &font=i==3||widgets.textWidth(vals[i],ValueFont)>120?LabelFont:ValueFont;
   widgets.text(39,29,vals[i],font,Theme::White);widgets.present();drawn=true;
  }
  if(force||chartChanged||dataChanged){history[selected].draw(widgets,target);drawn=true;}
  String age=d.received?"vor "+String((now-d.receivedAt)/1000)+" s":"noch keine Daten";
  String pk=haProbeName()+age+String(valid);
  if(force||pk!=probeKey){probeKey=pk;widgets.region(300,338,168,130);widgets.roundRect(0,0,168,130,7,Theme::Panel);
   widgets.text(12,8,"MEATER PROBE",SmallFont,Theme::Blue);widgets.text(12,34,haProbeName(),SmallFont,Theme::White);
   widgets.text(12,57,valid?"Aktuelle Daten":"Keine Messung",SmallFont,valid?Theme::Green:Theme::Orange);
   widgets.text(12,83,"Aktualisierung",SmallFont,Theme::Grey);widgets.text(12,101,age,SmallFont,Theme::Grey);
   widgets.line(152,43,140,112,4,Theme::Grey);widgets.line(151,44,148,63,5,Theme::Track);widgets.line(141,109,138,120,1,Theme::White);
   widgets.present();drawn=true;}
  bool reached=valid&&isfinite(target)&&d.core>=target;
  String banner=!valid?"Warte auf Messwerte":!isfinite(target)?"Kein Ziel eingestellt":reached?"Zieltemperatur erreicht!":"Ziel noch nicht erreicht";
  if(force||banner!=bannerKey){bannerKey=banner;widgets.region(12,438,280,30);widgets.roundRect(0,0,280,30,6,reached?Theme::rgb(0x102610):Theme::Panel);
   widgets.icon(18,9,6,reached?Theme::Green:Theme::Orange);widgets.text(36,6,banner,SmallFont,reached?Theme::Green:Theme::White);widgets.present();drawn=true;}
  force=false;
  if(drawn)++frames;
 }
 int s=wifiConnected()|(haLive()<<1)|(valid<<2)|((valid&&isfinite(d.battery))<<3);
 if(page&&s!=states){states=s;bar.draw(widgets,s&1,s&2,s&4,s&8);}
 if(now-perfAt>=5000){Serial.printf("UI changed frames/s: %.1f, buffers: %d\n",frames*1000.f/(now-perfAt),widgets.doubled()?2:1);frames=0;perfAt=now;}
}
