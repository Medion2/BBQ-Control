#include "Graphics.h"
#include "Display.h"
#include "Config.h"
#include "ProbeData.h"
#include "WiFiManager.h"
#include "HomeAssistant.h"
namespace {
uint32_t lastDraw = 0;
void text(int x, int y, const String &value, uint8_t size, uint16_t color) {
  gfx->setTextSize(size); gfx->setTextColor(color); gfx->setCursor(x,y); gfx->print(value);
}
String temperature(float value, bool valid) {
  return valid && isfinite(value) ? String(value,1) : "--";
}
void degree(int x, int y, int r, uint16_t color) { gfx->drawCircle(x,y,r,color); }
void card(int x, const char *label, float value, bool valid) {
  gfx->fillRoundRect(x,256,214,83,12,Config::Panel);
  text(x+14,268,label,2,Config::Muted);
  text(x+14,298,temperature(value,valid),4,Config::White);
  degree(x+172,300,3,Config::Muted); text(x+181,300,"C",2,Config::Muted);
}
String status(const ProbeData &d, bool fresh) {
  if (!d.received) return "Warte auf Meater-Daten";
  if (!fresh) return d.probeOnline ? "Daten veraltet" : "Sonde nicht verbunden";
  String s(d.cookState);
  if(s=="started" || s=="cooking") return "Garen";
  if(s=="resting") return "Ruhephase";
  if(s=="finished") return "Fertig";
  if(s=="not_started" || s=="not started") return "Nicht gestartet";
  if(s=="ready") return "Bereit";
  return s.length() ? s : "Garstatus unbekannt";
}
void values() {
  const auto &d=probeData(); bool fresh=probeFresh(millis()) && haLive();
  gfx->fillRect(16,110,448,135,Config::Background);
  String value=temperature(d.core,fresh);
  uint8_t size=value.length()>5 ? 8 : 10;
  int width=value.length()*6*size;
  text((480-width)/2-15,125,value,size,fresh?Config::White:Config::Muted);
  degree((480+width)/2-2,131,6,Config::Accent);
  text((480+width)/2+10,132,"C",3,Config::Accent);
  text(18,225,fresh?"AKTUELLE KERNTEMPERATUR":"KEINE AKTUELLE MESSUNG",1,Config::Muted);
  card(16,"GARRAUM",d.ambient,fresh); card(250,"ZIEL",d.target,fresh);
  gfx->fillRoundRect(16,353,448,48,10,Config::Panel);
  text(30,369,status(d,fresh),2,fresh?Config::Good:Config::Accent);
  gfx->fillRect(16,414,448,62,Config::Background);
  gfx->drawRoundRect(18,420,32,16,3,Config::Muted); gfx->fillRect(50,425,3,6,Config::Muted);
  bool batteryValid=fresh && isfinite(d.battery);
  if(batteryValid) gfx->fillRect(21,423,int(26*d.battery/100),10,d.battery<20?0xF800:Config::Good);
  text(63,420,"MEATER "+(batteryValid?String(d.battery,0)+" %":String("--")),2,Config::Muted);
  text(18,452,wifiConnected()?"WLAN verbunden":"WLAN offline",1,Config::Muted);
  text(250,452,String(Config::HaProbe),1,Config::Muted);
  text(18,467,haStatusText(),1,Config::Muted);
}
}
void graphicsBegin() {
  gfx->setTextWrap(false); gfx->fillScreen(Config::Background);
  text(18,18,"BBQ CONTROL",3,Config::White); text(400,25,"1.0",2,Config::Accent);
  gfx->drawFastHLine(16,57,448,Config::Panel);
  text(18,80,"KERNTEMPERATUR",2,Config::Accent);
  values();
}
void graphicsUpdate() {
  if(millis()-lastDraw<1000) return;
  lastDraw=millis(); values();
}
