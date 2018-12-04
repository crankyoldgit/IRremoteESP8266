// Copyright 2017 David Conran
#include "ir_Daikin.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

// Tests for sendDaikin().

// Test sending typical data only.
TEST(TestSendDaikin, SendDataOnly) {
  IRsendTest irsend(4);
  irsend.begin();

  uint8_t daikin_code[kDaikinStateLength] = {
      0x11, 0xDA, 0x27, 0xF0, 0x00, 0x00, 0x00, 0x20, 0x11,
      0xDA, 0x27, 0x00, 0x00, 0x41, 0x1E, 0x00, 0xB0, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0xE3};

  irsend.reset();
  irsend.sendDaikin(daikin_code);
  EXPECT_EQ(
      "m428s428m428s428m428s428m428s428m428s428"
      "m428s29428m3650s1623"
      "m428s1280m428s428m428s428m428s428m428s1280m428s428m428s428m428s428"
      "m428s428m428s1280m428s428m428s1280m428s1280m428s428m428s1280m428s1280"
      "m428s1280m428s1280m428s1280m428s428m428s428m428s1280m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s1280m428s428m428s1280m428s428m428s428m428s428m428s1280m428s1280"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s1280m428s1280m428s1280m428s428m428s1280m428s428m428s1280m428s1280"
      "m428s29428m3650s1623"
      "m428s1280m428s428m428s428m428s428m428s1280m428s428m428s428m428s428"
      "m428s428m428s1280m428s428m428s1280m428s1280m428s428m428s1280m428s1280"
      "m428s1280m428s1280m428s1280m428s428m428s428m428s1280m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s1280m428s1280m428s1280m428s1280"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s1280m428s428m428s428"
      "m428s29428m3650s1623"
      "m428s1280m428s428m428s428m428s428m428s1280m428s428m428s428m428s428"
      "m428s428m428s1280m428s428m428s1280m428s1280m428s428m428s1280m428s1280"
      "m428s1280m428s1280m428s1280m428s428m428s428m428s1280m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s1280m428s428m428s428m428s428m428s428m428s428m428s1280m428s428"
      "m428s428m428s1280m428s1280m428s1280m428s1280m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s1280m428s1280m428s428m428s1280"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s1280m428s1280"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s1280m428s1280m428s428m428s428m428s428m428s1280m428s1280m428s1280"
      "m428s29428",
      irsend.outputStr());
}

// Test sending with repeats.
TEST(TestSendDaikin, SendWithRepeats) {
  IRsendTest irsend(4);
  irsend.begin();

  irsend.reset();
  uint8_t daikin_code[kDaikinStateLength] = {
      0x11, 0xDA, 0x27, 0xF0, 0x00, 0x00, 0x00, 0x20, 0x11,
      0xDA, 0x27, 0x00, 0x00, 0x41, 0x1E, 0x00, 0xB0, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0xE3};
  irsend.reset();

  irsend.sendDaikin(daikin_code, kDaikinStateLength, 1);
  EXPECT_EQ(
      "m428s428m428s428m428s428m428s428m428s428"
      "m428s29428m3650s1623"
      "m428s1280m428s428m428s428m428s428m428s1280m428s428m428s428m428s428"
      "m428s428m428s1280m428s428m428s1280m428s1280m428s428m428s1280m428s1280"
      "m428s1280m428s1280m428s1280m428s428m428s428m428s1280m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s1280m428s428m428s1280m428s428m428s428m428s428m428s1280m428s1280"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s1280m428s1280m428s1280m428s428m428s1280m428s428m428s1280m428s1280"
      "m428s29428m3650s1623"
      "m428s1280m428s428m428s428m428s428m428s1280m428s428m428s428m428s428"
      "m428s428m428s1280m428s428m428s1280m428s1280m428s428m428s1280m428s1280"
      "m428s1280m428s1280m428s1280m428s428m428s428m428s1280m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s1280m428s1280m428s1280m428s1280"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s1280m428s428m428s428"
      "m428s29428m3650s1623"
      "m428s1280m428s428m428s428m428s428m428s1280m428s428m428s428m428s428"
      "m428s428m428s1280m428s428m428s1280m428s1280m428s428m428s1280m428s1280"
      "m428s1280m428s1280m428s1280m428s428m428s428m428s1280m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s1280m428s428m428s428m428s428m428s428m428s428m428s1280m428s428"
      "m428s428m428s1280m428s1280m428s1280m428s1280m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s1280m428s1280m428s428m428s1280"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s1280m428s1280"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s1280m428s1280m428s428m428s428m428s428m428s1280m428s1280m428s1280"
      "m428s29428"
      "m428s428m428s428m428s428m428s428m428s428"
      "m428s29428m3650s1623"
      "m428s1280m428s428m428s428m428s428m428s1280m428s428m428s428m428s428"
      "m428s428m428s1280m428s428m428s1280m428s1280m428s428m428s1280m428s1280"
      "m428s1280m428s1280m428s1280m428s428m428s428m428s1280m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s1280m428s428m428s1280m428s428m428s428m428s428m428s1280m428s1280"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s1280m428s1280m428s1280m428s428m428s1280m428s428m428s1280m428s1280"
      "m428s29428m3650s1623"
      "m428s1280m428s428m428s428m428s428m428s1280m428s428m428s428m428s428"
      "m428s428m428s1280m428s428m428s1280m428s1280m428s428m428s1280m428s1280"
      "m428s1280m428s1280m428s1280m428s428m428s428m428s1280m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s1280m428s1280m428s1280m428s1280"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s1280m428s428m428s428"
      "m428s29428m3650s1623"
      "m428s1280m428s428m428s428m428s428m428s1280m428s428m428s428m428s428"
      "m428s428m428s1280m428s428m428s1280m428s1280m428s428m428s1280m428s1280"
      "m428s1280m428s1280m428s1280m428s428m428s428m428s1280m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s1280m428s428m428s428m428s428m428s428m428s428m428s1280m428s428"
      "m428s428m428s1280m428s1280m428s1280m428s1280m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s1280m428s1280m428s428m428s1280"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s1280m428s1280"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s1280m428s1280m428s428m428s428m428s428m428s1280m428s1280m428s1280"
      "m428s29428",
      irsend.outputStr());
}

