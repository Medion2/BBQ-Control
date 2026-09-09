#include "Widgets.h"
#include "Display.h"
#include <esp_heap_caps.h>
#include <math.h>
#include <algorithm>
bool Widgets::begin() {
 for(auto &b:buffers) b=(uint16_t*)heap_caps_malloc(480*240*2,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
 if(!buffers[0]) buffers[0]=(uint16_t*)malloc(480*240*2);
 return buffers[0]!=nullptr;
}
bool Widgets::region(int x,int y,int w,int h) {
 if(!buffers[0] || w<=0 || h<=0 || w*h>480*240) return false;
 rx=x;ry=y;width=w;height=h;pixels=buffers[active];
 std::fill(pixels,pixels+w*h,Theme::Background);return true;
}
void Widgets::present() {
 gfx->draw16bitRGBBitmap(rx,ry,pixels,width,height);
 if(buffers[1]) active^=1;
}
void Widgets::pixel(int x,int y,uint16_t c,float a) {
 if(x<0||y<0||x>=width||y>=height||a<=0)return;
 uint16_t &d=pixels[y*width+x];
 if(a>=1){d=c;return;}
 int r=((d>>11)&31)*(1-a)+((c>>11)&31)*a;
 int g=((d>>5)&63)*(1-a)+((c>>5)&63)*a;
 int b=(d&31)*(1-a)+(c&31)*a;d=(r<<11)|(g<<5)|b;
}
void Widgets::rect(int x,int y,int w,int h,uint16_t c){for(int j=y;j<y+h;++j)for(int i=x;i<x+w;++i)pixel(i,j,c);}
void Widgets::roundRect(int x,int y,int w,int h,int r,uint16_t c){
 for(int j=y;j<y+h;++j)for(int i=x;i<x+w;++i){
 float dx=std::max(float(x+r-i),std::max(0.f,float(i-(x+w-r-1))));
 float dy=std::max(float(y+r-j),std::max(0.f,float(j-(y+h-r-1))));
 pixel(i,j,c,std::min(1.f,float(r)+.5f-sqrtf(dx*dx+dy*dy)));
 }}
void Widgets::line(float x,float y,float x2,float y2,float t,uint16_t c){
 float dx=x2-x,dy=y2-y,d=dx*dx+dy*dy;
 for(int j=floorf(std::min(y,y2)-t);j<=ceilf(std::max(y,y2)+t);++j)
 for(int i=floorf(std::min(x,x2)-t);i<=ceilf(std::max(x,x2)+t);++i){
 float u=d?std::max(0.f,std::min(1.f,((i-x)*dx+(j-y)*dy)/d)):0;
 float a=i-x-u*dx,b=j-y-u*dy;pixel(i,j,c,std::min(1.f,t/2+.5f-sqrtf(a*a+b*b)));}}
void Widgets::circle(float x,float y,float r,float t,uint16_t c){
 for(int j=floorf(y-r-t);j<=ceilf(y+r+t);++j)for(int i=floorf(x-r-t);i<=ceilf(x+r+t);++i)
 pixel(i,j,c,std::min(1.f,t/2+.5f-fabsf(hypotf(i-x,j-y)-r)));}
int Widgets::textWidth(const String&s,const SmoothFont&f){int n=0;for(unsigned i=0;i<s.length();++i){unsigned c=(unsigned char)s[i];n+=f.glyphs[c>=32&&c<=126?c-32:'?'-32].advance;}return n;}
void Widgets::text(int x,int y,const String&s,const SmoothFont&f,uint16_t c){
 for(unsigned k=0;k<s.length();++k){unsigned ch=(unsigned char)s[k];const Glyph &g=f.glyphs[ch>=32&&ch<=126?ch-32:'?'-32];
 for(int j=0;j<g.h;++j)for(int i=0;i<g.w;++i){int n=j*g.w+i;uint8_t a=f.alpha[g.offset+n/2];a=n%2?a&15:a>>4;pixel(x+g.x+i,y+f.ascent+g.y+j,c,a/15.f);}x+=g.advance;}}
void Widgets::centered(int x,int y,const String&s,const SmoothFont&f,uint16_t c){text(x-textWidth(s,f)/2,y,s,f,c);}
void Widgets::icon(int x,int y,int type,uint16_t c){
 if(type==0){ // WiFi
  for(int r:{5,10,15})for(int a=220;a<=320;++a){float q=a*.0174533f;pixel(x+int(cosf(q)*r),y+14+int(sinf(q)*r),c);}circle(x,y+13,1,2,c);
 }else if(type==1){line(x-10,y+3,x,y-6,2,c);line(x,y-6,x+10,y+3,2,c);line(x-7,y+2,x-7,y+14,2,c);line(x+7,y+2,x+7,y+14,2,c);line(x-7,y+14,x+7,y+14,2,c);
 }else if(type==2){circle(x,y+4,8,2,c);line(x-9,y+14,x+9,y-5,2,c);
 }else if(type==3){circle(x,y+8,6,2,c);line(x,y+2,x+7,y-7,3,c);
 }else if(type==4){line(x-10,y-2,x+8,y-2,2,c);line(x-10,y+12,x+8,y+12,2,c);line(x-10,y-2,x-10,y+12,2,c);line(x+8,y-2,x+8,y+12,2,c);rect(x+9,y+2,3,6,c);
 }else if(type==5){circle(x,y+5,10,2,c);line(x,y+5,x,y-2,2,c);line(x,y+5,x+5,y+8,2,c);
 }else if(type==6){circle(x,y+5,9,2,c);circle(x,y+5,4,2,c);
 }else {line(x-6,y+13,x,y-6,3,c);line(x,y-6,x+6,y+13,3,c);line(x+6,y+13,x-6,y+13,3,c);}
}
