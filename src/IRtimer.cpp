// Copyright 2017 David Conran

#include "IRtimer.h"
#ifndef UNIT_TEST
#include <Arduino.h>
#endif

#ifdef UNIT_TEST
// Used to help simulate elapsed time in unit tests.
extern uint32_t _IRtimer_unittest_now;
#endif  // UNIT_TEST

// This class performs a simple time in useconds since instantiated.
// Handles when the system timer wraps around (once).

IRtimer::IRtimer() {
  reset();
}

void IRtimer::reset() {
#ifndef UNIT_TEST
  start = micros();
#else
  start = _IRtimer_unittest_now;
#endif
}

uint32_t IRtimer::elapsed() {
#ifndef UNIT_TEST
  uint32_t now = micros();
#else
  uint32_t now = _IRtimer_unittest_now;
#endif
  if (start <= now)  // Check if the system timer has wrapped.
    return now - start;  // No wrap.
  else
    return UINT32_MAX - start + now;  // Has wrapped.
}

// Only used in unit testing.
void IRtimer::add(uint32_t usecs) {
#ifdef UNIT_TEST
  _IRtimer_unittest_now += usecs;
#endif  // UNIT_TEST
}
