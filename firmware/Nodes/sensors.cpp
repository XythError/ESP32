// ===== sensors.cpp ========================
#include "sensors.h"
#include "util.h"

bool Sensors::begin(){
  // HX711
  _scale.begin(HX711_DT, HX711_SCK);
  _scale.set_scale(CAL_FACTOR);
  _scale.tare();
  // BME280
  Wire.begin(I2C_SDA,I2C_SCL);
  bool bmeOK=_bme.begin(0x76);
  if(!bmeOK){ DBG("BME280 not found!\n"); }
  return bmeOK;
}

bool Sensors::readWeight(float &kg){
  if(!_scale.is_ready()) return false;
  const uint8_t N=10;
  double sum=0;
  for(uint8_t i=0;i<N;i++) sum+=_scale.get_units(1);
  kg=sum/N;
  return true;
}

bool Sensors::readEnv(float &t,float &rh,float &p){
  if(!_bme.begin()) return false; // already init but safety
  t  = _bme.readTemperature();
  rh = _bme.readHumidity();
  p  = _bme.readPressure()/100.0f; // hPa
  return true;
}

void Sensors::powerDown(){
  _scale.power_down();
}
// ===== Ende sensors.cpp ===================