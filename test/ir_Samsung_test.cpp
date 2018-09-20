// Copyright 2017 David Conran

#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "ir_Samsung.h"
#include "gtest/gtest.h"

// Tests for sendSAMSUNG().

// Test sending typical data only.
TEST(TestSendSamsung, SendDataOnly) {
  IRsendTest irsend(4);
  irsend.begin();

  irsend.reset();
  irsend.sendSAMSUNG(0xE0E09966);  // Samsung TV Power On.
  EXPECT_EQ("m4480s4480"
            "m560s1680m560s1680m560s1680m560s560m560s560m560s560m560s560"
            "m560s560m560s1680m560s1680m560s1680m560s560m560s560m560s560"
            "m560s560m560s560m560s1680m560s560m560s560m560s1680m560s1680"
            "m560s560m560s560m560s1680m560s560m560s1680m560s1680m560s560"
            "m560s560m560s1680m560s1680m560s560m560s47040",
            irsend.outputStr());

  irsend.reset();
}

// Test sending with different repeats.
TEST(TestSendSamsung, SendWithRepeats) {
  IRsendTest irsend(4);
  irsend.begin();

  irsend.reset();
  irsend.sendSAMSUNG(0xE0E09966, kSamsungBits, 1);  // 1 repeat.
  EXPECT_EQ("m4480s4480"
            "m560s1680m560s1680m560s1680m560s560m560s560m560s560m560s560"
            "m560s560m560s1680m560s1680m560s1680m560s560m560s560m560s560"
            "m560s560m560s560m560s1680m560s560m560s560m560s1680m560s1680"
            "m560s560m560s560m560s1680m560s560m560s1680m560s1680m560s560"
            "m560s560m560s1680m560s1680m560s560m560s47040"
            "m4480s4480"
            "m560s1680m560s1680m560s1680m560s560m560s560m560s560m560s560"
            "m560s560m560s1680m560s1680m560s1680m560s560m560s560m560s560"
            "m560s560m560s560m560s1680m560s560m560s560m560s1680m560s1680"
            "m560s560m560s560m560s1680m560s560m560s1680m560s1680m560s560"
            "m560s560m560s1680m560s1680m560s560m560s47040"
            , irsend.outputStr());
  irsend.sendSAMSUNG(0xE0E09966, kSamsungBits, 2);  // 2 repeats.
  EXPECT_EQ("m4480s4480"
            "m560s1680m560s1680m560s1680m560s560m560s560m560s560m560s560"
            "m560s560m560s1680m560s1680m560s1680m560s560m560s560m560s560"
            "m560s560m560s560m560s1680m560s560m560s560m560s1680m560s1680"
            "m560s560m560s560m560s1680m560s560m560s1680m560s1680m560s560"
            "m560s560m560s1680m560s1680m560s560m560s47040"
            "m4480s4480"
            "m560s1680m560s1680m560s1680m560s560m560s560m560s560m560s560"
            "m560s560m560s1680m560s1680m560s1680m560s560m560s560m560s560"
            "m560s560m560s560m560s1680m560s560m560s560m560s1680m560s1680"
            "m560s560m560s560m560s1680m560s560m560s1680m560s1680m560s560"
            "m560s560m560s1680m560s1680m560s560m560s47040"
            "m4480s4480"
            "m560s1680m560s1680m560s1680m560s560m560s560m560s560m560s560"
            "m560s560m560s1680m560s1680m560s1680m560s560m560s560m560s560"
            "m560s560m560s560m560s1680m560s560m560s560m560s1680m560s1680"
            "m560s560m560s560m560s1680m560s560m560s1680m560s1680m560s560"
            "m560s560m560s1680m560s1680m560s560m560s47040"
            , irsend.outputStr());
}

// Tests for encodeSAMSUNG().

TEST(TestEncodeSamsung, NormalEncoding) {
  IRsendTest irsend(4);
  EXPECT_EQ(0xFF, irsend.encodeSAMSUNG(0, 0));
  EXPECT_EQ(0x8080807F, irsend.encodeSAMSUNG(1, 1));
  EXPECT_EQ(0xF8F805FA, irsend.encodeSAMSUNG(0x1F, 0xA0));
  EXPECT_EQ(0xA0A0CC33, irsend.encodeSAMSUNG(0x05, 0x33));
  EXPECT_EQ(0xFFFFFF00, irsend.encodeSAMSUNG(0xFF, 0xFF));
  EXPECT_EQ(0xE0E09966, irsend.encodeSAMSUNG(0x07, 0x99));
}

