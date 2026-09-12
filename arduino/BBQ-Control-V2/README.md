# BBQ Control V2

Arduino ESP32-S3 Smart 86 Box. Pins, Panel-Befehle und Resetfolge entsprechen V1.
Mit Nutzerfreigabe ist der RGB-Pixeltakt nun explizit auf 8 MHz reduziert.
V2 verwendet Roboto (OFL, siehe FONT-LICENSE.txt), geglaettete 4-Bit-Schriftmasken,
RGB565-Farben und zwei PSRAM-Zeichenpuffer fuer einzelne Bildschirmbereiche.

Secrets.example.h lokal nach Secrets.h kopieren und WLAN sowie HA-Token eintragen.
Die REST-Verbindung nutzt http://192.168.178.200:8123 ohne MQTT.

Buildbasis fuer den Bounce-Buffer-Test: ESP32 Arduino Core 3.3.11 und
GFX Library for Arduino 1.6.7 (entspricht der gemeldeten Nutzerinstallation).
Board ESP32S3 Dev Module, OPI PSRAM, 16 MB Flash, USB CDC aktiviert.

```sh
arduino-cli compile --fqbn esp32:esp32:esp32s3:PSRAM=opi,CDCOnBoot=cdc,FlashSize=16M arduino/BBQ-Control-V2
```

Der Verlauf beginnt beim Start und umfasst maximal 60 Minuten im RAM.
Fehlende Daten werden als Luecken behandelt. MQTT bleibt deaktiviert.
Fehlender Akku ist unbekannt (oranges Icon), kein erfundener Prozentwert.
60 FPS ist ein Zeitbudget fuer die UI, keine gemessene Zusage fuer die Hardware.

## Bedienung

- Links/rechts wischen: Meater 1 bis 4 und Probe 0ae3b60f wechseln.
- Garraum antippen: Garraumtemperatur, Spitzentemperatur, verstrichene Garzeit,
  Gericht und HA-Prozessstatus anzeigen. Oben links geht es zurueck.
- Den grossen Temperaturbereich links antippen: mit Minus/Plus ein lokales Anzeigeziel zwischen 1 und
  150 Grad C einstellen. Erst "Lokal speichern" uebernimmt es. "HA-Ziel verwenden"
  entfernt die lokale Ueberschreibung. Oben links bricht ohne Speichern ab.
- Die lokale Ueberschreibung gilt je Sonde bis zum Neustart. Sie steuert weder
  Meater noch Home Assistant; deine Solltemperatur-Entitaeten sind nur lesbare Sensoren.
- Verbindungskarte oder WLAN-/Haus-Icon oben antippen: Verbindungsdetails.
  Auf Detailseiten ist auch die untere Statusleiste antippbar.

Alle acht gelieferten Sensoren je Sonde werden abgefragt. Akkudaten fehlen in der
Sensorliste; die Karte bleibt --. Restzeit (HA) wird aus dem Endzeitpunkt berechnet,
Garzeit aus dem Startzeitpunkt. Beide stammen von HA und aendern sich nicht durch
ein lokales Anzeigeziel. Nicht verfuegbare Sensoren erzeugen keine Ersatzmesswerte.

## Gargrad und Ring

Der Ring bildet Kern-/Zieltemperatur ab (0 bis 100 Prozent). Die Zahlen zeigen stets
den letzten echten Messwert; nur der Ringuebergang wird geglaettet.
Das vorlaeufige Gargrad-Profil ist ausdruecklich als Rind gekennzeichnet.
Grenzen in Config.h: Aufheizen unter 49, Rare ab 49, Medium Rare ab 54,
Medium ab 57, Medium Well ab 63, Well Done ab 68 Grad C.
Diese kulinarische Einteilung ist keine Lebensmittelsicherheitsbewertung und nicht
fuer beliebige Fleischarten geeignet. Profil vor Verwendung fuer anderes Fleisch anpassen.
Quelle: https://www.thermoworks.com/blogs/thermoblog/steak-temperature-guide

