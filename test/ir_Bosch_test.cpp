// Copyright 2022 David Conran

#include "IRac.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"


TEST(TestUtils, Housekeeping) {
  // Bosch144
  ASSERT_EQ("BOSCH144", typeToString(decode_type_t::BOSCH144));
  ASSERT_EQ(decode_type_t::BOSCH144, strToDecodeType("BOSCH144"));
  ASSERT_TRUE(hasACState(decode_type_t::BOSCH144));
  ASSERT_TRUE(IRac::isProtocolSupported(decode_type_t::BOSCH144));
  ASSERT_EQ(kBosch144Bits, IRsend::defaultBits(decode_type_t::BOSCH144));
  ASSERT_EQ(kNoRepeat, IRsend::minRepeats(decode_type_t::BOSCH144));
}

// Tests for decodeBosch144().

// Decode normal Bosch144 messages.
TEST(TestDecodeBosch144, RealExample) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);

  // Ref: https://github.com/crankyoldgit/IRremoteESP8266/issues/1787#issuecomment-1099993189
  // Mode: Cool; Fan: 100% ; Temp: 16°C
  const uint16_t rawData[299] = {
      4380, 4400,
      528, 1646, 504, 570, 504, 1646, 504, 1646, 504, 572, 502, 570, 504, 1646,
      504, 570, 504, 572, 502, 1646, 504, 570, 502, 570, 502, 1648, 502, 1646,
      502, 570, 502, 1646, 504, 572, 502, 572, 502, 1644, 504, 1646, 504, 1646,
      504, 1646, 502, 1648, 500, 1646, 504, 1646, 504, 1646, 504, 572, 502, 570,
      504, 570, 504, 570, 504, 570, 504, 570, 506, 570, 502, 572, 502, 570, 502,
      572, 502, 572, 502, 572, 502, 572, 502, 572, 500, 1648, 502, 1644, 502,
      1646, 504, 1646, 502, 1646, 504, 1646, 504, 1644, 504, 1646,
      504, 5234,
      4360, 4422,
      504, 1646, 502, 596, 478, 1670, 478, 1646, 504, 570, 504, 572, 500, 1646,
      502, 572, 502, 572, 502, 1644, 506, 570, 502, 570, 504, 1644, 506, 1644,
      502, 574, 502, 1644, 504, 570, 504, 570, 504, 1644, 504, 1646, 504, 1644,
      506, 1644, 504, 1646, 504, 1646, 504, 1644, 504, 1646, 502, 570, 504, 570,
      504, 570, 504, 570, 502, 570, 504, 570, 502, 572, 502, 570, 504, 570, 504,
      570, 504, 570, 502, 572, 502, 570, 506, 570, 504, 1646, 502, 1646, 504,
      1646, 504, 1646, 504, 1646, 502, 1644, 504, 1644, 504, 1646,
      502, 5236,
      4360, 4424,
      504, 1646, 504, 1646, 502, 572, 504, 1644, 504, 570, 504, 1646, 504, 570,
      502, 1644, 504, 570, 504, 1644, 506, 1646, 502, 572, 502, 572, 502, 1646,
      504, 570, 504, 570, 504, 570, 502, 572, 504, 570, 504, 570, 504, 570, 502,
      572, 502, 570, 504, 570, 502, 570, 504, 572, 502, 572, 502, 1646, 504,
      570, 504, 570, 504, 570, 502, 574, 502, 572, 502, 572, 502, 572, 502, 572,
      502, 572, 502, 570, 504, 572, 502, 572, 502, 572, 502, 1646, 504, 572,
      502, 570, 502, 1646, 504, 572, 504, 570, 504, 1644,
      504};  // COOLIX B23F00
  const uint8_t expectedState[kBosch144StateLength] = {
      0xB2, 0x4D, 0x3F, 0xC0, 0x00, 0xFF,
      0xB2, 0x4D, 0x3F, 0xC0, 0x00, 0xFF,
      0xD5, 0x64, 0x00, 0x10, 0x00, 0x49};
  irsend.begin();
  irsend.reset();

  irsend.sendRaw(rawData, 299, 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(decode_type_t::BOSCH144, irsend.capture.decode_type);
  EXPECT_EQ(kBosch144Bits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
  EXPECT_EQ(
      "Power: On, Mode: 0 (Cool), Fan: 5 (High), Temp: 16C, Quiet: Off",
      IRAcUtils::resultAcToString(&irsend.capture));
  stdAc::state_t result, prev;
  ASSERT_TRUE(IRAcUtils::decodeToState(&irsend.capture, &result, &prev));
}

TEST(TestDecodeBosch144, DurastarExample) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);

  // Mode: Heat; Fan: auto ; Temp: 73°F
  uint16_t rawData[299] = {
      4382, 4414, 522, 1630, 522, 552, 550, 1600, 538, 1616, 540, 532, 548,
      526, 550, 1602, 548, 528, 548, 528, 550, 1604, 522, 552, 548, 528, 548,
      1604, 522, 1630, 546, 528, 548, 1604, 522, 1628, 524, 552, 548, 1604,
      546, 1606, 548, 1604, 522, 1630, 520, 1632, 546, 1604, 522, 554, 522,
      1630, 520, 554, 522, 552, 522, 552, 524, 552, 546, 530, 546, 528, 548,
      528, 548, 1604, 546, 528, 522, 1630, 522, 1630, 522, 1630, 546, 528, 524,
      552, 522, 1630, 524, 552, 524, 1630, 520, 554, 522, 554, 522, 554, 548,
      1606, 520, 1630, 522, 5224, 4404, 4390, 522, 1630, 522, 554, 520, 1632,
      520, 1632, 520, 554, 520, 556, 520, 1632, 520, 556, 520, 556, 520, 1632,
      518, 558, 518, 556, 518, 1634, 518, 1634, 518, 558, 518, 1634, 518, 1634,
      518, 558, 516, 1636, 516, 1636, 516, 1636, 514, 1638, 514, 1638, 514,
      1638, 514, 582, 494, 1658, 492, 582, 492, 584, 492, 584, 490, 584, 492,
      584, 490, 586, 488, 586, 488, 1664, 488, 588, 486, 1666, 484, 1666, 486,
      1666, 484, 590, 484, 592, 484, 1668, 484, 592, 484, 1666, 486, 590, 484,
      592, 484, 592, 484, 1668, 484, 1666, 486, 5262, 4344, 4450, 484, 1668,
      484, 1668, 482, 592, 484, 1668, 482, 592, 484, 1680, 472, 594, 482, 1668,
      482, 616, 460, 1692, 460, 1692, 458, 616, 460, 616, 460, 1692, 460, 1692,
      458, 616, 460, 616, 458, 618, 458, 616, 460, 616, 458, 616, 460, 614,
      460, 616, 460, 616, 458, 616, 460, 616, 460, 616, 460, 616, 460, 616,
      460, 616, 460, 616, 458, 1692, 458, 616, 460, 616, 460, 616, 460, 616,
      444, 632, 438, 638, 434, 642, 434, 642, 434, 640, 434, 640, 434, 1718,
      434, 1718, 434, 1718, 434, 1718, 432, 644, 432, 642, 434
  };  // DURASTAR DRAW09F2A

  uint8_t expectedState[18] = {
      0xB2, 0x4D, 0xBF, 0x40, 0x5C, 0xA3,
      0xB2, 0x4D, 0xBF, 0x40, 0x5C, 0xA3,
      0xD5, 0x66, 0x00, 0x01, 0x00, 0x3C};

  irsend.begin();
  irsend.reset();

  irsend.sendRaw(rawData, 299, 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(decode_type_t::BOSCH144, irsend.capture.decode_type);
  EXPECT_EQ(kBosch144Bits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
  EXPECT_EQ(
      "Power: On, Mode: 6 (Heat), Fan: 0 (Auto), Temp: 73F, Quiet: Off",
      IRAcUtils::resultAcToString(&irsend.capture));
  stdAc::state_t result, prev;
  ASSERT_TRUE(IRAcUtils::decodeToState(&irsend.capture, &result, &prev));
}

// This example command has more extreme timings
TEST(TestDecodeBosch144, DurastarExample61F) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);

  // Mode: Heat; Fan: High ; Temp: 61°F
  uint16_t rawData[299] = {
      4450, 4345, 590, 1560, 590, 510, 560, 1560, 590, 1560, 590, 510, 565,
      510, 560, 1565, 590, 510, 560, 515, 560, 1565, 590, 515, 560, 515, 560,
      1565, 590, 1565, 585, 520, 560, 1570, 560, 515, 560, 515, 555, 1595, 560,
      1595, 555, 1595, 530, 1620, 530, 1620, 530, 1625, 525, 1625, 525, 1630,
      520, 555, 525, 550, 525, 550, 525, 550, 525, 550, 550, 525, 550, 530,
      550, 525, 550, 530, 550, 525, 550, 1605, 520, 1630, 520, 550, 550, 525,
      550, 1605, 520, 1630, 520, 1630, 525, 1630, 520, 550, 525, 550, 525,
      1625, 550, 1605, 550, 5200, 4405, 4390, 520, 1630, 520, 555, 520, 1630,
      520, 1630, 520, 555, 520, 555, 520, 1630, 520, 555, 520, 555, 520, 1635,
      520, 555, 520, 560, 520, 1635, 515, 1635, 515, 560, 515, 1655, 495, 580,
      495, 580, 495, 1660, 495, 1660, 495, 1660, 490, 1660, 490, 1660, 490,
      1660, 490, 1660, 490, 1660, 490, 585, 490, 590, 485, 590, 485, 590, 485,
      595, 480, 590, 480, 600, 480, 590, 485, 590, 485, 590, 485, 1670, 485,
      1670, 485, 590, 485, 590, 485, 1670, 480, 1670, 485, 1670, 480, 1670,
      480, 595, 480, 590, 485, 1670, 480, 1690, 460, 5290, 4320, 4475, 460,
      1690, 460, 1690, 460, 615, 460, 1690, 460, 615, 460, 1690, 460, 615,
      460, 1690, 460, 615, 460, 1690, 460, 1690, 460, 615, 460, 615, 460,
      1690, 460, 615, 460, 615, 460, 615, 460, 620, 460, 1695, 435, 640, 435,
      640, 435, 640, 435, 640, 435, 640, 435, 640, 440, 640, 435, 665, 410,
      1740, 410, 665, 410, 665, 410, 665, 410, 1740, 410, 665, 410, 665, 410,
      670, 410, 665, 410, 665, 410, 665, 410, 670, 405, 670, 405, 670, 410,
      1745, 405, 1745, 410, 670, 410, 1745, 380, 670, 405, 1770, 385, 690, 385
  };  // DURASTAR DRAW09F2A

  uint8_t expectedState[18] = {
      0xB2, 0x4D, 0x3F, 0xC0, 0x0C, 0xF3,
      0xB2, 0x4D, 0x3F, 0xC0, 0x0C, 0xF3,
      0xD5, 0x64, 0x20, 0x11, 0x00, 0x6A};

  irsend.begin();
  irsend.reset();

  irsend.sendRaw(rawData, 299, 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(decode_type_t::BOSCH144, irsend.capture.decode_type);
  EXPECT_EQ(kBosch144Bits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
  EXPECT_EQ(
      "Power: On, Mode: 6 (Heat), Fan: 5 (High), Temp: 61F, Quiet: Off",
      IRAcUtils::resultAcToString(&irsend.capture));
  stdAc::state_t result, prev;
  ASSERT_TRUE(IRAcUtils::decodeToState(&irsend.capture, &result, &prev));
}

TEST(TestDecodeBosch144, SyntheticSelfDecode) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  irsend.reset();
  const uint8_t expectedState[kBosch144StateLength] = {
      0xB2, 0x4D, 0x3F, 0xC0, 0x00, 0xFF,
      0xB2, 0x4D, 0x3F, 0xC0, 0x00, 0xFF,
      0xD5, 0x64, 0x00, 0x10, 0x00, 0x49};
  irsend.sendBosch144(expectedState);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(decode_type_t::BOSCH144, irsend.capture.decode_type);
  EXPECT_EQ(kBosch144Bits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
  EXPECT_EQ(
      "Power: On, Mode: 0 (Cool), Fan: 5 (High), Temp: 16C, Quiet: Off",
      IRAcUtils::resultAcToString(&irsend.capture));
  stdAc::state_t result, prev;
  ASSERT_TRUE(IRAcUtils::decodeToState(&irsend.capture, &result, &prev));
}
