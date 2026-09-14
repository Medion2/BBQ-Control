# BBQ Control 2.1.1 – UI nach Designvorlage

Arduino-Projekt fuer die Waveshare ESP32-S3 Smart 86 Box (480 x 480).
Die vom Nutzer bestaetigte Displayinitialisierung und die DMA-Bounce-Puffer bleiben unveraendert.
Keine Akkuanzeigen, kein MQTT, keine direkte Bluetooth-Verbindung: Messwerte kommen aus Home Assistant.

## Installation

Den kompletten Ordner BBQ-Control-V2 oeffnen, nicht nur die INO-Datei kopieren.
Die eigene Secrets.h aus dem bisherigen Projekt uebernehmen. Alternativ Secrets.example.h
nach Secrets.h kopieren und WLAN sowie Home-Assistant-Token lokal eintragen.
Home Assistant: http://192.168.178.200:8123. Zugangsdaten gehoeren nicht ins Repository.

Getestete Buildbasis: esp32 by Espressif Systems 3.3.11, GFX Library for Arduino 1.6.7.
Board: ESP32S3 Dev Module; OPI PSRAM; 16 MB Flash; USB CDC On Boot aktiviert.

```sh
arduino-cli compile --fqbn esp32:esp32:esp32s3:PSRAM=opi,CDCOnBoot=cdc,FlashSize=16M arduino/BBQ-Control-V2
```

## Die fuenf Seiten

- Start: grosse Kerntemperatur, geglaetteter Farbring, Ziel, Garstufe, HA-Restzeit,
  Trend, Alter der letzten Messung, WLAN/HA-Status, Sondenname und Garraumtemperatur.
- Verlauf: Kern, Garraum oder beide Kurven; 30/60/120 Minuten; automatische Skalierung;
  gestrichelte aktuelle Zieltemperatur und Minimum, Maximum, Mittelwert.
- Sonden: alle fuenf konfigurierten Sonden, aktuelle Temperatur, Ziel und Datenstatus.
  Antippen waehlt eine Sonde und oeffnet ihre Details.
- Details: Fleischart, Garstufe, Ziel, Garraum, Restzeit, Startzeit und visueller Zielalarm.
- Einstellungen: automatische Dimmung, Software-Helligkeit, Dunkel/AMOLED,
  visueller Alarm der gewaehlten Sonde, Celsius/Fahrenheit sowie Verbindung/Speicherinfo.

Unten wechseln vier feste Touch-Tabs zwischen Start, Verlauf, Sonden und Menue.
Auf Start, Sonden und Details wechselt Wischen die Sonde. Oben links geht es zurueck.
Das Menue-Icon im Ring oeffnet die Einstellungen. Garraum unten oeffnet den Verlauf.

Ziel antippen: mit Minus/Plus einstellen und speichern; Zurueck verwirft die Bearbeitung.
"Ziel aus Home Assistant verwenden" entfernt die lokale Ueberschreibung.
Lokale Ziele (1 bis 150 Grad C), Fleischart, Alarme und Anzeigeoptionen bleiben im NVS
ueber Neustarts erhalten. Ein Speicherfehler wird angezeigt. Es werden keine Sollwerte
an Home Assistant oder Meater geschrieben. Restzeit und Startzeit stammen weiterhin aus HA.

## Daten und Alarm

Meater 1 bis 4 sowie Probe 0ae3b60f nutzen die vorhandenen HA-Entitaeten.
Nicht verfuegbare Werte erscheinen als --. Veraltete oder getrennte Messungen loesen
keinen Zielalarm aus. Der sichtbare Alarm nennt die Sonde und wird mit OK quittiert.
Er wird erst nach Unterschreiten des Ziels um 0,5 Grad C oder einem neuen Ziel erneut scharf.
Ton und Vibration sind ohne bestaetigte Hardwareanbindung nicht implementiert.

Garstufen sind nur fuer das bestehende Rind-Profil abgeleitet. Bei Schwein, Gefluegel
und Fisch zeigt die UI MANUELLES ZIEL; es werden keine automatischen Garziele erfunden.
Der Trend benoetigt vier gueltige Minutenpunkte. Der Verlauf liegt im RAM und beginnt
nach Neustart neu. Datenluecken werden nicht verbunden. Min/Max/Mittel beziehen sich
auf die angezeigten Minutenpunkte (bei BEIDE auf die Kerntemperatur).

