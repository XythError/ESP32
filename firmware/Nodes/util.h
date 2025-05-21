// ===== util.h ============================
#ifndef UTIL_H
#define UTIL_H
#include "config.h"
#if DEBUG_LVL
  #define DBG(...) do{ Serial.printf(__VA_ARGS__); }while(0)
#else
  #define DBG(...)
#endif
#endif
// ===== Ende util.h ========================