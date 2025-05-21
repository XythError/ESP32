// ===== power.h ============================
#ifndef POWER_H
#define POWER_H
#include <Arduino.h>
#include "config.h"

namespace Power {
  float readBattery();
  bool  isLow(float v=BATT_MIN_V);
}
#endif
// ===== Ende power.h =======================