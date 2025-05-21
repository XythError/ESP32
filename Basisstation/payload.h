// ===== payload.h ==========================
#ifndef PAYLOAD_H
#define PAYLOAD_H
#include <Arduino.h>
#include <FastCRC.h>
#include "config.h"

#pragma pack(push,1)
struct HivePayload {
  uint8_t  version;
  uint8_t  id;
  int32_t  weight_g;
  int16_t  temp_c;
  uint16_t hum_pct;
  uint16_t press_hpa;
  uint16_t batt_mv;
  uint8_t  flags;
  uint32_t crc32;
};
#pragma pack(pop)

static bool payloadCRCok(const HivePayload &p){
  FastCRC32 CRC32;
  uint32_t calc=CRC32.crc32((uint8_t*)&p, sizeof(HivePayload)-sizeof(uint32_t));
  return calc==p.crc32;
}

#endif // PAYLOAD_H
// ===== Ende payload.h =====================