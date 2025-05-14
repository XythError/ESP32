# ESP32
Developing ESP32 including devices for edjucational purpose

# ESP32-S3 Sensorstation mit Display‑Menü, SD‑Logging & Webserver

Dieses Projekt richtet sich an alle Bastler, die eine kompakte IoT‑Sensorstation mit ESP32‑S3 realisieren möchten. Es verbindet:

* **OLED‑Display (SSD1306)** mit Menüführung und Statusanzeige
* **DHT11‑Temperatur‑ und Luftfeuchtesensor**
* **DS1307‑RTC** (Echtzeituhr) auf separatem Hardware‑I²C
* **SD‑Kartenlogging** der Sensordaten im CSV‑Format
* **Webserver im AP‑Modus** mit Dateiserver und JSON‑API
* **RGB‑LED** für Statusfeedback
* **4 Taster** zur Bedienung (Up, Down, OK, Back)

---

## Inhaltsverzeichnis

1. [Features](#features)
2. [Hardware](#hardware)
3. [Schaltplan & Pinbelegung](#schaltplan--pinbelegung)
4. [Software‑Setup](#software‑setup)
5. [Installation](#installation)
6. [Menü‑Navigation & Bedienung](#menü‑navigation--bedienung)
7. [Web‑Schnittstelle](#web‑schnittstelle)
8. [CSV‑Logging auf SD](#csv‑logging-auf-sd)
9. [Anpassung & Erweiterung](#anpassung--erweiterung)
10. [Fehlerbehebung](#fehlerbehebung)
11. [Lizenz](#lizenz)

---

## Features

* **Startbildschirm**: Anzeige von WLAN‑SSID, IP, Uhrzeit, Temperatur/Luftfeuchte, SD‑Status
* **Hauptmenü** mit vier Einträgen:

  1. WiFi Scan (Suche nach Netzwerken)
  2. File Server (AP‑Modus & HTTP‑Dateiserver)
  3. Sensors (Großanzeige der aktuellen Messwerte)
  4. Reboot (Neustart des ESP)
  5. Home (Zurück zum Startbildschirm)
* **Webserver im Access‑Point‑Modus**:

  * `/` → HTML‑Übersichtsseite
  * `/logs` → Liste der auf SD gespeicherten CSV‑Dateien
  * `/data.json` → Aktuelle Messwerte als JSON
* **Automatisches SD‑Logging** im CSV‑Format (stündlich, konfigurierbar)
* **Programmierung in Arduino-IDE** (C++) mit modularen Setup‑Funktionen
* **Status‑RGB‑LED** (grün: Webserver aktiv, blau: WLAN, gelb/orange: Fehler)

## Hardware

* **ESP32‑S3 Dev Module**
* **SSD1306 OLED** 128×64 (I²C, Software‑I²C auf GPIO 20/21)
* **DS1307 RTC** (Hardware‑I²C auf GPIO 15/16)
* **DHT11** Temperatursensor (Daten auf GPIO 39)
* **SD‑Card‑Module** (SPI: CS=GPIO 10, MOSI=13, MISO=11, SCK=12)
* **RGB‑LED** (Common Cathode/Anode, GPIO 1/2/37)
* **4 Pushbuttons** (Up 36, Down 35, OK 48, Back 47)

## Schaltplan & Pinbelegung

| Funktion       | Modul/Pin   | ESP32‑S3 GPIO |
| -------------- | ----------- | ------------- |
| **OLED SDA**   | SSD1306 SDA | 20            |
| **OLED SCL**   | SSD1306 SCL | 21            |
| **RTC SDA**    | DS1307 SDA  | 15            |
| **RTC SCL**    | DS1307 SCL  | 16            |
| **DHT11 Data** | DHT11 DATA  | 39            |
| **SD CS**      | SD CS       | 10            |
| **SD MOSI**    | SD MOSI     | 13            |
| **SD MISO**    | SD MISO     | 11            |
| **SD SCK**     | SD SCK      | 12            |
| **RGB Red**    | LED R       | 1             |
| **RGB Green**  | LED G       | 2             |
| **RGB Blue**   | LED B       | 37            |
| **BTN Up**     | Taster Up   | 36            |
| **BTN Down**   | Taster Down | 35            |
| **BTN OK**     | Taster OK   | 48            |
| **BTN Back**   | Taster Back | 47            |

> **Hinweis:** Pin 1 (UART0 TX) kann mit Serial kollidieren. Alternativ eigene PWM-fähige Pins wählen.

## Software‑Setup

1. Arduino IDE installieren (Version ≥1.8.13).
2. ESP32‑S3 Board‑Definition hinzufügen:

   * URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   * Bord „ESP32S3 Dev Module“ auswählen.
3. Bibliotheken installieren (über Library‑Manager):

   * `U8g2`
   * `Adafruit RTClib`
   * `DHT sensor library` + `Adafruit Unified Sensor`
   * `ESP32 SD library`
   * `WebServer`

## Installation

1. Pin‑Macros oben im Sketch an deine Verdrahtung anpassen.
2. In Arduino IDE unter **Tools → Board → ESP32S3 Dev Module** auswählen.
3. **Upload**. Öffne den Seriellen Monitor (115200 Baud) für Debug‑Ausgaben.

## Menü‑Navigation & Bedienung

* **K0 (Up) / K1 (Down):** Durch Menüs scrollen
* **K2 (OK):** Auswahl bestätigen
* **K3 (Back):** Zurück / Home

### Menüstruktur

```text
HOME → [K2]
  MENU:
    0. WiFi Scan     → Scan starten, Ergebnisse anzeigen
    1. File Server   → AP + HTTP-Server an/aus
    2. Sensors       → Messwerte groß anzeigen
    3. Reboot        → ESP neu starten
    4. Home          → Zurück zum Startbildschirm
```

## Web‑Schnittstelle

Wenn der File Server aktiv ist, verbindet sich dein PC/Smartphone mit dem WLAN „ESP32‑S3‑AP“ (Passwort `password123`) und ruft im Browser:

* `http://192.168.4.1/` → HTML‑Dashboard
* `http://192.168.4.1/logs` → Linkliste der CSV‑Logs
* `http://192.168.4.1/data.json` → Aktuelle Messwerte als JSON

## CSV‑Logging auf SD

* Tägliche Dateien: `log_YYYYMMDD.csv`
* Intervall konfigurierbar (`SENSOR_LOG_INTERVAL_MINUTES`) in Minuten
* Jede Datei enthält Header: `Timestamp,Temperature,Humidity`

## Anpassung & Erweiterung

* **Intervall ändern:** `#define SENSOR_LOG_INTERVAL_MINUTES 60`
* **Menüpunkte erweitern:** `mainMenuItems[]` und `performAction()` anpassen
* **OTA‑Update:** ArduinoOTA integrieren
* **Grafische Web‑Dashboards:** Chart.js + AJAX in HTML einbinden

## Fehlerbehebung

* **RTC-Fehler:** Prüfe SDA/SCL, Batterie für DS1307
* **DHT-Fehler:** Sensor resetten, `delay(2000)` vor dem ersten `read()`
* **Display flackert:** I²C‑Bus-Konflikt vermeiden, evtl. auf HW‑I²C umstellen
* **SD-Karte nicht mountbar:** 3.3 V‑Level, richtige CS‑Pin‑Belegung
* **WebServer 404:** Routen in `startFileServer()` und `setup()` korrekt registrieren

## Lizenz

Dieses Projekt steht unter der **MIT License**.

---

*Viel Spaß beim Basteln und Anpassen!*
