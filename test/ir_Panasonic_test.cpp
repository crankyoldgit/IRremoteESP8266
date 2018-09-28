// Copyright 2017, 2018 David Conran

#include "ir_Panasonic.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "IRutils.h"
#include "gtest/gtest.h"

// Tests for encodePanasonic().

TEST(TestEncodePanasonic, General) {
  IRsendTest irsend(4);
  EXPECT_EQ(0x0, irsend.encodePanasonic(0, 0, 0, 0));
  EXPECT_EQ(0x101010101, irsend.encodePanasonic(1, 1, 1, 1));
  EXPECT_EQ(0xFFFF, irsend.encodePanasonic(0, 0, 0, 0xFF));
  EXPECT_EQ(0xFF00FF, irsend.encodePanasonic(0, 0, 0xFF, 0));
  EXPECT_EQ(0xFF0000FF, irsend.encodePanasonic(0, 0xFF, 0, 0));
  EXPECT_EQ(0xFFFF00000000, irsend.encodePanasonic(0xFFFF, 0, 0, 0));
  EXPECT_EQ(0xFFFFFFFFFFFF, irsend.encodePanasonic(0xFFFF, 0xFF, 0xFF, 0xFF));
  EXPECT_EQ(0x40040190ED7C, irsend.encodePanasonic(0x4004, 0x01, 0x90, 0xED));
}

// Tests for sendPanasonic64().

