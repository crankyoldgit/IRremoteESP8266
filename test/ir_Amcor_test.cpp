// Copyright 2018 David Conran

#include "IRac.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "IRutils.h"
#include "gtest/gtest.h"

TEST(TestUtils, Housekeeping) {
  ASSERT_EQ("AMCOR", typeToString(decode_type_t::AMCOR));
  ASSERT_EQ(decode_type_t::AMCOR, strToDecodeType("AMCOR"));
  ASSERT_FALSE(hasACState(decode_type_t::AMCOR));
  ASSERT_FALSE(IRac::isProtocolSupported(decode_type_t::AMCOR));
}

TEST(TestDecodeAmcor, RealExample) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  // Data from Issue #834 captured by ldellus
  // Turn on, cooling, 27 deg C.
  uint16_t rawData[263] = {
    8210, 4276, 1544, 480, 596, 1510, 596, 1510, 596, 1692, 388, 1534, 596,
    1510, 596, 1510, 596, 1684, 1450, 480, 596, 1510, 570, 1534, 570, 1718, 386,
    1536, 594, 1500, 1632, 482, 596, 1694, 362, 1550, 1632, 472, 1658, 456, 596,
    1684, 1474, 446, 1634, 480, 572, 1534, 572, 1718, 362, 1558, 572, 1534, 570,
    1534, 570, 1720, 360, 1558, 572, 1534, 570, 1534, 570, 1718, 360, 1560, 572,
    1534, 570, 1534, 570, 1718, 362, 1560, 572, 1532, 572, 1534, 570, 1718, 362,
    1558, 572, 1532, 572, 1534, 570, 1710, 1448, 472, 1634, 480, 572, 1534, 570,
    1718, 362, 1558, 572, 1534, 570, 1534, 572, 1716, 362, 1560, 572, 1534, 572,
    1534, 570, 1718, 362, 1550, 1634, 480, 570, 1536, 570, 1710, 1448, 482, 570,
    1534, 570, 1536, 570, 1508, 1856, 34298,
    8218, 4314, 1502, 522, 530, 1576, 504, 1602, 504, 1786, 392, 1528, 504,
    1600, 504, 1600, 504, 1770, 1414, 522, 528, 1578, 502, 1602, 504, 1784, 394,
    1528, 504, 1584, 1574, 548, 528, 1762, 392, 1512, 1572, 530, 1600, 524, 528,
    1744, 1390, 530, 1574, 546, 506, 1600, 504, 1784, 394, 1528, 504, 1600, 578,
    1528, 504, 1784, 394, 1526, 504, 1600, 504, 1600, 506, 1784, 394, 1528, 504,
    1602, 504, 1602, 504, 1784, 394, 1526, 506, 1600, 504, 1600, 506, 1784, 392,
    1526, 506, 1600, 504, 1602, 502, 1768, 1390, 530, 1574, 548, 504, 1600, 504,
    1786, 392, 1530, 504, 1600, 504, 1600, 504, 1786, 392, 1528, 504, 1600, 504,
    1600, 506, 1784, 394, 1512, 1574, 548, 504, 1602, 504, 1768, 1388, 548, 504,
    1602, 504, 1602, 502, 1574, 1792};  // UNKNOWN D510A6EF

  irsend.reset();
  irsend.sendRaw(rawData, 263, 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(AMCOR, irsend.capture.decode_type);
  EXPECT_EQ(kAmcorBits, irsend.capture.bits);
  EXPECT_EQ(0x80826C00000C0048, irsend.capture.value);

  // Verify the repeat is the same decode.
  irsend.reset();
  irsend.sendRaw(rawData + 132, 131, 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(AMCOR, irsend.capture.decode_type);
  EXPECT_EQ(kAmcorBits, irsend.capture.bits);
  EXPECT_EQ(0x80826C00000C0048, irsend.capture.value);
}

TEST(TestDecodeAmcor, SyntheticSelfDecode) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);

  irsend.begin();
  irsend.reset();
  irsend.sendAmcor(0x80826C00000C0048);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(AMCOR, irsend.capture.decode_type);
  EXPECT_EQ(kAmcorBits, irsend.capture.bits);
  EXPECT_EQ(0x80826C00000C0048, irsend.capture.value);
}


// Test sending typical data only.
TEST(TestSendAmcor, SendDataOnly) {
  IRsendTest irsend(0);
  irsend.begin();

  irsend.reset();
  irsend.sendAmcor(0x80826C00000C0048);
  EXPECT_EQ(
      "f38000d50"
      "m8200s4200"
      "m1500s500m500s1500m500s1500m500s1500m500s1500m500s1500m500s1500m500s1500"
      "m1500s500m500s1500m500s1500m500s1500m500s1500m500s1500m1500s500m500s1500"
      "m500s1500m1500s500m1500s500m500s1500m1500s500m1500s500m500s1500m500s1500"
      "m500s1500m500s1500m500s1500m500s1500m500s1500m500s1500m500s1500m500s1500"
      "m500s1500m500s1500m500s1500m500s1500m500s1500m500s1500m500s1500m500s1500"
      "m500s1500m500s1500m500s1500m500s1500m1500s500m1500s500m500s1500m500s1500"
      "m500s1500m500s1500m500s1500m500s1500m500s1500m500s1500m500s1500m500s1500"
      "m500s1500m1500s500m500s1500m500s1500m1500s500m500s1500m500s1500m500s1500"
      "m1900s34300"
      "m8200s4200"
      "m1500s500m500s1500m500s1500m500s1500m500s1500m500s1500m500s1500m500s1500"
      "m1500s500m500s1500m500s1500m500s1500m500s1500m500s1500m1500s500m500s1500"
      "m500s1500m1500s500m1500s500m500s1500m1500s500m1500s500m500s1500m500s1500"
      "m500s1500m500s1500m500s1500m500s1500m500s1500m500s1500m500s1500m500s1500"
      "m500s1500m500s1500m500s1500m500s1500m500s1500m500s1500m500s1500m500s1500"
      "m500s1500m500s1500m500s1500m500s1500m1500s500m1500s500m500s1500m500s1500"
      "m500s1500m500s1500m500s1500m500s1500m500s1500m500s1500m500s1500m500s1500"
      "m500s1500m1500s500m500s1500m500s1500m1500s500m500s1500m500s1500m500s1500"
      "m1900s34300",
      irsend.outputStr());
}
