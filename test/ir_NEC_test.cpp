// Copyright 2017 David Conran

#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

// Tests for sendNEC().

// Test sending typical data only.
TEST(TestSendNEC, SendDataOnly) {
  IRsendTest irsend(4);
  irsend.begin();
  irsend.sendNEC(0);
  EXPECT_EQ("m9000s4500m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
            "m560s560m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
            "m560s560m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
            "m560s560m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
            "m560s560m560s108000", irsend.outputStr());
  irsend.sendNEC(0xAA00FF55);
  EXPECT_EQ("m9000s4500m560s1690m560s560m560s1690m560s560m560s1690m560s560"
            "m560s1690m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
            "m560s560m560s560m560s1690m560s1690m560s1690m560s1690m560s1690"
            "m560s1690m560s1690m560s1690m560s560m560s1690m560s560m560s1690"
            "m560s560m560s1690m560s560m560s1690m560s108000",
            irsend.outputStr());
}

// Test sending different bit lengths.
TEST(TestSendNEC, SendSmallData) {
  IRsendTest irsend(4);
  irsend.begin();
  irsend.sendNEC(0xA, 4);  // Send only 4 data bits.
  EXPECT_EQ("m9000s4500m560s1690m560s560m560s1690m560s560m560s108000",
            irsend.outputStr());
  irsend.sendNEC(0, 8);  // Send only 8 data bits.
  EXPECT_EQ("m9000s4500m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
            "m560s560m560s108000", irsend.outputStr());
  irsend.sendNEC(0x1234567890ABCDEF, 64);  // Send 64 data bits.
  EXPECT_EQ("m9000s4500m560s560m560s560m560s560m560s1690m560s560m560s560"
            "m560s1690m560s560m560s560m560s560m560s1690m560s1690m560s560"
            "m560s1690m560s560m560s560m560s560m560s1690m560s560m560s1690"
            "m560s560m560s1690m560s1690m560s560m560s560m560s1690m560s1690"
            "m560s1690m560s1690m560s560m560s560m560s560m560s1690m560s560"
            "m560s560m560s1690m560s560m560s560m560s560m560s560m560s1690m560s560"
            "m560s1690m560s560m560s1690m560s560m560s1690m560s1690m560s1690"
            "m560s1690m560s560m560s560m560s1690m560s1690m560s560m560s1690"
            "m560s1690m560s1690m560s1690m560s560m560s1690m560s1690m560s1690"
            "m560s1690m560s108000", irsend.outputStr());
}

// Test sending with repeats.
TEST(TestSendNEC, SendWithRepeats) {
  IRsendTest irsend(4);
  irsend.begin();
  irsend.sendNEC(0, 8, 0);  // Send a command with 0 repeats.
  EXPECT_EQ("m9000s4500m560s560m560s560m560s560m560s560m560s560m560s560m560s560"
            "m560s560m560s108000", irsend.outputStr());
  irsend.sendNEC(0xAA, 8, 1);  // Send a command with 1 repeat.
  EXPECT_EQ("m9000s4500m560s1690m560s560m560s1690m560s560m560s1690m560s560"
            "m560s1690m560s560m560s108000"
            "m9000s2250m560s108000",
            irsend.outputStr());
  irsend.sendNEC(0xAA, 8, 3);  // Send a command with 3 repeats.
  EXPECT_EQ("m9000s4500m560s1690m560s560m560s1690m560s560m560s1690m560s560"
            "m560s1690m560s560m560s108000"
            "m9000s2250m560s108000"
            "m9000s2250m560s108000"
            "m9000s2250m560s108000",
            irsend.outputStr());
}

// Tests for encodeNEC().

TEST(TestEncodeNEC, NormalNECEncoding) {
  IRsendTest irsend(4);
  EXPECT_EQ(0x807F40BF, irsend.encodeNEC(1, 2));
  EXPECT_EQ(0x9A656897, irsend.encodeNEC(0x59, 0x16));
}

TEST(TestEncodeNEC, ExtendedNECEncoding) {
  IRsendTest irsend(4);
  EXPECT_EQ(0x9A806897, irsend.encodeNEC(0x159, 0x16));
}

TEST(TestEncodeNEC, CommandTrimmedTo8Bits) {
  IRsendTest irsend(4);
  EXPECT_EQ(irsend.encodeNEC(0x1, 0x2), irsend.encodeNEC(0x1, 0xF02));
  EXPECT_EQ(irsend.encodeNEC(0xFFF0, 0x2), irsend.encodeNEC(0xFFF0, 0xF02));
}
