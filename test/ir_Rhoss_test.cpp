// Copyright 2021 Tom Rosenback

#include "IRac.h"
#include "ir_Rhoss.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "IRutils.h"
#include "gtest/gtest.h"

TEST(TestUtils, Housekeeping) {
  ASSERT_EQ("RHOSS", typeToString(decode_type_t::RHOSS));
  ASSERT_EQ(decode_type_t::RHOSS, strToDecodeType("RHOSS"));
  ASSERT_TRUE(hasACState(decode_type_t::RHOSS));
  ASSERT_TRUE(IRac::isProtocolSupported(decode_type_t::RHOSS));
}

/*
Protocol  : RHOSS
Code      : 0xAA056000508054000000003300 (97 Bits)
Mesg Desc.: Power: On, Swing(V): Off, Mode: 5 (Auto), Fan: 0 (Auto), Temp: 21C
uint16_t rawData[197] = {3042, 4250,  646, 462,  646, 1540,  672, 434,  650, 1538,  648, 458,  648, 1540,  650, 458,  672, 1518,  646, 1544,  646, 460,  648, 1538,  650, 458,  650, 458,  674, 434,  648, 458,  650, 456,  674, 432,  650, 458,  650, 458,  648, 458,  650, 460,  648, 1538,  650, 1540,  646, 460,  672, 434,  650, 456,  674, 434,  650, 458,  672, 434,  650, 456,  674, 432,  674, 434,  650, 458,  648, 458,  650, 458,  674, 434,  672, 1516,  650, 458,  672, 1516,  674, 434,  650, 458,  648, 458,  650, 456,  650, 458,  674, 432,  650, 456,  650, 458,  674, 1514,  672, 434,  650, 458,  650, 1566,  620, 462,  672, 1514,  674, 434,  650, 1538,  672, 434,  650, 458,  650, 458,  672, 434,  650, 458,  648, 458,  648, 458,  672, 434,  650, 458,  674, 432,  650, 458,  674, 432,  650, 456,  650, 458,  650, 458,  674, 434,  650, 456,  650, 458,  650, 456,  650, 458,  648, 458,  
670, 436,  650, 458,  650, 458,  648, 458,  650, 456,  650, 458,  648, 458,  672, 434,  674, 434,  650, 458,  650, 458,  672, 434,  674, 1516,  650, 1540,  670, 434,  650, 458,  650, 1540,  646, 1542,  646, 458,  648, 458,  650, 458,  648};  // RHOSS
uint8_t state[12] = {0xAA, 0x05, 0x60, 0x00, 0x50, 0x80, 0x54, 0x00, 0x00, 0x00, 0x00, 0x33};
*/

// Test sending typical data only.
TEST(TestSendRhoss, SendDataOnly) {
  IRsendTest irsend(0);
  irsend.begin();

  uint8_t expectedState[kRhossStateLength] = { 0xAA, 0x05, 0x60, 0x00, 0x50, 0x80, 0x54, 0x00, 0x00, 0x00, 0x00, 0x33 };

  irsend.reset();
  irsend.sendRhoss(expectedState);
  
  EXPECT_EQ(
    "f38000d50"
    "m3040s4250"
    "m650s460m650s1540m650s460m650s1540m650s460m650s1540m650s460m650s1540"
    "m650s1540m650s460m650s1540m650s460m650s460m650s460m650s460m650s460"
    "m650s460m650s460m650s460m650s460m650s460m650s1540m650s1540m650s460"
    "m650s460m650s460m650s460m650s460m650s460m650s460m650s460m650s460"
    "m650s460m650s460m650s460m650s460m650s1540m650s460m650s1540m650s460"
    "m650s460m650s460m650s460m650s460m650s460m650s460m650s460m650s1540"
    "m650s460m650s460m650s1540m650s460m650s1540m650s460m650s1540m650s460"
    "m650s460m650s460m650s460m650s460m650s460m650s460m650s460m650s460"
    "m650s460m650s460m650s460m650s460m650s460m650s460m650s460m650s460"
    "m650s460m650s460m650s460m650s460m650s460m650s460m650s460m650s460"
    "m650s460m650s460m650s460m650s460m650s460m650s460m650s460m650s460"
    "m650s1540m650s1540m650s460m650s460m650s1540m650s1540m650s460m650s460"
    "m650s460m650s100000",
    irsend.outputStr()
  );
}

