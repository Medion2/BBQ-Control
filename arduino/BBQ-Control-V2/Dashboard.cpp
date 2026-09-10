#include "Dashboard.h"
#include "ProbeData.h"
#include "HomeAssistant.h"
#include "WiFiManager.h"
#include "Config.h"
namespace {
bool fresh(const ProbeData&d,uint32_t now){return haLive()&&d.received&&d.probeOnline&&uint32_t(now-d.receivedAt)<Config::StaleMs;}
String temp(float v,bool ok){return ok&&isfinite(v)?String(v,1)+" C":"--";}
}
void Dashboard::header(){
 widgets.region(0,0,480,40);widgets.text(20,10,page?"< Zurueck":"BBQ CONTROL",LabelFont,Theme::White);widgets.text(300,12,haProbeName(),SmallFont,Theme::Orange);widgets.present();
}
void Dashboard::navigate(int destination){
 page=destination;force=true;previous="";detailKey="";for(auto &c:cards)c="";
 // Clear only the changing body, once on navigation. Header and status stay separate.
 for(int y=40;y<448;y+=204){widgets.region(0,y,480,204);widgets.present();}
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
  else if(e.y>=448)navigate(page==3?0:3);
  else if(!page&&e.y>=280&&e.y<336){if(e.x<232)navigate(1);else{settings.open(haSelected(),probeData().target);navigate(2);}}
 }
 uint32_t now=millis();if(now-last<16)return;last=now;
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
  String key=String(valid)+":"+String(d.core,1)+":"+String(target,1);
  bool drawn=false;
  if(force||key!=previous||moving){previous=key;gauge.draw(widgets,d.core,target,valid,arc);drawn=true;}
  const char *labels[]={"Garraum",settings.local(selected)?"Ziel lokal":"Zieltemperatur","Akku","Restzeit (HA)"};
  float remaining=d.remaining-(now-d.receivedAt)/1000.f;
  String vals[]={temp(d.ambient,valid),temp(target,isfinite(target)),valid&&isfinite(d.battery)?String(d.battery,0)+" %":"--",valid&&isfinite(d.remaining)?String(ceilf(fmaxf(0,remaining)/60),0)+" min":"--"};
  int icons[]={7,6,4,5};
  for(int i=0;i<4;++i){String k=String(labels[i])+vals[i];if(!force&&cards[i]==k)continue;cards[i]=k;
   widgets.region(16+(i%2)*232,280+(i/2)*60,216,56);widgets.roundRect(0,0,216,56,13,Theme::Panel);widgets.icon(22,19,icons[i],Theme::Orange);widgets.text(44,5,labels[i],SmallFont,Theme::Grey);widgets.text(44,22,vals[i],ValueFont,Theme::White);widgets.present();drawn=true;
  }
  if(force||chartChanged){history[selected].draw(widgets);drawn=true;}force=false;
  if(drawn)++frames;
 }
 int s=wifiConnected()|(haLive()<<1)|(valid<<2)|((valid&&isfinite(d.battery))<<3);
 if(s!=states){states=s;bar.draw(widgets,s&1,s&2,s&4,s&8);}
 if(now-perfAt>=5000){Serial.printf("UI changed frames/s: %.1f, buffers: %d\n",frames*1000.f/(now-perfAt),widgets.doubled()?2:1);frames=0;perfAt=now;}
}