// Test sending typical data only.
TEST(TestSendPanasonic64, SendDataOnly) {
  IRsendTest irsend(4);
  irsend.begin();

  irsend.reset();
  irsend.sendPanasonic64(0x0);
  EXPECT_EQ(
      "m3456s1728"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s116208", irsend.outputStr());

  irsend.reset();
  irsend.sendPanasonic64(0x40040190ED7C);
  EXPECT_EQ(
      "m3456s1728"
      "m432s432m432s1296m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s1296m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s1296"
      "m432s1296m432s432m432s432m432s1296m432s432m432s432m432s432m432s432"
      "m432s1296m432s1296m432s1296m432s432m432s1296m432s1296m432s432m432s1296"
      "m432s432m432s1296m432s1296m432s1296m432s1296m432s1296m432s432m432s432"
      "m432s102384", irsend.outputStr());

  irsend.reset();
  irsend.sendPanasonic64(0xFFFFFFFFFFFF);
  EXPECT_EQ(
      "m3456s1728"
      "m432s1296m432s1296m432s1296m432s1296m432s1296m432s1296m432s1296m432s1296"
      "m432s1296m432s1296m432s1296m432s1296m432s1296m432s1296m432s1296m432s1296"
      "m432s1296m432s1296m432s1296m432s1296m432s1296m432s1296m432s1296m432s1296"
      "m432s1296m432s1296m432s1296m432s1296m432s1296m432s1296m432s1296m432s1296"
      "m432s1296m432s1296m432s1296m432s1296m432s1296m432s1296m432s1296m432s1296"
      "m432s1296m432s1296m432s1296m432s1296m432s1296m432s1296m432s1296m432s1296"
      "m432s74736", irsend.outputStr());
}

// Test sending with different repeats.
TEST(TestSendPanasonic64, SendWithRepeats) {
  IRsendTest irsend(4);
  irsend.begin();

  irsend.reset();
  irsend.sendPanasonic64(0x40040190ED7C, kPanasonicBits, 0);  // 0 repeats.
  EXPECT_EQ(
      "m3456s1728"
      "m432s432m432s1296m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s1296m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s1296"
      "m432s1296m432s432m432s432m432s1296m432s432m432s432m432s432m432s432"
      "m432s1296m432s1296m432s1296m432s432m432s1296m432s1296m432s432m432s1296"
      "m432s432m432s1296m432s1296m432s1296m432s1296m432s1296m432s432m432s432"
      "m432s102384", irsend.outputStr());

  irsend.reset();
  irsend.sendPanasonic64(0x40040190ED7C, kPanasonicBits, 1);  // 1 repeat.
  EXPECT_EQ(
      "m3456s1728"
      "m432s432m432s1296m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s1296m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s1296"
      "m432s1296m432s432m432s432m432s1296m432s432m432s432m432s432m432s432"
      "m432s1296m432s1296m432s1296m432s432m432s1296m432s1296m432s432m432s1296"
      "m432s432m432s1296m432s1296m432s1296m432s1296m432s1296m432s432m432s432"
      "m432s102384"
      "m3456s1728"
      "m432s432m432s1296m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s1296m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s1296"
      "m432s1296m432s432m432s432m432s1296m432s432m432s432m432s432m432s432"
      "m432s1296m432s1296m432s1296m432s432m432s1296m432s1296m432s432m432s1296"
      "m432s432m432s1296m432s1296m432s1296m432s1296m432s1296m432s432m432s432"
      "m432s102384", irsend.outputStr());

  irsend.sendPanasonic64(0x40040190ED7C, kPanasonicBits, 2);  // 2 repeats.
  EXPECT_EQ(
      "m3456s1728"
      "m432s432m432s1296m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s1296m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s1296"
      "m432s1296m432s432m432s432m432s1296m432s432m432s432m432s432m432s432"
      "m432s1296m432s1296m432s1296m432s432m432s1296m432s1296m432s432m432s1296"
      "m432s432m432s1296m432s1296m432s1296m432s1296m432s1296m432s432m432s432"
      "m432s102384"
      "m3456s1728"
      "m432s432m432s1296m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s1296m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s1296"
      "m432s1296m432s432m432s432m432s1296m432s432m432s432m432s432m432s432"
      "m432s1296m432s1296m432s1296m432s432m432s1296m432s1296m432s432m432s1296"
      "m432s432m432s1296m432s1296m432s1296m432s1296m432s1296m432s432m432s432"
      "m432s102384"
      "m3456s1728"
      "m432s432m432s1296m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s1296m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s1296"
      "m432s1296m432s432m432s432m432s1296m432s432m432s432m432s432m432s432"
      "m432s1296m432s1296m432s1296m432s432m432s1296m432s1296m432s432m432s1296"
      "m432s432m432s1296m432s1296m432s1296m432s1296m432s1296m432s432m432s432"
      "m432s102384", irsend.outputStr());
}

// Test sending an atypical data size.
TEST(TestSendPanasonic64, SendUnusualSize) {
  IRsendTest irsend(4);
  irsend.begin();

  irsend.reset();
  irsend.sendPanasonic64(0x0, 8);
  EXPECT_EQ(
      "m3456s1728"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s150768", irsend.outputStr());

  irsend.reset();
  irsend.sendPanasonic64(0x1234567890ABCDEF, 64);
  EXPECT_EQ(
      "m3456s1728"
      "m432s432m432s432m432s432m432s1296m432s432m432s432m432s1296m432s432"
      "m432s432m432s432m432s1296m432s1296m432s432m432s1296m432s432m432s432"
      "m432s432m432s1296m432s432m432s1296m432s432m432s1296m432s1296m432s432"
      "m432s432m432s1296m432s1296m432s1296m432s1296m432s432m432s432m432s432"
      "m432s1296m432s432m432s432m432s1296m432s432m432s432m432s432m432s432"
      "m432s1296m432s432m432s1296m432s432m432s1296m432s432m432s1296m432s1296"
      "m432s1296m432s1296m432s432m432s432m432s1296m432s1296m432s432m432s1296"
      "m432s1296m432s1296m432s1296m432s432m432s1296m432s1296m432s1296m432s1296"
      "m432s74736", irsend.outputStr());
}

// Tests for sendPanasonic().

TEST(TestSendPanasonic, CompareToSendPanasonic64) {
  IRsendTest panasonic(4);
  IRsendTest panasonic64(0);

  panasonic.begin();
  panasonic64.begin();

  panasonic.reset();
  panasonic64.reset();

  panasonic.sendPanasonic(0x4004, 0x0190ED7C);
  panasonic64.sendPanasonic64(0x40040190ED7C);
  EXPECT_EQ(panasonic64.outputStr(), panasonic.outputStr());

  panasonic.sendPanasonic(0x0, 0x0);
  panasonic64.sendPanasonic64(0x0);
  EXPECT_EQ(panasonic64.outputStr(), panasonic.outputStr());

  panasonic.sendPanasonic(0x0, 0x0, 8);
  panasonic64.sendPanasonic64(0x0, 8);
  EXPECT_EQ(panasonic64.outputStr(), panasonic.outputStr());

  panasonic.sendPanasonic(0x1234, 0x567890AB, 64);
  panasonic64.sendPanasonic64(0x1234567890AB, 64);
  EXPECT_EQ(panasonic64.outputStr(), panasonic.outputStr());

  panasonic.sendPanasonic(0x1234, 0x567890AB, kPanasonicBits, 2);
  panasonic64.sendPanasonic64(0x1234567890AB, kPanasonicBits, 2);
  EXPECT_EQ(panasonic64.outputStr(), panasonic.outputStr());
}

// Tests for decodePanasonic().

// Decode normal Panasonic messages.
TEST(TestDecodePanasonic, NormalDecodeWithStrict) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  // Normal Panasonic 48-bit message.
  irsend.reset();
  irsend.sendPanasonic64(0x40040190ED7C);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodePanasonic(&irsend.capture, kPanasonicBits, true));
  EXPECT_EQ(PANASONIC, irsend.capture.decode_type);
  EXPECT_EQ(kPanasonicBits, irsend.capture.bits);
  EXPECT_EQ(0x40040190ED7C, irsend.capture.value);
  EXPECT_EQ(0x4004, irsend.capture.address);
  EXPECT_EQ(0x0190ED7C, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);

  // Synthesised Normal Panasonic 48-bit message.
  irsend.reset();
  irsend.sendPanasonic64(irsend.encodePanasonic(0x4004, 0x12, 0x34, 0x56));
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodePanasonic(&irsend.capture, kPanasonicBits, true));
  EXPECT_EQ(PANASONIC, irsend.capture.decode_type);
  EXPECT_EQ(kPanasonicBits, irsend.capture.bits);
  EXPECT_EQ(0x400412345670, irsend.capture.value);
  EXPECT_EQ(0x4004, irsend.capture.address);
  EXPECT_EQ(0x12345670, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);

  // Synthesised Normal Panasonic 48-bit message.
  irsend.reset();
  irsend.sendPanasonic64(irsend.encodePanasonic(0x4004, 0x1, 0x1, 0x1));
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodePanasonic(&irsend.capture, kPanasonicBits, true));
  EXPECT_EQ(PANASONIC, irsend.capture.decode_type);
  EXPECT_EQ(kPanasonicBits, irsend.capture.bits);
  EXPECT_EQ(0x400401010101, irsend.capture.value);
  EXPECT_EQ(0x4004, irsend.capture.address);
  EXPECT_EQ(0x1010101, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);
}