// Tests for decodeSAMSUNG().

// Decode normal Samsung messages.
TEST(TestDecodeSamsung, NormalDecodeWithStrict) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  // Normal Samsung 32-bit message.
  irsend.reset();
  irsend.sendSAMSUNG(0xE0E09966);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeSAMSUNG(&irsend.capture, kSamsungBits, true));
  EXPECT_EQ(SAMSUNG, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungBits, irsend.capture.bits);
  EXPECT_EQ(0xE0E09966, irsend.capture.value);
  EXPECT_EQ(0x07, irsend.capture.address);
  EXPECT_EQ(0x99, irsend.capture.command);

  // Synthesised Normal Samsung 32-bit message.
  irsend.reset();
  irsend.sendSAMSUNG(irsend.encodeSAMSUNG(0x07, 0x99));
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeSAMSUNG(&irsend.capture, kSamsungBits, true));
  EXPECT_EQ(SAMSUNG, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungBits, irsend.capture.bits);
  EXPECT_EQ(0xE0E09966, irsend.capture.value);
  EXPECT_EQ(0x07, irsend.capture.address);
  EXPECT_EQ(0x99, irsend.capture.command);

  // Synthesised Normal Samsung 32-bit message.
  irsend.reset();
  irsend.sendSAMSUNG(irsend.encodeSAMSUNG(0x1, 0x1));
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeSAMSUNG(&irsend.capture, kSamsungBits, true));
  EXPECT_EQ(SAMSUNG, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungBits, irsend.capture.bits);
  EXPECT_EQ(0x8080807F, irsend.capture.value);
  EXPECT_EQ(0x1, irsend.capture.address);
  EXPECT_EQ(0x1, irsend.capture.command);
}

// Decode normal repeated Samsung messages.
TEST(TestDecodeSamsung, NormalDecodeWithRepeatAndStrict) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  // Normal Samsung 32-bit message.
  irsend.reset();
  irsend.sendSAMSUNG(0xE0E09966, kSamsungBits, 2);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeSAMSUNG(&irsend.capture, kSamsungBits, true));
  EXPECT_EQ(SAMSUNG, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungBits, irsend.capture.bits);
  EXPECT_EQ(0xE0E09966, irsend.capture.value);
  EXPECT_EQ(0x07, irsend.capture.address);
  EXPECT_EQ(0x99, irsend.capture.command);
}

