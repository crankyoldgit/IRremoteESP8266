// Copyright 2017 David Conran

#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

// Tests for encodeRC5().

TEST(TestEncodeRC5, NormalEncoding) {
  IRsendTest irsend(4);
  EXPECT_EQ(0x0, irsend.encodeRC5(0, 0));
  EXPECT_EQ(0x800, irsend.encodeRC5(0, 0, true));
  EXPECT_EQ(0x41, irsend.encodeRC5(1, 1));
  EXPECT_EQ(0x42, irsend.encodeRC5(1, 2));
  EXPECT_EQ(0x7FF, irsend.encodeRC5(0x1F, 0x3F));
  EXPECT_EQ(0xFFF, irsend.encodeRC5(0x1F, 0x3F, true));
  EXPECT_EQ(0x7FF, irsend.encodeRC5(0xFF, 0xFF));
  EXPECT_EQ(0xFFF, irsend.encodeRC5(0xFF, 0xFF, true));

  EXPECT_EQ(0x175, irsend.encodeRC5(0x05, 0x35));
}

// Tests for encodeRC5X().
TEST(TestEncodeRC5X, NormalEncoding) {
  IRsendTest irsend(4);
  EXPECT_EQ(0x0, irsend.encodeRC5X(0, 0));
  EXPECT_EQ(0x800, irsend.encodeRC5X(0, 0, true));
  EXPECT_EQ(0x41, irsend.encodeRC5X(1, 1));
  EXPECT_EQ(0x42, irsend.encodeRC5X(1, 2));
  EXPECT_EQ(0x3FF, irsend.encodeRC5X(0x0F, 0x3F));
  EXPECT_EQ(0x3FF, irsend.encodeRC5X(0x0F, 0x3F, false));
  EXPECT_EQ(0xBFF, irsend.encodeRC5X(0x0F, 0x3F, true));
  EXPECT_EQ(0x17FF, irsend.encodeRC5X(0x1F, 0x7F));
  EXPECT_EQ(0x1FFF, irsend.encodeRC5X(0x1F, 0x7F, true));
  EXPECT_EQ(0x17FF, irsend.encodeRC5X(0xFF, 0xFF));
  EXPECT_EQ(0x1FFF, irsend.encodeRC5X(0xFF, 0xFF, true));

  EXPECT_EQ(0x175, irsend.encodeRC5X(0x05, 0x35));

  // Values of command <= 6-bits. i.e (<= 63 (0x3F)) should be the same
  // as encodeRC5.
  EXPECT_EQ(irsend.encodeRC5X(0, 0), irsend.encodeRC5(0, 0));
  EXPECT_EQ(irsend.encodeRC5X(0, 0, true), irsend.encodeRC5(0, 0, true));
  EXPECT_EQ(irsend.encodeRC5X(0x5, 0x35, false),
            irsend.encodeRC5(0x5, 0x35, false));
  EXPECT_EQ(irsend.encodeRC5X(0x5, 0x35, true),
            irsend.encodeRC5(0x5, 0x35, true));
  EXPECT_EQ(irsend.encodeRC5X(0x1F, 0x3F, true),
            irsend.encodeRC5(0x1F, 0x3F, true));
  EXPECT_NE(irsend.encodeRC5X(0x1F, 0x7F, true),
            irsend.encodeRC5(0x1F, 0x7F, true));
}

// Tests for sendRC5().

