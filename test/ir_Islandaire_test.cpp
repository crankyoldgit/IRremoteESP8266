// Copyright 2026

#include "ir_Islandaire.h"
#include "IRac.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "IRutils.h"
#include "gtest/gtest.h"

// General housekeeping
TEST(TestIslandaireAc, Housekeeping) {
  ASSERT_EQ("ISLANDAIRE_AC", typeToString(decode_type_t::ISLANDAIRE_AC));
  ASSERT_EQ(decode_type_t::ISLANDAIRE_AC, strToDecodeType("ISLANDAIRE_AC"));
  ASSERT_TRUE(hasACState(decode_type_t::ISLANDAIRE_AC));
  ASSERT_TRUE(IRac::isProtocolSupported(decode_type_t::ISLANDAIRE_AC));
  ASSERT_EQ(kIslandaireBits, IRsend::defaultBits(decode_type_t::ISLANDAIRE_AC));
  ASSERT_EQ(kNoRepeat, IRsend::minRepeats(decode_type_t::ISLANDAIRE_AC));
}

// Tests for sendIslandaireAc().

TEST(TestSendIslandaireAc, SendDataOnly) {
  IRsendTest irsend(kGpioUnused);
  irsend.begin();
  // Heat, 72F, Fan Auto, Timer Off
  const uint8_t state[kIslandaireStateLength] = {
      0x22, 0x22, 0x22, 0x22, 0x22, 0x24, 0x01, 0x48, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x17};
  irsend.reset();
  irsend.sendIslandaireAc(state);
  EXPECT_EQ(
      "f38000d50m3608s1520m560s408m560s1208m560s408m560s408m560s408m560s1208"
      "m560s408m560s408m560s408m560s1208m560s408m560s408m560s408m560s1208m560"
      "s408m560s408m560s408m560s1208m560s408m560s408m560s408m560s1208m560s408"
      "m560s408m560s408m560s1208m560s408m560s408m560s408m560s1208m560s408m560"
      "s408m560s408m560s1208m560s408m560s408m560s408m560s1208m560s408m560s408"
      "m560s408m560s408m560s1208m560s408m560s408m560s1208m560s408m560s408m560"
      "s1208m560s408m560s408m560s408m560s408m560s408m560s408m560s408m560s408"
      "m560s408m560s408m560s1208m560s408m560s408m560s1208m560s408m560s408m560"
      "s408m560s408m560s408m560s408m560s408m560s408m560s408m560s408m560s408"
      "m560s408m560s408m560s408m560s408m560s408m560s408m560s408m560s408m560"
      "s408m560s408m560s408m560s408m560s408m560s408m560s408m560s408m560s408"
      "m560s408m560s408m560s408m560s408m560s408m560s408m560s408m560s408m560"
      "s408m560s408m560s408m560s408m560s408m560s1208m560s1208m560s1208m560"
      "s408m560s1208m560s408m560s408m560s408m560s100000",
      irsend.outputStr());
}

// Tests for decodeIslandaireAc().

