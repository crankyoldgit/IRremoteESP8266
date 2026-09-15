// Copyright 2017 David Conran
// Copyright 2018 Brett T. Warden

#include "IRrecv.h"
#include "IRrecv_test.h"
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
  unsigned char test1[] = {0x96, 0x19, 0x10, 0x24, 0x0A,
                           0x6B, 0x20, 0x03, 0x82};
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
      "f38000d25"
      "m834s834m417s417m834s834"
      "m417s417m834s834m1251s417"
      "m2085s417m1251s417"
      "m1251s417m834s417m834s417"
      "m834s417m417s417m1668s417"
      "m417s834m417s417m417s834m417s417"
      "m2502s417m834s417"
      "m417s834m2502s417"
      "m834s417m2085s30834", irsend.outputStr());

  irsend.reset();
  unsigned char test2[] = {
      0x99, 0x26, 0x66, 0x6E, 0xD1, 0x42, 0x06,
      0x20, 0xD0, 0x32, 0xF0, 0x0B
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
      "f38000d25"
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
      "m417s834m417s417m1668s30417", irsend.outputStr());
}

// Tests for decodeMWM().

// Example data
TEST(TestDecodeMWM, RealExamples) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  irsend.reset();
  uint16_t short_code[] = {
      915,  793, 488,  366, 915,  793, 427, 366,  915, 793,  1281, 427,
      2136, 366, 1281, 366, 915,  793, 427, 854,  854, 366,  1281, 854,
      1708, 366, 488,  793, 854,  427, 427, 427,  427, 366,  854,  427,
      2563, 366, 488,  793, 2563, 366, 488, 2075, 427, 34057};
  unsigned char short_expected[] = {0x96, 0x19, 0x10, 0x36, 0x0C,
                                    0x53, 0x02, 0x03, 0xDF};
  irsend.sendRaw(short_code, sizeof(short_code) / sizeof(short_code[0]), 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MWM, irsend.capture.decode_type);
  EXPECT_EQ(8 * sizeof(short_expected) / sizeof(short_expected[0]),
            irsend.capture.bits);
  EXPECT_STATE_EQ(short_expected, irsend.capture.state, irsend.capture.bits);

  irsend.reset();
  uint16_t long_code[] = {
      427,  427,  854,  854,  854, 793,  915,  793, 854,  366,  915, 366, 854,
      854,  854,  793,  488,  366, 915,  1220, 427, 793,  488,  366, 488, 366,
      1281, 427,  427,  1220, 915, 366,  1708, 366, 488,  366,  854, 854, 2136,
      366,  2563, 366,  854,  427, 2136, 366,  488, 1220, 854,  427, 854, 793,
      915,  366,  2136, 2075, 427, 793,  488,  366, 1708, 30517};
  unsigned char long_expected[] = {0x99, 0x26, 0x66, 0x6E, 0xD1, 0x42,
                                   0x06, 0x20, 0xD0, 0x32, 0xF0, 0x0B};
  irsend.sendRaw(long_code, sizeof(long_code) / sizeof(long_code[0]), 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MWM, irsend.capture.decode_type);
  EXPECT_EQ(8 * sizeof(long_expected) / sizeof(long_expected[0]),
            irsend.capture.bits);
  EXPECT_STATE_EQ(long_expected, irsend.capture.state, irsend.capture.bits);
}

// Regression test for crankyoldgit/IRremoteESP8266#2289.
// Real-world MWM captures (via Tasmota IRsend raw) that lacked the trailing
// stop-bit entry used to decode to only 64 bits; the 9th byte was silently
// dropped. Both the full (43-entry, stop bit present) and truncated
// (41-entry, stop bit removed) versions must now decode to the same 72-bit
// message.
TEST(TestDecodeMWM, MissingStopBit) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  unsigned char expected[] = {0x96, 0x19, 0x07, 0x09, 0x0E, 0xDE,
                              0xC0, 0x04, 0x39};
  // Issue's Test 1: 43 entries ending with the stop bit.
  uint16_t with_stop[] = {
      872, 824, 474, 374, 858, 832, 472, 380, 826, 858, 1284, 404, 420, 1264,
      2124, 396, 468, 378, 880, 386, 1720, 386, 880, 1220, 1694, 426, 828,
      1696, 456, 1234, 2986, 1216, 1228, 464, 2148, 382, 456, 390, 832, 1272,
      892, 402, 1022};
  // Issue's Test 2: 41 entries, trailing stop-bit entry removed.
  uint16_t no_stop[] = {
      872, 824, 472, 378, 854, 832, 474, 378, 826, 858, 1286, 400, 420, 1268,
      2126, 392, 474, 374, 884, 388, 1716, 386, 880, 1224, 1690, 426, 832,
      1692, 456, 1232, 2986, 1220, 1254, 426, 2154, 384, 472, 378, 796, 1308,
      876};

  irsend.sendRaw(with_stop, sizeof(with_stop) / sizeof(with_stop[0]), 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MWM, irsend.capture.decode_type);
  EXPECT_EQ(8 * sizeof(expected) / sizeof(expected[0]), irsend.capture.bits);
  EXPECT_STATE_EQ(expected, irsend.capture.state, irsend.capture.bits);

  irsend.reset();
  irsend.sendRaw(no_stop, sizeof(no_stop) / sizeof(no_stop[0]), 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MWM, irsend.capture.decode_type);
  EXPECT_EQ(8 * sizeof(expected) / sizeof(expected[0]), irsend.capture.bits);
  EXPECT_STATE_EQ(expected, irsend.capture.state, irsend.capture.bits);
}

// Test that a trailing stop bit cut off by raw buffer overflow doesn't drop
// the last byte. The decoder stores each byte only inside case 9 (stop bit)
// inside IRrecv::decodeMWM. If the buffer ends before the stop bit is read,
// the last byte is silently lost.
//
// Uses sendMWM then truncates rawlen by 1.  Because sendMWM merges the
// stop-bit space with the trailing gap into one raw entry, removing that
// entry drops the stop bit while preserving every data bit.  The last byte
// (0x00) has bit 7 = 0 so the stop bit is NOT merged into a data-bit entry.
TEST(TestDecodeMWM, TrailingStopBit) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  // 9-byte MWM command: header 0x96 (payload_length=6, total=9 bytes),
  // eight data bytes of 0x00.
  unsigned char code[] = {0x96, 0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00};
  uint16_t nbytes = 9;
  uint16_t nbits = nbytes * 8;  // 72

  // --- Full message: should decode cleanly. ---
  irsend.reset();
  irsend.sendMWM(code, nbytes, 0);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MWM, irsend.capture.decode_type);
  EXPECT_EQ(nbits, irsend.capture.bits);
  EXPECT_STATE_EQ(code, irsend.capture.state, nbits);

  // --- Truncated: remove last raw entry (stop bit + trailing gap). ---
  // For byte 0x00 the stop-bit space is NOT merged into data bits (bit 7 = 0
  // means a long mark precedes the stop bit).  sendMWM merges the stop-bit
  // space with the trailing gap into one space entry.  Removing it drops
  // the stop bit while all 8 data bits remain in the preceding mark entry.
  irsend.reset();
  irsend.sendMWM(code, nbytes, 0);
  irsend.makeDecodeResult();
  ASSERT_GE(irsend.capture.rawlen, 4);  // at least header + a few entries
  irsend.capture.rawlen--;
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MWM, irsend.capture.decode_type);
  EXPECT_EQ(nbits, irsend.capture.bits);
  EXPECT_STATE_EQ(code, irsend.capture.state, nbits);
}

// vim: et:ts=2:sw=2
