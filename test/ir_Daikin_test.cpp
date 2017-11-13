// Copyright 2017 David Conran
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "ir_Daikin.h"
#include "gtest/gtest.h"

// Tests for sendDaikin().

// Test sending typical data only.
TEST(TestSendDaikin, SendDataOnly) {
  IRsendTest irsend(4);
  irsend.begin();

  uint8_t daikin_code[DAIKIN_COMMAND_LENGTH] = {
      0x11, 0xDA, 0x27, 0xF0, 0x00, 0x00, 0x00, 0x20,
      0x11, 0xDA, 0x27, 0x00, 0x00, 0x41, 0x1E, 0x00,
      0xB0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0xE3};

  irsend.reset();
  irsend.sendDaikin(daikin_code);
  EXPECT_EQ(
      "m3650s1623"
      "m428s1280m428s428m428s428m428s428m428s1280m428s428m428s428m428s428"
      "m428s428m428s1280m428s428m428s1280m428s1280m428s428m428s1280m428s1280"
      "m428s1280m428s1280m428s1280m428s428m428s428m428s1280m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s1280m428s1280m428s1280m428s1280"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s1280m428s428m428s428"
      "m428s29428"
      "m3650s1623"
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
      "m428s29428", irsend.outputStr());
}

// Test sending with repeats.
TEST(TestSendDaikin, SendWithRepeats) {
  IRsendTest irsend(4);
  irsend.begin();

  irsend.reset();
  uint8_t daikin_code[DAIKIN_COMMAND_LENGTH] = {
      0x11, 0xDA, 0x27, 0xF0, 0x00, 0x00, 0x00, 0x20,
      0x11, 0xDA, 0x27, 0x00, 0x00, 0x41, 0x1E, 0x00,
      0xB0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0xE3};
  irsend.reset();

  irsend.sendDaikin(daikin_code, DAIKIN_COMMAND_LENGTH, 1);
  EXPECT_EQ(
    "m3650s1623"
    "m428s1280m428s428m428s428m428s428m428s1280m428s428m428s428m428s428"
    "m428s428m428s1280m428s428m428s1280m428s1280m428s428m428s1280m428s1280"
    "m428s1280m428s1280m428s1280m428s428m428s428m428s1280m428s428m428s428"
    "m428s428m428s428m428s428m428s428m428s1280m428s1280m428s1280m428s1280"
    "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
    "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
    "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
    "m428s428m428s428m428s428m428s428m428s428m428s1280m428s428m428s428"
    "m428s29428"
    "m3650s1623"
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
    "m3650s1623"
    "m428s1280m428s428m428s428m428s428m428s1280m428s428m428s428m428s428"
    "m428s428m428s1280m428s428m428s1280m428s1280m428s428m428s1280m428s1280"
    "m428s1280m428s1280m428s1280m428s428m428s428m428s1280m428s428m428s428"
    "m428s428m428s428m428s428m428s428m428s1280m428s1280m428s1280m428s1280"
    "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
    "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
    "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
    "m428s428m428s428m428s428m428s428m428s428m428s1280m428s428m428s428"
    "m428s29428"
    "m3650s1623"
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
    "m428s29428", irsend.outputStr());
}