// Decode normal repeated Panasonic messages.
TEST(TestDecodePanasonic, NormalDecodeWithRepeatAndStrict) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  // Normal Panasonic 48-bit message with 2 repeats.
  irsend.reset();
  irsend.sendPanasonic64(0x40040190ED7C, kPanasonicBits, 2);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodePanasonic(&irsend.capture, kPanasonicBits, true));
  EXPECT_EQ(PANASONIC, irsend.capture.decode_type);
  EXPECT_EQ(kPanasonicBits, irsend.capture.bits);
  EXPECT_EQ(0x40040190ED7C, irsend.capture.value);
  EXPECT_EQ(0x4004, irsend.capture.address);
  EXPECT_EQ(0x190ED7C, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);

  irsend.makeDecodeResult(2 * kPanasonicBits + 4);
  ASSERT_TRUE(irrecv.decodePanasonic(&irsend.capture, kPanasonicBits, true));
  EXPECT_EQ(PANASONIC, irsend.capture.decode_type);
  EXPECT_EQ(kPanasonicBits, irsend.capture.bits);
  EXPECT_EQ(0x40040190ED7C, irsend.capture.value);

  irsend.makeDecodeResult(2 * (2 * kPanasonicBits + 4));
  ASSERT_TRUE(irrecv.decodePanasonic(&irsend.capture, kPanasonicBits, true));
  EXPECT_EQ(PANASONIC, irsend.capture.decode_type);
  EXPECT_EQ(kPanasonicBits, irsend.capture.bits);
  EXPECT_EQ(0x40040190ED7C, irsend.capture.value);
}

// Decode Panasonic messages with unsupported values.
TEST(TestDecodePanasonic, DecodeWithNonStrictValues) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  irsend.sendPanasonic64(0x0);  // Illegal value Panasonic 48-bit message.
  irsend.makeDecodeResult();
  // Should fail with strict on.
  ASSERT_FALSE(irrecv.decodePanasonic(&irsend.capture, kPanasonicBits, true));
  // Should pass if strict off.
  ASSERT_TRUE(irrecv.decodePanasonic(&irsend.capture, kPanasonicBits, false));
  EXPECT_EQ(PANASONIC, irsend.capture.decode_type);
  EXPECT_EQ(kPanasonicBits, irsend.capture.bits);
  EXPECT_EQ(0x0, irsend.capture.value);
  EXPECT_EQ(0x0, irsend.capture.address);
  EXPECT_EQ(0x0, irsend.capture.command);

  irsend.reset();
  // Illegal address/Manufacturer code. The rest is legal.
  irsend.sendPanasonic64(irsend.encodePanasonic(0, 1, 2, 3));
  irsend.makeDecodeResult();
  // Should fail with strict on.
  ASSERT_FALSE(irrecv.decodePanasonic(&irsend.capture, kPanasonicBits, true));
  // Should pass if strict off.
  ASSERT_TRUE(irrecv.decodePanasonic(&irsend.capture, kPanasonicBits, false));
  EXPECT_EQ(PANASONIC, irsend.capture.decode_type);
  EXPECT_EQ(kPanasonicBits, irsend.capture.bits);
  EXPECT_EQ(0x1020300, irsend.capture.value);
  EXPECT_EQ(0x0, irsend.capture.address);
  EXPECT_EQ(0x1020300, irsend.capture.command);
}

// Decode Panasonic messages with unsupported size/lengths.
TEST(TestDecodePanasonic, DecodeWithNonStrictSize) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  irsend.sendPanasonic64(0x12345678, 32);  // Illegal size Panasonic message.
  irsend.makeDecodeResult();

  ASSERT_FALSE(irrecv.decodePanasonic(&irsend.capture, kPanasonicBits, true));

  irsend.makeDecodeResult();
  // Should fail with strict when we ask for the wrong bit size.
  ASSERT_FALSE(irrecv.decodePanasonic(&irsend.capture, 32, true));
  // Should pass if strict off.
  ASSERT_TRUE(irrecv.decodePanasonic(&irsend.capture, 32, false));
  EXPECT_EQ(PANASONIC, irsend.capture.decode_type);
  EXPECT_EQ(32, irsend.capture.bits);
  EXPECT_EQ(0x12345678, irsend.capture.value);
  EXPECT_EQ(0x0, irsend.capture.address);
  EXPECT_EQ(0x12345678, irsend.capture.command);

  // Illegal over length (56-bit) message.
  irsend.reset();
  irsend.sendPanasonic64(irsend.encodePanasonic(0x4004, 1, 2, 3), 56);
  irsend.makeDecodeResult();

  ASSERT_FALSE(irrecv.decodePanasonic(&irsend.capture, kPanasonicBits, true));
  // Shouldn't pass if strict off and wrong bit size.
  ASSERT_FALSE(irrecv.decodePanasonic(&irsend.capture, kPanasonicBits, false));
  // Re-decode with correct bit size.
  ASSERT_FALSE(irrecv.decodePanasonic(&irsend.capture, 56, true));
  ASSERT_TRUE(irrecv.decodePanasonic(&irsend.capture, 56, false));
  EXPECT_EQ(PANASONIC, irsend.capture.decode_type);
  EXPECT_EQ(56, irsend.capture.bits);
  EXPECT_EQ(0x400401020300, irsend.capture.value);
  EXPECT_EQ(0x4004, irsend.capture.address);
  EXPECT_EQ(0x01020300, irsend.capture.command);
}

