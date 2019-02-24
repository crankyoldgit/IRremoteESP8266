// Copyright 2019 David Conran

#include "ir_Teco.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

// General housekeeping
TEST(TestTeco, Housekeeping) {
  ASSERT_EQ("TECO", typeToString(TECO));
  ASSERT_FALSE(hasACState(TECO));  // Uses uint64_t, not uint8_t*.
}

// Tests for sendTeco()

// Test sending typical data only.
TEST(TestSendTeco, SendDataOnly) {
  IRsendTest irsend(0);
  irsend.begin();

  irsend.reset();
  irsend.sendTeco(kTecoReset);
  EXPECT_EQ(
      "m9000s4440"
      "m620s580m620s580m620s580m620s580m620s580m620s580m620s580m620s580"
      "m620s580m620s580m620s580m620s580m620s580m620s580m620s580m620s580"
      "m620s580m620s580m620s580m620s580m620s580m620s580m620s580m620s580"
      "m620s580m620s580m620s580m620s580m620s1650m620s580m620s1650m620s580"
      "m620s580m620s1650m620s580"
      "m620s1000000",
      irsend.outputStr());
}

// Test sending typical data with repeats.
TEST(TestSendTeco, SendWithRepeats) {
  IRsendTest irsend(0);
  irsend.begin();

  irsend.reset();
  irsend.sendTeco(kTecoReset, kTecoBits, 2);  // two repeats.
  EXPECT_EQ(
      "m9000s4440"
      "m620s580m620s580m620s580m620s580m620s580m620s580m620s580m620s580"
      "m620s580m620s580m620s580m620s580m620s580m620s580m620s580m620s580"
      "m620s580m620s580m620s580m620s580m620s580m620s580m620s580m620s580"
      "m620s580m620s580m620s580m620s580m620s1650m620s580m620s1650m620s580"
      "m620s580m620s1650m620s580"
      "m620s1000000"
      "m9000s4440"
      "m620s580m620s580m620s580m620s580m620s580m620s580m620s580m620s580"
      "m620s580m620s580m620s580m620s580m620s580m620s580m620s580m620s580"
      "m620s580m620s580m620s580m620s580m620s580m620s580m620s580m620s580"
      "m620s580m620s580m620s580m620s580m620s1650m620s580m620s1650m620s580"
      "m620s580m620s1650m620s580"
      "m620s1000000"
      "m9000s4440"
      "m620s580m620s580m620s580m620s580m620s580m620s580m620s580m620s580"
      "m620s580m620s580m620s580m620s580m620s580m620s580m620s580m620s580"
      "m620s580m620s580m620s580m620s580m620s580m620s580m620s580m620s580"
      "m620s580m620s580m620s580m620s580m620s1650m620s580m620s1650m620s580"
      "m620s580m620s1650m620s580"
      "m620s1000000",
      irsend.outputStr());
}


// Tests for IRTeco class.

