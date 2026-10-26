// Copyright 2021 Davide Depau

#include <string>
#include "IRac.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

// Tests for sendKelon().

// Test sending typical data only.
TEST(TestSendKelon, SendDataOnly) {
  IRsendTest irsend(kGpioUnused);
  irsend.begin();

  // Temp: 26C, Mode: 2 (Cool), Fan: 0 (Auto), Sleep: Off, Dry: 0, Timer: Off,
  // Turbo: Off
  irsend.reset();
  irsend.sendKelon(0x82000683);
  EXPECT_EQ(
      "f38000d50"
      "m9000s4600m560s1680m560s1680m560s600m560s600m560s600m560s600"
      "m560s600m560s1680m560s600m560s1680m560s1680m560s600m560s600m560s600"
      "m560s600m560s600m560s600m560s600m560s600m560s600m560s600m560s600"
      "m560s600m560s600m560s600m560s1680m560s600m560s600m560s600m560s600"
      "m560s600m560s1680m560s600m560s600m560s600m560s600m560s600m560s600"
      "m560s600m560s600m560s600m560s600m560s600m560s600m560s600m560s600"
      "m560s600m560s600m560s200000",
      irsend.outputStr());

  // Temp: 18C, Mode: 2 (Cool), Fan: 1 (Low), Sleep: Off, Dry: 0, Timer: Off,
  // Turbo: On
  irsend.reset();
  irsend.sendKelon(0x900002010683);
  EXPECT_EQ(
      "f38000d50"
      "m9000s4600m560s1680m560s1680m560s600m560s600m560s600m560s600m560s600"
      "m560s1680m560s600m560s1680m560s1680m560s600m560s600m560s600"
      "m560s600m560s600m560s1680m560s600m560s600m560s600m560s600"
      "m560s600m560s600m560s600m560s600m560s1680m560s600m560s600m560s600"
      "m560s600m560s600m560s600m560s600m560s600m560s600m560s600"
      "m560s600m560s600m560s600m560s600m560s600m560s600m560s600"
      "m560s600m560s1680m560s600m560s600m560s1680m560s200000",
      irsend.outputStr());

  // Temp: 23C, Mode: 0 (Heat), Fan: 0 (Auto), Sleep: Off, Dry: 0, Timer: Off,
  // Turbo: Off, Power Toggle: On
  irsend.reset();
  irsend.sendKelon(0x50040683);
  EXPECT_EQ(
      "f38000d50"
      "m9000s4600m560s1680m560s1680m560s600m560s600m560s600m560s600"
      "m560s600m560s1680m560s600m560s1680m560s1680m560s600m560s600"
      "m560s600m560s600m560s600m560s600m560s600m560s1680m560s600m560s600"
      "m560s600m560s600m560s600m560s600m560s600m560s600m560s600m560s1680"
      "m560s600m560s1680m560s600m560s600m560s600m560s600m560s600m560s600"
      "m560s600m560s600m560s600m560s600m560s600m560s600m560s600m560s600"
      "m560s600m560s600m560s600m560s200000",
      irsend.outputStr());

  // Temp: 26C, Mode: 2 (Cool), Fan: 0 (Auto), Sleep: Off, Dry: 0, Timer:
  // On (9.5h), Turbo:
  irsend.reset();
  irsend.sendKelon(0x138A000683);
  EXPECT_EQ(
      "f38000d50"
      "m9000s4600m560s1680m560s1680m560s600m560s600m560s600"
      "m560s600m560s600m560s1680m560s600m560s1680m560s1680m560s600"
      "m560s600m560s600m560s600m560s600m560s600m560s600m560s600"
      "m560s600m560s600m560s600m560s600m560s600m560s600m560s1680"
      "m560s600m560s1680m560s600m560s600m560s600m560s1680m560s1680"
      "m560s1680m560s600m560s600m560s1680m560s600m560s600m560s600"
      "m560s600m560s600m560s600m560s600m560s600m560s600m560s600"
      "m560s600m560s200000",
      irsend.outputStr());

  // Temp: 26C, Mode: 2 (Cool), Fan: 0 (Auto), Sleep: Off, Dry: 0, Timer:
  // On (15h), Turbo: Off:
  irsend.reset();
  irsend.sendKelon(0x198A000683);
  EXPECT_EQ(
      "f38000d50"
      "m9000s4600m560s1680m560s1680m560s600m560s600m560s600"
      "m560s600m560s600m560s1680m560s600m560s1680m560s1680"
      "m560s600m560s600m560s600m560s600m560s600m560s600m560s600"
      "m560s600m560s600m560s600m560s600m560s600m560s600m560s600"
      "m560s1680m560s600m560s1680m560s600m560s600m560s600m560s1680"
      "m560s1680m560s600m560s600m560s1680m560s1680m560s600"
      "m560s600m560s600m560s600m560s600m560s600m560s600m560s600"
      "m560s600m560s600m560s600m560s200000",
      irsend.outputStr());
}

// Tests for decodeKelon().
// Decode normal Kelon messages.
TEST(TestDecodeKelon, Timer12HSmartMode) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  irsend.reset();
  irsend.sendKelon(0x1679030683);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(KELON, irsend.capture.decode_type);
  EXPECT_EQ(kKelonBits, irsend.capture.bits);
  EXPECT_EQ(0x1679030683, irsend.capture.value);
  EXPECT_EQ(
      "Temp: 25C, Mode: 1 (Auto), Fan: 3 (High), Sleep: Off, Dry: 0, "
      "Timer: 12:00, Turbo: Off",
      IRAcUtils::resultAcToString(&irsend.capture));
  stdAc::state_t result, prev;
  ASSERT_TRUE(IRAcUtils::decodeToState(&irsend.capture, &result, &prev));
}

