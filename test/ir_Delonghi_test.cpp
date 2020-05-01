// Copyright 2020 David Conran

#include "IRac.h"
#include "ir_Delonghi.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "IRutils.h"
#include "gtest/gtest.h"

TEST(TestUtils, Housekeeping) {
  ASSERT_EQ("DELONGHI_AC", typeToString(decode_type_t::DELONGHI_AC));
  ASSERT_EQ(decode_type_t::DELONGHI_AC, strToDecodeType("DELONGHI_AC"));
  ASSERT_FALSE(hasACState(decode_type_t::DELONGHI_AC));
  ASSERT_FALSE(IRac::isProtocolSupported(decode_type_t::DELONGHI_AC));
  ASSERT_EQ(kDelonghiAcBits, IRsend::defaultBits(decode_type_t::DELONGHI_AC));
  ASSERT_EQ(kDelonghiAcDefaultRepeat,
            IRsend::minRepeats(decode_type_t::DELONGHI_AC));
}

TEST(TestDecodeDelonghiAc, SyntheticSelfDecode) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);

  irsend.begin();
  irsend.reset();
  irsend.sendDelonghiAc(0x6900000D0D01FB53);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(DELONGHI_AC, irsend.capture.decode_type);
  EXPECT_EQ(kDelonghiAcBits, irsend.capture.bits);
  EXPECT_EQ(0x6900000D0D01FB53, irsend.capture.value);
  EXPECT_EQ(0, irsend.capture.command);
  EXPECT_EQ(0, irsend.capture.address);
}

TEST(TestDecodeDelonghiAc, RealExample) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  // Data from:
  // https://github.com/crankyoldgit/IRremoteESP8266/issues/1096#issue-610665633
  uint16_t rawData[131] = {
      8984, 4200,
      608, 1516, 608, 1516, 612, 472, 556, 528, 560, 1564, 556, 528, 560, 1564,
      564, 528, 552, 1572, 556, 1568, 556, 528, 552, 1572, 556, 1568, 560, 1564,
      552, 1572, 556, 1576, 552, 1568, 560, 528, 560, 524, 556, 528, 552, 532,
      560, 528, 552, 532, 556, 532, 560, 1564, 560, 528, 552, 1568, 560, 1564,
      564, 524, 556, 528, 560, 524, 556, 536, 556, 1568, 560, 524, 556, 1568,
      560, 1564, 584, 500, 588, 496, 584, 500, 592, 500, 588, 496, 584, 500,
      592, 496, 584, 500, 588, 496, 584, 500, 592, 492, 584, 508, 584, 500,
      588, 496, 584, 500, 592, 496, 584, 500, 580, 504, 584, 500, 580, 508,
      584, 1544, 584, 500, 588, 496, 584, 1540, 588, 500, 580, 1540, 588, 1536,
      588, 500,
      592};

  irsend.reset();
  irsend.sendRaw(rawData, 263, 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(DELONGHI_AC, irsend.capture.decode_type);
  EXPECT_EQ(kDelonghiAcBits, irsend.capture.bits);
  EXPECT_EQ(0x6900000D0D01FB53, irsend.capture.value);
  EXPECT_EQ(0, irsend.capture.command);
  EXPECT_EQ(0, irsend.capture.address);
}
