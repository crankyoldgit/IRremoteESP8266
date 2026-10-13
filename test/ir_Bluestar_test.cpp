// Copyright 2026 IRremoteESP8266 contributors

#include <cstring>
#include <string>
#include "ir_Bluestar.h"
#include "IRac.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend_test.h"
#include "IRutils.h"
#include "gtest/gtest.h"
#include "ir_Bluestar_test_data.h"

using bluestar_test::BluestarCapture;
using bluestar_test::kBluestarCaptures;

namespace {
const uint8_t kCool16[kBluestarStateLength] = {
    0x33, 0x88, 0x82, 0x10, 0x19, 0x99, 0x23, 0xA4, 0x05, 0x34, 0x05, 0xC4};
const uint8_t kCool24[kBluestarStateLength] = {
    0x33, 0x88, 0x82, 0x18, 0x06, 0x86, 0x36, 0xA4, 0x05, 0x3F, 0x05, 0xC4};
}  // namespace

TEST(TestDecodeBluestar, RealCaptures) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  for (const BluestarCapture &frame : kBluestarCaptures) {
    SCOPED_TRACE(frame.label);
    irsend.reset();
    irsend.sendRaw(frame.raw, 185, 38);
    irsend.makeDecodeResult();
    ASSERT_TRUE(irrecv.decode(&irsend.capture));
    ASSERT_EQ(BLUESTAR, irsend.capture.decode_type);
    ASSERT_EQ(91, irsend.capture.bits);
    EXPECT_STATE_EQ(frame.state, irsend.capture.state, kBluestarBits);
    EXPECT_TRUE(IRBluestarAc::validChecksum(irsend.capture.state));
    EXPECT_FALSE(IRAcUtils::resultAcToString(&irsend.capture).empty());
    stdAc::state_t common;
    EXPECT_TRUE(IRAcUtils::decodeToState(&irsend.capture, &common));
    EXPECT_EQ(BLUESTAR, common.protocol);
  }
}

TEST(TestSendBluestar, CapturedStateRoundTrips) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  for (const BluestarCapture &frame : kBluestarCaptures) {
    SCOPED_TRACE(frame.label);
    irsend.reset();
    irsend.sendBluestar(frame.state);
    // 185 captured durations plus a trailing inter-message silence.
    EXPECT_EQ(185, irsend.last);
    EXPECT_EQ(38000, irsend.freq[0]);
    EXPECT_EQ(8950, irsend.output[161]);
    EXPECT_EQ(660, irsend.output[184]);
    irsend.makeDecodeResult();
    ASSERT_TRUE(irrecv.decode(&irsend.capture));
    ASSERT_EQ(BLUESTAR, irsend.capture.decode_type);
    EXPECT_STATE_EQ(frame.state, irsend.capture.state, kBluestarBits);
  }
}

TEST(TestDecodeBluestar, ChecksumSignatureLengthAndSecondary) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  uint8_t state[kBluestarStateLength];
  std::memcpy(state, kCool24, sizeof(state));
  state[9] ^= 1;
  irsend.sendBluestar(state);
  irsend.makeDecodeResult();
  EXPECT_FALSE(irrecv.decodeBluestar(&irsend.capture));
  EXPECT_TRUE(irrecv.decodeBluestar(&irsend.capture, kStartOffset,
                                    kBluestarBits, false));
  EXPECT_FALSE(irrecv.decodeBluestar(&irsend.capture, kStartOffset, 80, false));
  irsend.reset();
  state[0] = 0x34;
  state[9] = IRBluestarAc::calcChecksum(state);
  irsend.sendBluestar(state);
  irsend.makeDecodeResult();
  EXPECT_FALSE(irrecv.decodeBluestar(&irsend.capture));
  EXPECT_TRUE(irrecv.decodeBluestar(&irsend.capture, kStartOffset,
                                    kBluestarBits, false));
  irsend.reset();
  irsend.sendBluestar(kCool24);
  irsend.makeDecodeResult();
  irsend.capture.rawlen = 163;  // Entire main frame, secondary missing.
  EXPECT_FALSE(irrecv.decodeBluestar(&irsend.capture));
  irsend.makeDecodeResult();
  irsend.capture.rawbuf[162] = 4000 / kRawTick;  // Wrong inter-section gap.
  EXPECT_FALSE(irrecv.decodeBluestar(&irsend.capture));
}