// Decode unsupported Samsung messages.
TEST(TestDecodeSamsung, DecodeWithNonStrictValues) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  irsend.sendSAMSUNG(0x0);  // Illegal value Samsung 32-bit message.
  irsend.makeDecodeResult();
  // Should fail with strict on.
  ASSERT_FALSE(irrecv.decodeSAMSUNG(&irsend.capture, kSamsungBits, true));
  // Should pass if strict off.
  ASSERT_TRUE(irrecv.decodeSAMSUNG(&irsend.capture, kSamsungBits, false));
  EXPECT_EQ(SAMSUNG, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungBits, irsend.capture.bits);
  EXPECT_EQ(0x0, irsend.capture.value);
  EXPECT_EQ(0x0, irsend.capture.address);
  EXPECT_EQ(0x0, irsend.capture.command);

  irsend.reset();
  irsend.sendSAMSUNG(0x12345678);  // Illegal value Samsung 32-bit message.
  irsend.makeDecodeResult();
  // Should fail with strict on.
  ASSERT_FALSE(irrecv.decodeSAMSUNG(&irsend.capture, kSamsungBits, true));
  // Should pass if strict off.
  ASSERT_TRUE(irrecv.decodeSAMSUNG(&irsend.capture, kSamsungBits, false));
  EXPECT_EQ(SAMSUNG, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungBits, irsend.capture.bits);
  EXPECT_EQ(0x12345678, irsend.capture.value);
  EXPECT_EQ(0x48, irsend.capture.address);
  EXPECT_EQ(0x6A, irsend.capture.command);

  // Illegal over length (36-bit) message.
  irsend.reset();
  irsend.sendSAMSUNG(irsend.encodeSAMSUNG(0, 0), 36);
  irsend.makeDecodeResult();
  // Should fail with strict on.
  ASSERT_FALSE(irrecv.decodeSAMSUNG(&irsend.capture, kSamsungBits, true));
  // Shouldn't pass if strict off and wrong expected bit size.
  ASSERT_FALSE(irrecv.decodeSAMSUNG(&irsend.capture, kSamsungBits, false));
  // Re-decode with correct bit size.
  ASSERT_FALSE(irrecv.decodeSAMSUNG(&irsend.capture, 36, true));
  ASSERT_TRUE(irrecv.decodeSAMSUNG(&irsend.capture, 36, false));
  EXPECT_EQ(SAMSUNG, irsend.capture.decode_type);
  EXPECT_EQ(36, irsend.capture.bits);
  EXPECT_EQ(0xFF, irsend.capture.value);  // We told it to expect 8 bits less.
  EXPECT_EQ(0x00, irsend.capture.address);
  EXPECT_EQ(0x00, irsend.capture.command);

  // Illegal under length (16-bit) message
  irsend.reset();
  irsend.sendSAMSUNG(irsend.encodeSAMSUNG(0x0, 0x0), 16);
  irsend.makeDecodeResult();
  // Should fail with strict on.
  ASSERT_FALSE(irrecv.decodeSAMSUNG(&irsend.capture, kSamsungBits, true));
  // And it should fail when we expect more bits.
  ASSERT_FALSE(irrecv.decodeSAMSUNG(&irsend.capture, kSamsungBits, false));

  // Should pass if strict off if we ask for correct nr. of bits sent.
  ASSERT_TRUE(irrecv.decodeSAMSUNG(&irsend.capture, 16, false));
  EXPECT_EQ(SAMSUNG, irsend.capture.decode_type);
  EXPECT_EQ(16, irsend.capture.bits);
  EXPECT_EQ(0xFF, irsend.capture.value);  // We told it to expect 4 bits less.
  EXPECT_EQ(0x00, irsend.capture.address);
  EXPECT_EQ(0x00, irsend.capture.command);

  // Should fail as we are expecting less bits than there are.
  ASSERT_FALSE(irrecv.decodeSAMSUNG(&irsend.capture, 12, false));
}

// Decode (non-standard) 64-bit messages.
// Decode unsupported Samsung messages.
TEST(TestDecodeSamsung, Decode64BitMessages) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  // Illegal value & size Samsung 64-bit message.
  irsend.sendSAMSUNG(0xFFFFFFFFFFFFFFFF, 64);
  irsend.makeDecodeResult();
  ASSERT_FALSE(irrecv.decodeSAMSUNG(&irsend.capture, kSamsungBits, true));
  // Should work with a 'normal' match (not strict)
  ASSERT_TRUE(irrecv.decodeSAMSUNG(&irsend.capture, 64, false));
  EXPECT_EQ(SAMSUNG, irsend.capture.decode_type);
  EXPECT_EQ(64, irsend.capture.bits);
  EXPECT_EQ(0xFFFFFFFFFFFFFFFF, irsend.capture.value);
  EXPECT_EQ(0xFF, irsend.capture.address);
  EXPECT_EQ(0xFF, irsend.capture.command);
}

// Decode a 'real' example via GlobalCache
TEST(TestDecodeSamsung, DecodeGlobalCacheExample) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  // Samsung TV Power On from Global Cache.
  uint16_t gc_test[71] = {38000, 1, 1, 172, 172, 22, 64, 22, 64, 22, 64, 22, 21,
                          22, 21, 22, 21, 22, 21, 22, 21, 22, 64, 22, 64, 22,
                          64, 22, 21, 22, 21, 22, 21, 22, 21, 22, 21, 22, 64,
                          22, 21, 22, 21, 22, 64, 22, 64, 22, 21, 22, 21, 22,
                          64, 22, 21, 22, 64, 22, 64, 22, 21, 22, 21, 22, 64,
                          22, 64, 22, 21, 22, 1820};
  irsend.sendGC(gc_test, 71);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decodeSAMSUNG(&irsend.capture));
  EXPECT_EQ(SAMSUNG, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungBits, irsend.capture.bits);
  EXPECT_EQ(0xE0E09966, irsend.capture.value);
  EXPECT_EQ(0x07, irsend.capture.address);
  EXPECT_EQ(0x99, irsend.capture.command);
}

