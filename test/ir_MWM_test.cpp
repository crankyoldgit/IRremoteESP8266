// Copyright 2017 David Conran
// Copyright 2018 Brett T. Warden

#include <string.h>

#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

//   MM   MM WW   WW MM   MM
//   MMM MMM WW   WW MMM MMM
//   MM M MM WW W WW MM   MM
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
// dropped. Pins the fix's behaviour: both the full (43-entry, stop bit
// present) and truncated (41-entry, stop bit removed) versions must now decode
// to the same 72-bit message.
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

// Tests for multiple MWM messages arriving in a single raw capture.
//
// Real MWM (Disney Made With Magic) remotes transmit each command as an
// A-B-A' bundle: a 9-byte command, a 12-15 byte status frame, then a repeated
// copy of the command, with ~21-40ms gaps between the three messages. Because
// that gap is shorter than the receiver's default kTimeout (50ms in DECODE_AC
// builds), the whole bundle lands in ONE capture buffer. The pre-fix library
// only decoded (and delivered) the first message; the follow-ups were
// unreachable and were silently discarded the moment resume() cleared the
// buffer.
//
// IRrecv::decodeMWM() now returns ALL complete messages in the capture,
// concatenated into a single result. Each message is self-describing (its
// header byte declares its length), so a caller can split the bundle apart
// again:
//   const uint16_t length = (result->state[0] & 0x0f) + 3;
// and so on. Trailing truncated messages (e.g. A' cut off at RF level) are
// dropped rather than mis-decoded.

// Single message: no bundle, decode() returns the message as-is.
TEST(TestDecodeMWMBundle, SingleMessage) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  irsend.reset();
  uint16_t short_code[] = {
      915,  793, 488,  366, 915,  793, 427, 366,  915, 793,  1281, 427,
      2136, 366, 1281, 366, 915,  793, 427, 854,  854, 366,  1281, 854,
      1708, 366, 488,  793, 854,  427, 427, 427,  427, 366,  854,  427,
      2563, 366, 488,  793, 2563, 366, 488, 2075, 427, 34057};
  unsigned char expected[] = {0x96, 0x19, 0x10, 0x36, 0x0C,
                              0x53, 0x02, 0x03, 0xDF};
  irsend.sendRaw(short_code, sizeof(short_code) / sizeof(short_code[0]), 38000);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MWM, irsend.capture.decode_type);
  EXPECT_EQ(sizeof(expected) * 8, irsend.capture.bits);
  EXPECT_STATE_EQ(expected, irsend.capture.state, irsend.capture.bits);
}

// Three identical messages, separated by ~33ms gaps, all inside one raw
// buffer: all three are returned concatenated by a single decode().
TEST(TestDecodeMWMBundle, TripleMessage) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  // The 9-byte command; note the message ends in a mark (bit 7 of 0xDF is
  // set) so the following space is the merged stop-bit + inter-message gap.
  uint16_t msg1[] = {
      915,  793, 488,  366, 915,  793, 427, 366,  915, 793,  1281, 427,
      2136, 366, 1281, 366, 915,  793, 427, 854,  854, 366,  1281, 854,
      1708, 366, 488,  793, 854,  427, 427, 427,  427, 366,  854,  427,
      2563, 366, 488,  793, 2563, 366, 488, 2075, 427};
  unsigned char expected[] = {0x96, 0x19, 0x10, 0x36, 0x0C,
                              0x53, 0x02, 0x03, 0xDF};

  irsend.reset();
  // Build a raw buffer with 3 messages separated by ~33ms gaps (i.e. always
  // inside one capture timout window).
  uint16_t bundle[200];
  uint16_t pos = 0;
  for (int m = 0; m < 3; m++) {
    for (uint16_t i = 0; i < sizeof(msg1) / sizeof(msg1[0]); i++)
      bundle[pos++] = msg1[i];
    // Inter-message gap (~33ms = 33000 / kRawTick raw units).
    bundle[pos++] = 16500;
  }
  bundle[pos++] = 16500;  // Trailing gap.

  irsend.sendRaw(bundle, pos, 38000);
  irsend.makeDecodeResult();

  unsigned char all[27];
  for (int m = 0; m < 3; m++)
    for (uint8_t i = 0; i < sizeof(expected); i++)
      all[m * sizeof(expected) + i] = expected[i];

  // A single decode() returns all three messages concatenated.
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MWM, irsend.capture.decode_type);
  EXPECT_EQ(sizeof(all) * 8, irsend.capture.bits);
  EXPECT_STATE_EQ(all, irsend.capture.state, irsend.capture.bits);
}

