// ===== lora.h =============================
#ifndef LORA_WRAP_H
#define LORA_WRAP_H
#include <Arduino.h>
#include <LoRa.h>
#include <SPI.h>
#include "payload.h"

namespace LoraWrap {
  bool begin();
  bool available();
  bool readPacket(HivePayload &out);
}
#endif
// ===== Ende lora.h ========================