## Darstellung und Grenzen

Der Verlauf haelt 60 Minuten je Sonde im RAM. Der aktuelle Minutenpunkt wird bei
neuen Messungen aktualisiert; bei Ausfaellen entstehen Luecken. Nach einem Neustart
beginnt ein neuer Verlauf. Eine historische HA-Abfrage ist nicht erforderlich.
Die Y-Achse skaliert automatisch. Werte werden nach 90 Sekunden ohne Antwort ungueltig.
Die HA-Abfrage laeuft alle zehn Sekunden in einer eigenen Aufgabe.

Jede Karte wird nur bei Aenderung uebertragen. Ringanimationen erhalten ein
16-ms-Zeitbudget. Die serielle Ausgabe meldet die tatsaechlich gezeichneten
Aenderungsframes pro Sekunde; im ruhenden Dashboard ist dieser Wert absichtlich niedrig.
Zwei Widget-Puffer brauchen zusammen 460800 Byte PSRAM. Sie sind keine neuen
Hardware-Scanout-Puffer: sie aendern die RGB-Pufferkonfiguration nicht.
Bei Speichermangel wird auf einen Widget-Puffer zurueckgefallen.
Seitenwechsel zeichnen den Seiteninhalt neu; die Kopfzeile bleibt separat.

Touch verwendet den vorhandenen GT911 am bestehenden I2C-Bus, Adresse 0x5D bzw. 0x14.
Fehlende/fehlerhafte Touchmeldungen brechen eine Geste ab statt einen Klick auszulösen.
Touch-Ausrichtung, reale Bildrate und Darstellung muessen noch am Geraet geprueft werden.

Die Ringgeometrie und Kantenglaettung liegen vorberechnet im Flash (RingPixels.h).
Damit gibt es pro Animationsbild keine atan2-/Wurzelberechnung fuer den Ring.
Die Tabelle kann mit `python tools/generate_ring.py` reproduziert werden.
Breite, dreistellige Kernwerte erscheinen ohne Nachkommastelle, damit sie innerhalb
des Rings bleiben. Messdaten und Verlauf behalten ihre urspruengliche Genauigkeit.

## Test gegen dauerhaftes Flackern

Der Pixeltakt ist explizit 8 MHz statt des OPI-Standardwerts 12 MHz der getesteten
GFX-Version 1.4.7. Das reduziert die fortlaufenden PSRAM-Lesezugriffe des Displays.
Pinbelegung, Taktflanke, Synchronisationszeiten, Panel-Befehlssequenz und Resetfolge
bleiben gleich. Die serielle Ausgabe nennt beim Start den angeforderten Pixeltakt.

Bei unveraenderten Timings entspricht das nominal etwa 28 Hz statt 42 Hz.
Diese Testeinstellung priorisiert Stabilitaet; 60 Bildschirmbilder/s sind damit
nicht erreichbar. Die Reduktion allein hat das Flackern laut Nutzer nicht behoben.
Der folgende Test aktiviert deshalb die DMA-Zwischenpuffer der neueren Bibliothek.

Zum direkten Vergleich kann in Secrets.h `#define BBQ_LCD_PCLK_HZ 12000000L`
eingetragen und neu kompiliert werden. Ohne diese Zeile werden 8 MHz verwendet.


## Design nach Referenzbild

Das Querformat der Vorlage ist fuer das vorhandene 480 x 480 Display angepasst:
links der grosse Temperatur-Ring, darunter ein 60-Minuten-Verlauf mit gestricheltem
Zielwert und ein Zielstatus. Rechts stehen gleich grosse Karten fuer Garraum,
Restzeit, Akku und Home Assistant sowie eine Karte fuer die ausgewaehlte Sonde.
Schwarzer Hintergrund, dunkle Karten, blaue Beschriftung und gruene Verlaufslinie.
Die Kopfzeile zeigt echte WLAN-/HA-Zustaende. Es gibt keine direkte Bluetooth-
Verbindung zur Sonde; darum erscheint kein Bluetooth-Verbunden-Symbol.

