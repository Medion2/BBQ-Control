#include "Dashboard.h"
#include "ProbeData.h"
#include "HomeAssistant.h"
#include "WiFiManager.h"
#include "Config.h"
#include "StatusBar.h"
#include "SettingsPage.h"
#include <time.h>
#include <algorithm>
namespace {
bool fresh(const ProbeData&d,uint32_t now){return haDataAvailable()&&d.received&&d.probeOnline&&uint32_t(now-d.receivedAt)<Config::StaleMs;}
String name(unsigned i){return i<4?"MEATER "+String(i+1):"PROBE 0AE3B60F";}
String remaining(const ProbeData&d,bool valid){
 if(!valid||!isfinite(d.remaining))return "--:--";
 unsigned m=unsigned(ceilf(fmaxf(0,d.remaining-uint32_t(millis()-d.receivedAt)/1000.f)/60));
 char b[20];snprintf(b,sizeof(b),"%02u:%02u h",m/60,m%60);return b;
}
String started(const ProbeData&d,bool valid){
 time_t t=time(nullptr);if(!valid||!isfinite(d.elapsed)||t<1700000000)return "--";
 t-=time_t(d.elapsed+uint32_t(millis()-d.receivedAt)/1000);struct tm local;localtime_r(&t,&local);
 char b[24];strftime(b,sizeof(b),"%d.%m. %H:%M",&local);return b;
}
}
void Dashboard::begin(){
 ready=widgets.begin();if(!ready){Serial.println("UI: buffer allocation failed");return;}
 ui.begin();Theme::setAmoled(ui.amoled);widgets.brightness(ui.brightness);
 for(int y=0;y<480;y+=16){widgets.region(0,y,480,16);widgets.present();}
 touch.begin();lastInteraction=millis();navigate(0);
}
void Dashboard::navigate(int destination){
 page=destination;force=true;headerKey="";bodyKey="";gaugeKey="";footerKey="";for(auto &r:rows)r="";
 for(int y=48;y<432;y+=192){widgets.region(0,y,480,192);widgets.present();}
 header();nav();
}
void Dashboard::header(){
 char clock[8]="--:--";time_t t=time(nullptr);struct tm local;
 if(t>1700000000&&localtime_r(&t,&local))strftime(clock,sizeof(clock),"%H:%M",&local);
 String key=String(page)+clock+String(wifiConnected());if(key==headerKey)return;headerKey=key;
 widgets.region(0,0,480,48);
 if(page==0){widgets.icon(24,13,0,wifiConnected()?Theme::White:Theme::Red);widgets.symbol(157,24,0,Theme::Orange);widgets.text(179,13,"BBQ CONTROL",LabelFont,Theme::White);widgets.text(411,15,clock,SmallFont,Theme::White);}
 else {widgets.line(20,24,34,24,2,Theme::White);widgets.line(20,24,26,18,2,Theme::White);widgets.line(20,24,26,30,2,Theme::White);
 const char *titles[]={"","TEMPERATURVERLAUF","SONDEN","DETAILS","EINSTELLUNGEN","ZIELTEMPERATUR","VERBINDUNG / INFO"};widgets.centered(254,14,page==3?String("DETAILS / ")+name(haSelected()):String(titles[page]),LabelFont,Theme::White);}
 widgets.line(12,47,468,47,1,Theme::Track);widgets.present();
}
void Dashboard::nav(){
 int selected=page==0?0:page==1?1:page==2||page==3?2:page==5?(backPage==0?0:2):3;
 StatusBar::navigation(widgets,selected);
}
void Dashboard::save(){if(!ui.save()){toast="Speichern fehlgeschlagen";toastUntil=millis()+4000;}force=true;}
void Dashboard::openTarget(){backPage=page;editing=ui.target(haSelected(),probeData().target);if(!isfinite(editing))editing=60;editing=fminf(150,fmaxf(1,editing));navigate(5);}
void Dashboard::home(){
 unsigned s=haSelected();const auto &d=probeData();bool ok=fresh(d,millis()),valid=ok&&isfinite(d.core);float target=ui.target(s,d.target);
 String key=String(valid)+ui.temperature(valid?d.core:NAN)+ui.temperature(target)+ui.grade(s,d.core,valid)+remaining(d,ok)+String(ui.local(s));
 if(force||key!=bodyKey){bodyKey=key;widgets.region(0,48,180,324);
 widgets.text(16,20,"KERNTEMPERATUR",SmallFont,Theme::Grey);
 String v=valid?String(ui.display(d.core),1):"--";float scale=std::min(.65f,132.f/widgets.textWidth(v,HeroFont));
 widgets.textScaled(16,40,v,HeroFont,Theme::White,scale);widgets.circle(19+widgets.textWidth(v,HeroFont)*scale,76,2,1,Theme::White);widgets.text(24+widgets.textWidth(v,HeroFont)*scale,73,ui.fahrenheit?"F":"C",SmallFont,Theme::White);
 for(int y:{108,180,248})widgets.line(16,y,168,y,1,Theme::Track);
 widgets.icon(31,135,6,Theme::White);widgets.text(54,120,ui.local(s)?"ZIEL LOKAL":"ZIELTEMPERATUR",SmallFont,Theme::Grey);widgets.fitted(54,139,118,ui.temperature(target),ValueFont,Theme::Orange);
 widgets.symbol(31,216,1,Theme::Red);widgets.fitted(54,195,118,String("GARSTUFE / ")+ui.meatName(s),SmallFont,Theme::Grey);widgets.fitted(54,216,118,ui.grade(s,d.core,valid),SmallFont,valid?Theme::Red:Theme::Grey);
 widgets.icon(31,280,5,Theme::White);widgets.text(54,259,"RESTZEIT",SmallFont,Theme::Grey);widgets.fitted(54,278,118,remaining(d,ok),ValueFont,Theme::White);widgets.present();}
 float wanted=valid&&isfinite(target)&&target>0?fminf(1,fmaxf(0,d.core/target)):0;bool moving=fabsf(arc-wanted)>.002f;arc=moving?arc+(wanted-arc)*.5f:wanted;
 String age=d.received?"vor "+String(uint32_t(millis()-d.receivedAt)/1000)+" s":"keine Daten";
 String g=key+String(history[s].trend());
 if(force||moving||g!=gaugeKey){gaugeKey=g;gaugeAge=age;gauge.draw(widgets,d.core,arc,valid,ui.fahrenheit,history[s].trend(),age);}
 else if(age!=gaugeAge){gaugeAge=age;gauge.drawAge(widgets,age);}
}
void Dashboard::chart(bool changed){
 float target=ui.target(haSelected(),probeData().target);String key=String(plotMode)+String(plotWindow)+String(target)+String(ui.fahrenheit);
 if(!force&&!changed&&key==bodyKey)return;bodyKey=key;
 widgets.region(0,48,480,44);const char *labels[]={"KERN","GARRAUM","BEIDE"};int xs[]={12,110,224},widths[]={90,104,90};
 for(int i=0;i<3;++i){widgets.roundRect(xs[i],8,widths[i],28,6,plotMode==unsigned(i)?Theme::Orange:Theme::Panel);widgets.centered(xs[i]+widths[i]/2,14,labels[i],SmallFont,Theme::White);}
 widgets.roundRect(346,8,122,28,6,Theme::Panel);widgets.centered(407,14,String(plotWindow)+" MIN",SmallFont,Theme::Grey);widgets.present();
 history[haSelected()].drawPage(widgets,ambient[haSelected()],target,plotMode,plotWindow,ui.fahrenheit);
}
void Dashboard::probes(){
 const uint16_t colors[]={Theme::Red,Theme::Orange,Theme::Blue,Theme::Green,Theme::rgb(0xB48BFA)};
 for(unsigned i=0;i<5;++i){const auto &d=haProbe(i);bool ok=fresh(d,millis());float target=ui.target(i,d.target);String value=ui.temperature(ok?d.core:NAN);String key=value+ui.temperature(target)+String(ok)+String(i==haSelected());if(!force&&rows[i]==key)continue;rows[i]=key;
 widgets.region(12,60+i*70,456,64);widgets.roundRect(0,0,456,64,7,Theme::Panel);if(i==haSelected())widgets.rect(0,9,3,46,Theme::Orange);
 widgets.symbol(25,32,1,colors[i]);widgets.text(51,5,name(i),SmallFont,Theme::White);widgets.fitted(51,24,170,value,ValueFont,colors[i]);widgets.text(233,10,"Ziel: "+ui.temperature(target),SmallFont,Theme::Grey);widgets.text(233,35,ok?"Verbunden":"Keine Daten",SmallFont,ok?Theme::Green:Theme::Grey);
 widgets.circle(423,32,20,4,Theme::Track);float ratio=ok&&isfinite(d.core)&&isfinite(target)&&target>0?fminf(1,fmaxf(0,d.core/target)):0;
 for(int n=0;n<int(ratio*36);++n){float a=(n*10-90)*.0174532925f,b=(n*10-80)*.0174532925f;widgets.line(423+20*cosf(a),32+20*sinf(a),423+20*cosf(b),32+20*sinf(b),4,colors[i]);}widgets.present();}
}
void Dashboard::row(int i,const String &label,const String &value,uint16_t color,bool toggle,bool on){
 String key=label+value+String(color)+String(toggle)+String(on);if(!force&&rows[i]==key)return;rows[i]=key;
 SettingsPage::row(widgets,i,label,value,color,toggle,on,page==4);
}
void Dashboard::details(){
 unsigned s=haSelected();const auto &d=probeData();bool ok=fresh(d,millis());
 row(0,"FLEISCHART",String(ui.meatName(s))+" >",Theme::White);row(1,"GARSTUFE",ui.grade(s,d.core,ok&&isfinite(d.core)),Theme::Red);
 row(2,ui.local(s)?"ZIEL LOKAL":"ZIELTEMPERATUR",ui.temperature(ui.target(s,d.target))+" >",Theme::Orange);row(3,"GARRAUM",ui.temperature(ok?d.ambient:NAN),Theme::Orange);
 row(4,"RESTZEIT",remaining(d,ok),Theme::White);row(5,"GESTARTET",started(d,ok),Theme::Grey);row(6,"ALARM BEI ZIEL","",Theme::Orange,true,ui.alarms[s]);
 if(force){widgets.region(0,404,480,26);widgets.centered(240,4,"Ziel und Alarm gelten lokal auf diesem Display.",SmallFont,Theme::Grey);widgets.present();}
}
void Dashboard::settings(){
 const char *dims[]={"Aus","30 s","1 min","2 min"};row(0,"DISPLAY",String("Dimmen: ")+dims[ui.dimAfter]+" >",Theme::Grey);
 String k=String(ui.brightness);if(force||rows[1]!=k){rows[1]=k;widgets.region(12,107,456,45);widgets.roundRect(0,0,456,45,6,Theme::Panel);widgets.symbol(23,22,7,Theme::Grey);widgets.text(46,14,"HELLIGKEIT",SmallFont,Theme::White);int x=260+(ui.brightness-20)*130/80;widgets.line(260,23,390,23,3,Theme::Track);widgets.line(260,23,x,23,3,Theme::Orange);widgets.circle(x,23,5,10,Theme::White);widgets.text(404,15,String(ui.brightness)+"%",SmallFont,Theme::Grey);widgets.present();}
 row(2,"THEMA",ui.amoled?"AMOLED >":"Dunkel >",Theme::Grey);row(3,"ALARM VISUELL","",Theme::Orange,true,ui.alarms[haSelected()]);row(4,"EINHEITEN",ui.fahrenheit?"Fahrenheit >":"Celsius >",Theme::Grey);row(5,"WLAN",wifiConnected()?"Verbunden >":"Offline >",wifiConnected()?Theme::Green:Theme::Red);row(6,"INFO","BBQ Control 2.1.1 >",Theme::Grey);
}
void Dashboard::info(){
 String key=String(millis()/1000);if(!force&&key==bodyKey)return;bodyKey=key;
 row(0,"WLAN",wifiStatusText(),Theme::Grey);row(1,"IP",wifiAddress(),Theme::White);row(2,"HOME ASSISTANT",haStatusText(),haRecovering()?Theme::Orange:haLive()?Theme::Green:Theme::Red);row(3,"SONDE",fresh(probeData(),millis())?"Aktuelle Daten":"Keine Daten",Theme::Grey);
 row(4,"HEAP FREI",String(ESP.getFreeHeap()/1024)+" KB",Theme::Grey);row(5,"PSRAM FREI",String(ESP.getFreePsram()/1024)+" KB",Theme::Grey);row(6,"VERSION","2.1.1 / 480 x 480",Theme::Orange);
}
void Dashboard::targetPage(){
 String key=String(editing,3)+String(ui.fahrenheit);if(!force&&key==bodyKey)return;bodyKey=key;
 widgets.region(0,48,480,192);widgets.centered(240,23,"LOKALES ANZEIGEZIEL",LabelFont,Theme::Orange);String value=String(ui.display(editing),1);float scale=std::min(1.f,280.f/widgets.textWidth(value,HeroFont));widgets.textScaled(240-widgets.textWidth(value,HeroFont)*scale/2,69,value,HeroFont,Theme::White,scale);widgets.centered(240,160,ui.fahrenheit?"Grad Fahrenheit":"Grad Celsius",SmallFont,Theme::Grey);widgets.present();
 widgets.region(0,240,480,192);widgets.roundRect(24,12,120,60,8,Theme::Panel);widgets.roundRect(336,12,120,60,8,Theme::Panel);widgets.centered(84,25,"-",ValueFont,Theme::White);widgets.centered(396,25,"+",ValueFont,Theme::White);widgets.roundRect(24,92,432,48,8,Theme::Orange);widgets.centered(240,103,"SPEICHERN",LabelFont,Theme::White);widgets.centered(240,158,"Ziel aus Home Assistant verwenden",SmallFont,Theme::Grey);widgets.present();
}
void Dashboard::alarms(){
 int next=-1;uint32_t now=millis();
 for(unsigned i=0;i<5;++i){const auto &d=haProbe(i);float target=ui.target(i,d.target);bool changed=isfinite(target)!=isfinite(alarmTargets[i])||(isfinite(target)&&target!=alarmTargets[i]);if(changed){alarmTargets[i]=target;pending[i]=tripped[i]=false;}
 if(!ui.alarms[i]||!isfinite(target)){pending[i]=tripped[i]=false;continue;}
 bool valid=fresh(d,now)&&isfinite(d.core);if(valid&&d.core<target-.5f)pending[i]=tripped[i]=false;
 if(valid&&d.core>=target&&!tripped[i]){pending[i]=tripped[i]=true;lastInteraction=now;}
 if(next<0&&pending[i]&&valid)next=i;
 }
 if(next!=activeAlarm){activeAlarm=next;navigate(page);}
}
void Dashboard::footer(){
 bool warning=toast.length()&&int32_t(toastUntil-millis())>0;String key=String(activeAlarm)+String(warning)+String(wifiConnected())+String(haLive())+String(haRecovering())+name(haSelected())+ui.temperature(fresh(probeData(),millis())?probeData().ambient:NAN);
 if(!force&&key==footerKey)return;footerKey=key;
 if(page!=0&&activeAlarm<0&&!warning)return;
 int y=page==0?378:404,h=page==0?48:28;widgets.region(0,y,480,h);
 if(activeAlarm>=0||warning){widgets.roundRect(8,0,464,h-2,6,Theme::Panel);widgets.symbol(27,h/2,4,Theme::Orange);widgets.fitted(49,7,348,warning?toast:name(activeAlarm)+" - ZIEL ERREICHT",SmallFont,Theme::Orange);widgets.text(432,7,"OK",SmallFont,Theme::White);}
 else {widgets.line(12,0,468,0,1,Theme::Track);widgets.icon(25,18,0,wifiConnected()?Theme::White:Theme::Red);widgets.icon(62,18,1,haRecovering()?Theme::Orange:haLive()?Theme::Blue:Theme::Red);widgets.fitted(88,16,220,name(haSelected()),LabelFont,Theme::White);widgets.symbol(322,29,0,Theme::Orange);widgets.fitted(345,12,123,ui.temperature(fresh(probeData(),millis())?probeData().ambient:NAN),ValueFont,Theme::Orange);}
 widgets.present();
}
void Dashboard::handle(const TouchEvent&e){
 if(e.kind==TouchEvent::None)return;lastInteraction=millis();if(dimmed){dimmed=false;widgets.brightness(ui.brightness);navigate(page);return;}
 if(e.kind==TouchEvent::Left||e.kind==TouchEvent::Right){if(page==0||page==2||page==3){haSelect((haSelected()+(e.kind==TouchEvent::Left?1:4))%5);arc=0;navigate(page);}return;}
 if(e.kind!=TouchEvent::Tap)return;
 if(e.y>=432){int pages[]={0,1,2,4};navigate(pages[std::min(3,e.x/120)]);return;}
 if((activeAlarm>=0||toast.length())&&e.y>=(page==0?378:404)){if(activeAlarm>=0)pending[activeAlarm]=false;toast="";navigate(page);return;}
 if(page&&e.y<48){navigate(page==5?backPage:page==6?4:0);return;}
 if(page==0){if(e.x>=426&&e.y<96)navigate(4);else if(e.x<180&&e.y>=156&&e.y<228)openTarget();else if(e.y>=378&&e.x>=310){plotMode=1;navigate(1);}else if(e.y>=96)navigate(3);}
 else if(page==1&&e.y>=48&&e.y<96){if(e.x>=346)plotWindow=plotWindow==30?60:plotWindow==60?120:30;else plotMode=e.x<110?0:e.x<224?1:2;force=true;}
 else if(page==2&&e.y>=60&&e.y<410){haSelect(std::min(4,(e.y-60)/70));arc=0;navigate(3);}
 else if((page==3||page==4)&&e.y>=58&&e.y<401){int r=(e.y-58)/49;unsigned s=haSelected();
 if(page==3){if(r==0){ui.meat[s]=(ui.meat[s]+1)%4;save();}else if(r==2)openTarget();else if(r==6){ui.alarms[s]=!ui.alarms[s];save();}}
 else {if(r==0)ui.dimAfter=(ui.dimAfter+1)%4;else if(r==1){if(e.x<260)return;ui.brightness=std::max(20,std::min(100,20+(e.x-272)*80/130));}else if(r==2)ui.amoled=!ui.amoled;else if(r==3)ui.alarms[s]=!ui.alarms[s];else if(r==4)ui.fahrenheit=!ui.fahrenheit;else {navigate(6);return;}save();Theme::setAmoled(ui.amoled);widgets.brightness(ui.brightness);navigate(4);}}
 else if(page==5){if(e.y>=252&&e.y<312){float step=ui.fahrenheit?1/1.8f:1;if(e.x<144)editing=fmaxf(1,editing-step);else if(e.x>=336)editing=fminf(150,editing+step);}else if(e.y>=332&&e.y<380){ui.setTarget(haSelected(),editing);save();navigate(backPage);}else if(e.y>=390){ui.setTarget(haSelected(),NAN);save();navigate(backPage);}}
}
void Dashboard::update(){
 if(!ready)return;handle(touch.update());uint32_t now=millis();
 // Sensor values arrive every 10 s. Keep touch responsive without rebuilding
 // every page/cache key 30 times per second and competing with HTTP for heap/CPU.
 if(!force&&now-last<200)return;last=now;
 if(!ntpStarted&&wifiConnected()){configTzTime("CET-1CEST,M3.5.0,M10.5.0/3","pool.ntp.org","time.nist.gov");ntpStarted=true;}
 if(shown!=haSelected()){shown=haSelected();arc=0;navigate(page);}
 alarms();bool shouldDim=ui.dimMs()&&uint32_t(now-lastInteraction)>=ui.dimMs();if(shouldDim!=dimmed){dimmed=shouldDim;widgets.brightness(dimmed?10:ui.brightness);navigate(page);}
 if(toast.length()&&int32_t(now-toastUntil)>=0){toast="";navigate(page);}
 bool changed=false;for(unsigned i=0;i<5;++i){const auto &d=haProbe(i);bool valid=fresh(d,now)&&haLive();bool a=history[i].sample(now,valid?d.core:NAN),b=ambient[i].sample(now,valid?d.ambient:NAN);if(i==haSelected())changed=a||b;}
 header();switch(page){case 0:home();break;case 1:chart(changed);break;case 2:probes();break;case 3:details();break;case 4:settings();break;case 5:targetPage();break;case 6:info();break;}footer();force=false;
}
