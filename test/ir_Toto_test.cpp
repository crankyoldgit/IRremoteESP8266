// Copyright 2022 crankyoldgit (David Conran)

#include "IRac.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "IRutils.h"
#include "gtest/gtest.h"


// General housekeeping
TEST(TestToto, Housekeeping) {
  ASSERT_EQ("TOTO", typeToString(TOTO));
  ASSERT_FALSE(hasACState(TOTO));
  ASSERT_EQ(kTotoBits, IRsend::defaultBits(decode_type_t::TOTO));
  ASSERT_EQ(kSingleRepeat, IRsend::minRepeats(decode_type_t::TOTO));
}

// Tests for decodeToto().

// Decode normal Toto messages.
TEST(TestDecodeToto, SyntheticDecode) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  // Normal Toto 39-bit message.
  irsend.reset();
  irsend.sendToto(0x200800B0B0);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(TOTO, irsend.capture.decode_type);
  EXPECT_EQ(kTotoBits, irsend.capture.bits);
  EXPECT_EQ(0xB0B0, irsend.capture.value);
  EXPECT_EQ(0, irsend.capture.address);
  EXPECT_EQ(0xB0B0, irsend.capture.command);
}

// Decode real example via Issue #1806
TEST(TestDecodeToto, RealDecode) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  irsend.reset();
  // Toto Full Flush from Issue #1806
  const uint16_t rawData[163] = {
      6266, 2734,
      598, 540, 598, 1626, 598, 512, 622, 516, 598, 514, 598, 510, 598, 514,
      628, 512, 596, 514, 600, 512, 598, 538, 600, 1622, 600, 512, 598, 540,
      602, 510, 598, 512, 598, 512, 624, 514, 598, 512, 598, 512, 598, 514,
      624, 512, 598, 514, 598, 1652, 596, 514, 598, 1626, 598, 1650, 598, 514,
      598, 512, 598, 540, 598, 514, 596, 1626, 626, 512, 574, 1648, 598, 1650,
      598, 514, 598, 512, 594, 544, 596, 514,
      598, 37996,
      6182, 2764,
      598, 514, 600, 1648, 600, 512, 596, 514, 598, 540, 598, 512, 600, 512,
      598, 512, 624, 514, 598, 514, 598, 512, 596, 1652, 598, 514, 598, 512,
      596, 540, 598, 514, 598, 512, 598, 512, 598, 540, 596, 516, 596, 514, 598,
      512, 574, 564, 598, 1626, 568, 542, 624, 1624, 626, 1622, 598, 514, 596,
      514, 598, 514, 596, 540, 600, 1622, 598, 512, 600, 1650, 598, 1624, 596,
      540, 600, 512, 598, 514, 596, 514,
      622};  // UNKNOWN 43BD67B3

  irsend.sendRaw(rawData, 163, 38);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(TOTO, irsend.capture.decode_type);
  EXPECT_EQ(kTotoBits, irsend.capture.bits);
  EXPECT_EQ(0xB0B0, irsend.capture.value);
  EXPECT_EQ(0, irsend.capture.address);
  EXPECT_EQ(0xB0B0, irsend.capture.command);
}
