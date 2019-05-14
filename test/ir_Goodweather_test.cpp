// Copyright 2019 David Conran

#include "ir_Goodweather.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

TEST(TestIRUtils, Goodweather) {
  ASSERT_EQ("GOODWEATHER", typeToString(decode_type_t::GOODWEATHER));
  ASSERT_EQ(decode_type_t::GOODWEATHER, strToDecodeType("GOODWEATHER"));
  ASSERT_FALSE(hasACState(decode_type_t::GOODWEATHER));
}

// Tests for sendGoodweather().

// Test sending typical data only.
TEST(TestSendGoodweather, SendDataOnly) {
  IRsendTest irsend(0);
  irsend.begin();

  irsend.reset();
  irsend.sendGoodweather(0x0);
  EXPECT_EQ(
      "f38000d50"
      "m6800s6800"
      "m640s1600m640s1600m640s1600m640s1600m640s1600m640s1600m640s1600m640s1600"
      "m640s580m640s580m640s580m640s580m640s580m640s580m640s580m640s580"
      "m640s1600m640s1600m640s1600m640s1600m640s1600m640s1600m640s1600m640s1600"
      "m640s580m640s580m640s580m640s580m640s580m640s580m640s580m640s580"
      "m640s1600m640s1600m640s1600m640s1600m640s1600m640s1600m640s1600m640s1600"
      "m640s580m640s580m640s580m640s580m640s580m640s580m640s580m640s580"
      "m640s1600m640s1600m640s1600m640s1600m640s1600m640s1600m640s1600m640s1600"
      "m640s580m640s580m640s580m640s580m640s580m640s580m640s580m640s580"
      "m640s1600m640s1600m640s1600m640s1600m640s1600m640s1600m640s1600m640s1600"
      "m640s580m640s580m640s580m640s580m640s580m640s580m640s580m640s580"
      "m640s1600m640s1600m640s1600m640s1600m640s1600m640s1600m640s1600m640s1600"
      "m640s580m640s580m640s580m640s580m640s580m640s580m640s580m640s580"
      "m640s6800m640s100000",
      irsend.outputStr());

  irsend.reset();
}

// Tests for decodeGoodweather().

// Decode normal Goodweather messages.
TEST(TestDecodeGoodweather, SyntheticDecode) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  // Normal (made-up value) Goodweather 48-bit message.
  irsend.reset();
  irsend.sendGoodweather(0x1234567890AB);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(decode_type_t::GOODWEATHER, irsend.capture.decode_type);
  EXPECT_EQ(kGoodweatherBits, irsend.capture.bits);
  EXPECT_EQ(0x1234567890AB, irsend.capture.value);
  EXPECT_EQ(0x0, irsend.capture.address);
  EXPECT_EQ(0x0, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);
  // Normal (Real) Goodweather 48-bit message.
  irsend.reset();
  irsend.sendGoodweather(0xD5276A030000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(decode_type_t::GOODWEATHER, irsend.capture.decode_type);
  EXPECT_EQ(kGoodweatherBits, irsend.capture.bits);
  EXPECT_EQ(0xD5276A030000, irsend.capture.value);
  EXPECT_EQ(0x0, irsend.capture.address);
  EXPECT_EQ(0x0, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);
}

