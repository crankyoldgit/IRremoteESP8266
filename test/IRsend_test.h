// Copyright 2017 David Conran

#ifndef TEST_IRSEND_TEST_H_
#define TEST_IRSEND_TEST_H_

#include <sstream>
#include <string>
#include "IRsend.h"

#define OUTPUT_BUF 1000U
class IRsendTest: public IRsend {
 public:
  uint32_t output[OUTPUT_BUF];
  uint16_t last;

  explicit IRsendTest(uint16_t x) : IRsend(x) {
    reset();
  }

  void reset() {
    last = 0;
    output[last] = 0;
  }

  std::string outputStr() {
    std::stringstream result;
    if (last == 0 && output[0] == 0)
      return "";
    for (uint16_t i = 0; i <= last; i++) {
      if (i & 1)  // Odd
        result << "s";
      else
        result << "m";
      result << output[i];
    }
    reset();
    return result.str();
  }

 protected:
  uint16_t mark(uint16_t usec) {
    if (last >= OUTPUT_BUF)
      return 0;
    if (last & 1)  // Is odd? (i.e. last call was a space())
      output[++last] = usec;
    else
      output[last] += usec;
    return 0;
  }

  void space(uint32_t time) {
    if (last >= OUTPUT_BUF)
      return;
    if (last & 1) {  // Is odd? (i.e. last call was a space())
      output[last] += time;
    } else {
      output[++last] = time;
    }
  }
};
#endif  // TEST_IRSEND_TEST_H_