// Fail to decode a non-Samsung example via GlobalCache
TEST(TestDecodeSamsung, FailToDecodeNonSamsungExample) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  // Modified a few entries to unexpected values, based on previous test case.
  uint16_t gc_test[71] = {38000, 1, 1, 172, 172, 22, 64, 22, 64, 22, 64, 22, 21,
                          22, 21, 22, 21, 22, 11, 22, 21, 22, 128, 22, 64, 22,
                          64, 22, 21, 22, 21, 22, 21, 22, 21, 22, 21, 22, 64,
                          22, 21, 22, 21, 22, 64, 22, 64, 22, 21, 22, 21, 22,
                          64, 22, 21, 22, 64, 22, 64, 22, 21, 22, 21, 22, 64,
                          22, 64, 22, 21, 22, 1820};
  irsend.sendGC(gc_test, 71);
  irsend.makeDecodeResult();

  ASSERT_FALSE(irrecv.decodeSAMSUNG(&irsend.capture));
  ASSERT_FALSE(irrecv.decodeSAMSUNG(&irsend.capture, kSamsungBits, false));
}

// Tests for sendSamsungAC().

// Test sending typical data only.
TEST(TestSendSamsungAC, SendDataOnly) {
  IRsendTest irsend(0);
  irsend.begin();
  uint8_t data[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0x02, 0xAF, 0x71, 0x00, 0x15, 0xF0};
  irsend.sendSamsungAC(data);
  EXPECT_EQ(
      "m690s17844"
      "m3086s8864"
      "m586s436m586s1432m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s1432m586s436m586s436m586s1432m586s436m586s436m586s1432"
      "m586s1432m586s1432m586s1432m586s1432m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s1432m586s1432m586s1432m586s1432"
      "m586s2886"
      "m3086s8864"
      "m586s1432m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s436m586s1432m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s1432m586s1432m586s1432m586s1432m586s436m586s1432m586s436m586s1432"
      "m586s1432m586s436m586s436m586s436m586s1432m586s1432m586s1432m586s436"
      "m586s436m586s436m586s436m586s436m586s436m586s436m586s436m586s436"
      "m586s1432m586s436m586s1432m586s436m586s1432m586s436m586s436m586s436"
      "m586s436m586s436m586s436m586s436m586s1432m586s1432m586s1432m586s1432"
      "m586s100000", irsend.outputStr());
}

// Tests for IRSamsungAc class.

TEST(TestIRSamsungAcClass, SetAndGetPower) {
  IRSamsungAc samsung(0);
  samsung.on();
  EXPECT_TRUE(samsung.getPower());
  samsung.off();
  EXPECT_FALSE(samsung.getPower());
  samsung.setPower(true);
  EXPECT_TRUE(samsung.getPower());
  samsung.setPower(false);
  EXPECT_FALSE(samsung.getPower());
}

TEST(TestIRSamsungAcClass, SetAndGetSwing) {
  IRSamsungAc samsung(0);
  samsung.setSwing(true);
  EXPECT_TRUE(samsung.getSwing());
  samsung.setSwing(false);
  EXPECT_FALSE(samsung.getSwing());
  samsung.setSwing(true);
  EXPECT_TRUE(samsung.getSwing());
}

TEST(TestIRSamsungAcClass, SetAndGetClean) {
  IRSamsungAc samsung(0);
  samsung.setClean(true);
  EXPECT_TRUE(samsung.getClean());
  samsung.setClean(false);
  EXPECT_FALSE(samsung.getClean());
  samsung.setClean(true);
  EXPECT_TRUE(samsung.getClean());
}

