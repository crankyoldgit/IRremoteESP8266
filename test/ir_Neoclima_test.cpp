// Copyright 2019 David Conran

#include "ir_Neoclima.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "gtest/gtest.h"

TEST(TestUtils, Housekeeping) {
  ASSERT_EQ("NEOCLIMA", typeToString(decode_type_t::NEOCLIMA));
  ASSERT_EQ(decode_type_t::NEOCLIMA, strToDecodeType("NEOCLIMA"));
  ASSERT_TRUE(hasACState(decode_type_t::NEOCLIMA));
}

// Test sending typical data only.
TEST(TestSendNeoclima, SendDataOnly) {
  IRsendTest irsend(0);
  irsend.begin();

  uint8_t state[kNeoclimaStateLength] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x6A, 0x00, 0x2A, 0xA5, 0x39};
  irsend.reset();
  irsend.sendNeoclima(state);
  EXPECT_EQ(
      "f38000d50"
      "m6112s7391"
      "m537s571m537s571m537s571m537s571m537s571m537s571m537s571m537s571"
      "m537s571m537s571m537s571m537s571m537s571m537s571m537s571m537s571"
      "m537s571m537s571m537s571m537s571m537s571m537s571m537s571m537s571"
      "m537s571m537s571m537s571m537s571m537s571m537s571m537s571m537s571"
      "m537s571m537s571m537s571m537s571m537s571m537s571m537s571m537s571"
      "m537s571m537s571m537s571m537s571m537s571m537s571m537s571m537s571"
      "m537s571m537s571m537s571m537s571m537s571m537s571m537s571m537s571"
      "m537s571m537s1651m537s571m537s1651m537s571m537s1651m537s1651m537s571"
      "m537s571m537s571m537s571m537s571m537s571m537s571m537s571m537s571"
      "m537s571m537s1651m537s571m537s1651m537s571m537s1651m537s571m537s571"
      "m537s1651m537s571m537s1651m537s571m537s571m537s1651m537s571m537s1651"
      "m537s1651m537s571m537s571m537s1651m537s1651m537s1651m537s571m537s571"
      "m537s7391"
      "m537s100000",
      irsend.outputStr());
}

// https://github.com/markszabo/IRremoteESP8266/issues/764#issuecomment-503755096
TEST(TestDecodeNeoclima, RealExample) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  uint16_t rawData[197] = {
      6112, 7392, 540, 602, 516, 578, 522, 604, 540, 554, 540, 554, 540, 576,
      518, 576, 516, 554, 540, 608, 542, 554, 540, 554, 540, 576, 518, 604, 516,
      556, 540, 576, 546, 580, 542, 578, 542, 602, 518, 554, 542, 554, 568, 582,
      540, 554, 540, 582, 540, 578, 518, 582, 542, 576, 544, 530, 566, 534, 562,
      534, 562, 552, 542, 582, 540, 604, 518, 608, 542, 554, 540, 582, 540, 604,
      518, 580, 540, 606, 544, 554, 542, 554, 542, 580, 542, 576, 520, 554, 540,
      578, 518, 578, 518, 582, 544, 552, 570, 580, 544, 580, 542, 554, 542, 604,
      520, 576, 520, 580, 540, 556, 540, 556, 542, 584, 566, 580, 542, 1622,
      542, 554, 542, 1620, 544, 604, 520, 1642, 518, 1674, 548, 560, 564, 580,
      544, 554, 544, 552, 544, 554, 542, 556, 542, 576, 522, 554, 542, 556, 542,
      580, 542, 1670, 520, 578, 520, 1622, 542, 580, 518, 1646, 520, 558, 568,
      552, 546, 1628, 566, 580, 544, 1668, 522, 576, 520, 578, 520, 1670, 522,
      576, 522, 1670, 496, 1676, 570, 560, 566, 532, 564, 1648, 544, 1670, 522,
      1650, 544, 552, 544, 576, 520, 7390, 544};  // UNKNOWN EE182D95

  uint8_t expectedState[kNeoclimaStateLength] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x6A, 0x00, 0x2A, 0xA5, 0x39};

  irsend.begin();
  irsend.reset();
  irsend.sendRaw(rawData, 197, 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(decode_type_t::NEOCLIMA, irsend.capture.decode_type);
  ASSERT_EQ(kNeoclimaBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
  IRNeoclimaAc ac(0);
  ac.setRaw(irsend.capture.state);
  EXPECT_EQ("Temp: 26C", ac.toString());
}

// Self decode.
TEST(TestDecodeNeoclima, SyntheticExample) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);

  uint8_t expectedState[kNeoclimaStateLength] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x6A, 0x00, 0x2A, 0xA5, 0x39};

  irsend.begin();
  irsend.reset();
  irsend.sendNeoclima(expectedState);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(decode_type_t::NEOCLIMA, irsend.capture.decode_type);
  ASSERT_EQ(kNeoclimaBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
}

TEST(TestIRNeoclimaAcClass, SetAndGetTemp) {
  IRNeoclimaAc ac(0);
  ac.setTemp(25);
  EXPECT_EQ(25, ac.getTemp());
  ac.setTemp(kNeoclimaMinTemp);
  EXPECT_EQ(kNeoclimaMinTemp, ac.getTemp());
  ac.setTemp(kNeoclimaMinTemp - 1);
  EXPECT_EQ(kNeoclimaMinTemp, ac.getTemp());
  ac.setTemp(kNeoclimaMaxTemp);
  EXPECT_EQ(kNeoclimaMaxTemp, ac.getTemp());
  ac.setTemp(kNeoclimaMaxTemp + 1);
  EXPECT_EQ(kNeoclimaMaxTemp, ac.getTemp());
}

TEST(TestIRNeoclimaAcClass, ChecksumCalculation) {
  uint8_t examplestate[kNeoclimaStateLength] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x6A, 0x00, 0x2A, 0xA5, 0x39};
  const uint8_t originalstate[kNeoclimaStateLength] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x6A, 0x00, 0x2A, 0xA5, 0x39};

  EXPECT_TRUE(IRNeoclimaAc::validChecksum(examplestate));
  EXPECT_EQ(0x39, IRNeoclimaAc::calcChecksum(examplestate));

  examplestate[11] = 0x12;  // Set an incorrect checksum.
  EXPECT_FALSE(IRNeoclimaAc::validChecksum(examplestate));
  EXPECT_EQ(0x39, IRNeoclimaAc::calcChecksum(examplestate));
  IRNeoclimaAc ac(0);
  ac.setRaw(examplestate);
  // Extracting the state from the object should have a correct checksum.
  EXPECT_TRUE(IRNeoclimaAc::validChecksum(ac.getRaw()));
  EXPECT_STATE_EQ(originalstate, ac.getRaw(), kNeoclimaBits);
  examplestate[11] = 0x39;  // Restore old checksum value.

  // Change the state to force a different checksum.
  examplestate[8] = 0x01;
  EXPECT_FALSE(IRNeoclimaAc::validChecksum(examplestate));
  EXPECT_EQ(0x3A, IRNeoclimaAc::calcChecksum(examplestate));
}
