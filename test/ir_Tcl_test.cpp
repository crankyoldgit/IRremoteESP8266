// Copyright 2019 David Conran

#include "ir_Tcl.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

// General housekeeping
TEST(TestTcl112Ac, Housekeeping) {
  ASSERT_EQ("TCL112AC", typeToString(TCL112AC));
  ASSERT_TRUE(hasACState(TCL112AC));
}

// Tests for decodeTcl112Ac().

// Decode a real Tcl112Ac A/C example from Issue #619
TEST(TestDecodeTcl112Ac, DecodeRealExample) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  irsend.reset();
  // Tcl112Ac A/C example from Issue #619 On.txt
  uint16_t rawData[227] = {
      3030, 1658, 494,  1066, 494,  1068, 498,  320,  494,
      326,  498,  320,  494,  1068, 500,  320,  494,  332,
      494,  1068, 500,  1062, 496,  324,  492,  1044, 524,
      322,  492,  326,  498,  1062, 494,  1074, 494,  326,
      500,  1062, 496,  1066, 490,  328,  496,  322,  492,
      1070, 498,  322,  494,  332,  492,  1068, 498,  320,
      494,  326,  498,  320,  496,  324,  500,  320,  494,
      324,  490,  336,  500,  320,  496,  324,  490,  328,
      496,  322,  492,  328,  498,  322,  492,  326,  498,
      328,  496,  322,  492,  328,  498,  1064, 494,  326,
      498,  320,  494,  1066, 490,  330,  496,  330,  494,
      1066, 490,  1070, 498,  322,  492,  328,  498,  322,
      492,  326,  498,  322,  492,  332,  492,  1068, 498,
      1062, 494,  1066, 500,  318,  496,  324,  490,  328,
      496,  324,  492,  334,  490,  328,  496,  324,  492,
      328,  496,  322,  492,  328,  498,  320,  494,  1068,
      500,  326,  500,  320,  492,  326,  500,  320,  496,
      324,  500,  318,  496,  324,  490,  328,  496,  330,
      496,  324,  490,  328,  496,  324,  490,  328,  498,
      322,  492,  328,  498,  320,  492,  334,  492,  328,
      498,  322,  494,  326,  498,  320,  494,  324,  500,
      322,  492,  324,  490,  336,  498,  320,  494,  324,
      500,  320,  496,  324,  490,  328,  498,  322,  492,
      328,  496,  1070, 496,  1064, 492,  1070, 498,  322,
      494,  326,  500,  320,  494,  324,  500,  320,  494,
      324,  470};  // UNKNOWN CE60D6B9
  uint8_t expectedState[kTcl112AcStateLength] = {
      0x23, 0xCB, 0x26, 0x01, 0x00, 0x24, 0x03,
      0x07, 0x40, 0x00, 0x00, 0x00, 0x80, 0x03};

  irsend.sendRaw(rawData, 227, 38000);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(TCL112AC, irsend.capture.decode_type);
  EXPECT_EQ(kTcl112AcBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);

  IRTcl112Ac ac(0);
  ac.setRaw(irsend.capture.state);
  EXPECT_EQ("Temp: 24C", ac.toString());
}

// Decode a synthetic Tcl112Ac A/C example from Issue #619
TEST(TestDecodeTcl112Ac, DecodeSyntheticExample) {
  IRsendTest irsend(0);
  IRrecv irrecv(0);
  irsend.begin();

  irsend.reset();

  uint8_t expectedState[kTcl112AcStateLength] = {0x23, 0xCB, 0x26, 0x01, 0x00,
                                                 0x24, 0x03, 0x07, 0x40, 0x00,
                                                 0x00, 0x00, 0x80, 0x03};

  irsend.sendTcl112Ac(expectedState);
  irsend.makeDecodeResult();

  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(TCL112AC, irsend.capture.decode_type);
  EXPECT_EQ(kTcl112AcBits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
}

TEST(TestTcl112AcClass, TemperatureExamples) {
  const uint8_t temp16C[kTcl112AcStateLength] = {
      0x23, 0xCB, 0x26, 0x01, 0x00, 0x24, 0x03,
      0x0F, 0x00, 0x00, 0x00, 0x00, 0x80, 0xCB};
  const uint8_t temp16point5C[kTcl112AcStateLength] = {
      0x23, 0xCB, 0x26, 0x01, 0x00, 0x24, 0x03,
      0x0F, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xEB};
  const uint8_t temp19point5C[kTcl112AcStateLength] = {
      0x23, 0xCB, 0x26, 0x01, 0x00, 0x24, 0x03,
      0x0C, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xE8};
  const uint8_t temp31C[kTcl112AcStateLength] = {
      0x23, 0xCB, 0x26, 0x01, 0x00, 0x24, 0x03,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xBC};
  IRTcl112Ac ac(0);
  ac.setRaw(temp16C);
  EXPECT_EQ("Temp: 16C", ac.toString());
  ac.setRaw(temp16point5C);
  EXPECT_EQ("Temp: 16.5C", ac.toString());
  ac.setRaw(temp19point5C);
  EXPECT_EQ("Temp: 19.5C", ac.toString());
  ac.setRaw(temp31C);
  EXPECT_EQ("Temp: 31C", ac.toString());
}