TEST(TestDecodeKelon, Timer5_5hSuperCoolMode) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  irsend.reset();
  irsend.sendKelon(0x100B0A010683);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(KELON, irsend.capture.decode_type);
  EXPECT_EQ(kKelonBits, irsend.capture.bits);
  EXPECT_EQ(0x100B0A010683, irsend.capture.value);
  EXPECT_EQ(
      "Temp: 18C, Mode: 2 (Cool), Fan: 1 (Low), Sleep: Off, Dry: 0, "
      "Timer: 05:30, Turbo: On",
      IRAcUtils::resultAcToString(&irsend.capture));
  stdAc::state_t result, prev;
  ASSERT_TRUE(IRAcUtils::decodeToState(&irsend.capture, &result, &prev));
}

TEST(TestDecodeKelon, ChangeSettingsWithTimerSetHeatMode) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  irsend.reset();
  irsend.sendKelon(0x58000683);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(KELON, irsend.capture.decode_type);
  EXPECT_EQ(kKelonBits, irsend.capture.bits);
  EXPECT_EQ(0x58000683, irsend.capture.value);
  EXPECT_EQ(
      "Temp: 23C, Mode: 0 (Heat), Fan: 0 (Auto), Sleep: Off, Dry: 0, "
      "Timer: On, Turbo: Off",
      IRAcUtils::resultAcToString(&irsend.capture));
  stdAc::state_t result, prev;
  ASSERT_TRUE(IRAcUtils::decodeToState(&irsend.capture, &result, &prev));
}

TEST(TestDecodeKelon, TestPowerToggleDryMode) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  irsend.reset();
  irsend.sendKelon(0x83040683);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(KELON, irsend.capture.decode_type);
  EXPECT_EQ(kKelonBits, irsend.capture.bits);
  EXPECT_EQ(0x83040683, irsend.capture.value);
  EXPECT_EQ(
      "Temp: 26C, Mode: 3 (Dry), Fan: 0 (Auto), Sleep: Off, Dry: 0, Timer:"
      " Off, Turbo: Off, Power Toggle: On",
      IRAcUtils::resultAcToString(&irsend.capture));
  stdAc::state_t result, prev;
  ASSERT_TRUE(IRAcUtils::decodeToState(&irsend.capture, &result, &prev));
}

TEST(TestDecodeKelon, TestSwingToggleDryMode) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  irsend.reset();
  irsend.sendKelon(0x83800683);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(KELON, irsend.capture.decode_type);
  EXPECT_EQ(kKelonBits, irsend.capture.bits);
  EXPECT_EQ(0x83800683, irsend.capture.value);
  EXPECT_EQ(
      "Temp: 26C, Mode: 3 (Dry), Fan: 0 (Auto), Sleep: Off, Dry: 0, Timer:"
      " Off, Turbo: Off, Swing(V) Toggle: On",
      IRAcUtils::resultAcToString(&irsend.capture));
  stdAc::state_t result, prev;
  ASSERT_TRUE(IRAcUtils::decodeToState(&irsend.capture, &result, &prev));
}

TEST(TestDecodeKelon, TestDryGradeNegativeValue) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  irsend.reset();
  irsend.sendKelon(0x83600683);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(KELON, irsend.capture.decode_type);
  EXPECT_EQ(kKelonBits, irsend.capture.bits);
  EXPECT_EQ(0x83600683, irsend.capture.value);
  EXPECT_EQ(
      "Temp: 26C, Mode: 3 (Dry), Fan: 0 (Auto), Sleep: Off, Dry: -2,"
      " Timer: Off, Turbo: Off",
      IRAcUtils::resultAcToString(&irsend.capture));
  stdAc::state_t result, prev;
  ASSERT_TRUE(IRAcUtils::decodeToState(&irsend.capture, &result, &prev));
}

TEST(TestIRKelonClass, SetAndGetRaw) {
  uint64_t rawData = 0x100B0A010683;
  IRKelonAc ac(kGpioUnused);
  ac.setRaw(rawData);
  EXPECT_EQ(rawData, ac.getRaw());
}

TEST(TestIRKelonClass, SetAndGetTemp) {
  IRKelonAc ac(kGpioUnused);

  for (uint8_t temp = kKelonMinTemp; temp <= kKelonMaxTemp; temp++) {
    ac.setTemp(temp);
    EXPECT_EQ(temp, ac.getTemp());
  }

  // Check bounds
  ac.setTemp(kKelonMinTemp - 1);
  EXPECT_EQ(kKelonMinTemp, ac.getTemp());
  ac.setTemp(kKelonMaxTemp + 1);
  EXPECT_EQ(kKelonMaxTemp, ac.getTemp());
}

