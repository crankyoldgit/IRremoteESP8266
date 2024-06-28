// Copyright 2024 Daniel Wallner

#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

// Tests for sendBangOlufsen().

// Test sending typical data only.
TEST(TestSendBangOlufsen, SendDataOnly) {
  IRsendTest irsend(4);
  irsend.begin();

  irsend.reset();
  irsend.sendBangOlufsen(0x1234);
  EXPECT_EQ(
      "f455000d50"
      "m200s2925m200s2925m200s15425"
      "m200s2925m200s6050m200s6050m200s6050m200s9175m200s2925"
      "m200s6050m200s9175m200s2925m200s6050m200s6050m200s9175"
      "m200s6050m200s2925m200s9175m200s2925m200s6050"
      "m200s12300m200",
      irsend.outputStr());

  // Example from Datalink '86 document
  // 22 bits are shown but the top bit is the start data bit which probably shouldn't be counted
  irsend.reset();
  irsend.sendBangOlufsen(0x083E35, 21);
  EXPECT_EQ(
      "f455000d50"
      "m200s2925m200s2925m200s15425" // START
      "m200s2925m200s6050m200s9175m200s2925" // Format (0)010
      "m200s6050m200s6050m200s6050m200s6050m200s9175" // Address (to) 00001
      "m200s6050m200s6050m200s6050m200s6050m200s2925" // Address (from) 11110
      "m200s6050m200s6050m200s9175m200s6050m200s2925m200s9175m200s2925m200s9175" // DATA 00110101
      "m200s12300m200", // STOP
      irsend.outputStr());

  // Test sending 1 as start data bit (which may be illegal but is supported)
  irsend.reset();
  irsend.sendBangOlufsen(0x15555);
  EXPECT_EQ(
      "f455000d50"
      "m200s2925m200s2925m200s15425"
      "m200s6050m200s2925m200s9175m200s2925m200s9175m200s2925"
      "m200s9175m200s2925m200s9175m200s2925m200s9175m200s2925"
      "m200s9175m200s2925m200s9175m200s2925m200s9175"
      "m200s12300m200",
      irsend.outputStr());

  // Test sending with datalink timing
  irsend.reset();
  irsend.sendBangOlufsenRaw(0xAB, 9, true, false);
  EXPECT_EQ(
      "f455000d100"
      "m1562s1563m1562s1563m1562s14063"
      "m1562s1563m1562s7813m1562s1563m1562s7813m1562s1563"
      "m1562s7813m1562s1563m1562s7813m1562s4688"
      "m1562s10938m1562",
      irsend.outputStr());
}

// Test sending typical data with extra repeats.
TEST(TestSendBangOlufsen, SendDataWithRepeats) {
  IRsendTest irsend(4);
  irsend.begin();

  irsend.reset();
  irsend.sendBangOlufsen(0x4321, 16, 2);
  EXPECT_EQ(
      "f455000d50"
      "m200s2925m200s2925m200s15425"
      "m200s2925m200s6050m200s9175m200s2925m200s6050m200s6050"
      "m200s6050m200s9175m200s6050m200s2925m200s6050m200s9175"
      "m200s2925m200s6050m200s6050m200s6050m200s9175"
      "m200s12300"
      "m200s2925m200s2925m200s15425"
      "m200s2925m200s6050m200s9175m200s2925m200s6050m200s6050"
      "m200s6050m200s9175m200s6050m200s2925m200s6050m200s9175"
      "m200s2925m200s6050m200s6050m200s6050m200s9175"
      "m200s12300"
      "m200s2925m200s2925m200s15425"
      "m200s2925m200s6050m200s9175m200s2925m200s6050m200s6050"
      "m200s6050m200s9175m200s6050m200s2925m200s6050m200s9175"
      "m200s2925m200s6050m200s6050m200s6050m200s9175"
      "m200s12300m200",
      irsend.outputStr());

  // Send different messages back to back.
  irsend.reset();
  irsend.sendBangOlufsenRaw(0x1234, 17, false, false);
  irsend.sendBangOlufsenRaw(0x4321, 17, false, true);
  EXPECT_EQ(
      "f455000d50"
      "m200s2925m200s2925m200s15425"
      "m200s2925m200s6050m200s6050m200s6050m200s9175m200s2925"
      "m200s6050m200s9175m200s2925m200s6050m200s6050m200s9175"
      "m200s6050m200s2925m200s9175m200s2925m200s6050"
      "m200s12300"
      "m200s2925m200s2925m200s15425"
      "m200s2925m200s6050m200s9175m200s2925m200s6050m200s6050"
      "m200s6050m200s9175m200s6050m200s2925m200s6050m200s9175"
      "m200s2925m200s6050m200s6050m200s6050m200s9175"
      "m200s12300m200",
      irsend.outputStr());
}

// Tests for decodeBangOlufsen().

