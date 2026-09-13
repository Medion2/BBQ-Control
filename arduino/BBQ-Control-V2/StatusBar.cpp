#include "StatusBar.h"
void StatusBar::navigation(Widgets&w,int selected){
 w.region(0,432,480,48);
 const char *labels[]={"START","VERLAUF","SONDEN","MENUE"};
 for(int i=0;i<4;++i){int x=i*120;if(i==selected){w.roundRect(x+4,1,112,46,5,Theme::Panel);w.rect(x+35,0,50,2,Theme::Orange);}uint16_t c=i==selected?Theme::Orange:Theme::Grey;
 if(!i)w.icon(x+60,10,1,c);else w.symbol(x+60,17,i==1?2:i==2?1:3,c);w.centered(x+60,30,labels[i],SmallFont,c);}
 w.present();
}
