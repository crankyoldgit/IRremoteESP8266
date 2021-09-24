// Copyright 2020-2021 David Conran

#include "ir_Mirage.h"
#include "IRac.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

TEST(TestUtils, Housekeeping) {
  ASSERT_EQ("MIRAGE", typeToString(decode_type_t::MIRAGE));
  ASSERT_EQ(decode_type_t::MIRAGE, strToDecodeType("MIRAGE"));
  ASSERT_TRUE(hasACState(decode_type_t::MIRAGE));
  ASSERT_TRUE(IRac::isProtocolSupported(decode_type_t::MIRAGE));
  ASSERT_EQ(kMirageBits, IRsend::defaultBits(decode_type_t::MIRAGE));
  ASSERT_EQ(kMirageMinRepeat, IRsend::minRepeats(decode_type_t::MIRAGE));
}

// Tests for decodeMirage().
// Ref: https://github.com/crankyoldgit/IRremoteESP8266/issues/1289
// Data from:
//   https://github.com/crankyoldgit/IRremoteESP8266/issues/1289#issuecomment-705826015
//   But it is corrected to adjust for poor capture.
TEST(TestDecodeMirage, RealExample) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  const uint16_t rawData[243] = {
      8360, 4248, 582, 518, 556, 1582, 586, 1572, 528, 572, 556, 1590, 526, 572,
      554, 1586, 528, 578, 558, 1582, 556, 542, 558, 1598, 528, 572, 556, 1590,
      528, 1610, 556, 1600, 554, 546, 556, 544, 558, 542, 558, 542, 676, 400,
      606, 492, 582, 542, 556, 544, 556, 542, 558, 544, 556, 542, 556, 544, 558,
      542, 556, 544, 530, 570, 586, 516, 584, 514, 558, 542, 558, 542, 558, 542,
      554, 546, 558, 542, 558, 1582, 534, 542, 580, 552, 528, 1610, 556, 544,
      554, 546, 554, 544, 556, 544, 556, 544, 558, 542, 558, 552, 558, 542, 558,
      542, 558, 542, 556, 544, 558, 542, 558, 542, 554, 544, 584, 516, 558, 542,
      528, 572, 588, 512, 556, 544, 532, 568, 560, 542, 558, 542, 560, 540, 560,
      538, 530, 570, 558, 542, 558, 542, 560, 540, 558, 542, 558, 542, 530, 568,
      558, 542, 558, 542, 532, 570, 530, 570, 558, 542, 558, 542, 558, 542, 530,
      570, 530, 568, 560, 540, 560, 540, 532, 568, 558, 542, 558, 542, 532, 568,
      560, 542, 532, 568, 532, 568, 530, 570, 532, 570, 530, 570, 558, 540, 560,
      540, 558, 534, 558, 542, 556, 1600, 558, 1592, 558, 542, 560, 1590, 530,
      570, 530, 570, 556, 544, 560, 540, 556, 544, 558, 1582, 556, 544, 558,
      1600, 556, 542, 560, 542, 532, 568, 558, 542, 610, 1538, 504, 1646, 582,
      518, 528, 572, 528, 1612, 556, 544, 528, 580, 554};  // UNKNOWN 28DACDC4
  const uint8_t expected[kMirageStateLength] = {
      0x56, 0x75, 0x00, 0x00, 0x20, 0x01, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x16, 0x14, 0x26};
  irsend.begin();
  irsend.reset();
  irsend.sendRaw(rawData, 243, 38);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(decode_type_t::MIRAGE, irsend.capture.decode_type);
  ASSERT_EQ(kMirageBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expected, irsend.capture.state, irsend.capture.bits);
  EXPECT_EQ(
      "Power: On, Mode: 2 (Cool), Temp: 25C, Fan: 0 (Auto), "
      "Turbo: Off, Light: Off, Sleep: Off, Clock: 14:16",
      IRAcUtils::resultAcToString(&irsend.capture));
}

TEST(TestDecodeMirage, SyntheticExample) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  const uint8_t expected[kMirageStateLength] = {
      0x56, 0x75, 0x00, 0x00, 0x20, 0x01, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x16, 0x14, 0x26};
  irsend.begin();
  irsend.reset();
  irsend.sendMirage(expected);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(decode_type_t::MIRAGE, irsend.capture.decode_type);
  ASSERT_EQ(kMirageBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expected, irsend.capture.state, irsend.capture.bits);
  EXPECT_EQ(
      "Power: On, Mode: 2 (Cool), Temp: 25C, Fan: 0 (Auto), "
      "Turbo: Off, Light: Off, Sleep: Off, Clock: 14:16",
      IRAcUtils::resultAcToString(&irsend.capture));
}

