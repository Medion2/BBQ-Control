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

## Einrichten

1. Secrets.example.h als Secrets.h kopieren. WLAN und MQTT-Broker (Host, Benutzer,
   Passwort) eintragen. Secrets.h wird durch die lokale .gitignore ausgeschlossen.
   Port in Config.h: 1883 / MQTT ueber TCP im lokalen Netz.
2. Sketch kompilieren und auf die Smart 86 Box laden.
3. Home Assistant: MQTT-Integration mit demselben Broker einrichten.
4. Den Blueprint `home-assistant/blueprints/automation/bbq_control/meater_to_mqtt.yaml`
   nach `/config/blueprints/automation/bbq_control/meater_to_mqtt.yaml` kopieren,
   Blueprints neu laden und daraus eine Automation erstellen.
5. Kern- und Garraumtemperatur auswaehlen; Ziel, Garstatus und Akku optional zuordnen.
   Zielwerte als sensor/number/input_number muessen Celsius oder Fahrenheit liefern;
   ohne Einheit wird Celsius angenommen. Akku muss Prozent liefern, nicht Volt.

Alternativ den Blueprint ueber seine GitHub-URL in Home Assistant importieren.
Die Standard-Meater-Integration liefert laut ihrer Dokumentation keinen Akkuwert.
Mit einer anderen Integration vorhandene Prozent-Entitaet auswaehlen; sonst bleibt --.
Die Automation wird hier nur bereitgestellt, nicht in Home Assistant installiert.

## MQTT-Vertrag

| Topic | Richtung | Inhalt |
|---|---|---|
| bbq/control/v1/meater/state | Home Assistant zum Display | JSON-Snapshot, QoS 1, retain false |
| bbq/control/v1/availability | Display zum Broker | online/offline, retained, Last Will |
| homeassistant/status | Home Assistant zum Display | online/offline (HA-Standardtopic) |

```json
{"schema":1,"probe_online":true,"core_c":54.2,"ambient_c":120.0,"target_c":65.0,"cook_state":"started","battery_pct":null}
```

Alle Felder sind erforderlich; fehlende Sensorwerte werden als JSON null gesendet.
Keine numerischen Strings. Grenzen: Kern -40..150 Grad C, Garraum -40..600 Grad C,
Ziel 0..150 Grad C, Akku 0..100 Prozent. Ungueltige Datensaetze werden verworfen
und erneuern nicht den Frische-Timer. Daten laufen nach 90 Sekunden ohne gueltigen
Snapshot ab. Retained Temperaturdatensaetze werden verworfen. Nach Wiederverbindung
wird auf einen neuen Snapshot gewartet. MQTT laeuft in der ESP32-Netzwerkaufgabe;
nur die Arduino-Hauptschleife aktualisiert Messwerte und Display. Fragmentierte
Nachrichten werden bis maximal 1024 Byte zusammengesetzt.

Der Blueprint sendet bei Temperaturaenderung, alle 30 Sekunden und wenn das Display
online geht. Aeltere last_reported-Zeitstempel der Temperatursensoren fuehren zu
probe_online=false (standardmaessig nach 120 Sekunden, im Blueprint einstellbar).
MQTT-Verbindung und Home-Assistant-Status werden separat angezeigt.
Fuer mehrere Displays muessen AvailabilityTopic und MQTT-Trigger im Blueprint je
Geraet angepasst werden. Die Client-ID wird automatisch aus der Chip-ID erzeugt.

## Pruefen

- Gueltigen Snapshot senden: alle Werte erscheinen; null bleibt --.
- Sonde unavailable, HA offline, WLAN/Broker-Ausfall und Wiederverbindung pruefen.
- Ohne Nachrichten 90 Sekunden warten: keine aktuelle Temperatur anzeigen.
- Alten retained Snapshot senden: darf beim Verbinden keine aktuelle Messung werden.
- Akku 101, Text statt Zahl, abgeschnittenes JSON: Datensatz muss verworfen werden.
- Garstatus finished wird als Fertig angezeigt; keine automatische Heizungssteuerung.

Compile-Test der aktivierten Netzwerkpfade mit Dummy-Konfiguration (nicht hochladen):

```sh
arduino-cli compile --fqbn esp32:esp32:esp32s3:PSRAM=opi,CDCOnBoot=cdc,FlashSize=16M --build-property compiler.cpp.extra_flags=-DBBQ_NETWORK_COMPILE_TEST arduino/BBQ-Control-V1
```

Quellen:
- https://www.home-assistant.io/integrations/meater/
- https://www.home-assistant.io/integrations/mqtt/
- https://docs.espressif.com/projects/esp-idf/en/v4.4.7/esp32s3/api-reference/protocols/mqtt.html
