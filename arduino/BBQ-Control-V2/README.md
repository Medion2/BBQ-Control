# BBQ Control V2

Arduino ESP32-S3 Smart 86 Box. Die Displayinitialisierung ist bytegleich zu V1.
V2 verwendet Roboto (OFL, siehe FONT-LICENSE.txt), geglaettete 4-Bit-Schriftmasken,
RGB565-Farben und zwei PSRAM-Zeichenpuffer fuer einzelne Bildschirmbereiche.

Secrets.example.h lokal nach Secrets.h kopieren und WLAN sowie HA-Token eintragen.
Die REST-Verbindung nutzt http://192.168.178.200:8123 ohne MQTT.

Buildbasis: ESP32 Arduino Core 2.0.17 und GFX Library for Arduino 1.4.7.
Board ESP32S3 Dev Module, OPI PSRAM, 16 MB Flash, USB CDC aktiviert.

```sh
arduino-cli compile --fqbn esp32:esp32:esp32s3:PSRAM=opi,CDCOnBoot=cdc,FlashSize=16M arduino/BBQ-Control-V2
```

Der Verlauf beginnt beim Start und umfasst maximal 60 Minuten im RAM.
Fehlende Daten werden als Luecken behandelt. MQTT ist deaktiviert (graues Icon),
fehlender Akku ist unbekannt (oranges Icon), kein erfundener Prozentwert.
60 FPS ist ein Zeitbudget fuer die UI, keine gemessene Zusage fuer die Hardware.
