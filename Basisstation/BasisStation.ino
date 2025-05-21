// ===== baseStation.ino ====================
#include "config.h"
#include "lora.h"
#include "gui.h"

HiveData hives[HIVE_COUNT];
uint32_t lastUI = 0;

void setup(){
  Serial.begin(115200);
  DBG("\nHiveScale Base – Boot\n");
  if(!LoraWrap::begin()){
    DBG("LoRa init FAILED!\n");
    while(1) delay(1000);
  }
  GUI::begin();
}

void loop(){
  // --- LoRa Empfang ---
  while(LoraWrap::available()){
    HivePayload p;
    if(LoraWrap::readPacket(p)){
      uint8_t idx=p.id-1; // Node‑ID 1..9 → 0..8
      if(idx<HIVE_COUNT){
        hives[idx].pl = p;
        hives[idx].lastUpdate = millis();
        DBG("Node %u update – %.2f kg\n",p.id,p.weight_g/1000.0f);
      }
    }
  }

  // --- UI Refresh ---
  if(millis()-lastUI>UI_REFRESH_MS){
    GUI::update(hives);
    lastUI = millis();
  }

  // --- Touch ---
  GUI::loopTouch(hives);
}
// ===== Ende baseStation.ino ===============