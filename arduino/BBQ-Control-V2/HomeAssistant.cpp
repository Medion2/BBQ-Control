#include "HomeAssistant.h"
#include "Config.h"
#include "ProbeData.h"
#include "HaTemplate.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <atomic>
namespace {
QueueHandle_t inbox=nullptr;
std::atomic<int> result{0};
struct Snapshot { ProbeData probes[5]; };
Snapshot latest;unsigned selected=0;
const char* prefixes[]={"meater_1","meater_2","meater_3","meater_4","meater_probe_0ae3b60f"};
String requestBody() {
 cJSON *root=cJSON_CreateObject();if(!root)return "";
 cJSON_AddStringToObject(root,"template",HaTemplate);
 char *json=cJSON_PrintUnformatted(root);String body=json?json:"";
 cJSON_free(json);cJSON_Delete(root);return body;
}
float optionalNumber(cJSON *r,const char *name,float max){
 cJSON *v=cJSON_GetObjectItemCaseSensitive(r,name);
 return cJSON_IsNumber(v)&&isfinite(v->valuedouble)&&v->valuedouble>=0&&v->valuedouble<=max?v->valuedouble:NAN;
}
bool parse(const String &payload,Snapshot &out){
 if(payload.length()>4096)return false;
 cJSON *root=cJSON_Parse(payload.c_str());bool valid=cJSON_IsArray(root)&&cJSON_GetArraySize(root)==5;
 for(int i=0;valid&&i<5;++i){
  cJSON *row=cJSON_GetArrayItem(root,i);char *json=cJSON_PrintUnformatted(row);
  valid=json&&probeParse(json,strlen(json),millis(),out.probes[i]);cJSON_free(json);
  if(valid){auto &d=out.probes[i];d.peak=optionalNumber(row,"peak_c",150);d.remaining=optionalNumber(row,"remaining_s",604800);d.elapsed=optionalNumber(row,"elapsed_s",604800);
   cJSON *v=cJSON_GetObjectItemCaseSensitive(row,"cooking");if(cJSON_IsString(v)){strlcpy(d.cooking,v->valuestring,sizeof(d.cooking));for(char &c:d.cooking)if((unsigned char)c>126||(c&&c<32))c='?';}
  }
 }
 cJSON_Delete(root);return valid;
}
void worker(void *) {
  const String body=requestBody();
  for(;;) {
    if(WiFi.status()==WL_CONNECTED && body.length()) {
      WiFiClient socket; HTTPClient http;
      http.setConnectTimeout(3000); http.setTimeout(3000);
      http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
      Snapshot next;
      int code=-1;
      if(http.begin(socket,String(Config::HaUrl)+"/api/template")) {
        http.addHeader("Authorization",String("Bearer ")+Config::HaToken);
        http.addHeader("Content-Type","application/json");
        code=http.POST(body);
        if(code==200) {
          String payload=http.getString();
          if(!parse(payload,next)) { code=-2; next=Snapshot{}; }
        }
        http.end();
      }
      xQueueOverwrite(inbox,&next);
      result=code;
    } else result=0;
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
}
void haBegin() {
  for(unsigned i=0;i<5;++i)if(!strcmp(Config::HaProbe,prefixes[i]))selected=i;
  if(!Config::HaToken[0]) return;
  inbox=xQueueCreate(1,sizeof(Snapshot));
  if(!inbox) { result=-3; return; }
  if(xTaskCreate(worker,"ha-read",8192,nullptr,1,nullptr)!=pdPASS) result=-3;
}
void haUpdate() {
  Snapshot next;
  if(inbox && xQueueReceive(inbox,&next,0)==pdTRUE) { latest=next; probeSet(latest.probes[selected]); }
}
bool haLive() { return WiFi.status()==WL_CONNECTED && result==200; }
String haStatusText() {
  if(!Config::HaToken[0]) return "Home Assistant: Token fehlt";
  if(WiFi.status()!=WL_CONNECTED) return "Home Assistant: WLAN offline";
  int code=result;
  if(code==200) return "Home Assistant: verbunden";
  if(code==401 || code==403) return "Home Assistant: Token abgelehnt";
  if(code==0) return "Home Assistant: verbinde";
  return "Home Assistant: Fehler "+String(code);
}

const ProbeData &haProbe(unsigned i){return latest.probes[i%5];}
unsigned haSelected(){return selected;}
void haSelect(unsigned i){selected=i%5;probeSet(latest.probes[selected]);}
String haProbeName(){return selected<4?"Meater "+String(selected+1):"Probe 0ae3b60f";}