// Test sending typical RC-5 & RC-5X data only.
TEST(TestSendRC5, SendDataOnly) {
  IRsendTest irsend(4);
  irsend.begin();

  irsend.reset();
  irsend.sendRC5(0x0, RC5_BITS);
  EXPECT_EQ(
      "m889s889m1778s889m889s889m889s889m889s889m889s889m889"
      "s889m889s889m889s889m889s889m889s889m889s889m889s114667",
      irsend.outputStr());

  irsend.reset();
  irsend.sendRC5(0x1AAA, RC5_BITS);
  EXPECT_EQ(
      "m889s889m889s889m1778s1778m1778s1778m1778s1778"
      "m1778s1778m1778s1778m1778s114667", irsend.outputStr());

  irsend.reset();
  irsend.sendRC5(0x175, RC5_BITS);
  EXPECT_EQ(
      "m889s889m1778s889m889s889m889s1778m1778s1778"
      "m889s889m889s889m1778s1778m1778s1778m889s113778", irsend.outputStr());

  irsend.reset();
  irsend.sendRC5(0x3FFF, RC5_BITS);
  EXPECT_EQ(
      "m889s889m889s889m889s889m889s889m889s889m889s889m889s889"
      "m889s889m889s889m889s889m889s889m889s889m889s889m889s113778",
      irsend.outputStr());

  irsend.reset();
  irsend.sendRC5(0x0, RC5X_BITS);
  EXPECT_EQ(
      "m889s889m1778s889m889s889m889s889m889s889m889s889m889"
      "s889m889s889m889s889m889s889m889s889m889s889m889s114667",
      irsend.outputStr());

  irsend.reset();
  irsend.sendRC5(0x1AAA, RC5X_BITS);
  EXPECT_EQ(
      "m1778s1778m1778s1778m1778s1778m1778"
      "s1778m1778s1778m1778s1778m1778s114667", irsend.outputStr());

  irsend.reset();
  irsend.sendRC5(0x175, RC5X_BITS);
  EXPECT_EQ(
      "m889s889m1778s889m889s889m889s1778m1778s1778"
      "m889s889m889s889m1778s1778m1778s1778m889s113778", irsend.outputStr());

  irsend.reset();
  irsend.sendRC5(0x3FFF, RC5X_BITS);
  EXPECT_EQ(
      "m1778s1778m889s889m889s889m889s889m889s889m889s889m889"
      "s889m889s889m889s889m889s889m889s889m889s889m889s113778",
      irsend.outputStr());
}

// Test sending RC-5 & RC-5X with different repeats.
TEST(TestSendRC5, SendWithRepeats) {
  IRsendTest irsend(4);
  irsend.begin();

  irsend.reset();
  irsend.sendRC5(0x175, RC5_BITS, 1);
  EXPECT_EQ(
      "m889s889m1778s889m889s889m889s1778m1778s1778"
      "m889s889m889s889m1778s1778m1778s1778m889s114667"
      "m889s889m1778s889m889s889m889s1778m1778s1778"
      "m889s889m889s889m1778s1778m1778s1778m889s113778", irsend.outputStr());

  irsend.reset();
  irsend.sendRC5(0x175, RC5_BITS, 2);
  EXPECT_EQ(
      "m889s889m1778s889m889s889m889s1778m1778s1778"
      "m889s889m889s889m1778s1778m1778s1778m889s114667"
      "m889s889m1778s889m889s889m889s1778m1778s1778"
      "m889s889m889s889m1778s1778m1778s1778m889s114667"
      "m889s889m1778s889m889s889m889s1778m1778s1778"
      "m889s889m889s889m1778s1778m1778s1778m889s113778", irsend.outputStr());

  irsend.reset();
  irsend.sendRC5(0x175, RC5X_BITS, 1);
  EXPECT_EQ(
      "m889s889m1778s889m889s889m889s1778m1778s1778"
      "m889s889m889s889m1778s1778m1778s1778m889s114667"
      "m889s889m1778s889m889s889m889s1778m1778s1778"
      "m889s889m889s889m1778s1778m1778s1778m889s113778", irsend.outputStr());

  irsend.reset();
  irsend.sendRC5(0x1175, RC5X_BITS, 2);
  EXPECT_EQ(
      "m1778s889m889s889m889s889m889s1778m1778s1778"
      "m889s889m889s889m1778s1778m1778s1778m889s114667"
      "m1778s889m889s889m889s889m889s1778m1778s1778"
      "m889s889m889s889m1778s1778m1778s1778m889s114667"
      "m1778s889m889s889m889s889m889s1778m1778s1778"
      "m889s889m889s889m1778s1778m1778s1778m889s113778", irsend.outputStr());
}
// Tests for decodeRC5().