// Test sending atypical sizes.
TEST(TestSendDaikin, SendUnexpectedSizes) {
  IRsendTest irsend(4);
  irsend.begin();

  uint8_t daikin_short_code[kDaikinStateLength - 1] = {
      0x11, 0xDA, 0x27, 0xF0, 0x00, 0x00, 0x00, 0x20, 0x11,
      0xDA, 0x27, 0x00, 0x00, 0x41, 0x1E, 0x00, 0xB0, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00};

  irsend.reset();
  irsend.sendDaikin(daikin_short_code, kDaikinStateLength - 1);
  ASSERT_EQ("", irsend.outputStr());

  uint8_t daikin_long_code[kDaikinStateLength + 1] = {
      0x11, 0xDA, 0x27, 0xF0, 0x00, 0x00, 0x00, 0x20, 0x11, 0xDA,
      0x27, 0x00, 0x00, 0x41, 0x1E, 0x00, 0xB0, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0xE3, 0x11};
  irsend.reset();
  irsend.sendDaikin(daikin_long_code, kDaikinStateLength + 1);
  ASSERT_EQ(
      "m428s428m428s428m428s428m428s428m428s428"
      "m428s29428m3650s1623"
      "m428s1280m428s428m428s428m428s428m428s1280m428s428m428s428m428s428"
      "m428s428m428s1280m428s428m428s1280m428s1280m428s428m428s1280m428s1280"
      "m428s1280m428s1280m428s1280m428s428m428s428m428s1280m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s1280m428s428m428s1280m428s428m428s428m428s428m428s1280m428s1280"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s1280m428s1280m428s1280m428s428m428s1280m428s428m428s1280m428s1280"
      "m428s29428m3650s1623"
      "m428s1280m428s428m428s428m428s428m428s1280m428s428m428s428m428s428"
      "m428s428m428s1280m428s428m428s1280m428s1280m428s428m428s1280m428s1280"
      "m428s1280m428s1280m428s1280m428s428m428s428m428s1280m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s1280m428s1280m428s1280m428s1280"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s1280m428s428m428s428"
      "m428s29428m3650s1623"
      "m428s1280m428s428m428s428m428s428m428s1280m428s428m428s428m428s428"
      "m428s428m428s1280m428s428m428s1280m428s1280m428s428m428s1280m428s1280"
      "m428s1280m428s1280m428s1280m428s428m428s428m428s1280m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s1280m428s428m428s428m428s428m428s428m428s428m428s1280m428s428"
      "m428s428m428s1280m428s1280m428s1280m428s1280m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s1280m428s1280m428s428m428s1280"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s1280m428s1280"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s1280m428s1280m428s428m428s428m428s428m428s1280m428s1280m428s1280"
      "m428s1280m428s428m428s428m428s428m428s1280m428s428m428s428m428s428"
      "m428s29428",
      irsend.outputStr());
}

// Tests for IRDaikinESP class.

TEST(TestDaikinClass, Power) {
  IRDaikinESP irdaikin(0);
  irdaikin.begin();

  irdaikin.on();
  EXPECT_TRUE(irdaikin.getPower());

  irdaikin.off();
  EXPECT_FALSE(irdaikin.getPower());

  irdaikin.setPower(true);
  EXPECT_TRUE(irdaikin.getPower());

  irdaikin.setPower(false);
  EXPECT_FALSE(irdaikin.getPower());
}

TEST(TestDaikinClass, Temperature) {
  IRDaikinESP irdaikin(0);
  irdaikin.begin();

  irdaikin.setTemp(0);
  EXPECT_EQ(kDaikinMinTemp, irdaikin.getTemp());

  irdaikin.setTemp(255);
  EXPECT_EQ(kDaikinMaxTemp, irdaikin.getTemp());

  irdaikin.setTemp(kDaikinMinTemp);
  EXPECT_EQ(kDaikinMinTemp, irdaikin.getTemp());

  irdaikin.setTemp(kDaikinMaxTemp);
  EXPECT_EQ(kDaikinMaxTemp, irdaikin.getTemp());

  irdaikin.setTemp(kDaikinMinTemp - 1);
  EXPECT_EQ(kDaikinMinTemp, irdaikin.getTemp());

  irdaikin.setTemp(kDaikinMaxTemp + 1);
  EXPECT_EQ(kDaikinMaxTemp, irdaikin.getTemp());

  irdaikin.setTemp(kDaikinMinTemp + 1);
  EXPECT_EQ(kDaikinMinTemp + 1, irdaikin.getTemp());

  irdaikin.setTemp(21);
  EXPECT_EQ(21, irdaikin.getTemp());

  irdaikin.setTemp(25);
  EXPECT_EQ(25, irdaikin.getTemp());

  irdaikin.setTemp(29);
  EXPECT_EQ(29, irdaikin.getTemp());
}