// Regression test for a real MWM bundle captured by an ESP8266 running stock
// v2.9.0 IRrecvDumpV3. The wand transmits an A-B-A' triple inside a single
// raw capture (below), separated by ~33ms & ~38ms gaps:
//   msg A  @ rawbuf offset   1: 9 bytes 0x961908121435440041
//   msg B  @ rawbuf offset  51: 15 bytes 0x9C260C35626AD032F0D241040A20AA
//   msg A' @ rawbuf offset 133: 9 bytes 0x961908121435440041 (== msg A)
//
// Behaviour of the pre-fix library (master@afd43878, verified by replaying
// this exact capture through it):
//   - decode() always returned only message A; repeated calls re-scanned from
//     offset 1 & returned A again. Messages B/A' were unreachable via any
//     public API.
//   - decodeMWM() pointed directly at message A' returned only 64 bits
//     (0x9619081214354400): the capture ends before A''s trailing stop-bit
//     space, so the final data byte was silently dropped.
TEST(TestDecodeMWMBundle, RealCaptureBundle) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  irsend.reset();
  // Real 181-entry raw capture of one button press (3 messages).
  uint16_t rawData[181] = {
      926,  762, 468,  374, 914,  776, 498,  348, 856,  828, 1316, 378,
      1770, 336, 1774, 348, 852,  408, 852,  404, 1284, 406, 1284, 406,
      502,  348, 1314, 380, 502,  352, 498,  348, 450,  812, 934,  340,
      1316, 370, 1340, 336, 498,  348, 3878, 332, 502,  348, 2206, 328,
      450,  33318,      1376, 1164, 940,  750, 856,  842, 912,  366,
      860,  418, 1346, 750, 1748, 380, 480,  366, 506,  344, 506,  754,
      914,  370, 862,  400, 1320, 794, 502,  348, 908,  370, 482,  370,
      476,  790, 480,  366, 2206, 330, 480,  1218, 856,  406, 914,  784,
      856,  406, 2210, 2012,      888,  382, 940,  340, 450,  1244, 450,
      400,  2150,      382,  482,  370, 1288, 392,  2212, 334, 888,  392,
      454,  398, 1774, 330, 2482, 462, 888,  392, 856,  410, 506,  336,
      508,  340, 502,  38460,      944,  750, 450,  396, 940,  750, 454,
      396,  914, 784,  1316, 378,  1780, 338, 1744, 380, 846,  414, 800,
      462,  1232, 466, 1284, 410,  528,  318, 1288, 410, 468,  380, 498,
      358,  494, 764,  962,  322,  1340, 354, 1336, 336, 502,  358, 3874,
      348,  508, 348,  2206, 336,  472};
  // Message A (command): header 0x96, payload_length 6 -> 9 bytes total.
  unsigned char msg_a[] = {0x96, 0x19, 0x08, 0x12, 0x14,
                           0x35, 0x44, 0x00, 0x41};
  // Message B (status): header 0x9C, payload_length 12 -> 15 bytes total.
  unsigned char msg_b[] = {0x9C, 0x26, 0x0C, 0x35, 0x62, 0x6A, 0xD0,
                           0x32, 0xF0, 0xD2, 0x41, 0x04, 0x0A, 0x20,
                           0xAA};
  // Message A' is an identical retransmission of A.
  uint8_t expected[sizeof(msg_a) + sizeof(msg_b) + sizeof(msg_a)];
  memcpy(expected, msg_a, sizeof(msg_a));
  memcpy(expected + sizeof(msg_a), msg_b, sizeof(msg_b));
  memcpy(expected + sizeof(msg_a) + sizeof(msg_b), msg_a, sizeof(msg_a));

  irsend.sendRaw(rawData, 181, 38000);
  irsend.makeDecodeResult();

  // A single decode() returns the whole A-B-A' bundle.
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MWM, irsend.capture.decode_type);
  EXPECT_EQ(sizeof(expected) * 8, irsend.capture.bits);
  EXPECT_STATE_EQ(expected, irsend.capture.state, irsend.capture.bits);
}


