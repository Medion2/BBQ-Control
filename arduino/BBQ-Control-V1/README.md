# BBQ Control 1.0

Arduino-Anzeige fuer Waveshare ESP32-S3 Smart 86 Box (480 x 480).
Display.cpp und Display.h sind unveraendert aus BBQ-Control-Test-V4 kopiert.

## Bauen

ESP32 Arduino Core 2.0.17, GFX Library for Arduino 1.4.7.
Board ESP32S3 Dev Module, OPI PSRAM, USB CDC enabled, 16 MB Flash.

```sh
arduino-cli compile --fqbn esp32:esp32:esp32s3:PSRAM=opi,CDCOnBoot=cdc,FlashSize=16M arduino/BBQ-Control-V1
```

WLAN lokal in Config.h eintragen. Die Anzeige startet ohne erfundene Messwerte.
Kern-, Garraum-, Zieltemperatur und Meater-Akku sind bei fehlenden oder veralteten
Daten als -- dargestellt. Garstatus wird nicht aus einer Temperatur abgeleitet.
Die Version steuert weder Heizung noch Luefter oder Wechselrichter.
