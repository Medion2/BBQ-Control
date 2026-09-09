#include "Dashboard.h"
#include "ProbeData.h"
#include "HomeAssistant.h"
#include "WiFiManager.h"
void Dashboard::begin(){
 if(!widgets.begin()){Serial.println("UI: buffer allocation failed");return;}
 widgets.region(0,0,480,40);widgets.text(20,10,"BBQ CONTROL",LabelFont,Theme::White);widgets.text(412,10,"2.0",LabelFont,Theme::Orange);widgets.present();
 update();
}
void Dashboard::update(){
 uint32_t now=millis();if(now-last<16)return;last=now;
 const ProbeData &d=probeData();bool valid=probeFresh(now)&&haLive();
 String key=String(valid)+":"+String(d.core,1)+":"+String(d.ambient,1)+":"+String(d.target,1)+":"+String(d.battery,0);
 if(key!=previous){previous=key;gauge.draw(widgets,d.core,d.target,valid);
  const char *labels[]={"Garraum","Zieltemperatur","Akku","Restzeit"};
  String vals[]={valid&&isfinite(d.ambient)?String(d.ambient,1)+" C":"--",valid&&isfinite(d.target)?String(d.target,1)+" C":"--",valid&&isfinite(d.battery)?String(d.battery,0)+" %":"--","--"};
  int icons[]={7,6,4,5};
  for(int i=0;i<4;++i){widgets.region(16+(i%2)*232,280+(i/2)*60,216,56);widgets.roundRect(0,0,216,56,13,Theme::Panel);widgets.icon(22,19,icons[i],Theme::Orange);widgets.text(44,5,labels[i],SmallFont,Theme::Grey);widgets.text(44,22,vals[i],ValueFont,Theme::White);widgets.present();}
 }
 if(history.sample(now,valid?d.core:NAN))history.draw(widgets);
 int s=wifiConnected()|(haLive()<<1)|(valid<<2)|(isfinite(d.battery)<<3);
 if(s!=states){states=s;bar.draw(widgets,s&1,s&2,s&4,s&8);}
}
