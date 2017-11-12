// Copyright 2017 David Conran

#include "IRsend.h"
#include "IRsend_test.h"
#include "ir_Toshiba.h"
#include "gtest/gtest.h"

// Tests for Toshiba A/C methods.

// Test sending typical data only.
TEST(TestSendToshibaAC, SendDataOnly) {
  IRsendTest irsend(4);
  irsend.begin();

  uint8_t toshiba_code[TOSHIBA_AC_STATE_LENGTH] = {
      0xF2, 0x0D, 0x03, 0xFC, 0x01, 0x00, 0x00, 0x00, 0x00};
  irsend.reset();
  irsend.sendToshibaAC(toshiba_code);
  EXPECT_EQ(
      "m4400s4300"
      "m543s1623m543s1623m543s1623m543s1623m543s472m543s472m543s1623m543s472"
      "m543s472m543s472m543s472m543s472m543s1623m543s1623m543s472m543s1623"
      "m543s472m543s472m543s472m543s472m543s472m543s472m543s1623m543s1623"
      "m543s1623m543s1623m543s1623m543s1623m543s1623m543s1623m543s472m543s472"
      "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s1623"
      "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s472"
      "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s472"
      "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s472"
      "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s472"
      "m440s7048", irsend.outputStr());
}

// Test sending with repeats.
TEST(TestSendToshibaAC, SendWithRepeats) {
  IRsendTest irsend(4);
  irsend.begin();

  irsend.reset();
  uint8_t toshiba_code[TOSHIBA_AC_STATE_LENGTH] = {
      0xF2, 0x0D, 0x03, 0xFC, 0x01, 0x00, 0x00, 0x00, 0x00};

  irsend.sendToshibaAC(toshiba_code, TOSHIBA_AC_STATE_LENGTH, 0);
  EXPECT_EQ(
    "m4400s4300"
    "m543s1623m543s1623m543s1623m543s1623m543s472m543s472m543s1623m543s472"
    "m543s472m543s472m543s472m543s472m543s1623m543s1623m543s472m543s1623"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s1623m543s1623"
    "m543s1623m543s1623m543s1623m543s1623m543s1623m543s1623m543s472m543s472"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s1623"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s472"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s472"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s472"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s472"
    "m440s7048", irsend.outputStr());

  irsend.reset();
  irsend.sendToshibaAC(toshiba_code, TOSHIBA_AC_STATE_LENGTH, 2);
  EXPECT_EQ(
    "m4400s4300"
    "m543s1623m543s1623m543s1623m543s1623m543s472m543s472m543s1623m543s472"
    "m543s472m543s472m543s472m543s472m543s1623m543s1623m543s472m543s1623"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s1623m543s1623"
    "m543s1623m543s1623m543s1623m543s1623m543s1623m543s1623m543s472m543s472"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s1623"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s472"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s472"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s472"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s472"
    "m440s7048"
    "m4400s4300"
    "m543s1623m543s1623m543s1623m543s1623m543s472m543s472m543s1623m543s472"
    "m543s472m543s472m543s472m543s472m543s1623m543s1623m543s472m543s1623"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s1623m543s1623"
    "m543s1623m543s1623m543s1623m543s1623m543s1623m543s1623m543s472m543s472"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s1623"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s472"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s472"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s472"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s472"
    "m440s7048"
    "m4400s4300"
    "m543s1623m543s1623m543s1623m543s1623m543s472m543s472m543s1623m543s472"
    "m543s472m543s472m543s472m543s472m543s1623m543s1623m543s472m543s1623"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s1623m543s1623"
    "m543s1623m543s1623m543s1623m543s1623m543s1623m543s1623m543s472m543s472"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s1623"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s472"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s472"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s472"
    "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s472"
    "m440s7048", irsend.outputStr());
}

// Test sending atypical sizes.
TEST(TestSendToshibaAC, SendUnexpectedSizes) {
  IRsendTest irsend(4);
  irsend.begin();

  uint8_t toshiba_short_code[8] = {0x01, 0x02, 0x03, 0x04,
                                   0x05, 0x06, 0x07, 0x08};
  uint8_t toshiba_long_code[10] = {0x01, 0x02, 0x03, 0x04, 0x05,
                                   0x06, 0x07, 0x08, 0x09, 0x0A};
  irsend.reset();
  irsend.sendToshibaAC(toshiba_short_code, TOSHIBA_AC_STATE_LENGTH - 1);
  ASSERT_EQ("", irsend.outputStr());

  irsend.reset();
  irsend.sendToshibaAC(toshiba_long_code, TOSHIBA_AC_STATE_LENGTH + 1);
  ASSERT_EQ(
      "m4400s4300"
      "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s1623"
      "m543s472m543s472m543s472m543s472m543s472m543s472m543s1623m543s472"
      "m543s472m543s472m543s472m543s472m543s472m543s472m543s1623m543s1623"
      "m543s472m543s472m543s472m543s472m543s472m543s1623m543s472m543s472"
      "m543s472m543s472m543s472m543s472m543s472m543s1623m543s472m543s1623"
      "m543s472m543s472m543s472m543s472m543s472m543s1623m543s1623m543s472"
      "m543s472m543s472m543s472m543s472m543s472m543s1623m543s1623m543s1623"
      "m543s472m543s472m543s472m543s472m543s1623m543s472m543s472m543s472"
      "m543s472m543s472m543s472m543s472m543s1623m543s472m543s472m543s1623"
      "m543s472m543s472m543s472m543s472m543s1623m543s472m543s1623m543s472"
      "m440s7048", irsend.outputStr());
}

