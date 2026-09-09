#include "HomeAssistant.h"
#include "Config.h"
#include "ProbeData.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <atomic>
namespace {
QueueHandle_t inbox=nullptr;
std::atomic<int> result{0};
// Render only the chosen probe's states on HA; never download all HA entities.
String requestBody() {
  String t="{% set p = 'sensor.";
  t+=Config::HaProbe;
  t+="_' %}{% set c = states(p ~ 'innentemperatur') | float(none) %}"
     "{% set a = states(p ~ 'umgebungstemperatur') | float(none) %}"
     "{{ {'schema':1, 'probe_online':c is number and a is number,"
     "'core_c':c, 'ambient_c':a,"
     "'target_c':states(p ~ 'soll_temperatur') | float(none),"
     "'cook_state':states(p ~ 'kochstatus') if has_value(p ~ 'kochstatus') else none,"
     "'battery_pct':none} | to_json }}";
  cJSON *root=cJSON_CreateObject();
  if(!root) return "";
  cJSON_AddStringToObject(root,"template",t.c_str());
  char *json=cJSON_PrintUnformatted(root);
  String body=json?json:"";
  cJSON_free(json); cJSON_Delete(root); return body;
}
void worker(void *) {
  const String body=requestBody();
  for(;;) {
    if(WiFi.status()==WL_CONNECTED && body.length()) {
      WiFiClient socket; HTTPClient http;
      http.setConnectTimeout(3000); http.setTimeout(3000);
      http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
      ProbeData next;
      int code=-1;
      if(http.begin(socket,String(Config::HaUrl)+"/api/template")) {
        http.addHeader("Authorization",String("Bearer ")+Config::HaToken);
        http.addHeader("Content-Type","application/json");
        code=http.POST(body);
        if(code==200) {
          String payload=http.getString();
          if(!probeParse(payload.c_str(),payload.length(),millis(),next)) code=-2;
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
  if(!Config::HaToken[0]) return;
  inbox=xQueueCreate(1,sizeof(ProbeData));
  if(!inbox) { result=-3; return; }
  if(xTaskCreate(worker,"ha-read",8192,nullptr,1,nullptr)!=pdPASS) result=-3;
}
void haUpdate() {
  ProbeData next;
  if(inbox && xQueueReceive(inbox,&next,0)==pdTRUE) probeSet(next);
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
