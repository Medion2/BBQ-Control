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
