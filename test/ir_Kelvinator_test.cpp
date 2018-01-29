// Copyright 2017 David Conran

#include "IRremoteESP8266.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "ir_Kelvinator.h"
#include "gtest/gtest.h"

// Tests for sendKelvinator().

// Test sending typical data only.
TEST(TestSendKelvinator, SendDataOnly) {
  IRsendTest irsend(4);
  irsend.begin();

  uint8_t kelv_code[KELVINATOR_STATE_LENGTH] = {
      0x19, 0x0B, 0x80, 0x50, 0x00, 0x00, 0x00, 0xE0,
      0x19, 0x0B, 0x80, 0x70, 0x00, 0x00, 0x10, 0xf0};
  irsend.reset();
  irsend.sendKelvinator(kelv_code);
  EXPECT_EQ(
      "m9010s4505"
      "m680s1530m680s510m680s510m680s1530m680s1530m680s510m680s510m680s510"
      "m680s1530m680s1530m680s510m680s1530m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s1530"
      "m680s510m680s510m680s510m680s510m680s1530m680s510m680s1530m680s510"
      "m680s510m680s1530m680s510"
      "m680s19975"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s1530m680s1530m680s1530"
      "m680s39950"
      "m9010s4505"
      "m680s1530m680s510m680s510m680s1530m680s1530m680s510m680s510m680s510"
      "m680s1530m680s1530m680s510m680s1530m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s1530"
      "m680s510m680s510m680s510m680s510m680s1530m680s1530m680s1530m680s510"
      "m680s510m680s1530m680s510"
      "m680s19975"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s1530m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s1530m680s1530m680s1530m680s1530"
      "m680s39950", irsend.outputStr());
}

// Test sending with repeats.
TEST(TestSendKelvinator, SendWithRepeats) {
  IRsendTest irsend(4);
  irsend.begin();

  irsend.reset();
  uint8_t kelv_code[KELVINATOR_STATE_LENGTH] = {
      0x19, 0x0B, 0x80, 0x50, 0x00, 0x00, 0x00, 0xE0,
      0x19, 0x0B, 0x80, 0x70, 0x00, 0x00, 0x10, 0xf0};
  irsend.reset();

  irsend.sendKelvinator(kelv_code, KELVINATOR_STATE_LENGTH, 1);
  EXPECT_EQ(
      "m9010s4505"
      "m680s1530m680s510m680s510m680s1530m680s1530m680s510m680s510m680s510"
      "m680s1530m680s1530m680s510m680s1530m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s1530"
      "m680s510m680s510m680s510m680s510m680s1530m680s510m680s1530m680s510"
      "m680s510m680s1530m680s510"
      "m680s19975"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s1530m680s1530m680s1530"
      "m680s39950"
      "m9010s4505"
      "m680s1530m680s510m680s510m680s1530m680s1530m680s510m680s510m680s510"
      "m680s1530m680s1530m680s510m680s1530m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s1530"
      "m680s510m680s510m680s510m680s510m680s1530m680s1530m680s1530m680s510"
      "m680s510m680s1530m680s510"
      "m680s19975"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s1530m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s1530m680s1530m680s1530m680s1530"
      "m680s39950"
      "m9010s4505"
      "m680s1530m680s510m680s510m680s1530m680s1530m680s510m680s510m680s510"
      "m680s1530m680s1530m680s510m680s1530m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s1530"
      "m680s510m680s510m680s510m680s510m680s1530m680s510m680s1530m680s510"
      "m680s510m680s1530m680s510"
      "m680s19975"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s1530m680s1530m680s1530"
      "m680s39950"
      "m9010s4505"
      "m680s1530m680s510m680s510m680s1530m680s1530m680s510m680s510m680s510"
      "m680s1530m680s1530m680s510m680s1530m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s1530"
      "m680s510m680s510m680s510m680s510m680s1530m680s1530m680s1530m680s510"
      "m680s510m680s1530m680s510"
      "m680s19975"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s1530m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s1530m680s1530m680s1530m680s1530"
      "m680s39950", irsend.outputStr());
}

