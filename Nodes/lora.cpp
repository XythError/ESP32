// ===== lora.cpp ===========================
#include "lora.h"
#include "util.h"

bool LoraWrap::begin(){
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  LoRa.setPins(LORA_CS,LORA_RST,LORA_DIO0);
  if(!LoRa.begin(LORA_FREQ)){
    DBG("LoRa init failed\n");
    return false;
  }
  LoRa.setSpreadingFactor(LORA_SF);
  LoRa.setTxPower(LORA_TX_PWR);
  LoRa.idle();
  return true;
}

bool LoraWrap::send(const uint8_t *buf,size_t len){
  LoRa.beginPacket();
  LoRa.write(buf,len);
  int res = LoRa.endPacket(true); // async = true
  LoRa.idle();
  return res==0; // 0=OK
}
// ===== Ende lora.cpp ======================