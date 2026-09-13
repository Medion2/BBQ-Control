#pragma once
#include <Arduino.h>
#include <math.h>
// All temperatures stored in Celsius; units affect presentation only.
class UiState {
 float targets[5]={NAN,NAN,NAN,NAN,NAN};
 public:
 bool fahrenheit=false,amoled=false,alarms[5]={true,true,true,true,true};
 uint8_t brightness=100,dimAfter=0,meat[5]={};
 void begin();bool save();
 float target(unsigned i,float remote)const{return isfinite(targets[i])?targets[i]:remote;}
 bool local(unsigned i)const{return isfinite(targets[i]);}
 void setTarget(unsigned i,float value){targets[i]=value;}
 float display(float c)const{return fahrenheit?c*1.8f+32:c;}
 String temperature(float c,int decimals=1)const{return isfinite(c)?String(display(c),decimals)+ (fahrenheit?" F":" C"):"--";}
 const char *meatName(unsigned i)const;
 String grade(unsigned i,float core,bool valid)const;
 uint32_t dimMs()const{return dimAfter==0?0:dimAfter==1?30000:dimAfter==2?60000:120000;}
};