// Test sending atypical sizes.
TEST(TestSendKelvinator, SendUnexpectedSizes) {
  IRsendTest irsend(4);
  irsend.begin();

  uint8_t kelv_short_code[15] = {0x19, 0x0B, 0x80, 0x50, 0x00, 0x00, 0x00, 0xE0,
                                 0x19, 0x0B, 0x80, 0x70, 0x00, 0x00, 0x10};
  uint8_t kelv_long_code[17] = {0x19, 0x0B, 0x80, 0x50, 0x00, 0x00, 0x00, 0xE0,
                                0x19, 0x0B, 0x80, 0x70, 0x00, 0x00, 0x10, 0xf0,
                                0x00};
  irsend.reset();
  irsend.sendKelvinator(kelv_short_code, 15);
  ASSERT_EQ("", irsend.outputStr());

  irsend.reset();
  // Shouldn't be different from the SendDataOnly. We just don't send the
  // extra data.
  irsend.sendKelvinator(kelv_long_code, 17);
  ASSERT_EQ(
      "m9010s4505"
      "m680s1530m680s510m680s510m680s1530m680s1530m680s510m680s510m680s510"
      "m680s1530m680s1530m680s510m680s1530m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s1530"
      "m680s510m680s510m680s510m680s510m680s1530m680s510m680s1530m680s510"
      "m680s510m680s1530m680s510"
      "m680s19975"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s1530m680s1530m680s1530"
      "m680s39950"
      "m9010s4505"
      "m680s1530m680s510m680s510m680s1530m680s1530m680s510m680s510m680s510"
      "m680s1530m680s1530m680s510m680s1530m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s1530"
      "m680s510m680s510m680s510m680s510m680s1530m680s1530m680s1530m680s510"
      "m680s510m680s1530m680s510"
      "m680s19975"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s1530m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s1530m680s1530m680s1530m680s1530"
      "m680s39950", irsend.outputStr());
}


// Tests for IRKelvinatorAC class.

TEST(TestKelvinatorClass, Power) {
  IRKelvinatorAC irkelv(0);
  irkelv.begin();

  irkelv.on();
  EXPECT_TRUE(irkelv.getPower());

  irkelv.off();
  EXPECT_FALSE(irkelv.getPower());

  irkelv.setPower(true);
  EXPECT_TRUE(irkelv.getPower());

  irkelv.setPower(false);
  EXPECT_FALSE(irkelv.getPower());
}

TEST(TestKelvinatorClass, Temperature) {
  IRKelvinatorAC irkelv(0);
  irkelv.begin();

  irkelv.setTemp(0);
  EXPECT_EQ(KELVINATOR_MIN_TEMP, irkelv.getTemp());

  irkelv.setTemp(255);
  EXPECT_EQ(KELVINATOR_MAX_TEMP, irkelv.getTemp());

  irkelv.setTemp(KELVINATOR_MIN_TEMP);
  EXPECT_EQ(KELVINATOR_MIN_TEMP, irkelv.getTemp());

  irkelv.setTemp(KELVINATOR_MAX_TEMP);
  EXPECT_EQ(KELVINATOR_MAX_TEMP, irkelv.getTemp());

  irkelv.setTemp(KELVINATOR_MIN_TEMP - 1);
  EXPECT_EQ(KELVINATOR_MIN_TEMP, irkelv.getTemp());

  irkelv.setTemp(KELVINATOR_MAX_TEMP + 1);
  EXPECT_EQ(KELVINATOR_MAX_TEMP, irkelv.getTemp());

  irkelv.setTemp(17);
  EXPECT_EQ(17, irkelv.getTemp());

  irkelv.setTemp(21);
  EXPECT_EQ(21, irkelv.getTemp());

  irkelv.setTemp(25);
  EXPECT_EQ(25, irkelv.getTemp());

  irkelv.setTemp(29);
  EXPECT_EQ(29, irkelv.getTemp());
}

