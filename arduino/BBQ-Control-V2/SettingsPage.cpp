#include "SettingsPage.h"
void SettingsPage::draw(Widgets&w){
 w.region(0,40,480,200);w.centered(240,20,"Zieltemperatur",ValueFont,Theme::White);w.centered(240,65,"Lokales Anzeigeziel",LabelFont,Theme::Orange);w.centered(240,105,String(editing,0),HeroFont,Theme::White);w.present();
 w.region(0,240,480,208);
 w.roundRect(24,12,120,60,14,Theme::Panel);w.centered(84,24,"-",ValueFont,Theme::White);
 w.roundRect(336,12,120,60,14,Theme::Panel);w.centered(396,24,"+",ValueFont,Theme::White);
 w.roundRect(24,92,432,48,14,Theme::Orange);w.centered(240,102,"Lokal speichern",LabelFont,Theme::Background);
 w.centered(240,158,"HA-Ziel verwenden",LabelFont,Theme::Grey);w.present();
}
int SettingsPage::touch(TouchEvent e){
 if(e.kind!=TouchEvent::Tap)return 0;
 if(e.y<40)return 2;
 if(e.y>=252&&e.y<=312){if(e.x>=24&&e.x<=144)editing=fmaxf(1,editing-1);else if(e.x>=336&&e.x<=456)editing=fminf(150,editing+1);else return 0;return 1;}
 if(e.y>=332&&e.y<=380){targets[probe]=editing;return 2;}
 if(e.y>=394&&e.y<440){targets[probe]=NAN;return 2;}return 0;
}
