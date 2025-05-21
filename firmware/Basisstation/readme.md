// ===========================================
//  Benötigte Bibliotheken (Arduino‑IDE):
//   • "TFT_eSPI"  von Bodmer  (User_Setup: ILI9488 + SPI Pins oben)
//   • "XPT2046_Touchscreen"  von Paul Stoffregen
//   • "LoRa"  von Sandeep Mistry
//   • "FastCRC"  von Frank Boesing
// ===========================================

gleich aufgebaut wie die Node-Firmware, aber mit:

4″ IPS-Touch-Display (ILI9488 + XPT2046)
480 × 320 px, Landscape, ca. 18 € – passt aufs Dev-Kit-ESP32 und liefert genug Platz für 9 Kacheln.

Dashboard-GUI
– Gewicht, Temperatur, Luft­feuchte, Druck & Akku pro Stock.
– Berühr eine Kachel ⇒ Pop-up mit Detaildaten.

LoRa-Empfang + CRC-Prüfung
identische HivePayload-Struktur, daher 100 % kompatibel mit deinen Nodes.

Modular wie gehabt
config.h, lora.*, gui.*, payload.h, baseStation.ino.
Pins, Farben, Update-Intervalle usw. stellst du wieder zentral in config.h um.

Kurz-Checkliste zum Loslegen
TFT_eSPI Setup
Öffne User_Setup.h der Library → Panel ILI9488_DRIVER, Pin-Mapping laut Kommentar im Code.

Libraries installieren
TFT_eSPI, XPT2046_Touchscreen, LoRa, FastCRC (alle via Bibliotheks-Manager).

LoRa-Pins & Frequenz
Falls deine Basis andere Pins nutzt oder 915 MHz nötig ist – in config.h tweaken.

Touch-Kalibrierung
Ist dein Touch leicht versetzt, einfach die ts.setRotation()- und Mapping-Zeilen in gui.cpp anpassen.

Erweitern

OTA-Updates verteilen

Wetter-Node separat anzeigen

Logging auf SD-Karte oder MQTT-Bridge fürs Smart-Home