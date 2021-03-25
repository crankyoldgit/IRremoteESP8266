// Copyright 2021 David Conran (crankyoldgit)

#include "IRac.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

// Tests for decodeTruma().

TEST(TestDecodeTruma, RealExample) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  //  16 / AUTO / -
  const uint16_t rawData[117] = {
      20194, 1028, 1798, 628, 558, 662, 1190, 640,
      1190, 638, 1190, 634, 1190, 638, 1188, 640,
      1164, 662, 558, 658, 1192, 636, 1166, 662,
      1190, 638, 1164, 658, 1190, 638, 582, 640,
      556, 662, 558, 660, 1188, 638, 558, 662,
      558, 662, 1190, 634, 1188, 638, 556, 664,
      558, 662, 556, 660, 556, 664, 558, 662,
      556, 664, 556, 662, 556, 664, 558, 662,
      558, 662, 558, 660, 556, 664, 556, 664,
      556, 664, 556, 660, 558, 664, 578, 640,
      580, 640, 556, 660, 556, 664, 556, 664,
      556, 664, 580, 636, 556, 664, 556, 664,
      556, 664, 556, 660, 556, 664, 1188, 638,
      1188, 640, 554, 662, 1188, 640, 1188, 638,
      554, 666, 1164, 654, 582};
  irsend.begin();
  irsend.reset();
  irsend.sendRaw(rawData, 117, 38);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(decode_type_t::TRUMA, irsend.capture.decode_type);
  ASSERT_EQ(kTrumaBits, irsend.capture.bits);
  EXPECT_EQ(0x49ffffffe6e081, irsend.capture.value);
  EXPECT_EQ(0x0, irsend.capture.address);
  EXPECT_EQ(0x0, irsend.capture.command);
}

TEST(TestDecodeTruma, SyntheticExample) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();
  irsend.reset();

  //  16 / AUTO / -
  irsend.sendTruma(0x49ffffffe6e081);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(decode_type_t::TRUMA, irsend.capture.decode_type);
  EXPECT_EQ(kTrumaBits, irsend.capture.bits);
  EXPECT_EQ(0x49ffffffe6e081, irsend.capture.value);
  EXPECT_EQ(0x0, irsend.capture.address);
  EXPECT_EQ(0x0, irsend.capture.command);

  EXPECT_EQ(
    "f38000d50"
    "m20200s1000"
    "m1800s630"
    "m600s630m1200s630m1200s630m1200s630m1200s630m1200s630m1200s630m600s630"
    "m1200s630m1200s630m1200s630m1200s630m1200s630m600s630m600s630m600s630"
    "m1200s630m600s630m600s630m1200s630m1200s630m600s630m600s630m600s630"
    "m600s630m600s630m600s630m600s630m600s630m600s630m600s630m600s630"
    "m600s630m600s630m600s630m600s630m600s630m600s630m600s630m600s630"
    "m600s630m600s630m600s630m600s630m600s630m600s630m600s630m600s630"
    "m600s630m1200s630m1200s630m600s630m1200s630m1200s630m600s630m1200s630"
    "m600s100000",
    irsend.outputStr());
}

TEST(TestUtils, Housekeeping) {
  ASSERT_EQ("TRUMA", typeToString(decode_type_t::TRUMA));
  ASSERT_EQ(decode_type_t::TRUMA, strToDecodeType("TRUMA"));
  ASSERT_FALSE(hasACState(decode_type_t::TRUMA));
  ASSERT_FALSE(IRac::isProtocolSupported(decode_type_t::TRUMA));
  ASSERT_EQ(kTrumaBits, IRsend::defaultBits(decode_type_t::TRUMA));
  ASSERT_EQ(kNoRepeat, IRsend::minRepeats(decode_type_t::TRUMA));
}
