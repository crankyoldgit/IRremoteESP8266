// Copyright 2019 David Conran

#include "ir_Vestel.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

// Tests for sendVestelAC()

// Test sending typical data only.
TEST(TestSendVestelAC, SendDataOnly) {
  IRsendTest irsend(0);
  irsend.begin();

  irsend.reset();
  irsend.sendVestelAC(0x0F00D9001FEF201ULL);
  EXPECT_EQ(
      "m3110s9066"
      "m520s1535m520s480m520s480m520s480m520s480m520s480m520s480m520s480"
      "m520s480m520s1535m520s480m520s480m520s1535m520s1535m520s1535m520s1535"
      "m520s480m520s1535m520s1535m520s1535m520s1535m520s1535m520s1535m520s1535"
      "m520s1535m520s480m520s480m520s480m520s480m520s480m520s480m520s480"
      "m520s480m520s480m520s480m520s480m520s1535m520s480m520s480m520s1535"
      "m520s1535m520s480m520s1535m520s1535m520s480m520s480m520s480m520s480"
      "m520s480m520s480m520s480m520s480m520s1535m520s1535m520s1535m520s1535"
      "m520s100000",
      irsend.outputStr());
}

// Test sending typical data with repeats.
TEST(TestSendVestelAC, SendWithRepeats) {
  IRsendTest irsend(0);
  irsend.begin();

  irsend.reset();
  irsend.sendVestelAC(0x0F00D9001FEF201ULL, kVestelACBits, 2);  // two repeats.
  EXPECT_EQ(
      "m3110s9066"
      "m520s1535m520s480m520s480m520s480m520s480m520s480m520s480m520s480"
      "m520s480m520s1535m520s480m520s480m520s1535m520s1535m520s1535m520s1535"
      "m520s480m520s1535m520s1535m520s1535m520s1535m520s1535m520s1535m520s1535"
      "m520s1535m520s480m520s480m520s480m520s480m520s480m520s480m520s480"
      "m520s480m520s480m520s480m520s480m520s1535m520s480m520s480m520s1535"
      "m520s1535m520s480m520s1535m520s1535m520s480m520s480m520s480m520s480"
      "m520s480m520s480m520s480m520s480m520s1535m520s1535m520s1535m520s1535"
      "m520s100000"
      "m3110s9066"
      "m520s1535m520s480m520s480m520s480m520s480m520s480m520s480m520s480"
      "m520s480m520s1535m520s480m520s480m520s1535m520s1535m520s1535m520s1535"
      "m520s480m520s1535m520s1535m520s1535m520s1535m520s1535m520s1535m520s1535"
      "m520s1535m520s480m520s480m520s480m520s480m520s480m520s480m520s480"
      "m520s480m520s480m520s480m520s480m520s1535m520s480m520s480m520s1535"
      "m520s1535m520s480m520s1535m520s1535m520s480m520s480m520s480m520s480"
      "m520s480m520s480m520s480m520s480m520s1535m520s1535m520s1535m520s1535"
      "m520s100000"
      "m3110s9066"
      "m520s1535m520s480m520s480m520s480m520s480m520s480m520s480m520s480"
      "m520s480m520s1535m520s480m520s480m520s1535m520s1535m520s1535m520s1535"
      "m520s480m520s1535m520s1535m520s1535m520s1535m520s1535m520s1535m520s1535"
      "m520s1535m520s480m520s480m520s480m520s480m520s480m520s480m520s480"
      "m520s480m520s480m520s480m520s480m520s1535m520s480m520s480m520s1535"
      "m520s1535m520s480m520s1535m520s1535m520s480m520s480m520s480m520s480"
      "m520s480m520s480m520s480m520s480m520s1535m520s1535m520s1535m520s1535"
      "m520s100000",
      irsend.outputStr());
}

// Tests for IRVestelAC class.

TEST(TestVestelACClass, Power) {
  IRVestelAC ac(0);
  ac.begin();

  ac.setPower(true);
  EXPECT_TRUE(ac.getPower());

  ac.setPower(false);
  EXPECT_EQ(false, ac.getPower());

  ac.setPower(true);
  EXPECT_TRUE(ac.getPower());

  ac.off();
  EXPECT_EQ(false, ac.getPower());

  ac.on();
  EXPECT_TRUE(ac.getPower());
}

TEST(TestVestelACClass, OperatingMode) {
  IRVestelAC ac(0);
  ac.begin();

  ac.setMode(kVestelACAuto);
  EXPECT_EQ(kVestelACAuto, ac.getMode());

  ac.setMode(kVestelACCool);
  EXPECT_EQ(kVestelACCool, ac.getMode());

  ac.setMode(kVestelACHeat);
  EXPECT_EQ(kVestelACHeat, ac.getMode());

  ac.setMode(kVestelACFan);
  EXPECT_EQ(kVestelACFan, ac.getMode());

  ac.setMode(kVestelACDry);
  EXPECT_EQ(kVestelACDry, ac.getMode());

  ac.setMode(kVestelACAuto - 1);
  EXPECT_EQ(kVestelACAuto, ac.getMode());

  ac.setMode(kVestelACCool);
  EXPECT_EQ(kVestelACCool, ac.getMode());

  ac.setMode(kVestelACHeat + 1);
  EXPECT_EQ(kVestelACAuto, ac.getMode());

  ac.setMode(255);
  EXPECT_EQ(kVestelACAuto, ac.getMode());
}