TEST(TestDecodeRhoss, SyntheticSelfDecode) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  IRRhossAc ac(0);

  uint8_t expectedState[kRhossStateLength] = { 0xAA, 0x05, 0x60, 0x00, 0x50, 0x80, 0x54, 0x00, 0x00, 0x00, 0x00, 0x33 };

  irsend.begin();
  irsend.reset();
  irsend.sendRhoss(expectedState);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(RHOSS, irsend.capture.decode_type);
  EXPECT_EQ(kRhossBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
  ac.setRaw(irsend.capture.state);
  EXPECT_EQ(
      "Power: On, Swing(V): Off, Mode: 5 (Auto), Fan: 0 (Auto), Temp: 21C",
      ac.toString());
}

// Tests for IRRhossAc class.

TEST(TestRhossAcClass, Power) {
  IRRhossAc ac(0);
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

TEST(TestRhossAcClass, Temperature) {
  IRRhossAc ac(0);
  ac.begin();

  ac.setTemp(0);
  EXPECT_EQ(kRhossMinTemp, ac.getTemp());

  ac.setTemp(255);
  EXPECT_EQ(kRhossMaxTemp, ac.getTemp());

  ac.setTemp(kRhossMinTemp);
  EXPECT_EQ(kRhossMinTemp, ac.getTemp());

  ac.setTemp(kRhossMaxTemp);
  EXPECT_EQ(kRhossMaxTemp, ac.getTemp());

  ac.setTemp(kRhossMinTemp - 1);
  EXPECT_EQ(kRhossMinTemp, ac.getTemp());

  ac.setTemp(kRhossMaxTemp + 1);
  EXPECT_EQ(kRhossMaxTemp, ac.getTemp());

  ac.setTemp(17);
  EXPECT_EQ(17, ac.getTemp());

  ac.setTemp(21);
  EXPECT_EQ(21, ac.getTemp());

  ac.setTemp(25);
  EXPECT_EQ(25, ac.getTemp());

  ac.setTemp(29);
  EXPECT_EQ(29, ac.getTemp());
}


TEST(TestRhossAcClass, OperatingMode) {
  IRRhossAc ac(0);
  ac.begin();

  ac.setMode(kRhossAuto);
  EXPECT_EQ(kRhossAuto, ac.getMode());

  ac.setMode(kRhossCool);
  EXPECT_EQ(kRhossCool, ac.getMode());

  ac.setMode(kRhossHeat);
  EXPECT_EQ(kRhossHeat, ac.getMode());

  ac.setMode(kRhossDry);
  EXPECT_EQ(kRhossDry, ac.getMode());

  ac.setMode(kRhossFan);
  EXPECT_EQ(kRhossFan, ac.getMode());

  ac.setMode(kRhossAuto + 1);
  EXPECT_EQ(kRhossAuto, ac.getMode());

  ac.setMode(255);
  EXPECT_EQ(kRhossAuto, ac.getMode());
}

TEST(TestRhossAcClass, FanSpeed) {
  IRRhossAc ac(0);
  ac.begin();

  ac.setFan(0);
  EXPECT_EQ(kRhossFanAuto, ac.getFan());

  ac.setFan(255);
  EXPECT_EQ(kRhossFanAuto, ac.getFan());

  ac.setFan(kRhossFanMax);
  EXPECT_EQ(kRhossFanMax, ac.getFan());

  ac.setFan(kRhossFanMax + 1);
  EXPECT_EQ(kRhossFanAuto, ac.getFan());

  ac.setFan(kRhossFanMax - 1);
  EXPECT_EQ(kRhossFanMax - 1, ac.getFan());

  ac.setFan(1);
  EXPECT_EQ(1, ac.getFan());

  ac.setFan(1);
  EXPECT_EQ(1, ac.getFan());

  ac.setFan(3);
  EXPECT_EQ(3, ac.getFan());
}
TEST(TestRhossAcClass, Checksums) {
  uint8_t state[kRhossStateLength] = { 0xAA, 0x05, 0x60, 0x00, 0x50, 0x80, 0x54, 0x00, 0x00, 0x00, 0x00, 0x33 };

  ASSERT_EQ(0x33, IRRhossAc::calcChecksum(state));
  EXPECT_TRUE(IRRhossAc::validChecksum(state));
  // Change the array so the checksum is invalid.
  state[0] ^= 0xFF;
  EXPECT_FALSE(IRRhossAc::validChecksum(state));
  // Restore the previous change, and change another byte.
  state[0] ^= 0xFF;
  state[4] ^= 0xFF;
  EXPECT_FALSE(IRRhossAc::validChecksum(state));
  state[4] ^= 0xFF;
  EXPECT_TRUE(IRRhossAc::validChecksum(state));

  // Additional known good states.
  uint8_t knownGood1[kRhossStateLength] = { 0xAA, 0x06, 0x60, 0x00, 0x50, 0x80, 0x54, 0x00, 0x00, 0x00, 0x00, 0x34 };
  EXPECT_TRUE(IRRhossAc::validChecksum(knownGood1));
  ASSERT_EQ(0x17, IRRhossAc::calcChecksum(knownGood1));

  uint8_t knownGood2[kRhossStateLength] = { 0xAA, 0x07, 0x60, 0x00, 0x50, 0x80, 0x54, 0x00, 0x00, 0x00, 0x00, 0x35 };
  EXPECT_TRUE(IRRhossAc::validChecksum(knownGood2));
  ASSERT_EQ(0x10, IRRhossAc::calcChecksum(knownGood2));
  
  uint8_t knownGood3[kRhossStateLength] = { 0xAA, 0x07, 0x60, 0x00, 0x53, 0x80, 0x54, 0x00, 0x00, 0x00, 0x00, 0x38 };
  EXPECT_TRUE(IRRhossAc::validChecksum(knownGood3));
  ASSERT_EQ(0x18, IRRhossAc::calcChecksum(knownGood3));

  // For a recalculation, same as knownGood3 except for the checksum.
  uint8_t knownBad[kRhossStateLength] = { 0xAA, 0x07, 0x60, 0x00, 0x53, 0x80, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00 };
  EXPECT_FALSE(IRRhossAc::validChecksum(knownBad));
  IRRhossAc ac(0);
  ac.setRaw(knownBad);
  EXPECT_STATE_EQ(knownGood3, ac.getRaw(), kRhossBits);
}

/*
TEST(TestRhossAcClass, Max) {
  IRRhossAc ac(0);
  ac.begin();

  ac.setMode(kRhossCool);
  ac.setMax(true);
  EXPECT_EQ(kRhossCool, ac.getMode());
  EXPECT_EQ(kRhossMinTemp, ac.getTemp());
  EXPECT_TRUE(ac.getMax());
  ac.setMax(false);
  EXPECT_EQ(kRhossCool, ac.getMode());
  EXPECT_EQ(kRhossMinTemp, ac.getTemp());
  EXPECT_FALSE(ac.getMax());

  ac.setMode(kRhossHeat);
  ac.setMax(true);
  EXPECT_EQ(kRhossHeat, ac.getMode());
  EXPECT_EQ(kRhossMaxTemp, ac.getTemp());
  EXPECT_TRUE(ac.getMax());
  ac.setMax(false);
  EXPECT_EQ(kRhossHeat, ac.getMode());
  EXPECT_EQ(kRhossMaxTemp, ac.getTemp());
  EXPECT_FALSE(ac.getMax());

  ac.setMode(kRhossAuto);
  ac.setTemp(25);
  ac.setMax(true);
  EXPECT_EQ(kRhossAuto, ac.getMode());
  EXPECT_EQ(25, ac.getTemp());
  EXPECT_FALSE(ac.getMax());

  // Test known real data.
  uint8_t lo[kRhossStateLength] = {
      0x01, 0x41, 0x18, 0x00, 0x00, 0x30, 0x03, 0x15};
  uint8_t hi[kRhossStateLength] = {
      0x01, 0x12, 0x40, 0x00, 0x00, 0x30, 0x03, 0x0E};
  ac.setRaw(lo);
  EXPECT_EQ("Power: On, Mode: 1 (Cool), Fan: 4 (Auto), Temp: 12C, Max: On",
            ac.toString());
  ac.setRaw(hi);
  EXPECT_EQ("Power: On, Mode: 2 (Heat), Fan: 1 (Low), Temp: 32C, Max: On",
            ac.toString());
}
*/