// Decode (non-standard) 64-bit messages.
TEST(TestDecodePanasonic, Decode64BitMessages) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  // Illegal value & size Panasonic 64-bit message.
  irsend.sendPanasonic64(0xFFFFFFFFFFFFFFFF, 64);
  irsend.makeDecodeResult();
  ASSERT_FALSE(irrecv.decodePanasonic(&irsend.capture, 64, true));
  // Should work with a 'normal' match (not strict)
  ASSERT_TRUE(irrecv.decodePanasonic(&irsend.capture, 64, false));
  EXPECT_EQ(PANASONIC, irsend.capture.decode_type);
  EXPECT_EQ(64, irsend.capture.bits);
  EXPECT_EQ(0xFFFFFFFFFFFFFFFF, irsend.capture.value);
  EXPECT_EQ(0xFFFFFFFF, irsend.capture.address);
  EXPECT_EQ(0xFFFFFFFF, irsend.capture.command);
}

// Decode a 'real' example via GlobalCache
TEST(TestDecodePanasonic, DecodeGlobalCacheExample) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  // Panasonic code from Global Cache.
  uint16_t gc_test[103] = {37000, 1, 1, 126, 64, 16, 17, 16, 49, 15, 16, 16, 16,
                          16, 16, 16, 17, 15, 17, 15, 17, 15, 17, 15, 16, 16,
                          16, 16, 16, 16, 17, 15, 49, 16, 16, 16, 16, 16, 17,
                          15, 17, 15, 17, 15, 17, 15, 16, 16, 16, 16, 16, 16,
                          49, 15, 49, 16, 17, 15, 17, 15, 49, 16, 16, 16, 17,
                          16, 17, 15, 17, 15, 49, 16, 49, 15, 49, 16, 17, 16,
                          49, 15, 49, 16, 17, 15, 48, 16, 16, 16, 49, 15, 48,
                          16, 49, 15, 49, 16, 49, 15, 17, 15, 16, 16, 2721};
  irsend.sendGC(gc_test, 103);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decodePanasonic(&irsend.capture, kPanasonicBits, true));
  EXPECT_EQ(PANASONIC, irsend.capture.decode_type);
  EXPECT_EQ(kPanasonicBits, irsend.capture.bits);
  EXPECT_EQ(0x40040190ED7C, irsend.capture.value);
  EXPECT_EQ(0x4004, irsend.capture.address);
  EXPECT_EQ(0x0190ED7C, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);

  ASSERT_TRUE(irrecv.decodePanasonic(&irsend.capture));
  EXPECT_EQ(PANASONIC, irsend.capture.decode_type);
  EXPECT_EQ(kPanasonicBits, irsend.capture.bits);
  EXPECT_EQ(0x40040190ED7C, irsend.capture.value);
  EXPECT_EQ(0x4004, irsend.capture.address);
  EXPECT_EQ(0x0190ED7C, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);
}

// Fail to decode a non-Panasonic example via GlobalCache
TEST(TestDecodePanasonic, FailToDecodeNonPanasonicExample) {
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

  ASSERT_FALSE(irrecv.decodePanasonic(&irsend.capture));
  ASSERT_FALSE(irrecv.decodePanasonic(&irsend.capture, kPanasonicBits, false));
}

// Failing to decode Panasonic in Issue #245
TEST(TestDecodePanasonic, DecodeIssue245) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();

  uint16_t rawData[100] = {3550, 1750, 500, 450, 500, 1300, 500, 450, 500, 450,
                          500, 450, 500, 450, 500, 450, 500, 450, 500, 450,
                          500, 450, 500, 450, 500, 450, 500, 450, 500, 1300,
                          500, 450, 500, 450, 500, 450, 500, 450, 500, 450,
                          500, 450, 500, 450, 500, 450, 500, 450, 500, 1300,
                          500, 450, 500, 450, 500, 450, 500, 450, 500, 450,
                          500, 450, 500, 450, 500, 450, 500, 1300, 500, 450,
                          500, 1300, 500, 1300, 500, 1300, 500, 1300, 500, 450,
                          500, 450, 500, 1300, 500, 450, 500, 1300, 500, 1300,
                          500, 1300, 500, 1300, 500, 450, 500, 1300, 500, 5000};

  irsend.sendRaw(rawData, 100, 37);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decodePanasonic(&irsend.capture));
  EXPECT_EQ(PANASONIC, irsend.capture.decode_type);
  EXPECT_EQ(kPanasonicBits, irsend.capture.bits);
  EXPECT_EQ(0x40040100BCBD, irsend.capture.value);
  EXPECT_EQ(0x4004, irsend.capture.address);
  EXPECT_EQ(0x100BCBD, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);

  irsend.reset();
  irsend.sendRaw(rawData, 99, 37);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(PANASONIC, irsend.capture.decode_type);
  EXPECT_EQ(kPanasonicBits, irsend.capture.bits);
  EXPECT_EQ(0x40040100BCBD, irsend.capture.value);
  EXPECT_EQ(0x4004, irsend.capture.address);
  EXPECT_EQ(0x100BCBD, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);
}