TEST(TestDaikinClass, OperatingMode) {
  IRDaikinESP irdaikin(0);
  irdaikin.begin();

  irdaikin.setMode(kDaikinAuto);
  EXPECT_EQ(kDaikinAuto, irdaikin.getMode());

  irdaikin.setMode(kDaikinCool);
  EXPECT_EQ(kDaikinCool, irdaikin.getMode());

  irdaikin.setMode(kDaikinHeat);
  EXPECT_EQ(kDaikinHeat, irdaikin.getMode());

  irdaikin.setMode(kDaikinDry);
  EXPECT_EQ(kDaikinDry, irdaikin.getMode());

  irdaikin.setMode(kDaikinFan);
  EXPECT_EQ(kDaikinFan, irdaikin.getMode());

  irdaikin.setMode(kDaikinFan + 1);
  EXPECT_EQ(kDaikinAuto, irdaikin.getMode());

  irdaikin.setMode(kDaikinAuto + 1);
  EXPECT_EQ(kDaikinAuto, irdaikin.getMode());

  irdaikin.setMode(255);
  EXPECT_EQ(kDaikinAuto, irdaikin.getMode());
}

TEST(TestDaikinClass, VaneSwing) {
  IRDaikinESP irdaikin(0);
  irdaikin.begin();

  irdaikin.setSwingHorizontal(true);
  irdaikin.setSwingVertical(false);

  irdaikin.setSwingHorizontal(true);
  EXPECT_TRUE(irdaikin.getSwingHorizontal());
  EXPECT_FALSE(irdaikin.getSwingVertical());

  irdaikin.setSwingVertical(true);
  EXPECT_TRUE(irdaikin.getSwingHorizontal());
  EXPECT_TRUE(irdaikin.getSwingVertical());

  irdaikin.setSwingHorizontal(false);
  EXPECT_FALSE(irdaikin.getSwingHorizontal());
  EXPECT_TRUE(irdaikin.getSwingVertical());

  irdaikin.setSwingVertical(false);
  EXPECT_FALSE(irdaikin.getSwingHorizontal());
  EXPECT_FALSE(irdaikin.getSwingVertical());
}

TEST(TestDaikinClass, QuietMode) {
  IRDaikinESP irdaikin(0);
  irdaikin.begin();

  irdaikin.setQuiet(true);
  EXPECT_TRUE(irdaikin.getQuiet());

  irdaikin.setQuiet(false);
  EXPECT_FALSE(irdaikin.getQuiet());

  irdaikin.setQuiet(true);
  EXPECT_TRUE(irdaikin.getQuiet());

  // Setting Econo mode should NOT change out of quiet mode.
  irdaikin.setEcono(true);
  EXPECT_TRUE(irdaikin.getQuiet());
  irdaikin.setEcono(false);
  EXPECT_TRUE(irdaikin.getQuiet());

  // But setting Powerful mode should exit out of quiet mode.
  irdaikin.setPowerful(true);
  EXPECT_FALSE(irdaikin.getQuiet());
}

TEST(TestDaikinClass, PowerfulMode) {
  IRDaikinESP irdaikin(0);
  irdaikin.begin();

  irdaikin.setPowerful(true);
  EXPECT_TRUE(irdaikin.getPowerful());

  irdaikin.setPowerful(false);
  EXPECT_FALSE(irdaikin.getPowerful());

  irdaikin.setPowerful(true);
  EXPECT_TRUE(irdaikin.getPowerful());

  irdaikin.setQuiet(true);
  EXPECT_FALSE(irdaikin.getPowerful());

  irdaikin.setPowerful(true);
  irdaikin.setEcono(true);
  EXPECT_FALSE(irdaikin.getPowerful());
}

TEST(TestDaikinClass, EconoMode) {
  IRDaikinESP irdaikin(0);
  irdaikin.begin();

  irdaikin.setEcono(true);
  EXPECT_TRUE(irdaikin.getEcono());

  irdaikin.setEcono(false);
  EXPECT_FALSE(irdaikin.getEcono());

  irdaikin.setEcono(true);
  EXPECT_TRUE(irdaikin.getEcono());

  // Setting Quiet mode should NOT change out of Econo mode.
  irdaikin.setQuiet(true);
  EXPECT_TRUE(irdaikin.getEcono());
  irdaikin.setQuiet(false);
  EXPECT_TRUE(irdaikin.getEcono());

  // But setting Powerful mode should exit out of Econo mode.
  irdaikin.setPowerful(true);
  EXPECT_FALSE(irdaikin.getEcono());
}

TEST(TestDaikinClass, FanSpeed) {
  IRDaikinESP irdaikin(0);
  irdaikin.begin();

  // Unexpected value should default to Auto.
  irdaikin.setFan(0);
  EXPECT_EQ(kDaikinFanAuto, irdaikin.getFan());

  // Unexpected value should default to Auto.
  irdaikin.setFan(255);
  EXPECT_EQ(kDaikinFanAuto, irdaikin.getFan());

  irdaikin.setFan(kDaikinFanMax);
  EXPECT_EQ(kDaikinFanMax, irdaikin.getFan());

  // Beyond Max should default to Auto.
  irdaikin.setFan(kDaikinFanMax + 1);
  EXPECT_EQ(kDaikinFanAuto, irdaikin.getFan());

  irdaikin.setFan(kDaikinFanMax - 1);
  EXPECT_EQ(kDaikinFanMax - 1, irdaikin.getFan());

  irdaikin.setFan(kDaikinFanMin);
  EXPECT_EQ(kDaikinFanMin, irdaikin.getFan());

  irdaikin.setFan(kDaikinFanMin + 1);
  EXPECT_EQ(kDaikinFanMin + 1, irdaikin.getFan());

  // Beyond Min should default to Auto.
  irdaikin.setFan(kDaikinFanMin - 1);
  EXPECT_EQ(kDaikinFanAuto, irdaikin.getFan());

  irdaikin.setFan(3);
  EXPECT_EQ(3, irdaikin.getFan());

  irdaikin.setFan(kDaikinFanAuto);
  EXPECT_EQ(kDaikinFanAuto, irdaikin.getFan());

  irdaikin.setFan(kDaikinFanQuiet);
  EXPECT_EQ(kDaikinFanQuiet, irdaikin.getFan());
}