// Button press #1 (162 entries): command + status + command-repeat
// triple, extracted verbatim from lirc mode2 capture 'mode2-capture.log',
// lines 6279-6440. Spaces >= 65535us (inter-press idles) are
// clamped to 65000; they only need to read as 'long'.
TEST(TestDecodeMWMBundle, Mode2LogBundle1) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  const uint16_t rawData[162] = {
      854, 854, 427, 366, 915, 793, 427, 366, 915, 793, 1281, 366,
      488, 793, 488, 366, 1708, 366, 488, 366, 854, 366, 1769, 366,
      1708, 427, 1647, 427, 1281, 427, 1708, 793, 1708, 854, 1281, 366,
      488, 366, 2928, 427, 488, 366, 427, 854, 854, 366, 488, 33062,
      488, 366, 854, 854, 854, 793, 1281, 366, 915, 366, 915, 305,
      1708, 854, 488, 366, 427, 427, 1281, 366, 2135, 427, 1708, 366,
      915, 366, 427, 366, 488, 366, 488, 793, 1708, 427, 2135, 366,
      427, 1281, 854, 2074, 854, 427, 854, 366, 915, 793, 854, 366,
      488, 427, 1708, 793, 427, 427, 427, 366, 915, 366, 488, 793,
      427, 427, 427, 793, 2135, 30988, 854, 793, 488, 366, 854, 854,
      427, 366, 915, 793, 1281, 427, 427, 793, 488, 366, 1647, 427,
      488, 366, 915, 366, 1647, 427, 1708, 427, 1708, 366, 1281, 427,
      1647, 854, 1708, 793, 1281, 427, 427, 366, 2989, 366, 488, 366,
      488, 793, 854, 427, 427, 65000,
  };
  // Messages 1 & 3: 72-bit command. Message 2: status frame.
  const uint8_t cmd[] = {0x96, 0x19, 0x0B, 0x09, 0x08, 0x84, 0x18, 0x01, 0x4D};
  const uint8_t status[] = {0x99, 0x24, 0x58, 0x04, 0x48, 0x0D, 0xD0, 0x3E,
                            0x32, 0x61, 0x69, 0x83};
  uint8_t expected[sizeof(cmd) + sizeof(status) + sizeof(cmd)];
  memcpy(expected, cmd, sizeof(cmd));
  memcpy(expected + sizeof(cmd), status, sizeof(status));
  memcpy(expected + sizeof(cmd) + sizeof(status), cmd, sizeof(cmd));

  irsend.reset();
  irsend.sendRaw(rawData, 162, 38000);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MWM, irsend.capture.decode_type);
  EXPECT_EQ(sizeof(expected) * 8, irsend.capture.bits);
  EXPECT_STATE_EQ(expected, irsend.capture.state, irsend.capture.bits);
}

// Button press #2 (184 entries): command + status + command-repeat
// triple, extracted verbatim from lirc mode2 capture 'mode2-capture.log',
// lines 6442-6625. Spaces >= 65535us (inter-press idles) are
// clamped to 65000; they only need to read as 'long'.
TEST(TestDecodeMWMBundle, Mode2LogBundle2) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  const uint16_t rawData[184] = {
      854, 854, 427, 427, 854, 793, 427, 427, 854, 793, 1281, 427,
      427, 427, 427, 854, 1647, 427, 793, 488, 854, 366, 1281, 427,
      854, 1220, 427, 1647, 488, 793, 488, 793, 1220, 427, 1708, 793,
      1342, 366, 427, 427, 1281, 427, 1220, 427, 427, 427, 793, 854,
      427, 427, 488, 33184, 427, 793, 488, 793, 915, 793, 1220, 427,
      915, 366, 915, 366, 793, 427, 1281, 854, 427, 427, 793, 427,
      427, 427, 488, 793, 427, 427, 2074, 427, 427, 1220, 854, 427,
      793, 854, 854, 427, 427, 2074, 488, 1220, 854, 366, 427, 854,
      427, 427, 427, 427, 854, 366, 427, 427, 1647, 427, 1708, 427,
      1647, 427, 427, 1281, 427, 1220, 427, 427, 1647, 427, 854, 427,
      427, 427, 427, 366, 1281, 427, 854, 793, 427, 854, 1220, 33489,
      854, 854, 366, 427, 854, 793, 488, 366, 854, 854, 1281, 366,
      427, 427, 427, 854, 1708, 366, 854, 427, 854, 427, 1220, 427,
      854, 1281, 366, 1708, 427, 854, 427, 793, 1281, 427, 1647, 854,
      1220, 488, 427, 366, 1220, 488, 1220, 427, 427, 427, 854, 793,
      488, 366, 427, 65000,
  };
  // Messages 1 & 3: 72-bit command. Message 2: status frame.
  const uint8_t cmd[] = {0x96, 0x19, 0x0D, 0x12, 0xEE, 0x1B, 0x18, 0x11, 0x59};
  const uint8_t status[] = {0x9B, 0x24, 0x62, 0x6A, 0xD0, 0x32, 0xDF, 0x5A,
                            0x0A, 0x08, 0x77, 0x48, 0x91, 0xE3};
  uint8_t expected[sizeof(cmd) + sizeof(status) + sizeof(cmd)];
  memcpy(expected, cmd, sizeof(cmd));
  memcpy(expected + sizeof(cmd), status, sizeof(status));
  memcpy(expected + sizeof(cmd) + sizeof(status), cmd, sizeof(cmd));

  irsend.reset();
  irsend.sendRaw(rawData, 184, 38000);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MWM, irsend.capture.decode_type);
  EXPECT_EQ(sizeof(expected) * 8, irsend.capture.bits);
  EXPECT_STATE_EQ(expected, irsend.capture.state, irsend.capture.bits);
}