TEST(TestIRSamsungAcClass, SetAndGetBeep) {
  IRSamsungAc samsung(0);
  samsung.setBeep(false);
  EXPECT_FALSE(samsung.getBeep());
  samsung.setBeep(true);
  EXPECT_TRUE(samsung.getBeep());
  samsung.setBeep(false);
  EXPECT_FALSE(samsung.getBeep());
  samsung.setBeep(true);
  EXPECT_TRUE(samsung.getBeep());
}

TEST(TestIRSamsungAcClass, SetAndGetTemp) {
  IRSamsungAc samsung(0);
  samsung.setTemp(25);
  EXPECT_EQ(25, samsung.getTemp());
  samsung.setTemp(kSamsungAcMinTemp);
  EXPECT_EQ(kSamsungAcMinTemp, samsung.getTemp());
  samsung.setTemp(kSamsungAcMinTemp - 1);
  EXPECT_EQ(kSamsungAcMinTemp, samsung.getTemp());
  samsung.setTemp(kSamsungAcMaxTemp);
  EXPECT_EQ(kSamsungAcMaxTemp, samsung.getTemp());
  samsung.setTemp(kSamsungAcMaxTemp + 1);
  EXPECT_EQ(kSamsungAcMaxTemp, samsung.getTemp());
}

TEST(TestIRSamsungAcClass, SetAndGetMode) {
  IRSamsungAc samsung(0);
  samsung.setMode(kSamsungAcCool);
  EXPECT_EQ(kSamsungAcCool, samsung.getMode());
  EXPECT_NE(kSamsungAcFanAuto2, samsung.getFan());
  samsung.setMode(kSamsungAcHeat);
  EXPECT_EQ(kSamsungAcHeat, samsung.getMode());
  EXPECT_NE(kSamsungAcFanAuto2, samsung.getFan());
  samsung.setMode(kSamsungAcAuto);
  EXPECT_EQ(kSamsungAcAuto, samsung.getMode());
  EXPECT_EQ(kSamsungAcFanAuto2, samsung.getFan());
  samsung.setMode(kSamsungAcDry);
  EXPECT_EQ(kSamsungAcDry, samsung.getMode());
  EXPECT_NE(kSamsungAcFanAuto2, samsung.getFan());
}

TEST(TestIRSamsungAcClass, SetAndGetFan) {
  IRSamsungAc samsung(0);
  samsung.setMode(kSamsungAcCool);  // Most fan modes avail in this setting.
  samsung.setFan(kSamsungAcFanAuto);
  EXPECT_EQ(kSamsungAcFanAuto, samsung.getFan());
  samsung.setFan(kSamsungAcFanLow);
  EXPECT_EQ(kSamsungAcFanLow, samsung.getFan());
  samsung.setFan(kSamsungAcFanAuto2);  // Not available in Cool mode.
  EXPECT_EQ(kSamsungAcFanLow, samsung.getFan());  // Shouldn't change.
  samsung.setMode(kSamsungAcAuto);  // Has special fan setting.
  EXPECT_EQ(kSamsungAcFanAuto2, samsung.getFan());
  samsung.setFan(kSamsungAcFanLow);  // Shouldn't be available in Auto mode.
  EXPECT_EQ(kSamsungAcFanAuto2, samsung.getFan());
  samsung.setMode(kSamsungAcHeat);  // Most fan modes avail in this setting.
  samsung.setFan(kSamsungAcFanHigh);
  EXPECT_EQ(kSamsungAcFanHigh, samsung.getFan());
}

TEST(TestIRSamsungAcClass, SetAndGetQuiet) {
  IRSamsungAc samsung(0);
  samsung.setQuiet(false);
  EXPECT_FALSE(samsung.getQuiet());
  samsung.setFan(kSamsungAcFanHigh);
  samsung.setQuiet(true);
  EXPECT_TRUE(samsung.getQuiet());
  EXPECT_EQ(kSamsungAcFanAuto, samsung.getFan());
  samsung.setQuiet(false);
  EXPECT_FALSE(samsung.getQuiet());
}