/// Decode real captures from the remote. These frames also match TEKNOPOINT,
/// so this checks that decode() tries Islandaire first.
TEST(TestDecodeIslandaireAc, RealExampleHeat72F) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();
  // Heat, 72F, Fan Auto, Timer Off
  const uint16_t rawData[227] = {
      3588, 1505, 600, 368, 599, 1168, 602, 367, 600, 369, 597, 372, 599,
      1168, 599, 370, 599, 390, 579, 389, 578, 1169, 598, 393, 578, 391, 577,
      393, 578, 1189, 578, 391, 577, 393, 575, 393, 574, 1193, 574, 396, 607,
      363, 607, 362, 607, 1158, 553, 419, 612, 356, 611, 358, 587, 1155, 636,
      358, 590, 379, 616, 352, 592, 1149, 643, 352, 617, 351, 618, 350, 617,
      1125, 643, 328, 602, 368, 600, 368, 603, 1163, 605, 366, 629, 339, 628,
      340, 628, 342, 605, 1163, 603, 365, 625, 365, 581, 1186, 581, 388, 579,
      391, 579, 1188, 579, 391, 565, 404, 564, 404, 564, 406, 564, 404, 563,
      405, 564, 406, 563, 406, 563, 404, 563, 406, 564, 1203, 564, 404, 563,
      406, 562, 1205, 565, 405, 563, 405, 564, 404, 563, 407, 563, 405, 564,
      405, 563, 406, 562, 406, 564, 404, 564, 406, 563, 405, 563, 405, 563,
      407, 563, 404, 563, 405, 563, 406, 563, 405, 563, 407, 563, 405, 563,
      405, 563, 407, 563, 405, 561, 407, 562, 407, 563, 405, 562, 406, 562,
      408, 562, 406, 563, 405, 562, 407, 562, 406, 562, 406, 562, 407, 563,
      406, 562, 407, 563, 406, 562, 406, 563, 407, 561, 407, 562, 406, 562,
      407, 543, 1224, 540, 1228, 538, 1229, 539, 429, 539, 1229, 539, 431,
      538, 430, 539, 430, 539};
  const uint8_t expectedState[kIslandaireStateLength] = {
      0x22, 0x22, 0x22, 0x22, 0x22, 0x24, 0x01, 0x48, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x17};
  irsend.reset();
  irsend.sendRaw(rawData, 227, 38);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(decode_type_t::ISLANDAIRE_AC, irsend.capture.decode_type);
  EXPECT_EQ(kIslandaireBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
  EXPECT_EQ(
      "Power: On, Mode: 1 (Heat), Temp: 72F, Fan: 0 (Auto), Off Timer: Off",
      IRAcUtils::resultAcToString(&irsend.capture));
  stdAc::state_t r, p;
  ASSERT_TRUE(IRAcUtils::decodeToState(&irsend.capture, &r, &p));
  EXPECT_EQ(decode_type_t::ISLANDAIRE_AC, r.protocol);
  EXPECT_TRUE(r.power);
  EXPECT_EQ(stdAc::opmode_t::kHeat, r.mode);
  EXPECT_FALSE(r.celsius);
  EXPECT_EQ(72, r.degrees);
  EXPECT_EQ(stdAc::fanspeed_t::kAuto, r.fanspeed);
}

TEST(TestDecodeIslandaireAc, RealExampleHeat86F) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();
  // Heat, 86F, Fan Auto, Timer Off
  const uint16_t rawData[227] = {
      3626, 1470, 678, 326, 642, 1090, 679, 323, 641, 325, 646, 322, 649,
      1088, 679, 319, 648, 321, 650, 317, 646, 1092, 678, 320, 647, 321, 645,
      324, 647, 1092, 675, 321, 646, 323, 620, 347, 620, 1120, 650, 346, 618,
      349, 618, 351, 617, 1123, 647, 348, 617, 351, 617, 351, 621, 1120, 640,
      354, 623, 346, 623, 345, 623, 1117, 611, 385, 621, 346, 622, 347, 620,
      1122, 637, 356, 619, 351, 611, 356, 610, 1131, 637, 359, 611, 356, 609,
      359, 607, 362, 609, 1133, 633, 360, 608, 362, 606, 1136, 608, 385, 605,
      364, 582, 1160, 606, 388, 605, 363, 605, 363, 582, 388, 580, 387, 581,
      387, 580, 389, 581, 387, 581, 1163, 603, 1165, 602, 389, 579, 1187, 581,
      389, 579, 1187, 581, 390, 580, 388, 580, 388, 578, 391, 576, 392, 577,
      392, 575, 394, 577, 391, 577, 392, 578, 391, 579, 389, 579, 390, 577,
      392, 576, 392, 576, 392, 575, 395, 574, 394, 576, 394, 577, 391, 577,
      391, 577, 392, 576, 393, 573, 395, 572, 398, 573, 395, 598, 370, 575,
      395, 595, 372, 562, 406, 561, 408, 561, 407, 561, 408, 559, 409, 562,
      407, 562, 408, 561, 408, 561, 408, 560, 409, 560, 408, 560, 407, 562,
      407, 561, 1204, 562, 408, 561, 1205, 562, 408, 560, 410, 560, 1205, 562,
      409, 559, 410, 559};
  const uint8_t expectedState[kIslandaireStateLength] = {
      0x22, 0x22, 0x22, 0x22, 0x22, 0x24, 0x01, 0x56, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x25};
  irsend.reset();
  irsend.sendRaw(rawData, 227, 38);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(decode_type_t::ISLANDAIRE_AC, irsend.capture.decode_type);
  EXPECT_EQ(kIslandaireBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
  EXPECT_EQ(
      "Power: On, Mode: 1 (Heat), Temp: 86F, Fan: 0 (Auto), Off Timer: Off",
      IRAcUtils::resultAcToString(&irsend.capture));
}

