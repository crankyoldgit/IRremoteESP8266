// Copyright 2017 David Conran
// Copyright 2018 Brett T. Warden

#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

//   LL        AAA    SSSSS  EEEEEEE RRRRRR  TTTTTTT   AAA     GGGG
//   LL       AAAAA  SS      EE      RR   RR   TTT    AAAAA   GG  GG
//   LL      AA   AA  SSSSS  EEEEE   RRRRRR    TTT   AA   AA GG
//   LL      AAAAAAA      SS EE      RR  RR    TTT   AAAAAAA GG   GG
//   LLLLLLL AA   AA  SSSSS  EEEEEEE RR   RR   TTT   AA   AA  GGGGGG


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
      "m834s417m2085s20834"
      "", irsend.outputStr());

  irsend.reset();
  unsigned char test2[] = {
	  0x96, 0x19, 0x0B, 0x36, 0x06, 0xD4, 0x30, 0x01, 0x7F
  };
  irsend.sendMWM(test2, sizeof(test2), 0);
  EXPECT_EQ("FOO", irsend.outputStr());

  irsend.reset();
  unsigned char test3[] = {
	  0x90, 0x24, 0x58, 0x03, 0x48, 0x0D, 0xD0, 0x3E, 0x32, 0x66, 0x6E, 0xBE
  };
  irsend.sendMWM(test3, sizeof(test3), 0);
  EXPECT_EQ("FOO", irsend.outputStr());

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