// Tests for IRToshibaAC class.

TEST(TestToshibaACClass, Power) {
  IRToshibaAC toshiba(0);
  toshiba.begin();

  toshiba.on();
  EXPECT_TRUE(toshiba.getPower());

  toshiba.off();
  EXPECT_FALSE(toshiba.getPower());

  toshiba.setPower(true);
  EXPECT_TRUE(toshiba.getPower());

  toshiba.setPower(false);
  EXPECT_FALSE(toshiba.getPower());
}

TEST(TestToshibaACClass, Temperature) {
  IRToshibaAC toshiba(0);
  toshiba.begin();

  toshiba.setTemp(0);
  EXPECT_EQ(TOSHIBA_AC_MIN_TEMP, toshiba.getTemp());

  toshiba.setTemp(255);
  EXPECT_EQ(TOSHIBA_AC_MAX_TEMP, toshiba.getTemp());

  toshiba.setTemp(TOSHIBA_AC_MIN_TEMP);
  EXPECT_EQ(TOSHIBA_AC_MIN_TEMP, toshiba.getTemp());

  toshiba.setTemp(TOSHIBA_AC_MAX_TEMP);
  EXPECT_EQ(TOSHIBA_AC_MAX_TEMP, toshiba.getTemp());

  toshiba.setTemp(TOSHIBA_AC_MIN_TEMP - 1);
  EXPECT_EQ(TOSHIBA_AC_MIN_TEMP, toshiba.getTemp());

  toshiba.setTemp(TOSHIBA_AC_MAX_TEMP + 1);
  EXPECT_EQ(TOSHIBA_AC_MAX_TEMP, toshiba.getTemp());

  toshiba.setTemp(17);
  EXPECT_EQ(17, toshiba.getTemp());

  toshiba.setTemp(21);
  EXPECT_EQ(21, toshiba.getTemp());

  toshiba.setTemp(25);
  EXPECT_EQ(25, toshiba.getTemp());

  toshiba.setTemp(30);
  EXPECT_EQ(30, toshiba.getTemp());
}

TEST(TestToshibaACClass, OperatingMode) {
  IRToshibaAC toshiba(0);
  toshiba.begin();

  toshiba.setMode(TOSHIBA_AC_AUTO);
  EXPECT_EQ(TOSHIBA_AC_AUTO, toshiba.getMode());

  toshiba.setMode(TOSHIBA_AC_COOL);
  EXPECT_EQ(TOSHIBA_AC_COOL, toshiba.getMode());

  toshiba.setMode(TOSHIBA_AC_HEAT);
  EXPECT_EQ(TOSHIBA_AC_HEAT, toshiba.getMode());

  toshiba.setMode(TOSHIBA_AC_DRY);
  EXPECT_EQ(TOSHIBA_AC_DRY, toshiba.getMode());

  toshiba.setMode(TOSHIBA_AC_HEAT + 1);
  EXPECT_EQ(TOSHIBA_AC_AUTO, toshiba.getMode());

  toshiba.setMode(255);
  EXPECT_EQ(TOSHIBA_AC_AUTO, toshiba.getMode());
}

TEST(TestToshibaACClass, FanSpeed) {
  IRToshibaAC toshiba(0);
  toshiba.begin();

  toshiba.setFan(TOSHIBA_AC_FAN_AUTO);
  EXPECT_EQ(TOSHIBA_AC_FAN_AUTO, toshiba.getFan());

  toshiba.setFan(255);
  EXPECT_EQ(TOSHIBA_AC_FAN_MAX, toshiba.getFan());

  toshiba.setFan(TOSHIBA_AC_FAN_MAX);
  EXPECT_EQ(TOSHIBA_AC_FAN_MAX, toshiba.getFan());

  toshiba.setFan(TOSHIBA_AC_FAN_MAX - 1);
  EXPECT_EQ(TOSHIBA_AC_FAN_MAX - 1, toshiba.getFan());

  toshiba.setFan(1);
  EXPECT_EQ(1, toshiba.getFan());

  toshiba.setFan(2);
  EXPECT_EQ(2, toshiba.getFan());

  toshiba.setFan(3);
  EXPECT_EQ(3, toshiba.getFan());

  toshiba.setFan(4);
  EXPECT_EQ(4, toshiba.getFan());

  toshiba.setFan(TOSHIBA_AC_FAN_MAX + 1);
  EXPECT_EQ(TOSHIBA_AC_FAN_MAX, toshiba.getFan());
}