TEST(TestDecodeIslandaireAc, RealExampleTimer1h) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();
  // Heat, 74F, Fan Auto, Timer 1h
  const uint16_t rawData[227] = {
      3663, 1432, 673, 296, 578, 1189, 672, 296, 577, 393, 673, 295, 578,
      1190, 671, 299, 632, 336, 578, 392, 672, 1096, 670, 298, 647, 323, 663,
      306, 672, 1096, 665, 304, 645, 322, 645, 325, 645, 1123, 641, 327, 576,
      392, 645, 324, 644, 1124, 643, 325, 642, 328, 642, 329, 640, 1128, 640,
      327, 638, 334, 636, 329, 640, 1127, 637, 331, 638, 332, 636, 330, 638,
      1131, 598, 371, 599, 368, 635, 336, 598, 1170, 599, 370, 628, 342, 600,
      368, 601, 370, 600, 1165, 600, 1168, 600, 371, 597, 1168, 599, 370, 599,
      371, 598, 1167, 600, 371, 599, 371, 598, 371, 598, 370, 598, 371, 597,
      371, 638, 330, 642, 328, 670, 1095, 672, 300, 670, 1095, 672, 303, 663,
      330, 620, 1118, 670, 327, 619, 350, 619, 351, 620, 349, 618, 350, 620,
      350, 619, 349, 618, 1142, 648, 328, 640, 328, 618, 352, 617, 351, 641,
      327, 617, 352, 637, 331, 620, 349, 619, 350, 641, 1127, 638, 330, 616,
      353, 640, 330, 615, 351, 615, 354, 614, 354, 613, 356, 589, 379, 611,
      358, 612, 357, 618, 350, 624, 344, 623, 347, 650, 318, 650, 318, 650,
      319, 650, 319, 649, 319, 649, 321, 648, 320, 647, 321, 647, 323, 646,
      322, 645, 325, 643, 1094, 663, 334, 620, 349, 642, 324, 618, 1124, 641,
      1128, 633, 358, 609};
  const uint8_t expectedState[kIslandaireStateLength] = {
      0x22, 0x22, 0x22, 0x22, 0x22, 0x2C, 0x01, 0x4A, 0x40, 0x00, 0x01, 0x00,
      0x00, 0x62};
  irsend.reset();
  irsend.sendRaw(rawData, 227, 38);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(decode_type_t::ISLANDAIRE_AC, irsend.capture.decode_type);
  EXPECT_EQ(kIslandaireBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
  EXPECT_EQ(
      "Power: On, Mode: 1 (Heat), Temp: 74F, Fan: 0 (Auto), "
      "Off Timer: 01:00",
      IRAcUtils::resultAcToString(&irsend.capture));

  // The library builds the same message.
  IRIslandaireAc ac(kGpioUnused);
  ac.setMode(kIslandaireHeat);
  ac.setTemp(74, true);
  ac.setTimer(1);
  EXPECT_STATE_EQ(expectedState, ac.getRaw(), kIslandaireBits);
}

/// Our own messages must decode as Islandaire, not TEKNOPOINT.
TEST(TestDecodeIslandaireAc, SyntheticSelfDecode) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  IRIslandaireAc ac(kGpioUnused);
  ac.setMode(kIslandaireFan);
  ac.setFan(kIslandaireFanHigh);
  ac.setTemp(65, true);
  ac.setTimer(5);
  irsend.begin();
  irsend.reset();
  irsend.sendIslandaireAc(ac.getRaw());
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(decode_type_t::ISLANDAIRE_AC, irsend.capture.decode_type);
  EXPECT_STATE_EQ(ac.getRaw(), irsend.capture.state, irsend.capture.bits);
  EXPECT_EQ(
      "Power: On, Mode: 4 (Fan), Temp: 65F, Fan: 4 (High), "
      "Off Timer: 05:00",
      IRAcUtils::resultAcToString(&irsend.capture));
}

