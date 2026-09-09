# BBQ Control 1.0 – Home Assistant direkt

Arduino-Anzeige fuer Waveshare ESP32-S3 Smart 86 Box. Display.cpp und Display.h
bleiben bytegleich zur funktionierenden V4. Kein MQTT, kein ESPHome.

## Einrichtung
1. Secrets.example.h als Secrets.h im Sketch-Ordner kopieren.
2. WLAN-Zugangsdaten eintragen.
3. Home Assistant unter http://192.168.178.200:8123 oeffnen. Im Benutzerprofil
   unter Sicherheit einen langlebigen Zugriffstoken erstellen und ausschliesslich
   lokal als BBQ_HA_TOKEN eintragen. Secrets.h ist von Git ausgeschlossen.
4. BBQ_HA_PROBE waehlen: meater_1 (Standard), meater_2, meater_3, meater_4
   oder meater_probe_0ae3b60f. Es wird eine Sonde gleichzeitig angezeigt.
5. Sketch kompilieren und erneut hochladen. Keine Automation und kein Broker noetig.

Die direkte REST-Abfrage nutzt POST /api/template alle zehn Sekunden in einer
separaten FreeRTOS-Aufgabe. Nur die Hauptschleife aktualisiert die Anzeige.
HTTP-Fehler und ungueltige Antworten leeren die Werte; ohne neue Antwort verfallen
sie nach 90 Sekunden. unavailable/unknown werden nicht als Null Grad dargestellt.
Die angegebene lokale HTTP-Adresse wird verwendet; Token nur im vertrauenswuerdigen
Heimnetz verwenden. Weiterleitungen werden nicht verfolgt.

## Sensorzuordnung
Fuer die gewaehlte Sonde wird sensor.<BBQ_HA_PROBE>_<Suffix> verwendet:

| Anzeige | Suffix |
|---|---|
| Kerntemperatur | innentemperatur |
| Garraumtemperatur | umgebungstemperatur |
| Zieltemperatur | soll_temperatur |
| Garstatus | kochstatus |

Die gelieferten Temperatursensoren haben Grad Celsius als Einheit.
In deiner Liste existiert kein Akkusensor: Akku bleibt --.
Die ebenfalls vorhandenen Sensoren kocht, spitzentemperatur, verbleibende_zeit
und verstrichene_zeit haben in der bisherigen V1-Anzeige noch kein Anzeigefeld.
Die Sonde wird bewusst ausgewaehlt; es wird nicht zwischen moeglicherweise
unterschiedlichen Grillstuecken automatisch gewechselt.

## Bauen und pruefen
Getestete Hardwarebasis: ESP32 Arduino Core 2.0.17 und GFX Library for Arduino 1.4.7.
Board ESP32S3 Dev Module, OPI PSRAM, USB CDC enabled, 16 MB Flash.

```sh
arduino-cli compile --fqbn esp32:esp32:esp32s3:PSRAM=opi,CDCOnBoot=cdc,FlashSize=16M arduino/BBQ-Control-V1
```

BBQ_NETWORK_COMPILE_TEST aktiviert Netzwerkpfade mit Dummywerten fuer Buildchecks;
diese Test-Firmware nicht hochladen. Ein Live-Test benoetigt deinen lokalen Token.
Alte MQTT-Blueprints im Repository sind nur fuer aeltere Firmwarestaende relevant.

REST-Dokumentation: https://developers.home-assistant.io/docs/api/rest/