// Tests for sendPanasonicAC().

// Test sending typical data only.
TEST(TestSendPanasonicAC, SendDataOnly) {
  IRsendTest irsend(0);
  irsend.begin();

  irsend.reset();

  uint8_t state[kPanasonicAcStateLength] = {
       0x02, 0x20, 0xE0, 0x04, 0x00, 0x00, 0x00, 0x06,
       0x02, 0x20, 0xE0, 0x04, 0x00, 0x30, 0x32, 0x80, 0xAF, 0x00,
       0x00, 0x06, 0x60, 0x00, 0x00, 0x80, 0x00, 0x06, 0x83};
  irsend.sendPanasonicAC(state);
  EXPECT_EQ(
      "m3456s1728"
      "m432s432m432s1296m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s1296m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s1296m432s1296m432s1296"
      "m432s432m432s432m432s1296m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s1296m432s1296m432s432m432s432m432s432m432s432m432s432"
      "m432s10000"
      "m3456s1728"
      "m432s432m432s1296m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s1296m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s1296m432s1296m432s1296"
      "m432s432m432s432m432s1296m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s1296m432s1296m432s432m432s432"
      "m432s432m432s1296m432s432m432s432m432s1296m432s1296m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s1296"
      "m432s1296m432s1296m432s1296m432s1296m432s432m432s1296m432s432m432s1296"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s1296m432s1296m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s1296m432s1296m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s1296"
      "m432s432m432s432m432s432m432s432m432s432m432s432m432s432m432s432"
      "m432s432m432s1296m432s1296m432s432m432s432m432s432m432s432m432s432"
      "m432s1296m432s1296m432s432m432s432m432s432m432s432m432s432m432s1296"
      "m432s100000", irsend.outputStr());
}

// Tests for the IRPanasonicAc class.

TEST(TestIRPanasonicAcClass, ChecksumCalculation) {
  IRPanasonicAc pana(0);

  const uint8_t originalstate[kPanasonicAcStateLength] = {
      0x02, 0x20, 0xE0, 0x04, 0x00, 0x00, 0x00, 0x06,
      0x02, 0x20, 0xE0, 0x04, 0x00, 0x30, 0x32, 0x80, 0xAF, 0x00,
      0x00, 0x06, 0x60, 0x00, 0x00, 0x80, 0x00, 0x06, 0x83};
  uint8_t examplestate[kPanasonicAcStateLength] = {
      0x02, 0x20, 0xE0, 0x04, 0x00, 0x00, 0x00, 0x06,
      0x02, 0x20, 0xE0, 0x04, 0x00, 0x30, 0x32, 0x80, 0xAF, 0x00,
      0x00, 0x06, 0x60, 0x00, 0x00, 0x80, 0x00, 0x06, 0x83};

  EXPECT_TRUE(IRPanasonicAc::validChecksum(examplestate));
  EXPECT_EQ(0x83, IRPanasonicAc::calcChecksum(examplestate));

  examplestate[kPanasonicAcStateLength - 1] = 0x0;  // Set incoorect checksum.
  EXPECT_FALSE(IRPanasonicAc::validChecksum(examplestate));
  EXPECT_EQ(0x83, IRPanasonicAc::calcChecksum(examplestate));
  pana.setRaw(examplestate);
  // Extracting the state from the object should have a correct checksum.
  EXPECT_TRUE(IRPanasonicAc::validChecksum(pana.getRaw()));
  EXPECT_STATE_EQ(originalstate, pana.getRaw(), kPanasonicAcBits);
  examplestate[kPanasonicAcStateLength - 1] = 0x83;  // Restore old checksum.

  // Change the state to force a different checksum.
  examplestate[6] = 0x01;  // Should increase checksum by 1.
  EXPECT_FALSE(IRPanasonicAc::validChecksum(examplestate));
  EXPECT_EQ(0x84, IRPanasonicAc::calcChecksum(examplestate));
}

TEST(TestIRPanasonicAcClass, SetAndGetPower) {
  IRPanasonicAc pana(0);
  pana.on();
  EXPECT_TRUE(pana.getPower());
  pana.off();
  EXPECT_FALSE(pana.getPower());
  pana.setPower(true);
  EXPECT_TRUE(pana.getPower());
  pana.setPower(false);
  EXPECT_FALSE(pana.getPower());
}