TEST(TestToshibaACClass, MessageConstuction) {
  IRToshibaAC toshiba(0);
  IRsendTest irsend(4);
  toshiba.begin();
  irsend.begin();

  toshiba.setFan(1);
  toshiba.setMode(TOSHIBA_AC_COOL);
  toshiba.setTemp(27);
  toshiba.on();

  // Check everything for kicks.
  EXPECT_EQ(1, toshiba.getFan());
  EXPECT_EQ(TOSHIBA_AC_COOL, toshiba.getMode());
  EXPECT_EQ(27, toshiba.getTemp());
  EXPECT_TRUE(toshiba.getPower());

  irsend.reset();
  irsend.sendToshibaAC(toshiba.getRaw());
  EXPECT_EQ(
      "m4400s4300"
      "m543s1623m543s1623m543s1623m543s1623m543s472m543s472m543s1623m543s472"
      "m543s472m543s472m543s472m543s472m543s1623m543s1623m543s472m543s1623"
      "m543s472m543s472m543s472m543s472m543s472m543s472m543s1623m543s1623"
      "m543s1623m543s1623m543s1623m543s1623m543s1623m543s1623m543s472m543s472"
      "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s1623"
      "m543s1623m543s472m543s1623m543s472m543s472m543s472m543s472m543s472"
      "m543s472m543s1623m543s472m543s472m543s472m543s472m543s472m543s1623"
      "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s472"
      "m543s1623m543s1623m543s1623m543s472m543s472m543s472m543s472m543s472"
      "m440s7048", irsend.outputStr());

  // Turn off the power and re-check.
  toshiba.setPower(false);
  // Check everything for kicks.
  EXPECT_EQ(1, toshiba.getFan());
  EXPECT_EQ(TOSHIBA_AC_COOL, toshiba.getMode());
  EXPECT_EQ(27, toshiba.getTemp());
  EXPECT_FALSE(toshiba.getPower());

  irsend.reset();
  irsend.sendToshibaAC(toshiba.getRaw());
  EXPECT_EQ(
      "m4400s4300"
      "m543s1623m543s1623m543s1623m543s1623m543s472m543s472m543s1623m543s472"
      "m543s472m543s472m543s472m543s472m543s1623m543s1623m543s472m543s1623"
      "m543s472m543s472m543s472m543s472m543s472m543s472m543s1623m543s1623"
      "m543s1623m543s1623m543s1623m543s1623m543s1623m543s1623m543s472m543s472"
      "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s1623"
      "m543s1623m543s472m543s1623m543s472m543s472m543s472m543s472m543s472"
      "m543s472m543s1623m543s472m543s472m543s472m543s1623m543s1623m543s1623"
      "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s472"
      "m543s1623m543s1623m543s1623m543s472m543s472m543s1623m543s1623m543s472"
      "m440s7048", irsend.outputStr());

  // Turn the power back on, and check nothing changed.
  toshiba.on();

  EXPECT_EQ(1, toshiba.getFan());
  EXPECT_EQ(TOSHIBA_AC_COOL, toshiba.getMode());
  EXPECT_EQ(27, toshiba.getTemp());
  EXPECT_TRUE(toshiba.getPower());

  irsend.reset();
  irsend.sendToshibaAC(toshiba.getRaw());
  EXPECT_EQ(
      "m4400s4300"
      "m543s1623m543s1623m543s1623m543s1623m543s472m543s472m543s1623m543s472"
      "m543s472m543s472m543s472m543s472m543s1623m543s1623m543s472m543s1623"
      "m543s472m543s472m543s472m543s472m543s472m543s472m543s1623m543s1623"
      "m543s1623m543s1623m543s1623m543s1623m543s1623m543s1623m543s472m543s472"
      "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s1623"
      "m543s1623m543s472m543s1623m543s472m543s472m543s472m543s472m543s472"
      "m543s472m543s1623m543s472m543s472m543s472m543s472m543s472m543s1623"
      "m543s472m543s472m543s472m543s472m543s472m543s472m543s472m543s472"
      "m543s1623m543s1623m543s1623m543s472m543s472m543s472m543s472m543s472"
      "m440s7048", irsend.outputStr());
}
