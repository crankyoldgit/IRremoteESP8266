// Copyright 2017 David Conran

#include "IRremoteESP8266.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

// Tests decode().

// Test decode of a NEC message.
TEST(TestDecode, DecodeNEC) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendNEC(0x807F40BF);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(NEC, irsend.capture.decode_type);
  EXPECT_EQ(NEC_BITS, irsend.capture.bits);
  EXPECT_EQ(0x807F40BF, irsend.capture.value);
}

// Test decode of a JVC message.
TEST(TestDecode, DecodeJVC) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendJVC(0xC2B8);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(JVC, irsend.capture.decode_type);
  EXPECT_EQ(JVC_BITS, irsend.capture.bits);
  EXPECT_EQ(0xC2B8, irsend.capture.value);
}

// Test decode of a LG message.
TEST(TestDecode, DecodeLG) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendLG(0x4B4AE51);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(LG, irsend.capture.decode_type);
  EXPECT_EQ(LG_BITS, irsend.capture.bits);
  EXPECT_EQ(0x4B4AE51, irsend.capture.value);

  irsend.reset();
  irsend.sendLG(0xB4B4AE51, LG32_BITS);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(LG, irsend.capture.decode_type);
  EXPECT_EQ(LG32_BITS, irsend.capture.bits);
  EXPECT_EQ(0xB4B4AE51, irsend.capture.value);
}

// Test decode of a Panasonic message.
TEST(TestDecode, DecodePanasonic) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendPanasonic64(0x40040190ED7C);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodePanasonic(&irsend.capture, PANASONIC_BITS, true));
  EXPECT_EQ(PANASONIC, irsend.capture.decode_type);
  EXPECT_EQ(PANASONIC_BITS, irsend.capture.bits);
  EXPECT_EQ(0x40040190ED7C, irsend.capture.value);
}

// Test decode of a Samsun message.
TEST(TestDecode, DecodeSamsung) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendSAMSUNG(0xE0E09966);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(SAMSUNG, irsend.capture.decode_type);
  EXPECT_EQ(SAMSUNG_BITS, irsend.capture.bits);
  EXPECT_EQ(0xE0E09966, irsend.capture.value);
}

// Test decode of a Sherwood message.
TEST(TestDecode, DecodeSherwood) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendSherwood(0x807F40BF);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  // Sherwood codes are really NEC codes.
  EXPECT_EQ(NEC, irsend.capture.decode_type);
  EXPECT_EQ(NEC_BITS, irsend.capture.bits);
  EXPECT_EQ(0x807F40BF, irsend.capture.value);
}

// Test decode of a Whynter message.
TEST(TestDecode, DecodeWhynter) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendWhynter(0x87654321);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(WHYNTER, irsend.capture.decode_type);
  EXPECT_EQ(WHYNTER_BITS, irsend.capture.bits);
  EXPECT_EQ(0x87654321, irsend.capture.value);
}

// Test decode of Sony messages.
TEST(TestDecode, DecodeSony) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();

  // Synthesised Normal Sony 20-bit message.
  irsend.reset();
  irsend.sendSony(irsend.encodeSony(SONY_20_BITS, 0x1, 0x1, 0x1));
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(SONY, irsend.capture.decode_type);
  EXPECT_EQ(SONY_20_BITS, irsend.capture.bits);
  EXPECT_EQ(0x81080, irsend.capture.value);

  // Synthesised Normal Sony 15-bit message.
  irsend.reset();
  irsend.sendSony(irsend.encodeSony(SONY_15_BITS, 21, 1), SONY_15_BITS);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(SONY, irsend.capture.decode_type);
  EXPECT_EQ(SONY_15_BITS, irsend.capture.bits);
  EXPECT_EQ(0x5480, irsend.capture.value);


  // Synthesised Normal Sony 12-bit message.
  irsend.reset();
  irsend.sendSony(irsend.encodeSony(SONY_12_BITS, 21, 1), SONY_12_BITS);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(SONY, irsend.capture.decode_type);
  EXPECT_EQ(SONY_12_BITS, irsend.capture.bits);
  EXPECT_EQ(0xA90, irsend.capture.value);
}

// Test decode of Sharp messages.
TEST(TestDecode, DecodeSharp) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendSharpRaw(0x454A);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(SHARP, irsend.capture.decode_type);
  EXPECT_EQ(SHARP_BITS, irsend.capture.bits);
  EXPECT_EQ(0x454A, irsend.capture.value);
}

// Test decode of Sanyo messages.
TEST(TestDecode, DecodeSanyo) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendSanyoLC7461(0x2468DCB56A9);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(SANYO_LC7461, irsend.capture.decode_type);
  EXPECT_EQ(SANYO_LC7461_BITS, irsend.capture.bits);
  EXPECT_EQ(0x2468DCB56A9, irsend.capture.value);
}

