#include "MqttManager.h"
#include "Config.h"
#include "WiFiManager.h"
#include "ProbeData.h"
#include "mqtt_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <atomic>
#include <cstring>
namespace {
esp_mqtt_client_handle_t client = nullptr;
QueueHandle_t inbox = nullptr;
struct Message { uint32_t generation; size_t length; char data[Config::MaxPayload+1]; };
std::atomic<bool> connected{false}, invalidate{false}, failed{false};
std::atomic<int> haState{0}; // 0 unknown, 1 online, -1 offline
std::atomic<uint32_t> generation{0};
uint32_t attemptedAt = 0, rejected = 0;
char clientId[40];
// Assembly state belongs exclusively to the MQTT callback task.
Message pending = {};
size_t used = 0;
bool assembling = false;
bool topicIs(esp_mqtt_event_handle_t e, const char *topic) {
  return e->topic && e->topic_len == int(strlen(topic)) && !memcmp(e->topic,topic,e->topic_len);
}
void event(void *, esp_event_base_t, int32_t id, void *eventData) {
  auto e=static_cast<esp_mqtt_event_handle_t>(eventData);
  if(id==MQTT_EVENT_CONNECTED) {
    generation.fetch_add(1); invalidate=true; haState=0;
    connected=true; failed=false; assembling=false;
    esp_mqtt_client_subscribe(client,Config::StateTopic,1);
    esp_mqtt_client_subscribe(client,Config::HaStatusTopic,1);
    esp_mqtt_client_enqueue(client,Config::AvailabilityTopic,"online",0,1,1,true);
  } else if(id==MQTT_EVENT_DISCONNECTED) {
    connected=false; haState=0; generation.fetch_add(1); invalidate=true; assembling=false;
  } else if(id==MQTT_EVENT_ERROR) {
    failed=true;
  } else if(id==MQTT_EVENT_DATA) {
    if(e->current_data_offset==0) {
      assembling=false; used=0;
      if(topicIs(e,Config::HaStatusTopic)) {
        if(e->data_len==e->total_data_len && e->data_len==6 && !memcmp(e->data,"online",6)) haState=1;
        if(e->data_len==e->total_data_len && e->data_len==7 && !memcmp(e->data,"offline",7)) {
          haState=-1; invalidate=true; generation.fetch_add(1);
        }
        return;
      }
      // Retained temperatures may be arbitrarily old. Wait for a live snapshot.
      if(!topicIs(e,Config::StateTopic) || e->retain || e->total_data_len<=0 ||
         e->total_data_len>int(Config::MaxPayload)) return;
      pending.length=e->total_data_len; pending.generation=generation.load(); assembling=true;
    }
    if(!assembling) return;
    if(e->data_len<=0 || e->current_data_offset!=int(used) ||
       e->total_data_len!=int(pending.length) || used+size_t(e->data_len)>pending.length) {
      assembling=false; return;
    }
    memcpy(pending.data+used,e->data,e->data_len); used+=e->data_len;
    if(used==pending.length) {
      pending.data[used]='\0';
      xQueueOverwrite(inbox,&pending); assembling=false;
    }
  }
}
void start() {
  esp_mqtt_client_config_t c = {};
  c.host=Config::MqttHost; c.port=Config::MqttPort;
  c.username=Config::MqttUser[0]?Config::MqttUser:nullptr;
  c.password=Config::MqttUser[0]?Config::MqttPassword:nullptr;
  c.client_id=clientId; c.transport=MQTT_TRANSPORT_OVER_TCP;
  c.keepalive=30; c.reconnect_timeout_ms=10000; c.network_timeout_ms=5000;
  c.buffer_size=512;
  c.lwt_topic=Config::AvailabilityTopic; c.lwt_msg="offline"; c.lwt_qos=1; c.lwt_retain=1;
  client=esp_mqtt_client_init(&c);
  if(!client) { failed=true; return; }
  if(esp_mqtt_client_register_event(client,MQTT_EVENT_ANY,event,nullptr)!=ESP_OK ||
     esp_mqtt_client_start(client)!=ESP_OK) {
    esp_mqtt_client_destroy(client); client=nullptr; failed=true;
  }
}
}
void mqttBegin() {
  if(!Config::MqttHost[0]) return;
  inbox=xQueueCreate(1,sizeof(Message));
  if(!inbox) { failed=true; return; }
  uint64_t mac=ESP.getEfuseMac();
  snprintf(clientId,sizeof(clientId),"bbq-v1-%04x%08lx",unsigned(mac>>32),static_cast<unsigned long>(mac));
  attemptedAt=millis()-10000;
}
void mqttUpdate() {
  if(!Config::MqttHost[0] || !inbox) return;
  if(!client && wifiConnected() && millis()-attemptedAt>=10000) { attemptedAt=millis(); start(); }
  if(invalidate.exchange(false)) probeSet(ProbeData{});
  Message message;
  if(xQueueReceive(inbox,&message,0)==pdTRUE && connected && message.generation==generation.load()) {
    ProbeData value;
    if(probeParse(message.data,message.length,millis(),value)) {
      // Recheck after parsing in case the callback reported a disconnect meanwhile.
      if(connected && message.generation==generation.load()) { probeSet(value); haState=1; }
    } else {
      ++rejected;
      Serial.printf("MQTT: ungueltiger Datensatz (%lu)\n",static_cast<unsigned long>(rejected));
    }
  }
}
bool mqttLive() { return connected && wifiConnected() && haState.load()!=-1; }
String mqttStatusText() {
  if(!Config::MqttHost[0]) return "MQTT nicht konfiguriert";
  if(connected && wifiConnected()) return "MQTT verbunden";
  return failed ? "MQTT Fehler / Retry" : "MQTT wartet";
}
String haStatusText() {
  if(!connected || !wifiConnected()) return "Home Assistant: nicht verbunden";
  int state=haState.load();
  return state==1 ? "Home Assistant: online" : state==-1 ? "Home Assistant: offline" : "Home Assistant: warte auf Status";
}