TEST(TestIRKelonClass, SetAndGetTimer) {
  IRKelonAc ac(kGpioUnused);

  // 0.5h to 10h timers have a granularity of 30 minutes
  for (uint16_t minutes = 0; minutes <= 60 * 10; minutes += 30) {
    ac.setTimer(minutes);
    EXPECT_EQ(minutes, ac.getTimer());
  }
  // 10h to 24h timers have a granularity of 1h
  for (uint16_t minutes = 600; minutes <= 60 * 24; minutes += 60) {
    ac.setTimer(minutes);
    EXPECT_EQ(minutes, ac.getTimer());
  }
  // For 10h to 24h timers, we expect the minutes to be floored down to the hour
  for (uint16_t minutes = 600 + 30; minutes <= 60 * 24; minutes += 60) {
    ac.setTimer(minutes);
    EXPECT_EQ(minutes - 30, ac.getTimer());
  }
}

TEST(TestIRKelonClass, CheckToggles) {
  IRKelonAc ac(kGpioUnused);

  ac.setTogglePower(true);
  EXPECT_TRUE(ac.getTogglePower());
  ac.send();
  EXPECT_FALSE(ac.getTogglePower());

  ac.setToggleSwingVertical(true);
  EXPECT_TRUE(ac.getToggleSwingVertical());
  ac.send();
  EXPECT_FALSE(ac.getToggleSwingVertical());

  // Known state with a power toggle
  ac.setRaw(0x62040683);
  EXPECT_TRUE(ac.getTogglePower());

  // Known state with a swing toggle
  ac.setRaw(0x62800683);
  EXPECT_TRUE(ac.getToggleSwingVertical());
}

TEST(TestIRKelonClass, SetAndGetMode) {
  IRKelonAc ac(kGpioUnused);

  uint8_t initial_temp = 20;
  ac.setMode(kKelonModeHeat);
  ac.setTemp(initial_temp);

  ac.setMode(kKelonModeCool);
  EXPECT_EQ(kKelonModeCool, ac.getMode());
  EXPECT_EQ(initial_temp, ac.getTemp());

  ac.setMode(kKelonModeHeat);
  EXPECT_EQ(kKelonModeHeat, ac.getMode());
  EXPECT_EQ(initial_temp, ac.getTemp());

  ac.setMode(kKelonModeDry);
  EXPECT_EQ(kKelonModeDry, ac.getMode());
  EXPECT_EQ(25, ac.getTemp());

  // Should revert back to previous temp
  ac.setMode(kKelonModeHeat);
  EXPECT_EQ(kKelonModeHeat, ac.getMode());
  EXPECT_EQ(initial_temp, ac.getTemp());

  ac.setMode(kKelonModeFan);
  EXPECT_EQ(kKelonModeFan, ac.getMode());
  EXPECT_EQ(25, ac.getTemp());

  // Should revert back to previous temp
  ac.setMode(kKelonModeHeat);
  EXPECT_EQ(kKelonModeHeat, ac.getMode());
  EXPECT_EQ(initial_temp, ac.getTemp());

  ac.setMode(kKelonModeSmart);
  EXPECT_EQ(kKelonModeSmart, ac.getMode());
  EXPECT_EQ(26, ac.getTemp());

  // Should revert back to previous temp
  ac.setMode(kKelonModeHeat);
  EXPECT_EQ(kKelonModeHeat, ac.getMode());
  EXPECT_EQ(initial_temp, ac.getTemp());
}

TEST(TestIRKelonClass, CheckSuperCoolMode) {
  IRKelonAc ac(kGpioUnused);

  uint8_t initial_temp = 20;
  uint8_t initial_fan = kKelonFanMin;
  ac.setMode(kKelonModeHeat);
  ac.setTemp(initial_temp);
  ac.setFan(initial_fan);

  ac.setSupercool(true);
  EXPECT_TRUE(ac.getSupercool());
  EXPECT_EQ(kKelonModeCool, ac.getMode());
  EXPECT_EQ(kKelonMinTemp, ac.getTemp());
  EXPECT_EQ(kKelonFanMax, ac.getFan());

  // Should revert back to previous temp and mode
  ac.setSupercool(false);
  EXPECT_EQ(kKelonModeHeat, ac.getMode());
  EXPECT_EQ(initial_temp, ac.getTemp());
  EXPECT_EQ(initial_fan, ac.getFan());

  ac.setSupercool(true);
  EXPECT_TRUE(ac.getSupercool());
  EXPECT_EQ(kKelonModeCool, ac.getMode());
  EXPECT_EQ(kKelonMinTemp, ac.getTemp());
  EXPECT_EQ(kKelonFanMax, ac.getFan());

  // Setting any mode should cancel supercool
  ac.setMode(kKelonModeHeat);
  EXPECT_EQ(kKelonModeHeat, ac.getMode());
  EXPECT_EQ(initial_temp, ac.getTemp());
  EXPECT_EQ(initial_fan, ac.getFan());
}

TEST(TestIRKelonClass, SetAndGetDryGrade) {
  IRKelonAc ac(kGpioUnused);

  for (int i = -2; i <= 2; i++) {
    ac.setDryGrade(i);
    EXPECT_EQ(i, ac.getDryGrade());
  }
  // Check bounds
  ac.setDryGrade(3);
  EXPECT_EQ(2, ac.getDryGrade());
  ac.setDryGrade(-3);
  EXPECT_EQ(-2, ac.getDryGrade());
}