// Test sending atypical sizes.
TEST(TestSendDaikin, SendUnexpectedSizes) {
  IRsendTest irsend(4);
  irsend.begin();

  uint8_t daikin_short_code[DAIKIN_COMMAND_LENGTH - 1] = {
      0x11, 0xDA, 0x27, 0xF0, 0x00, 0x00, 0x00, 0x20,
      0x11, 0xDA, 0x27, 0x00, 0x00, 0x41, 0x1E, 0x00,
      0xB0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00};

  irsend.reset();
  irsend.sendDaikin(daikin_short_code, DAIKIN_COMMAND_LENGTH - 1);
  ASSERT_EQ("", irsend.outputStr());

  uint8_t daikin_long_code[DAIKIN_COMMAND_LENGTH + 1] = {
      0x11, 0xDA, 0x27, 0xF0, 0x00, 0x00, 0x00, 0x20,
      0x11, 0xDA, 0x27, 0x00, 0x00, 0x41, 0x1E, 0x00,
      0xB0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0xE3, 0x11};
  irsend.reset();
  irsend.sendDaikin(daikin_long_code, DAIKIN_COMMAND_LENGTH + 1);
  ASSERT_EQ(
      "m3650s1623"
      "m428s1280m428s428m428s428m428s428m428s1280m428s428m428s428m428s428"
      "m428s428m428s1280m428s428m428s1280m428s1280m428s428m428s1280m428s1280"
      "m428s1280m428s1280m428s1280m428s428m428s428m428s1280m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s1280m428s1280m428s1280m428s1280"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s1280m428s428m428s428"
      "m428s29428"
      "m3650s1623"
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
      "m428s29428", irsend.outputStr());
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
  EXPECT_EQ(DAIKIN_MIN_TEMP, irdaikin.getTemp());

  irdaikin.setTemp(255);
  EXPECT_EQ(DAIKIN_MAX_TEMP, irdaikin.getTemp());

  irdaikin.setTemp(DAIKIN_MIN_TEMP);
  EXPECT_EQ(DAIKIN_MIN_TEMP, irdaikin.getTemp());

  irdaikin.setTemp(DAIKIN_MAX_TEMP);
  EXPECT_EQ(DAIKIN_MAX_TEMP, irdaikin.getTemp());

  irdaikin.setTemp(DAIKIN_MIN_TEMP - 1);
  EXPECT_EQ(DAIKIN_MIN_TEMP, irdaikin.getTemp());

  irdaikin.setTemp(DAIKIN_MAX_TEMP + 1);
  EXPECT_EQ(DAIKIN_MAX_TEMP, irdaikin.getTemp());

  irdaikin.setTemp(DAIKIN_MIN_TEMP + 1);
  EXPECT_EQ(DAIKIN_MIN_TEMP + 1, irdaikin.getTemp());

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

  irdaikin.setMode(DAIKIN_AUTO);
  EXPECT_EQ(DAIKIN_AUTO, irdaikin.getMode());

  irdaikin.setMode(DAIKIN_COOL);
  EXPECT_EQ(DAIKIN_COOL, irdaikin.getMode());

  irdaikin.setMode(DAIKIN_HEAT);
  EXPECT_EQ(DAIKIN_HEAT, irdaikin.getMode());

  irdaikin.setMode(DAIKIN_DRY);
  EXPECT_EQ(DAIKIN_DRY, irdaikin.getMode());

  irdaikin.setMode(DAIKIN_FAN);
  EXPECT_EQ(DAIKIN_FAN, irdaikin.getMode());

  irdaikin.setMode(DAIKIN_FAN + 1);
  EXPECT_EQ(DAIKIN_AUTO, irdaikin.getMode());

  irdaikin.setMode(DAIKIN_AUTO + 1);
  EXPECT_EQ(DAIKIN_AUTO, irdaikin.getMode());

  irdaikin.setMode(255);
  EXPECT_EQ(DAIKIN_AUTO, irdaikin.getMode());
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
  EXPECT_EQ(DAIKIN_FAN_AUTO, irdaikin.getFan());

  // Unexpected value should default to Auto.
  irdaikin.setFan(255);
  EXPECT_EQ(DAIKIN_FAN_AUTO, irdaikin.getFan());

  irdaikin.setFan(DAIKIN_FAN_MAX);
  EXPECT_EQ(DAIKIN_FAN_MAX, irdaikin.getFan());

  // Beyond Max should default to Auto.
  irdaikin.setFan(DAIKIN_FAN_MAX + 1);
  EXPECT_EQ(DAIKIN_FAN_AUTO, irdaikin.getFan());

  irdaikin.setFan(DAIKIN_FAN_MAX - 1);
  EXPECT_EQ(DAIKIN_FAN_MAX - 1, irdaikin.getFan());

  irdaikin.setFan(DAIKIN_FAN_MIN);
  EXPECT_EQ(DAIKIN_FAN_MIN, irdaikin.getFan());

  irdaikin.setFan(DAIKIN_FAN_MIN + 1);
  EXPECT_EQ(DAIKIN_FAN_MIN + 1, irdaikin.getFan());

  // Beyond Min should default to Auto.
  irdaikin.setFan(DAIKIN_FAN_MIN - 1);
  EXPECT_EQ(DAIKIN_FAN_AUTO, irdaikin.getFan());

  irdaikin.setFan(3);
  EXPECT_EQ(3, irdaikin.getFan());

  irdaikin.setFan(DAIKIN_FAN_AUTO);
  EXPECT_EQ(DAIKIN_FAN_AUTO, irdaikin.getFan());

  irdaikin.setFan(DAIKIN_FAN_QUITE);
  EXPECT_EQ(DAIKIN_FAN_QUITE, irdaikin.getFan());
}