TEST(TestIRSamsungAcClass, ChecksumCalculation) {
  IRSamsungAc samsung(0);

  const uint8_t originalstate[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0x02, 0xAF, 0x71, 0x00, 0x15, 0xF0};
  uint8_t examplestate[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0x02, 0xAF, 0x71, 0x00, 0x15, 0xF0};

  EXPECT_TRUE(IRSamsungAc::validChecksum(examplestate));
  EXPECT_EQ(0, IRSamsungAc::calcChecksum(examplestate));

  examplestate[8] = 0x12;  // Set an incoorect checksum.
  EXPECT_FALSE(IRSamsungAc::validChecksum(examplestate));
  EXPECT_EQ(0, IRSamsungAc::calcChecksum(examplestate));
  samsung.setRaw(examplestate);
  // Extracting the state from the object should have a correct checksum.
  EXPECT_TRUE(IRSamsungAc::validChecksum(samsung.getRaw()));
  EXPECT_STATE_EQ(originalstate, samsung.getRaw(), kSamsungAcBits);
  examplestate[8] = 0x02;  // Restore old checksum value.

  // Change the state to force a different checksum.
  examplestate[11] = 0x01;
  EXPECT_FALSE(IRSamsungAc::validChecksum(examplestate));
  EXPECT_EQ(0xF, IRSamsungAc::calcChecksum(examplestate));
}

TEST(TestIRSamsungAcClass, HumanReadable) {
  IRSamsungAc samsung(0);
  EXPECT_EQ("Power: On, Mode: 1 (COOL), Temp: 16C, Fan: 2 (LOW), Swing: Off, "
            "Beep: Off, Clean: Off, Quiet: Off",
            samsung.toString());
  samsung.setTemp(kSamsungAcMaxTemp);
  samsung.setMode(kSamsungAcHeat);
  samsung.off();
  samsung.setFan(kSamsungAcFanHigh);
  samsung.setSwing(true);
  samsung.setBeep(true);
  samsung.setClean(true);
  EXPECT_EQ("Power: Off, Mode: 4 (HEAT), Temp: 30C, Fan: 5 (HIGH), Swing: On, "
            "Beep: On, Clean: On, Quiet: Off",
            samsung.toString());
  samsung.setQuiet(true);
  EXPECT_EQ("Power: Off, Mode: 4 (HEAT), Temp: 30C, Fan: 0 (AUTO), Swing: On, "
            "Beep: On, Clean: On, Quiet: On",
            samsung.toString());
}

// Tests for decodeSamsungAC().

// Decode normal SamsungAC messages.
TEST(TestDecodeSamsungAC, SyntheticDecode) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();
  irsend.reset();
  uint8_t expectedState[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0x02, 0xAF, 0x71, 0x00, 0x15, 0xF0};
  // Synthesised Normal Samsung A/C message.
  irsend.sendSamsungAC(expectedState);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(SAMSUNG_AC, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungAcBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
}

// Decode a real Samsung A/C example from Issue #505
TEST(TestDecodeSamsungAC, DecodeRealExample) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  // Samsung A/C example from Issue #505
  uint16_t rawData[233] = {
      690, 17844, 3084, 8864, 606, 406, 586, 1410, 580, 436, 570, 424, 570, 426,
      570, 404, 596, 418, 580, 416, 584, 410, 586, 1402, 588, 408, 586, 410,
      584, 1380, 610, 408, 586, 408, 586, 1404, 586, 1404, 586, 1408, 594, 1396,
      596, 1394, 602, 418, 582, 410, 586, 408, 584, 408, 586, 408, 586, 410,
      586, 408, 586, 410, 586, 408, 586, 408, 586, 408, 586, 408, 586, 410, 584,
      436, 558, 436, 570, 424, 570, 424, 574, 420, 578, 416, 582, 412, 586, 410,
      586, 408, 584, 410, 586, 408, 586, 410, 584, 410, 584, 408, 586, 408, 586,
      410, 586, 408, 586, 412, 584, 436, 556, 1410, 592, 1396, 602, 1390, 608,
      1384, 608, 2886, 3086, 8858, 610, 1380, 610, 410, 586, 408, 586, 410, 586,
      408, 586, 410, 586, 408, 586, 436, 558, 436, 554, 1410, 594, 426, 572,
      422, 578, 418, 582, 412, 586, 410, 584, 410, 586, 1380, 610, 1382, 608,
      1404, 586, 1404, 586, 408, 586, 1432, 558, 436, 554, 1414, 590, 1398, 602,
      418, 580, 414, 586, 410, 584, 1382, 606, 1382, 608, 1382, 608, 408, 586,
      408, 586, 408, 586, 408, 586, 410, 584, 436, 560, 434, 570, 426, 566, 430,
      568, 1400, 600, 416, 584, 1406, 586, 410, 584, 1384, 606, 410, 586, 410,
      584, 408, 586, 408, 586, 408, 586, 408, 588, 410, 584, 1408, 590, 1400,
      592, 1398, 602, 1388, 612};
  uint8_t expectedState[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0x02, 0xAF, 0x71, 0x00, 0x15, 0xF0};

  irsend.sendRaw(rawData, 233, 38000);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(SAMSUNG_AC, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungAcBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);

  IRSamsungAc samsung(0);
  samsung.setRaw(irsend.capture.state);
  EXPECT_EQ("Power: On, Mode: 1 (COOL), Temp: 16C, Fan: 2 (LOW), Swing: Off, "
            "Beep: Off, Clean: Off, Quiet: Off",
            samsung.toString());
}

