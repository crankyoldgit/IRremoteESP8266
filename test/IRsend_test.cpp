// Copyright 2017 David Conran

#include <string>
#include "IRsend.h"
#include "gtest/gtest.h"

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
    // std::string result = "";
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

// Tests sendData().

// Test sending zero bits.
TEST(sendData, SendZeroBits) {
  IRsendTest irsend(4);
  irsend.begin();
  irsend.sendData(1, 2, 3, 4, 0b1, 0, true);
  EXPECT_EQ("", irsend.outputStr());
}

// Test sending zero and one.
TEST(sendData, SendSingleBit) {
  IRsendTest irsend(4);
  irsend.begin();
  irsend.sendData(1, 2, 3, 4, 0b1, 1, true);
  EXPECT_EQ("m1s2", irsend.outputStr());
  irsend.sendData(1, 2, 3, 4, 0b0, 1, true);
  EXPECT_EQ("m3s4", irsend.outputStr());
}

// Test sending bit order.
TEST(TestSendData, TestingBitSendOrder) {
  IRsendTest irsend(4);
  irsend.begin();
  irsend.sendData(1, 2, 3, 4, 0b10, 2, true);
  EXPECT_EQ("m1s2m3s4", irsend.outputStr());
  irsend.sendData(1, 2, 3, 4, 0b10, 2, false);
  EXPECT_EQ("m3s4m1s2", irsend.outputStr());
  irsend.sendData(1, 2, 3, 4, 0b0001, 4, false);
  EXPECT_EQ("m1s2m3s4m3s4m3s4", irsend.outputStr());
}

// Test sending typical data.
TEST(TestSendData, SendTypicalData) {
  IRsendTest irsend(4);
  irsend.begin();
  irsend.sendData(1, 2, 3, 4, 0b1010110011110000, 16, true);
  EXPECT_EQ("m1s2m3s4m1s2m3s4m1s2m1s2m3s4m3s4m1s2m1s2m1s2m1s2m3s4m3s4m3s4m3s4",
            irsend.outputStr());
  irsend.sendData(1, 2, 3, 4, 0x1234567890ABCDEF, 64, true);
  EXPECT_EQ("m3s4m3s4m3s4m1s2m3s4m3s4m1s2m3s4m3s4m3s4m1s2m1s2m3s4m1s2m3s4m3s4"
            "m3s4m1s2m3s4m1s2m3s4m1s2m1s2m3s4m3s4m1s2m1s2m1s2m1s2m3s4m3s4m3s4"
            "m1s2m3s4m3s4m1s2m3s4m3s4m3s4m3s4m1s2m3s4m1s2m3s4m1s2m3s4m1s2m1s2"
            "m1s2m1s2m3s4m3s4m1s2m1s2m3s4m1s2m1s2m1s2m1s2m3s4m1s2m1s2m1s2m1s2",
            irsend.outputStr());
}

// Test sending more than expected bits.
TEST(TestSendData, SendOverLargeData) {
  IRsendTest irsend(4);
  irsend.begin();
  irsend.sendData(1, 2, 3, 4, 0xFFFFFFFFFFFFFFFF, 70, true);
  EXPECT_EQ("m3s4m3s4m3s4m3s4m3s4m3s4"
            "m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2"
            "m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2"
            "m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2"
            "m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2m1s2",
            irsend.outputStr());
  }