/// A message without the Islandaire preamble must not decode as Islandaire.
TEST(TestDecodeIslandaireAc, RejectOtherPreamble) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  // A TCL112/Teknopoint style message with a valid checksum.
  const uint8_t state[kIslandaireStateLength] = {
      0x23, 0xCB, 0x26, 0x01, 0x00, 0x24, 0x03, 0x0F, 0x08, 0x00, 0x00, 0x00,
      0x00, 0x53};
  irsend.begin();
  irsend.reset();
  irsend.sendIslandaireAc(state);
  irsend.makeDecodeResult();
  EXPECT_FALSE(irrecv.decodeIslandaireAc(&irsend.capture));
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_NE(decode_type_t::ISLANDAIRE_AC, irsend.capture.decode_type);
}

// Tests for the IRIslandaireAc class.

TEST(TestIslandaireAcClass, Checksum) {
  uint8_t state[kIslandaireStateLength] = {
      0x22, 0x22, 0x22, 0x22, 0x22, 0x24, 0x03, 0x46, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x17};
  EXPECT_TRUE(IRIslandaireAc::validChecksum(state));
  EXPECT_EQ(0x17, IRIslandaireAc::calcChecksum(state));
  state[13]++;
  EXPECT_FALSE(IRIslandaireAc::validChecksum(state));
}

TEST(TestIslandaireAcClass, Power) {
  IRIslandaireAc ac(kGpioUnused);
  ac.on();
  EXPECT_TRUE(ac.getPower());
  EXPECT_EQ(0x24, ac.getRaw()[5]);
  ac.off();
  EXPECT_FALSE(ac.getPower());
  EXPECT_EQ(0x20, ac.getRaw()[5]);
  ac.setPower(true);
  EXPECT_TRUE(ac.getPower());
}

TEST(TestIslandaireAcClass, Mode) {
  IRIslandaireAc ac(kGpioUnused);
  ac.setMode(kIslandaireHeat);
  EXPECT_EQ(kIslandaireHeat, ac.getMode());
  EXPECT_EQ(0x01, ac.getRaw()[6]);
  ac.setMode(kIslandaireFan);
  EXPECT_EQ(kIslandaireFan, ac.getMode());
  EXPECT_EQ(0x04, ac.getRaw()[6]);
  ac.setMode(kIslandaireCool);
  EXPECT_EQ(kIslandaireCool, ac.getMode());
  EXPECT_EQ(0x03, ac.getRaw()[6]);
  ac.setMode(kIslandaireHeat);
  ac.setMode(0x0F);  // Unknown, so Cool.
  EXPECT_EQ(kIslandaireCool, ac.getMode());
}

TEST(TestIslandaireAcClass, Temperature) {
  IRIslandaireAc ac(kGpioUnused);
  ac.setTemp(70, true);
  EXPECT_EQ(70, ac.getTemp(true));
  EXPECT_EQ(70, ac.getRaw()[7]);
  EXPECT_NEAR(21.1, ac.getTemp(), 0.1);
  ac.setTemp(kIslandaireMinTempF, true);
  EXPECT_EQ(kIslandaireMinTempF, ac.getTemp(true));
  ac.setTemp(kIslandaireMaxTempF, true);
  EXPECT_EQ(kIslandaireMaxTempF, ac.getTemp(true));
  ac.setTemp(50, true);
  EXPECT_EQ(kIslandaireMinTempF, ac.getTemp(true));
  ac.setTemp(100, true);
  EXPECT_EQ(kIslandaireMaxTempF, ac.getTemp(true));
  // Celsius is converted to the nearest Fahrenheit.
  ac.setTemp(22);
  EXPECT_EQ(72, ac.getTemp(true));
  ac.setTemp(-5);
  EXPECT_EQ(kIslandaireMinTempF, ac.getTemp(true));
  ac.setTemp(40);
  EXPECT_EQ(kIslandaireMaxTempF, ac.getTemp(true));
}