TEST(TestIRPanasonicAcClass, SetAndGetModel) {
  IRPanasonicAc pana(0);
  EXPECT_EQ(kPanasonicJke, pana.getModel());
  pana.setModel(kPanasonicDke);
  EXPECT_EQ(kPanasonicDke, pana.getModel());
  pana.setModel(kPanasonicLke);
  EXPECT_EQ(kPanasonicLke, pana.getModel());
  pana.setModel(kPanasonicNke);
  EXPECT_EQ(kPanasonicNke, pana.getModel());
  pana.setModel(kPanasonicUnknown);  // shouldn't change.
  EXPECT_EQ(kPanasonicNke, pana.getModel());
  pana.setModel((panasonic_ac_remote_model_t) 255);  // shouldn't change.
  EXPECT_EQ(kPanasonicNke, pana.getModel());
  pana.setModel(kPanasonicJke);
  EXPECT_EQ(kPanasonicJke, pana.getModel());
}

TEST(TestIRPanasonicAcClass, SetAndGetMode) {
  IRPanasonicAc pana(0);
  pana.setMode(kPanasonicAcCool);
  EXPECT_EQ(kPanasonicAcCool, pana.getMode());
  pana.setMode(kPanasonicAcHeat);
  EXPECT_EQ(kPanasonicAcHeat, pana.getMode());
  pana.setMode(kPanasonicAcAuto);
  EXPECT_EQ(kPanasonicAcAuto, pana.getMode());
  pana.setMode(kPanasonicAcDry);
  EXPECT_EQ(kPanasonicAcDry, pana.getMode());
  pana.setMode(kPanasonicAcCool);
  EXPECT_EQ(kPanasonicAcCool, pana.getMode());
}

TEST(TestIRPanasonicAcClass, SetAndGetTemp) {
  IRPanasonicAc pana(0);
  pana.setTemp(25);
  EXPECT_EQ(25, pana.getTemp());
  pana.setTemp(kPanasonicAcMinTemp);
  EXPECT_EQ(kPanasonicAcMinTemp, pana.getTemp());
  pana.setTemp(kPanasonicAcMinTemp - 1);
  EXPECT_EQ(kPanasonicAcMinTemp, pana.getTemp());
  pana.setTemp(kPanasonicAcMaxTemp);
  EXPECT_EQ(kPanasonicAcMaxTemp, pana.getTemp());
  pana.setTemp(kPanasonicAcMaxTemp + 1);
  EXPECT_EQ(kPanasonicAcMaxTemp, pana.getTemp());
}

TEST(TestIRPanasonicAcClass, SetAndGetFan) {
  IRPanasonicAc pana(0);
  pana.setFan(kPanasonicAcFanAuto);
  EXPECT_EQ(kPanasonicAcFanAuto, pana.getFan());
  pana.setFan(kPanasonicAcFanMin);
  EXPECT_EQ(kPanasonicAcFanMin, pana.getFan());
  pana.setFan(kPanasonicAcFanMin - 1);
  EXPECT_EQ(kPanasonicAcFanMin, pana.getFan());
  pana.setFan(kPanasonicAcFanMin + 1);
  EXPECT_EQ(kPanasonicAcFanMin + 1, pana.getFan());
  pana.setFan(kPanasonicAcFanMax);
  EXPECT_EQ(kPanasonicAcFanMax, pana.getFan());
  pana.setFan(kPanasonicAcFanMax + 1);
  EXPECT_EQ(kPanasonicAcFanMax, pana.getFan());
  pana.setFan(kPanasonicAcFanMax - 1);
  EXPECT_EQ(kPanasonicAcFanMax - 1, pana.getFan());
}

TEST(TestIRPanasonicAcClass, SetAndGetSwings) {
  IRPanasonicAc pana(0);

  // Vertical
  pana.setSwingV(kPanasonicAcSwingVAuto);
  EXPECT_EQ(kPanasonicAcSwingVAuto, pana.getSwingVertical());

  pana.setSwingV(kPanasonicAcSwingVUp);
  EXPECT_EQ(kPanasonicAcSwingVUp, pana.getSwingVertical());
  pana.setSwingV(kPanasonicAcSwingVUp - 1);
  EXPECT_EQ(kPanasonicAcSwingVUp, pana.getSwingVertical());
  pana.setSwingV(kPanasonicAcSwingVUp + 1);
  EXPECT_EQ(kPanasonicAcSwingVUp + 1, pana.getSwingVertical());

  pana.setSwingV(kPanasonicAcSwingVDown);
  EXPECT_EQ(kPanasonicAcSwingVDown, pana.getSwingVertical());
  pana.setSwingV(kPanasonicAcSwingVDown + 1);
  EXPECT_EQ(kPanasonicAcSwingVDown, pana.getSwingVertical());
  pana.setSwingV(kPanasonicAcSwingVDown - 1);
  EXPECT_EQ(kPanasonicAcSwingVDown - 1, pana.getSwingVertical());

  pana.setSwingV(kPanasonicAcSwingVAuto);
  EXPECT_EQ(kPanasonicAcSwingVAuto, pana.getSwingVertical());

  // Horizontal is model dependant.
  pana.setModel(kPanasonicNke);  // NKE is always fixed in the middle.
  EXPECT_EQ(kPanasonicAcSwingHMiddle, pana.getSwingHorizontal());
  pana.setSwingH(kPanasonicAcSwingHAuto);
  EXPECT_EQ(kPanasonicAcSwingHMiddle, pana.getSwingHorizontal());

  pana.setModel(kPanasonicJke);  // JKE has no H swing.
  EXPECT_EQ(0, pana.getSwingHorizontal());
  pana.setSwingH(kPanasonicAcSwingHMiddle);
  EXPECT_EQ(0, pana.getSwingHorizontal());

  pana.setModel(kPanasonicLke);  // LKE is always fixed in the middle.
  EXPECT_EQ(kPanasonicAcSwingHMiddle, pana.getSwingHorizontal());
  pana.setSwingH(kPanasonicAcSwingHAuto);
  EXPECT_EQ(kPanasonicAcSwingHMiddle, pana.getSwingHorizontal());

  pana.setModel(kPanasonicDke);  // DKE has full control.
  ASSERT_EQ(kPanasonicDke, pana.getModel());
  // Auto was last requested.
  EXPECT_EQ(kPanasonicAcSwingHAuto, pana.getSwingHorizontal());
  pana.setSwingH(kPanasonicAcSwingHLeft);
  EXPECT_EQ(kPanasonicAcSwingHLeft, pana.getSwingHorizontal());
  // Changing models from DKE to something else, then back should not change
  // the intended swing.
  pana.setModel(kPanasonicLke);
  EXPECT_EQ(kPanasonicAcSwingHMiddle, pana.getSwingHorizontal());
  pana.setModel(kPanasonicDke);
  EXPECT_EQ(kPanasonicAcSwingHLeft, pana.getSwingHorizontal());
}

