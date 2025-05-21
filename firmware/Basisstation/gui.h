// ===== gui.h ==============================
#ifndef GUI_H
#define GUI_H
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "payload.h"
#include "config.h"

struct HiveData {
  HivePayload pl;
  uint32_t    lastUpdate  = 0; // millis
};

namespace GUI {
  bool begin();
  void update(const HiveData *dataArr);
  void loopTouch(HiveData *dataArr);
}
#endif
// ===== Ende gui.h =========================