TEST(TestSendBluestar, CompleteRepeatsAndInvalidLengths) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.sendBluestar(kCool24, kBluestarStateLength, 1);
  EXPECT_EQ(371, irsend.last);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeBluestar(&irsend.capture));
  EXPECT_STATE_EQ(kCool24, irsend.capture.state, kBluestarBits);
  irsend.makeDecodeResult(186);
  ASSERT_TRUE(irrecv.decodeBluestar(&irsend.capture));
  EXPECT_STATE_EQ(kCool24, irsend.capture.state, kBluestarBits);
  irsend.reset();
  irsend.sendBluestar(kCool24, 10);
  EXPECT_EQ("", irsend.outputStr());
  irsend.sendBluestar(nullptr);
  EXPECT_EQ("", irsend.outputStr());
  uint8_t state[kBluestarStateLength];
  std::memcpy(state, kCool24, sizeof(state));
  state[10] = 0x80;  // Invalid command storage must not be silently truncated.
  irsend.sendBluestar(state);
  EXPECT_EQ("", irsend.outputStr());
}

TEST(TestBluestar, HousekeepingAndStateSerialization) {
  EXPECT_EQ("BLUESTAR", typeToString(BLUESTAR));
  EXPECT_EQ(BLUESTAR, strToDecodeType("BLUESTAR"));
  EXPECT_TRUE(hasACState(BLUESTAR));
  EXPECT_TRUE(IRac::isProtocolSupported(BLUESTAR));
  EXPECT_EQ(91, IRsend::defaultBits(BLUESTAR));
  EXPECT_EQ(kNoRepeat, IRsend::minRepeats(BLUESTAR));
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  ASSERT_TRUE(irsend.send(BLUESTAR, kCool24, kBluestarStateLength));
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ("0x33888218068636A4053F05C4",
            resultToHexidecimal(&irsend.capture));
  EXPECT_NE(std::string::npos, resultToSourceCode(&irsend.capture).find(
      "uint8_t state[12] = {0x33, 0x88, 0x82, 0x18, 0x06, 0x86, 0x36, "
      "0xA4, 0x05, 0x3F, 0x05, 0xC4};"));
}

TEST(TestBluestarClass, ChecksumAndRawStorage) {
  IRBluestarAc ac(kGpioUnused);
  EXPECT_FALSE(ac.getPower());
  EXPECT_TRUE(IRBluestarAc::validChecksum(ac.getRaw()));
  EXPECT_TRUE(ac.setRaw(kCool24));
  EXPECT_STATE_EQ(kCool24, ac.getRaw(), kBluestarBits);
  EXPECT_EQ(0x3F, IRBluestarAc::calcChecksum(kCool24));
  EXPECT_FALSE(IRBluestarAc::validChecksum(kCool24, 9));
  EXPECT_FALSE(IRBluestarAc::validChecksum(nullptr));
  EXPECT_FALSE(ac.setRaw(nullptr));
  EXPECT_FALSE(ac.setRaw(kCool24, 9));
  EXPECT_FALSE(ac.setRaw(kCool24, 11));
  ac.setCommand(0x7FF);
  ASSERT_TRUE(ac.setRaw(kCool16, kBluestarMainStateLength));
  EXPECT_EQ(0x7FF, ac.getCommand());
  EXPECT_EQ(16, ac.getTemp());
  ac.setCommand(0x800);  // Out-of-range input must leave valid storage intact.
  EXPECT_EQ(0x7FF, ac.getCommand());
  EXPECT_TRUE(IRBluestarAc::validChecksum(ac.getRaw()));
}

