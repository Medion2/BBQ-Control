# BBQ-Control-Test-V4

Arduino-Projekt fuer Waveshare ESP32-S3 Smart 86 Box, 480 x 480.
Hardwarebasis: `soyosource_86box (3).ino`, SHA256
`3596f6237216c4fedd84b0c4b3c343ce20b4c5800a5973e0b1ce98861d195d5f`.
Display-Konstruktoren, Pins, Timings, ST7701-Sequenz, Expander-Reset und
Backlight-Initialisierung sind unveraendert in Display.cpp uebernommen.

## Bauen

- ESP32 Arduino Core **2.0.17**
- GFX Library for Arduino **1.4.7**
- Board: ESP32S3 Dev Module
- PSRAM: OPI PSRAM; USB CDC On Boot: Enabled; Flash Size: 16MB

```sh
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@2.0.17 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install "GFX Library for Arduino@1.4.7"
arduino-cli compile --fqbn esp32:esp32:esp32s3:PSRAM=opi,CDCOnBoot=cdc,FlashSize=16M arduino/BBQ-Control-Test-V4
```

Keine RS485-, Wechselrichter- oder ESPHome-Funktionen.
Ein erfolgreicher Build ersetzt keinen Test auf dem physischen Board.

## Bedienung und Tests

Nach dem Bootlogo: Displaytest, Hardware, Touchtest und Grafiktest.
Unten links/rechts wechseln; alternativ `p` / `n` im seriellen Monitor (115200).
Der Touchtest zeigt Koordinaten, Kontaktanzahl und I2C-Fehler. Bei Adressvariante
kann Config::TouchAddress auf 0x14 gesetzt werden; der Reset bleibt unveraendert.
Die Koordinatenbytes ab 0x8150 werden gemaess GT911-Registertabelle ausgewertet
(https://www.crystalfontz.com/controllers/GOODIX/GT911ProgrammingGuide/478/).
Kein neuer Touchdatensatz wird nicht als Loslassen interpretiert.
UI FPS misst die ausgefuehrten UI-Zyklen, nicht die physische Panel-Bildfrequenz.
Heap und PSRAM werden nach der Displayinitialisierung live abgefragt.

## WLAN und Uhr

In Config.h WiFiSsid und WiFiPassword lokal eintragen und Sketch neu hochladen.
Keine echten Zugangsdaten committen. Leere SSID deaktiviert Verbindungsversuche.
Verbindungsversuche haben 15 Sekunden Zeit; erneuter Versuch nach 30 Sekunden.
Display und Touch bleiben dabei aktiv. Es wird kein Setup-Access-Point gestartet.
Weitere Seiten: WLAN/NTP, digitale Uhr und analoge Uhr.
NTP startet bei WLAN-Verbindung und synchronisiert stuendlich. Vor dem ersten
Zeitabgleich wird keine erfundene Uhrzeit angezeigt. Nach Verbindungsverlust
laeuft die Systemuhr weiter; der Offline-Status bleibt sichtbar.
Die voreingestellte POSIX-Zeitzone bildet CET/CEST mit Sommerzeitwechsel ab.

## Pruefung auf dem Board

1. Bootlogo, vollstaendigen Rand, Farb- und Graubalken pruefen.
2. Heap/PSRAM pruefen, insbesondere ob OPI PSRAM erkannt wird.
3. Touch-Raster an allen Ecken, Ziehen und genau einen Seitenwechsel pro Tippen testen.
4. Grafikseite: bewegter Ball, Linien, Kreis, Dreieck und UI-FPS beobachten.
5. WLAN konfigurieren, IP/RSSI sowie NTP-Abgleich abwarten.
6. Digitale und analoge Zeit vergleichen; WLAN kurz abschalten und Wiederverbindung testen.