TEST(TestIRPanasonicAcClass, QuietAndPowerful) {
  IRPanasonicAc pana(0);
  pana.setQuiet(false);
  EXPECT_FALSE(pana.getQuiet());
  pana.setQuiet(true);
  EXPECT_TRUE(pana.getQuiet());
  EXPECT_FALSE(pana.getPowerful());
  pana.setPowerful(false);
  EXPECT_FALSE(pana.getPowerful());
  EXPECT_TRUE(pana.getQuiet());
  pana.setPowerful(true);
  EXPECT_TRUE(pana.getPowerful());
  EXPECT_FALSE(pana.getQuiet());
  pana.setPowerful(false);
  EXPECT_FALSE(pana.getPowerful());
  EXPECT_FALSE(pana.getQuiet());
  pana.setPowerful(true);
  pana.setQuiet(true);
  EXPECT_TRUE(pana.getQuiet());
  EXPECT_FALSE(pana.getPowerful());
}

TEST(TestIRPanasonicAcClass, HumanReadable) {
  IRPanasonicAc pana(0);
  EXPECT_EQ("Model: 4 (JKE), Power: Off, Mode: 0 (AUTO), Temp: 0C, "
            "Fan: 253 (UNKNOWN), Swing (Vertical): 0 (UNKNOWN), Quiet: Off, "
            "Powerful: Off",
            pana.toString());
  pana.setPower(true);
  pana.setTemp(kPanasonicAcMaxTemp);
  pana.setMode(kPanasonicAcHeat);
  pana.setFan(kPanasonicAcFanMax);
  pana.setSwingV(kPanasonicAcSwingVAuto);
  pana.setPowerful(true);
  EXPECT_EQ("Model: 4 (JKE), Power: On, Mode: 4 (HEAT), Temp: 30C, "
            "Fan: 4 (MAX), Swing (Vertical): 15 (AUTO), Quiet: Off, "
            "Powerful: On", pana.toString());
  pana.setQuiet(true);
  pana.setModel(kPanasonicLke);
  EXPECT_EQ("Model: 1 (LKE), Power: Off, Mode: 4 (HEAT), Temp: 30C, "
            "Fan: 4 (MAX), Swing (Vertical): 15 (AUTO), "
            "Swing (Horizontal): 6 (Middle), Quiet: On, Powerful: Off",
            pana.toString());
  pana.setModel(kPanasonicDke);
  pana.setSwingH(kPanasonicAcSwingHRight);
  EXPECT_EQ("Model: 3 (DKE), Power: Off, Mode: 4 (HEAT), Temp: 30C, "
            "Fan: 4 (MAX), Swing (Vertical): 15 (AUTO), "
            "Swing (Horizontal): 11 (Right), Quiet: On, Powerful: Off",
            pana.toString());
}

// Tests for decodePanasonicAC().

