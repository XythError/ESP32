// ===== packet.h ===========================
#ifndef PACKET_H
#define PACKET_H
#include <Arduino.h>
#include <FastCRC.h>
#include "config.h"

#pragma pack(push,1)
struct HivePayload {
  uint8_t  version   = 0x01;
  uint8_t  id        = NODE_ID;
  int32_t  weight_g  = 0;    // 0.1 g Auflösung (×10)
  int16_t  temp_c    = 0;    // 0.01 °C (×100)
  uint16_t hum_pct   = 0;    // 0.01 %  (×100)
  uint16_t press_hpa = 0;    // 0.1 hPa (×10)
  uint16_t batt_mv   = 0;    // mV
  uint8_t  flags     = 0;    // bit0 = LowBatt / bit1 = SensorErr
  uint32_t crc32     = 0;    // am Ende anhängen
};
#pragma pack(pop)

inline void packet_update_crc(HivePayload &p) {
  FastCRC32 CRC32;
  p.crc32 = CRC32.crc32((uint8_t *)&p, sizeof(HivePayload) - sizeof(uint32_t));
}

#endif // PACKET_H
// ===== Ende packet.h ======================