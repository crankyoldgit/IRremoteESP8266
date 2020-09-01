// Copyright 2020 crankyoldgit

#include "IRac.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

// Tests for decodeTranscold().

TEST(TestDecodeTranscold, RealExample) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  // Ref: https://github.com/crankyoldgit/IRremoteESP8266/issues/1256#issuecomment-683608224
  const uint16_t rawData[101] = {
      5944, 7612,
      558, 3556, 556, 3556, 556, 3556, 556, 1526, 554, 3556, 556, 1528,
      554, 1526, 556, 3558, 554, 1524, 556, 1528, 556, 1526, 556, 3556,
      554, 1528, 556, 3556, 554, 3556, 556, 1528, 554, 1526, 556, 3556,
      556, 3556, 554, 1528, 554, 1526, 554, 3558, 554, 1528, 554, 3556,
      556, 3556, 556, 1526, 554, 1526, 556, 3556, 554, 3556, 554, 1526,
      554, 3556, 556, 1526, 556, 1526, 554, 3558, 554, 1526, 556, 3556,
      556, 1526, 556, 3554, 556, 1524, 556, 1526, 556, 3556, 556, 1526,
      554, 3556, 556, 1524, 558, 3556, 554, 1526, 556, 3556, 554, 3556,
      556, 7514,
      556};  // UNKNOWN C38A8243

  irsend.begin();
  irsend.reset();
  irsend.sendRaw(rawData, 101, 38);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(decode_type_t::TRANSCOLD, irsend.capture.decode_type);
  ASSERT_EQ(kTranscoldBits, irsend.capture.bits);
  EXPECT_EQ(0xE916659A54AB, irsend.capture.value);
  EXPECT_EQ(0x0, irsend.capture.command);
  EXPECT_EQ(0x0, irsend.capture.address);
}

TEST(TestDecodeTranscold, SyntheticExample) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();
  irsend.reset();
  irsend.sendTranscold(0xE916659A54AB);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(decode_type_t::TRANSCOLD, irsend.capture.decode_type);
  EXPECT_EQ(kTranscoldBits, irsend.capture.bits);
  EXPECT_EQ(0xE916659A54AB, irsend.capture.value);
  EXPECT_EQ(0x0, irsend.capture.command);
  EXPECT_EQ(0x0, irsend.capture.address);

  EXPECT_EQ(
    "f38000d50"
    "m5944s7563"
    "m555s3556m555s3556m555s3556m555s1526m555s3556m555s1526m555s1526m555s3556"
    "m555s1526m555s1526m555s1526m555s3556m555s1526m555s3556m555s3556m555s1526"
    "m555s1526m555s3556m555s3556m555s1526m555s1526m555s3556m555s1526m555s3556"
    "m555s3556m555s1526m555s1526m555s3556m555s3556m555s1526m555s3556m555s1526"
    "m555s1526m555s3556m555s1526m555s3556m555s1526m555s3556m555s1526m555s1526"
    "m555s3556m555s1526m555s3556m555s1526m555s3556m555s1526m555s3556m555s3556"
    "m555s7563"
    "m555s100000",
    irsend.outputStr());
}

TEST(TestUtils, Housekeeping) {
  ASSERT_EQ("TRANSCOLD", typeToString(decode_type_t::TRANSCOLD));
  ASSERT_EQ(decode_type_t::TRANSCOLD, strToDecodeType("TRANSCOLD"));
  ASSERT_FALSE(hasACState(decode_type_t::TRANSCOLD));
  ASSERT_FALSE(IRac::isProtocolSupported(decode_type_t::TRANSCOLD));
  ASSERT_EQ(kTranscoldBits, IRsend::defaultBits(decode_type_t::TRANSCOLD));
  ASSERT_EQ(kNoRepeat, IRsend::minRepeats(decode_type_t::TRANSCOLD));
}