// Decode normal RC-5/RC5X messages.
TEST(TestDecodeRC5, NormalDecodeWithStrict) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  // Normal RC-5 12-bit message.
  irsend.reset();
  irsend.sendRC5(0x175);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeRC5(&irsend.capture, RC5_BITS, true));
  EXPECT_EQ(RC5, irsend.capture.decode_type);
  EXPECT_EQ(RC5_BITS, irsend.capture.bits);
  EXPECT_EQ(0x175, irsend.capture.value);
  EXPECT_EQ(0x05, irsend.capture.address);
  EXPECT_EQ(0x35, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);

  // Normal RC-5 12-bit message decoded as RC5-X.
  irsend.reset();
  irsend.sendRC5(0x175);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeRC5(&irsend.capture, RC5X_BITS, true));
  EXPECT_EQ(RC5, irsend.capture.decode_type);
  EXPECT_EQ(RC5_BITS, irsend.capture.bits);
  EXPECT_EQ(0x175, irsend.capture.value);
  EXPECT_EQ(0x05, irsend.capture.address);
  EXPECT_EQ(0x35, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);

  // A RC-5X 13-bit message but with a value that is valid for RC-5 decoded
  // as RC5-X.
  irsend.reset();
  irsend.sendRC5(0x175, RC5X_BITS);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeRC5(&irsend.capture, RC5X_BITS, true));
  EXPECT_EQ(RC5, irsend.capture.decode_type);
  EXPECT_EQ(RC5_BITS, irsend.capture.bits);
  EXPECT_EQ(0x175, irsend.capture.value);
  EXPECT_EQ(0x05, irsend.capture.address);
  EXPECT_EQ(0x35, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);

  // Synthesised Normal RC-5 12-bit message.
  irsend.reset();
  irsend.sendRC5(irsend.encodeRC5(0x00, 0x0B, true));
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeRC5(&irsend.capture, RC5_BITS, true));
  EXPECT_EQ(RC5, irsend.capture.decode_type);
  EXPECT_EQ(RC5_BITS, irsend.capture.bits);
  EXPECT_EQ(0x80B, irsend.capture.value);
  EXPECT_EQ(0x00, irsend.capture.address);
  EXPECT_EQ(0x0B, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);

  // Synthesised Normal RC-5X 13-bit message.
  irsend.reset();
  irsend.sendRC5(irsend.encodeRC5X(0x02, 0x41, true), RC5X_BITS);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeRC5(&irsend.capture, RC5X_BITS, true));
  EXPECT_EQ(RC5X, irsend.capture.decode_type);
  EXPECT_EQ(RC5X_BITS, irsend.capture.bits);
  EXPECT_EQ(0x1881, irsend.capture.value);
  EXPECT_EQ(0x02, irsend.capture.address);
  EXPECT_EQ(0x41, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);

  // Synthesised Normal RC-5X 13-bit message should fail at being decoded
  // as a normal RC-5 (12 bit) message.
  irsend.reset();
  irsend.sendRC5(irsend.encodeRC5X(0x02, 0x41, true), RC5X_BITS);
  irsend.makeDecodeResult();
  ASSERT_FALSE(irrecv.decodeRC5(&irsend.capture, RC5_BITS, true));
}

// Decode normal repeated RC5 messages.
TEST(TestDecodeRC5, NormalDecodeWithRepeatAndStrict) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  // Normal RC-5 12-bit message with one repeat.
  irsend.reset();
  irsend.sendRC5(0x175, RC5_BITS, 1);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeRC5(&irsend.capture, RC5_BITS, true));
  EXPECT_EQ(RC5, irsend.capture.decode_type);
  EXPECT_EQ(RC5_BITS, irsend.capture.bits);
  EXPECT_EQ(0x175, irsend.capture.value);
  EXPECT_EQ(0x05, irsend.capture.address);
  EXPECT_EQ(0x35, irsend.capture.command);

  // Synthesised Normal RC-5X 13-bit message with 2 repeats.
  irsend.reset();
  irsend.sendRC5(irsend.encodeRC5X(0x02, 0x41, true), RC5X_BITS, 2);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeRC5(&irsend.capture, RC5X_BITS, true));
  EXPECT_EQ(RC5X, irsend.capture.decode_type);
  EXPECT_EQ(RC5X_BITS, irsend.capture.bits);
  EXPECT_EQ(0x1881, irsend.capture.value);
  EXPECT_EQ(0x02, irsend.capture.address);
  EXPECT_EQ(0x41, irsend.capture.command);
}

