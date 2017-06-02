// Copyright 2017 David Conran

#ifndef TEST_IRSEND_TEST_H_
#define TEST_IRSEND_TEST_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <iostream>
#include <sstream>
#include <string>
#include "IRrecv.h"
#include "IRsend.h"

#define OUTPUT_BUF 1000U
#define RAW_BUF 1000U
class IRsendTest: public IRsend {
 public:
  uint32_t output[OUTPUT_BUF];
  uint16_t last;
  uint16_t rawbuf[RAW_BUF];
  decode_results capture;

  explicit IRsendTest(uint16_t x, bool i = false) : IRsend(x, i) {
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
      if ((i & 1) != outputOff )  // Odd XOR outputOff
        result << "s";
      else
        result << "m";
      result << output[i];
    }
    reset();
    return result.str();
  }

  void makeDecodeResult(uint16_t offset = 0) {
    capture.decode_type = UNKNOWN;
    capture.bits = 0;
    capture.rawlen = last + 1 - offset;
    capture.overflow = (last - offset >= (int16_t) RAW_BUF);
    capture.repeat = false;
    capture.address = 0;
    capture.command = 0;
    capture.value = 0;
    capture.rawbuf = rawbuf;
    for (uint16_t i = 0;
         (i < RAW_BUF - 1) && (offset < OUTPUT_BUF);
         i++, offset++)
      if (output[offset] > UINT16_MAX)
        rawbuf[i + 1] = UINT16_MAX / USECPERTICK;
      else
        rawbuf[i + 1] = output[offset] / USECPERTICK;
  }

  void dumpRawResult() {
    std::cout << "uint16_t rawbuf["<< capture.rawlen << "] =\n";
    for (uint16_t i = 0; i < capture.rawlen; i++) {
      std::cout << capture.rawbuf[i];
      std::cout << "(";
      std::cout << capture.rawbuf[i] * USECPERTICK;
      std::cout << "), ";
      if (i % 8 == 7)
        std::cout << "\n";
    }
    std::cout << "\n";
  }

  void addGap(uint32_t usecs) {
    space(usecs);
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