TEST(TestTecoClass, Power) {
  IRTecoAC ac(0);
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

TEST(TestTecoClass, OperatingMode) {
  IRTecoAC ac(0);
  ac.begin();

  ac.setMode(kTecoAuto);
  EXPECT_EQ(kTecoAuto, ac.getMode());

  ac.setMode(kTecoCool);
  EXPECT_EQ(kTecoCool, ac.getMode());

  ac.setMode(kTecoHeat);
  EXPECT_EQ(kTecoHeat, ac.getMode());

  ac.setMode(kTecoFan);
  EXPECT_EQ(kTecoFan, ac.getMode());

  ac.setMode(kTecoDry);
  EXPECT_EQ(kTecoDry, ac.getMode());

  ac.setMode(kTecoAuto - 1);
  EXPECT_EQ(kTecoAuto, ac.getMode());

  ac.setMode(kTecoCool);
  EXPECT_EQ(kTecoCool, ac.getMode());

  ac.setMode(kTecoHeat + 1);
  EXPECT_EQ(kTecoAuto, ac.getMode());

  ac.setMode(255);
  EXPECT_EQ(kTecoAuto, ac.getMode());
}

TEST(TestTecoClass, Temperature) {
  IRTecoAC ac(0);
  ac.begin();

  ac.setTemp(kTecoMinTemp);
  EXPECT_EQ(kTecoMinTemp, ac.getTemp());

  ac.setTemp(kTecoMinTemp + 1);
  EXPECT_EQ(kTecoMinTemp + 1, ac.getTemp());

  ac.setTemp(kTecoMaxTemp);
  EXPECT_EQ(kTecoMaxTemp, ac.getTemp());

  ac.setTemp(kTecoMinTemp - 1);
  EXPECT_EQ(kTecoMinTemp, ac.getTemp());

  ac.setTemp(kTecoMaxTemp + 1);
  EXPECT_EQ(kTecoMaxTemp, ac.getTemp());

  ac.setTemp(23);
  EXPECT_EQ(23, ac.getTemp());

  ac.setTemp(0);
  EXPECT_EQ(kTecoMinTemp, ac.getTemp());

  ac.setTemp(255);
  EXPECT_EQ(kTecoMaxTemp, ac.getTemp());
}

TEST(TestTecoClass, FanSpeed) {
  IRTecoAC ac(0);
  ac.begin();
  ac.setFan(kTecoFanLow);

  ac.setFan(kTecoFanAuto);
  EXPECT_EQ(kTecoFanAuto, ac.getFan());

  ac.setFan(kTecoFanLow);
  EXPECT_EQ(kTecoFanLow, ac.getFan());
  ac.setFan(kTecoFanMed);
  EXPECT_EQ(kTecoFanMed, ac.getFan());
  ac.setFan(kTecoFanHigh);
  EXPECT_EQ(kTecoFanHigh, ac.getFan());

  ac.setFan(kTecoFanHigh);
  EXPECT_EQ(kTecoFanHigh, ac.getFan());
}

TEST(TestTecoClass, Swing) {
  IRTecoAC ac(0);
  ac.begin();

  ac.setSwing(true);
  EXPECT_TRUE(ac.getSwing());

  ac.setSwing(false);
  EXPECT_EQ(false, ac.getSwing());

  ac.setSwing(true);
  EXPECT_TRUE(ac.getSwing());
}

TEST(TestTecoClass, Sleep) {
  IRTecoAC ac(0);
  ac.begin();

  ac.setSleep(true);
  EXPECT_TRUE(ac.getSleep());

  ac.setSleep(false);
  EXPECT_EQ(false, ac.getSleep());

  ac.setSleep(true);
  EXPECT_TRUE(ac.getSleep());
}

TEST(TestTecoClass, MessageConstuction) {
  IRTecoAC ac(0);

  EXPECT_EQ(
      "Power: Off, Mode: 0 (AUTO), Temp: 16C, Fan: 0 (Auto), Sleep: Off, "
      "Swing: Off",
      ac.toString());
  ac.setPower(true);
  ac.setMode(kTecoCool);
  ac.setTemp(21);
  ac.setFan(kTecoFanHigh);
  ac.setSwing(false);
  EXPECT_EQ(
      "Power: On, Mode: 1 (COOL), Temp: 21C, Fan: 3 (High), Sleep: Off, "
      "Swing: Off",
      ac.toString());
  ac.setSwing(true);
  EXPECT_EQ(
      "Power: On, Mode: 1 (COOL), Temp: 21C, Fan: 3 (High), Sleep: Off, "
      "Swing: On",
      ac.toString());
  ac.setSwing(false);
  ac.setFan(kTecoFanLow);
  ac.setSleep(true);
  ac.setMode(kTecoHeat);
  EXPECT_EQ(
      "Power: On, Mode: 4 (HEAT), Temp: 21C, Fan: 1 (Low), Sleep: On, "
      "Swing: Off",
      ac.toString());
  ac.setSleep(false);
  EXPECT_EQ(
      "Power: On, Mode: 4 (HEAT), Temp: 21C, Fan: 1 (Low), Sleep: Off, "
      "Swing: Off",
      ac.toString());
  ac.setTemp(25);
  EXPECT_EQ(
      "Power: On, Mode: 4 (HEAT), Temp: 25C, Fan: 1 (Low), Sleep: Off, "
      "Swing: Off",
      ac.toString());
}

// Tests for decodeTeco().

// Decode normal "synthetic" messages.
TEST(TestDecodeTeco, NormalDecodeWithStrict) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  // With the specific decoder.
  uint64_t expectedState = kTecoReset;
  irsend.reset();
  irsend.sendTeco(expectedState);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeTeco(&irsend.capture, kTecoBits, true));
  EXPECT_EQ(TECO, irsend.capture.decode_type);
  EXPECT_EQ(kTecoBits, irsend.capture.bits);
  EXPECT_FALSE(irsend.capture.repeat);
  EXPECT_EQ(expectedState, irsend.capture.value);
  EXPECT_EQ(0, irsend.capture.address);
  EXPECT_EQ(0, irsend.capture.command);

  // With the all the decoders.
  irsend.reset();
  irsend.sendTeco(expectedState);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(TECO, irsend.capture.decode_type);
  EXPECT_EQ(kTecoBits, irsend.capture.bits);
  EXPECT_FALSE(irsend.capture.repeat);
  EXPECT_EQ(expectedState, irsend.capture.value);
  EXPECT_EQ(0, irsend.capture.address);
  EXPECT_EQ(0, irsend.capture.command);

  IRTecoAC ac(0);
  ac.begin();
  ac.setRaw(irsend.capture.value);
  EXPECT_EQ(
      "Power: Off, Mode: 0 (AUTO), Temp: 16C, Fan: 0 (Auto), Sleep: Off, "
      "Swing: Off",
      ac.toString());
}