// Decode a real example of a Goodweather message.
// https://github.com/markszabo/IRremoteESP8266/issues/697#issuecomment-490209819
TEST(TestDecodeGoodweather, RealExampleDecode) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  IRGoodweatherAc ac(0);
  irsend.begin();
  ac.begin();

  irsend.reset();
  // Raw Goodweather 48-bit message.
  uint16_t rawData_1[197] = {
      6828, 6828, 732, 1780, 652, 1830, 652, 1806, 678, 1830, 652, 1806, 678,
      1830, 652, 1830, 652, 1834, 706, 518, 734, 508, 734, 514, 734, 510, 732,
      510, 732, 510, 732, 510, 732, 514, 732, 1776, 706, 1780, 628, 1854, 628,
      1832, 654, 1832, 654, 1856, 628, 1832, 634, 1876, 680, 536, 708, 536, 708,
      536, 706, 538, 706, 538, 706, 538, 706, 536, 680, 564, 680, 1828, 708,
      1758, 680, 1804, 680, 1828, 708, 1778, 732, 1754, 732, 1754, 732, 1756,
      732, 490, 658, 586, 658, 586, 658, 586, 658, 586, 658, 584, 658, 586, 658,
      586, 660, 1850, 704, 520, 658, 1828, 658, 1826, 658, 1826, 658, 586, 660,
      584, 684, 1826, 730, 490, 686, 1824, 660, 560, 710, 532, 710, 534, 712,
      1776, 712, 1774, 686, 560, 712, 1774, 712, 1798, 730, 492, 712, 1798, 684,
      1798, 678, 568, 730, 1756, 686, 1796, 686, 532, 712, 532, 712, 1796, 728,
      494, 712, 532, 738, 1772, 730, 492, 712, 532, 738, 506, 738, 1772, 660,
      582, 728, 1736, 712, 558, 710, 1750, 710, 558, 710, 510, 738, 1748, 738,
      508, 736, 1772, 684, 534, 736, 1772, 704, 518, 738, 1772, 660, 1824, 678,
      6770, 684};  // COOLIX 4624AB
  irsend.sendRaw(rawData_1, 197, 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(decode_type_t::GOODWEATHER, irsend.capture.decode_type);
  EXPECT_EQ(kGoodweatherBits, irsend.capture.bits);
  EXPECT_EQ(0xD52462000000, irsend.capture.value);
  EXPECT_EQ(0x0, irsend.capture.address);
  EXPECT_EQ(0x0, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);
  ac.setRaw(irsend.capture.value);
  EXPECT_EQ(
      "Power: On, Mode: 1 (COOL), Temp: 20C, Fan: 3 (LOW), "
      "Turbo: -, Light: -, Sleep: -, Swing: 0 (Fast)",
      ac.toString());

  uint16_t rawData_2[197] = {
      6190, 7296, 696, 1496, 634, 1562, 642, 1582, 640, 1564, 564, 1598, 638,
      1558, 646, 1560, 588, 1616, 618, 520, 620, 494, 622, 494, 646, 494, 620,
      496, 644, 494, 590, 528, 642, 494, 642, 1544, 638, 1584, 618, 1564, 804,
      1394, 620, 1564, 640, 1558, 644, 1586, 562, 1616, 620, 492, 672, 470, 622,
      494, 646, 494, 622, 494, 646, 494, 620, 498, 644, 492, 596, 520, 644, 494,
      592, 1596, 612, 1584, 642, 1560, 614, 1612, 594, 1584, 620, 1558, 646,
      1556, 644, 1562, 618, 520, 620, 494, 620, 494, 646, 494, 568, 548, 644,
      494, 616, 1570, 638, 494, 670, 1534, 568, 550, 646, 1556, 616, 526, 618,
      492, 672, 1532, 568, 550, 646, 1558, 640, 500, 618, 1560, 668, 470, 642,
      1548, 658, 1536, 642, 520, 588, 504, 644, 492, 644, 478, 642, 1582, 618,
      1586, 590, 506, 640, 1556, 646, 1584, 562, 1616, 620, 1558, 646, 1556,
      670, 454, 638, 492, 648, 1558, 642, 478, 644, 492, 590, 530, 858, 1342,
      642, 496, 618, 1564, 642, 492, 642, 1548, 636, 492, 648, 494, 622, 1562,
      642, 492, 644, 1562, 618, 520, 620, 1558, 644, 476, 640, 1558, 646, 1558,
      612, 7382, 594};  // UNKNOWN 71DD9105

  irsend.reset();
  irsend.sendRaw(rawData_2, 197, 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(decode_type_t::GOODWEATHER, irsend.capture.decode_type);
  EXPECT_EQ(kGoodweatherBits, irsend.capture.bits);
  EXPECT_EQ(0xD5276A030000, irsend.capture.value);
  EXPECT_EQ(0x0, irsend.capture.address);
  EXPECT_EQ(0x0, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);
  ac.setRaw(irsend.capture.value);
  EXPECT_EQ(
      "Power: On, Mode: 1 (COOL), Temp: 23C, Fan: 3 (LOW), "
      "Turbo: -, Light: -, Sleep: -, Swing: 2 (Off)",
      ac.toString());
}