// Data from:
//   https://github.com/crankyoldgit/IRremoteESP8266/issues/1289#issuecomment-705624234
TEST(TestDecodeMirage, RealExampleWithDodgyHardwareCapture) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  const uint16_t rawData[243] = {
      8360, 4248, 582, 518, 556, 1582, 586, 1572, 528, 572, 556, 1590, 526, 572,
      554, 1586, 528, 578, 558, 1582, 556, 542, 558, 1598, 528, 572, 556, 1590,
      528, 1610, 556, 1600, 554, 546, 556, 544, 558, 542, 558, 542, 676, 400,
      606, 492, 582, 542, 556, 544, 556, 542, 558, 544, 556, 542, 556, 544, 558,
      542, 556, 544, 530, 570, 586, 516, 584, 514, 558, 542, 558, 542, 558, 542,
      554, 546, 558, 542, 558, 1582,
      734, 342,  // Really poor data here.
      580, 552, 528, 1610, 556, 544, 554, 546, 554, 544, 556, 544, 556, 544,
      558, 542, 558, 552, 558, 542, 558, 542, 558, 542, 556, 544, 558, 542, 558,
      542, 554, 544, 584, 516, 558, 542, 528, 572, 588, 512, 556, 544, 532, 568,
      560, 542, 558, 542, 560, 540, 560, 538, 530, 570, 558, 542, 558, 542, 560,
      540, 558, 542, 558, 542, 530, 568, 558, 542, 558, 542, 532, 570, 530, 570,
      558, 542, 558, 542, 558, 542, 530, 570, 530, 568, 560, 540, 560, 540, 532,
      568, 558, 542, 558, 542, 532, 568, 560, 542, 532, 568, 532, 568, 530, 570,
      532, 570, 530, 570, 558, 540, 560, 540, 558, 534, 558, 542, 556, 1600,
      558, 1592, 558, 542, 560, 1590, 530, 570, 530, 570, 556, 544, 560, 540,
      556, 544, 558, 1582, 556, 544, 558, 1600, 556, 542, 560, 542, 532, 568,
      558, 542, 610, 1538, 504, 1646, 582, 518, 528, 572, 528, 1612, 556, 544,
      528, 580, 554};  // UNKNOWN 28DACDC4
  const uint8_t expected[kMirageStateLength] = {
      0x56, 0x75, 0x00, 0x00, 0x20, 0x01, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x16, 0x14, 0x26};
  irrecv.setTolerance(kTolerance + 10);  // Bump tolerance to match poor data.
  irsend.begin();
  irsend.reset();
  irsend.sendRaw(rawData, 243, 38);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(decode_type_t::MIRAGE, irsend.capture.decode_type);
  ASSERT_EQ(kMirageBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expected, irsend.capture.state, irsend.capture.bits);
  EXPECT_EQ(
      "Power: On, Mode: 2 (Cool), Temp: 25C, Fan: 0 (Auto), "
      "Turbo: Off, Light: Off, Sleep: Off, Clock: 14:16",
      IRAcUtils::resultAcToString(&irsend.capture));
}

TEST(TestMirageAcClass, Power) {
  IRMirageAc ac(kGpioUnused);
  ac.begin();

  ac.on();
  EXPECT_TRUE(ac.getPower());
  ac.on();
  EXPECT_TRUE(ac.getPower());

  ac.off();
  EXPECT_FALSE(ac.getPower());
  ac.off();
  EXPECT_FALSE(ac.getPower());

  ac.setPower(true);
  EXPECT_TRUE(ac.getPower());

  ac.setPower(false);
  EXPECT_FALSE(ac.getPower());

  const uint8_t on[kMirageStateLength] = {
      0x56, 0x75, 0x00, 0x00, 0x20, 0x01, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x16, 0x14, 0x26};
  ac.setRaw(on);
  EXPECT_TRUE(ac.getPower());
  const uint8_t off[kMirageStateLength] = {
      0x56, 0x6C, 0x00, 0x00, 0x21, 0xD8, 0x00, 0x00,
      0x0C, 0x00, 0x0C, 0x2C, 0x23, 0x01, 0x61};
  ac.setRaw(off);
  EXPECT_FALSE(ac.getPower());
}

TEST(TestMirageAcClass, OperatingMode) {
  IRMirageAc ac(kGpioUnused);
  ac.begin();

  ac.setMode(kMirageAcCool);
  EXPECT_EQ(kMirageAcCool, ac.getMode());
  ac.setMode(kMirageAcHeat);
  EXPECT_EQ(kMirageAcHeat, ac.getMode());
  ac.setMode(kMirageAcDry);
  EXPECT_EQ(kMirageAcDry, ac.getMode());
  ac.setMode(kMirageAcFan);
  EXPECT_EQ(kMirageAcFan, ac.getMode());
  ac.setMode(kMirageAcRecycle);
  EXPECT_EQ(kMirageAcRecycle, ac.getMode());
  ac.setMode(255);
  EXPECT_EQ(kMirageAcCool, ac.getMode());
}