TEST(TestDaikinClass, MessageConstuction) {
  IRDaikinESP irdaikin(0);
  IRsendTest irsend(4);
  irdaikin.begin();
  irsend.begin();

  irdaikin.setFan(DAIKIN_FAN_MIN);
  irdaikin.setMode(DAIKIN_COOL);
  irdaikin.setTemp(27);
  irdaikin.setSwingVertical(false);
  irdaikin.setSwingHorizontal(true);
  irdaikin.setQuiet(false);
  irdaikin.setPower(true);

  // Check everything for kicks.
  EXPECT_EQ(DAIKIN_FAN_MIN, irdaikin.getFan());
  EXPECT_EQ(DAIKIN_COOL, irdaikin.getMode());
  EXPECT_EQ(27, irdaikin.getTemp());
  EXPECT_FALSE(irdaikin.getSwingVertical());
  EXPECT_TRUE(irdaikin.getSwingHorizontal());
  EXPECT_FALSE(irdaikin.getQuiet());
  EXPECT_TRUE(irdaikin.getPower());

  irsend.reset();
  irsend.sendDaikin(irdaikin.getRaw());
  EXPECT_EQ(
      "m3650s1623"
      "m428s1280m428s428m428s428m428s428m428s1280m428s428m428s428m428s428"
      "m428s428m428s1280m428s428m428s1280m428s1280m428s428m428s1280m428s1280"
      "m428s1280m428s1280m428s1280m428s428m428s428m428s1280m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s1280m428s1280m428s1280m428s1280"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s1280m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s29428"
      "m3650s1623"
      "m428s1280m428s428m428s428m428s428m428s1280m428s428m428s428m428s428"
      "m428s428m428s1280m428s428m428s1280m428s1280m428s428m428s1280m428s1280"
      "m428s1280m428s1280m428s1280m428s428m428s428m428s1280m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s1280m428s428m428s428m428s428m428s1280m428s1280m428s428m428s428"
      "m428s428m428s1280m428s1280m428s428m428s1280m428s1280m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s1280m428s1280m428s428m428s428"
      "m428s1280m428s1280m428s1280m428s1280m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s1280m428s1280"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s428m428s428m428s428m428s428m428s428"
      "m428s428m428s428m428s428m428s1280m428s1280m428s1280m428s1280m428s428"
      "m428s29428", irsend.outputStr());
}

// Tests for decodeDaikin().