// Button press #3 (172 entries): command + status + command-repeat
// triple, extracted verbatim from lirc mode2 capture 'mode2-capture.log',
// lines 6627-6798. Spaces >= 65535us (inter-press idles) are
// clamped to 65000; they only need to read as 'long'.
TEST(TestDecodeMWMBundle, Mode2LogBundle3) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  const uint16_t rawData[172] = {
      915, 793, 427, 427, 854, 793, 488, 366, 854, 854, 1220, 427,
      854, 1281, 1647, 427, 427, 854, 427, 793, 1342, 366, 1281, 793,
      1769, 366, 1281, 427, 427, 366, 488, 1220, 1708, 793, 1342, 366,
      1281, 366, 2196, 366, 427, 793, 915, 366, 1281, 33245, 1342, 1220,
      854, 793, 915, 793, 854, 366, 915, 366, 1281, 854, 1647, 427,
      488, 366, 427, 427, 427, 366, 488, 1220, 427, 854, 1220, 854,
      488, 366, 427, 854, 427, 366, 488, 793, 427, 427, 1708, 793,
      488, 366, 427, 427, 854, 1220, 488, 1647, 1647, 427, 854, 427,
      427, 427, 427, 793, 2562, 427, 427, 427, 1220, 427, 427, 1281,
      1281, 1647, 854, 427, 427, 1220, 2135, 427, 854, 793, 2135, 427,
      427, 366, 488, 366, 1708, 38491, 854, 854, 427, 427, 854, 793,
      488, 366, 915, 793, 1220, 427, 854, 1281, 1708, 366, 488, 793,
      427, 793, 1281, 427, 1281, 793, 1708, 427, 1281, 427, 427, 366,
      488, 1220, 1708, 793, 1281, 427, 1281, 366, 2135, 427, 427, 793,
      915, 366, 1281, 65000,
  };
  // Messages 1 & 3: 72-bit command. Message 2: status frame.
  const uint8_t cmd[] = {0x96, 0x19, 0x0E, 0x1B, 0x0C, 0xD4, 0x18, 0x04, 0x13};
  const uint8_t status[] = {0x9C, 0x26, 0x0C, 0xD5, 0x63, 0x6B, 0x58, 0xEE,
                            0x48, 0x03, 0xD1, 0x3C, 0x07, 0x06, 0x85};
  uint8_t expected[sizeof(cmd) + sizeof(status) + sizeof(cmd)];
  memcpy(expected, cmd, sizeof(cmd));
  memcpy(expected + sizeof(cmd), status, sizeof(status));
  memcpy(expected + sizeof(cmd) + sizeof(status), cmd, sizeof(cmd));

  irsend.reset();
  irsend.sendRaw(rawData, 172, 38000);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MWM, irsend.capture.decode_type);
  EXPECT_EQ(sizeof(expected) * 8, irsend.capture.bits);
  EXPECT_STATE_EQ(expected, irsend.capture.state, irsend.capture.bits);
}