TEST(TestMirageAcClass, HumanReadable) {
  IRMirageAc ac(kGpioUnused);
  ac.begin();
  EXPECT_EQ(
      "Power: On, Mode: 2 (Cool), Temp: 16C, Fan: 0 (Auto), "
      "Turbo: Off, Light: Off, Sleep: Off, Clock: 00:00",
      ac.toString());
  // Ref: https://docs.google.com/spreadsheets/d/1Ucu9mOOIIJoWQjUJq_VCvwgV3EwKaRk8K2AuZgccYEk/edit#gid=0&range=C7
  // 0x56710000201A00000C000C26010041
  const uint8_t cool_21c_auto[kMirageStateLength] = {
      0x56, 0x71, 0x00, 0x00, 0x20, 0x1A, 0x00, 0x00,
      0x0C, 0x00, 0x0C, 0x26, 0x01, 0x00, 0x41};
  ac.setRaw(cool_21c_auto);
  EXPECT_EQ(
      "Power: On, Mode: 2 (Cool), Temp: 21C, Fan: 0 (Auto), "
      "Turbo: Off, Light: Off, Sleep: Off, Clock: 00:01",
      ac.toString());

  const uint8_t SyntheticExample[kMirageStateLength] = {
      0x56, 0x75, 0x00, 0x00, 0x20, 0x01, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x16, 0x14, 0x26};
  ac.setRaw(SyntheticExample);
  EXPECT_EQ(
      "Power: On, Mode: 2 (Cool), Temp: 25C, Fan: 0 (Auto), "
      "Turbo: Off, Light: Off, Sleep: Off, Clock: 14:16",
      ac.toString());
}

TEST(TestMirageAcClass, Temperature) {
  IRMirageAc ac(kGpioUnused);
  ac.begin();

  ac.setTemp(0);
  EXPECT_EQ(kMirageAcMinTemp, ac.getTemp());

  ac.setTemp(255);
  EXPECT_EQ(kMirageAcMaxTemp, ac.getTemp());

  ac.setTemp(kMirageAcMinTemp);
  EXPECT_EQ(kMirageAcMinTemp, ac.getTemp());

  ac.setTemp(kMirageAcMaxTemp);
  EXPECT_EQ(kMirageAcMaxTemp, ac.getTemp());

  ac.setTemp(kMirageAcMinTemp - 1);
  EXPECT_EQ(kMirageAcMinTemp, ac.getTemp());

  ac.setTemp(kMirageAcMaxTemp + 1);
  EXPECT_EQ(kMirageAcMaxTemp, ac.getTemp());

  ac.setTemp(17);
  EXPECT_EQ(17, ac.getTemp());

  ac.setTemp(21);
  EXPECT_EQ(21, ac.getTemp());

  ac.setTemp(25);
  EXPECT_EQ(25, ac.getTemp());

  ac.setTemp(30);
  EXPECT_EQ(30, ac.getTemp());
}

TEST(TestMirageAcClass, FanSpeed) {
  IRMirageAc ac(kGpioUnused);
  ac.begin();

  ac.setFan(kMirageAcFanAuto);
  EXPECT_EQ(kMirageAcFanAuto, ac.getFan());
  ac.setFan(kMirageAcFanLow);
  EXPECT_EQ(kMirageAcFanLow, ac.getFan());
  ac.setFan(kMirageAcFanMed);
  EXPECT_EQ(kMirageAcFanMed, ac.getFan());
  ac.setFan(kMirageAcFanHigh);
  EXPECT_EQ(kMirageAcFanHigh, ac.getFan());

  ac.setFan(255);
  EXPECT_EQ(kMirageAcFanAuto, ac.getFan());
}

TEST(TestMirageAcClass, Turbo) {
  IRMirageAc ac(kGpioUnused);
  ac.begin();

  ac.setTurbo(true);
  EXPECT_TRUE(ac.getTurbo());
  ac.setTurbo(false);
  EXPECT_FALSE(ac.getTurbo());
  ac.setTurbo(true);
  EXPECT_TRUE(ac.getTurbo());
}

TEST(TestMirageAcClass, Light) {
  IRMirageAc ac(kGpioUnused);
  ac.begin();

  ac.setLight(true);
  EXPECT_TRUE(ac.getLight());
  ac.setLight(false);
  EXPECT_FALSE(ac.getLight());
  ac.setLight(true);
  EXPECT_TRUE(ac.getLight());
}

TEST(TestMirageAcClass, Sleep) {
  IRMirageAc ac(kGpioUnused);
  ac.begin();

  ac.setSleep(true);
  EXPECT_TRUE(ac.getSleep());
  ac.setSleep(false);
  EXPECT_FALSE(ac.getSleep());
  ac.setSleep(true);
  EXPECT_TRUE(ac.getSleep());
}

TEST(TestMirageAcClass, Clock) {
  IRMirageAc ac(kGpioUnused);
  ac.begin();

  ac.setClock(0);
  EXPECT_EQ(0, ac.getClock());
  ac.setClock(12 * 60 * 60 + 30 * 60 + 59);  // aka. 12:30:59
  EXPECT_EQ(12 * 60 * 60 + 30 * 60 + 59, ac.getClock());
  ac.setClock(23 * 60 * 60 + 59 * 60 + 59);  // aka. 23:59:59
  EXPECT_EQ(23 * 60 * 60 + 59 * 60 + 59, ac.getClock());
  ac.setClock(24 * 60 * 60);  // aka. 24:00:00
  EXPECT_EQ(23 * 60 * 60 + 59 * 60 + 59, ac.getClock());  // aka. 23:59:59
}