TEST(TestIRKelonClass, toCommon) {
  IRKelonAc ac(kGpioUnused);

  ac.setSleep(false);
  ac.setTemp(23);
  ac.setMode(kKelonModeHeat);
  ac.setFan(kAmcorFanMed);
  auto common = ac.toCommon();

  EXPECT_EQ(decode_type_t::KELON, common.protocol);
  EXPECT_EQ(23, common.degrees);
  EXPECT_TRUE(common.celsius);
  EXPECT_EQ(stdAc::fanspeed_t::kMedium, common.fanspeed);
  EXPECT_FALSE(common.turbo);
  EXPECT_EQ(-1, common.sleep);

  ac.setSleep(true);
  ac.setSupercool(true);

  common = ac.toCommon();
  EXPECT_EQ(decode_type_t::KELON, common.protocol);
  EXPECT_EQ(kKelonMinTemp, common.degrees);
  EXPECT_TRUE(common.celsius);
  EXPECT_EQ(stdAc::fanspeed_t::kHigh, common.fanspeed);
  EXPECT_TRUE(common.turbo);
  EXPECT_EQ(0, common.sleep);
}

TEST(TestIRKelonClass, toCommonToggles) {
  IRKelonAc ac(kGpioUnused);

  stdAc::state_t common = ac.toCommon();
  stdAc::state_t prev = common;

  EXPECT_TRUE(common.power);
  EXPECT_EQ(stdAc::swingv_t::kAuto, common.swingv);

  ac.setTogglePower(true);
  ac.setToggleSwingVertical(true);

  common = ac.toCommon(&prev);
  prev = common;

  EXPECT_FALSE(common.power);
  EXPECT_EQ(stdAc::swingv_t::kOff, common.swingv);

  ac.setTogglePower(true);
  ac.setToggleSwingVertical(true);

  common = ac.toCommon(&prev);

  EXPECT_TRUE(common.power);
  EXPECT_EQ(stdAc::swingv_t::kAuto, common.swingv);
}

TEST(TestUtils, Housekeeping) {
  // KELON
  ASSERT_EQ("KELON", typeToString(decode_type_t::KELON));
  ASSERT_EQ(decode_type_t::KELON, strToDecodeType("KELON"));
  ASSERT_FALSE(hasACState(decode_type_t::KELON));
  ASSERT_TRUE(IRac::isProtocolSupported(decode_type_t::KELON));
  ASSERT_EQ(kKelonBits, IRsend::defaultBits(decode_type_t::KELON));
  ASSERT_EQ(kNoRepeat, IRsend::minRepeats(decode_type_t::KELON));

  // KELON168
  ASSERT_EQ("KELON168", typeToString(decode_type_t::KELON168));
  ASSERT_EQ(decode_type_t::KELON168, strToDecodeType("KELON168"));
  ASSERT_TRUE(hasACState(decode_type_t::KELON168));
  ASSERT_FALSE(IRac::isProtocolSupported(decode_type_t::KELON168));
  ASSERT_EQ(kKelon168Bits, IRsend::defaultBits(decode_type_t::KELON168));
  ASSERT_EQ(kNoRepeat, IRsend::minRepeats(decode_type_t::KELON168));
}

TEST(TestDecodeKelon, Discussion1744) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  // Ref: https://github.com/crankyoldgit/IRremoteESP8266/discussions/1744#discussioncomment-2061968
  // Note: This message is NOT at 48-bit Kelon protocol message.
  //       It is actually 48 + 64 + 56 = 168 bit (21 byte) message.
  //       i.e. A different protocol all together.
  const uint16_t rawData[343] = {
      8922, 4494,  // Header
      548, 1714, 524, 1718, 524, 572, 524, 550,  // Byte 0
      550, 556, 550, 560, 548, 586, 524, 1724,
      526, 564, 524, 1692, 550, 1696, 550, 552,  // Byte 1
      550, 554, 550, 560, 548, 586, 524, 550,
      548, 1688, 550, 1694, 548, 548, 550, 552,  // Byte 2
      548, 556, 548, 582, 524, 586, 524, 576,
      524, 540, 548, 568, 526, 572, 526, 548,    // Byte 3
      550, 1702, 550, 1706, 550, 1734, 526, 574,
      526, 564, 526, 544, 550, 548, 548, 550,    // Byte 4
      550, 554, 550, 582, 526, 586, 524, 550,
      548, 540, 550, 544, 550, 570, 526, 550,    // Byte 5
      550, 558, 546, 582, 524, 586, 524, 562,
      524, 7978,  // Section Footer
      548, 1690, 548, 568, 524, 1696, 548, 1700,  // Byte 6
      548, 554, 550, 560, 548, 560, 550, 1722,
      526, 1686, 550, 1692, 548, 1696, 550, 576,  // Byte 7
      524, 1704, 548, 558, 550, 562, 548, 550,
      550, 564, 524, 570, 524, 548, 548, 576,     // Byte 8
      524, 578, 526, 558, 550, 586, 524, 574,
      524, 566, 524, 568, 526, 548, 548, 576,     // Byte 9
      526, 556, 548, 558, 550, 560, 550, 550,
      550, 564, 526, 544, 548, 548, 548, 552,     // Byte 10
      548, 556, 548, 582, 526, 564, 548, 550,
      550, 540, 548, 568, 526, 546, 550, 550,     // Byte 11
      550, 580, 524, 558, 550, 560, 550, 1698,
      550, 542, 548, 542, 550, 546, 550, 1722,    // Byte 12
      524, 1706, 548, 582, 526, 586, 524, 574,
      526, 1690, 548, 544, 550, 546, 550, 552,    // Byte 13
      548, 1726, 526, 1730, 524, 1734, 526, 562,
      524, 7976,  // Section footer
      550, 566, 524, 544, 550, 570, 526, 550,     // Byte 14
      548, 554, 550, 1706, 550, 562, 548, 574,
      526, 542, 548, 544, 550, 572, 526, 548,     // Byte 15
      552, 554, 550, 582, 526, 584, 526, 574,
      524, 542, 548, 544, 548, 572, 524, 552,     // Byte 16
      548, 578, 524, 560, 548, 562, 548, 550,
      550, 540, 550, 546, 548, 572, 524, 576,     // Byte 17
      526, 556, 548, 560, 546, 564, 548, 574,
      526, 540, 550, 546, 546, 546, 550, 1698,    // Byte 18
      550, 1728, 524, 1706, 548, 564, 548, 574,
      524, 544, 548, 568, 526, 548, 548, 574,     // Byte 19
      524, 554, 550, 558, 550, 562, 548, 550,
      550, 566, 524, 544, 548, 546, 552, 1698,    // Byte 20
      548, 1702, 550, 560, 546, 586, 524, 538,
      546  // Footer
  };  // KELON 178D000070030683

  irsend.begin();
  irsend.reset();
  irsend.sendRaw(rawData, 343, 38000);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_NE(KELON, irsend.capture.decode_type);  // Not a KELON message
  EXPECT_NE(kKelonBits, irsend.capture.bits);    // Not a 48 bit message.
}