TEST(TestBluestarClass, UnknownClockAndFlagBitsArePreserved) {
  IRBluestarAc ac(kGpioUnused);
  uint8_t state[kBluestarStateLength];
  std::memcpy(state, kCool16, sizeof(state));
  state[1] |= 0x10;
  state[2] |= 0x24;
  state[3] |= 0xC0;
  state[8] |= 0xA8;
  ASSERT_TRUE(ac.setRaw(state));
  ac.setMode(kBluestarDry);
  ac.setTemp(29);
  ac.setFan(kBluestarFanMed);
  ac.setPower(true);
  ac.setLight(false);
  ac.setSleep(true);
  ac.setTurbo(true);
  ac.setEcono(true);
  ac.setCapacity(kBluestarCapacity60);
  ac.setSwingH(false);
  ac.setSwingV(true);
  const uint8_t *raw = ac.getRaw();
  EXPECT_EQ(0x10, raw[1] & 0x10);
  EXPECT_EQ(0x24, raw[2] & 0x24);
  EXPECT_EQ(0xC0, raw[3] & 0xC0);
  EXPECT_EQ(0xA8, raw[8] & 0xF8);
  for (uint8_t i = 4; i <= 7; i++) EXPECT_EQ(state[i], raw[i]);
  EXPECT_EQ(state[6], ac.getClockMinute());
  EXPECT_TRUE(IRBluestarAc::validChecksum(raw));
}

TEST(TestBluestarClass, PowerAndTemperatureBoundaries) {
  IRBluestarAc ac(kGpioUnused);
  ASSERT_TRUE(ac.setRaw(kCool16));
  ac.off();
  EXPECT_FALSE(ac.getPower());
  EXPECT_EQ(24, ac.getTemp());
  ac.on();
  EXPECT_TRUE(ac.getPower());
  EXPECT_EQ(24, ac.getTemp());
  ac.setTemp(16);  // Lower temperatures remain legal in ordinary On packets.
  EXPECT_EQ(16, ac.getTemp());
  ac.setTemp(0);
  EXPECT_EQ(16, ac.getTemp());
  ac.setTemp(255);
  EXPECT_EQ(30, ac.getTemp());
  ac.setTemp(25);
  ac.off();
  ac.on();
  EXPECT_EQ(25, ac.getTemp());
  EXPECT_EQ(kBluestarCommandPower, ac.getCommand());
}

TEST(TestBluestarClass, ModeAndFanOnlyHiddenTemperature) {
  IRBluestarAc ac(kGpioUnused);
  ASSERT_TRUE(ac.setRaw(kCool16));
  ac.setMode(kBluestarDry);
  EXPECT_EQ(kBluestarDry, ac.getMode());
  EXPECT_EQ(16, ac.getTemp());  // No invented remembered Dry preference.
  ac.setTemp(27);
  ac.setMode(kBluestarFanOnly);
  EXPECT_EQ(24, ac.getTemp());
  ac.setTemp(19);  // No user temperature control in Fan-only.
  EXPECT_EQ(24, ac.getTemp());
  ac.setMode(kBluestarAuto);
  ac.setTemp(23);
  EXPECT_EQ(23, ac.getTemp());
  ac.setMode(0xFF);
  EXPECT_EQ(kBluestarCool, ac.getMode());
  EXPECT_EQ(kBluestarCommandMode, ac.getCommand());
}

TEST(TestBluestarClass, CapturedFanCycle) {
  IRBluestarAc ac(kGpioUnused);
  const uint8_t medium[kBluestarStateLength] = {
      0x33, 0x48, 0x82, 0x10, 0x1D, 0x9D, 0x1F, 0xA4, 0x05, 0x70, 0x07, 0xD0};
  const uint8_t high[kBluestarStateLength] = {
      0x33, 0x28, 0x82, 0x10, 0x1D, 0x9D, 0x1F, 0xA4, 0x05, 0x90, 0x07, 0xD0};
  const uint8_t automatic[kBluestarStateLength] = {
      0x33, 0xE8, 0x82, 0x10, 0x1D, 0x9D, 0x1F, 0xA4, 0x05, 0xD0, 0x07, 0xD0};
  const uint8_t low[kBluestarStateLength] = {
      0x33, 0x88, 0x82, 0x10, 0x1D, 0x9D, 0x1F, 0xA4, 0x05, 0x30, 0x07, 0xD0};
  ASSERT_TRUE(ac.setRaw(medium));
  ac.setFan(kBluestarFanHigh);
  EXPECT_STATE_EQ(high, ac.getRaw(), kBluestarBits);
  ac.setFan(kBluestarFanAuto);
  EXPECT_STATE_EQ(automatic, ac.getRaw(), kBluestarBits);
  ac.setFan(kBluestarFanLow);
  EXPECT_STATE_EQ(low, ac.getRaw(), kBluestarBits);
  ac.setFan(kBluestarFanMed);
  EXPECT_STATE_EQ(medium, ac.getRaw(), kBluestarBits);
}

