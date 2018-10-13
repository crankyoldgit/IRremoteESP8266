// Copyright 2018 David Conran

#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

// Tests for sendPioneer().

// Test sending typical data only.
TEST(TestSendPioneer, SendDataOnly) {
  IRsendTest irsend(0);
  irsend.begin();
  irsend.sendPioneer(0);
  EXPECT_EQ(
      "m8960s4480"
      "m560s560m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
      "m560s560m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
      "m560s560m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
      "m560s560m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
      "m560s58240"
      "m560s989"
      "m8960s4480"
      "m560s560m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
      "m560s560m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
      "m560s560m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
      "m560s560m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
      "m560s58240", irsend.outputStr());
  irsend.sendPioneer(0x55FF00AAAA00FF55);
  EXPECT_EQ(
      "m8960s4480"
      "m560s560m560s1680m560s560m560s1680m560s560m560s1680m560s560m560s1680"
      "m560s1680m560s1680m560s1680m560s1680m560s1680m560s1680m560s1680m560s1680"
      "m560s560m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
      "m560s1680m560s560m560s1680m560s560m560s1680m560s560m560s1680m560s560"
      "m560s40320"
      "m560s989"
      "m8960s4480"
      "m560s1680m560s560m560s1680m560s560m560s1680m560s560m560s1680m560s560"
      "m560s560m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
      "m560s1680m560s1680m560s1680m560s1680m560s1680m560s1680m560s1680m560s1680"
      "m560s560m560s1680m560s560m560s1680m560s560m560s1680m560s560m560s1680"
      "m560s40320", irsend.outputStr());
}

// Tests for decodePioneer().

// Synthesised Normal Pioneer message.
TEST(TestDecodePioneer, SyntheticPioneerDecode) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  irsend.reset();
  EXPECT_EQ(0x0123456789ABCDEF, irsend.encodePioneer(0x01234567, 0x89ABCDEF));
  irsend.sendPioneer(irsend.encodePioneer(0x01234567, 0x89ABCDEF));
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(PIONEER, irsend.capture.decode_type);
  EXPECT_EQ(kPioneerBits, irsend.capture.bits);
  EXPECT_EQ(0x0123456789ABCDEF, irsend.capture.value);
  EXPECT_EQ(0x01234567, irsend.capture.address);
  EXPECT_EQ(0x89ABCDEF, irsend.capture.command);
}
