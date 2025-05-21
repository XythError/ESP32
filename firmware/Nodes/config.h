// ===== config.h =============================
#ifndef CONFIG_H
#define CONFIG_H

// ---------- Node‑spezifische Einstellungen ----------
#define NODE_ID            3          // 1 … 9
constexpr float CAL_FACTOR = 2150.0f; // HX711 Skalierung (g pro Roh‑Einheit)
constexpr uint32_t TX_INTERVAL_MS = 300000; // 5 min Deep‑Sleep‑Intervall

// ---------- Energie‑Management ----------
constexpr float BATT_MIN_V = 3.50f; // Unterer Akku‑Schwellwert

// ---------- LoRa‑Parameter ----------
constexpr long   LORA_FREQ   = 868E6;   // 868 MHz EU‑ISM
constexpr int8_t LORA_TX_PWR = 14;      // dBm
constexpr uint8_t LORA_SF    = 9;       // Spreading‑Factor (7‑12)
// LoRa‑Pins (angepasst an das Pin‑Bild)
#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23
#define LORA_CS    14  // NSS/SS
#define LORA_RST   26
#define LORA_DIO0  33  // IRQ

// ---------- HX711 ----------
#define HX711_DT   4
#define HX711_SCK  5

// ---------- I²C (BME280) ----------
#define I2C_SDA    21
#define I2C_SCL    22

// ---------- Batterie‑Messung ----------
#define BATT_ADC_PIN  34          // ADC1_CH6
constexpr float BATT_R1 = 100.0f;  // kΩ  – zu 3V3
constexpr float BATT_R2 = 27.0f;   // kΩ  – GND

// ---------- Debug ----------
#define DEBUG_LVL 1   // 0 = OFF / 1 = ON

#endif // CONFIG_H
// ===== Ende config.h ======================