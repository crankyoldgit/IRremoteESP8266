// Copyright 2020 David Conran

#include "IRac.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

// Tests for sendSymphony().

// Test sending typical data only.
TEST(TestSendSymphony, SendDataOnly) {
  IRsendTest irsend(kGpioUnused);
  irsend.begin();
  irsend.sendSymphony(0x137);
  EXPECT_EQ(
      "f38000d50"
      "m1250s400m1250s400m400s1250m1250s400m1250s400m400s1250m400s1250m1250s400"
      "m400s1250m400s1250m400s1250"
      "m400s8000"
      "m1250s400m1250s400m400s1250m1250s400m1250s400m400s1250m400s1250m1250s400"
      "m400s1250m400s1250m400s1250"
      "m400s8000",
      irsend.outputStr());
}

// Tests for decodeSymphony().

// Decode normal Symphony messages.
TEST(TestDecodeSymphony, SyntheticSelfDecode) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  // Real-life Symphony code from an actual capture/decode.
  irsend.reset();
  irsend.sendSymphony(0x123);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(decode_type_t::SYMPHONY, irsend.capture.decode_type);
  EXPECT_EQ(kSymphonyBits, irsend.capture.bits);
  EXPECT_EQ(0x123, irsend.capture.value);
  EXPECT_EQ(0, irsend.capture.address);
  EXPECT_EQ(0, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);
}

// Decode a real Symphony message.
TEST(TestDecodeSymphony, RealMessageDecode) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  // Real-life Symphony code from an actual capture/decode.
  // Ref: https://github.com/crankyoldgit/IRremoteESP8266/issues/1057#issue-577216614
  irsend.reset();
  const uint16_t button_1[95] = {
      1296, 412, 1294, 386, 420, 1224, 1322, 390, 1290, 390, 420, 1224, 452,
      1220, 1314, 394, 420, 1222, 482, 1190, 480, 1192, 452, 7960,
      1290, 420, 1290, 390, 418, 1226, 1318, 394, 1262, 416, 420, 1224, 454,
      1220, 1292, 416, 422, 1222, 450, 1222, 452, 1218, 454, 8208,
      1296, 414, 1292, 386, 418, 1226, 1292, 422, 1260, 420, 424, 1218, 454,
      1226, 1312, 390, 420, 1224, 454, 1220, 482, 1186, 454, 7960,
      1318, 392, 1264, 416, 392, 1252, 1318, 394, 1288, 394, 418, 1224, 452,
      1224, 1292, 422, 414, 1222, 458, 1214, 450, 1222, 454};
  irsend.sendRaw(button_1, 95, 38000);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(decode_type_t::SYMPHONY, irsend.capture.decode_type);
  EXPECT_EQ(kSymphonyBits, irsend.capture.bits);
  EXPECT_EQ(0x137, irsend.capture.value);
  EXPECT_EQ(0x0, irsend.capture.address);
  EXPECT_EQ(0x0, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);

  // Ref: https://github.com/crankyoldgit/IRremoteESP8266/issues/1057#issuecomment-596038442
  irsend.reset();
  const uint16_t power[23] = {
      1308, 368, 1310, 368, 448, 1222, 1310, 372, 1308, 400, 442, 1198, 472,
      1198, 1284, 396, 444, 1224, 470, 1200, 470, 1198, 472};
  irsend.sendRaw(power, 23, 38000);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(decode_type_t::SYMPHONY, irsend.capture.decode_type);
  EXPECT_EQ(kSymphonyBits, irsend.capture.bits);
  EXPECT_EQ(0x137, irsend.capture.value);
  EXPECT_EQ(0x0, irsend.capture.address);
  EXPECT_EQ(0x0, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);

  irsend.reset();
  const uint16_t swing[23] = {
      1290, 418, 1286, 392, 422, 1248, 1284, 400, 1294, 386, 422, 1248, 422,
      1250, 444, 1228, 424, 1248, 446, 1226, 1258, 420, 446};
  irsend.sendRaw(swing, 23, 38000);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(decode_type_t::SYMPHONY, irsend.capture.decode_type);
  EXPECT_EQ(kSymphonyBits, irsend.capture.bits);
  EXPECT_EQ(0x13E, irsend.capture.value);
  EXPECT_EQ(0x0, irsend.capture.address);
  EXPECT_EQ(0x0, irsend.capture.command);
  EXPECT_FALSE(irsend.capture.repeat);
}


TEST(TestUtils, Housekeeping) {
  ASSERT_EQ("SYMPHONY", typeToString(decode_type_t::SYMPHONY));
  ASSERT_EQ(decode_type_t::SYMPHONY, strToDecodeType("SYMPHONY"));
  ASSERT_FALSE(hasACState(decode_type_t::SYMPHONY));
  ASSERT_FALSE(IRac::isProtocolSupported(decode_type_t::SYMPHONY));
  ASSERT_EQ(kSymphonyBits, IRsendTest::defaultBits(decode_type_t::SYMPHONY));
  ASSERT_EQ(kSymphonyDefaultRepeat,
            IRsendTest::minRepeats(decode_type_t::SYMPHONY));
}