// Button press #4 (154 entries): command + status + command-repeat
// triple, extracted verbatim from lirc mode2 capture 'mode2-capture.log',
// lines 6800-6953. Spaces >= 65535us (inter-press idles) are
// clamped to 65000; they only need to read as 'long'.
TEST(TestDecodeMWMBundle, Mode2LogBundle4) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  const uint16_t rawData[154] = {
      854, 854, 427, 427, 854, 793, 427, 427, 793, 854, 1281, 427,
      2074, 427, 1281, 427, 1281, 366, 854, 427, 854, 427, 793, 1281,
      1708, 427, 427, 366, 427, 854, 1647, 488, 1647, 854, 1220, 488,
      427, 793, 2501, 488, 427, 793, 1281, 427, 366, 33672, 366, 427,
      854, 854, 793, 854, 854, 854, 793, 427, 854, 427, 1220, 488,
      793, 854, 427, 427, 1220, 854, 427, 854, 366, 488, 366, 427,
      1220, 488, 427, 1220, 854, 427, 1647, 488, 366, 427, 427, 1281,
      2074, 427, 2501, 427, 854, 427, 2074, 427, 427, 1220, 854, 488,
      793, 854, 793, 488, 2074, 2135, 366, 488, 1220, 427, 1220, 30683,
      793, 854, 427, 427, 854, 854, 427, 366, 854, 854, 1220, 488,
      2074, 427, 1220, 427, 1220, 488, 793, 427, 854, 427, 854, 1220,
      1708, 427, 427, 427, 366, 854, 1647, 488, 1647, 854, 1220, 427,
      427, 854, 2501, 427, 427, 854, 1220, 427, 427, 65000,
  };
  // Messages 1 & 3: 72-bit command. Message 2: status frame.
  const uint8_t cmd[] = {0x96, 0x19, 0x10, 0x24, 0x0E, 0x0D, 0x18, 0x03, 0xA3};
  const uint8_t status[] = {0x99, 0x26, 0x64, 0x6C, 0xD1, 0x42, 0x07, 0x20,
                            0xD0, 0x32, 0xF0, 0x11};
  uint8_t expected[sizeof(cmd) + sizeof(status) + sizeof(cmd)];
  memcpy(expected, cmd, sizeof(cmd));
  memcpy(expected + sizeof(cmd), status, sizeof(status));
  memcpy(expected + sizeof(cmd) + sizeof(status), cmd, sizeof(cmd));

  irsend.reset();
  irsend.sendRaw(rawData, 154, 38000);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MWM, irsend.capture.decode_type);
  EXPECT_EQ(sizeof(expected) * 8, irsend.capture.bits);
  EXPECT_STATE_EQ(expected, irsend.capture.state, irsend.capture.bits);
}

// Button press #5 (142 entries): command + status + command-repeat
// triple, extracted verbatim from lirc mode2 capture 'mode2-capture.log',
// lines 6955-7096. Spaces >= 65535us (inter-press idles) are
// clamped to 65000; they only need to read as 'long'.
TEST(TestDecodeMWMBundle, Mode2LogBundle5) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  const uint16_t rawData[142] = {
      915, 793, 488, 366, 854, 793, 488, 366, 854, 854, 1220, 427,
      488, 366, 2989, 366, 3416, 793, 2135, 366, 1281, 427, 427, 1647,
      915, 1220, 1647, 854, 1281, 427, 1708, 366, 1708, 427, 854, 427,
      854, 34831, 854, 427, 427, 793, 854, 793, 854, 854, 915, 366,
      793, 488, 1220, 854, 1647, 427, 427, 1708, 854, 1220, 1708, 427,
      793, 488, 427, 366, 488, 793, 2135, 793, 2135, 427, 427, 1220,
      427, 427, 427, 427, 1281, 366, 488, 366, 427, 2135, 427, 427,
      427, 366, 2135, 427, 427, 1220, 854, 427, 1708, 366, 427, 427,
      1708, 427, 1708, 366, 427, 854, 2501, 36417, 915, 793, 427, 427,
      854, 793, 427, 427, 854, 854, 1220, 427, 427, 427, 2928, 427,
      3416, 793, 2074, 427, 1281, 427, 427, 1647, 854, 1281, 1647, 854,
      1281, 427, 1647, 427, 1708, 427, 854, 366, 854, 65000,
  };
  // Messages 1 & 3: 72-bit command. Message 2: status frame.
  const uint8_t cmd[] = {0x96, 0x19, 0x01, 0x80, 0x10, 0xCF, 0x18, 0x08, 0xF2};
  const uint8_t status[] = {0x9A, 0x26, 0x0C, 0xCF, 0x48, 0x83, 0xD0, 0x45,
                            0x5F, 0xD0, 0x42, 0x08, 0x03};
  uint8_t expected[sizeof(cmd) + sizeof(status) + sizeof(cmd)];
  memcpy(expected, cmd, sizeof(cmd));
  memcpy(expected + sizeof(cmd), status, sizeof(status));
  memcpy(expected + sizeof(cmd) + sizeof(status), cmd, sizeof(cmd));

  irsend.reset();
  irsend.sendRaw(rawData, 142, 38000);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MWM, irsend.capture.decode_type);
  EXPECT_EQ(sizeof(expected) * 8, irsend.capture.bits);
  EXPECT_STATE_EQ(expected, irsend.capture.state, irsend.capture.bits);
}

