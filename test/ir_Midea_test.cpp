// Copyright 2017 David Conran

#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

// Tests for sendMidea().

// Test sending typical data only.
TEST(TestSendMidea, SendDataOnly) {
  IRsendTest irsend(4);
  irsend.begin();

  irsend.reset();
  irsend.sendMidea(0x0);
  EXPECT_EQ(
      "m4300s4300"
      "m500s500m500s500m500s500m500s500m500s500m500s500m500s500m500s500"
      "m500s500m500s500m500s500m500s500m500s500m500s500m500s500m500s500"
      "m500s500m500s500m500s500m500s500m500s500m500s500m500s500m500s500"
      "m500s500m500s500m500s500m500s500m500s500m500s500m500s500m500s500"
      "m500s500m500s500m500s500m500s500m500s500m500s500m500s500m500s500"
      "m500s500m500s500m500s500m500s500m500s500m500s500m500s500m500s500"
      "m500s5300"
      "m4300s4300"
      "m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700"
      "m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700"
      "m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700"
      "m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700"
      "m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700"
      "m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700"
      "m500s5300", irsend.outputStr());

  irsend.reset();
  irsend.sendMidea(0x55AA55AA55AA);
  EXPECT_EQ(
      "m4300s4300"
      "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
      "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
      "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
      "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
      "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
      "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
      "m500s5300"
      "m4300s4300"
      "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
      "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
      "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
      "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
      "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
      "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
      "m500s5300", irsend.outputStr());

  irsend.reset();
  irsend.sendMidea(0xFFFFFFFFFFFF);
  EXPECT_EQ(
      "m4300s4300"
      "m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700"
      "m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700"
      "m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700"
      "m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700"
      "m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700"
      "m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700"
      "m500s5300"
      "m4300s4300"
      "m500s500m500s500m500s500m500s500m500s500m500s500m500s500m500s500"
      "m500s500m500s500m500s500m500s500m500s500m500s500m500s500m500s500"
      "m500s500m500s500m500s500m500s500m500s500m500s500m500s500m500s500"
      "m500s500m500s500m500s500m500s500m500s500m500s500m500s500m500s500"
      "m500s500m500s500m500s500m500s500m500s500m500s500m500s500m500s500"
      "m500s500m500s500m500s500m500s500m500s500m500s500m500s500m500s500"
      "m500s5300", irsend.outputStr());
}

// Test sending with different repeats.
TEST(TestSendMidea, SendWithRepeats) {
  IRsendTest irsend(4);
  irsend.begin();

  irsend.reset();
  irsend.sendMidea(0x55AA55AA55AA, MIDEA_BITS, 1);  // 1 repeat.
  EXPECT_EQ(
      "m4300s4300"
      "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
      "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
      "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
      "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
      "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
      "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
      "m500s5300"
      "m4300s4300"
      "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
      "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
      "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
      "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
      "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
      "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
      "m500s5300"
      "m4300s4300"
      "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
      "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
      "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
      "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
      "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
      "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
      "m500s5300"
      "m4300s4300"
      "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
      "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
      "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
      "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
      "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
      "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
      "m500s5300", irsend.outputStr());
  irsend.sendMidea(0x55AA55AA55AA, MIDEA_BITS, 2);  // 2 repeats.
  EXPECT_EQ(
    "m4300s4300"
    "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
    "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
    "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
    "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
    "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
    "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
    "m500s5300"
    "m4300s4300"
    "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
    "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
    "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
    "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
    "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
    "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
    "m500s5300"
    "m4300s4300"
    "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
    "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
    "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
    "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
    "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
    "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
    "m500s5300"
    "m4300s4300"
    "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
    "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
    "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
    "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
    "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
    "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
    "m500s5300"
    "m4300s4300"
    "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
    "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
    "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
    "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
    "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
    "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
    "m500s5300"
    "m4300s4300"
    "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
    "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
    "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
    "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
    "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500"
    "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700"
    "m500s5300", irsend.outputStr());
}

// Test sending an atypical data size.
TEST(TestSendMidea, SendUsualSize) {
  IRsendTest irsend(4);
  irsend.begin();

  irsend.reset();
  irsend.sendMidea(0x0, 8);
  EXPECT_EQ(
      "m4300s4300"
      "m500s500m500s500m500s500m500s500m500s500m500s500m500s500m500s500"
      "m500s5300"
      "m4300s4300"
      "m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700m500s1700"
      "m500s5300", irsend.outputStr());

  irsend.reset();
  irsend.sendMidea(0x1234567890ABCDEF, 64);
  EXPECT_EQ(
      "m4300s4300"
      "m500s500m500s500m500s500m500s1700m500s500m500s500m500s1700m500s500"
      "m500s500m500s500m500s1700m500s1700m500s500m500s1700m500s500m500s500"
      "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s1700m500s500"
      "m500s500m500s1700m500s1700m500s1700m500s1700m500s500m500s500m500s500"
      "m500s1700m500s500m500s500m500s1700m500s500m500s500m500s500m500s500"
      "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s1700"
      "m500s1700m500s1700m500s500m500s500m500s1700m500s1700m500s500m500s1700"
      "m500s1700m500s1700m500s1700m500s500m500s1700m500s1700m500s1700m500s1700"
      "m500s5300"
      "m4300s4300"
      "m500s1700m500s1700m500s1700m500s500m500s1700m500s1700m500s500m500s1700"
      "m500s1700m500s1700m500s500m500s500m500s1700m500s500m500s1700m500s1700"
      "m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s500m500s1700"
      "m500s1700m500s500m500s500m500s500m500s500m500s1700m500s1700m500s1700"
      "m500s500m500s1700m500s1700m500s500m500s1700m500s1700m500s1700m500s1700"
      "m500s500m500s1700m500s500m500s1700m500s500m500s1700m500s500m500s500"
      "m500s500m500s500m500s1700m500s1700m500s500m500s500m500s1700m500s500"
      "m500s500m500s500m500s500m500s1700m500s500m500s500m500s500m500s500"
      "m500s5300", irsend.outputStr());

  // Bit sizes must be a multiple of 8.
  irsend.reset();
  irsend.sendMidea(0x0, 17);
  EXPECT_EQ("" , irsend.outputStr());
}

