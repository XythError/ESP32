// ===== HiveScale.ino ======================
#include "config.h"
#include "util.h"
#include "sensors.h"
#include "power.h"
#include "lora.h"
#include "packet.h"
#include <Preferences.h>

Sensors sensors;
Preferences prefs;

void goDeepSleep(){
  sensors.powerDown();
  esp_sleep_enable_timer_wakeup( (uint64_t)TX_INTERVAL_MS * 1000ULL );
  DBG("Going to sleep for %lu ms\n",TX_INTERVAL_MS);
  esp_deep_sleep_start();
}

void setup(){
  Serial.begin(115200);
  delay(200);
  DBG("\nHiveScale Node – Boot\n");
  if(!sensors.begin()){
    DBG("Sensor init failed, halt\n");
  }
  if(!LoraWrap::begin()){
    DBG("LoRa failed – halt\n");
  }
  prefs.begin("hive",false);
}

void loop(){
  float kg, t, rh, p;
  bool okWeight = sensors.readWeight(kg);
  bool okEnv    = sensors.readEnv(t,rh,p);

  float battV   = Power::readBattery();
  uint8_t flags = 0;
  if(Power::isLow()) flags |= 0x01;
  if(!(okWeight && okEnv)) flags |= 0x02;

  HivePayload pl;
  pl.weight_g  = (int32_t)(kg * 1000.0f); // g
  pl.temp_c    = (int16_t)(t * 100.0f);
  pl.hum_pct   = (uint16_t)(rh * 100.0f);
  pl.press_hpa = (uint16_t)(p * 10.0f);
  pl.batt_mv   = (uint16_t)(battV * 1000.0f);
  pl.flags     = flags;
  packet_update_crc(pl);

  DBG("Weight: %.2f kg\n",kg);
  DBG("Temp: %.2f °C  Hum: %.1f %%  Press: %.1f hPa\n",t,rh,p);
  DBG("Batt: %.2f V\n",battV);
  DBG("Sending packet…\n");

  LoraWrap::send((uint8_t*)&pl,sizeof(pl));

  // Optional Retry/ACK could be implemented here

  goDeepSleep();
}
// ===== Ende HiveScale.ino =================