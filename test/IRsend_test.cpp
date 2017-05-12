// Copyright 2017 David Conran

#include "IRsend_test.h"
#include "IRsend.h"
#include "gtest/gtest.h"

// Tests sendData().

// Test sending zero bits.
TEST(TestSendData, SendZeroBits) {
  IRsendTest irsend(4);
  irsend.begin();
  irsend.sendData(1, 2, 3, 4, 0b1, 0, true);
  EXPECT_EQ("", irsend.outputStr());
}

// Test sending zero and one.
TEST(TestSendData, SendSingleBit) {
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
