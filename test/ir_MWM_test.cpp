// Copyright 2017 David Conran
// Copyright 2018 Brett T. Warden

#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

//   MM   MM WW   WW MM   MM
//   MMM MMM WW   WW MMM MMM
//   MM M MM WW W WW MM M MM
//   MM   MM WWW WWW MM   MM
//   MM   MM WW   WW MM   MM

// Tests for sendMWM().

// Test sending simplest case data only.
TEST(TestSendMWM, SendDataOnly) {
  IRsendTest irsend(4);
  irsend.begin();

  irsend.reset();
  unsigned char test1[] = {
    0x96, 0x19, 0x10, 0x24, 0x0A, 0x6B, 0x20, 0x03, 0x82
  };
  /*
  ++--+-++--
  +-++--+++-
  +++++-+++-
  +++-++-++-
  ++-+-++++-
  +--+-+--+-
  ++++++-++-
  +--++++++-
  ++-+++++--
  */
  irsend.sendMWM(test1, sizeof(test1), 0);
  EXPECT_EQ(
      "m834s834m417s417m834s834"
      "m417s417m834s834m1251s417"
      "m2085s417m1251s417"
      "m1251s417m834s417m834s417"
      "m834s417m417s417m1668s417"
      "m417s834m417s417m417s834m417s417"
      "m2502s417m834s417"
      "m417s834m2502s417"
      "m834s417m2085s30834"
      "", irsend.outputStr());

  irsend.reset();
  unsigned char test2[] = {
    0x99, 0x26, 0x66, 0x6E, 0xD1, 0x42, 0x06, 0x20, 0xD0, 0x32, 0xF0, 0x0B
      // +-++--++--
      // ++--++-++-
      // ++--++--+-
      // ++---+--+-
      // +-+++-+---
      // ++-++++-+-
      // ++--+++++-
      // ++++++-++-
      // +++++-+---
      // ++-++--++-
      // +++++-----
      // +--+-++++-
  };
  irsend.sendMWM(test2, sizeof(test2), 0);
  EXPECT_EQ(
      "m417s417m834s834m834s834"
      "m834s834m834s417m834s417"
      "m834s834m834s834m417s417"
      "m834s1251m417s834m417s417"
      "m417s417m1251s417m417s1251"
      "m834s417m1668s417m417s417"
      "m834s834m2085s417"
      "m2502s417m834s417"
      "m2085s417m417s1251"
      "m834s417m834s834m834s417"
      "m2085s2085"
      "m417s834m417s417m1668s30417"
      "", irsend.outputStr());

}

// Tests for decodeMWM().

// Example data
TEST(TestDecodeMWM, RealExamples) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  irsend.reset();
  uint16_t green3[21] = {
      360, 364, 272, 360, 420, 248, 360, 360, 332, 308, 388, 612, 692, 696,
      636, 360, 332, 700, 300, 308, 416};
  irsend.sendRaw(green3, 21, 36000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MWM, irsend.capture.decode_type);
  EXPECT_EQ(kLasertagBits, irsend.capture.bits); // Number of bits
  EXPECT_EQ(0x53, irsend.capture.value);
  EXPECT_EQ(0x3, irsend.capture.address);  // Unit
  EXPECT_EQ(0x5, irsend.capture.command);  // Team
}

// vim: et:ts=2:sw=2