Die Uhr synchronisiert sich bei WLAN-Verbindung per NTP (pool.ntp.org/time.nist.gov),
mit deutscher Sommer-/Winterzeit. Bis zur ersten Synchronisation erscheint --:--.
Restzeit ist Stunden:Minuten, auf volle Minuten aufgerundet. Die Aktualisierungs-
angabe nennt das Alter der letzten erfolgreichen HA-Abfrage, nicht den Zeitpunkt
der physischen Messung. Ein erreichbares HA allein bedeutet keine aktuelle Sonde.

Der Zielhinweis wird nur gruen, wenn eine frische Kerntemperatur mindestens das
angezeigte Ziel erreicht. Ein lokales Ziel ist im Ring mit LOKAL gekennzeichnet.
Diagramm, Ring und Zielhinweis verwenden dasselbe Anzeigeziel; die HA-Restzeit
bleibt davon unabhaengig. Akku bleibt mangels Sensor --. Es werden keine
Beispielmessungen, Vorhersagekurven oder Verbindungszustaende im Sketch erzeugt.

Das Referenzdesign hat die Displaykonfiguration nicht veraendert. Der nachfolgende
Bounce-Buffer-Test aendert nur die RGB-Pufferoptionen und Startdiagnose.

## Flackertest: interne DMA-Zwischenpuffer

Fuer ESP32-Core 3.x und Arduino-GFX 1.6.7 wird der bisher standardmaessig
abgeschaltete Bounce Buffer aktiviert: 480 * 10 Pixel je Puffer, zwei Puffer
mit zusammen 19200 Byte internem RAM (RGB565). Der Bildspeicher bleibt im PSRAM;
der Treiber speist den RGB-DMA-Datenstrom ueber die internen Zwischenpuffer.
Dies adressiert moegliche Unterbrechungen beim PSRAM-Zugriff, nicht alle Ursachen
von Flackern oder sichtbare Bildwechsel. Die Wirksamkeit muss am Geraet geprueft werden.

Unveraendert: alle GPIOs, RGB-Zuordnung, Taktflanke, Porches, Pixeltakt,
ST7701-Befehlsfolge, Expander-Reset, Hintergrundbeleuchtung und Dashboard.
Unter Core 2.x bleibt der vorherige Pfad aktiv; er testet diese Puffer nicht.

1. ESP32-Core 3.3.11 und GFX Library for Arduino 1.6.7 verwenden.
2. Board: ESP32S3 Dev Module, OPI PSRAM, 16 MB Flash; bestehende Secrets.h behalten.
3. Sketch kompilieren und hochladen. Serieller Monitor: 115200 Baud.
4. Startausgabe kontrollieren: LCD bounce buffer: 4800 pixels (10 lines).
5. Mit unveraendertem Netzteil und Kabel das ruhende Dashboard sowie wechselnde
   Messwerte beobachten. Auf Helligkeitspulsieren, horizontale Versetzung und
   Streifen achten. Ein erfolgreiches Kompilieren beweist noch keine Bildstabilitaet.

Zum A/B-Vergleich in Secrets.h `#define BBQ_LCD_BOUNCE_LINES 0` setzen und erneut
hochladen. Mit `10` oder ohne diese Zeile sind die Zwischenpuffer aktiv.
Die Zeilenzahl muss 480 teilen; erlaubt sind 0 bis 40. Fuer den ersten Test 10
verwenden. Bestehende BBQ_LCD_PCLK_HZ-Einstellungen bei beiden Tests gleich lassen.

Quellen:
- https://github.com/moononournation/Arduino_GFX/blob/v1.6.7/src/databus/Arduino_ESP32RGBPanel.cpp
- https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/lcd/rgb_lcd.html