TEST(TestDaikinClass, CurrentTime) {
  IRDaikinESP irdaikin(0);
  irdaikin.begin();

  irdaikin.setCurrentTime(0);  // 00:00
  EXPECT_EQ(0, irdaikin.getCurrentTime());

  irdaikin.setCurrentTime(754);  // 12:34
  EXPECT_EQ(754, irdaikin.getCurrentTime());

  irdaikin.setCurrentTime(1439);  // 23:59
  EXPECT_EQ(1439, irdaikin.getCurrentTime());
}

TEST(TestDaikinClass, OnOffTimers) {
  IRDaikinESP irdaikin(0);
  irdaikin.begin();

  // Both timers turned off.
  irdaikin.disableOnTimer();
  irdaikin.disableOffTimer();
  EXPECT_FALSE(irdaikin.getOnTimerEnabled());
  EXPECT_EQ(0x600, irdaikin.getOnTime());
  EXPECT_FALSE(irdaikin.getOffTimerEnabled());
  EXPECT_EQ(0x600, irdaikin.getOffTime());

  // Turn on just the On Timer.
  irdaikin.enableOnTimer(123);
  EXPECT_TRUE(irdaikin.getOnTimerEnabled());
  EXPECT_EQ(123, irdaikin.getOnTime());
  EXPECT_FALSE(irdaikin.getOffTimerEnabled());
  EXPECT_EQ(0x600, irdaikin.getOffTime());

  // Now turn on the Off Timer.
  irdaikin.enableOffTimer(754);
  EXPECT_TRUE(irdaikin.getOffTimerEnabled());
  EXPECT_EQ(754, irdaikin.getOffTime());
  EXPECT_TRUE(irdaikin.getOnTimerEnabled());
  EXPECT_EQ(123, irdaikin.getOnTime());

  // Turn off the just the On Timer.
  irdaikin.disableOnTimer();
  EXPECT_FALSE(irdaikin.getOnTimerEnabled());
  EXPECT_EQ(0x600, irdaikin.getOnTime());
  EXPECT_TRUE(irdaikin.getOffTimerEnabled());
  EXPECT_EQ(754, irdaikin.getOffTime());

  // Now turn off the Off Timer.
  irdaikin.disableOffTimer();
  EXPECT_FALSE(irdaikin.getOffTimerEnabled());
  EXPECT_EQ(0x600, irdaikin.getOffTime());
  EXPECT_FALSE(irdaikin.getOnTimerEnabled());
  EXPECT_EQ(0x600, irdaikin.getOnTime());

  // Use some canary values around the timers to ensure no accidental
  // bit flips happen. i.e. Neighbouring bytes in the state.
  // (Found some during testing on systems with different endian-ness)
  // Tests here to make sure it never happens again.
  irdaikin.setSwingHorizontal(true);
  irdaikin.setPowerful(true);
  irdaikin.disableOffTimer();
  irdaikin.disableOnTimer();
  ASSERT_TRUE(irdaikin.getSwingHorizontal());
  ASSERT_TRUE(irdaikin.getPowerful());
  irdaikin.enableOnTimer(123);
  irdaikin.enableOffTimer(456);
  ASSERT_TRUE(irdaikin.getSwingHorizontal());
  ASSERT_TRUE(irdaikin.getPowerful());
  irdaikin.disableOffTimer();
  irdaikin.disableOnTimer();
  ASSERT_TRUE(irdaikin.getSwingHorizontal());
  ASSERT_TRUE(irdaikin.getPowerful());

  irdaikin.setSwingHorizontal(false);
  irdaikin.setPowerful(false);
  irdaikin.disableOffTimer();
  irdaikin.disableOnTimer();
  ASSERT_FALSE(irdaikin.getSwingHorizontal());
  ASSERT_FALSE(irdaikin.getPowerful());
  irdaikin.enableOnTimer(123);
  irdaikin.enableOffTimer(456);
  ASSERT_FALSE(irdaikin.getSwingHorizontal());
  ASSERT_FALSE(irdaikin.getPowerful());
  irdaikin.disableOffTimer();
  irdaikin.disableOnTimer();
  ASSERT_FALSE(irdaikin.getSwingHorizontal());
  ASSERT_FALSE(irdaikin.getPowerful());
}

// Test Eye mode.
TEST(TestDaikinClass, EyeSetting) {
  IRDaikinESP irdaikin(0);
  irdaikin.begin();

  // The Eye setting is stored in the same byte as Econo mode.
  // Econo mode tests are there to make sure it isn't harmed and vice-versa.
  irdaikin.setEcono(false);
  irdaikin.setEye(false);
  ASSERT_FALSE(irdaikin.getEye());
  EXPECT_FALSE(irdaikin.getEcono());

  irdaikin.setEye(true);
  ASSERT_TRUE(irdaikin.getEye());
  EXPECT_FALSE(irdaikin.getEcono());

  irdaikin.setEcono(false);
  ASSERT_TRUE(irdaikin.getEye());
  EXPECT_FALSE(irdaikin.getEcono());

  irdaikin.setEcono(true);
  ASSERT_TRUE(irdaikin.getEye());
  EXPECT_TRUE(irdaikin.getEcono());

  irdaikin.setEye(false);
  ASSERT_FALSE(irdaikin.getEye());
  EXPECT_TRUE(irdaikin.getEcono());
}