// Test decoding a message captured from a real IR remote.
TEST(TestDecodeDaikin, RealExample) {
  IRDaikinESP irdaikin(0);
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  uint8_t expectedState[DAIKIN_COMMAND_LENGTH] = {
      0x11, 0xDA, 0x27, 0x00, 0x42, 0x3A, 0x05, 0x93, 0x11, 0xDA, 0x27, 0x00,
      0x00, 0x3F, 0x3A, 0x00, 0xA0, 0x00, 0x0A, 0x25, 0x17, 0x01, 0x00, 0xC0,
      0x00, 0x00, 0x32};
  uint16_t rawData[DAIKIN_RAW_BITS] = {
      416, 446,  416, 446,  416, 446,  418, 446,  416, 446,  416, 25434,
      3436, 1768,  390, 1336,  390, 446,  416, 446,  416, 446,  416, 1336,
      390, 446,  416, 446,  416, 446,  416, 446,  416, 1336,  390, 448,
      416, 1336,  390, 1336,  390, 448,  416, 1336,  390, 1336,  390, 1338,
      388, 1338,  390, 1336,  390, 446,  416, 446,  416, 1336,  390, 446,
      416, 446,  416, 446,  416, 446,  416, 446,  416, 446,  416, 448,
      416, 446,  416, 446,  416, 446,  416, 1336,  390, 446,  416, 1336,
      390, 448,  416, 446,  416, 446,  416, 1336,  390, 1336,  390, 446,
      416, 446,  416, 446,  416, 446,  416, 446,  416, 446,  416, 446,
      416, 446,  416, 446,  416, 448,  416, 446,  416, 446,  416, 446,
      416, 448,  414, 448,  416, 448,  416, 1336,  390, 1336,  390, 1336,
      390, 446,  414, 1336,  390, 448,  414, 1336,  390, 1336,  390, 34878,
      3436, 1768,  390, 1336,  390, 446,  416, 448,  416, 446,  416, 1336,
      390, 446,  416, 448,  416, 446,  416, 446,  416, 1336,  390, 446,
      416, 1336,  390, 1336,  390, 446,  416, 1336,  390, 1336,  390, 1336,
      390, 1336,  390, 1336,  392, 446,  414, 448,  416, 1336,  390, 446,
      416, 446,  416, 446,  416, 446,  414, 448,  416, 446,  416, 448,
      414, 448,  416, 446,  416, 446,  416, 446,  414, 1336,  390, 448,
      416, 446,  416, 446,  416, 448,  416, 1336,  390, 446,  416, 446,
      416, 1336,  390, 446,  416, 1336,  390, 1336,  390, 1336,  390, 446,
      416, 446,  414, 1338,  390, 446,  416, 1336,  390, 446,  416, 446,
      416, 446,  416, 446,  416, 446,  416, 1336,  390, 1336,  390, 446,
      416, 446,  416, 1336,  390, 446,  416, 446,  416, 1336,  390, 34876,
      3436, 1768,  388, 1336,  390, 446,  416, 446,  416, 448,  416, 1336,
      390, 446,  416, 446,  416, 446,  416, 448,  416, 1336,  390, 448,
      414, 1336,  390, 1336,  390, 446,  416, 1336,  388, 1338,  388, 1336,
      390, 1336,  390, 1336,  390, 446,  416, 446,  416, 1336,  390, 446,
      420, 442,  416, 446,  416, 446,  416, 446,  416, 446,  416, 446,
      416, 446,  416, 446,  416, 446,  416, 446,  416, 446,  416, 448,
      416, 446,  416, 448,  416, 446,  416, 448,  416, 446,  416, 1336,
      390, 1336,  390, 1336,  388, 1338,  390, 1336,  390, 1336,  392, 446,
      416, 446,  416, 448,  416, 1334,  390, 446,  416, 1338,  388, 1336,
      390, 1336,  390, 446,  416, 446,  416, 448,  414, 446,  416, 446,
      416, 446,  416, 448,  416, 446,  416, 446,  416, 446,  416, 446,
      416, 446,  416, 446,  416, 446,  416, 446,  416, 1336,  390, 446,
      416, 1336,  390, 446,  414, 448,  416, 446,  416, 446,  416, 446,
      416, 448,  416, 446,  416, 446,  416, 446,  416, 1336,  390, 446,
      416, 1336,  390, 446,  416, 446,  416, 446,  416, 448,  416, 1338,
      390, 444,  418, 1336,  390, 448,  416, 446,  416, 1336,  390, 446,
      416, 446,  416, 1336,  390, 1336,  388, 1336,  390, 446,  416, 1336,
      390, 448,  414, 448,  414, 448,  416, 1334,  390, 446,  416, 446,
      416, 446,  416, 448,  416, 446,  416, 446,  416, 448,  416, 446,
      416, 446,  416, 446,  416, 446,  416, 446,  416, 446,  416, 446,
      416, 446,  416, 446,  416, 446,  416, 446,  416, 446,  416, 446,
      416, 448,  416, 1336,  390, 1336,  390, 446,  416, 446,  416, 446,
      416, 446,  414, 446,  416, 448,  416, 446,  416, 448,  414, 446,
      418, 446,  416, 446,  416, 448,  416, 446,  416, 448,  416, 446,
      416, 448,  416, 446,  416, 1336,  390, 446,  416, 446,  416, 1338,
      390, 1336,  390, 446,  416, 446,  416};  // Captured by @sillyfrog

  irsend.reset();
  irsend.sendRaw(rawData, DAIKIN_RAW_BITS, 38000);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(DAIKIN, irsend.capture.decode_type);
  ASSERT_EQ(DAIKIN_BITS, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
}

// Test decoding a message we entirely constructed based soley on a given state.
TEST(TestDecodeDaikin, SyntheticExample) {
  IRDaikinESP irdaikin(0);
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  uint8_t expectedState[DAIKIN_COMMAND_LENGTH] = {
      0x11, 0xDA, 0x27, 0x00, 0x42, 0x3A, 0x05, 0x93, 0x11, 0xDA, 0x27, 0x00,
      0x00, 0x3F, 0x3A, 0x00, 0xA0, 0x00, 0x0A, 0x25, 0x17, 0x01, 0x00, 0xC0,
      0x00, 0x00, 0x32};

  irsend.reset();
  irsend.sendDaikin(expectedState);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(DAIKIN, irsend.capture.decode_type);
  ASSERT_EQ(DAIKIN_BITS, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
}
