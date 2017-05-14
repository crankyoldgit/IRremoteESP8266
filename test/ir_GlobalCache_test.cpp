// Copyright 2017 David Conran

#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

// Tests for sendGlobalCache().

// Test sending a typical command wihtout a repeat.
TEST(TestSendGlobalCache, NonRepeatingCode) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();
  irsend.reset();

    // Modified NEC TV "Power On" from Global Cache with no repeats
  uint16_t gc_test[71] = {38000, 1, 1, 342, 172, 21, 22, 21, 21, 21, 65, 21, 21,
                          21, 22, 21, 22, 21, 21, 21, 22, 21, 65, 21, 65, 21,
                          22, 21, 65, 21, 65, 21, 65, 21, 65, 21, 65, 21, 65,
                          21, 22, 21, 22, 21, 21, 21, 22, 21, 22, 21, 65, 21,
                          22, 21, 21, 21, 65, 21, 65, 21, 65, 21, 64, 22, 65,
                          21, 22, 21, 65, 21, 1519};
  irsend.sendGC(gc_test, 71);
  irsend.makeDecodeResult();
  EXPECT_EQ("m7866s3956m483s506m483s483m483s1495m483s483m483s506m483s506"
            "m483s483m483s506m483s1495m483s1495m483s506m483s1495m483s1495"
            "m483s1495m483s1495m483s1495m483s1495m483s506m483s506m483s483"
            "m483s506m483s506m483s1495m483s506m483s483m483s1495m483s1495"
            "m483s1495m483s1472m506s1495m483s506m483s1495m483s34937",
            irsend.outputStr());
  EXPECT_TRUE(irrecv.decodeNEC(&irsend.capture));
  EXPECT_EQ(NEC, irsend.capture.decode_type);
  EXPECT_EQ(NEC_BITS, irsend.capture.bits);
  EXPECT_EQ(0x20DF827D, irsend.capture.value);
  EXPECT_EQ(0x4, irsend.capture.address);
  EXPECT_EQ(0x41, irsend.capture.command);
}

// Test sending typical command with repeats.
TEST(TestSendGlobalCache, RepeatCode) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();
  irsend.reset();

    // Sherwood (NEC-like) "Power On" from Global Cache with 2 repeats
  uint16_t gc_test[75] = {38000, 2, 69, 341, 171, 21, 64, 21, 64, 21, 21, 21,
                          21, 21, 21, 21, 21, 21, 21, 21, 64, 21, 64, 21, 21,
                          21, 64, 21, 21, 21, 21, 21, 21, 21, 64, 21, 21, 21,
                          64, 21, 21, 21, 21, 21, 21, 21, 64, 21, 21, 21, 21,
                          21, 21, 21, 21, 21, 64, 21, 64, 21, 64, 21, 21, 21,
                          64, 21, 64, 21, 64, 21, 1600, 341, 85, 21, 3647};
  irsend.sendGC(gc_test, 75);
  irsend.makeDecodeResult();
  EXPECT_EQ("m7843s3933m483s1472m483s1472m483s483m483s483m483s483m483s483"
            "m483s483m483s1472m483s1472m483s483m483s1472m483s483m483s483"
            "m483s483m483s1472m483s483m483s1472m483s483m483s483m483s483"
            "m483s1472m483s483m483s483m483s483m483s483m483s1472m483s1472"
            "m483s1472m483s483m483s1472m483s1472m483s1472m483s36800"
            "m7843s1955m483s83881"
            "m7843s1955m483s83881", irsend.outputStr());
  EXPECT_TRUE(irrecv.decodeNEC(&irsend.capture));
  EXPECT_EQ(NEC, irsend.capture.decode_type);
  EXPECT_EQ(NEC_BITS, irsend.capture.bits);
  EXPECT_EQ(0xC1A28877, irsend.capture.value);
  EXPECT_EQ(0x4583, irsend.capture.address);
  EXPECT_EQ(0x11, irsend.capture.command);
}
