// Copyright 2021 David Conran

#include "IRac.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "IRutils.h"
#include "gtest/gtest.h"

TEST(TestUtils, Housekeeping) {
  ASSERT_EQ("ECOCLIM", typeToString(decode_type_t::ECOCLIM));
  ASSERT_EQ(decode_type_t::ECOCLIM, strToDecodeType("ECOCLIM"));
  ASSERT_FALSE(hasACState(decode_type_t::ECOCLIM));
  ASSERT_FALSE(IRac::isProtocolSupported(decode_type_t::ECOCLIM));
  ASSERT_EQ(kEcoclimBits, IRsend::defaultBits(decode_type_t::ECOCLIM));
  ASSERT_EQ(kNoRepeat, IRsend::minRepeats(decode_type_t::ECOCLIM));
}

// Test sending typical data only.
TEST(TestSendEcoclim, SendDataOnly) {
  IRsendTest irsend(kGpioUnused);
  irsend.begin();

  irsend.reset();
  irsend.sendEcoclim(0x110673AEFFFF72);
  EXPECT_EQ(
      "f38000d50"
      "m5730s1935"
      "m440s637m440s637m440s637m440s1739m440s637m440s637m440s637m440s1739"
      "m440s637m440s637m440s637m440s637m440s637m440s1739m440s1739m440s637"
      "m440s637m440s1739m440s1739m440s1739m440s637m440s637m440s1739m440s1739"
      "m440s1739m440s637m440s1739m440s637m440s1739m440s1739m440s1739m440s637"
      "m440s1739m440s1739m440s1739m440s1739m440s1739m440s1739m440s1739m440s1739"
      "m440s1739m440s1739m440s1739m440s1739m440s1739m440s1739m440s1739m440s1739"
      "m440s637m440s1739m440s1739m440s1739m440s637m440s637m440s1739m440s637"
      "m5730s1935"
      "m440s637m440s637m440s637m440s1739m440s637m440s637m440s637m440s1739"
      "m440s637m440s637m440s637m440s637m440s637m440s1739m440s1739m440s637"
      "m440s637m440s1739m440s1739m440s1739m440s637m440s637m440s1739m440s1739"
      "m440s1739m440s637m440s1739m440s637m440s1739m440s1739m440s1739m440s637"
      "m440s1739m440s1739m440s1739m440s1739m440s1739m440s1739m440s1739m440s1739"
      "m440s1739m440s1739m440s1739m440s1739m440s1739m440s1739m440s1739m440s1739"
      "m440s637m440s1739m440s1739m440s1739m440s637m440s637m440s1739m440s637"
      "m5730s1935"
      "m440s637m440s637m440s637m440s1739m440s637m440s637m440s637m440s1739"
      "m440s637m440s637m440s637m440s637m440s637m440s1739m440s1739m440s637"
      "m440s637m440s1739m440s1739m440s1739m440s637m440s637m440s1739m440s1739"
      "m440s1739m440s637m440s1739m440s637m440s1739m440s1739m440s1739m440s637"
      "m440s1739m440s1739m440s1739m440s1739m440s1739m440s1739m440s1739m440s1739"
      "m440s1739m440s1739m440s1739m440s1739m440s1739m440s1739m440s1739m440s1739"
      "m440s637m440s1739m440s1739m440s1739m440s637m440s637m440s1739m440s637"
      "m7820s100000",
      irsend.outputStr());
}

TEST(TestDecodeEcoclim, SyntheticSelfDecode) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);

  irsend.begin();
  irsend.reset();
  irsend.sendEcoclim(0x110673AEFFFF72);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(ECOCLIM, irsend.capture.decode_type);
  EXPECT_EQ(kEcoclimBits, irsend.capture.bits);
  EXPECT_EQ(0x110673AEFFFF72, irsend.capture.value);
}

TEST(TestDecodeEcoclim, RealExample) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  // Ref: https://github.com/crankyoldgit/IRremoteESP8266/issues/1397#issuecomment-770376241
  uint16_t rawData[343] = {
      5834, 1950, 482, 580, 506, 614, 480, 612, 456, 1738, 482, 562, 532, 582,
      508, 608, 430, 1760, 456, 642, 456, 608, 484, 638, 458, 634, 456, 634,
      456, 1734, 482, 1704, 402, 690, 456, 638, 404, 1786, 458, 1730, 430, 1762,
      454, 638, 456, 636, 456, 1732, 426, 1764, 426, 1788, 400, 692, 402, 1764,
      452, 638, 430, 1760, 424, 1786, 374, 1818, 402, 690, 374, 1786, 424, 1796,
      402, 1758, 426, 1790, 376, 1782, 426, 1766, 400, 1810, 398, 1796, 400,
      1788, 428, 1734, 398, 1814, 400, 1762, 470, 1742, 400, 1786, 398, 1794,
      400, 1762, 398, 718, 400, 1792, 400, 1788, 400, 1788, 400, 694, 400, 694,
      402, 1788, 398, 664, 5720, 1944, 426, 642, 450, 696, 442, 650, 396, 1794,
      468, 602, 422, 642, 448, 696, 442, 1744, 392, 702, 392, 678, 420, 700,
      394, 700, 464, 628, 466, 1720, 464, 1726, 462, 628, 464, 630, 464, 1728,
      438, 1752, 462, 1722, 464, 636, 438, 626, 464, 1722, 490, 1698, 488, 1722,
      464, 628, 466, 1724, 466, 626, 464, 1724, 464, 1724, 462, 1732, 462, 626,
      464, 1726, 464, 1724, 464, 1722, 464, 1732, 464, 1690, 490, 1724, 464,
      1726, 464, 1728, 464, 1728, 462, 1690, 492, 1728, 462, 1724, 464, 1726,
      464, 1728, 464, 1720, 462, 1736, 460, 604, 490, 1718, 464, 1730, 462,
      1720, 462, 630, 464, 628, 462, 1734, 462, 600, 5632, 2028, 490, 630, 464,
      632, 464, 630, 462, 1724, 462, 630, 462, 636, 462, 602, 488, 1728, 464,
      630, 462, 632, 464, 630, 462, 628, 462, 630, 464, 1730, 460, 1724, 464,
      630, 464, 630, 462, 1728, 464, 1724, 462, 1728, 462, 630, 464, 630, 462,
      1724, 464, 1728, 460, 1730, 462, 628, 460, 1732, 464, 602, 492, 1722, 464,
      1726, 460, 1726, 464, 632, 464, 1696, 488, 1728, 460, 1732, 462, 1728,
      462, 1694, 488, 1728, 462, 1724, 464, 1732, 460, 1700, 490, 1728, 462,
      1694, 488, 1730, 462, 1720, 462, 1728, 464, 1726, 462, 1726, 460, 632,
      464, 1724, 462, 1726, 460, 1730, 464, 630, 464, 632, 464, 1728, 462, 596,
      7862};  // UNKNOWN 842242BF

  irsend.reset();
  irsend.sendRaw(rawData, 343, 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeEcoclim(&irsend.capture));
  EXPECT_EQ(ECOCLIM, irsend.capture.decode_type);
  EXPECT_EQ(kEcoclimBits, irsend.capture.bits);
  EXPECT_EQ(0x110673AEFFFF72, irsend.capture.value);
  EXPECT_EQ(
      "",
      IRAcUtils::resultAcToString(&irsend.capture));
  stdAc::state_t r, p;
  ASSERT_FALSE(IRAcUtils::decodeToState(&irsend.capture, &r, &p));
}