TEST(TestDecodeKelon168, RealExample) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  // Ref: https://github.com/crankyoldgit/IRremoteESP8266/discussions/1744#discussioncomment-2061968
  const uint16_t rawData[343] = {
      8922, 4494,  // Header
      548, 1714, 524, 1718, 524, 572, 524, 550,  // Byte 0
      550, 556, 550, 560, 548, 586, 524, 1724,
      526, 564, 524, 1692, 550, 1696, 550, 552,  // Byte 1
      550, 554, 550, 560, 548, 586, 524, 550,
      548, 1688, 550, 1694, 548, 548, 550, 552,  // Byte 2
      548, 556, 548, 582, 524, 586, 524, 576,
      524, 540, 548, 568, 526, 572, 526, 548,    // Byte 3
      550, 1702, 550, 1706, 550, 1734, 526, 574,
      526, 564, 526, 544, 550, 548, 548, 550,    // Byte 4
      550, 554, 550, 582, 526, 586, 524, 550,
      548, 540, 550, 544, 550, 570, 526, 550,    // Byte 5
      550, 558, 546, 582, 524, 586, 524, 562,
      524, 7978,  // Section Footer
      548, 1690, 548, 568, 524, 1696, 548, 1700,  // Byte 6
      548, 554, 550, 560, 548, 560, 550, 1722,
      526, 1686, 550, 1692, 548, 1696, 550, 576,  // Byte 7
      524, 1704, 548, 558, 550, 562, 548, 550,
      550, 564, 524, 570, 524, 548, 548, 576,     // Byte 8
      524, 578, 526, 558, 550, 586, 524, 574,
      524, 566, 524, 568, 526, 548, 548, 576,     // Byte 9
      526, 556, 548, 558, 550, 560, 550, 550,
      550, 564, 526, 544, 548, 548, 548, 552,     // Byte 10
      548, 556, 548, 582, 526, 564, 548, 550,
      550, 540, 548, 568, 526, 546, 550, 550,     // Byte 11
      550, 580, 524, 558, 550, 560, 550, 1698,
      550, 542, 548, 542, 550, 546, 550, 1722,    // Byte 12
      524, 1706, 548, 582, 526, 586, 524, 574,
      526, 1690, 548, 544, 550, 546, 550, 552,    // Byte 13
      548, 1726, 526, 1730, 524, 1734, 526, 562,
      524, 7976,  // Section footer
      550, 566, 524, 544, 550, 570, 526, 550,     // Byte 14
      548, 554, 550, 1706, 550, 562, 548, 574,
      526, 542, 548, 544, 550, 572, 526, 548,     // Byte 15
      552, 554, 550, 582, 526, 584, 526, 574,
      524, 542, 548, 544, 548, 572, 524, 552,     // Byte 16
      548, 578, 524, 560, 548, 562, 548, 550,
      550, 540, 550, 546, 548, 572, 524, 576,     // Byte 17
      526, 556, 548, 560, 546, 564, 548, 574,
      526, 540, 550, 546, 546, 546, 550, 1698,    // Byte 18
      550, 1728, 524, 1706, 548, 564, 548, 574,
      524, 544, 548, 568, 526, 548, 548, 574,     // Byte 19
      524, 554, 550, 558, 550, 562, 548, 550,
      550, 566, 524, 544, 548, 546, 552, 1698,    // Byte 20
      548, 1702, 550, 560, 546, 586, 524, 538,
      546  // Footer
  };  // KELON 178D000070030683

  const uint8_t expected[kKelon168StateLength] = {
      0x83, 0x06, 0x03, 0x70, 0x00, 0x00, 0x8D,
      0x17, 0x00, 0x00, 0x00, 0x80, 0x18, 0x71,
      0x20, 0x00, 0x00, 0x00, 0x38, 0x00, 0x18};

  irsend.begin();
  irsend.reset();
  irsend.sendRaw(rawData, 343, 38000);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(decode_type_t::KELON168, irsend.capture.decode_type);
  EXPECT_EQ(kKelon168Bits, irsend.capture.bits);
  EXPECT_STATE_EQ(expected, irsend.capture.state, irsend.capture.bits);
  EXPECT_EQ("", IRAcUtils::resultAcToString(&irsend.capture));
  stdAc::state_t r, p;
  ASSERT_FALSE(IRAcUtils::decodeToState(&irsend.capture, &r, &p));
}

