// ===== config.h ===========================
#ifndef CONFIG_H
#define CONFIG_H

//-------------------------------------------
// LoRa Empfang
//-------------------------------------------
#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23
#define LORA_CS    14
#define LORA_RST   26
#define LORA_DIO0  33  // IRQ
constexpr long   LORA_FREQ   = 868E6;
constexpr uint8_t LORA_SF    = 9;

//-------------------------------------------
// TFT & Touch
//-------------------------------------------
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST   4   // kann auch -1 -> Reset‑Pin TFT verbunden
#define TOUCH_CS  5
#define TOUCH_IRQ 27
#define TFT_ROTATION  1  // Landscape

//-------------------------------------------
// Anzeigeeinstellungen
//-------------------------------------------
constexpr uint32_t UI_REFRESH_MS = 1000; // 1 s UI‑Update
constexpr uint8_t  HIVE_COUNT    = 9;    // max. Nodes

//-------------------------------------------
// Debug
//-------------------------------------------
#define DEBUG_LVL 1
#if DEBUG_LVL
 #define DBG(...)  do{ Serial.printf(__VA_ARGS__);}while(0)
#else
 #define DBG(...)
#endif

#endif // CONFIG_H
// ===== Ende config.h ======================