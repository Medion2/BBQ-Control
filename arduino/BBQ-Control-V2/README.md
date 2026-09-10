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

## Bedienung

- Links/rechts wischen: Meater 1 bis 4 und Probe 0ae3b60f wechseln.
- Garraum antippen: Garraumtemperatur, Spitzentemperatur, verstrichene Garzeit,
  Gericht und HA-Prozessstatus anzeigen. Oben links geht es zurueck.
- Zieltemperatur antippen: mit Minus/Plus ein lokales Anzeigeziel zwischen 1 und
  150 Grad C einstellen. Erst "Lokal speichern" uebernimmt es. "HA-Ziel verwenden"
  entfernt die lokale Ueberschreibung. Oben links bricht ohne Speichern ab.
- Die lokale Ueberschreibung gilt je Sonde bis zum Neustart. Sie steuert weder
  Meater noch Home Assistant; deine Solltemperatur-Entitaeten sind nur lesbare Sensoren.
- Statusleiste antippen: Verbindungsdetails. MQTT bleibt bewusst deaktiviert.

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
Hardware-Scanout-Puffer: die funktionierende Displayinitialisierung bleibt unberuehrt.
Bei Speichermangel wird auf einen Widget-Puffer zurueckgefallen.
Seitenwechsel zeichnen nur den Seiteninhalt neu; die Statusleiste bleibt separat.

Touch verwendet den vorhandenen GT911 am bestehenden I2C-Bus, Adresse 0x5D bzw. 0x14.
Fehlende/fehlerhafte Touchmeldungen brechen eine Geste ab statt einen Klick auszulösen.
Touch-Ausrichtung, reale Bildrate und Darstellung muessen noch am Geraet geprueft werden.