TEST(TestIslandaireAcClass, Fan) {
  IRIslandaireAc ac(kGpioUnused);
  ac.setFan(kIslandaireFanLow);
  EXPECT_EQ(kIslandaireFanLow, ac.getFan());
  EXPECT_EQ(0x02, ac.getRaw()[8]);
  ac.setFan(kIslandaireFanHigh);
  EXPECT_EQ(kIslandaireFanHigh, ac.getFan());
  EXPECT_EQ(0x04, ac.getRaw()[8]);
  ac.setFan(kIslandaireFanAuto);
  EXPECT_EQ(kIslandaireFanAuto, ac.getFan());
  EXPECT_EQ(0x00, ac.getRaw()[8]);
  ac.setFan(kIslandaireFanHigh);
  ac.setFan(0x07);  // Unknown, so Auto.
  EXPECT_EQ(kIslandaireFanAuto, ac.getFan());
}

TEST(TestIslandaireAcClass, Timer) {
  IRIslandaireAc ac(kGpioUnused);
  EXPECT_EQ(0, ac.getTimer());
  ac.setTimer(3);
  EXPECT_EQ(3, ac.getTimer());
  EXPECT_EQ(0x2C, ac.getRaw()[5]);
  EXPECT_EQ(0x40, ac.getRaw()[8]);
  EXPECT_EQ(0x03, ac.getRaw()[10]);
  ac.setTimer(24);  // Capped at 12 hours.
  EXPECT_EQ(12, ac.getTimer());
  // Changing the fan or power keeps the timer flags.
  ac.setFan(kIslandaireFanLow);
  EXPECT_EQ(0x42, ac.getRaw()[8]);
  ac.off();
  EXPECT_EQ(0x28, ac.getRaw()[5]);
  ac.on();
  ac.setTimer(0);
  EXPECT_EQ(0, ac.getTimer());
  EXPECT_EQ(0x24, ac.getRaw()[5]);
  EXPECT_EQ(0x02, ac.getRaw()[8]);
  EXPECT_EQ(0x00, ac.getRaw()[10]);
}

/// Known states, rebuilt from the class's setters.
TEST(TestIslandaireAcClass, KnownStates) {
  IRIslandaireAc ac(kGpioUnused);

  const uint8_t heat61[kIslandaireStateLength] = {
      0x22, 0x22, 0x22, 0x22, 0x22, 0x24, 0x01, 0x3D, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x0C};
  ac.stateReset();
  ac.setMode(kIslandaireHeat);
  ac.setTemp(61, true);
  EXPECT_STATE_EQ(heat61, ac.getRaw(), kIslandaireBits);

  const uint8_t heat88[kIslandaireStateLength] = {
      0x22, 0x22, 0x22, 0x22, 0x22, 0x24, 0x01, 0x58, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x27};
  ac.stateReset();
  ac.setMode(kIslandaireHeat);
  ac.setTemp(88, true);
  EXPECT_STATE_EQ(heat88, ac.getRaw(), kIslandaireBits);

  const uint8_t timer3h[kIslandaireStateLength] = {
      0x22, 0x22, 0x22, 0x22, 0x22, 0x2C, 0x01, 0x4A, 0x40, 0x00, 0x03, 0x00,
      0x00, 0x64};
  ac.stateReset();
  ac.setMode(kIslandaireHeat);
  ac.setTemp(74, true);
  ac.setTimer(3);
  EXPECT_STATE_EQ(timer3h, ac.getRaw(), kIslandaireBits);

  const uint8_t timer12h[kIslandaireStateLength] = {
      0x22, 0x22, 0x22, 0x22, 0x22, 0x2C, 0x01, 0x4A, 0x40, 0x00, 0x0C, 0x00,
      0x00, 0x6D};
  ac.setTimer(12);
  EXPECT_STATE_EQ(timer12h, ac.getRaw(), kIslandaireBits);

  const uint8_t cool70low[kIslandaireStateLength] = {
      0x22, 0x22, 0x22, 0x22, 0x22, 0x24, 0x03, 0x46, 0x02, 0x00, 0x00, 0x00,
      0x00, 0x19};
  ac.stateReset();
  ac.setTemp(70, true);
  ac.setFan(kIslandaireFanLow);
  EXPECT_STATE_EQ(cool70low, ac.getRaw(), kIslandaireBits);

  const uint8_t offCool63high[kIslandaireStateLength] = {
      0x22, 0x22, 0x22, 0x22, 0x22, 0x20, 0x03, 0x3F, 0x04, 0x00, 0x00, 0x00,
      0x00, 0x10};
  ac.stateReset();
  ac.off();
  ac.setTemp(63, true);
  ac.setFan(kIslandaireFanHigh);
  EXPECT_STATE_EQ(offCool63high, ac.getRaw(), kIslandaireBits);
}