TEST(TestBluestarClass, CapturedEnergySaveCoupling) {
  IRBluestarAc ac(kGpioUnused);
  const uint8_t baseline[kBluestarStateLength] = {
      0x33, 0xE8, 0x82, 0x10, 0x24, 0xA4, 0x18, 0xA4, 0x05, 0xC9, 0x05, 0xC4};
  const uint8_t on[kBluestarStateLength] = {
      0x33, 0x88, 0x82, 0x38, 0x24, 0xA4, 0x18, 0xA4, 0x05, 0x01, 0x07, 0xC1};
  const uint8_t off[kBluestarStateLength] = {
      0x33, 0x88, 0x82, 0x18, 0x24, 0xA4, 0x18, 0xA4, 0x05, 0x21, 0x07, 0xC1};
  ASSERT_TRUE(ac.setRaw(baseline));
  ac.setEcono(true);
  EXPECT_TRUE(ac.getEcono());
  EXPECT_STATE_EQ(on, ac.getRaw(), kBluestarBits);
  ac.setEcono(false);
  EXPECT_FALSE(ac.getEcono());
  EXPECT_STATE_EQ(off, ac.getRaw(), kBluestarBits);
  EXPECT_EQ(24, ac.getTemp());
  EXPECT_EQ(kBluestarFanLow, ac.getFan());
}

TEST(TestBluestarClass, StandaloneTurboPreservesBaseSettings) {
  IRBluestarAc ac(kGpioUnused);
  const uint8_t on[kBluestarStateLength] = {
      0x33, 0x88, 0x92, 0x18, 0x06, 0x86, 0x36, 0xA4, 0x05, 0x2F, 0x07, 0xC9};
  ASSERT_TRUE(ac.setRaw(kCool24));
  ac.setTurbo(true);
  EXPECT_STATE_EQ(on, ac.getRaw(), kBluestarBits);
  EXPECT_EQ(24, ac.getTemp());
  EXPECT_EQ(kBluestarFanLow, ac.getFan());
  EXPECT_EQ(16, ac.getEffectiveTemp());
  EXPECT_EQ(kBluestarFanHigh, ac.getEffectiveFan());
  EXPECT_EQ(16, ac.toCommon().degrees);
  EXPECT_EQ(stdAc::fanspeed_t::kHigh, ac.toCommon().fanspeed);
  EXPECT_EQ(kBluestarCapacityDefault, ac.getCapacity());
  ac.setTurbo(false);
  EXPECT_EQ(24, ac.getEffectiveTemp());
  EXPECT_EQ(kBluestarFanLow, ac.getEffectiveFan());
  for (uint8_t i = 0; i < 10; i++) EXPECT_EQ(kCool24[i], ac.getRaw()[i]);
}

