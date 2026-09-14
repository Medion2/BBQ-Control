#include "HomeAssistant.h"
#include "HaResponsePolicy.h"
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
constexpr int InvalidPayload=-1001, ClientInitFailed=-1002, WorkerUnavailable=-1003;
QueueHandle_t inbox=nullptr;
std::atomic<int> result{0};
struct Snapshot { ProbeData probes[5]; };
Snapshot latest;unsigned selected=0;bool haveSnapshot=false;uint32_t receivedAt=0;
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
  uint32_t failures=0;
  for(;;) {
    if(WiFi.status()==WL_CONNECTED && body.length()) {
      const uint32_t startedAt=millis();
      Snapshot next;
      int code=ClientInitFailed;
      for(unsigned attempt=0;attempt<2;++attempt) {
        WiFiClient socket; HTTPClient http;
        http.setConnectTimeout(6000); http.setTimeout(3000);
        http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
        code=ClientInitFailed;
        if(http.begin(socket,String(Config::HaUrl)+"/api/template")) {
          http.addHeader("Authorization",String("Bearer ")+Config::HaToken);
          http.addHeader("Content-Type","application/json");
          code=http.POST(body);
          if(code==200) {
            String payload=http.getString();
            if(!parse(payload,next)) { code=InvalidPayload; next=Snapshot{}; }
          }
          http.end();
        }
        socket.stop();
        if(code==200 && attempt>0)Serial.println("HA TCP retry succeeded");
        // -1 occurs before an HTTP request can be sent. Retry once, never in a tight loop.
        if(!HaResponsePolicy::retryConnection(code,attempt,WiFi.status()==WL_CONNECTED))break;
        Serial.println("HA TCP connection failed (-1), retrying once in 500 ms");
        vTaskDelay(pdMS_TO_TICKS(500));
      }
      // Only complete, validated responses replace the displayed measurements.
      // A failed request must not publish an empty Snapshot or refresh its age.
      if(code==200) {
        xQueueOverwrite(inbox,&next);
        if(failures)Serial.printf("HA recovered after %lu failed requests, duration=%lu ms\n",(unsigned long)failures,(unsigned long)(millis()-startedAt));
        failures=0;
      } else {
        ++failures;
        Serial.printf("HA request failed: code=%d, consecutive=%lu, duration=%lu ms, RSSI=%d, heap=%lu, minHeap=%lu\n",code,(unsigned long)failures,(unsigned long)(millis()-startedAt),WiFi.RSSI(),(unsigned long)ESP.getFreeHeap(),(unsigned long)ESP.getMinFreeHeap());
      }
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
  if(!inbox) { result=WorkerUnavailable; return; }
  if(xTaskCreate(worker,"ha-read",8192,nullptr,1,nullptr)!=pdPASS) result=WorkerUnavailable;
}
void haUpdate() {
  Snapshot next;
  if(inbox && xQueueReceive(inbox,&next,0)==pdTRUE) { latest=next; haveSnapshot=true; receivedAt=next.probes[0].receivedAt; probeSet(latest.probes[selected]); }
}
bool haDataAvailable() {
  const int code=result;
  return HaResponsePolicy::usable(haveSnapshot,WiFi.status()==WL_CONNECTED,code,millis(),receivedAt,Config::StaleMs);
}
bool haLive() { return haDataAvailable() && result==200; }
bool haRecovering() { return haDataAvailable() && result!=200; }
String haStatusText() {
  if(!Config::HaToken[0]) return "Home Assistant: Token fehlt";
  if(WiFi.status()!=WL_CONNECTED) return "Home Assistant: WLAN offline";
  int code=result;
  if(haLive()) return "Home Assistant: verbunden";
  if(haRecovering()) return "HA: Wiederholung ("+String(code)+")";
  if(code==200) return "HA: Daten veraltet";
  if(code==401 || code==403) return "Home Assistant: Token abgelehnt";
  if(code==0) return "Home Assistant: verbinde";
  return "Home Assistant: Fehler "+String(code);
}

const ProbeData &haProbe(unsigned i){return latest.probes[i%5];}
unsigned haSelected(){return selected;}
void haSelect(unsigned i){selected=i%5;probeSet(latest.probes[selected]);}
String haProbeName(){return selected<4?"Meater "+String(selected+1):"Probe 0ae3b60f";}
