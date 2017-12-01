// Copyright 2017 David Conran

#include "IRrecv.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "ir_Magiquest.h"
#include "gtest/gtest.h"

// Tests for encodeMagiQuest()
TEST(TestEncodeMagiQuest, General) {
  IRsendTest irsend(0);
  EXPECT_EQ(0x0, irsend.encodeMagiQuest(0x0, 0x0));
  EXPECT_EQ(0x10001, irsend.encodeMagiQuest(0x1, 0x1));
  EXPECT_EQ(0x20003, irsend.encodeMagiQuest(0x2, 0x3));
  EXPECT_EQ(0x123456781234, irsend.encodeMagiQuest(0x12345678, 0x1234));
  EXPECT_EQ(0xFFFFFFFFFFFF, irsend.encodeMagiQuest(0xFFFFFFFF, 0xFFFF));
}

// Tests for sendMagiQuest()

// Test sending typical data only.
TEST(TestSendMagiQuest, SendDataOnly) {
  IRsendTest irsend(0);
  irsend.begin();

  irsend.reset();
  irsend.sendMagiQuest(0x0);
  EXPECT_EQ(
      "m280s850m280s850m280s850m280s850m280s850m280s850m280s850m280s850"
      "m280s850m280s850m280s850m280s850m280s850m280s850m280s850m280s850"
      "m280s850m280s850m280s850m280s850m280s850m280s850m280s850m280s850"
      "m280s850m280s850m280s850m280s850m280s850m280s850m280s850m280s850"
      "m280s850m280s850m280s850m280s850m280s850m280s850m280s850m280s850"
      "m280s850m280s850m280s850m280s850m280s850m280s850m280s850m280s850"
      "m280s850m280s850m280s850m280s850m280s850m280s850m280s850m280s100850",
      irsend.outputStr());
  irsend.reset();
  irsend.sendMagiQuest(0x123456789ABC);
  EXPECT_EQ(
      "m280s850m280s850m280s850m280s850m280s850m280s850m280s850m280s850"
      "m280s850m280s850m280s850m580s600m280s850m280s850m580s600m280s850"
      "m280s850m280s850m580s600m580s600m280s850m580s600m280s850m280s850"
      "m280s850m580s600m280s850m580s600m280s850m580s600m580s600m280s850"
      "m280s850m580s600m580s600m580s600m580s600m280s850m280s850m280s850"
      "m580s600m280s850m280s850m580s600m580s600m280s850m580s600m280s850"
      "m580s600m280s850m580s600m580s600m580s600m580s600m280s850m280s100850",
      irsend.outputStr());
}

// Test sending typical data only.
TEST(TestSendMagiQuest, SendWithRepeats) {
  IRsendTest irsend(0);
  irsend.begin();

  irsend.reset();
  irsend.sendMagiQuest(0x12345678ABCD, MAGIQUEST_BITS, 2);  // two repeats.
  EXPECT_EQ(
      "m280s850m280s850m280s850m280s850m280s850m280s850m280s850m280s850"
      "m280s850m280s850m280s850m580s600m280s850m280s850m580s600m280s850"
      "m280s850m280s850m580s600m580s600m280s850m580s600m280s850m280s850"
      "m280s850m580s600m280s850m580s600m280s850m580s600m580s600m280s850"
      "m280s850m580s600m580s600m580s600m580s600m280s850m280s850m280s850"
      "m580s600m280s850m580s600m280s850m580s600m280s850m580s600m580s600"
      "m580s600m580s600m280s850m280s850m580s600m580s600m280s850m580s100600"
      "m280s850m280s850m280s850m280s850m280s850m280s850m280s850m280s850"
      "m280s850m280s850m280s850m580s600m280s850m280s850m580s600m280s850"
      "m280s850m280s850m580s600m580s600m280s850m580s600m280s850m280s850"
      "m280s850m580s600m280s850m580s600m280s850m580s600m580s600m280s850"
      "m280s850m580s600m580s600m580s600m580s600m280s850m280s850m280s850"
      "m580s600m280s850m580s600m280s850m580s600m280s850m580s600m580s600"
      "m580s600m580s600m280s850m280s850m580s600m580s600m280s850m580s100600"
      "m280s850m280s850m280s850m280s850m280s850m280s850m280s850m280s850"
      "m280s850m280s850m280s850m580s600m280s850m280s850m580s600m280s850"
      "m280s850m280s850m580s600m580s600m280s850m580s600m280s850m280s850"
      "m280s850m580s600m280s850m580s600m280s850m580s600m580s600m280s850"
      "m280s850m580s600m580s600m580s600m580s600m280s850m280s850m280s850"
      "m580s600m280s850m580s600m280s850m580s600m280s850m580s600m580s600"
      "m580s600m580s600m280s850m280s850m580s600m580s600m280s850m580s100600",
      irsend.outputStr());
}

// Tests for decodeMagiQuest().

// Decode normal "synthetic" messages.
TEST(TestDecodeMagiQuest, NormalDecodeWithStrict) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  irsend.reset();
  irsend.sendMagiQuest(0x0);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeMagiQuest(&irsend.capture, MAGIQUEST_BITS, true));
  EXPECT_EQ(MAGIQUEST, irsend.capture.decode_type);
  EXPECT_EQ(MAGIQUEST_BITS, irsend.capture.bits);
  EXPECT_EQ(0x0, irsend.capture.value);
  EXPECT_EQ(0x0, irsend.capture.address);
  EXPECT_EQ(0x0, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);

  irsend.reset();
  irsend.sendMagiQuest(irsend.encodeMagiQuest(0x1, 0x1));
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeMagiQuest(&irsend.capture, MAGIQUEST_BITS, true));
  EXPECT_EQ(MAGIQUEST, irsend.capture.decode_type);
  EXPECT_EQ(MAGIQUEST_BITS, irsend.capture.bits);
  EXPECT_EQ(0x10001, irsend.capture.value);
  EXPECT_EQ(0x1, irsend.capture.address);
  EXPECT_EQ(0x1, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);

  irsend.reset();
  irsend.sendMagiQuest(irsend.encodeMagiQuest(0x12345678, 0xABCD));
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeMagiQuest(&irsend.capture, MAGIQUEST_BITS, true));
  EXPECT_EQ(MAGIQUEST, irsend.capture.decode_type);
  EXPECT_EQ(MAGIQUEST_BITS, irsend.capture.bits);
  EXPECT_EQ(0x12345678ABCD, irsend.capture.value);
  EXPECT_EQ(0x12345678, irsend.capture.address);
  EXPECT_EQ(0xABCD, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);

  // Do the last one again, & use the full decoder, not just protocol specific.
  irsend.reset();
  irsend.sendMagiQuest(irsend.encodeMagiQuest(0x12345678, 0xABCD));
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MAGIQUEST, irsend.capture.decode_type);
  EXPECT_EQ(MAGIQUEST_BITS, irsend.capture.bits);
  EXPECT_EQ(0x12345678ABCD, irsend.capture.value);
}
