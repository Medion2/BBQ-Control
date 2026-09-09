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
#ifndef BBQ_HA_TOKEN
#define BBQ_HA_TOKEN ""
#endif
#ifndef BBQ_HA_PROBE
#define BBQ_HA_PROBE "meater_1"
#endif
#ifdef BBQ_NETWORK_COMPILE_TEST
#undef BBQ_WIFI_SSID
#undef BBQ_HA_TOKEN
#define BBQ_WIFI_SSID "compile-only"
#define BBQ_HA_TOKEN "compile-only"
#endif
namespace Config {
constexpr char WiFiSsid[] = BBQ_WIFI_SSID;
constexpr char WiFiPassword[] = BBQ_WIFI_PASSWORD;
constexpr uint32_t WiFiTimeoutMs = 15000, WiFiRetryMs = 30000;
constexpr uint16_t Background = 0x0841, White = 0xFFFF, Muted = 0x94B2;
constexpr uint16_t Accent = 0xFD20, Good = 0x4EAA, Panel = 0x18E3;
constexpr uint32_t StaleMs = 90000;
constexpr char HaUrl[] = "http://192.168.178.200:8123";
constexpr char HaToken[] = BBQ_HA_TOKEN;
constexpr char HaProbe[] = BBQ_HA_PROBE;
constexpr size_t MaxPayload = 1024;
}
