#include "Clock.h"
#include "Config.h"
#include "WiFiManager.h"
#include <esp_sntp.h>
namespace {
bool started = false, synchronized = false;
volatile bool syncEvent = false;
uint32_t lastSyncAt = 0;
void onSync(struct timeval *) { syncEvent = true; }
}
void clockUpdate() {
  if (wifiConnected() && !started) {
    sntp_set_time_sync_notification_cb(onSync);
    configTzTime(Config::TimeZone, Config::NtpServer1, Config::NtpServer2);
    sntp_set_sync_interval(3600000);
    started = true;
  }
  if (syncEvent) { syncEvent = false; synchronized = true; lastSyncAt = millis(); }
}
bool clockLocalTime(tm &local) {
  time_t now = time(nullptr);
  if (!synchronized || now < 1704067200) return false;
  return localtime_r(&now, &local) != nullptr;
}
String clockStatusText() {
  if (!synchronized) return wifiConnected() ? "NTP: warte auf Zeit" : "NTP: WLAN erforderlich";
  return String(wifiConnected() ? "NTP vor " : "Offline / NTP vor ") + String((millis()-lastSyncAt)/60000) + " min";
}