// Test Mold mode.
TEST(TestDaikinClass, MoldSetting) {
  IRDaikinESP irdaikin(0);
  irdaikin.begin();

  irdaikin.setMold(false);
  ASSERT_FALSE(irdaikin.getMold());

  irdaikin.setMold(true);
  ASSERT_TRUE(irdaikin.getMold());

  irdaikin.setMold(false);
  ASSERT_FALSE(irdaikin.getMold());
}

// Test Sensor mode.
TEST(TestDaikinClass, SensorSetting) {
  IRDaikinESP irdaikin(0);
  irdaikin.begin();

  irdaikin.setSensor(false);
  ASSERT_FALSE(irdaikin.getSensor());

  irdaikin.setSensor(true);
  ASSERT_TRUE(irdaikin.getSensor());

  irdaikin.setSensor(false);
  ASSERT_FALSE(irdaikin.getSensor());
}

TEST(TestDaikinClass, RenderTime) {
  EXPECT_EQ("0:00", IRDaikinESP::renderTime(0));
  EXPECT_EQ("0:10", IRDaikinESP::renderTime(10));
  EXPECT_EQ("1:00", IRDaikinESP::renderTime(1 * 60 + 0));
  EXPECT_EQ("23:59", IRDaikinESP::renderTime(23 * 60 + 59));
}

TEST(TestDaikinClass, SetAndGetRaw) {
  IRDaikinESP irdaikin(0);
  uint8_t initialState[kDaikinStateLength] = {
      0x11, 0xDA, 0x27, 0x00, 0x42, 0x00, 0x00, 0x54, 0x11,
      0xDA, 0x27, 0x00, 0x00, 0x49, 0x1E, 0x00, 0xB0, 0x00,
      0x00, 0x06, 0x60, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x4F};
  uint8_t expectedState[kDaikinStateLength] = {
      0x11, 0xDA, 0x27, 0x00, 0x42, 0x00, 0x00, 0x54, 0x11,
      0xDA, 0x27, 0x00, 0x00, 0x48, 0x2A, 0x00, 0xB0, 0x00,
      0x00, 0x06, 0x60, 0x00, 0x00, 0xC0, 0x00, 0x02, 0x5A};

  EXPECT_STATE_EQ(initialState, irdaikin.getRaw(), kDaikinBits);
  // toggle the power state.
  irdaikin.setPower(!irdaikin.getPower());
  irdaikin.setTemp(21);
  irdaikin.setMold(true);
  EXPECT_STATE_EQ(expectedState, irdaikin.getRaw(), kDaikinBits);
  irdaikin.setRaw(initialState);
  EXPECT_STATE_EQ(initialState, irdaikin.getRaw(), kDaikinBits);
}

TEST(TestDaikinClass, ChecksumValidation) {
  uint8_t daikin_code[kDaikinStateLength] = {
      0x11, 0xDA, 0x27, 0xF0, 0x00, 0x00, 0x00, 0x02, 0x11,
      0xDA, 0x27, 0x00, 0x00, 0x41, 0x1E, 0x00, 0xB0, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0xE1};

  EXPECT_TRUE(IRDaikinESP::validChecksum(daikin_code));
  // Change the array so the checksum is invalid.
  daikin_code[0] ^= 0xFF;
  EXPECT_FALSE(IRDaikinESP::validChecksum(daikin_code));
  // Restore the previous change, and change another byte.
  daikin_code[0] ^= 0xFF;
  daikin_code[4] ^= 0xFF;
  EXPECT_FALSE(IRDaikinESP::validChecksum(daikin_code));
  daikin_code[4] ^= 0xFF;
  // Change something in the 2nd block.
  daikin_code[10] ^= 0xFF;
  EXPECT_FALSE(IRDaikinESP::validChecksum(daikin_code));
  daikin_code[10] ^= 0xFF;
  EXPECT_TRUE(IRDaikinESP::validChecksum(daikin_code));
}

// Test human readable output.
TEST(TestDaikinClass, HumanReadable) {
  IRDaikinESP irdaikin(0);

  EXPECT_EQ(
      "Power: On, Mode: 4 (HEAT), Temp: 15C, Fan: 11 (QUIET), "
      "Powerful: Off, Quiet: Off, Sensor: Off, Eye: Off, Mold: Off, "
      "Swing (Horizontal): Off, Swing (Vertical): Off, "
      "Current Time: 0:00, On Time: Off, Off Time: Off",
      irdaikin.toString());
  irdaikin.setMode(kDaikinAuto);
  irdaikin.setTemp(25);
  irdaikin.setFan(kDaikinFanAuto);
  irdaikin.setQuiet(true);
  irdaikin.setSensor(true);
  irdaikin.setEye(true);
  irdaikin.setMold(true);
  irdaikin.setSwingVertical(true);
  irdaikin.setSwingHorizontal(true);
  irdaikin.setCurrentTime(9 * 60 + 15);
  irdaikin.enableOnTimer(8 * 60 + 0);
  irdaikin.enableOffTimer(17 * 60 + 30);
  irdaikin.off();
  EXPECT_EQ(
      "Power: Off, Mode: 0 (AUTO), Temp: 25C, Fan: 10 (AUTO), "
      "Powerful: Off, Quiet: On, Sensor: On, Eye: On, Mold: On, "
      "Swing (Horizontal): On, Swing (Vertical): On, "
      "Current Time: 9:15, On Time: 8:00, Off Time: 17:30",
      irdaikin.toString());
}