TEST(TestKelvinatorClass, OperatingMode) {
  IRKelvinatorAC irkelv(0);
  irkelv.begin();

  irkelv.setTemp(24);
  irkelv.setMode(KELVINATOR_AUTO);
  EXPECT_EQ(KELVINATOR_AUTO, irkelv.getMode());
  EXPECT_EQ(KELVINATOR_AUTO_TEMP, irkelv.getTemp());

  irkelv.setMode(KELVINATOR_COOL);
  EXPECT_EQ(KELVINATOR_COOL, irkelv.getMode());

  irkelv.setMode(KELVINATOR_HEAT);
  EXPECT_EQ(KELVINATOR_HEAT, irkelv.getMode());

  irkelv.setTemp(24);
  irkelv.setMode(KELVINATOR_DRY);
  EXPECT_EQ(KELVINATOR_DRY, irkelv.getMode());
  EXPECT_EQ(KELVINATOR_AUTO_TEMP, irkelv.getTemp());

  irkelv.setMode(KELVINATOR_FAN);
  EXPECT_EQ(KELVINATOR_FAN, irkelv.getMode());

  irkelv.setMode(KELVINATOR_HEAT + 1);
  EXPECT_EQ(KELVINATOR_AUTO, irkelv.getMode());

  irkelv.setMode(255);
  EXPECT_EQ(KELVINATOR_AUTO, irkelv.getMode());
}

TEST(TestKelvinatorClass, VaneSwing) {
  IRKelvinatorAC irkelv(0);
  irkelv.begin();

  irkelv.setSwingHorizontal(true);
  irkelv.setSwingVertical(false);

  irkelv.setSwingHorizontal(true);
  EXPECT_TRUE(irkelv.getSwingHorizontal());
  EXPECT_FALSE(irkelv.getSwingVertical());

  irkelv.setSwingVertical(true);
  EXPECT_TRUE(irkelv.getSwingHorizontal());
  EXPECT_TRUE(irkelv.getSwingVertical());

  irkelv.setSwingHorizontal(false);
  EXPECT_FALSE(irkelv.getSwingHorizontal());
  EXPECT_TRUE(irkelv.getSwingVertical());

  irkelv.setSwingVertical(false);
  EXPECT_FALSE(irkelv.getSwingHorizontal());
  EXPECT_FALSE(irkelv.getSwingVertical());
}

TEST(TestKelvinatorClass, QuietMode) {
  IRKelvinatorAC irkelv(0);
  irkelv.begin();

  irkelv.setQuiet(true);
  EXPECT_TRUE(irkelv.getQuiet());

  irkelv.setQuiet(false);
  EXPECT_FALSE(irkelv.getQuiet());

  irkelv.setQuiet(true);
  EXPECT_TRUE(irkelv.getQuiet());
}

TEST(TestKelvinatorClass, IonFilter) {
  IRKelvinatorAC irkelv(0);
  irkelv.begin();

  irkelv.setIonFilter(true);
  EXPECT_TRUE(irkelv.getIonFilter());

  irkelv.setIonFilter(false);
  EXPECT_FALSE(irkelv.getIonFilter());

  irkelv.setIonFilter(true);
  EXPECT_TRUE(irkelv.getIonFilter());
}

TEST(TestKelvinatorClass, Light) {
  IRKelvinatorAC irkelv(0);
  irkelv.begin();

  irkelv.setLight(true);
  EXPECT_TRUE(irkelv.getLight());

  irkelv.setLight(false);
  EXPECT_FALSE(irkelv.getLight());

  irkelv.setLight(true);
  EXPECT_TRUE(irkelv.getLight());
}

TEST(TestKelvinatorClass, XFan) {
  IRKelvinatorAC irkelv(0);
  irkelv.begin();

  irkelv.setXFan(true);
  EXPECT_TRUE(irkelv.getXFan());

  irkelv.setXFan(false);
  EXPECT_FALSE(irkelv.getXFan());

  irkelv.setXFan(true);
  EXPECT_TRUE(irkelv.getXFan());
}