// Decode a real Samsung A/C example from Issue #505
TEST(TestDecodeSamsungAC, DecodeRealExample2) {
  IRsendTest irsend(4);
  IRrecv irrecv(4);
  irsend.begin();

  irsend.reset();
  // Samsung A/C example from Issue #505
  uint16_t rawData[233] = {
     668, 17834, 3092, 8862, 608, 410, 586, 1378, 612, 410, 584, 410, 586, 410,
     584, 410, 586, 408, 586, 408, 586, 410, 586, 1404, 588, 436, 558, 436, 570,
     1398, 592, 424, 576, 420, 578, 1388, 608, 1382, 610, 1382, 608, 1380, 610,
     1384, 606, 408, 586, 408, 588, 408, 588, 408, 586, 436, 558, 436, 570, 424,
     570, 426, 572, 422, 578, 418, 582, 412, 586, 408, 586, 410, 584, 410, 584,
     410, 584, 410, 586, 410, 586, 408, 586, 408, 586, 408, 586, 408, 586, 408,
     586, 438, 558, 436, 568, 426, 570, 424, 574, 422, 576, 418, 582, 414, 584,
     410, 586, 410, 584, 410, 586, 1380, 610, 1382, 608, 1404, 586, 1404, 602,
     2872, 3096, 8878, 582, 1432, 570, 426, 568, 426, 574, 420, 578, 416, 582,
     412, 586, 410, 584, 410, 586, 410, 586, 1382, 608, 410, 586, 410, 586, 408,
     586, 1404, 586, 1408, 582, 1410, 590, 428, 568, 1400, 598, 1394, 606, 1382,
     610, 1382, 608, 1378, 612, 1382, 608, 1384, 606, 1404, 586, 408, 586, 414,
     582, 436, 558, 1410, 590, 1422, 576, 1390, 608, 410, 586, 410, 586, 410,
     584, 410, 584, 410, 586, 410, 586, 410, 584, 410, 586, 1404, 586, 1404,
     588, 436, 560, 436, 486, 510, 566, 1400, 598, 420, 576, 418, 582, 414, 586,
     410, 584, 410, 584, 410, 586, 410, 584, 1382, 608, 1384, 606, 1384, 606,
     1408, 600};
  uint8_t expectedState[kSamsungAcStateLength] = {
      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
      0x01, 0xE2, 0xFE, 0x71, 0x80, 0x11, 0xF0};

  irsend.sendRaw(rawData, 233, 38000);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decodeSamsungAC(&irsend.capture));
  ASSERT_EQ(SAMSUNG_AC, irsend.capture.decode_type);
  EXPECT_EQ(kSamsungAcBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);

  IRSamsungAc samsung(0);
  samsung.setRaw(irsend.capture.state);
  EXPECT_EQ("Power: On, Mode: 1 (COOL), Temp: 24C, Fan: 0 (AUTO), Swing: Off, "
            "Beep: Off, Clean: Off, Quiet: Off",
            samsung.toString());
}
