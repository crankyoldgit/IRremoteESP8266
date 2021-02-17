// Copyright 2021 David Conran

#include "IRac.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

// Tests for sendXmp().

// Test sending typical data only.
TEST(TestSendXmp, SendDataOnly) {
  IRsendTest irsend(kGpioUnused);
  irsend.begin();
  irsend.sendXmp(0x170F443E1C002100);  // Up.
  EXPECT_EQ(
      "f38000d50"
      "m210s895m210s1705m210s760m210s2785m210s1300m210s1300m210s1165m210s2650"
      "m210s13000"
      "m210s895m210s2380m210s760m210s760m210s1030m210s895m210s760m210s760"
      "m210s80400",
      irsend.outputStr());
}

// Tests for decodeXmp().

TEST(TestDecodeXmp, SyntheticSelfDecode) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  // Real-life Xmp code from an actual capture/decode.
  irsend.reset();
  irsend.sendXmp(0x170F443E1C002100);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(decode_type_t::XMP, irsend.capture.decode_type);
  EXPECT_EQ(kXmpBits, irsend.capture.bits);
  EXPECT_EQ(0x170F443E1C002100, irsend.capture.value);
  EXPECT_EQ(0, irsend.capture.address);
  EXPECT_EQ(0, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);
}

// Decode a real Xmp message.
TEST(TestDecodeXmp, RealMessageDecode) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  // Real-life XMP code from an actual capture/decode.
  // Ref: https://github.com/crankyoldgit/IRremoteESP8266/issues/1414#issuecomment-780289958
  irsend.reset();
  const uint16_t up[35] = {
      217, 887, 213, 1702, 213, 759, 215, 2807, 208, 1310, 214, 1310, 212, 1180,
      211, 2675, 188, 13014,
      180, 923, 187, 2438, 180, 794, 183, 786, 185, 1054, 179, 918, 187, 784,
      180, 793, 184};
  irsend.sendRaw(up, 35, 38000);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(decode_type_t::XMP, irsend.capture.decode_type);
  EXPECT_EQ(kXmpBits, irsend.capture.bits);
  EXPECT_EQ(0x170F443E1C002100, irsend.capture.value);
  EXPECT_EQ(0x0, irsend.capture.address);
  EXPECT_EQ(0x0, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);
}

TEST(TestUtils, Housekeeping) {
  ASSERT_EQ("XMP", typeToString(decode_type_t::XMP));
  ASSERT_EQ(decode_type_t::XMP, strToDecodeType("XMP"));
  ASSERT_FALSE(hasACState(decode_type_t::XMP));
  ASSERT_FALSE(IRac::isProtocolSupported(decode_type_t::XMP));
  ASSERT_EQ(kXmpBits, IRsendTest::defaultBits(decode_type_t::XMP));
  ASSERT_EQ(kNoRepeat, IRsendTest::minRepeats(decode_type_t::XMP));
}