TEST(TestIslandaireAcClass, ConvertMode) {
  EXPECT_EQ(kIslandaireCool,
            IRIslandaireAc::convertMode(stdAc::opmode_t::kCool));
  EXPECT_EQ(kIslandaireHeat,
            IRIslandaireAc::convertMode(stdAc::opmode_t::kHeat));
  EXPECT_EQ(kIslandaireFan, IRIslandaireAc::convertMode(stdAc::opmode_t::kFan));
  EXPECT_EQ(kIslandaireCool,
            IRIslandaireAc::convertMode(stdAc::opmode_t::kAuto));
  EXPECT_EQ(stdAc::opmode_t::kHeat,
            IRIslandaireAc::toCommonMode(kIslandaireHeat));
  EXPECT_EQ(stdAc::opmode_t::kFan,
            IRIslandaireAc::toCommonMode(kIslandaireFan));
}

TEST(TestIslandaireAcClass, ConvertFan) {
  EXPECT_EQ(kIslandaireFanAuto,
            IRIslandaireAc::convertFan(stdAc::fanspeed_t::kAuto));
  EXPECT_EQ(kIslandaireFanLow,
            IRIslandaireAc::convertFan(stdAc::fanspeed_t::kMin));
  EXPECT_EQ(kIslandaireFanLow,
            IRIslandaireAc::convertFan(stdAc::fanspeed_t::kLow));
  EXPECT_EQ(kIslandaireFanHigh,
            IRIslandaireAc::convertFan(stdAc::fanspeed_t::kMedium));
  EXPECT_EQ(kIslandaireFanHigh,
            IRIslandaireAc::convertFan(stdAc::fanspeed_t::kMax));
  EXPECT_EQ(stdAc::fanspeed_t::kLow,
            IRIslandaireAc::toCommonFanSpeed(kIslandaireFanLow));
  EXPECT_EQ(stdAc::fanspeed_t::kHigh,
            IRIslandaireAc::toCommonFanSpeed(kIslandaireFanHigh));
}

TEST(TestIslandaireAcClass, HumanReadable) {
  IRIslandaireAc ac(kGpioUnused);
  EXPECT_EQ(
      "Power: On, Mode: 3 (Cool), Temp: 72F, Fan: 0 (Auto), Off Timer: Off",
      ac.toString());
  ac.off();
  ac.setMode(kIslandaireFan);
  ac.setFan(kIslandaireFanLow);
  ac.setTimer(12);
  EXPECT_EQ(
      "Power: Off, Mode: 4 (Fan), Temp: 72F, Fan: 2 (Low), "
      "Off Timer: 12:00",
      ac.toString());
}

// Tests for IRac.

TEST(TestIslandaireAcClass, IRac) {
  IRac irac(kGpioUnused);
  IRIslandaireAc ac(kGpioUnused);
  // Fahrenheit is used as-is.
  irac.islandaire(&ac, true, stdAc::opmode_t::kHeat, false, 75,
                  stdAc::fanspeed_t::kHigh);
  EXPECT_EQ(
      "Power: On, Mode: 1 (Heat), Temp: 75F, Fan: 4 (High), Off Timer: Off",
      ac.toString());
  // Celsius is converted.
  irac.islandaire(&ac, false, stdAc::opmode_t::kCool, true, 22,
                  stdAc::fanspeed_t::kLow);
  EXPECT_EQ(
      "Power: Off, Mode: 3 (Cool), Temp: 72F, Fan: 2 (Low), Off Timer: Off",
      ac.toString());
}