// Test decode of RC-MM messages.
TEST(TestDecode, DecodeRCMM) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();

  // Normal RCMM 24-bit message.
  irsend.reset();
  irsend.sendRCMM(0xe0a600);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(RCMM, irsend.capture.decode_type);
  EXPECT_EQ(RCMM_BITS, irsend.capture.bits);
  EXPECT_EQ(0xe0a600, irsend.capture.value);

  // Normal RCMM 12-bit message.
  irsend.reset();
  irsend.sendRCMM(0x600, 12);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(RCMM, irsend.capture.decode_type);
  EXPECT_EQ(12, irsend.capture.bits);
  EXPECT_EQ(0x600, irsend.capture.value);

  // Normal RCMM 32-bit message.
  irsend.reset();
  irsend.sendRCMM(0x28e0a600, 32);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(RCMM, irsend.capture.decode_type);
  EXPECT_EQ(32, irsend.capture.bits);
  EXPECT_EQ(0x28e0a600, irsend.capture.value);
}

// Test decode of Mitsubishi messages.
TEST(TestDecode, DecodeMitsubishi) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendMitsubishi(0xC2B8);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MITSUBISHI, irsend.capture.decode_type);
  EXPECT_EQ(MITSUBISHI_BITS, irsend.capture.bits);
  EXPECT_EQ(0xC2B8, irsend.capture.value);
}

// Test decode of RC-5/RC-5X messages.
TEST(TestDecode, DecodeRC5) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  // Normal RC-5 12-bit message.
  irsend.reset();
  irsend.sendRC5(0x175);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(RC5, irsend.capture.decode_type);
  EXPECT_EQ(RC5_BITS, irsend.capture.bits);
  EXPECT_EQ(0x175, irsend.capture.value);
  // Synthesised Normal RC-5X 13-bit message.
  irsend.reset();
  irsend.sendRC5(irsend.encodeRC5X(0x02, 0x41, true), RC5X_BITS);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(RC5X, irsend.capture.decode_type);
  EXPECT_EQ(RC5X_BITS, irsend.capture.bits);
  EXPECT_EQ(0x1881, irsend.capture.value);
}

// Test decode of RC-6 messages.
TEST(TestDecode, DecodeRC6) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  // Normal RC-6 Mode 0 (20-bit) message.
  irsend.reset();
  irsend.sendRC6(0x175);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(RC6, irsend.capture.decode_type);
  EXPECT_EQ(RC6_MODE0_BITS, irsend.capture.bits);
  EXPECT_EQ(0x175, irsend.capture.value);

  // Normal RC-6 36-bit message.
  irsend.reset();
  irsend.sendRC6(0xC800F742A, RC6_36_BITS);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(RC6, irsend.capture.decode_type);
  EXPECT_EQ(RC6_36_BITS, irsend.capture.bits);
  EXPECT_EQ(0xC800F742A, irsend.capture.value);
}

// Test decode of Dish messages.
TEST(TestDecode, DecodeDish) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendDISH(0x9C00);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(DISH, irsend.capture.decode_type);
  EXPECT_EQ(DISH_BITS, irsend.capture.bits);
  EXPECT_EQ(0x9C00, irsend.capture.value);
}

// Test decode of Denon messages.
TEST(TestDecode, DecodeDenon) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  // Normal Denon 15-bit message. (Sharp)
  irsend.reset();
  irsend.sendDenon(0x2278);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(DENON, irsend.capture.decode_type);
  EXPECT_EQ(DENON_BITS, irsend.capture.bits);
  EXPECT_EQ(0x2278, irsend.capture.value);
  // Legacy Denon 14-bit message.
  irsend.reset();
  irsend.sendDenon(0x1278, DENON_LEGACY_BITS);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(DENON, irsend.capture.decode_type);
  EXPECT_EQ(DENON_BITS, irsend.capture.bits);
  EXPECT_EQ(0x1278, irsend.capture.value);
  // Normal Denon 48-bit message. (Panasonic/Kaseikyo)
  irsend.reset();
  irsend.sendDenon(0x2A4C028D6CE3, DENON_48_BITS);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(DENON, irsend.capture.decode_type);
  EXPECT_EQ(DENON_48_BITS, irsend.capture.bits);
  EXPECT_EQ(0x2A4C028D6CE3, irsend.capture.value);
}

// Test decode of Coolix messages.
TEST(TestDecode, DecodeCoolix) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendCOOLIX(0x123456);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(COOLIX, irsend.capture.decode_type);
  EXPECT_EQ(COOLIX_BITS, irsend.capture.bits);
  EXPECT_EQ(0x123456, irsend.capture.value);
}

// Test decode of Aiwa messages.
TEST(TestDecode, DecodeAiwa) {
  IRsendTest irsend(0);
  IRrecv irrecv(1);
  irsend.begin();
  irsend.reset();
  irsend.sendAiwaRCT501(0x7F);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(AIWA_RC_T501, irsend.capture.decode_type);
  EXPECT_EQ(AIWA_RC_T501_BITS, irsend.capture.bits);
  EXPECT_EQ(0x7F, irsend.capture.value);
}