TEST(TestDecodeKelon168, SyntheticExample) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);

  const uint8_t expected[kKelon168StateLength] = {
      0x83, 0x06, 0x03, 0x70, 0x00, 0x00, 0x8D,
      0x17, 0x00, 0x00, 0x00, 0x80, 0x18, 0x71,
      0x20, 0x00, 0x00, 0x00, 0x38, 0x00, 0x18};

  irsend.begin();
  irsend.reset();
  irsend.sendKelon168(expected);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(decode_type_t::KELON168, irsend.capture.decode_type);
  EXPECT_EQ(kKelon168Bits, irsend.capture.bits);
  EXPECT_STATE_EQ(expected, irsend.capture.state, irsend.capture.bits);
  EXPECT_EQ("", IRAcUtils::resultAcToString(&irsend.capture));
  stdAc::state_t r, p;
  ASSERT_FALSE(IRAcUtils::decodeToState(&irsend.capture, &r, &p));
}

// Tests for IRKelon168Ac using frames captured from a real Hisense DG11R1-01
// remote (A/C unit TG25VE0AG) with a Flipper Zero. Each frame is a full
// 168 bit message with valid checksums. Names are the capture step ids.
namespace {
// Capture step: power_off
const uint8_t kCapPowerOff[kKelon168StateLength] = {
    0x83, 0x06, 0x04, 0x60, 0x00, 0x00, 0x8A,
    0x2F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC1,
    0x00, 0x01, 0x00, 0x00, 0x28, 0x00, 0x29};
// Capture step: power_on
const uint8_t kCapPowerOn[kKelon168StateLength] = {
    0x83, 0x06, 0x04, 0x60, 0x00, 0x00, 0x8A,
    0x2F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC1,
    0x00, 0x01, 0x00, 0x00, 0x38, 0x00, 0x39};
// Capture step: fan_press_1
const uint8_t kCapFanHigh[kKelon168StateLength] = {
    0x83, 0x06, 0x01, 0x60, 0x00, 0x00, 0x8A,
    0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD9,
    0x00, 0x11, 0x00, 0x00, 0x38, 0x00, 0x29};
// Capture step: fan_press_6
const uint8_t kCapFanAuto[kKelon168StateLength] = {
    0x83, 0x06, 0x00, 0x60, 0x00, 0x00, 0x8A,
    0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD8,
    0x00, 0x11, 0x00, 0x00, 0x38, 0x00, 0x29};
// Capture step: vswing_on
const uint8_t kCapVSwing[kKelon168StateLength] = {
    0x83, 0x06, 0x80, 0x60, 0x00, 0x00, 0x8A,
    0x32, 0x40, 0x00, 0x00, 0x00, 0x00, 0x18,
    0x00, 0x07, 0x00, 0x00, 0x38, 0x00, 0x3F};
// Capture step: hswing_on
const uint8_t kCapHSwing[kKelon168StateLength] = {
    0x83, 0x06, 0x00, 0x60, 0x00, 0x00, 0x8A,
    0x32, 0x80, 0x00, 0x00, 0x00, 0x00, 0x58,
    0x00, 0x08, 0x00, 0x00, 0x38, 0x00, 0x30};
// Capture step: hswing_off (the same bytes as hswing_on: the button toggles)
const uint8_t kCapHSwingOff[kKelon168StateLength] = {
    0x83, 0x06, 0x00, 0x60, 0x00, 0x00, 0x8A,
    0x32, 0x80, 0x00, 0x00, 0x00, 0x00, 0x58,
    0x00, 0x08, 0x00, 0x00, 0x38, 0x00, 0x30};
// Capture step: sleep_on
const uint8_t kCapSleepOn[kKelon168StateLength] = {
    0x83, 0x06, 0x0B, 0x60, 0x00, 0x00, 0x8A,
    0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD3,
    0x00, 0x03, 0x00, 0x00, 0x38, 0x00, 0x3B};
// Capture step: turbo_on
const uint8_t kCapTurboOn[kKelon168StateLength] = {
    0x83, 0x06, 0x00, 0xE0, 0x00, 0x90, 0x8A,
    0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC9,
    0x00, 0x04, 0x00, 0x00, 0x38, 0x00, 0x3C};
// Capture step: turbo_off
const uint8_t kCapTurboOff[kKelon168StateLength] = {
    0x83, 0x06, 0x00, 0x60, 0x00, 0x00, 0x8A,
    0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD9,
    0x00, 0x04, 0x00, 0x00, 0x38, 0x00, 0x3C};
// Capture step: economy_on
const uint8_t kCapEconoOn[kKelon168StateLength] = {
    0x83, 0x06, 0x03, 0x60, 0x00, 0x00, 0x8A,
    0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDA,
    0x20, 0x0C, 0x00, 0x00, 0x38, 0x00, 0x14};
// Capture step: economy_off
const uint8_t kCapEconoOff[kKelon168StateLength] = {
    0x83, 0x06, 0x00, 0x60, 0x00, 0x00, 0x8A,
    0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD9,
    0x00, 0x0C, 0x00, 0x00, 0x38, 0x00, 0x34};
// Capture step: display_1
const uint8_t kCapDisplay[kKelon168StateLength] = {
    0x83, 0x06, 0x00, 0x60, 0x00, 0x00, 0xAA,
    0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF9,
    0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x38};
// Capture step: rebase_modes
const uint8_t kCapTempBase[kKelon168StateLength] = {
    0x83, 0x06, 0x00, 0x60, 0x00, 0x00, 0x8A,
    0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDA,
    0x00, 0x02, 0x00, 0x00, 0x38, 0x00, 0x3A};
// Capture step: temp_up_30
const uint8_t kCapTemp30[kKelon168StateLength] = {
    0x83, 0x06, 0x00, 0xE0, 0x00, 0x00, 0x8A,
    0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5A,
    0x00, 0x02, 0x00, 0x00, 0x38, 0x00, 0x3A};
// Capture step: mode_cool
const uint8_t kCapCool24[kKelon168StateLength] = {
    0x83, 0x06, 0x00, 0x82, 0x00, 0x00, 0x8A,
    0x2F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x27,
    0x00, 0x06, 0x00, 0x00, 0x38, 0x00, 0x3E};
}  // namespace