// Test general message construction after tweaking some settings.
TEST(TestDaikinClass, MessageConstuction) {
  IRDaikinESP irdaikin(0);
  IRsendTest irsend(4);
  irdaikin.begin();
  irsend.begin();

  irdaikin.setFan(kDaikinFanMin);
  irdaikin.setMode(kDaikinCool);
  irdaikin.setTemp(27);
  irdaikin.setSwingVertical(false);
  irdaikin.setSwingHorizontal(true);
  irdaikin.setQuiet(false);
  irdaikin.setPower(true);

  // Check everything for kicks.
  EXPECT_EQ(kDaikinFanMin, irdaikin.getFan());
  EXPECT_EQ(kDaikinCool, irdaikin.getMode());
  EXPECT_EQ(27, irdaikin.getTemp());
  EXPECT_FALSE(irdaikin.getSwingVertical());
  EXPECT_TRUE(irdaikin.getSwingHorizontal());
  EXPECT_FALSE(irdaikin.getQuiet());
  EXPECT_TRUE(irdaikin.getPower());

  irsend.reset();
  irsend.sendDaikin(irdaikin.getRaw());
  EXPECT_EQ(
      "m428s428m428s428m428s428m428s428m428s428"
      "m428s29428m3650s1623"
      "m428s1280m428s428m428s428m428s428m428s1280m428s428m428s428m428s428"
      "m428s428m428s1280m428s428m428s1280m428s1280m428s428m428s1280m428s1280"
      "m428s1280m428s1280m428s1280m428s428m428s428m428s1280m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s1280m428s428m428s1280m428s428m428s428m428s428m428s1280m428s1280"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s1280m428s1280m428s1280m428s428m428s1280m428s428m428s1280m428s1280"
      "m428s29428m3650s1623"
      "m428s1280m428s428m428s428m428s428m428s1280m428s428m428s428m428s428"
      "m428s428m428s1280m428s428m428s1280m428s1280m428s428m428s1280m428s1280"
      "m428s1280m428s1280m428s1280m428s428m428s428m428s1280m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s1280m428s428m428s428m428s428m428s428m428s1280m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s1280m428s428m428s1280m428s428m428s1280m428s428"
      "m428s29428m3650s1623"
      "m428s1280m428s428m428s428m428s428m428s1280m428s428m428s428m428s428"
      "m428s428m428s1280m428s428m428s1280m428s1280m428s428m428s1280m428s1280"
      "m428s1280m428s1280m428s1280m428s428m428s428m428s1280m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s1280m428s428m428s428m428s1280m428s1280m428s1280m428s428m428s428"
      "m428s428m428s1280m428s1280m428s428m428s1280m428s1280m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s1280m428s1280m428s428m428s428"
      "m428s1280m428s1280m428s1280m428s1280m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s1280m428s1280m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s1280m428s1280m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s1280m428s1280"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s1280m428s1280m428s428m428s428m428s1280m428s1280m428s1280"
      "m428s29428",
      irsend.outputStr());
}

// Tests for decodeDaikin().