TEST(TestKelvinatorClass, TurboFan) {
  IRKelvinatorAC irkelv(0);
  irkelv.begin();

  irkelv.setTurbo(true);
  EXPECT_TRUE(irkelv.getTurbo());

  irkelv.setTurbo(false);
  EXPECT_FALSE(irkelv.getTurbo());

  irkelv.setFan(2);
  irkelv.setTurbo(true);
  EXPECT_TRUE(irkelv.getTurbo());

  // Turbo mode is turned off if the temperature is changed.
  irkelv.setFan(3);
  EXPECT_FALSE(irkelv.getTurbo());

  // But only when it is changed, not set to the same value again.
  irkelv.setTurbo(true);
  irkelv.setFan(3);
  EXPECT_TRUE(irkelv.getTurbo());
}

TEST(TestKelvinatorClass, FanSpeed) {
  IRKelvinatorAC irkelv(0);
  irkelv.begin();

  irkelv.setFan(0);
  EXPECT_EQ(0, irkelv.getFan());

  irkelv.setFan(255);
  EXPECT_EQ(KELVINATOR_FAN_MAX, irkelv.getFan());

  irkelv.setFan(KELVINATOR_FAN_MAX);
  EXPECT_EQ(KELVINATOR_FAN_MAX, irkelv.getFan());

  irkelv.setFan(KELVINATOR_FAN_MAX + 1);
  EXPECT_EQ(KELVINATOR_FAN_MAX, irkelv.getFan());

  irkelv.setFan(KELVINATOR_FAN_MAX - 1);
  EXPECT_EQ(KELVINATOR_FAN_MAX - 1, irkelv.getFan());

  irkelv.setFan(1);
  EXPECT_EQ(1, irkelv.getFan());

  irkelv.setFan(1);
  EXPECT_EQ(1, irkelv.getFan());

  irkelv.setFan(3);
  EXPECT_EQ(3, irkelv.getFan());
}

TEST(TestKelvinatorClass, Checksums) {
  uint8_t kelv_code[KELVINATOR_STATE_LENGTH] = {
      0x19, 0x0B, 0x80, 0x50, 0x00, 0x00, 0x00, 0xE0,
      0x19, 0x0B, 0x80, 0x70, 0x00, 0x00, 0x10, 0xf0};

  EXPECT_TRUE(IRKelvinatorAC::validChecksum(kelv_code));
  // Change the array so the checksum is invalid.
  kelv_code[0] ^= 0xFF;
  EXPECT_FALSE(IRKelvinatorAC::validChecksum(kelv_code));
  // Restore the previous change, and change another byte.
  kelv_code[0] ^= 0xFF;
  kelv_code[4] ^= 0xFF;
  EXPECT_FALSE(IRKelvinatorAC::validChecksum(kelv_code));
  kelv_code[4] ^= 0xFF;
  // Change something in the 2nd block.
  kelv_code[10] ^= 0xFF;
  EXPECT_FALSE(IRKelvinatorAC::validChecksum(kelv_code));
  kelv_code[10] ^= 0xFF;
  EXPECT_TRUE(IRKelvinatorAC::validChecksum(kelv_code));
}

TEST(TestKelvinatorClass, SetAndGetRaw) {
  IRKelvinatorAC irkelv(0);
  uint8_t initialState[KELVINATOR_STATE_LENGTH] = {
      0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0xA0,
      0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0xA0};
  uint8_t expectedState[KELVINATOR_STATE_LENGTH] = {
      0x08, 0x05, 0x20, 0x50, 0x00, 0x00, 0x00, 0x70,
      0x08, 0x05, 0x20, 0x70, 0x00, 0x00, 0x00, 0x70};

  EXPECT_STATE_EQ(initialState, irkelv.getRaw(), KELVINATOR_BITS);
  // toggle the power state.
  irkelv.setPower(!irkelv.getPower());
  irkelv.setTemp(21);
  irkelv.setLight(true);
  EXPECT_STATE_EQ(expectedState, irkelv.getRaw(), KELVINATOR_BITS);
  irkelv.setRaw(initialState);
  EXPECT_STATE_EQ(initialState, irkelv.getRaw(), KELVINATOR_BITS);
}

