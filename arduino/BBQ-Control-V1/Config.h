#pragma once
#include <Arduino.h>
namespace Config {
constexpr char WiFiSsid[] = "";
constexpr char WiFiPassword[] = "";
constexpr uint32_t WiFiTimeoutMs = 15000, WiFiRetryMs = 30000;
constexpr uint16_t Background = 0x0841, White = 0xFFFF, Muted = 0x94B2;
constexpr uint16_t Accent = 0xFD20, Good = 0x4EAA, Panel = 0x18E3;
constexpr uint32_t StaleMs = 90000;
}
