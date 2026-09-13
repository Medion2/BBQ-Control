#include "SettingsPage.h"
#include <algorithm>
void SettingsPage::row(Widgets&w,int i,const String&label,const String&value,uint16_t color,bool toggle,bool on,bool settings){
 w.region(12,58+i*49,456,45);w.roundRect(0,0,456,45,6,Theme::Panel);if(settings){int icons[]={8,7,3,4,7,0,9};if(i==4||i==5)w.icon(23,15,icons[i],Theme::Grey);else w.symbol(23,22,icons[i],Theme::Grey);}w.text(settings?46:13,14,label,SmallFont,Theme::White);
 if(toggle){w.roundRect(381,10,56,26,13,on?Theme::Orange:Theme::Track);w.circle(on?423:395,23,9,18,Theme::White);}
 else {int width=std::min(220,w.textWidth(value,LabelFont));w.fitted(441-width,11,width,value,LabelFont,color);}w.present();
}