// Decode normal messages.
TEST(TestDecodeBangOlufsen, NormalDecode) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  // Synthesised Normal 16-bit message.
  irsend.reset();
  irsend.sendBangOlufsen(0xF00F);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(BANG_OLUFSEN, irsend.capture.decode_type);
  EXPECT_EQ(kBangOlufsenBits, irsend.capture.bits);
  EXPECT_EQ(0xF00F, irsend.capture.value);
  EXPECT_EQ(0xF0, irsend.capture.address);
  EXPECT_EQ(0x0F, irsend.capture.command);

  // Synthesised Normal 32-bit message.
  irsend.reset();
  irsend.sendBangOlufsen(0xF2345678, 32);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(BANG_OLUFSEN, irsend.capture.decode_type);
  EXPECT_EQ(32, irsend.capture.bits);
  EXPECT_EQ(0xF2345678, irsend.capture.value);
  EXPECT_EQ(0xF23456, irsend.capture.address);
  EXPECT_EQ(0x78, irsend.capture.command);

  // Synthesised Normal 58-bit message which may be the longest ever being sent.
  irsend.reset();
  ASSERT_TRUE(irsend.send(BANG_OLUFSEN, 0x123456789ABCULL, 58));
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(BANG_OLUFSEN, irsend.capture.decode_type);
  EXPECT_EQ(58, irsend.capture.bits);
  EXPECT_EQ(0x123456789ABCULL, irsend.capture.value);

  // Synthesised Repeated 16-bit message.
  irsend.reset();
  irsend.sendBangOlufsenRaw(0x1234, 17, false, false);
  irsend.sendBangOlufsenRaw(0x4321, 17, false, true);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
#ifdef BANG_OLUFSEN_STRICT
  EXPECT_EQ(UNKNOWN, irsend.capture.decode_type);
#else
  EXPECT_EQ(BANG_OLUFSEN, irsend.capture.decode_type);
  EXPECT_EQ(kBangOlufsenBits, irsend.capture.bits);
  EXPECT_EQ(0x1234, irsend.capture.value);
  EXPECT_EQ(0x12, irsend.capture.address);
  EXPECT_EQ(0x34, irsend.capture.command);
#endif
}

// Decode unexpected messages. i.e 1 start data bit.
TEST(TestDecodeBangOlufsen, UnexpectedDecode) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  // Synthesised 16-bit message with extra start data bit.
  irsend.reset();
  irsend.sendBangOlufsen(0x3F00F);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(BANG_OLUFSEN, irsend.capture.decode_type);
  EXPECT_EQ(kBangOlufsenBits, irsend.capture.bits);
  EXPECT_EQ(0x1F00F, irsend.capture.value);
  EXPECT_EQ(0x1F0, irsend.capture.address);
  EXPECT_EQ(0x0F, irsend.capture.command);
}

// Check decoding of messages split at start and stop.
TEST(TestDecodeBangOlufsen, DecodeGlobalCacheExample) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  // AGC / START
  irsend.reset();
  uint16_t gc_test1[8] = {40000, 1,  1,  8, 117, 8, 117, 8};
  irsend.sendGC(gc_test1, 8);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
#ifdef BANG_OLUFSEN_STRICT
  EXPECT_EQ(UNKNOWN, irsend.capture.decode_type);
#else
  EXPECT_EQ(BANG_OLUFSEN, irsend.capture.decode_type);
  EXPECT_EQ(0, irsend.capture.bits);
  EXPECT_EQ(0, irsend.capture.value);
  EXPECT_EQ(0, irsend.capture.address);
  EXPECT_EQ(0, irsend.capture.command);
#endif

  // Lone STOP
  irsend.reset();
  uint16_t gc_test2[4] = {40000, 1,  1,  8};
  irsend.sendGC(gc_test2, 4);
  irsend.makeDecodeResult();

  ASSERT_FALSE(irrecv.decode(&irsend.capture));

  // Example from Datalink '86 document without start and stop
  irsend.reset();
  uint16_t gc_test3[48] = {40000, 1,  1,  8, 117, 8, 242, 8, 367, 8, 117,
                                        8, 242, 8, 242, 8, 242, 8, 242, 8, 367,
                                        8, 242, 8, 242, 8, 242, 8, 242, 8, 117,
                                        8, 242, 8, 242, 8, 367, 8, 242, 8, 117, 8, 367, 8, 117, 8, 367,
                                        8};
  irsend.sendGC(gc_test3, 48);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
#ifdef BANG_OLUFSEN_STRICT
  EXPECT_EQ(UNKNOWN, irsend.capture.decode_type);
#else
  EXPECT_EQ(BANG_OLUFSEN, irsend.capture.decode_type);
  EXPECT_EQ(21, irsend.capture.bits);
  EXPECT_EQ(0x083E35, irsend.capture.value);
  EXPECT_EQ(0x083E, irsend.capture.address);
  EXPECT_EQ(0x35, irsend.capture.command);
#endif

  // Same as above except that the start data bit is 1
  // This does not currently decode as BANG_OLUFSEN since enabling this would make decoding split messages less robust
  // It should only be enabled if 1 start data bits are actually used
  irsend.reset();
  uint16_t gc_test4[48] = {40000, 1,  1,  8, 242, 8, 117, 8, 367, 8, 117,
                                        8, 242, 8, 242, 8, 242, 8, 242, 8, 367,
                                        8, 242, 8, 242, 8, 242, 8, 242, 8, 117,
                                        8, 242, 8, 242, 8, 367, 8, 242, 8, 117, 8, 367, 8, 117, 8, 367,
                                        8};
  irsend.sendGC(gc_test4, 48);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(UNKNOWN, irsend.capture.decode_type);
}
