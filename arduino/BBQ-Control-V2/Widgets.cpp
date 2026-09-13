#include "Widgets.h"
#include "Display.h"
#include <esp_heap_caps.h>
#include <math.h>
#include <algorithm>
namespace Theme {
uint16_t Background=rgb(0x0F1115),Panel=rgb(0x161A1F);
void setAmoled(bool enabled){Background=enabled?0:rgb(0x0F1115);Panel=enabled?rgb(0x080B10):rgb(0x161A1F);}
}
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
 if(level<100)for(int i=0;i<width*height;++i){uint16_t c=pixels[i];pixels[i]=((((c>>11)&31)*level/100)<<11)|((((c>>5)&63)*level/100)<<5)|((c&31)*level/100);}

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
 }else { // Thermometer
  circle(x,y+10,5,2,c);line(x-3,y+6,x-3,y-7,2,c);line(x+3,y+6,x+3,y-7,2,c);
  line(x-3,y-7,x+3,y-7,2,c);line(x,y+9,x,y-3,2,c);
 }
}

void Widgets::fitted(int x,int y,int maxWidth,const String&s,const SmoothFont&f,uint16_t c){
 int tw=textWidth(s,f);if(tw<=maxWidth){text(x,y,s,f,c);return;}
 textScaled(x,y,s,f,c,float(maxWidth)/tw);
}
void Widgets::textScaled(int x,int y,const String&s,const SmoothFont&f,uint16_t c,float scale){
 if(scale<=0)return;float pen=x;
 for(unsigned k=0;k<s.length();++k){unsigned ch=(unsigned char)s[k];const Glyph &g=f.glyphs[ch>=32&&ch<=126?ch-32:'?'-32];
  auto alpha=[&](int i,int j)->float{if(i<0||j<0||i>=g.w||j>=g.h)return 0;int n=j*g.w+i;uint8_t a=f.alpha[g.offset+n/2];return (n%2?a&15:a>>4)/15.f;};
  for(int j=0;j<ceilf(g.h*scale);++j)for(int i=0;i<ceilf(g.w*scale);++i){float u=(i+.5f)/scale-.5f,v=(j+.5f)/scale-.5f;int a=floorf(u),b=floorf(v);float dx=u-a,dy=v-b;
   float opacity=alpha(a,b)*(1-dx)*(1-dy)+alpha(a+1,b)*dx*(1-dy)+alpha(a,b+1)*(1-dx)*dy+alpha(a+1,b+1)*dx*dy;
   pixel(lroundf(pen+g.x*scale)+i,lroundf(y+(f.ascent+g.y)*scale)+j,c,opacity);}
  pen+=g.advance*scale;
 }
}
void Widgets::symbol(int x,int y,int type,uint16_t c){
 if(type==0){ // flame
  line(x-8,y+8,x-10,y,2,c);line(x-10,y,x-3,y-10,2,c);line(x-3,y-10,x-2,y,2,c);line(x-2,y,x+5,y-13,2,c);
  line(x+5,y-13,x+11,y+2,2,c);line(x+11,y+2,x+7,y+11,2,c);line(x+7,y+11,x-5,y+11,2,c);line(x-5,y+11,x,y+1,2,c);
 }else if(type==1){ // steak outline
  line(x-13,y+6,x-10,y-4,2,c);line(x-10,y-4,x+1,y-10,2,c);line(x+1,y-10,x+11,y-6,2,c);
  line(x+11,y-6,x+14,y+2,2,c);line(x+14,y+2,x+7,y+10,2,c);line(x+7,y+10,x-5,y+12,2,c);line(x-5,y+12,x-13,y+6,2,c);
  line(x-7,y+4,x+7,y-2,2,c);line(x-4,y-1,x+1,y-4,2,c);
 }else if(type==2){line(x-11,y-10,x-11,y+11,2,c);line(x-11,y+11,x+13,y+11,2,c);
  line(x-7,y+5,x-1,y-2,2,c);line(x-1,y-2,x+5,y+1,2,c);line(x+5,y+1,x+12,y-9,2,c);
 }else if(type==3){circle(x,y,8,2,c);circle(x,y,3,2,c);for(int a=0;a<8;++a){float q=a*.785398f;line(x+cosf(q)*9,y+sinf(q)*9,x+cosf(q)*13,y+sinf(q)*13,3,c);}
 }else if(type==4){line(x-9,y+7,x-7,y-6,2,c);line(x-7,y-6,x,y-10,2,c);line(x,y-10,x+7,y-6,2,c);line(x+7,y-6,x+9,y+7,2,c);line(x-11,y+8,x+11,y+8,2,c);circle(x,y+11,2,2,c);
 }else if(type==5){for(int i=-1;i<=1;++i){line(x+i*9,y-10,x+i*9,y+10,2,c);circle(x+i*9,y-9,2,2,c);}}
 else if(type==7){circle(x,y,5,2,c);for(int a=0;a<8;++a){float q=a*.785398f;line(x+cosf(q)*9,y+sinf(q)*9,x+cosf(q)*12,y+sinf(q)*12,2,c);}}
 else if(type==8){line(x-12,y-9,x+12,y-9,2,c);line(x+12,y-9,x+12,y+8,2,c);line(x+12,y+8,x-12,y+8,2,c);line(x-12,y+8,x-12,y-9,2,c);line(x-5,y+12,x+5,y+12,2,c);}
 else if(type==9){circle(x,y,10,2,c);text(x-2,y-11,"i",LabelFont,c);}
 else {for(int j=-1;j<=1;++j)line(x-10,y+j*6,x+10,y+j*6,2,c);}
}
