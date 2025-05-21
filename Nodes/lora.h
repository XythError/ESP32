// ===== lora.h =============================
#ifndef LORA_H
#define LORA_H
#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include "config.h"

namespace LoraWrap {
  bool begin();
  bool send(const uint8_t *buf,size_t len);
}
#endif
// ===== Ende lora.h ========================