#include "StatusBar.h"
void StatusBar::draw(Widgets&w,bool wifi,bool ha,bool probe,bool battery){
 if(!w.region(0,448,480,32))return;
 bool state[]={wifi,ha,false,probe,battery};
 for(int i=0;i<5;++i)w.icon(80+i*80,8,i,i==2?Theme::Grey:i==4&&!battery?Theme::Orange:state[i]?Theme::Green:Theme::Red);
 w.present();
}
