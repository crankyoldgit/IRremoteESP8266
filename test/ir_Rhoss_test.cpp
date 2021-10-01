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
/*
// Tests for IRAmcorAc class.

TEST(TestAmcorAcClass, Power) {
  IRAmcorAc ac(0);
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

TEST(TestAmcorAcClass, Temperature) {
  IRAmcorAc ac(0);
  ac.begin();

  ac.setTemp(0);
  EXPECT_EQ(kAmcorMinTemp, ac.getTemp());

  ac.setTemp(255);
  EXPECT_EQ(kAmcorMaxTemp, ac.getTemp());

  ac.setTemp(kAmcorMinTemp);
  EXPECT_EQ(kAmcorMinTemp, ac.getTemp());

  ac.setTemp(kAmcorMaxTemp);
  EXPECT_EQ(kAmcorMaxTemp, ac.getTemp());

  ac.setTemp(kAmcorMinTemp - 1);
  EXPECT_EQ(kAmcorMinTemp, ac.getTemp());

  ac.setTemp(kAmcorMaxTemp + 1);
  EXPECT_EQ(kAmcorMaxTemp, ac.getTemp());

  ac.setTemp(17);
  EXPECT_EQ(17, ac.getTemp());

  ac.setTemp(21);
  EXPECT_EQ(21, ac.getTemp());

  ac.setTemp(25);
  EXPECT_EQ(25, ac.getTemp());

  ac.setTemp(29);
  EXPECT_EQ(29, ac.getTemp());
}

TEST(TestAmcorAcClass, OperatingMode) {
  IRAmcorAc ac(0);
  ac.begin();

  ac.setMode(kAmcorAuto);
  EXPECT_EQ(kAmcorAuto, ac.getMode());

  ac.setMode(kAmcorCool);
  EXPECT_EQ(kAmcorCool, ac.getMode());

  ac.setMode(kAmcorHeat);
  EXPECT_EQ(kAmcorHeat, ac.getMode());

  ac.setMode(kAmcorDry);
  EXPECT_EQ(kAmcorDry, ac.getMode());

  ac.setMode(kAmcorFan);
  EXPECT_EQ(kAmcorFan, ac.getMode());

  ac.setMode(kAmcorAuto + 1);
  EXPECT_EQ(kAmcorAuto, ac.getMode());

  ac.setMode(255);
  EXPECT_EQ(kAmcorAuto, ac.getMode());
}

TEST(TestAmcorAcClass, FanSpeed) {
  IRAmcorAc ac(0);
  ac.begin();

  ac.setFan(0);
  EXPECT_EQ(kAmcorFanAuto, ac.getFan());

  ac.setFan(255);
  EXPECT_EQ(kAmcorFanAuto, ac.getFan());

  ac.setFan(kAmcorFanMax);
  EXPECT_EQ(kAmcorFanMax, ac.getFan());

  ac.setFan(kAmcorFanMax + 1);
  EXPECT_EQ(kAmcorFanAuto, ac.getFan());

  ac.setFan(kAmcorFanMax - 1);
  EXPECT_EQ(kAmcorFanMax - 1, ac.getFan());

  ac.setFan(1);
  EXPECT_EQ(1, ac.getFan());

  ac.setFan(1);
  EXPECT_EQ(1, ac.getFan());

  ac.setFan(3);
  EXPECT_EQ(3, ac.getFan());
}

TEST(TestAmcorAcClass, Checksums) {
  uint8_t state[kAmcorStateLength] = {
      0x01, 0x41, 0x30, 0x00, 0x00, 0x30, 0x00, 0x0C};

  ASSERT_EQ(0x0C, IRAmcorAc::calcChecksum(state));
  EXPECT_TRUE(IRAmcorAc::validChecksum(state));
  // Change the array so the checksum is invalid.
  state[0] ^= 0xFF;
  EXPECT_FALSE(IRAmcorAc::validChecksum(state));
  // Restore the previous change, and change another byte.
  state[0] ^= 0xFF;
  state[4] ^= 0xFF;
  EXPECT_FALSE(IRAmcorAc::validChecksum(state));
  state[4] ^= 0xFF;
  EXPECT_TRUE(IRAmcorAc::validChecksum(state));

  // Additional known good states.
  uint8_t knownGood1[kAmcorStateLength] = {
      0x01, 0x11, 0x3E, 0x00, 0x00, 0x30, 0x00, 0x17};
  EXPECT_TRUE(IRAmcorAc::validChecksum(knownGood1));
  ASSERT_EQ(0x17, IRAmcorAc::calcChecksum(knownGood1));
  uint8_t knownGood2[kAmcorStateLength] = {
      0x01, 0x22, 0x26, 0x00, 0x00, 0x30, 0x00, 0x10};
  EXPECT_TRUE(IRAmcorAc::validChecksum(knownGood2));
  ASSERT_EQ(0x10, IRAmcorAc::calcChecksum(knownGood2));
  uint8_t knownGood3[kAmcorStateLength] = {
      0x01, 0x41, 0x24, 0x00, 0x00, 0xC0, 0x00, 0x18};
  EXPECT_TRUE(IRAmcorAc::validChecksum(knownGood3));
  ASSERT_EQ(0x18, IRAmcorAc::calcChecksum(knownGood3));

  // For a recalculation.
  uint8_t knownBad[kAmcorStateLength] = {
      // Same as knownGood3 except for the checksum.
      0x01, 0x41, 0x24, 0x00, 0x00, 0xC0, 0x00, 0x00};
  EXPECT_FALSE(IRAmcorAc::validChecksum(knownBad));
  IRAmcorAc ac(0);
  ac.setRaw(knownBad);
  EXPECT_STATE_EQ(knownGood3, ac.getRaw(), kAmcorBits);
}

TEST(TestAmcorAcClass, Max) {
  IRAmcorAc ac(0);
  ac.begin();

  ac.setMode(kAmcorCool);
  ac.setMax(true);
  EXPECT_EQ(kAmcorCool, ac.getMode());
  EXPECT_EQ(kAmcorMinTemp, ac.getTemp());
  EXPECT_TRUE(ac.getMax());
  ac.setMax(false);
  EXPECT_EQ(kAmcorCool, ac.getMode());
  EXPECT_EQ(kAmcorMinTemp, ac.getTemp());
  EXPECT_FALSE(ac.getMax());

  ac.setMode(kAmcorHeat);
  ac.setMax(true);
  EXPECT_EQ(kAmcorHeat, ac.getMode());
  EXPECT_EQ(kAmcorMaxTemp, ac.getTemp());
  EXPECT_TRUE(ac.getMax());
  ac.setMax(false);
  EXPECT_EQ(kAmcorHeat, ac.getMode());
  EXPECT_EQ(kAmcorMaxTemp, ac.getTemp());
  EXPECT_FALSE(ac.getMax());

  ac.setMode(kAmcorAuto);
  ac.setTemp(25);
  ac.setMax(true);
  EXPECT_EQ(kAmcorAuto, ac.getMode());
  EXPECT_EQ(25, ac.getTemp());
  EXPECT_FALSE(ac.getMax());

  // Test known real data.
  uint8_t lo[kAmcorStateLength] = {
      0x01, 0x41, 0x18, 0x00, 0x00, 0x30, 0x03, 0x15};
  uint8_t hi[kAmcorStateLength] = {
      0x01, 0x12, 0x40, 0x00, 0x00, 0x30, 0x03, 0x0E};
  ac.setRaw(lo);
  EXPECT_EQ("Power: On, Mode: 1 (Cool), Fan: 4 (Auto), Temp: 12C, Max: On",
            ac.toString());
  ac.setRaw(hi);
  EXPECT_EQ("Power: On, Mode: 2 (Heat), Fan: 1 (Low), Temp: 32C, Max: On",
            ac.toString());
}
*/