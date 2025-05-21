// ===== power.cpp ==========================
#include "power.h"

float Power::readBattery(){
  uint16_t raw = analogRead(BATT_ADC_PIN);
  float v = (raw/4095.0f)*3.3f; // ADC → V on pin
  float vin = v * (BATT_R1+BATT_R2)/BATT_R2;
  return vin; // Volt am Akku
}

bool Power::isLow(float thr){
  return readBattery() < thr;
}
// ===== Ende power.cpp =====================