// Button press #6 (166 entries): command + status + command-repeat
// triple, extracted verbatim from lirc mode2 capture 'mode2-capture.log',
// lines 7098-7263. Spaces >= 65535us (inter-press idles) are
// clamped to 65000; they only need to read as 'long'.
TEST(TestDecodeMWMBundle, Mode2LogBundle6) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  const uint16_t rawData[166] = {
      854, 793, 427, 427, 854, 854, 427, 366, 854, 854, 1281, 366,
      427, 1281, 2074, 427, 427, 1708, 366, 427, 854, 427, 854, 427,
      854, 427, 1220, 427, 427, 427, 366, 915, 793, 1281, 1647, 915,
      1220, 427, 1281, 427, 2074, 427, 488, 1220, 427, 34831, 427, 854,
      427, 854, 854, 793, 854, 854, 854, 366, 854, 427, 1281, 854,
      1647, 427, 854, 1220, 854, 1281, 427, 1281, 793, 854, 427, 427,
      366, 488, 427, 793, 427, 854, 427, 427, 1708, 793, 427, 427,
      427, 427, 854, 1220, 427, 1708, 1708, 366, 854, 427, 427, 427,
      1220, 488, 2074, 427, 2074, 427, 427, 1281, 427, 427, 427, 1647,
      854, 427, 427, 427, 854, 366, 1708, 427, 427, 1647, 427, 33489,
      854, 793, 488, 366, 854, 854, 427, 366, 854, 854, 1220, 427,
      427, 1281, 2135, 366, 427, 1708, 427, 366, 854, 427, 854, 427,
      793, 427, 1220, 488, 427, 427, 427, 793, 854, 1281, 1708, 793,
      1281, 427, 1220, 427, 2135, 427, 427, 1220, 427, 65000,
  };
  // Messages 1 & 3: 72-bit command. Message 2: status frame.
  const uint8_t cmd[] = {0x96, 0x19, 0x07, 0x2F, 0x12, 0xCD, 0x18, 0x04, 0xF7};
  const uint8_t status[] = {0x9B, 0x26, 0x0C, 0xCE, 0x67, 0x6D, 0x58, 0xEE,
                            0x48, 0x04, 0xD0, 0x3D, 0x09, 0xEF};
  uint8_t expected[sizeof(cmd) + sizeof(status) + sizeof(cmd)];
  memcpy(expected, cmd, sizeof(cmd));
  memcpy(expected + sizeof(cmd), status, sizeof(status));
  memcpy(expected + sizeof(cmd) + sizeof(status), cmd, sizeof(cmd));

  irsend.reset();
  irsend.sendRaw(rawData, 166, 38000);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MWM, irsend.capture.decode_type);
  EXPECT_EQ(sizeof(expected) * 8, irsend.capture.bits);
  EXPECT_STATE_EQ(expected, irsend.capture.state, irsend.capture.bits);
}

// Button press #7 (168 entries): command + status + command-repeat
// triple, extracted verbatim from lirc mode2 capture 'mode2-capture.log',
// lines 7265-7432. Spaces >= 65535us (inter-press idles) are
// clamped to 65000; they only need to read as 'long'.
TEST(TestDecodeMWMBundle, Mode2LogBundle7) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  const uint16_t rawData[168] = {
      854, 793, 488, 366, 854, 793, 488, 366, 854, 854, 1281, 366,
      1708, 427, 1708, 366, 427, 427, 854, 366, 1708, 427, 1220, 427,
      488, 366, 1281, 427, 793, 488, 427, 793, 427, 1281, 1647, 854,
      1220, 488, 3721, 427, 427, 1708, 427, 34526, 1281, 1220, 854, 854,
      793, 915, 793, 427, 854, 427, 1281, 854, 1647, 427, 427, 854,
      427, 793, 427, 1281, 366, 488, 1647, 854, 427, 427, 427, 427,
      793, 488, 427, 793, 366, 488, 2074, 427, 427, 1281, 793, 427,
      854, 854, 793, 488, 2074, 2074, 854, 427, 854, 366, 427, 1281,
      427, 427, 2074, 427, 427, 427, 1220, 488, 2074, 427, 793, 488,
      366, 427, 1647, 427, 2562, 427, 854, 366, 488, 366, 427, 854,
      427, 366, 854, 38125, 915, 732, 488, 366, 854, 854, 427, 427,
      854, 793, 1342, 366, 1708, 366, 1708, 427, 427, 427, 854, 366,
      1647, 488, 1220, 488, 366, 427, 1220, 488, 793, 488, 366, 915,
      366, 1281, 1708, 793, 1220, 488, 3782, 366, 427, 1708, 427, 65000,
  };
  // Messages 1 & 3: 72-bit command. Message 2: status frame.
  const uint8_t cmd[] = {0x96, 0x19, 0x08, 0x09, 0x14, 0xDA, 0x18, 0x00, 0xEF};
  const uint8_t status[] = {0x9C, 0x26, 0x0C, 0xDB, 0x61, 0x69, 0xD0, 0x32,
                            0xF0, 0xD2, 0x41, 0x04, 0x0A, 0x20, 0x2D};
  uint8_t expected[sizeof(cmd) + sizeof(status) + sizeof(cmd)];
  memcpy(expected, cmd, sizeof(cmd));
  memcpy(expected + sizeof(cmd), status, sizeof(status));
  memcpy(expected + sizeof(cmd) + sizeof(status), cmd, sizeof(cmd));

  irsend.reset();
  irsend.sendRaw(rawData, 168, 38000);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MWM, irsend.capture.decode_type);
  EXPECT_EQ(sizeof(expected) * 8, irsend.capture.bits);
  EXPECT_STATE_EQ(expected, irsend.capture.state, irsend.capture.bits);
}

