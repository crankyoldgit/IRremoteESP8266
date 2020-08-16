// Copyright 2020 crankyoldgit

#include "ir_Voltas.h"
#include "IRac.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

// Tests for decodeVoltas().

TEST(TestDecodeVoltas, RealExample) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  const uint16_t rawData[161] = {
      1002, 584, 1000, 586, 1000, 2568, 1002, 2570, 1002, 586, 998, 588, 1000,
      2568, 1002, 2570, 1002, 2572, 1002, 584, 1002, 586, 1000, 584, 1000, 586,
      1002, 2568, 1004, 584, 1000, 586, 1002, 2568, 1002, 584, 1002, 584, 1004,
      584, 1000, 2568, 1002, 586, 1000, 586, 998, 590, 998, 584, 1002, 584,
      1000, 586, 1000, 2570, 1002, 2568, 1004, 584, 1000, 584, 1002, 584, 1002,
      582, 1004, 584, 1002, 2568, 1002, 2570, 1004, 2570, 1000, 586, 1002, 2568,
      1004, 2568, 1006, 584, 1000, 584, 1002, 2568, 1002, 2570, 1002, 2568,
      1002, 586, 1002, 2570, 1000, 2570, 1002, 588, 998, 586, 1000, 2568, 1004,
      2568, 1004, 2568, 1002, 588, 998, 2570, 1002, 2568, 1004, 586, 1002, 584,
      1000, 586, 1000, 2570, 1000, 586, 1000, 584, 1002, 586, 1000, 2568, 1004,
      584, 1000, 586, 1000, 586, 1002, 584, 1002, 586, 1000, 586, 1000, 586,
      1000, 586, 1000, 2568, 1002, 2568, 1002, 2568, 1004, 586, 1000, 584,
      1000, 2570, 1004, 2568, 1004, 584, 1002};
  const uint8_t expected[kVoltasStateLength] = {
      0x33, 0x84, 0x88, 0x18, 0x3B, 0x3B, 0x3B, 0x11, 0x00, 0xE6};

  irsend.begin();
  irsend.reset();
  irsend.sendRaw(rawData, 161, 38);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(decode_type_t::VOLTAS, irsend.capture.decode_type);
  ASSERT_EQ(kVoltasBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expected, irsend.capture.state, irsend.capture.bits);
  EXPECT_EQ(
      "Power: On, Turbo: Off, WiFi: On, Light: Off",
      IRAcUtils::resultAcToString(&irsend.capture));
  stdAc::state_t r, p;
  ASSERT_TRUE(IRAcUtils::decodeToState(&irsend.capture, &r, &p));
}

TEST(TestDecodeVoltas, SyntheticExample) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();
  irsend.reset();
  const uint8_t expected[kVoltasStateLength] = {
      0x33, 0x84, 0x88, 0x18, 0x3B, 0x3B, 0x3B, 0x11, 0x00, 0xE6};
  // power
  irsend.sendVoltas(expected);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(decode_type_t::VOLTAS, irsend.capture.decode_type);
  EXPECT_EQ(kVoltasBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expected, irsend.capture.state, irsend.capture.bits);
}

TEST(TestUtils, Housekeeping) {
  ASSERT_EQ("VOLTAS", typeToString(decode_type_t::VOLTAS));
  ASSERT_EQ(decode_type_t::VOLTAS, strToDecodeType("VOLTAS"));
  ASSERT_TRUE(hasACState(decode_type_t::VOLTAS));
  ASSERT_FALSE(IRac::isProtocolSupported(decode_type_t::VOLTAS));
  ASSERT_EQ(kVoltasBits, IRsend::defaultBits(decode_type_t::VOLTAS));
  ASSERT_EQ(kNoRepeat, IRsend::minRepeats(decode_type_t::VOLTAS));
}

TEST(TestIRVoltasClass, Checksums) {
  const uint8_t valid[kVoltasStateLength] = {
      0x33, 0x84, 0x88, 0x18, 0x3B, 0x3B, 0x3B, 0x11, 0x00, 0xE6};
  EXPECT_TRUE(IRVoltas::validChecksum(valid));
  EXPECT_FALSE(IRVoltas::validChecksum(valid, kVoltasStateLength - 1));
  EXPECT_EQ(0xE6, IRVoltas::calcChecksum(valid));
}

TEST(TestIRVoltasClass, SetandGetRaw) {
  const uint8_t valid[kVoltasStateLength] = {
      0x33, 0x84, 0x88, 0x18, 0x3B, 0x3B, 0x3B, 0x11, 0x00, 0xE6};
  const uint8_t badchecksum[kVoltasStateLength] = {
      0x33, 0x84, 0x88, 0x18, 0x3B, 0x3B, 0x3B, 0x11, 0x00, 0xE6};
  IRVoltas ac(kGpioUnused);

  ac.setRaw(valid);
  EXPECT_STATE_EQ(valid, ac.getRaw(), kVoltasBits);
  ac.setRaw(badchecksum);
  EXPECT_STATE_EQ(valid, ac.getRaw(), kVoltasBits);
}

TEST(TestIRVoltasClass, Power) {
  IRVoltas ac(kGpioUnused);
  ac.begin();

  ac.on();
  EXPECT_TRUE(ac.getPower());

  ac.off();
  EXPECT_FALSE(ac.getPower());

  ac.setPower(true);
  EXPECT_TRUE(ac.getPower());

  ac.setPower(false);
  EXPECT_FALSE(ac.getPower());
}
