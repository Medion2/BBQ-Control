#include "WiFiManager.h"
#include "Config.h"
#include <WiFi.h>
namespace {
uint32_t attemptedAt = 0;
bool connecting = false;
void connect() {
  attemptedAt = millis();
  connecting = true;
  WiFi.begin(Config::WiFiSsid, Config::WiFiPassword);
}
}
void wifiBegin() {
  if (!Config::WiFiSsid[0]) return;
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(false);
  connect();
}
void wifiUpdate() {
  if (!Config::WiFiSsid[0]) return;
  if (wifiConnected()) { connecting = false; return; }
  uint32_t elapsed = millis() - attemptedAt;
  if (connecting && elapsed >= Config::WiFiTimeoutMs) {
    WiFi.disconnect(); connecting = false;
  }
  if (!connecting && elapsed >= Config::WiFiRetryMs) connect();
}
bool wifiConnected() { return WiFi.status() == WL_CONNECTED; }
String wifiStatusText() {
  if (!Config::WiFiSsid[0]) return "Nicht konfiguriert";
  if (wifiConnected()) return "Verbunden";
  return connecting ? "Verbinde..." : "Offline / neuer Versuch";
}
String wifiAddress() { return wifiConnected() ? WiFi.localIP().toString() : "--"; }
String wifiSsid() { return String(Config::WiFiSsid); }
int32_t wifiRssi() { return wifiConnected() ? WiFi.RSSI() : 0; }
