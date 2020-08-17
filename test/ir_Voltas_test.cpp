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
      "Power: On, Mode: 4 (Dry), Temp: 24C, Fan: 4 (High), "
      "Turbo: Off, Econo: Off, WiFi: On, Light: Off",
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

TEST(TestIRVoltasClass, Wifi) {
  IRVoltas ac(kGpioUnused);
  ac.begin();

  ac.setWifi(false);
  EXPECT_FALSE(ac.getWifi());
  ac.setWifi(true);
  EXPECT_TRUE(ac.getWifi());
  ac.setWifi(false);
  EXPECT_FALSE(ac.getWifi());
}

TEST(TestIRVoltasClass, Turbo) {
  IRVoltas ac(kGpioUnused);
  ac.begin();

  ac.setTurbo(false);
  EXPECT_FALSE(ac.getTurbo());
  ac.setTurbo(true);
  EXPECT_TRUE(ac.getTurbo());
  ac.setTurbo(false);
  EXPECT_FALSE(ac.getTurbo());
}

TEST(TestIRVoltasClass, Econo) {
  IRVoltas ac(kGpioUnused);
  ac.begin();

  ac.setEcono(false);
  EXPECT_FALSE(ac.getEcono());
  ac.setEcono(true);
  EXPECT_TRUE(ac.getEcono());
  ac.setEcono(false);
  EXPECT_FALSE(ac.getEcono());
}

TEST(TestIRVoltasClass, Light) {
  IRVoltas ac(kGpioUnused);
  ac.begin();

  ac.setLight(false);
  EXPECT_FALSE(ac.getLight());
  ac.setLight(true);
  EXPECT_TRUE(ac.getLight());
  ac.setLight(false);
  EXPECT_FALSE(ac.getLight());
}

TEST(TestVoltasClass, OperatingMode) {
  IRVoltas ac(kGpioUnused);
  ac.begin();

  ac.setMode(kVoltasCool);
  EXPECT_EQ(kVoltasCool, ac.getMode());
  ac.setMode(kVoltasFan);
  EXPECT_EQ(kVoltasFan, ac.getMode());
  ac.setMode(kVoltasDry);
  EXPECT_EQ(kVoltasDry, ac.getMode());
  ac.setMode(kVoltasHeat);
  EXPECT_EQ(kVoltasHeat, ac.getMode());

  ac.setMode(kVoltasCool - 1);
  EXPECT_EQ(kVoltasCool, ac.getMode());

  ac.setMode(kVoltasCool + 1);
  EXPECT_EQ(kVoltasCool, ac.getMode());

  ac.setMode(255);
  EXPECT_EQ(kVoltasCool, ac.getMode());
}

TEST(TestVoltasClass, Temperature) {
  IRVoltas ac(kGpioUnused);
  ac.begin();

  ac.setTemp(kVoltasMinTemp);
  EXPECT_EQ(kVoltasMinTemp, ac.getTemp());

  ac.setTemp(kVoltasMinTemp + 1);
  EXPECT_EQ(kVoltasMinTemp + 1, ac.getTemp());

  ac.setTemp(kVoltasMaxTemp);
  EXPECT_EQ(kVoltasMaxTemp, ac.getTemp());

  ac.setTemp(kVoltasMinTemp - 1);
  EXPECT_EQ(kVoltasMinTemp, ac.getTemp());

  ac.setTemp(kVoltasMaxTemp + 1);
  EXPECT_EQ(kVoltasMaxTemp, ac.getTemp());

  ac.setTemp(23);
  EXPECT_EQ(23, ac.getTemp());

  ac.setTemp(0);
  EXPECT_EQ(kVoltasMinTemp, ac.getTemp());

  ac.setTemp(255);
  EXPECT_EQ(kVoltasMaxTemp, ac.getTemp());
}

TEST(TestVoltasClass, FanSpeed) {
  IRVoltas ac(kGpioUnused);
  ac.begin();
  ac.setFan(kVoltasFanLow);

  ac.setFan(kVoltasFanAuto);
  EXPECT_EQ(kVoltasFanAuto, ac.getFan());

  ac.setFan(kVoltasFanLow);
  EXPECT_EQ(kVoltasFanLow, ac.getFan());
  ac.setFan(kVoltasFanMed);
  EXPECT_EQ(kVoltasFanMed, ac.getFan());
  ac.setFan(kVoltasFanHigh);
  EXPECT_EQ(kVoltasFanHigh, ac.getFan());

  ac.setFan(0);
  EXPECT_EQ(kVoltasFanAuto, ac.getFan());

  ac.setFan(255);
  EXPECT_EQ(kVoltasFanAuto, ac.getFan());
}