// Decode unsupported RC5 messages.
TEST(TestDecodeRC5, DecodeWithNonStrictValues) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  irsend.sendRC5(0xFA, 8);  // Illegal value RC5 8-bit message.
  irsend.makeDecodeResult();
  // Should fail with strict on.
  ASSERT_FALSE(irrecv.decodeRC5(&irsend.capture, RC5_BITS, true));
  ASSERT_FALSE(irrecv.decodeRC5(&irsend.capture, RC5X_BITS, true));
  // Should pass if strict off.
  ASSERT_TRUE(irrecv.decodeRC5(&irsend.capture, 8, false));
  EXPECT_EQ(RC5, irsend.capture.decode_type);
  EXPECT_EQ(8, irsend.capture.bits);
  EXPECT_EQ(0xFA, irsend.capture.value);
  EXPECT_EQ(0x3, irsend.capture.address);
  EXPECT_EQ(0x3A, irsend.capture.command);

  irsend.reset();
  irsend.sendRC5(0x12345678, 32);  // Illegal size RC5 32-bit message.
  irsend.makeDecodeResult();
  // Should fail with strict on.
  ASSERT_FALSE(irrecv.decodeRC5(&irsend.capture, RC5_BITS, true));
  ASSERT_FALSE(irrecv.decodeRC5(&irsend.capture, RC5X_BITS, true));

  irsend.makeDecodeResult();
  // Should fail with strict when we ask for the wrong bit size.
  ASSERT_FALSE(irrecv.decodeRC5(&irsend.capture, 32, true));
  // Should pass if strict off.
  ASSERT_TRUE(irrecv.decodeRC5(&irsend.capture, 32, false));
  EXPECT_EQ(RC5, irsend.capture.decode_type);
  EXPECT_EQ(31, irsend.capture.bits);
  EXPECT_EQ(0x12345678, irsend.capture.value);

  irsend.reset();
  irsend.sendRC5(0x87654321, 32);  // Illegal size RC5 32-bit message.
  irsend.makeDecodeResult();
  // Should fail with strict on.
  ASSERT_FALSE(irrecv.decodeRC5(&irsend.capture, RC5_BITS, true));
  ASSERT_FALSE(irrecv.decodeRC5(&irsend.capture, RC5X_BITS, true));

  irsend.makeDecodeResult();
  // Should fail with strict when we ask for the wrong bit size.
  ASSERT_FALSE(irrecv.decodeRC5(&irsend.capture, 32, true));
  // Should pass if strict off.
  ASSERT_TRUE(irrecv.decodeRC5(&irsend.capture, 32, false));
  EXPECT_EQ(RC5X, irsend.capture.decode_type);
  EXPECT_EQ(32, irsend.capture.bits);
  EXPECT_EQ(0x87654321, irsend.capture.value);
}

// Decode (non-standard) 64-bit messages.
TEST(TestDecodeRC5, Decode64BitMessages) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  // Illegal size RC-5 64-bit message.
  irsend.sendRC5(0xFFFFFFFFFFFFFFFF, 64);
  irsend.makeDecodeResult();
  // Should work with a 'normal' match (not strict)
  ASSERT_TRUE(irrecv.decodeRC5(&irsend.capture, 64, false));
  EXPECT_EQ(RC5X, irsend.capture.decode_type);
  EXPECT_EQ(64, irsend.capture.bits);
  EXPECT_EQ(0xFFFFFFFFFFFFFFFF, irsend.capture.value);
}

// Fail to decode a non-RC-5 example via GlobalCache
TEST(TestDecodeRC5, FailToDecodeNonRC5Example) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  uint16_t gc_test[39] = {38000, 1, 1, 322, 162, 20, 61, 20, 61, 20, 20, 20, 20,
                          20, 20, 20, 127, 20, 61, 9, 20, 20, 61, 20, 20, 20,
                          61, 20, 61, 20, 61, 20, 20, 20, 20, 20, 20, 20, 884};
  irsend.sendGC(gc_test, 39);
  irsend.makeDecodeResult();

  ASSERT_FALSE(irrecv.decodeRC5(&irsend.capture));
  ASSERT_FALSE(irrecv.decodeRC5(&irsend.capture, RC5_BITS, false));
}