## Darstellung und Performance

Geglaettete Roboto-Schriften (FONT-LICENSE.txt), gerundete Karten und vorberechneter
Farbring. Die Ringtabelle entsteht mit tools/generate_reference_ring.py.
Die UI zeichnet nur geaenderte Bereiche. Bei Seitenwechsel, Themewechsel und Dimmung
wird der jeweilige Inhalt einmal neu aufgebaut. Zwei Widget-Puffer im PSRAM vermeiden
sichtbare Zwischenschritte innerhalb eines Widgets; sie ersetzen keine Hardware-Framebuffer.

Die Helligkeit skaliert die Pixelfarben und veraendert keine Backlight-Pins.
Nach optional 30/60/120 Sekunden wird gedimmt. Die erste Beruehrung weckt nur das Display.
Neue Zielalarme wecken ebenfalls. Anzeigeoptionen sind lokal.

Die bewaehrten 8 MHz Pixeltakt und 10 DMA-Bounce-Zeilen bleiben erhalten.
Das Panel erreicht damit nominal etwa 28 Hz; 60 FPS sind mit diesen Timings nicht erreichbar.
Der UI-Datentakt ist 200 ms (Touch wird weiterhin in jeder Schleife abgefragt), reale Geschwindigkeit und Touch-Verhalten muessen am Geraet
geprueft werden. Kompilierung ersetzt keinen Hardwaretest.

## Module

Dashboard steuert Seiten, Datenbindung und Alarme. TemperatureGauge zeichnet den Ring,
HistoryChart verwaltet und zeichnet Messreihen, StatusBar zeichnet die Navigation,
SettingsPage die Einstellungszeilen, TouchManager erkennt Gesten und Widgets stellt
geglattete Zeichenfunktionen bereit. UiState speichert lokale Einstellungen im NVS.
HomeAssistant, WiFiManager und Display bleiben die bestehende Hardware-/Datenbasis.

## HA-Stabilitaetskorrektur

Ein einzelner HTTP-/Parsefehler loescht keine gueltigen Messwerte mehr. Das HA-Symbol
wird orange, solange noch eine gueltige Antwort innerhalb der bestehenden 90-Sekunden-Frist
vorliegt; unter Menue / Info steht der Fehlercode. Alter und Messzeit werden durch Fehler
nicht zurueckgesetzt. Nach Ablauf der Frist, bei WLAN-Ausfall oder abgelehntem Token werden
keine aktuellen Messwerte angezeigt. Eine erfolgreiche Antwort ersetzt die Werte wieder.

Die aufwendige UI-Datenbindung laeuft mit 5 statt etwa 30 Durchlaeufen pro Sekunde.
Das reduziert temporaere String-Allokationen und CPU-Last. Das Design bleibt gleich;
Ringuebergaenge haben weniger Animationsschritte. WLAN- und Displayinitialisierung bleiben
unveraendert. HTTP-Fehler und Wiederherstellung werden im seriellen Monitor mit 115200 Baud
protokolliert (Code, Dauer, RSSI, freier Heap), ohne Token oder Antwortinhalt auszugeben.
Die Ursache der beobachteten HA-Abbrueche ist ohne Geraetelogs noch nicht abschliessend belegt.

Diagnosecodes: -1001 ungueltige/zu grosse HA-Antwort, -1002 HTTP-Initialisierung,
-1003 HA-Aufgabe/Queue konnte nicht angelegt werden. HTTP-Statuscodes (z.B. 401, 503)
und die negativen HTTPClient-Transportcodes (z.B. -11 Lese-Timeout) bleiben unterscheidbar.

Der Sekundenzaehler unter dem Ring aktualisiert separat nur 284 x 46 statt
284 x 312 Pixel. Unveraenderte Ringgrafik wird dadurch nicht jede Sekunde neu
uebertragen (rund 85 % weniger Pixel je reiner Altersaktualisierung).

Bei Fehler -1 (TCP-Aufbau fehlgeschlagen) wird genau einmal nach 500 ms erneut
verbunden, sofern WLAN noch steht. Das Connect-Timeout betraegt 6 statt 3 Sekunden;
der Lese-Timeout bleibt 3 Sekunden. Es gibt keinen WLAN-Neustart und keine schnelle
Endlosschleife. Danach gilt wieder das normale Abfrageintervall von 10 Sekunden.
