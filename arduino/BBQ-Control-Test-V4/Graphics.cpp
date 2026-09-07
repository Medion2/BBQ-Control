#include "Graphics.h"
#include "Config.h"
#include "Display.h"
#include "Touch.h"
#include "WiFiManager.h"
#include "Clock.h"
#include <math.h>
namespace {
enum Page { DisplayTest, Hardware, TouchTest, GraphicTest, Network, DigitalClock, AnalogClock, PageCount };
Page page = DisplayTest;
uint32_t bootAt, frameAt, fpsAt, frames = 0, metricsAt = 0;
float fps = 0;
bool booting = true, dirty = true;
int ballX = 80, ballY = 140, dx = 4, dy = 3;
const char *titles[] = {"Displaytest", "Hardware", "Touchtest", "Grafiktest", "WLAN / NTP", "Digitale Uhr", "Analoge Uhr"};
void text(int x, int y, const String &s, uint8_t size = 2, uint16_t color = Config::White) {
  gfx->setTextSize(size); gfx->setTextColor(color); gfx->setCursor(x,y); gfx->print(s);
}
void shell() {
  gfx->fillScreen(Config::Background);
  text(16, 12, titles[page], 3);
  gfx->drawFastHLine(0, 50, 480, Config::Accent);
  gfx->fillRect(0, 438, 238, 42, 0x2104);
  gfx->fillRect(242, 438, 238, 42, 0x2104);
  text(55, 452, "< Zurueck"); text(295, 452, "Weiter >");
}
void displayTest() {
  const uint16_t colors[] = {0xF800,0x07E0,0x001F,0xFFFF,0x0000,0xFFE0,0xF81F,0x07FF};
  const char *labels[] = {"R","G","B","W","K","Y","M","C"};
  for (int i=0; i<8; ++i) {
    gfx->fillRect(i*60, 72, 60, 135, colors[i]);
    text(i*60+22, 216, labels[i]);
  }
  for (int i=0; i<32; ++i) {
    uint16_t c = (i<<11) | ((i*2)<<5) | i;
    gfx->fillRect(i*15, 250, 15, 55, c);
  }
  gfx->drawRect(1, 53, 478, 340, Config::White);
  text(22, 324, "480 x 480 / RGB565");
  text(22, 355, "Farben, Graustufen, Rand");
}
void hardware() {
  gfx->fillRect(10, 65, 460, 325, Config::Background);
  text(16,70,"Chip: " + String(ESP.getChipModel()));
  text(16,100,"Revision: " + String(ESP.getChipRevision()) + " / Kerne: " + String(ESP.getChipCores()));
  text(16,130,"CPU: " + String(ESP.getCpuFreqMHz()) + " MHz");
  text(16,160,"Flash: " + String(ESP.getFlashChipSize()/1024/1024) + " MiB");
  text(16,190,"Heap frei: " + String(ESP.getFreeHeap()/1024) + " KiB");
  text(16,220,"Heap Minimum: " + String(ESP.getMinFreeHeap()/1024) + " KiB");
  text(16,250,"PSRAM: " + String(ESP.getPsramSize()/1024) + " KiB");
  text(16,280,"PSRAM frei: " + String(ESP.getFreePsram()/1024) + " KiB");
  text(16,310,"SDK: " + String(ESP.getSdkVersion()), 1);
  text(16,338,"Laufzeit: " + String(millis()/1000) + " s");
  text(16,368,"Panel: 480x480 / GT911");
}
void touchBackground() {
  text(16,68,"Beruehren und Linien zeichnen",2);
  for (int x=20; x<480; x+=55) gfx->drawFastVLine(x,120,225,0x3186);
  for (int y=120; y<=340; y+=55) gfx->drawFastHLine(20,y,440,0x3186);
}
void touchDraw() {
  const auto &t=touchState();
  if (t.down && t.y>=115 && t.y<345) gfx->fillCircle(t.x,t.y,4,Config::Accent);
  gfx->fillRect(10,352,460,42,Config::Background);
  text(16,355,"X:"+String(t.x)+" Y:"+String(t.y)+" Punkte:"+String(t.points));
  text(16,378,String(t.down ? "DOWN" : "UP")+" / I2C-Fehler: "+String(t.errors),1);
}
void network() {
  gfx->fillRect(10,65,460,330,Config::Background);
  text(16,80,wifiStatusText());
  text(16,120,"SSID: "+wifiSsid().substring(0,30),1);
  text(16,150,"IP: "+wifiAddress());
  text(16,190,wifiConnected() ? "RSSI: "+String(wifiRssi())+" dBm" : "RSSI: --");
  text(16,235,clockStatusText());
  text(16,280,"Zeitzone: Europa/Berlin",2);
  text(16,315,"WLAN in Config.h eintragen.",2);
  text(16,350,"Automatische Wiederverbindung",2);
  text(16,380,"NTP-Abgleich stuendlich",1);
}
void hand(float angle, int length, uint16_t color) {
  gfx->drawLine(240,220,240+int(sinf(angle)*length),220-int(cosf(angle)*length),color);
}
void clockFace(bool analog) {
  gfx->fillRect(0,55,480,340,Config::Background);
  tm local = {};
  if (!clockLocalTime(local)) {
    text(55,185,"Warte auf NTP-Zeit",3);
    text(25,250,clockStatusText(),2);
    return;
  }
  char clockBuffer[16], dateBuffer[24];
  strftime(clockBuffer,sizeof(clockBuffer),"%H:%M:%S",&local);
  strftime(dateBuffer,sizeof(dateBuffer),"%d.%m.%Y",&local);
  if (analog) {
    constexpr float tau=6.28318530718f;
    gfx->drawCircle(240,220,135,Config::White);
    for(int i=0;i<60;++i) {
      float a=i*tau/60;
      int inner=i%5==0 ? 119 : 128;
      gfx->drawLine(240+int(sinf(a)*inner),220-int(cosf(a)*inner),
                    240+int(sinf(a)*133),220-int(cosf(a)*133),Config::White);
    }
    text(228,105,"12",2); text(344,212,"3",2);
    text(234,317,"6",2); text(127,212,"9",2);
    hand(((local.tm_hour%12)+local.tm_min/60.0f)*tau/12,72,Config::White);
    hand((local.tm_min+local.tm_sec/60.0f)*tau/60,104,Config::Accent);
    hand(local.tm_sec*tau/60,115,0xF800);
    gfx->fillCircle(240,220,5,Config::White);
    text(180,361,dateBuffer,2);
  } else {
    text(72,155,clockBuffer,7,Config::Accent);
    text(150,245,dateBuffer,3);
    text(95,295,local.tm_isdst>0 ? "Sommerzeit / CEST" : "Winterzeit / CET",2);
  }
  text(16,383,clockStatusText(),1);
}
void graphicBackground() {
  gfx->drawRect(20,70,440,300,Config::Accent);
  gfx->drawTriangle(240,95,190,180,290,180,0xFFE0);
  gfx->drawCircle(365,140,45,0xF81F);
  gfx->drawLine(40,330,440,220,0x07E0);
  text(30,380,"Linien, Kreis, Dreieck, Animation",1);
}
void animate() {
  gfx->fillCircle(ballX,ballY,13,Config::Background);
  ballX+=dx; ballY+=dy;
  if(ballX<35 || ballX>445) dx=-dx;
  if(ballY<85 || ballY>355) dy=-dy;
  // Restore primitives after erasing the moving ball.
  graphicBackground();
  gfx->fillCircle(ballX,ballY,12,0xF800);
}
}
void graphicsBegin() {
  bootAt = fpsAt = millis();
  gfx->setTextWrap(false);
  gfx->fillScreen(Config::Background);
  gfx->drawRoundRect(90,95,300,270,20,Config::Accent);
  for(int x=135;x<=345;x+=35) gfx->drawLine(x,145,x+15,205,0xF800);
  gfx->drawRoundRect(125,215,230,70,15,Config::White);
  for(int y=230;y<280;y+=15) gfx->drawFastHLine(140,y,200,0xFFE0);
  text(114,310,"BBQ CONTROL",3);
  text(120,390,"HARDWARE TEST V4",2,Config::Accent);
}
void graphicsUpdate() {
  uint32_t now=millis();
  if(booting) { if(now-bootAt<Config::BootMs) return; booting=false; dirty=true; fpsAt=now; }
  const auto &t=touchState();
  if(t.pressed && t.y>=438) {
    page=Page((int(page)+(t.x<240 ? PageCount-1 : 1))%PageCount); dirty=true;
  }
  if(Serial.available()) { char c=Serial.read(); if(c=='n' || c=='p') { page=Page((int(page)+(c=='n'?1:PageCount-1))%PageCount); dirty=true; } }
  if(now-frameAt<Config::FrameMs) return;
  frameAt=now;
  if(dirty) {
    shell();
    if(page==DisplayTest) displayTest();
    if(page==Hardware) hardware();
    if(page==TouchTest) touchBackground();
    if(page==GraphicTest) graphicBackground();
    if(page==Network) network();
    if(page==DigitalClock || page==AnalogClock) clockFace(page==AnalogClock);
    dirty=false;
  }
  if(page==TouchTest) touchDraw();
  if(page==GraphicTest) animate();
  ++frames;
  if(now-fpsAt>=1000) { fps=frames*1000.0f/(now-fpsAt); frames=0; fpsAt=now; }
  if(now-metricsAt>=1000) {
    metricsAt=now;
    if(page==Hardware) hardware();
    if(page==Network) network();
    if(page==DigitalClock || page==AnalogClock) clockFace(page==AnalogClock);
    gfx->fillRect(0,400,480,35,Config::Background);
    text(12,405,"UI FPS: "+String(fps,1)+" Heap: "+String(ESP.getFreeHeap()/1024)+"K",2,Config::Accent);
    text(12,424,"PSRAM frei: "+String(ESP.getFreePsram()/1024)+" KiB",1);
  }
}