// Decode normal Panasonic AC messages.
TEST(TestDecodePanasonicAC, RealExample) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  // Data from Issue #525
  uint16_t rawData[439] = {3582, 1686, 488, 378, 488, 1238, 488, 378, 488, 378,
    488, 378, 488, 378, 488, 378, 488, 384, 488, 378, 488, 378, 488, 378, 488,
    378, 488, 378, 488, 1242, 486, 378, 488, 384, 488, 378, 488, 378, 488, 380,
    486, 382, 484, 382, 484, 1264, 464, 1266, 460, 1272, 462, 378, 488, 406,
    460, 1266, 462, 380, 488, 382, 484, 388, 478, 406, 462, 410, 462, 404, 462,
    406, 462, 396, 470, 406, 462, 404, 462, 406, 460, 404, 462, 410, 462, 404,
    462, 404, 462, 406, 464, 406, 462, 404, 462, 406, 462, 404, 462, 410, 462,
    404, 462, 406, 462, 404, 462, 404, 462, 404, 462, 406, 460, 406, 462, 410,
    462, 404, 462, 1264, 484, 1244, 486, 382, 482, 382, 486, 382, 486, 378, 486,
    382, 488, 9924, 3554, 1686, 488, 378, 490, 1240, 486, 378, 488, 378, 488,
    378, 488, 378, 488, 382, 484, 386, 486, 378, 488, 382, 486, 378, 488, 382,
    486, 382, 484, 1242, 486, 380, 488, 386, 484, 382, 486, 380, 486, 382, 486,
    380, 486, 380, 486, 1242, 486, 1242, 484, 1248, 484, 380, 488, 382, 484,
    1242, 486, 382, 484, 382, 484, 382, 484, 382, 486, 386, 484, 382, 486, 382,
    484, 382, 486, 382, 486, 380, 484, 382, 486, 382, 488, 380, 486, 382, 484,
    380, 462, 406, 488, 376, 484, 1246, 482, 1246, 460, 404, 480, 392, 484, 386,
    482, 1244, 484, 382, 484, 382, 484, 1242, 482, 1244, 484, 382, 464, 410,
    460, 404, 462, 406, 462, 404, 462, 404, 470, 396, 462, 406, 462, 404, 462,
    1286, 460, 1268, 458, 1268, 460, 1266, 460, 1266, 460, 406, 460, 1266, 462,
    406, 460, 1272, 462, 406, 460, 406, 460, 406, 460, 406, 462, 404, 462, 406,
    460, 406, 462, 410, 462, 404, 462, 406, 460, 406, 460, 406, 462, 404, 462,
    406, 460, 406, 460, 410, 462, 406, 460, 1268, 460, 1266, 460, 404, 460, 406,
    462, 406, 460, 406, 460, 412, 456, 410, 460, 410, 438, 428, 460, 410, 456,
    410, 456, 1272, 436, 1288, 438, 434, 438, 428, 438, 428, 438, 428, 438, 428,
    438, 428, 438, 428, 438, 428, 438, 434, 438, 428, 438, 428, 438, 428, 438,
    428, 438, 428, 440, 428, 438, 428, 438, 432, 438, 428, 438, 428, 438, 428,
    438, 428, 438, 428, 438, 428, 438, 430, 438, 1294, 438, 428, 438, 428, 438,
    428, 438, 428, 438, 428, 438, 428, 438, 428, 438, 434, 438, 428, 438, 1288,
    438, 1290, 438, 428, 438, 428, 438, 428, 438, 428, 438, 432, 438, 1288, 438,
    1290, 438, 430, 438, 428, 438, 428, 438, 428, 438, 428, 438, 1292, 438};
  uint8_t expectedState[kPanasonicAcStateLength] = {
       0x02, 0x20, 0xE0, 0x04, 0x00, 0x00, 0x00, 0x06,
       0x02, 0x20, 0xE0, 0x04, 0x00, 0x30, 0x32, 0x80, 0xAF, 0x00,
       0x00, 0x06, 0x60, 0x00, 0x00, 0x80, 0x00, 0x06, 0x83};

  irsend.sendRaw(rawData, 439, kPanasonicFreq);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(PANASONIC_AC, irsend.capture.decode_type);
  EXPECT_EQ(kPanasonicAcBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
  EXPECT_FALSE(irsend.capture.repeat);
}

// Decode synthetic Panasonic AC message.
TEST(TestDecodePanasonicAC, SyntheticExample) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  // Data from Issue #525
  uint8_t expectedState[kPanasonicAcStateLength] = {
       0x02, 0x20, 0xE0, 0x04, 0x00, 0x00, 0x00, 0x06,
       0x02, 0x20, 0xE0, 0x04, 0x00, 0x30, 0x32, 0x80, 0xAF, 0x00,
       0x00, 0x06, 0x60, 0x00, 0x00, 0x80, 0x00, 0x06, 0x83};

  irsend.sendPanasonicAC(expectedState);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(PANASONIC_AC, irsend.capture.decode_type);
  EXPECT_EQ(kPanasonicAcBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
  EXPECT_FALSE(irsend.capture.repeat);

  IRPanasonicAc pana(0);
  pana.setRaw(irsend.capture.state);
  EXPECT_EQ("Model: 4 (JKE), Power: Off, Mode: 3 (COOL), Temp: 25C, "
            "Fan: 7 (AUTO), Swing (Vertical): 15 (AUTO), Quiet: Off, "
            "Powerful: Off", pana.toString());
}

// Tests for general utility functions.
TEST(TestGeneralPanasonic, hasACState) {
  EXPECT_TRUE(hasACState(PANASONIC_AC));
  ASSERT_FALSE(hasACState(PANASONIC));
}

TEST(TestGeneralPanasonic, typeToString) {
  EXPECT_EQ("PANASONIC_AC", typeToString(PANASONIC_AC));
  EXPECT_EQ("PANASONIC", typeToString(PANASONIC));
}
