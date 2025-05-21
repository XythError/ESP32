// ===== lora.cpp ===========================
#include "lora.h"
#include "config.h"

bool LoraWrap::begin(){
  SPI.begin(LORA_SCK,LORA_MISO,LORA_MOSI,LORA_CS);
  LoRa.setPins(LORA_CS,LORA_RST,LORA_DIO0);
  if(!LoRa.begin(LORA_FREQ)) return false;
  LoRa.setSpreadingFactor(LORA_SF);
  LoRa.idle();
  return true;
}

bool LoraWrap::available(){
  return LoRa.parsePacket();
}

bool LoraWrap::readPacket(HivePayload &out){
  int len = LoRa.readBytes((uint8_t*)&out, sizeof(HivePayload));
  if(len!=sizeof(HivePayload)) return false;
  return payloadCRCok(out);
}
// ===== Ende lora.cpp ======================