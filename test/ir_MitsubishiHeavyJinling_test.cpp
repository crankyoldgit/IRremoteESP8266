// Copyright 2026 bluebird
// Unit tests for Mitsubishi Heavy Jinling protocol.
// Captured frames from RYD502A003B remote (see 2262-dumps-2026-06-16.md)

#include "IRac.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

// Cool 18°C Low: FF 00 87 78 E6 19 2A D5
// NOTE: matchGeneric(MSBfirst=true) reads bits LSB-reversed per byte
// compared to the aircon firmware's bit extraction.
// Raw capture matchGeneric read = FF00E11E679854AB
TEST(TestDecodeMitsubishiHeavyJinling, RealCool18) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  const uint16_t rawData[133] = {
      5954,7476,508,3454,508,3456,508,3454,514,3448,508,3454,508,3454,
      508,3456,508,3478,508,1496,510,1496,508,1496,508,1496,508,1496,
      508,1496,508,1496,508,1518,508,3456,508,3454,508,3456,508,1496,
      508,1496,508,1496,508,1496,508,3476,508,1496,508,1496,508,1496,
      508,3454,508,3456,508,3456,508,3454,510,1518,508,1496,508,3456,
      508,3454,508,1496,508,1496,508,3454,508,3454,508,3478,508,3456,
      508,1496,508,1498,508,3456,508,3454,508,1496,508,1498,508,1518,
      510,1496,508,3456,512,1494,508,3456,506,1498,508,3456,508,1498,
      508,1520,506,3456,508,1496,508,3456,508,1496,508,3456,508,1496,
      508,3456,508,3462,508,7422,534};
  irsend.reset();
  irsend.sendRaw(rawData, 133, 38);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(MITSUBISHI_HEAVY_JINLING, irsend.capture.decode_type);
  ASSERT_EQ(64, irsend.capture.bits);
  EXPECT_EQ(0xFF00E11E679854ABULL, irsend.capture.value);
}

// Cool 24°C Low: FF 00 97 68 86 79 2A D5
// matchGeneric read: FF00E916619E54AB
TEST(TestDecodeMitsubishiHeavyJinling, RealCool24) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  const uint16_t rawData[133] = {
      5950,7476,508,3456,508,3456,508,3454,508,3454,508,3456,508,3454,
      508,3456,508,3478,508,1496,508,1496,508,1496,508,1496,508,1496,
      508,1496,508,1496,508,1520,508,3456,508,3456,508,3456,508,1498,
      508,3456,508,1496,510,1496,508,3478,508,1496,508,1498,506,1496,
      508,3454,508,1496,508,3456,508,3456,508,1518,508,1496,506,3456,
      506,3456,508,1496,508,1498,508,1496,508,1498,508,3478,506,3456,
      508,1498,506,1498,508,3456,508,3456,506,3456,508,3456,508,1520,
      508,1496,508,3456,508,1498,506,3456,508,1496,506,3456,508,1498,
      506,1520,506,3456,508,1496,508,3456,508,1496,508,3456,506,1498,
      508,3456,506,3464,506,7424,534};
  irsend.reset();
  irsend.sendRaw(rawData, 133, 38);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(MITSUBISHI_HEAVY_JINLING, irsend.capture.decode_type);
  ASSERT_EQ(64, irsend.capture.bits);
  EXPECT_EQ(0xFF00E916619E54ABULL, irsend.capture.value);
}

// Cool 30°C Low: FF 00 87 78 26 D9 2A D5
TEST(TestDecodeMitsubishiHeavyJinling, RealCool30) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  const uint16_t rawData[133] = {
      5954,7474,510,3454,508,3454,508,3454,510,3454,508,3454,510,3454,
      508,3454,508,3478,508,1496,510,1494,508,1496,508,1496,510,1494,
      510,1496,510,1494,510,1518,508,3454,510,3454,508,3454,508,1496,
      508,1496,508,1496,508,1496,508,3478,508,1496,510,1496,508,1496,
      508,3456,508,3456,508,3456,508,3454,508,1518,508,1496,508,3454,
      510,3454,508,1496,508,1496,508,3454,510,1494,510,1518,508,3456,
      508,1496,508,1496,510,3454,508,3454,508,1496,508,3456,508,3478,
      508,1496,510,3454,508,1496,510,3454,508,1496,508,3456,508,1496,
      508,1518,510,3454,508,1496,508,3454,508,1496,508,3454,508,1496,
      508,3454,508,3462,508,7422,536};
  irsend.reset();
  irsend.sendRaw(rawData, 133, 38);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(MITSUBISHI_HEAVY_JINLING, irsend.capture.decode_type);
  ASSERT_EQ(64, irsend.capture.bits);
  EXPECT_EQ(0xFF00E11E649B54ABULL, irsend.capture.value);
}

// OFF: FF 00 87 78 2E D1 2A D5
TEST(TestDecodeMitsubishiHeavyJinling, RealOff) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  const uint16_t rawData[133] = {
      5954,7476,508,3456,508,3456,508,3454,508,3454,508,3456,508,3456,
      508,3456,508,3478,508,1496,508,1496,508,1496,508,1496,508,1496,
      508,1496,508,1496,508,1518,508,3456,508,3456,508,3456,508,1496,
      508,1496,508,1496,508,1498,508,3478,508,1496,508,1496,508,1496,
      508,3456,508,3456,508,3456,508,3456,508,1520,508,1496,508,3456,
      508,3456,506,3456,508,1498,508,3456,508,1496,508,1520,508,3456,
      508,1498,508,1496,508,1496,508,3456,508,1496,508,3456,508,3478,
      508,1498,506,3456,508,1496,508,3456,506,1498,506,3456,508,1498,
      506,1520,508,3456,508,1496,508,3456,506,1498,508,3456,508,1496,
      508,3456,506,3464,506,7424,532};
  irsend.reset();
  irsend.sendRaw(rawData, 133, 38);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(MITSUBISHI_HEAVY_JINLING, irsend.capture.decode_type);
  ASSERT_EQ(64, irsend.capture.bits);
  EXPECT_EQ(0xFF00E11E748B54ABULL, irsend.capture.value);
}

// Round-trip: send own protocol → decode → verify value
TEST(TestDecodeMitsubishiHeavyJinling, RoundTrip) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  irsend.reset();
  irsend.sendMitsubishiHeavyJinling(0xFF008778E6192AD5ULL);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(MITSUBISHI_HEAVY_JINLING, irsend.capture.decode_type);
  ASSERT_EQ(64, irsend.capture.bits);
  EXPECT_EQ(0xFF008778E6192AD5ULL, irsend.capture.value);
}
