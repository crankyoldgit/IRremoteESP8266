// Copyright 2017 David Conran

#include "IRtimer.h"
#include <Arduino.h>

// This class performs a simple time in useconds since instantiated.
// Handles when the system timer wraps around (once).

IRtimer::IRtimer() {
  reset();
}

void IRtimer::reset() {
  start = micros();
}

uint32_t IRtimer::elapsed() {
  uint32_t now = micros();
  if (start <= now)  // Check if the system timer has wrapped.
    return now - start;  // No wrap.
  else
    return UINT32_MAX - start + now;  // Has wrapped.
}