// Decode a captured frame and describe it.
String describeKelon168(const uint8_t frame[]) {
  IRKelon168Ac ac(kGpioUnused);
  ac.setRaw(frame);
  return ac.toString();
}

// Captured frame -> toString().
TEST(TestKelon168Ac, CapturedToString) {
  EXPECT_EQ(
      "Model: 1 (UNKNOWN), Power Toggle: On, Mode: 0 (Heat), Temp: 22C, "
      "Fan: 0 (Auto), Swing: Off, Swing(H) Toggle: Off, Light: On, "
      "Clock: 10:47, On Timer: Off, Off Timer: Off, Sleep: Off, "
      "Super: Off, Econo: Off, Command: 1 (Power)",
      describeKelon168(kCapPowerOff));
  EXPECT_EQ(
      "Model: 1 (UNKNOWN), Power Toggle: Off, Mode: 2 (Cool), Temp: 24C, "
      "Fan: 0 (Auto), Swing: Off, Swing(H) Toggle: Off, Light: On, "
      "Clock: 10:47, On Timer: Off, Off Timer: Off, Sleep: Off, "
      "Super: Off, Econo: Off, Command: 6 (Mode)",
      describeKelon168(kCapCool24));
  EXPECT_EQ(
      "Model: 1 (UNKNOWN), Power Toggle: Off, Mode: 0 (Heat), Temp: 30C, "
      "Fan: 0 (Auto), Swing: Off, Swing(H) Toggle: Off, Light: On, "
      "Clock: 10:48, On Timer: Off, Off Timer: Off, Sleep: Off, "
      "Super: Off, Econo: Off, Command: 2 (Temp)",
      describeKelon168(kCapTemp30));
  EXPECT_EQ(
      "Model: 1 (UNKNOWN), Power Toggle: Off, Mode: 0 (Heat), Temp: 22C, "
      "Fan: 0 (Auto), Swing: On, Swing(H) Toggle: Off, Light: On, "
      "Clock: 10:50, On Timer: Off, Off Timer: Off, Sleep: Off, "
      "Super: Off, Econo: Off, Command: 7 (Swing)",
      describeKelon168(kCapVSwing));
  EXPECT_EQ(
      "Model: 1 (UNKNOWN), Power Toggle: Off, Mode: 0 (Heat), Temp: 22C, "
      "Fan: 0 (Auto), Swing: Off, Swing(H) Toggle: On, Light: On, "
      "Clock: 10:50, On Timer: Off, Off Timer: Off, Sleep: Off, "
      "Super: Off, Econo: Off, Command: 8 (Swing(H))",
      describeKelon168(kCapHSwing));
  EXPECT_EQ(
      "Model: 1 (UNKNOWN), Power Toggle: Off, Mode: 0 (Heat), Temp: 30C, "
      "Fan: 0 (Auto), Swing: Off, Swing(H) Toggle: Off, Light: On, "
      "Clock: 10:51, On Timer: Off, Off Timer: Off, Sleep: Off, "
      "Super: On, Econo: Off, Command: 4 (Super)",
      describeKelon168(kCapTurboOn));
  EXPECT_EQ(
      "Model: 1 (UNKNOWN), Power Toggle: Off, Mode: 0 (Heat), Temp: 22C, "
      "Fan: 0 (Auto), Swing: Off, Swing(H) Toggle: Off, Light: Off, "
      "Clock: 10:51, On Timer: Off, Off Timer: Off, Sleep: Off, "
      "Super: Off, Econo: Off, Command: 0 (Light)",
      describeKelon168(kCapDisplay));

  // Frames where the fan speed is part of the change.
  EXPECT_NE(std::string::npos,
            describeKelon168(kCapEconoOn).find(
                "Econo: On, Command: 12 (Econo)"));
  EXPECT_NE(std::string::npos,
            describeKelon168(kCapSleepOn).find("Sleep: On, "));
  EXPECT_NE(std::string::npos,
            describeKelon168(kCapFanHigh).find("Command: 17 (Fan)"));
}

