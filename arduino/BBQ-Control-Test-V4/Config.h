#pragma once
#include <Arduino.h>
namespace Config {
// Set locally before upload. Do not commit real credentials.
constexpr char WiFiSsid[] = "";
constexpr char WiFiPassword[] = "";
constexpr char TimeZone[] = "CET-1CEST,M3.5.0,M10.5.0/3";
constexpr char NtpServer1[] = "pool.ntp.org";
constexpr char NtpServer2[] = "time.nist.gov";
constexpr uint32_t WiFiTimeoutMs = 15000, WiFiRetryMs = 30000;

constexpr uint16_t Width = 480, Height = 480;
constexpr uint32_t FrameMs = 33, BootMs = 1800;
constexpr uint8_t TouchAddress = 0x5D;
constexpr uint16_t Background = 0x0841, White = 0xFFFF, Accent = 0x07FF;
}
