// Copyright 2017 David Conran
#include "IRrecv.h"
#include "IRrecv_test.h"
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
      "m543s7048", irsend.outputStr());
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
    "m543s7048", irsend.outputStr());

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
    "m543s7048"
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
    "m543s7048"
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
    "m543s7048", irsend.outputStr());
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
      "m543s7048", irsend.outputStr());
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

  // Setting the power off changes the underlying mode in the state to heat.
  toshiba.setPower(true);
  toshiba.setMode(TOSHIBA_AC_COOL);
  EXPECT_EQ(TOSHIBA_AC_COOL, toshiba.getMode());
  EXPECT_EQ(TOSHIBA_AC_COOL, toshiba.getMode(true));
  toshiba.setPower(false);
  EXPECT_EQ(TOSHIBA_AC_COOL, toshiba.getMode());
  EXPECT_EQ(TOSHIBA_AC_HEAT, toshiba.getMode(true));
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

TEST(TestToshibaACClass, RawState) {
  IRToshibaAC toshiba(0);
  toshiba.begin();

  uint8_t initial_state[TOSHIBA_AC_STATE_LENGTH] = {
      0xF2, 0x0D, 0x03, 0xFC, 0x01, 0x00, 0x00, 0x00, 0x01};
  uint8_t modified_state[TOSHIBA_AC_STATE_LENGTH] = {
      0xF2, 0x0D, 0x03, 0xFC, 0x01, 0x00, 0xC1, 0x00, 0xC0};

  // Verify the starting state.
  EXPECT_STATE_EQ(initial_state, toshiba.getRaw(), TOSHIBA_AC_BITS);
  EXPECT_TRUE(toshiba.getPower());
  EXPECT_EQ(TOSHIBA_AC_AUTO, toshiba.getMode());
  EXPECT_EQ(TOSHIBA_AC_FAN_AUTO, toshiba.getFan());

  // Change some settings.
  toshiba.setMode(TOSHIBA_AC_COOL);
  toshiba.setFan(TOSHIBA_AC_FAN_MAX);
  toshiba.setTemp(TOSHIBA_AC_MIN_TEMP);
  // Verify those were set.
  EXPECT_EQ(TOSHIBA_AC_COOL, toshiba.getMode());
  EXPECT_EQ(TOSHIBA_AC_FAN_MAX, toshiba.getFan());
  EXPECT_EQ(TOSHIBA_AC_MIN_TEMP, toshiba.getTemp());
  // Retrieve the modified state.
  EXPECT_STATE_EQ(modified_state, toshiba.getRaw(), TOSHIBA_AC_BITS);

  // Set it back to the initial state.
  toshiba.setRaw(initial_state);

  // Check the new state was set correctly.
  EXPECT_TRUE(toshiba.getPower());
  EXPECT_EQ(TOSHIBA_AC_AUTO, toshiba.getMode());
  EXPECT_EQ(TOSHIBA_AC_FAN_AUTO, toshiba.getFan());
  EXPECT_STATE_EQ(initial_state, toshiba.getRaw(), TOSHIBA_AC_BITS);
}

TEST(TestToshibaACClass, Checksums) {
  IRToshibaAC toshiba(0);
  toshiba.begin();

  uint8_t initial_state[TOSHIBA_AC_STATE_LENGTH] = {
      0xF2, 0x0D, 0x03, 0xFC, 0x01, 0x00, 0x00, 0x00, 0x01};
  uint8_t modified_state[TOSHIBA_AC_STATE_LENGTH] = {
      0xF2, 0x0D, 0x03, 0xFC, 0x01, 0x00, 0xC1, 0x00, 0xC0};
  uint8_t invalid_state[TOSHIBA_AC_STATE_LENGTH] = {
      0xF2, 0x0D, 0x03, 0xFC, 0x01, 0x00, 0x00, 0x00, 0x00};

  EXPECT_EQ(0x01, toshiba.calcChecksum(initial_state));
  EXPECT_EQ(0xC0, toshiba.calcChecksum(modified_state));
  // Check we can call it without instantiating the object.
  EXPECT_EQ(0x01, IRToshibaAC::calcChecksum(initial_state));
  // Use different lengths.
  EXPECT_EQ(0x01, IRToshibaAC::calcChecksum(initial_state,
                                            TOSHIBA_AC_STATE_LENGTH - 1));
  EXPECT_EQ(0xFF, IRToshibaAC::calcChecksum(initial_state, 3));
  // Minimum length that actually means anything.
  EXPECT_EQ(0xF2, IRToshibaAC::calcChecksum(initial_state, 2));
  // Technically, there is no such thing as a checksum for a length of < 2
  // But test it anyway
  EXPECT_EQ(0x00, IRToshibaAC::calcChecksum(initial_state, 1));
  EXPECT_EQ(0x00, IRToshibaAC::calcChecksum(initial_state, 0));

  // Validity tests.
  EXPECT_TRUE(IRToshibaAC::validChecksum(initial_state));
  EXPECT_TRUE(IRToshibaAC::validChecksum(modified_state));
  EXPECT_FALSE(IRToshibaAC::validChecksum(invalid_state));
  EXPECT_FALSE(IRToshibaAC::validChecksum(initial_state, 0));
  EXPECT_FALSE(IRToshibaAC::validChecksum(initial_state, 1));
  EXPECT_FALSE(IRToshibaAC::validChecksum(initial_state, 2));
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
      "m543s7048", irsend.outputStr());

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
      "m543s7048", irsend.outputStr());

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
      "m543s7048", irsend.outputStr());
}

// Test decoding a message we entirely constructed based soley on a given state.
TEST(TestDecodeToshibaAC, SyntheticExample) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  uint8_t expectedState[TOSHIBA_AC_STATE_LENGTH] = {
      0xF2, 0x0D, 0x03, 0xFC, 0x01, 0x00, 0x00, 0x00, 0x01};

  irsend.reset();
  irsend.sendToshibaAC(expectedState);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(TOSHIBA_AC, irsend.capture.decode_type);
  ASSERT_EQ(TOSHIBA_AC_BITS, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
}