TEST(TestBluestarClass, CapturedConvertibleCycleIsDistinctFromTurbo) {
  IRBluestarAc ac(kGpioUnused);
  const uint8_t states[][kBluestarStateLength] = {
    {0x33, 0x88, 0x82, 0x18, 0x00, 0x80, 0x00, 0xA4, 0x05, 0x81, 0x05, 0xC4},
    {0x33, 0x08, 0x82, 0x18, 0x00, 0x80, 0x00, 0xA4, 0x01, 0x05, 0x00, 0x00},
    {0x33, 0x28, 0x82, 0x18, 0x00, 0x80, 0x00, 0xA4, 0x02, 0xE4, 0x00, 0x00},
    {0x33, 0x48, 0x82, 0x18, 0x00, 0x80, 0x00, 0xA4, 0x03, 0xC3, 0x00, 0x00},
    {0x33, 0x88, 0x82, 0x18, 0x00, 0x80, 0x00, 0xA4, 0x04, 0x82, 0x00, 0x00},
    {0x33, 0x88, 0x82, 0x18, 0x00, 0x80, 0x00, 0xA4, 0x05, 0x81, 0x00, 0x00}};
  ASSERT_TRUE(ac.setRaw(states[0]));
  for (uint8_t capacity = 1; capacity <= 5; capacity++) {
    ac.setCapacity(capacity);
    EXPECT_STATE_EQ(states[capacity], ac.getRaw(), kBluestarBits);
    EXPECT_FALSE(ac.getTurbo());
    EXPECT_EQ(24, ac.getEffectiveTemp());
  }
  ac.setCapacity(kBluestarCapacityTurbo);
  EXPECT_EQ(kBluestarFanTurbo, ac.getFan());
  EXPECT_EQ(kBluestarFanHigh, ac.getEffectiveFan());
  EXPECT_TRUE(ac.toCommon().turbo);
  EXPECT_EQ(24, ac.toCommon().degrees);
}

TEST(TestBluestarClass, IndependentFlagsAndCommonConversion) {
  IRBluestarAc ac(kGpioUnused);
  ASSERT_TRUE(ac.setRaw(kCool16));
  ac.setSwingH(false);
  EXPECT_FALSE(ac.getSwingH());
  EXPECT_FALSE(ac.getSwingV());
  ac.setSwingV(true);
  EXPECT_TRUE(ac.getSwingV());
  EXPECT_FALSE(ac.getSwingH());
  ac.setLight(false);
  ac.setSleep(true);
  EXPECT_FALSE(ac.getLight());
  EXPECT_TRUE(ac.getSleep());
  EXPECT_EQ(kBluestarCommandSleep, ac.getCommand());
  const stdAc::state_t common = ac.toCommon();
  EXPECT_EQ(BLUESTAR, common.protocol);
  EXPECT_EQ(stdAc::swingh_t::kOff, common.swingh);
  EXPECT_EQ(stdAc::swingv_t::kAuto, common.swingv);
  EXPECT_EQ(0, common.sleep);
  EXPECT_FALSE(common.light);
  EXPECT_EQ(-1, common.clock);
  ac.setSleep(false);
  ac.setLight(true);
  ac.setSwingH(true);
  ac.setSwingV(false);
  for (uint8_t i = 0; i < 10; i++) EXPECT_EQ(kCool16[i], ac.getRaw()[i]);
}

TEST(TestBluestarClass, CommonSenderUsesDesiredState) {
  IRac common(kGpioUnused);
  IRBluestarAc ac(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  common.bluestar(&ac, true, stdAc::opmode_t::kCool, 18,
                   stdAc::fanspeed_t::kHigh, stdAc::swingv_t::kOff,
                   stdAc::swingh_t::kAuto, false, false, true, -1);
  ac._irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&ac._irsend.capture));
  ASSERT_EQ(BLUESTAR, ac._irsend.capture.decode_type);
  stdAc::state_t decoded;
  ASSERT_TRUE(IRAcUtils::decodeToState(&ac._irsend.capture, &decoded));
  EXPECT_TRUE(decoded.power);
  EXPECT_EQ(18, decoded.degrees);
  EXPECT_EQ(stdAc::fanspeed_t::kHigh, decoded.fanspeed);
  EXPECT_EQ(stdAc::swingh_t::kAuto, decoded.swingh);
  EXPECT_EQ(stdAc::swingv_t::kOff, decoded.swingv);
  EXPECT_TRUE(decoded.light);
  EXPECT_FALSE(decoded.turbo);
  EXPECT_FALSE(decoded.econo);
  EXPECT_EQ(-1, decoded.sleep);
  stdAc::state_t desired = decoded;
  desired.protocol = BLUESTAR;
  EXPECT_TRUE(common.sendAc(desired));
}