// Test decoding a message captured from a real IR remote.
TEST(TestDecodeDaikin, RealExample) {
  IRDaikinESP irdaikin(0);
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  uint8_t expectedState[kDaikinStateLength] = {
      0x11, 0xDA, 0x27, 0x00, 0x42, 0x3A, 0x05, 0x93, 0x11,
      0xDA, 0x27, 0x00, 0x00, 0x3F, 0x3A, 0x00, 0xA0, 0x00,
      0x0A, 0x25, 0x17, 0x01, 0x00, 0xC0, 0x00, 0x00, 0x32};
  uint16_t rawData[kDaikinRawBits] = {
      416,  446,  416, 446,  416, 446,  418, 446,  416, 446,  416, 25434,
      3436, 1768, 390, 1336, 390, 446,  416, 446,  416, 446,  416, 1336,
      390,  446,  416, 446,  416, 446,  416, 446,  416, 1336, 390, 448,
      416,  1336, 390, 1336, 390, 448,  416, 1336, 390, 1336, 390, 1338,
      388,  1338, 390, 1336, 390, 446,  416, 446,  416, 1336, 390, 446,
      416,  446,  416, 446,  416, 446,  416, 446,  416, 446,  416, 448,
      416,  446,  416, 446,  416, 446,  416, 1336, 390, 446,  416, 1336,
      390,  448,  416, 446,  416, 446,  416, 1336, 390, 1336, 390, 446,
      416,  446,  416, 446,  416, 446,  416, 446,  416, 446,  416, 446,
      416,  446,  416, 446,  416, 448,  416, 446,  416, 446,  416, 446,
      416,  448,  414, 448,  416, 448,  416, 1336, 390, 1336, 390, 1336,
      390,  446,  414, 1336, 390, 448,  414, 1336, 390, 1336, 390, 34878,
      3436, 1768, 390, 1336, 390, 446,  416, 448,  416, 446,  416, 1336,
      390,  446,  416, 448,  416, 446,  416, 446,  416, 1336, 390, 446,
      416,  1336, 390, 1336, 390, 446,  416, 1336, 390, 1336, 390, 1336,
      390,  1336, 390, 1336, 392, 446,  414, 448,  416, 1336, 390, 446,
      416,  446,  416, 446,  416, 446,  414, 448,  416, 446,  416, 448,
      414,  448,  416, 446,  416, 446,  416, 446,  414, 1336, 390, 448,
      416,  446,  416, 446,  416, 448,  416, 1336, 390, 446,  416, 446,
      416,  1336, 390, 446,  416, 1336, 390, 1336, 390, 1336, 390, 446,
      416,  446,  414, 1338, 390, 446,  416, 1336, 390, 446,  416, 446,
      416,  446,  416, 446,  416, 446,  416, 1336, 390, 1336, 390, 446,
      416,  446,  416, 1336, 390, 446,  416, 446,  416, 1336, 390, 34876,
      3436, 1768, 388, 1336, 390, 446,  416, 446,  416, 448,  416, 1336,
      390,  446,  416, 446,  416, 446,  416, 448,  416, 1336, 390, 448,
      414,  1336, 390, 1336, 390, 446,  416, 1336, 388, 1338, 388, 1336,
      390,  1336, 390, 1336, 390, 446,  416, 446,  416, 1336, 390, 446,
      420,  442,  416, 446,  416, 446,  416, 446,  416, 446,  416, 446,
      416,  446,  416, 446,  416, 446,  416, 446,  416, 446,  416, 448,
      416,  446,  416, 448,  416, 446,  416, 448,  416, 446,  416, 1336,
      390,  1336, 390, 1336, 388, 1338, 390, 1336, 390, 1336, 392, 446,
      416,  446,  416, 448,  416, 1334, 390, 446,  416, 1338, 388, 1336,
      390,  1336, 390, 446,  416, 446,  416, 448,  414, 446,  416, 446,
      416,  446,  416, 448,  416, 446,  416, 446,  416, 446,  416, 446,
      416,  446,  416, 446,  416, 446,  416, 446,  416, 1336, 390, 446,
      416,  1336, 390, 446,  414, 448,  416, 446,  416, 446,  416, 446,
      416,  448,  416, 446,  416, 446,  416, 446,  416, 1336, 390, 446,
      416,  1336, 390, 446,  416, 446,  416, 446,  416, 448,  416, 1338,
      390,  444,  418, 1336, 390, 448,  416, 446,  416, 1336, 390, 446,
      416,  446,  416, 1336, 390, 1336, 388, 1336, 390, 446,  416, 1336,
      390,  448,  414, 448,  414, 448,  416, 1334, 390, 446,  416, 446,
      416,  446,  416, 448,  416, 446,  416, 446,  416, 448,  416, 446,
      416,  446,  416, 446,  416, 446,  416, 446,  416, 446,  416, 446,
      416,  446,  416, 446,  416, 446,  416, 446,  416, 446,  416, 446,
      416,  448,  416, 1336, 390, 1336, 390, 446,  416, 446,  416, 446,
      416,  446,  414, 446,  416, 448,  416, 446,  416, 448,  414, 446,
      418,  446,  416, 446,  416, 448,  416, 446,  416, 448,  416, 446,
      416,  448,  416, 446,  416, 1336, 390, 446,  416, 446,  416, 1338,
      390,  1336, 390, 446,  416, 446,  416};  // Captured by @sillyfrog

  irsend.reset();
  irsend.sendRaw(rawData, kDaikinRawBits, 38000);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(DAIKIN, irsend.capture.decode_type);
  ASSERT_EQ(kDaikinBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
}

// Decoding a message we entirely constructed based solely on a given state.
TEST(TestDecodeDaikin, SyntheticExample) {
  IRDaikinESP irdaikin(0);
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  uint8_t expectedState[kDaikinStateLength] = {
      0x11, 0xDA, 0x27, 0x00, 0x42, 0x3A, 0x05, 0x93, 0x11,
      0xDA, 0x27, 0x00, 0x00, 0x3F, 0x3A, 0x00, 0xA0, 0x00,
      0x0A, 0x25, 0x17, 0x01, 0x00, 0xC0, 0x00, 0x00, 0x32};

  irsend.reset();
  irsend.sendDaikin(expectedState);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(DAIKIN, irsend.capture.decode_type);
  ASSERT_EQ(kDaikinBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
}

// Test decoding a message captured from a real IR remote.
TEST(TestDecodeDaikin2, RealExample) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  uint8_t expectedState[kDaikin2StateLength] = {
      0x11, 0xDA, 0x27, 0x00, 0x01, 0x7A, 0xC3, 0x70, 0x28, 0x0C,
      0x80, 0x04, 0xB0, 0x16, 0x24, 0x00, 0x00, 0xBE, 0xD5, 0xF5,
      0x11, 0xDA, 0x27, 0x00, 0x00, 0x08, 0x26, 0x00, 0xA0, 0x00,
      0x00, 0x06, 0x60, 0x00, 0x00, 0xC1, 0x80, 0x60, 0xE7};
  // "Off" Data from https://github.com/markszabo/IRremoteESP8266/issues/582
  uint16_t rawData[633] = {  // Data supplied by @sheppy99
      10024, 25180, 3494, 1732, 436, 1300, 436, 436,  432, 438,  430, 438,
      426,   1306,  430,  442,  430, 438,  428, 440,  430, 440,  430, 1304,
      432,   442,   428,  1308, 424, 1312, 428, 442,  428, 1306, 424, 1314,
      426,   1308,  434,  1306, 426, 1308, 428, 444,  426, 442,  428, 1310,
      428,   442,   424,  444,  426, 442,  426, 444,  424, 444,  426, 444,
      424,   446,   422,  446,  422, 446,  422, 446,  418, 1318, 418, 450,
      420,   448,   420,  448,  422, 448,  420, 450,  420, 448,  420, 450,
      420,   452,   418,  1318, 420, 450,  420, 1318, 420, 1314, 418, 1318,
      424,   1314,  424,  448,  422, 1316, 424, 1312, 426, 446,  422, 448,
      420,   448,   422,  448,  422, 1314, 418, 1320, 416, 452,  420, 448,
      420,   448,   422,  448,  422, 1314, 416, 1320, 422, 1316, 422, 450,
      418,   450,   420,  448,  420, 448,  416, 1320, 418, 452,  418, 1316,
      422,   448,   420,  450,  420, 450,  420, 448,  422, 1314, 418, 1320,
      418,   450,   420,  448,  420, 448,  420, 450,  420, 450,  418, 450,
      418,   450,   420,  450,  418, 452,  416, 452,  420, 450,  418, 1318,
      420,   452,   418,  452,  418, 1322, 416, 452,  416, 452,  418, 452,
      418,   452,   416,  454,  418, 452,  416, 456,  414, 452,  418, 454,
      416,   1320,  410,  1324, 418, 452,  418, 1320, 416, 452,  418, 1320,
      418,   1318,  420,  448,  420, 1316, 420, 450,  420, 450,  418, 450,
      420,   450,   418,  452,  418, 1320, 418, 450,  418, 450,  416, 1322,
      412,   458,   420,  450,  416, 452,  418, 452,  416, 452,  418, 452,
      416,   454,   416,  452,  418, 452,  416, 454,  414, 454,  416, 454,
      416,   454,   414,  456,  414, 454,  414, 456,  412, 454,  416, 456,
      414,   456,   412,  1326, 412, 1320, 412, 1322, 414, 1322, 418, 1320,
      420,   452,   418,  1318, 420, 1316, 422, 450,  420, 1314, 424, 448,
      422,   1314,  422,  448,  422, 1314, 418, 1318, 424, 1316, 422, 448,
      422,   1312,  424,  446,  422, 1314, 420, 1318, 422, 1316, 426, 1310,
      426,   35166, 3500, 1724, 446, 1296, 444, 432,  436, 432,  438, 432,
      436,   1296,  440,  434,  434, 436,  432, 436,  434, 436,  434, 1298,
      438,   438,   432,  1304, 428, 1304, 432, 442,  430, 1302, 430, 1308,
      430,   1306,  434,  1302, 432, 1306, 430, 440,  430, 438,  430, 1308,
      434,   438,   430,  440,  428, 440,  430, 440,  428, 442,  426, 444,
      428,   442,   426,  444,  426, 442,  426, 444,  424, 446,  422, 446,
      424,   446,   424,  446,  422, 446,  424, 448,  420, 448,  422, 446,
      422,   448,   422,  450,  420, 450,  414, 1320, 420, 450,  418, 450,
      418,   448,   420,  450,  418, 452,  418, 1320, 418, 1316, 422, 450,
      418,   452,   418,  1320, 420, 448,  418, 450,  420, 450,  418, 452,
      416,   452,   418,  450,  418, 452,  416, 452,  418, 452,  416, 454,
      416,   452,   416,  454,  416, 454,  414, 456,  416, 454,  414, 1322,
      416,   454,   416,  1320, 418, 452,  416, 454,  414, 454,  416, 454,
      414,   454,   414,  454,  414, 456,  414, 456,  412, 456,  414, 456,
      414,   456,   412,  456,  414, 458,  406, 464,  410, 458,  412, 458,
      410,   460,   410,  1326, 412, 1324, 414, 456,  412, 458,  412, 456,
      414,   456,   412,  458,  410, 458,  414, 458,  410, 458,  408, 460,
      410,   470,   400,  1324, 408, 1328, 410, 458,  410, 460,  414, 456,
      410,   456,   414,  458,  412, 460,  410, 458,  412, 458,  412, 460,
      408,   460,   410,  460,  408, 472,  396, 462,  408, 470,  402, 470,
      396,   472,   400,  470,  398, 1326, 412, 460,  408, 472,  396, 472,
      400,   470,   400,  472,  396, 1328, 410, 1324, 414, 458,  410, 458,
      410,   458,   412,  458,  412, 460,  408, 460,  410, 460,  410, 1324,
      414,   458,   410,  460,  408, 460,  410, 458,  410, 460,  410, 1326,
      412,   1322,  416,  456,  412, 1322, 412, 1326, 416, 1322, 418, 452,
      416,   454,   412,  1324, 418, 1320, 420, 1316, 420};
  irsend.reset();
  irsend.sendRaw(rawData, 633, 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(DAIKIN2, irsend.capture.decode_type);
  ASSERT_EQ(kDaikin2Bits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
}

// Decoding a message we entirely constructed based solely on a given state.
TEST(TestDecodeDaikin2, SyntheticExample) {
  IRDaikinESP irdaikin(0);
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  uint8_t expectedState[kDaikin2StateLength] = {
      0x11, 0xDA, 0x27, 0x00, 0x01, 0x7A, 0xC3, 0x70, 0x28, 0x0C,
      0x80, 0x04, 0xB0, 0x16, 0x24, 0x00, 0x00, 0xBE, 0xD5, 0xF5,
      0x11, 0xDA, 0x27, 0x00, 0x00, 0x08, 0x26, 0x00, 0xA0, 0x00,
      0x00, 0x06, 0x60, 0x00, 0x00, 0xC1, 0x80, 0x60, 0xE7};

  irsend.reset();
  irsend.sendDaikin2(expectedState);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(DAIKIN2, irsend.capture.decode_type);
  ASSERT_EQ(kDaikin2Bits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
}

TEST(TestUtils, Misc) {
  ASSERT_EQ("DAIKIN", typeToString(DAIKIN));
  ASSERT_TRUE(hasACState(DAIKIN));
  ASSERT_EQ("DAIKIN2", typeToString(DAIKIN2));
  ASSERT_TRUE(hasACState(DAIKIN2));
}
