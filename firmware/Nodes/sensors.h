// ===== sensors.h ==========================
#ifndef SENSORS_H
#define SENSORS_H
#include <Arduino.h>
#include <HX711.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include "config.h"

class Sensors {
public:
  bool begin();
  bool readWeight(float &kg);
  bool readEnv(float &t,float &rh,float &p);
  void powerDown();
private:
  HX711 _scale;
  Adafruit_BME280 _bme;
};
#endif
// ===== Ende sensors.h =====================