TEST(TestKelvinatorClass, HumanReadable) {
  IRKelvinatorAC irkelv(0);

  EXPECT_EQ("Power: Off, Mode: 0 (AUTO), Temp: 16C, Fan: 0 (AUTO), Turbo: Off, "
            "Quiet: Off, XFan: Off, IonFilter: Off, Light: Off, "
            "Swing (Horizontal): Off, Swing (Vertical): Off",
            irkelv.toString());
  irkelv.on();
  irkelv.setMode(KELVINATOR_COOL);
  irkelv.setTemp(25);
  irkelv.setFan(KELVINATOR_FAN_MAX);
  irkelv.setXFan(true);
  irkelv.setIonFilter(true);
  irkelv.setLight(true);
  irkelv.setSwingHorizontal(true);
  EXPECT_EQ("Power: On, Mode: 1 (COOL), Temp: 25C, Fan: 5 (MAX), Turbo: Off, "
            "Quiet: Off, XFan: On, IonFilter: On, Light: On, "
            "Swing (Horizontal): On, Swing (Vertical): Off",
            irkelv.toString());
}

TEST(TestKelvinatorClass, MessageConstuction) {
  IRKelvinatorAC irkelv(0);
  IRsendTest irsend(4);
  irkelv.begin();
  irsend.begin();

  irkelv.setFan(1);
  irkelv.setMode(KELVINATOR_COOL);
  irkelv.setTemp(27);
  irkelv.setSwingVertical(false);
  irkelv.setSwingHorizontal(true);
  irkelv.setIonFilter(true);
  irkelv.setQuiet(false);
  irkelv.setLight(false);
  irkelv.setPower(true);
  irkelv.setTurbo(false);
  irkelv.setXFan(true);

  // Check everything for kicks.
  EXPECT_EQ(1, irkelv.getFan());
  EXPECT_EQ(KELVINATOR_COOL, irkelv.getMode());
  EXPECT_EQ(27, irkelv.getTemp());
  EXPECT_FALSE(irkelv.getSwingVertical());
  EXPECT_TRUE(irkelv.getSwingHorizontal());
  EXPECT_TRUE(irkelv.getIonFilter());
  EXPECT_FALSE(irkelv.getQuiet());
  EXPECT_FALSE(irkelv.getLight());
  EXPECT_TRUE(irkelv.getPower());
  EXPECT_FALSE(irkelv.getTurbo());
  EXPECT_TRUE(irkelv.getXFan());

  irsend.reset();
  irsend.sendKelvinator(irkelv.getRaw());
  EXPECT_EQ(
      "m9010s4505"
      "m680s1530m680s510m680s510m680s1530m680s1530m680s510m680s1530m680s510"
      "m680s1530m680s1530m680s510m680s1530m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s1530m680s1530"
      "m680s510m680s510m680s510m680s510m680s1530m680s510m680s1530m680s510"
      "m680s510m680s1530m680s510"
      "m680s19975"
      "m680s510m680s510m680s510m680s510m680s1530m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s1530m680s1530m680s1530m680s1530"
      "m680s39950"
      "m9010s4505"
      "m680s1530m680s510m680s510m680s1530m680s1530m680s510m680s1530m680s510"
      "m680s1530m680s1530m680s510m680s1530m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s1530m680s1530"
      "m680s510m680s510m680s510m680s510m680s1530m680s1530m680s1530m680s510"
      "m680s510m680s1530m680s510"
      "m680s19975"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s510m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s1530m680s510m680s510m680s510"
      "m680s510m680s510m680s510m680s510m680s1530m680s1530m680s1530m680s1530"
      "m680s39950", irsend.outputStr());
}

// Decode a synthetic Kelvinator message.
TEST(TestDecodeKelvinator, NormalSynthetic) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  uint8_t kelv_code[KELVINATOR_STATE_LENGTH] = {
      0x19, 0x0B, 0x80, 0x50, 0x00, 0x00, 0x00, 0xE0,
      0x19, 0x0B, 0x80, 0x70, 0x00, 0x00, 0x10, 0xf0};
  irsend.reset();
  irsend.sendKelvinator(kelv_code);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(KELVINATOR, irsend.capture.decode_type);
  ASSERT_EQ(KELVINATOR_BITS, irsend.capture.bits);
  EXPECT_STATE_EQ(kelv_code, irsend.capture.state, KELVINATOR_BITS);
}
