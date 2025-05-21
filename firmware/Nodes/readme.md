/*
 ┌─────────────────────────────────────────────┐
 │  DATEISTRUKTUR                             │
 └─────────────────────────────────────────────┘
  HiveScale.ino           – Arduino‑Entry (setup/loop & Statemachine)
  config.h                – Alle anpassbaren Konstanten (Node‑ID, Pins, etc.)
  packet.h                – Payload‑Layout & CRC‑Handling
  sensors.h / sensors.cpp – HX711 + BME280 + (Wind)  Abstraktion
  power.h   / power.cpp   – Akkuspannung & Safety‑Shutdown
  lora.h    / lora.cpp    – LoRa‑Initialisierung & Senden
  util.h                  – Kleine Helper / Logging‑Macro

  (optionale Module OTA / Tempsense sind als Platzhalter markiert)
*/



// ===== Ende des Firmware‑Pakets ===========
// Hinweise zum Kompilieren (Arduino‑IDE):
//  1. Board: "ESP32 Dev Module" – Flash 4 MB, Partition: Default.
//  2. Benötigte Bibliotheken via Bibliotheks‑Manager:
//     • "HX711"  von Bogdan Necula (bogde)
//     • "Adafruit BME280" (+ Adafruit Sensor)
//     • "LoRa" von Sandeep Mistry
//     • "FastCRC" von Frank Boesing
//  3. Die Pins für LoRa/HX711 ggf. an Board‑Layout anpassen.
//  4. Kalibrierfaktor CAL_FACTOR im config.h pro Waage feinjustieren.
//  5. Für Batteriemessung geeignete Spannungsteiler (R1/R2) bestücken.
// ===========================================

Hier findest du einen ersten, lauffähigen Firmware-Satz für jede Bienenstock-Node.
Alle Quell­dateien ( HiveScale.ino, config.h, usw. ) stecken in der Canvas-Ansicht; du kannst dort sofort stöbern, Zeilen anpassen, oder neue Module ergänzen.

Was ist schon drin?

Bereich	Umsetzung	To-Dos / Ideen
Sensorebene	HX711-Anbindung mit Mittelwert-Filter, BME280 (Temp + Feuchte + Druck)	Temperatur-Driftkompensation, DS18B20 im Brutraum (falls gewünscht)
LoRa-Stack	SPI-Initialisierung, SF/BW per config.h, Sendefunktion ohne ACK (unidirektional)	AES-Verschlüsselung & CRC-ACK, Downlink-Konfigurations­pakete
Power-Management	Akku-Spannung per ADC, Low-Batt-Flag, Deep-Sleep ≈ 60 µA	Akku-Ladezustands­schätzung (Coulomb-Counter), dynamische Sende­leistung
Daten-Payload	Kompakte Binär­struktur, CRC-32 via FastCRC	Versions­inkrement bei Struktur­änderungen
Build-Setup	100 % Arduino-IDE-kompatibel, alle Bibliotheken im Kommentar	Umstieg auf PlatformIO (CI, Unit-Tests, Code-Metriken)

Nächste Schritte

Pins checken
Falls dein Board andere Bezeichnungen nutzt (s. Foto), einfach in config.h anpassen.

Kalibrieren
CAL_FACTOR im Betrieb feinjustieren: Gewicht aufsitzen, Rohwert im Serial-Plotter ansehen, Skalierung berechnen → Wert übernehmen.

Batterie-Teiler bestücken
Der Code rechnet mit 100 kΩ / 27 kΩ. Ändert sich das, unbedingt R1/R2 in config.h anpassen.

Reichweiten-Test
Bei Bedarf Spreading-Factor hochsetzen (bis 12 → mehr Reichweite, längere Airtime). Denk an Duty-Cycle-Regeln (1 % im 868 MHz-Band).

Basis­station anpassen
Dort muss die gleiche HivePayload-Struktur dekodiert werden; CRC-Prüfung nicht vergessen.