TEST(TestVestelACClass, Temperature) {
  IRVestelAC ac(0);
  ac.begin();

  ac.setTemp(kVestelACMinTempC);
  EXPECT_EQ(kVestelACMinTempC, ac.getTemp());

  ac.setTemp(kVestelACMinTempC + 1);
  EXPECT_EQ(kVestelACMinTempC + 1, ac.getTemp());

  ac.setTemp(kVestelACMaxTemp);
  EXPECT_EQ(kVestelACMaxTemp, ac.getTemp());

  ac.setTemp(kVestelACMinTempC - 1);
  EXPECT_EQ(kVestelACMinTempC, ac.getTemp());

  ac.setTemp(kVestelACMaxTemp + 1);
  EXPECT_EQ(kVestelACMaxTemp, ac.getTemp());

  ac.setTemp(23);
  EXPECT_EQ(23, ac.getTemp());

  ac.setTemp(0);
  EXPECT_EQ(kVestelACMinTempC, ac.getTemp());

  ac.setTemp(255);
  EXPECT_EQ(kVestelACMaxTemp, ac.getTemp());
}

TEST(TestVestelACClass, FanSpeed) {
  IRVestelAC ac(0);
  ac.begin();
  ac.setFan(kVestelACFanLow);

  ac.setFan(kVestelACFanAuto);
  EXPECT_EQ(kVestelACFanAuto, ac.getFan());

  ac.setFan(kVestelACFanLow);
  EXPECT_EQ(kVestelACFanLow, ac.getFan());
  ac.setFan(kVestelACFanMed);
  EXPECT_EQ(kVestelACFanMed, ac.getFan());
  ac.setFan(kVestelACFanHigh);
  EXPECT_EQ(kVestelACFanHigh, ac.getFan());

  ac.setFan(kVestelACFanHigh);
  EXPECT_EQ(kVestelACFanHigh, ac.getFan());
}

TEST(TestVestelACClass, Swing) {
  IRVestelAC ac(0);
  ac.begin();

  ac.setSwing(true);
  EXPECT_TRUE(ac.getSwing());

  ac.setSwing(false);
  EXPECT_EQ(false, ac.getSwing());

  ac.setSwing(true);
  EXPECT_TRUE(ac.getSwing());
}

TEST(TestVestelACClass, MessageConstuction) {
  IRVestelAC ac(0);

  EXPECT_EQ(
      "Power: On, Mode: 0 (AUTO), Temp: 25C, Fan: 13 (AUTO HOT), Sleep: Off, "
      "Turbo: Off, Ion: Off, Swing: Off",
      ac.toString());
  ac.setMode(kVestelACCool);
  ac.setTemp(21);
  ac.setFan(kVestelACFanHigh);
  EXPECT_EQ(
      "Power: On, Mode: 1 (COOL), Temp: 21C, Fan: 11 (HIGH), Sleep: Off, "
      "Turbo: Off, Ion: Off, Swing: Off",
      ac.toString());
  ac.setSwing(true);
  ac.setIon(true);
  ac.setTurbo(true);
  EXPECT_EQ(
      "Power: On, Mode: 1 (COOL), Temp: 21C, Fan: 11 (HIGH), Sleep: Off, "
      "Turbo: On, Ion: On, Swing: On",
      ac.toString());

  // Now change a few already set things.
  ac.setSleep(true);
  ac.setMode(kVestelACHeat);
  EXPECT_EQ(
      "Power: On, Mode: 4 (HEAT), Temp: 21C, Fan: 11 (HIGH), Sleep: On, "
      "Turbo: Off, Ion: On, Swing: On",
      ac.toString());

  ac.setTemp(25);
  ac.setPower(false);
  EXPECT_EQ(
      "Power: Off, Mode: 4 (HEAT), Temp: 25C, Fan: 11 (HIGH), Sleep: On, "
      "Turbo: Off, Ion: On, Swing: On",
      ac.toString());

  // Check that the checksum is valid.
  EXPECT_TRUE(IRVestelAC::validChecksum(ac.getRaw()));
}

// Tests for decodeVestelAC().

// Decode normal "synthetic" messages.
TEST(TestDecodeVestelAC, NormalDecodeWithStrict) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  // With the specific decoder.
  uint64_t expectedState = 0x0F00D9001FEF201ULL;
  irsend.reset();
  irsend.sendVestelAC(expectedState);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeVestelAC(&irsend.capture, kVestelACBits, true));
  EXPECT_EQ(VESTEL_AC, irsend.capture.decode_type);
  EXPECT_EQ(kVestelACBits, irsend.capture.bits);
  EXPECT_FALSE(irsend.capture.repeat);
  EXPECT_EQ(expectedState, irsend.capture.value);
  EXPECT_EQ(0, irsend.capture.address);
  EXPECT_EQ(0, irsend.capture.command);

  // With the all the decoders.
  irsend.reset();
  irsend.sendVestelAC(expectedState);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(VESTEL_AC, irsend.capture.decode_type);
  EXPECT_EQ(kVestelACBits, irsend.capture.bits);
  EXPECT_FALSE(irsend.capture.repeat);
  EXPECT_EQ(expectedState, irsend.capture.value);
  EXPECT_EQ(0, irsend.capture.address);
  EXPECT_EQ(0, irsend.capture.command);

  IRVestelAC ac(0);
  ac.begin();
  ac.setRaw(irsend.capture.value);
  EXPECT_EQ(
      "Power: On, Mode: 0 (AUTO), Temp: 25C, Fan: 13 (AUTO HOT), Sleep: Off, "
      "Turbo: Off, Ion: Off, Swing: Off",
      ac.toString());
}

// General housekeeping
TEST(TestDecodeVestelAC, Housekeeping) {
  ASSERT_EQ("VESTEL_AC", typeToString(VESTEL_AC));
  ASSERT_FALSE(hasACState(VESTEL_AC));  // Uses uint64_t, not uint8_t*.
}