// Live hardware session (ESP8266 + IRrecvDumpV3 v2.9.0, /dev/ttyUSB0),
// timestamp 000111.534: a COMPLETE A-B-A' triple from one wand press,
// captured verbatim. All three messages are returned concatenated.
TEST(TestDecodeMWMBundle, LiveCaptureCompleteTriple) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  const uint16_t rawData[167] = {
      892, 790, 498, 344, 812, 864, 494, 348, 812, 864, 1322, 358,
      2070, 440, 1240, 436, 502, 340, 916, 348, 1748, 344, 1324, 358,
      498, 344, 1324, 362, 1292, 798, 492, 344, 860, 392, 922, 754,
      498, 344, 918, 764, 498, 760, 2602, 344, 1750, 768, 488, 348,
      462, 33194,      502, 344, 868, 820, 812, 872, 812, 872, 864, 388,
      922, 352, 438, 392, 1730, 798, 502, 340, 528, 314, 896, 378,
      464, 794, 494, 348, 520, 322, 1324, 362, 492, 1196, 896, 362,
      1756, 340, 498, 340, 922, 348, 488, 348, 1752, 344, 2546, 388,
      922, 344, 2170, 350, 466, 1214, 918, 336, 816, 870, 812, 440,
      2174, 2022,      498, 344, 494, 758, 498, 340, 502, 30924,
      844, 846, 462, 362, 926, 760, 524, 318, 926, 760, 1298, 388,
      2214, 318, 1298, 388, 494, 348, 818, 440, 1752, 362, 1328, 340,
      502, 340, 1248, 440, 1244, 872, 490, 348, 874, 384, 864, 820,
      472, 370, 818, 872, 468, 790, 2578, 366, 1726, 804, 498, 344,
      498,
  };
  // Messages 1 & 3: 72-bit command. Message 2: status frame.
  const uint8_t cmd[] = {0x96, 0x19, 0x10, 0x09, 0x14, 0x2C, 0x96, 0x03, 0x58};
  const uint8_t status[] = {0x99, 0x26, 0x61, 0x69, 0xD1, 0x42,
                            0x0A, 0x20, 0xD0, 0x32, 0xF0, 0xAD};
  uint8_t expected[sizeof(cmd) + sizeof(status) + sizeof(cmd)];
  memcpy(expected, cmd, sizeof(cmd));
  memcpy(expected + sizeof(cmd), status, sizeof(status));
  memcpy(expected + sizeof(cmd) + sizeof(status), cmd, sizeof(cmd));

  irsend.reset();
  irsend.sendRaw(rawData, 167, 38000);
  irsend.makeDecodeResult();

  // A single decode() returns the whole A-B-A' triple.
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MWM, irsend.capture.decode_type);
  EXPECT_EQ(sizeof(expected) * 8, irsend.capture.bits);
  EXPECT_STATE_EQ(expected, irsend.capture.state, irsend.capture.bits);
}