// Tests for decodeMidea().

// Decode normal Midea messages with strict set.
TEST(TestDecodeMidea, NormalDecodeWithStrict) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  // Normal Midea 48-bit message.
  irsend.reset();
  irsend.sendMidea(0x1234567890AB);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeMidea(&irsend.capture, MIDEA_BITS, true));
  EXPECT_EQ(MIDEA, irsend.capture.decode_type);
  EXPECT_EQ(MIDEA_BITS, irsend.capture.bits);
  EXPECT_EQ(0x1234567890AB, irsend.capture.value);
  EXPECT_EQ(0x0, irsend.capture.address);
  EXPECT_EQ(0x0, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);

  // Normal Midea 48-bit message.
  irsend.reset();
  irsend.sendMidea(0x0);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeMidea(&irsend.capture, MIDEA_BITS, true));
  EXPECT_EQ(MIDEA, irsend.capture.decode_type);
  EXPECT_EQ(MIDEA_BITS, irsend.capture.bits);
  EXPECT_EQ(0x0, irsend.capture.value);
  EXPECT_EQ(0x0, irsend.capture.address);
  EXPECT_EQ(0x0, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);

  // Normal Midea 24-bit message.
  irsend.reset();
  irsend.sendMidea(0xFFFFFF);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeMidea(&irsend.capture, MIDEA_BITS, true));
  EXPECT_EQ(MIDEA, irsend.capture.decode_type);
  EXPECT_EQ(MIDEA_BITS, irsend.capture.bits);
  EXPECT_EQ(0xFFFFFF, irsend.capture.value);
  EXPECT_EQ(0x0, irsend.capture.address);
  EXPECT_EQ(0x0, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);

  // Normal Midea 48-bit message via just decode().
  // i.e. No conficts with other decoders.
  irsend.reset();
  irsend.sendMidea(0x1234567890AB);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MIDEA, irsend.capture.decode_type);
  EXPECT_EQ(MIDEA_BITS, irsend.capture.bits);
  EXPECT_EQ(0x1234567890AB, irsend.capture.value);
  EXPECT_EQ(0x0, irsend.capture.address);
  EXPECT_EQ(0x0, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);
}

// Decode normal repeated Midea messages.
TEST(TestDecodeMidea, NormalDecodeWithRepeatAndStrict) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  // Normal Midea 48-bit message with 2 repeats.
  irsend.reset();
  irsend.sendMidea(0x123456, MIDEA_BITS, 2);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeMidea(&irsend.capture, MIDEA_BITS, true));
  EXPECT_EQ(MIDEA, irsend.capture.decode_type);
  EXPECT_EQ(MIDEA_BITS, irsend.capture.bits);
  EXPECT_EQ(0x123456, irsend.capture.value);
  EXPECT_FALSE(irsend.capture.repeat);

  irsend.makeDecodeResult(2 * (2 * MIDEA_BITS + 4));
  ASSERT_TRUE(irrecv.decodeMidea(&irsend.capture, MIDEA_BITS, true));
  EXPECT_EQ(MIDEA, irsend.capture.decode_type);
  EXPECT_EQ(MIDEA_BITS, irsend.capture.bits);
  EXPECT_EQ(0x123456, irsend.capture.value);

  irsend.makeDecodeResult(4 * (2 * MIDEA_BITS + 4));
  ASSERT_TRUE(irrecv.decodeMidea(&irsend.capture, MIDEA_BITS, true));
  EXPECT_EQ(MIDEA, irsend.capture.decode_type);
  EXPECT_EQ(MIDEA_BITS, irsend.capture.bits);
  EXPECT_EQ(0x123456, irsend.capture.value);
}

