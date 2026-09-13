#include "UiState.h"
#include "Config.h"
#include <Preferences.h>
void UiState::begin(){
 Preferences n;if(!n.begin("bbq-ui",true))return;
 fahrenheit=n.getBool("fahrenheit",false);amoled=n.getBool("amoled",false);
 brightness=constrain(n.getUChar("brightness",100),20,100);dimAfter=constrain(n.getUChar("dimAfter",0),0,3);
 for(unsigned i=0;i<5;++i){String k=String(i);float t=n.getFloat(("target"+k).c_str(),NAN);
  targets[i]=isfinite(t)&&t>=1&&t<=150?t:NAN;alarms[i]=n.getBool(("alarm"+k).c_str(),true);meat[i]=constrain(n.getUChar(("meat"+k).c_str(),0),0,3);}
 n.end();
}
bool UiState::save(){
 Preferences n;if(!n.begin("bbq-ui",false))return false;bool ok=true;
 ok&=n.putBool("fahrenheit",fahrenheit)>0;ok&=n.putBool("amoled",amoled)>0;
 ok&=n.putUChar("brightness",brightness)>0;ok&=n.putUChar("dimAfter",dimAfter)>0;
 for(unsigned i=0;i<5;++i){String k=String(i);ok&=n.putFloat(("target"+k).c_str(),targets[i])>0;
  ok&=n.putBool(("alarm"+k).c_str(),alarms[i])>0;ok&=n.putUChar(("meat"+k).c_str(),meat[i])>0;}
 n.end();return ok;
}
const char *UiState::meatName(unsigned i)const{const char *names[]={"Rind","Schwein","Gefluegel","Fisch"};return names[meat[i]%4];}
String UiState::grade(unsigned i,float core,bool valid)const{
 if(!valid)return "KEINE MESSUNG";
 if(meat[i]!=0)return "MANUELLES ZIEL";
 return core<Config::RareMin?"AUFHEIZEN":core<Config::MediumRareMin?"RARE":core<Config::MediumMin?"MEDIUM RARE":core<Config::MediumWellMin?"MEDIUM":core<Config::WellDoneMin?"MEDIUM WELL":"WELL DONE";
}