// Live hardware session, timestamp 000108.661: an A-B-A' triple where the
// final repeat (A') lost its last data bit AND stop bit at RF level - the
// device's own rawbuf ends ~2 bit cells early (verified against the on-device
// dump; not a host-side truncation). decodeMWM() refuses to fabricate the
// missing bit, so the bundle degrades gracefully to command + status. The
// command itself is never lost: message A carries it in full.
TEST(TestDecodeMWMBundle, LiveCaptureTruncatedRepeatTriple) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  const uint16_t rawData[167] = {
      922, 764, 494, 344, 916, 764, 494, 344, 890, 790, 1298, 382,
      2174, 354, 1318, 362, 464, 362, 926, 344, 1756, 344, 1296, 388,
      524, 318, 1302, 388, 494, 348, 1324, 362, 1298, 388, 896, 790,
      436, 388, 926, 764, 498, 772, 2556, 388, 896, 790, 498, 344,
      838, 33636,      498, 344, 922, 760, 890, 790, 864, 816, 860, 388,
      922, 350, 466, 374, 1752, 772, 462, 376, 436, 388, 926, 344,
      492, 760, 472, 366, 498, 344, 1292, 388, 498, 1182, 864, 388,
      1750, 358, 494, 348, 864, 388, 494, 344, 1752, 344, 2602, 348,
      864, 388, 2184, 344, 472, 1214, 926, 330, 922, 768, 896, 374,
      2096, 2104,      498, 344, 498, 772, 494, 336, 498, 31010,
      868, 820, 494, 348, 816, 868, 488, 348, 870, 812, 1324, 358,
      2096, 414, 1294, 388, 498, 340, 890, 362, 1722, 388, 1298, 384,
      466, 376, 1324, 356, 464, 362, 1288, 392, 1240, 436, 922, 764,
      498, 340, 890, 790, 498, 754, 2604, 348, 866, 812, 462, 380,
      864,
  };
  const uint8_t cmd[] = {0x96, 0x19, 0x10, 0x09, 0x14, 0x11, 0x96, 0x03, 0x96};
  const uint8_t status[] = {0x99, 0x26, 0x61, 0x69, 0xD1, 0x42,
                            0x0A, 0x20, 0xD0, 0x32, 0xF0, 0xAD};
  // The truncated A' would need a 10th byte with bit 7 set (0x??96); the
  // captured signal ends after its first 7 data bits.
  uint8_t expected[sizeof(cmd) + sizeof(status)];
  memcpy(expected, cmd, sizeof(cmd));
  memcpy(expected + sizeof(cmd), status, sizeof(status));

  irsend.reset();
  irsend.sendRaw(rawData, 167, 38000);
  irsend.makeDecodeResult();

  // The bundle degrades gracefully: command + status, truncated A' dropped.
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MWM, irsend.capture.decode_type);
  EXPECT_EQ(sizeof(expected) * 8, irsend.capture.bits);
  EXPECT_STATE_EQ(expected, irsend.capture.state, irsend.capture.bits);
}

// Live hardware session, timestamp 000112.522: a status frame followed by a
// command whose tail was cut off at RF level (same failure mode as
// LiveCaptureTruncatedRepeatTriple). Only the complete status frame is
// delivered; the truncated command is refused rather than mis-decoded.
TEST(TestDecodeMWMBundle, LiveCaptureTruncatedCommandPair) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  const uint16_t rawData[117] = {
      532, 314, 922, 764, 886, 790, 816, 864, 812, 440, 892, 374,
      468, 374, 1752, 760, 498, 340, 498, 344, 916, 348, 494, 758,
      498, 340, 498, 344, 1296, 384, 494, 1192, 812, 440, 1752, 340,
      498, 356, 866, 388, 492, 344, 1756, 340, 2576, 362, 922, 344,
      2180, 348, 494, 1192, 922, 336, 868, 820, 840, 418, 2184, 2030,
      498, 344, 498, 754, 502, 344, 524, 30966,
      838, 868, 488, 354, 868, 798, 498, 340, 926, 754, 1244, 436,
      2184, 344, 1292, 384, 494, 350, 812, 436, 1756, 356, 1324, 342,
      496, 344, 1240, 440, 498, 340, 498, 344, 498, 768, 870, 384,
      890, 790, 498, 344, 812, 868, 494, 758, 2574, 374, 1324, 1182,
      922,
  };
  const uint8_t status[] = {0x99, 0x26, 0x61, 0x69, 0xD1, 0x42,
                            0x0A, 0x20, 0xD0, 0x32, 0xF0, 0xAD};

  irsend.reset();
  irsend.sendRaw(rawData, 117, 38000);
  irsend.makeDecodeResult();

  // Only the complete status frame is returned; the truncated command after
  // it is dropped.
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ(MWM, irsend.capture.decode_type);
  EXPECT_EQ(sizeof(status) * 8, irsend.capture.bits);
  EXPECT_STATE_EQ(status, irsend.capture.state, irsend.capture.bits);
}
// vim: et:ts=2:sw=2
