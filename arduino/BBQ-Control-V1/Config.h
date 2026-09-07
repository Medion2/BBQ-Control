#pragma once
#include <Arduino.h>
#if __has_include("Secrets.h")
#include "Secrets.h"
#endif
#ifndef BBQ_WIFI_SSID
#define BBQ_WIFI_SSID ""
#endif
#ifndef BBQ_WIFI_PASSWORD
#define BBQ_WIFI_PASSWORD ""
#endif
#ifndef BBQ_MQTT_HOST
#define BBQ_MQTT_HOST ""
#endif
#ifndef BBQ_MQTT_USER
#define BBQ_MQTT_USER ""
#endif
#ifndef BBQ_MQTT_PASSWORD
#define BBQ_MQTT_PASSWORD ""
#endif
// Compile-only configuration to check enabled network paths; never upload it.
#ifdef BBQ_NETWORK_COMPILE_TEST
#undef BBQ_WIFI_SSID
#undef BBQ_MQTT_HOST
#define BBQ_WIFI_SSID "compile-only"
#define BBQ_MQTT_HOST "192.0.2.1"
#endif
namespace Config {
constexpr char WiFiSsid[] = BBQ_WIFI_SSID;
constexpr char WiFiPassword[] = BBQ_WIFI_PASSWORD;
constexpr uint32_t WiFiTimeoutMs = 15000, WiFiRetryMs = 30000;
constexpr uint16_t Background = 0x0841, White = 0xFFFF, Muted = 0x94B2;
constexpr uint16_t Accent = 0xFD20, Good = 0x4EAA, Panel = 0x18E3;
constexpr uint32_t StaleMs = 90000;
constexpr char MqttHost[] = BBQ_MQTT_HOST;
constexpr uint16_t MqttPort = 1883;
constexpr char MqttUser[] = BBQ_MQTT_USER;
constexpr char MqttPassword[] = BBQ_MQTT_PASSWORD;
constexpr char StateTopic[] = "bbq/control/v1/meater/state";
constexpr char AvailabilityTopic[] = "bbq/control/v1/availability";
constexpr char HaStatusTopic[] = "homeassistant/status";
constexpr size_t MaxPayload = 1024;
}