// Decode unsupported Midea messages.
TEST(TestDecodeMidea, DecodeWithNonStrictSizes) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  irsend.sendMidea(0x12, 8);  // Illegal value Midea 8-bit message.
  irsend.makeDecodeResult();
  // Should fail with strict on.
  ASSERT_FALSE(irrecv.decodeMidea(&irsend.capture, MIDEA_BITS, true));
  // Should pass if strict off.
  ASSERT_TRUE(irrecv.decodeMidea(&irsend.capture, 8, false));
  EXPECT_EQ(MIDEA, irsend.capture.decode_type);
  EXPECT_EQ(8, irsend.capture.bits);
  EXPECT_EQ(0x12, irsend.capture.value);

  irsend.reset();
  irsend.sendMidea(0x12345678, 32);  // Illegal value Midea 32-bit message.
  irsend.makeDecodeResult();
  // Shouldn't pass with strict when we ask for less bits than we got.
  ASSERT_FALSE(irrecv.decodeMidea(&irsend.capture, MIDEA_BITS, true));

  irsend.makeDecodeResult();
  // Should fail with strict when we ask for the wrong bit size.
  ASSERT_FALSE(irrecv.decodeMidea(&irsend.capture, 32, true));
  // Should pass if strict off.
  ASSERT_TRUE(irrecv.decodeMidea(&irsend.capture, 32, false));
  EXPECT_EQ(MIDEA, irsend.capture.decode_type);
  EXPECT_EQ(32, irsend.capture.bits);
  EXPECT_EQ(0x12345678, irsend.capture.value);

  // Decode should fail if asked to decode non-multiples of 8 bits.
  irsend.reset();
  irsend.sendMidea(0x123456, MIDEA_BITS, 2);
  irsend.makeDecodeResult();
  ASSERT_FALSE(irrecv.decodeMidea(&irsend.capture, 9, false));
}

// Decode (non-standard) 64-bit messages.
TEST(TestDecodeMidea, Decode64BitMessages) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  // Illegal size Midea 64-bit message.
  irsend.sendMidea(0xFFFFFFFFFFFFFFFF, 64);
  irsend.makeDecodeResult();
  // Should work with a 'normal' match (not strict)
  ASSERT_TRUE(irrecv.decodeMidea(&irsend.capture, 64, false));
  EXPECT_EQ(MIDEA, irsend.capture.decode_type);
  EXPECT_EQ(64, irsend.capture.bits);
  EXPECT_EQ(0xFFFFFFFFFFFFFFFF, irsend.capture.value);
}

// Fail to decode a non-Midea example via GlobalCache
TEST(TestDecodeMidea, FailToDecodeNonMideaExample) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  // Modified a few entries to unexpected values, based on previous test case.
  uint16_t gc_test[39] = {38000, 1, 1, 322, 162, 20, 61, 20, 61, 20, 20, 20, 20,
                          20, 20, 20, 127, 20, 61, 9, 20, 20, 61, 20, 20, 20,
                          61, 20, 61, 20, 61, 20, 20, 20, 20, 20, 20, 20, 884};
  irsend.sendGC(gc_test, 39);
  irsend.makeDecodeResult();

  ASSERT_FALSE(irrecv.decodeMidea(&irsend.capture));
  ASSERT_FALSE(irrecv.decodeMidea(&irsend.capture, MIDEA_BITS, false));
}

// Decode against a real capture reported by a user. See issue #354
TEST(TestDecodeMidea, DecodeRealExample) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  uint16_t rawData[199] = {
      4278, 4584, 488, 1668, 436, 588, 514, 1642, 564, 538, 488, 618, 438, 690,
      360, 662, 410, 1718, 442, 610, 488, 666, 356, 640, 462, 666, 694, 356,
      466, 616, 460, 1708, 448, 642, 412, 662, 518, 1582, 522, 1666, 440, 1690,
      490, 1692, 464, 636, 442, 558, 540, 618, 410, 1662, 466, 1746, 462, 1666,
      490, 1748, 544, 1528, 490, 1666, 562, 1644, 468, 1642, 406, 1744, 486,
      1710, 448, 1746, 414, 1586, 548, 1708, 464, 1736, 494, 1616, 544, 1586,
      466, 1742, 416, 1662, 518, 1662, 508, 1644, 692, 1464, 516, 666, 528,
      472, 492, 584, 462, 5264, 4296, 4566, 466, 614, 468, 1738, 436, 588, 494,
      1642, 508, 1672, 460, 1694, 490, 1664, 538, 486, 490, 1690, 518, 1690,
      464, 1662, 490, 1692, 414, 1770, 410, 1662, 518, 590, 512, 1586, 516,
      1722, 388, 638, 514, 536, 632, 516, 516, 516, 492, 1672, 500, 1694, 464,
      1666, 518, 534, 468, 636, 380, 712, 478, 528, 598, 534, 466, 638, 458,
      592, 410, 668, 462, 642, 404, 644, 490, 612, 412, 612, 542, 562, 466, 614,
      460, 584, 468, 638, 466, 588, 488, 638, 384, 618, 512, 564, 490, 612, 542,
      1638, 488, 1668, 438, 1740, 492};
  irsend.makeDecodeResult();
  irsend.sendRaw(rawData, 199, 38000);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MIDEA, irsend.capture.decode_type);
  EXPECT_EQ(MIDEA_BITS, irsend.capture.bits);
  EXPECT_EQ(0xA10278FFFFF8, irsend.capture.value);
}