// Captured fan and sleep frames -> getters.
TEST(TestKelon168Ac, CapturedFanAndSleep) {
  IRKelon168Ac ac(kGpioUnused);
  ac.setRaw(kCapFanHigh);
  EXPECT_EQ(kKelon168FanMax, ac.getFan());
  ac.setRaw(kCapFanAuto);
  EXPECT_EQ(kKelon168FanAuto, ac.getFan());
  ac.setRaw(kCapSleepOn);
  EXPECT_TRUE(ac.getSleep());
  EXPECT_EQ(kKelon168FanMin, ac.getFan());
  ac.setRaw(kCapPowerOn);
  EXPECT_FALSE(ac.getSleep());
}

// Captured frame -> getters and toCommon().
TEST(TestKelon168Ac, CapturedToCommon) {
  IRKelon168Ac ac(kGpioUnused);
  ac.setRaw(kCapHSwing);
  EXPECT_TRUE(ac.getSwingHToggle());
  EXPECT_FALSE(ac.getEcono());
  EXPECT_EQ(kKelon168CommandSwingH, ac.getCommand());
  stdAc::state_t s = ac.toCommon();
  EXPECT_EQ(stdAc::swingh_t::kOff, s.swingh);
  EXPECT_FALSE(s.econo);

  ac.setRaw(kCapEconoOn);
  EXPECT_FALSE(ac.getSwingHToggle());
  EXPECT_TRUE(ac.getEcono());
  EXPECT_EQ(kKelon168CommandEcono, ac.getCommand());
  s = ac.toCommon();
  EXPECT_EQ(stdAc::swingh_t::kOff, s.swingh);
  EXPECT_TRUE(s.econo);

  ac.setRaw(kCapEconoOff);
  EXPECT_FALSE(ac.getEcono());
  EXPECT_FALSE(ac.toCommon().econo);
}

// Setters on a captured base frame -> the next captured frame, with checksums.
TEST(TestKelon168Ac, SetHorizontalSwingToggleMatchesCapture) {
  IRKelon168Ac ac(kGpioUnused);
  ac.setRaw(kCapFanAuto);
  ac.setSwingHToggle(true);
  EXPECT_STATE_EQ(kCapHSwing, ac.getRaw(), kKelon168Bits);
  EXPECT_TRUE(IRKelon168Ac::validChecksum(ac.getRaw()));
  ac.setRaw(kCapFanAuto);
  ac.setSwingHToggle(true);
  EXPECT_STATE_EQ(kCapHSwingOff, ac.getRaw(), kKelon168Bits);
}

TEST(TestKelon168Ac, HorizontalSwingOnAndOffCapturesAreToggles) {
  IRKelon168Ac ac(kGpioUnused);
  EXPECT_STATE_EQ(kCapHSwing, kCapHSwingOff, kKelon168Bits);
  ac.setRaw(kCapHSwing);
  EXPECT_TRUE(ac.getSwingHToggle());
  EXPECT_EQ(kKelon168CommandSwingH, ac.getCommand());
  ac.setRaw(kCapHSwingOff);
  EXPECT_TRUE(ac.getSwingHToggle());
  EXPECT_EQ(kKelon168CommandSwingH, ac.getCommand());
  EXPECT_EQ(stdAc::swingh_t::kOff, ac.toCommon().swingh);
  ac.setSwingHToggle(false);
  EXPECT_FALSE(ac.getSwingHToggle());
}

TEST(TestKelon168Ac, SetEconomyMatchesCapture) {
  IRKelon168Ac ac(kGpioUnused);
  ac.setRaw(kCapTurboOff);
  // The remote lowers the fan speed when economy is switched on.
  ac.setFan(kKelon168FanMin);
  ac.setEcono(true);
  EXPECT_STATE_EQ(kCapEconoOn, ac.getRaw(), kKelon168Bits);
  EXPECT_TRUE(IRKelon168Ac::validChecksum(ac.getRaw()));
  ac.setFan(kKelon168FanAuto);
  ac.setEcono(false);
  EXPECT_STATE_EQ(kCapEconoOff, ac.getRaw(), kKelon168Bits);
}

TEST(TestKelon168Ac, SetVerticalSwingMatchesCapture) {
  IRKelon168Ac ac(kGpioUnused);
  ac.setRaw(kCapFanAuto);
  ac.setSwing(true);
  EXPECT_STATE_EQ(kCapVSwing, ac.getRaw(), kKelon168Bits);
}

TEST(TestKelon168Ac, SetPowerMatchesCapture) {
  IRKelon168Ac ac(kGpioUnused);
  ac.setRaw(kCapPowerOn);
  ac.setPower(false);
  EXPECT_STATE_EQ(kCapPowerOff, ac.getRaw(), kKelon168Bits);
}

TEST(TestKelon168Ac, SetTempMatchesCapture) {
  IRKelon168Ac ac(kGpioUnused);
  ac.setRaw(kCapTempBase);
  ac.setTemp(30);
  EXPECT_STATE_EQ(kCapTemp30, ac.getRaw(), kKelon168Bits);
}

TEST(TestKelon168Ac, SetLightCommandMatchesCapture) {
  IRKelon168Ac ac(kGpioUnused);
  ac.setRaw(kCapEconoOff);
  ac.setLight(false);
  ac.setCommand(kKelon168CommandLight);
  EXPECT_STATE_EQ(kCapDisplay, ac.getRaw(), kKelon168Bits);
}

TEST(TestKelon168Ac, HorizontalSwingToggleResetsAfterSend) {
  IRKelon168Ac ac(kGpioUnused);
  ac.setSwingHToggle(true);
  EXPECT_TRUE(ac.getSwingHToggle());
  ac.send();
  EXPECT_FALSE(ac.getSwingHToggle());
}
