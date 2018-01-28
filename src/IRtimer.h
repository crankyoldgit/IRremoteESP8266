// Copyright 2017 David Conran

#ifndef IRTIMER_H_
#define IRTIMER_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>

// Classes
class IRtimer {
 public:
  IRtimer();
  void reset();
  uint32_t elapsed();
  static void add(uint32_t usecs);

 private:
  uint32_t start;
};

#endif  // IRTIMER_H_
