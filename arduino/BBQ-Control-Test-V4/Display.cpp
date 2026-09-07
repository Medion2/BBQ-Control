#include "Display.h"
#include <Wire.h>
#define BACKLIGHT_PIN 4
#define BLACK 0x0000

Arduino_XCA9554SWSPI *expander = new Arduino_XCA9554SWSPI(
    7 /* oe */, 0 /* cs */, 2 /* sck */, 1 /* sda */, &Wire, 0x20);

Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    17 /* DE */, 3 /* VSYNC */, 46 /* HSYNC */, 9 /* PCLK */,
    10, 11, 12, 13, 14,          // B0-B4
    21, 8, 18, 45, 38, 39,       // G0-G5
    40, 41, 42, 2, 1,            // R0-R4
    1, 10, 8, 50,                // hsync polarity, front, pulse, back
    1, 10, 8, 20);               // vsync polarity, front, pulse, back

Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
    480, 480, rgbpanel, 0, true,
    expander, GFX_NOT_DEFINED, st7701_type1_init_operations, sizeof(st7701_type1_init_operations));

void displayBegin() {
  Wire.begin(47, 48);

  expander->pinMode(5, OUTPUT);
  expander->pinMode(6, OUTPUT);
  expander->digitalWrite(6, LOW);
  delay(200);
  expander->digitalWrite(5, LOW);
  delay(200);
  expander->digitalWrite(5, HIGH);
  delay(200);

  if (!gfx->begin()) {
    Serial.println("FEHLER: gfx->begin() fehlgeschlagen!");
  }

  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, LOW);  // Backlight an

}
