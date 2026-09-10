#include "TouchManager.h"
#include <Wire.h>
bool TouchManager::read(uint16_t reg,uint8_t*d,int n){Wire.beginTransmission(address);Wire.write(uint8_t(reg>>8));Wire.write(uint8_t(reg));if(Wire.endTransmission(false))return false;if(Wire.requestFrom(int(address),n)!=n)return false;for(int i=0;i<n;++i)d[i]=Wire.read();return true;}
void TouchManager::begin(){uint8_t id[4];if(!read(0x8140,id,4))address=0x14;}
TouchEvent TouchManager::update(){
 TouchEvent event;uint32_t now=millis();if(now-last<15)return event;last=now;
 if(down&&now-report>300)down=false; // stale contact is cancelled, never a tap
 uint8_t status;if(!read(0x814e,&status,1)||!(status&128))return event;
 int count=status&15;uint8_t p[4];bool good=count==0||(count==1&&read(0x8150,p,4));
 Wire.beginTransmission(address);Wire.write(0x81);Wire.write(0x4e);Wire.write(0);good=Wire.endTransmission()==0&&good;
 if(!good){down=false;return event;}report=now;
 if(count){x=p[0]|(p[1]<<8);y=p[2]|(p[3]<<8);if(x>=480||y>=480){down=false;return event;}if(!down){sx=x;sy=y;start=now;}down=true;return event;}
 if(!down)return event;down=false;event.x=x;event.y=y;
 int dx=x-sx,dy=y-sy;
 if(now-start<1200&&abs(dx)>70&&abs(dy)<80)event.kind=dx<0?TouchEvent::Left:TouchEvent::Right;
 else if(now-start<700&&abs(dx)<15&&abs(dy)<15)event.kind=TouchEvent::Tap;
 return event;
}
