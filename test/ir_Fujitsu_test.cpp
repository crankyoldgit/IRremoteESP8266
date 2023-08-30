// Copyright 2017 Jonny Graham, David Conran
// Copyright 2023 Takeshi Shimizu
#include "IRac.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "ir_Fujitsu.h"
#include "gtest/gtest.h"

// Tests for Fujitsu A/C methods.

// Test sending typical data only.
TEST(TestIRFujitsuACClass, GetRawDefault) {
  IRFujitsuAC ac(kGpioUnused);  // AR-RAH2E
  ac.setSwing(kFujitsuAcSwingBoth);
  ac.setMode(kFujitsuAcModeCool);
  ac.setFanSpeed(kFujitsuAcFanHigh);
  ac.setTemp(24);
  ac.setCmd(kFujitsuAcCmdTurnOn);
  uint8_t expected_arrah2e[16] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
      0x81, 0x01, 0x31, 0x00, 0x00, 0x00, 0x20, 0xFD};
  EXPECT_STATE_EQ(expected_arrah2e, ac.getRaw(), 16 * 8);
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_EQ("Model: 1 (ARRAH2E), Id: 0, Power: On, Mode: 1 (Cool), Temp: 24C, "
            "Fan: 1 (High), Clean: Off, Filter: Off, 10C Heat: Off, "
            "Swing: 3 (Swing(V)+Swing(H)), Command: N/A, Timer: Off",
            ac.toString());

  uint8_t expected_ardb1[15] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFC, 0x08, 0x30,
      0x81, 0x01, 0x01, 0x00, 0x00, 0x00, 0x4D};
  ac.setModel(ARDB1);
  EXPECT_STATE_EQ(expected_ardb1, ac.getRaw(), 15 * 8);
  EXPECT_EQ(kFujitsuAcStateLength - 1, ac.getStateLength());
  EXPECT_EQ("Model: 2 (ARDB1), Id: 0, Power: On, Mode: 1 (Cool), Temp: 24C, "
            "Fan: 1 (High), Command: N/A",
            ac.toString());
}

TEST(TestIRFujitsuACClass, GetRawTurnOff) {
  IRFujitsuAC ac(kGpioUnused);
  ac.setModel(ARRAH2E);
  ac.off();
  uint8_t expected_arrah2e[7] = {0x14, 0x63, 0x0, 0x10, 0x10, 0x02, 0xFD};
  EXPECT_STATE_EQ(expected_arrah2e, ac.getRaw(), 7 * 8);
  EXPECT_EQ(kFujitsuAcStateLengthShort, ac.getStateLength());
  EXPECT_EQ("Model: 1 (ARRAH2E), Id: 0, Power: Off, Mode: 1 (Cool), Temp: 24C, "
            "Fan: 1 (High), Clean: Off, Filter: Off, 10C Heat: Off, "
            "Swing: 3 (Swing(V)+Swing(H)), Command: N/A, Timer: Off",
            ac.toString());

  ac.setModel(ARDB1);
  uint8_t expected_ardb1[6] = {0x14, 0x63, 0x0, 0x10, 0x10, 0x02};
  EXPECT_STATE_EQ(expected_ardb1, ac.getRaw(), 6 * 8);
  EXPECT_EQ(kFujitsuAcStateLengthShort - 1, ac.getStateLength());
  EXPECT_EQ("Model: 2 (ARDB1), Id: 0, Power: Off, Mode: 1 (Cool), Temp: 24C, "
            "Fan: 1 (High), Command: N/A",
            ac.toString());
}

TEST(TestIRFujitsuACClass, GetRawStepHoriz) {
  IRFujitsuAC ac(kGpioUnused);
  ac.stepHoriz();
  uint8_t expected[7] = {0x14, 0x63, 0x0, 0x10, 0x10, 0x79, 0x86};
  EXPECT_STATE_EQ(expected, ac.getRaw(), 7 * 8);
  EXPECT_EQ(kFujitsuAcStateLengthShort, ac.getStateLength());
  EXPECT_EQ(
      "Model: 1 (ARRAH2E), Id: 0, Power: On, Mode: 1 (Cool), Temp: 24C, "
      "Fan: 1 (High), Clean: Off, Filter: Off, 10C Heat: Off, "
      "Swing: 3 (Swing(V)+Swing(H)), Command: Step Swing(H), Timer: Off",
      ac.toString());
}

TEST(TestIRFujitsuACClass, GetRawStepVert) {
  IRFujitsuAC ac(kGpioUnused);
  ac.setModel(ARRAH2E);
  ac.stepVert();
  uint8_t expected_arrah2e[7] = {0x14, 0x63, 0x0, 0x10, 0x10, 0x6C, 0x93};
  EXPECT_STATE_EQ(expected_arrah2e, ac.getRaw(), 7 * 8);
  EXPECT_EQ(kFujitsuAcStateLengthShort, ac.getStateLength());
  EXPECT_EQ(
      "Model: 1 (ARRAH2E), Id: 0, Power: On, Mode: 1 (Cool), Temp: 24C, "
      "Fan: 1 (High), Clean: Off, Filter: Off, 10C Heat: Off, "
      "Swing: 3 (Swing(V)+Swing(H)), Command: Step Swing(V), Timer: Off",
      ac.toString());

  ac.setModel(ARDB1);
  ac.stepVert();
  uint8_t expected_ardb1[6] = {0x14, 0x63, 0x0, 0x10, 0x10, 0x6C};
  EXPECT_STATE_EQ(expected_ardb1, ac.getRaw(), 6 * 8);
  EXPECT_EQ(kFujitsuAcStateLengthShort - 1,
            ac.getStateLength());
  EXPECT_EQ("Model: 2 (ARDB1), Id: 0, Power: On, Mode: 1 (Cool), Temp: 24C, "
            "Fan: 1 (High), Command: Step Swing(V)",
            ac.toString());
}

TEST(TestIRFujitsuACClass, GetRawWithSwingHoriz) {
  IRFujitsuAC ac(kGpioUnused);
  ac.setCmd(kFujitsuAcCmdStayOn);
  ac.setSwing(kFujitsuAcSwingHoriz);
  ac.setMode(kFujitsuAcModeCool);
  ac.setFanSpeed(kFujitsuAcFanQuiet);
  ac.setTemp(25);
  uint8_t expected[16] = {0x14, 0x63, 0x0, 0x10, 0x10, 0xFE, 0x9, 0x30,
                          0x90, 0x1, 0x24, 0x0, 0x0, 0x0, 0x20, 0xFB};
  EXPECT_STATE_EQ(expected, ac.getRaw(), 16 * 8);
  EXPECT_EQ("Model: 1 (ARRAH2E), Id: 0, Power: On, Mode: 1 (Cool), Temp: 25C, "
            "Fan: 4 (Quiet), Clean: Off, Filter: Off, 10C Heat: Off, "
            "Swing: 2 (Swing(H)), Command: N/A, Timer: Off",
            ac.toString());
}

TEST(TestIRFujitsuACClass, GetRawWithFan) {
  IRFujitsuAC ac(kGpioUnused);
  ac.setCmd(kFujitsuAcCmdStayOn);
  ac.setSwing(kFujitsuAcSwingHoriz);
  ac.setMode(kFujitsuAcModeFan);
  ac.setFanSpeed(kFujitsuAcFanMed);
  ac.setTemp(20);  // temp doesn't matter for fan
                        // but it is sent by the RC anyway
  ac.setModel(ARRAH2E);
  uint8_t expected_arrah2e[16] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
      0x40, 0x03, 0x22, 0x00, 0x00, 0x00, 0x20, 0x4B};
  EXPECT_STATE_EQ(expected_arrah2e, ac.getRaw(), 16 * 8);
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_EQ("Model: 1 (ARRAH2E), Id: 0, Power: On, Mode: 3 (Fan), Temp: 20C, "
            "Fan: 2 (Medium), Clean: Off, Filter: Off, 10C Heat: Off, "
            "Swing: 2 (Swing(H)), Command: N/A, Timer: Off",
            ac.toString());

  ac.setModel(ARDB1);
  uint8_t expected_ardb1[15] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFC, 0x08, 0x30,
      0x40, 0x03, 0x02, 0x00, 0x00, 0x00, 0x8B};
  EXPECT_EQ(kFujitsuAcStateLength - 1, ac.getStateLength());
  EXPECT_STATE_EQ(expected_ardb1, ac.getRaw(), ac.getStateLength() * 8);
  EXPECT_EQ("Model: 2 (ARDB1), Id: 0, Power: On, Mode: 3 (Fan), Temp: 20C, "
            "Fan: 2 (Medium), Command: N/A", ac.toString());
}

TEST(TestIRFujitsuACClass, SetRaw) {
  IRFujitsuAC ac(kGpioUnused);
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  uint8_t expected_default_arrah2e[kFujitsuAcStateLength] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
      0x81, 0x01, 0x31, 0x00, 0x00, 0x00, 0x20, 0xFD};
  EXPECT_STATE_EQ(expected_default_arrah2e, ac.getRaw(),
                  ac.getStateLength() * 8);
  EXPECT_EQ("Model: 1 (ARRAH2E), Id: 0, Power: On, Mode: 1 (Cool), Temp: 24C, "
            "Fan: 1 (High), Clean: Off, Filter: Off, 10C Heat: Off, "
            "Swing: 3 (Swing(V)+Swing(H)), Command: N/A, "
            "Timer: Off",
            ac.toString());
  // Now set a new state via setRaw();
  // This state is a real state from an AR-DB1 remote.
  uint8_t new_state1[kFujitsuAcStateLength - 1] = {
    0x14, 0x63, 0x00, 0x10, 0x10, 0xFC, 0x08, 0x30,
    0x30, 0x01, 0x00, 0x00, 0x00, 0x00, 0x9F};
  ac.setRaw(new_state1, kFujitsuAcStateLength - 1);
  EXPECT_EQ(kFujitsuAcStateLength - 1, ac.getStateLength());
  EXPECT_STATE_EQ(new_state1, ac.getRaw(), ac.getStateLength() * 8);
  EXPECT_EQ("Model: 2 (ARDB1), Id: 0, Power: On, Mode: 1 (Cool), Temp: 19C, "
            "Fan: 0 (Auto), Command: N/A", ac.toString());
}

TEST(TestSendFujitsuAC, GenerateMessage) {
  IRFujitsuAC ac(kGpioUnused);
  IRsendTest irsend(kGpioUnused);
  ac.begin();
  irsend.begin();

  ac.setCmd(kFujitsuAcCmdStayOn);
  ac.setSwing(kFujitsuAcSwingBoth);
  ac.setMode(kFujitsuAcModeCool);
  ac.setFanSpeed(kFujitsuAcFanHigh);
  ac.setTemp(24);

  EXPECT_EQ(kFujitsuAcFanHigh, ac.getFanSpeed());
  EXPECT_EQ(kFujitsuAcModeCool, ac.getMode());
  EXPECT_EQ(24, ac.getTemp());
  EXPECT_EQ(kFujitsuAcSwingBoth, ac.getSwing());
  EXPECT_EQ(kFujitsuAcCmdStayOn, ac.getCmd());

  irsend.reset();
  irsend.sendFujitsuAC(ac.getRaw(), kFujitsuAcStateLength);
  EXPECT_EQ(
  "f38000d50"
  "m3324s1574"
  "m448s390m448s390m448s1182m448s390m448s1182m448s390m448s390m448s390"
  "m448s1182m448s1182m448s390m448s390m448s390m448s1182m448s1182m448s390"
  "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
  "m448s390m448s390m448s390m448s390m448s1182m448s390m448s390m448s390"
  "m448s390m448s390m448s390m448s390m448s1182m448s390m448s390m448s390"
  "m448s390m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182"
  "m448s1182m448s390m448s390m448s1182m448s390m448s390m448s390m448s390"
  "m448s390m448s390m448s390m448s390m448s1182m448s1182m448s390m448s390"
  "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s1182"
  "m448s1182m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
  "m448s1182m448s390m448s390m448s390m448s1182m448s1182m448s390m448s390"
  "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
  "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
  "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
  "m448s390m448s390m448s390m448s390m448s390m448s1182m448s390m448s390"
  "m448s390m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182"
  "m448s8100",
  irsend.outputStr());
}

TEST(TestSendFujitsuAC, GenerateShortMessage) {
  IRFujitsuAC ac(kGpioUnused);
  IRsendTest irsend(kGpioUnused);
  ac.begin();
  irsend.begin();

  ac.off();

  EXPECT_EQ(kFujitsuAcCmdTurnOff, ac.getCmd());

  irsend.reset();
  irsend.sendFujitsuAC(ac.getRaw(), kFujitsuAcStateLengthShort);
  EXPECT_EQ(
  "f38000d50"
  "m3324s1574m448s390m448s390m448s1182m448s390m448s1182m448s390m448s390m448"
  "s390m448s1182m448s1182m448s390m448s390m448s390m448s1182m448s1182m448s390"
  "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
  "m448s390m448s390m448s390m448s1182m448s390m448s390m448s390m448s390m448s390"
  "m448s390m448s390m448s1182m448s390m448s390m448s390m448s390m448s1182m448s390"
  "m448s390m448s390m448s390m448s390m448s390m448s1182m448s390m448s1182m448"
  "s1182m448s1182m448s1182m448s1182m448s1182m448s8100",
  irsend.outputStr());
}

// Issue #275
TEST(TestSendFujitsuAC, Issue275) {
  IRFujitsuAC ac(kGpioUnused);
  IRsendTest irsend(kGpioUnused);
  ac.begin();
  irsend.begin();
  irsend.reset();

  ac.setCmd(kFujitsuAcCmdTurnOff);
  irsend.sendFujitsuAC(ac.getRaw(), kFujitsuAcStateLengthShort);
  EXPECT_EQ(
      "f38000d50"
      // Header
      "m3324s1574"
      //  0       0       1        0       1        0       0       0     (0x28)
      "m448s390m448s390m448s1182m448s390m448s1182m448s390m448s390m448s390"
      //  1        1        0       0       0       1        1        0   (0xC6)
      "m448s1182m448s1182m448s390m448s390m448s390m448s1182m448s1182m448s390"
      //  0       0       0       0       0       0       0       0       (0x00)
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      //  0       0       0       0       1        0       0       0      (0x08)
      "m448s390m448s390m448s390m448s390m448s1182m448s390m448s390m448s390"
      //  0       0       0       0       1        0       0       0      (0x08)
      "m448s390m448s390m448s390m448s390m448s1182m448s390m448s390m448s390"
      //  0       1        0       0       0       0       0       0      (0x40)
      "m448s390m448s1182m448s390m448s390m448s390m448s390m448s390m448s390"
      //  1        0       1        1        1        1        1        1 (0xBF)
      "m448s1182m448s390m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182"
      // Footer
      "m448s8100", irsend.outputStr());

  irsend.reset();
  // Per report in Issue #275
  uint16_t off[115] = {
      3350, 1650,
      450, 400, 450, 450, 450, 1250, 450, 400, 450, 1250, 450, 400, 450, 400,
      450, 400, 450, 1250, 450, 1250, 450, 400, 450, 400, 450, 400, 450, 1250,
      450, 1250, 450, 400, 450, 400, 450, 400, 450, 400, 450, 400, 450, 400,
      450, 400, 450, 400, 450, 400, 450, 400, 450, 400, 450, 400, 450, 400,
      450, 1250, 450, 400, 450, 400, 450, 400, 450, 400, 450, 400, 450, 400,
      450, 400, 450, 1250, 450, 400, 450, 400, 450, 400, 450, 400, 450, 1250,
      450, 400, 450, 400, 450, 400, 450, 400, 450, 400, 450, 400, 450, 1250,
      450, 400, 450, 1250, 450, 1250, 450, 1250, 450, 1250, 450, 1250,
      450, 1250, 450};
  irsend.sendRaw(off, 115, 38);
  EXPECT_EQ(
      "f38000d50"
      // Header
      "m3350s1650"
      //  0       0       1        0       1        0       0       0     (0x28)
      "m450s400m450s450m450s1250m450s400m450s1250m450s400m450s400m450s400"
      //  1        1        0       0       0       1        1        0   (0xC6)
      "m450s1250m450s1250m450s400m450s400m450s400m450s1250m450s1250m450s400"
      //  0       0       0       0       0       0       0       0       (0x00)
      "m450s400m450s400m450s400m450s400m450s400m450s400m450s400m450s400"
      //  0       0       0       0       1        0       0       0      (0x08)
      "m450s400m450s400m450s400m450s400m450s1250m450s400m450s400m450s400"
      //  0       0       0       0       1        0       0       0      (0x08)
      "m450s400m450s400m450s400m450s400m450s1250m450s400m450s400m450s400"
      //  0       1        0       0       0       0       0       0      (0x40)
      "m450s400m450s1250m450s400m450s400m450s400m450s400m450s400m450s400"
      //  1        0       1        1        1        1        1        1 (0xBF)
      "m450s1250m450s400m450s1250m450s1250m450s1250m450s1250m450s1250m450s1250"
      // Footer
      "m450",
      irsend.outputStr());
}

TEST(TestDecodeFujitsuAC, SyntheticShortMessages) {
  IRsendTest irsend(kGpioUnused);
  IRFujitsuAC ac(kGpioUnused);
  IRrecv irrecv(kGpioUnused);

  irsend.begin();
  irsend.reset();

  ac.setModel(ARRAH2E);
  ac.setCmd(kFujitsuAcCmdTurnOff);
  irsend.sendFujitsuAC(ac.getRaw(), ac.getStateLength());
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcMinBits + 8, irsend.capture.bits);
  uint8_t expected_arrah2e[7] = {0x14, 0x63, 0x0, 0x10, 0x10, 0x02, 0xFD};
  EXPECT_STATE_EQ(expected_arrah2e, irsend.capture.state, irsend.capture.bits);
  EXPECT_EQ(
      "Model: 1 (ARRAH2E), Id: 0, Power: Off, Command: N/A",
      IRAcUtils::resultAcToString(&irsend.capture));
  stdAc::state_t r, p;
  ASSERT_TRUE(IRAcUtils::decodeToState(&irsend.capture, &r, &p));

  irsend.reset();

  ac.setModel(ARDB1);
  ac.setCmd(kFujitsuAcCmdTurnOff);
  irsend.sendFujitsuAC(ac.getRaw(), ac.getStateLength());
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcMinBits, irsend.capture.bits);
  uint8_t expected_ardb1[6] = {0x14, 0x63, 0x0, 0x10, 0x10, 0x02};
  EXPECT_STATE_EQ(expected_ardb1, irsend.capture.state, irsend.capture.bits);
}

TEST(TestDecodeFujitsuAC, SyntheticLongMessages) {
  IRsendTest irsend(kGpioUnused);
  IRFujitsuAC ac(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();

  irsend.reset();

  ac.setModel(ARRAH2E);
  ac.setCmd(kFujitsuAcCmdStayOn);
  ac.setSwing(kFujitsuAcSwingVert);
  ac.setMode(kFujitsuAcModeCool);
  ac.setFanSpeed(kFujitsuAcFanQuiet);
  ac.setTemp(18);
  irsend.sendFujitsuAC(ac.getRaw(), ac.getStateLength());
  ASSERT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decodeFujitsuAC(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcBits, irsend.capture.bits);
  uint8_t expected_arrah2e[kFujitsuAcStateLength] = {
    0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
    0x20, 0x01, 0x14, 0x00, 0x00, 0x00, 0x20, 0x7B};
  EXPECT_STATE_EQ(expected_arrah2e, irsend.capture.state, irsend.capture.bits);
  ac.setRaw(irsend.capture.state, irsend.capture.bits / 8);
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_EQ("Model: 1 (ARRAH2E), Id: 0, Power: On, Mode: 1 (Cool), Temp: 18C, "
            "Fan: 4 (Quiet), Clean: Off, Filter: Off, 10C Heat: Off, "
            "Swing: 1 (Swing(V)), Command: N/A, "
            "Timer: Off",
            ac.toString());

  irsend.reset();

  ac.setModel(ARDB1);
  irsend.sendFujitsuAC(ac.getRaw(), ac.getStateLength());
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcBits - 8, irsend.capture.bits);
  uint8_t expected_ardb1[kFujitsuAcStateLength - 1] = {
    0x14, 0x63, 0x00, 0x10, 0x10, 0xFC, 0x08, 0x30,
    0x20, 0x01, 0x04, 0x00, 0x00, 0x00, 0xAB};
  EXPECT_STATE_EQ(expected_ardb1, irsend.capture.state, irsend.capture.bits);
  ac.setRaw(irsend.capture.state, irsend.capture.bits / 8);
  EXPECT_EQ(kFujitsuAcStateLength - 1, ac.getStateLength());
  EXPECT_EQ("Model: 2 (ARDB1), Id: 0, Power: On, Mode: 1 (Cool), Temp: 18C, "
            "Fan: 4 (Quiet), Command: N/A", ac.toString());
}

TEST(TestDecodeFujitsuAC, RealShortARDB1OffExample) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  IRFujitsuAC ac(kGpioUnused);

  irsend.begin();

  irsend.reset();
  // "Off" Message recorded from an AR-DB1 remote.
  uint16_t rawData[99] = {
      3310, 1636,  440, 386,  440, 394,  442, 1210,  442, 390,  414, 1220,
      444, 390,  446, 380,  446, 380,  436, 1216,  438, 1214,  438, 388,
      438, 386,  438, 396,  410, 1222,  440, 1220,  442, 384,  442, 384,
      442, 384,  442, 382,  444, 382,  442, 382,  444, 380,  446, 380,
      446, 380,  444, 380,  436, 390,  436, 388,  436, 388,  438, 1214,
      438, 386,  438, 388,  438, 386,  440, 386,  440, 384,  442, 384,
      442, 384,  442, 1210,  444, 382,  444, 382,  444, 382,  444, 380,
      446, 1206,  436, 390,  436, 388,  436, 388,  438, 388,  438, 396,
      420, 388,  436};
  irsend.sendRaw(rawData, 99, 38000);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcMinBits, irsend.capture.bits);
  uint8_t expected[6] = {0x14, 0x63, 0x0, 0x10, 0x10, 0x02};
  EXPECT_STATE_EQ(expected, irsend.capture.state, irsend.capture.bits);
  ac.setRaw(irsend.capture.state, irsend.capture.bits / 8);
  EXPECT_EQ(kFujitsuAcStateLengthShort - 1, ac.getStateLength());
  EXPECT_EQ("Model: 2 (ARDB1), Id: 0, Power: Off, Mode: 0 (Auto), Temp: 16C, "
            "Fan: 0 (Auto), Command: N/A", ac.toString());
}

TEST(TestDecodeFujitsuAC, RealLongARDB1Example) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  IRFujitsuAC ac(kGpioUnused);

  irsend.begin();
  irsend.reset();
  uint16_t rawData1[243] = {
      3316, 1632,  444, 390,  438, 388,  436, 1216,  438, 388,  438, 1214,
      438, 388,  438, 386,  440, 386,  440, 1212,  440, 1210,  442, 392,
      412, 396,  442, 392,  444, 1208,  444, 1208,  444, 380,  444, 380,
      446, 380,  436, 390,  436, 390,  436, 390,  436, 388,  438, 388,
      438, 388,  438, 388,  438, 386,  438, 386,  440, 384,  440, 1210,
      442, 384,  442, 382,  442, 384,  442, 384,  442, 382,  442, 382,
      444, 382,  444, 1208,  444, 382,  444, 380,  446, 380,  436, 390,
      436, 390,  436, 1214,  438, 1214,  438, 1212,  440, 1212,  440, 1220,
      412, 1222,  440, 394,  442, 382,  442, 382,  444, 1208,  444, 382,
      444, 380,  446, 380,  446, 380,  434, 390,  436, 388,  438, 388,
      438, 388,  438, 1214,  438, 1212,  440, 386,  440, 394,  412, 1222,
      440, 394,  442, 384,  442, 384,  442, 382,  442, 1208,  444, 390,
      414, 394,  442, 1216,  446, 380,  436, 390,  436, 390,  436, 388,
      436, 390,  436, 388,  438, 386,  440, 386,  440, 386,  438, 1212,
      440, 386,  440, 384,  440, 384,  442, 392,  412, 396,  440, 394,
      442, 382,  444, 382,  444, 382,  444, 380,  444, 380,  444, 382,
      444, 380,  446, 380,  436, 388,  436, 390,  436, 388,  438, 388,
      438, 388,  438, 388,  438, 386,  440, 386,  440, 386,  442, 384,
      440, 386,  442, 384,  440, 384,  442, 384,  442, 382,  442, 382,
      444, 1208,  444, 382,  444, 1208,  444, 380,  446, 1206,  436, 390,
      436, 1216,  436};
  irsend.sendRaw(rawData1, 243, 38000);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcBits - 8, irsend.capture.bits);
  uint8_t expected1[kFujitsuAcStateLength - 1] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFC, 0x08, 0x30,
      0x21, 0x01, 0x04, 0x00, 0x00, 0x00, 0xAA};
  EXPECT_STATE_EQ(expected1, irsend.capture.state, irsend.capture.bits);
  ac.setRaw(irsend.capture.state, irsend.capture.bits / 8);
  EXPECT_EQ(kFujitsuAcStateLength - 1, ac.getStateLength());
  EXPECT_EQ("Model: 2 (ARDB1), Id: 0, Power: On, Mode: 1 (Cool), Temp: 18C, "
            "Fan: 4 (Quiet), Command: N/A", ac.toString());

  irsend.reset();
  uint16_t rawData2[243] = {
      3316, 1630,  436, 398,  438, 386,  438, 1212,  440, 384,  440, 1212,
      442, 384,  442, 392,  414, 394,  442, 1218,  446, 1206,  436, 390,
      436, 388,  438, 388,  438, 1214,  440, 1212,  440, 384,  442, 384,
      442, 384,  442, 382,  444, 382,  444, 382,  444, 380,  446, 380,
      444, 380,  436, 390,  436, 388,  438, 396,  418, 388,  438, 1232,
      410, 396,  440, 394,  442, 384,  442, 384,  442, 382,  442, 392,
      414, 392,  444, 1216,  446, 380,  436, 390,  436, 396,  418, 390,
      436, 398,  438, 1214,  440, 1212,  440, 1210,  442, 1208,  444, 1216,
      416, 1218,  444, 388,  436, 390,  436, 388,  438, 1214,  440, 386,
      438, 386,  440, 386,  440, 384,  442, 384,  442, 384,  442, 382,
      444, 382,  444, 1206,  446, 1206,  436, 390,  436, 388,  438, 388,
      438, 386,  440, 394,  410, 396,  440, 1220,  442, 1210,  442, 392,
      414, 394,  442, 1218,  446, 406,  410, 388,  436, 390,  436, 390,
      436, 388,  438, 386,  440, 386,  440, 386,  440, 386,  440, 384,
      442, 384,  442, 384,  442, 382,  444, 382,  444, 380,  446, 380,
      446, 380,  436, 390,  436, 390,  436, 388,  438, 386,  438, 388,
      438, 386,  440, 386,  440, 384,  442, 384,  442, 384,  442, 384,
      442, 382,  444, 382,  444, 380,  446, 380,  446, 380,  436, 390,
      436, 388,  436, 388,  438, 386,  438, 386,  440, 386,  440, 1212,
      440, 1210,  442, 1210,  442, 1208,  444, 1208,  436, 390,  436, 388,
      436, 1214,  440};
  irsend.sendRaw(rawData2, 243, 38000);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcBits - 8, irsend.capture.bits);
  uint8_t expected2[kFujitsuAcStateLength - 1] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFC, 0x08, 0x30,
      0x30, 0x01, 0x00, 0x00, 0x00, 0x00, 0x9F};
  EXPECT_STATE_EQ(expected2, irsend.capture.state, irsend.capture.bits);
  ac.setRaw(irsend.capture.state, irsend.capture.bits / 8);
  EXPECT_EQ(kFujitsuAcStateLength - 1, ac.getStateLength());
  EXPECT_EQ("Model: 2 (ARDB1), Id: 0, Power: On, Mode: 1 (Cool), Temp: 19C, "
            "Fan: 0 (Auto), Command: N/A", ac.toString());
}

TEST(TestDecodeFujitsuAC, Issue414) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  IRFujitsuAC ac(kGpioUnused);

  // Capture as supplied by arpmota
  uint16_t rawData[259] = {3352, 1574, 480, 350, 480, 346, 480, 1190, 458, 346,
      508, 1140, 480, 346, 506, 346, 458, 346, 480, 1168, 480, 1192, 452, 374,
      458, 346, 480, 346, 508, 1168, 480, 1140, 480, 346, 506, 346, 458, 346,
      480, 346, 480, 346, 480, 346, 484, 372, 454, 374, 456, 346, 508, 318,
      480, 374, 458, 374, 480, 318, 480, 1196, 452, 346, 480, 346, 484, 342,
      484, 346, 480, 374, 458, 346, 506, 318, 508, 1170, 452, 346, 480, 374,
      458, 346, 506, 318, 480, 1196, 452, 1190, 458, 1162, 480, 1196, 452,
      1170, 480, 1190, 458, 1164, 480, 1196, 480, 318, 508, 346, 456, 1192,
      480, 346, 456, 374, 452, 346, 480, 374, 458, 342, 484, 346, 508, 346,
      456, 342, 512, 1164, 458, 1164, 508, 346, 456, 346, 480, 1190, 456, 342,
      484, 346, 506, 346, 456, 374, 452, 346, 508, 346, 458, 1164, 508, 346,
      458, 374, 452, 1168, 480, 374, 480, 318, 480, 374, 456, 346, 508, 318,
      480, 346, 484, 374, 480, 318, 484, 342, 484, 374, 480, 318, 484, 342,
      484, 346, 508, 318, 508, 346, 458, 346, 506, 318, 480, 374, 458, 346,
      506, 318, 480, 346, 484, 374, 480, 318, 482, 372, 456, 346, 508, 318,
      506, 348, 456, 342, 484, 346, 508, 318, 484, 374, 480, 318, 508, 318,
      484, 346, 508, 318, 480, 374, 456, 346, 508, 346, 480, 318, 480, 346,
      484, 374, 480, 320, 484, 1164, 508, 346, 458, 342, 512, 1164, 458, 1190,
      454, 346, 484, 1164, 508, 346, 458, 1164, 480, 350, 480, 374, 480};
  uint8_t state[16] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30, 0x81, 0x04, 0x00, 0x00,
      0x00, 0x00, 0x20, 0x2B};
  irsend.begin();
  irsend.reset();
  irsend.sendRaw(rawData, 259, 38000);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcBits, irsend.capture.bits);
  EXPECT_STATE_EQ(state, irsend.capture.state, irsend.capture.bits);
  ac.setRaw(irsend.capture.state, irsend.capture.bits / 8);
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_EQ(
      "Model: 1 (ARRAH2E), Id: 0, Power: On, Mode: 4 (Heat), Temp: 24C, "
      "Fan: 0 (Auto), Clean: Off, Filter: Off, 10C Heat: Off, Swing: 0 (Off), "
      "Command: N/A, Timer: Off",
      ac.toString());

  // Resend it using the state this time.
  irsend.reset();
  irsend.sendFujitsuAC(state, 16);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcBits, irsend.capture.bits);
  EXPECT_STATE_EQ(state, irsend.capture.state, irsend.capture.bits);
  EXPECT_EQ(
      "f38000d50"
      "m3324s1574"
      "m448s390m448s390m448s1182m448s390m448s1182m448s390m448s390m448s390"
      "m448s1182m448s1182m448s390m448s390m448s390m448s1182m448s1182m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s1182m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s1182m448s390m448s390m448s390"
      "m448s390m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182"
      "m448s1182m448s390m448s390m448s1182m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s1182m448s1182m448s390m448s390"
      "m448s1182m448s390m448s390m448s390m448s390m448s390m448s390m448s1182"
      "m448s390m448s390m448s1182m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s1182m448s390m448s390"
      "m448s1182m448s1182m448s390m448s1182m448s390m448s1182m448s390m448s390"
      "m448s8100", irsend.outputStr());
}

TEST(TestIRFujitsuACClass, toCommon) {
  IRFujitsuAC ac(kGpioUnused);
  ac.setMode(kFujitsuAcModeCool);
  ac.setTemp(20);
  ac.setFanSpeed(kFujitsuAcFanQuiet);
  ac.setSwing(kFujitsuAcSwingBoth);

  // Now test it.
  ASSERT_EQ(decode_type_t::FUJITSU_AC, ac.toCommon().protocol);
  ASSERT_EQ(fujitsu_ac_remote_model_t::ARRAH2E, ac.toCommon().model);
  ASSERT_TRUE(ac.toCommon().power);
  ASSERT_TRUE(ac.toCommon().celsius);
  ASSERT_EQ(20, ac.toCommon().degrees);
  ASSERT_TRUE(ac.toCommon().quiet);

  ASSERT_EQ(stdAc::opmode_t::kCool, ac.toCommon().mode);
  ASSERT_EQ(stdAc::fanspeed_t::kMin, ac.toCommon().fanspeed);
  ASSERT_EQ(stdAc::swingv_t::kAuto, ac.toCommon().swingv);
  ASSERT_EQ(stdAc::swingh_t::kAuto, ac.toCommon().swingh);
  // Unsupported.
  ASSERT_FALSE(ac.toCommon().filter);
  ASSERT_FALSE(ac.toCommon().clean);
  ASSERT_FALSE(ac.toCommon().turbo);
  ASSERT_FALSE(ac.toCommon().light);
  ASSERT_FALSE(ac.toCommon().econo);
  ASSERT_FALSE(ac.toCommon().beep);
  ASSERT_EQ(-1, ac.toCommon().sleep);
  ASSERT_EQ(-1, ac.toCommon().clock);

  // Check off mode which is special.
  ac.off();
  ASSERT_FALSE(ac.toCommon().power);
  ac.send();
  ac.stateReset();
  IRrecv irrecv(kGpioUnused);
  ac._irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&ac._irsend.capture));
  ASSERT_EQ(FUJITSU_AC, ac._irsend.capture.decode_type);
  ac.setRaw(ac._irsend.capture.state, ac._irsend.capture.bits / 8);

  // Now test it.
  EXPECT_EQ(    // Off mode technically has no temp, mode, fan, etc.
      "Model: 1 (ARRAH2E), Id: 0, Power: Off, Command: N/A",
      ac.toString());
  ASSERT_EQ(decode_type_t::FUJITSU_AC, ac.toCommon().protocol);
  ASSERT_EQ(fujitsu_ac_remote_model_t::ARRAH2E, ac.toCommon().model);
  ASSERT_FALSE(ac.toCommon().power);
  ASSERT_TRUE(ac.toCommon().celsius);
  ASSERT_EQ(16, ac.toCommon().degrees);
  ASSERT_FALSE(ac.toCommon().quiet);

  ASSERT_EQ(stdAc::opmode_t::kAuto, ac.toCommon().mode);
  ASSERT_EQ(stdAc::fanspeed_t::kAuto, ac.toCommon().fanspeed);
  ASSERT_EQ(stdAc::swingv_t::kOff, ac.toCommon().swingv);
  ASSERT_EQ(stdAc::swingh_t::kOff, ac.toCommon().swingh);
  // Unsupported.
  ASSERT_FALSE(ac.toCommon().filter);
  ASSERT_FALSE(ac.toCommon().clean);
  ASSERT_FALSE(ac.toCommon().turbo);
  ASSERT_FALSE(ac.toCommon().light);
  ASSERT_FALSE(ac.toCommon().econo);
  ASSERT_FALSE(ac.toCommon().beep);
  ASSERT_EQ(-1, ac.toCommon().sleep);
  ASSERT_EQ(-1, ac.toCommon().clock);
}

TEST(TestDecodeFujitsuAC, Issue716) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  IRFujitsuAC ac(kGpioUnused);

  // Powerful command from a raw data capture.
  // Capture as supplied by u4mzu4
  uint16_t rawData[115] = {
      3320, 1610, 432, 406, 432, 406, 432, 1220, 432, 406, 432, 1192, 458, 406,
      432, 406, 432, 406, 432, 1218, 432, 1220, 432, 406, 432, 406, 432, 406,
      432, 1192, 458, 1192, 460, 406, 432, 406, 432, 406, 432, 406, 432, 406,
      432, 406, 432, 406, 432, 408, 432, 406, 432, 406, 430, 406, 432, 406, 432,
      406, 432, 1190, 460, 406, 432, 408, 430, 406, 432, 406, 432, 406, 432,
      406, 432, 406, 434, 1192, 458, 406, 432, 406, 432, 406, 432, 1194, 458,
      406, 432, 406, 432, 1194, 456, 1196, 454, 1220, 432, 406, 432, 406, 432,
      408, 430, 1194, 458, 1194, 456, 406, 432, 406, 430, 406, 432, 1194, 458,
      1194, 458};  // FUJITSU_AC
  uint8_t powerful[kFujitsuAcStateLengthShort] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0x39, 0xC6};
  irsend.begin();
  irsend.reset();
  irsend.sendRaw(rawData, 115, 38000);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcStateLengthShort * 8, irsend.capture.bits);
  EXPECT_STATE_EQ(powerful, irsend.capture.state, irsend.capture.bits);
  ac.setRaw(irsend.capture.state, irsend.capture.bits / 8);
  EXPECT_EQ(fujitsu_ac_remote_model_t::ARREB1E, ac.getModel());
  EXPECT_EQ(kFujitsuAcStateLengthShort, ac.getStateLength());
  EXPECT_EQ("Model: 3 (ARREB1E), Id: 0, Power: On, Mode: 0 (Auto), Temp: 16C, "
            "Fan: 0 (Auto), Clean: Off, Filter: Off, Swing: 0 (Off), "
            "Command: Powerful, Outside Quiet: Off, "
            "Timer: Off",
            ac.toString());

  // Economy (just from the state)
  uint8_t econo[kFujitsuAcStateLengthShort] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0x09, 0xF6};
  // Make sure we can't accidentally inherit the correct model.
  ASSERT_NE(fujitsu_ac_remote_model_t::ARDB1,
            fujitsu_ac_remote_model_t::ARREB1E);
  ac.setModel(fujitsu_ac_remote_model_t::ARDB1);
  ac.setRaw(econo, kFujitsuAcStateLengthShort);
  EXPECT_EQ(fujitsu_ac_remote_model_t::ARREB1E, ac.getModel());
  EXPECT_EQ(kFujitsuAcStateLengthShort, ac.getStateLength());
  EXPECT_EQ("Model: 3 (ARREB1E), Id: 0, Power: On, Mode: 0 (Auto), Temp: 16C, "
            "Fan: 0 (Auto), Clean: Off, Filter: Off, Swing: 0 (Off), "
            "Command: Econo, Outside Quiet: Off, "
            "Timer: Off",
            ac.toString());
}

TEST(TestIRFujitsuACClass, OutsideQuiet) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  IRFujitsuAC ac(kGpioUnused);

  ASSERT_NE(fujitsu_ac_remote_model_t::ARDB1,
    fujitsu_ac_remote_model_t::ARREB1E);
  ASSERT_NE(fujitsu_ac_remote_model_t::ARRAH2E,
    fujitsu_ac_remote_model_t::ARREB1E);
  // States as supplied by u4mzu4
  // https://github.com/crankyoldgit/IRremoteESP8266/issues/716#issuecomment-495852309
  uint8_t off[kFujitsuAcStateLength] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
      0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x20, 0x2F};
  uint8_t on[kFujitsuAcStateLength] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
      0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xAF};
  // Make sure we can't accidentally inherit the correct model.
  ac.setModel(fujitsu_ac_remote_model_t::ARDB1);
  ac.setRaw(off, kFujitsuAcStateLength);
  EXPECT_EQ(fujitsu_ac_remote_model_t::ARRAH2E, ac.getModel());
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_FALSE(ac.getOutsideQuiet());
  // We can really only tell the difference between ARRAH2E & ARREB1E if
  // the option is set. Otheriwse they appear the same.
  EXPECT_EQ(
      "Model: 1 (ARRAH2E), Id: 0, Power: On, Mode: 1 (Cool), Temp: 24C, "
      "Fan: 0 (Auto), Clean: Off, Filter: Off, 10C Heat: Off, Swing: 0 (Off), "
      "Command: N/A, Timer: Off", ac.toString());
  ac.setModel(fujitsu_ac_remote_model_t::ARREB1E);
  EXPECT_EQ(
      "Model: 3 (ARREB1E), Id: 0, Power: On, Mode: 1 (Cool), Temp: 24C, "
      "Fan: 0 (Auto), Clean: Off, Filter: Off, Swing: 0 (Off), "
      "Command: N/A, Outside Quiet: Off, Timer: Off",
      ac.toString());

  // Make sure we can't accidentally inherit the correct model.
  ac.setModel(fujitsu_ac_remote_model_t::ARDB1);
  ac.setRaw(on, kFujitsuAcStateLength);
  EXPECT_EQ(fujitsu_ac_remote_model_t::ARREB1E, ac.getModel());
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_TRUE(ac.getOutsideQuiet());
  EXPECT_EQ(
      "Model: 3 (ARREB1E), Id: 0, Power: On, Mode: 1 (Cool), Temp: 24C, "
      "Fan: 0 (Auto), Clean: Off, Filter: Off, Swing: 0 (Off), "
      "Command: N/A, Outside Quiet: On, Timer: Off",
      ac.toString());

  ac.setOutsideQuiet(false);
  EXPECT_FALSE(ac.getOutsideQuiet());
  ac.setOutsideQuiet(true);
  EXPECT_TRUE(ac.getOutsideQuiet());
  ac.setOutsideQuiet(false);
  EXPECT_FALSE(ac.getOutsideQuiet());
}

TEST(TestIRFujitsuACClass, toggleSwing) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  IRFujitsuAC ac(kGpioUnused);

  ac.begin();
  ac.setModel(ARJW2);
  ac.setSwing(kFujitsuAcSwingOff);
  ac.setCmd(kFujitsuAcCmdStayOn);
  ASSERT_EQ(kFujitsuAcSwingOff, ac.getSwing());
  ac.toggleSwingHoriz();
  EXPECT_EQ(kFujitsuAcCmdToggleSwingHoriz, ac.getCmd());
  EXPECT_EQ(kFujitsuAcSwingHoriz, ac.getSwing());
  ac.toggleSwingHoriz();
  EXPECT_EQ(kFujitsuAcCmdToggleSwingHoriz, ac.getCmd());
  EXPECT_EQ(kFujitsuAcSwingOff, ac.getSwing());
  ac.toggleSwingVert();
  EXPECT_EQ(kFujitsuAcCmdToggleSwingVert, ac.getCmd());
  EXPECT_EQ(kFujitsuAcSwingVert, ac.getSwing());
  ac.toggleSwingVert();
  EXPECT_EQ(kFujitsuAcCmdToggleSwingVert, ac.getCmd());
  EXPECT_EQ(kFujitsuAcSwingOff, ac.getSwing());

  // Both
  ac.toggleSwingHoriz();
  EXPECT_EQ(kFujitsuAcCmdToggleSwingHoriz, ac.getCmd());
  ac.toggleSwingVert();
  EXPECT_EQ(kFujitsuAcCmdToggleSwingVert, ac.getCmd());
  EXPECT_EQ(kFujitsuAcSwingBoth, ac.getSwing());
  ac.toggleSwingHoriz();
  EXPECT_EQ(kFujitsuAcCmdToggleSwingHoriz, ac.getCmd());
  EXPECT_EQ(kFujitsuAcSwingVert, ac.getSwing());
  ac.toggleSwingHoriz();
  EXPECT_EQ(kFujitsuAcSwingBoth, ac.getSwing());

  EXPECT_EQ(
      "Model: 4 (ARJW2), Id: 0, Power: On, Mode: 1 (Cool), Temp: 24C, "
      "Fan: 1 (High), Command: Toggle Swing(H)",
      ac.toString());

  // Test without the update set.
  ac.toggleSwingHoriz(false);
  EXPECT_EQ(kFujitsuAcSwingBoth, ac.getSwing());
  EXPECT_EQ(kFujitsuAcCmdToggleSwingHoriz, ac.getCmd());
  ac.toggleSwingVert(false);
  EXPECT_EQ(kFujitsuAcSwingBoth, ac.getSwing());
  EXPECT_EQ(kFujitsuAcCmdToggleSwingVert, ac.getCmd());
}

TEST(TestDecodeFujitsuAC, Issue726) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  IRFujitsuAC ac(kGpioUnused);

  // fan:auto mode:auto temp:24 power：on
  // Capture as supplied by huexpub
  // Rawdata was very messy. Had to use `./auto_analyse_raw_data.py -r 250` to
  // get it to parse due to timings being above tolerances.
  uint8_t auto_auto_on_24[kFujitsuAcStateLength] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
      0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x2F};
  irsend.begin();
  irsend.reset();
  irsend.sendFujitsuAC(auto_auto_on_24, 16);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcStateLength * 8, irsend.capture.bits);
  EXPECT_STATE_EQ(auto_auto_on_24, irsend.capture.state, irsend.capture.bits);
  ac.setRaw(irsend.capture.state, irsend.capture.bits / 8);
  EXPECT_EQ(fujitsu_ac_remote_model_t::ARRAH2E, ac.getModel());
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_EQ(
      "Model: 1 (ARRAH2E), Id: 0, Power: On, Mode: 0 (Auto), Temp: 24C, "
      "Fan: 0 (Auto), Clean: Off, Filter: Off, 10C Heat: Off, Swing: 0 (Off), "
      "Command: N/A, Timer: Off",
      ac.toString());
}

TEST(TestIRFujitsuACClass, Clean) {
  IRFujitsuAC ac(kGpioUnused);
  // Data from:
  //  https://docs.google.com/spreadsheets/d/1f8EGfIbBUo2B-CzUFdrgKQprWakoYNKM80IKZN4KXQE/edit#gid=646887633&range=A27:B30
  uint8_t clean_off[kFujitsuAcStateLength] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
      0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x10};
  uint8_t clean_on[kFujitsuAcStateLength] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
      0xA0, 0x08, 0x00, 0x00, 0x00, 0x00, 0x20, 0x08};
  ac.setRaw(clean_on, kFujitsuAcStateLength);
  EXPECT_TRUE(ac.getClean());
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_EQ(
      "Model: 5 (ARRY4), Id: 0, Power: On, Mode: 0 (Auto), Temp: 26C, "
      "Fan: 0 (Auto), Clean: On, Filter: Off, Swing: 0 (Off), Command: N/A",
      ac.toString());
  ac.setClean(false);
  EXPECT_FALSE(ac.getClean());
  EXPECT_STATE_EQ(clean_off, ac.getRaw(), ac.getStateLength() * 8)
  ac.setClean(true);
  EXPECT_TRUE(ac.getClean());
  EXPECT_STATE_EQ(clean_on, ac.getRaw(), ac.getStateLength() * 8)
  ac.setRaw(clean_off, kFujitsuAcStateLength);
  EXPECT_FALSE(ac.getClean());
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_EQ(
      "Model: 1 (ARRAH2E), Id: 0, Power: On, Mode: 0 (Auto), Temp: 26C, "
      "Fan: 0 (Auto), Clean: Off, Filter: Off, 10C Heat: Off, Swing: 0 (Off), "
      "Command: N/A, Timer: Off",
      ac.toString());
  // Now it is in ARRAH2E model mode, it shouldn't accept setting it on.
  ac.setClean(true);
  EXPECT_EQ(fujitsu_ac_remote_model_t::ARRAH2E, ac.getModel());
  EXPECT_EQ(
      "Model: 1 (ARRAH2E), Id: 0, Power: On, Mode: 0 (Auto), Temp: 26C, "
      "Fan: 0 (Auto), Clean: Off, Filter: Off, 10C Heat: Off, Swing: 0 (Off), "
      "Command: N/A, Timer: Off",
      ac.toString());
  // But ARRY4 does.
  ac.setModel(fujitsu_ac_remote_model_t::ARRY4);
  EXPECT_TRUE(ac.getClean());
  EXPECT_EQ(
      "Model: 5 (ARRY4), Id: 0, Power: On, Mode: 0 (Auto), Temp: 26C, "
      "Fan: 0 (Auto), Clean: On, Filter: Off, Swing: 0 (Off), Command: N/A",
      ac.toString());
}

TEST(TestIRFujitsuACClass, Filter) {
  IRFujitsuAC ac(kGpioUnused);
  // Data from:
  //  https://docs.google.com/spreadsheets/d/1f8EGfIbBUo2B-CzUFdrgKQprWakoYNKM80IKZN4KXQE/edit#gid=646887633&range=A27:B30
  uint8_t filter_on[kFujitsuAcStateLength] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
      0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x07};
  uint8_t filter_off[kFujitsuAcStateLength] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
      0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x10};
  ac.setRaw(filter_on, kFujitsuAcStateLength);
  EXPECT_TRUE(ac.getFilter());
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_EQ(
      "Model: 5 (ARRY4), Id: 0, Power: On, Mode: 0 (Auto), Temp: 26C, "
      "Fan: 0 (Auto), Clean: Off, Filter: On, Swing: 0 (Off), Command: N/A",
      ac.toString());
  ac.setFilter(false);
  EXPECT_FALSE(ac.getFilter());
  ac.setFilter(true);
  EXPECT_TRUE(ac.getFilter());
  ac.setRaw(filter_off, kFujitsuAcStateLength);
  EXPECT_FALSE(ac.getFilter());
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_EQ(
      "Model: 1 (ARRAH2E), Id: 0, Power: On, Mode: 0 (Auto), Temp: 26C, "
      "Fan: 0 (Auto), Clean: Off, Filter: Off, 10C Heat: Off, Swing: 0 (Off), "
      "Command: N/A, Timer: Off",
      ac.toString());
  // Now it is in ARRAH2E model mode, it shouldn't accept setting it on.
  ac.setFilter(true);
  EXPECT_FALSE(ac.getFilter());
  // But ARRY4 does.
  ac.setModel(fujitsu_ac_remote_model_t::ARRY4);
  EXPECT_TRUE(ac.getFilter());
  EXPECT_EQ(
      "Model: 5 (ARRY4), Id: 0, Power: On, Mode: 0 (Auto), Temp: 26C, "
      "Fan: 0 (Auto), Clean: Off, Filter: On, Swing: 0 (Off), Command: N/A",
      ac.toString());
}

TEST(TestIRFujitsuACClass, Timers) {
  IRFujitsuAC ac(kGpioUnused);
  // Data from:
  //  https://github.com/crankyoldgit/IRremoteESP8266/issues/1255#issuecomment-686445720
  const uint8_t timer_on_12h[kFujitsuAcStateLength] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
      0xA0, 0x30, 0x01, 0x00, 0x00, 0xAD, 0x20, 0x32};
  ac.setRaw(timer_on_12h, kFujitsuAcStateLength);
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_EQ(kFujitsuAcOnTimer, ac.getTimerType());
  EXPECT_EQ(12 * 60, ac.getOnTimer());
  EXPECT_EQ(0, ac.getOffSleepTimer());
  EXPECT_EQ(
      "Model: 1 (ARRAH2E), Id: 0, Power: On, Mode: 0 (Auto), Temp: 26C, "
      "Fan: 1 (High), Clean: Off, Filter: Off, 10C Heat: Off, Swing: 0 (Off), "
      "Command: N/A, On Timer: 12:00",
      ac.toString());

  const uint8_t timer_on_8h30m[kFujitsuAcStateLength] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
      0xA0, 0x30, 0x01, 0x00, 0xE0, 0x9F, 0x20, 0x60};
  ac.setRaw(timer_on_8h30m, kFujitsuAcStateLength);
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_EQ(kFujitsuAcOnTimer, ac.getTimerType());
  EXPECT_EQ(8 * 60 + 30, ac.getOnTimer());
  EXPECT_EQ(0, ac.getOffSleepTimer());
  EXPECT_EQ(
      "Model: 1 (ARRAH2E), Id: 0, Power: On, Mode: 0 (Auto), Temp: 26C, "
      "Fan: 1 (High), Clean: Off, Filter: Off, 10C Heat: Off, "
      "Swing: 0 (Off), Command: N/A, On Timer: 08:30",
      ac.toString());

  // TIMER OFF 11H
  const uint8_t timer_off_11h[kFujitsuAcStateLength] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
      0xA0, 0x20, 0x01, 0x94, 0x0A, 0x00, 0x20, 0x51};
  ac.setRaw(timer_off_11h, kFujitsuAcStateLength);
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_EQ(kFujitsuAcOffTimer, ac.getTimerType());
  EXPECT_EQ(11 * 60, ac.getOffSleepTimer());
  EXPECT_EQ(0, ac.getOnTimer());
  EXPECT_EQ(
      "Model: 1 (ARRAH2E), Id: 0, Power: On, Mode: 0 (Auto), Temp: 26C, "
      "Fan: 1 (High), Clean: Off, Filter: Off, 10C Heat: Off, "
      "Swing: 0 (Off), Command: N/A, Off Timer: 11:00",
      ac.toString());

  // TIMER OFF 0.5H
  const uint8_t timer_off_30m[kFujitsuAcStateLength] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
      0xA0, 0x20, 0x01, 0x1E, 0x08, 0x00, 0x20, 0xC9};
  ac.setRaw(timer_off_30m, kFujitsuAcStateLength);
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_EQ(kFujitsuAcOffTimer, ac.getTimerType());
  EXPECT_EQ(30, ac.getOffSleepTimer());
  EXPECT_EQ(0, ac.getOnTimer());
  EXPECT_EQ(
      "Model: 1 (ARRAH2E), Id: 0, Power: On, Mode: 0 (Auto), Temp: 26C, "
      "Fan: 1 (High), Clean: Off, Filter: Off, 10C Heat: Off, "
      "Swing: 0 (Off), Command: N/A, Off Timer: 00:30",
      ac.toString());

  // TIMER SLEEP 3H
  const uint8_t timer_sleep_3h[kFujitsuAcStateLength] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
      0xA0, 0x10, 0x01, 0xB4, 0x08, 0x00, 0x20, 0x43};
  ac.setRaw(timer_sleep_3h, kFujitsuAcStateLength);
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_EQ(kFujitsuAcSleepTimer, ac.getTimerType());
  EXPECT_EQ(3 * 60, ac.getOffSleepTimer());
  EXPECT_EQ(0, ac.getOnTimer());
  EXPECT_EQ(
      "Model: 1 (ARRAH2E), Id: 0, Power: On, Mode: 0 (Auto), Temp: 26C, "
      "Fan: 1 (High), Clean: Off, Filter: Off, 10C Heat: Off, "
      "Swing: 0 (Off), Command: N/A, Sleep Timer: 03:00",
      ac.toString());

  // Re-construct a known timer state from scratch.
  ac.stateReset();
  ac.setModel(fujitsu_ac_remote_model_t::ARRAH2E);
  ac.setPower(true);
  ac.setMode(kFujitsuAcModeAuto);
  ac.setTemp(26);
  ac.setFanSpeed(1);
  ac.setClean(false);
  ac.setFilter(false);
  ac.setSwing(0);

  ac.setOffTimer(30);
  EXPECT_EQ(
      "Model: 1 (ARRAH2E), Id: 0, Power: On, Mode: 0 (Auto), Temp: 26C, "
      "Fan: 1 (High), Clean: Off, Filter: Off, 10C Heat: Off, "
      "Swing: 0 (Off), Command: N/A, Off Timer: 00:30",
      ac.toString());
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_STATE_EQ(timer_off_30m, ac.getRaw(), ac.getStateLength() * 8);

  ac.setOnTimer(12 * 60);
  EXPECT_EQ(
      "Model: 1 (ARRAH2E), Id: 0, Power: On, Mode: 0 (Auto), Temp: 26C, "
      "Fan: 1 (High), Clean: Off, Filter: Off, 10C Heat: Off, "
      "Swing: 0 (Off), Command: N/A, On Timer: 12:00",
      ac.toString());
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_EQ(12 * 60, ac.getOnTimer());
  EXPECT_TRUE(ac.getOnTimer());
  EXPECT_STATE_EQ(timer_on_12h, ac.getRaw(), ac.getStateLength() * 8);
  EXPECT_EQ(12 * 60, ac.getOnTimer());
  EXPECT_TRUE(ac.getOnTimer());

  ac.setSleepTimer(3 * 60);
  EXPECT_EQ(
      "Model: 1 (ARRAH2E), Id: 0, Power: On, Mode: 0 (Auto), Temp: 26C, "
      "Fan: 1 (High), Clean: Off, Filter: Off, 10C Heat: Off, "
      "Swing: 0 (Off), Command: N/A, Sleep Timer: 03:00",
      ac.toString());
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());
  EXPECT_STATE_EQ(timer_sleep_3h, ac.getRaw(), ac.getStateLength() * 8);
}

TEST(TestIRFujitsuACClass, ARREW4E) {
  IRFujitsuAC ac(kGpioUnused);

  uint8_t on_18_cool_auto[kFujitsuAcStateLength] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x31,
      0x50, 0x01, 0x00, 0x21, 0x03, 0x20, 0x20, 0x1A};

  EXPECT_TRUE(ac.validChecksum(on_18_cool_auto, kFujitsuAcStateLength));
  ac.setRaw(on_18_cool_auto, kFujitsuAcStateLength);
  EXPECT_EQ(0, ac.getId());
  EXPECT_EQ(fujitsu_ac_remote_model_t::ARREW4E, ac.getModel());
  EXPECT_EQ(18, ac.getTemp());

  uint8_t mode_C_power_on_18[kFujitsuAcStateLength] = {
      0x14, 0x63, 0x20, 0x10, 0x10, 0xFE, 0x09, 0x31,
      0x51, 0x01, 0x00, 0x17, 0x07, 0x54, 0x20, 0xEB};
  EXPECT_TRUE(ac.validChecksum(mode_C_power_on_18, kFujitsuAcStateLength));
  ac.setRaw(mode_C_power_on_18, kFujitsuAcStateLength);
  EXPECT_EQ(2, ac.getId());
  EXPECT_EQ(fujitsu_ac_remote_model_t::ARREW4E, ac.getModel());
  EXPECT_EQ(18, ac.getTemp());

  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);

  irsend.begin();
  irsend.reset();
  irsend.sendFujitsuAC(mode_C_power_on_18, kFujitsuAcStateLength);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcStateLength * 8, irsend.capture.bits);
  EXPECT_STATE_EQ(mode_C_power_on_18, irsend.capture.state,
                  irsend.capture.bits);
}

TEST(TestDecodeFujitsuAC, Issue1455) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.begin();
  irsend.reset();
  uint16_t rawData[259] = {
      3220, 1700, 354, 446, 380, 448, 380, 1296, 352, 446, 382, 1296, 354, 448,
      380, 448, 378, 446, 382, 1296, 352, 1296, 354, 448, 378, 446, 380, 446,
      382, 1294, 354, 1270, 380, 448, 380, 446, 380, 448, 380, 446, 380, 450,
      378, 448, 380, 446, 380, 448, 380, 448, 380, 448, 380, 450, 376, 450, 380,
      448, 378, 1298, 352, 448, 378, 448, 380, 448, 380, 448, 378, 450, 378,
      450, 378, 448, 378, 1296, 354, 446, 382, 446, 380, 448, 378, 448, 380,
      1296, 352, 1296, 354, 1296, 354, 1272, 376, 1272, 378, 1296, 354, 1294,
      354, 1296, 354, 446, 380, 448, 378, 1296, 354, 448, 378, 448, 378, 448,
      380, 446, 380, 1272, 378, 446, 380, 450, 378, 448, 378, 1296, 354, 1296,
      354, 446, 380, 448, 378, 448, 378, 446, 382, 446, 380, 1296, 354, 1296,
      354, 446, 380, 1296, 354, 446, 380, 446, 380, 446, 380, 1294, 354, 448,
      380, 448, 380, 448, 380, 448, 380, 446, 380, 448, 380, 446, 380, 448,
      380, 446, 380, 446, 380, 448, 380, 446, 380, 448, 380, 448, 380, 446, 380,
      1296, 352, 446, 380, 1296, 354, 446, 380, 448, 380, 448, 380, 1296, 354,
      448, 378, 448, 380, 446, 380, 446, 382, 446, 380, 446, 382, 446, 380,
      1272, 378, 446, 380, 446, 382, 1294, 354, 446, 382, 1294, 354, 446, 382,
      446, 382, 446, 380, 448, 380, 448, 380, 448, 380, 448, 378, 1296, 354,
      446, 382, 446, 380, 1296, 354, 446, 382, 1296, 354, 446, 382, 1294, 354,
      446, 382, 446, 380, 446, 382};  // UNKNOWN 8383D7DE
  irsend.sendRaw(rawData, 259, 38);
  irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcStateLength * 8, irsend.capture.bits);
  EXPECT_EQ(
      "Model: 6 (ARREW4E), Id: 0, Power: On, Mode: 4 (Heat), Temp: 19C, "
      "Fan: 0 (Auto), 10C Heat: Off, Swing: 0 (Off), Command: N/A, "
      "Outside Quiet: Off, Timer: Off",
      IRAcUtils::resultAcToString(&irsend.capture));
  stdAc::state_t r, p;
  ASSERT_TRUE(IRAcUtils::decodeToState(&irsend.capture, &r, &p));
}

TEST(TestIRFujitsuACClass, Heat10Deg) {
  IRFujitsuAC ac(kGpioUnused);
  const uint8_t heat_on[kFujitsuAcStateLength] = {
      0x14, 0x63, 0x10, 0x10, 0x10, 0xFE, 0x09, 0x31,
      0x69, 0x0B, 0x00, 0x23, 0x06, 0x23, 0x20, 0xEF};
  ac.setRaw(heat_on, kFujitsuAcStateLength);
  EXPECT_EQ(
      "Model: 6 (ARREW4E), Id: 1, Power: On, Mode: 3 (Fan), Temp: 10C, "
      "Fan: 0 (Auto), 10C Heat: On, Swing: 0 (Off), Command: N/A, "
      "Outside Quiet: Off, Timer: Off",
      ac.toString());
  ac.stateReset();
  ac.setModel(fujitsu_ac_remote_model_t::ARREW4E);
  ac.setId(1);
  ac.setMode(kFujitsuAcModeFan);
  ac.setTemp(21);
  ac.setFanSpeed(kFujitsuAcFanAuto);
  ac.setSwing(0);
  ac.setOutsideQuiet(false);
  ac.setPower(true);
  ac.set10CHeat(true);
  EXPECT_TRUE(ac.get10CHeat());
  EXPECT_EQ(
      "Model: 6 (ARREW4E), Id: 1, Power: On, Mode: 3 (Fan), Temp: 10C, "
      "Fan: 0 (Auto), 10C Heat: On, Swing: 0 (Off), Command: N/A, "
      "Outside Quiet: Off, Timer: Off",
      ac.toString());
  EXPECT_EQ(kFujitsuAcStateLength, ac.getStateLength());

  ac.set10CHeat(false);
  EXPECT_FALSE(ac.get10CHeat());
  EXPECT_EQ(
      "Model: 6 (ARREW4E), Id: 1, Power: On, Mode: 3 (Fan), Temp: 21C, "
      "Fan: 0 (Auto), 10C Heat: Off, Swing: 0 (Off), Command: N/A, "
      "Outside Quiet: Off, Timer: Off",
      ac.toString());

  // For https://github.com/crankyoldgit/IRremoteESP8266/issues/1455#issuecomment-817339816
  ac.set10CHeat(true);
  EXPECT_TRUE(ac.get10CHeat());
  ac.setFanSpeed(kFujitsuAcFanHigh);
  ac.setSwing(kFujitsuAcSwingVert);
  EXPECT_FALSE(ac.get10CHeat());
  ac.set10CHeat(false);
  EXPECT_EQ(kFujitsuAcFanHigh, ac.getFanSpeed());
  EXPECT_EQ(kFujitsuAcSwingVert, ac.getSwing());
  EXPECT_FALSE(ac.get10CHeat());
  ac.set10CHeat(true);
  EXPECT_TRUE(ac.get10CHeat());
  EXPECT_EQ(kFujitsuAcFanAuto, ac.getFanSpeed());
  EXPECT_EQ(kFujitsuAcSwingOff, ac.getSwing());
}

TEST(TestUtils, Housekeeping) {
  ASSERT_EQ("FUJITSU_AC", typeToString(decode_type_t::FUJITSU_AC));
  ASSERT_EQ(decode_type_t::FUJITSU_AC, strToDecodeType("FUJITSU_AC"));
  ASSERT_TRUE(hasACState(decode_type_t::FUJITSU_AC));
  ASSERT_TRUE(IRac::isProtocolSupported(decode_type_t::FUJITSU_AC));
  ASSERT_EQ(0, IRsend::defaultBits(decode_type_t::FUJITSU_AC));  // No default
  ASSERT_EQ(kNoRepeat, IRsend::minRepeats(decode_type_t::FUJITSU_AC));

  ASSERT_EQ("FUJITSU_AC264", typeToString(decode_type_t::FUJITSU_AC264));
  ASSERT_EQ(decode_type_t::FUJITSU_AC264, strToDecodeType("FUJITSU_AC264"));
  ASSERT_TRUE(hasACState(decode_type_t::FUJITSU_AC264));
  ASSERT_TRUE(IRac::isProtocolSupported(decode_type_t::FUJITSU_AC264));
  ASSERT_EQ(264, IRsend::defaultBits(decode_type_t::FUJITSU_AC264));
  ASSERT_EQ(kNoRepeat, IRsend::minRepeats(decode_type_t::FUJITSU_AC264));
}

TEST(TestIRFujitsuACClass, Temperature) {
  IRFujitsuAC ac(kGpioUnused);
  // Most models
  // Celsius
  ac.setModel(fujitsu_ac_remote_model_t::ARRAH2E);
  ac.setTemp(kFujitsuAcMinTemp);
  EXPECT_TRUE(ac.getCelsius());
  EXPECT_EQ(kFujitsuAcMinTemp, ac.getTemp());
  ac.setTemp(kFujitsuAcMaxTemp);
  EXPECT_EQ(kFujitsuAcMaxTemp, ac.getTemp());
  ac.setTemp(kFujitsuAcMinTemp - 1);
  EXPECT_TRUE(ac.getCelsius());
  EXPECT_EQ(kFujitsuAcMinTemp, ac.getTemp());
  ac.setTemp(kFujitsuAcMaxTemp + 1);
  EXPECT_TRUE(ac.getCelsius());
  EXPECT_EQ(kFujitsuAcMaxTemp, ac.getTemp());
  // Fahrenheit (can't be used by most model, check it converts correctly)
  ac.setTemp(77, false);  // 77F is 25C
  EXPECT_TRUE(ac.getCelsius());
  EXPECT_EQ(25, ac.getTemp());

  // ARREW4E is different.
  ac.setModel(fujitsu_ac_remote_model_t::ARREW4E);
  ac.setTemp(kFujitsuAcMinTemp);
  EXPECT_TRUE(ac.getCelsius());
  EXPECT_EQ(kFujitsuAcMinTemp, ac.getTemp());
  ac.setTemp(kFujitsuAcMaxTemp);
  EXPECT_EQ(kFujitsuAcMaxTemp, ac.getTemp());
  ac.setTemp(kFujitsuAcMinTemp - 1);
  EXPECT_TRUE(ac.getCelsius());
  EXPECT_EQ(kFujitsuAcMinTemp, ac.getTemp());
  ac.setTemp(kFujitsuAcMaxTemp + 1);
  EXPECT_TRUE(ac.getCelsius());
  EXPECT_EQ(kFujitsuAcMaxTemp, ac.getTemp());
  ac.setTemp(22.5);
  EXPECT_TRUE(ac.getCelsius());
  EXPECT_EQ(22.5, ac.getTemp());
  // Fahrenheit
  ac.setTemp(77, false);
  EXPECT_FALSE(ac.getCelsius());
  EXPECT_EQ(77, ac.getTemp());

  // Real example
  const uint8_t arrew4e_22c[16] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x31,
      0x70, 0x01, 0x00, 0x20, 0x03, 0x58, 0x20, 0xC3};
  ac.setRaw(arrew4e_22c, 16);
  EXPECT_TRUE(ac.getCelsius());
  EXPECT_EQ(22, ac.getTemp());
  const uint8_t arrew4e_25_5c[16] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x31,
      0x8C, 0x01, 0x00, 0x21, 0x03, 0x12, 0x20, 0xEC};
  ac.setRaw(arrew4e_25_5c, 16);
  EXPECT_TRUE(ac.getCelsius());
  EXPECT_EQ(25.5, ac.getTemp());
  const uint8_t arrew4e_69f[16] = {
      0x14, 0x63, 0x20, 0x10, 0x10, 0xFE, 0x09, 0x31,
      0x66, 0x04, 0x00, 0x16, 0x01, 0x32, 0x20, 0xFC};
  ac.setRaw(arrew4e_69f, 16);
  EXPECT_EQ(fujitsu_ac_remote_model_t::ARREW4E, ac.getModel());
  EXPECT_FALSE(ac.getCelsius());
  EXPECT_EQ(69, ac.getTemp());
}

TEST(TestIRFujitsuACClass, ARREW4EShortCodes) {
  // ref: https://github.com/crankyoldgit/IRremoteESP8266/issues/1455#issuecomment-817339816
  IRFujitsuAC ac(kGpioUnused);
  ac.setId(3);
  ac.setModel(fujitsu_ac_remote_model_t::ARREW4E);

  const uint8_t off[kFujitsuAcStateLengthShort] = {
      0x14, 0x63, 0x30, 0x10, 0x10, 0x02, 0xFD};
  ac.off();
  ASSERT_EQ(kFujitsuAcStateLengthShort, ac.getStateLength());
  EXPECT_STATE_EQ(off, ac.getRaw(), kFujitsuAcStateLengthShort * 8);

  const uint8_t econo[kFujitsuAcStateLengthShort] = {
      0x14, 0x63, 0x30, 0x10, 0x10, 0x09, 0xF6};
  ac.setCmd(kFujitsuAcCmdEcono);
  ASSERT_EQ(kFujitsuAcStateLengthShort, ac.getStateLength());
  EXPECT_STATE_EQ(econo, ac.getRaw(), kFujitsuAcStateLengthShort * 8);

  const uint8_t powerful[kFujitsuAcStateLengthShort] = {
      0x14, 0x63, 0x30, 0x10, 0x10, 0x39, 0xC6};
  ac.setCmd(kFujitsuAcCmdPowerful);
  ASSERT_EQ(kFujitsuAcStateLengthShort, ac.getStateLength());
  EXPECT_STATE_EQ(powerful, ac.getRaw(), kFujitsuAcStateLengthShort * 8);

  const uint8_t stepvert[kFujitsuAcStateLengthShort] = {
      0x14, 0x63, 0x30, 0x10, 0x10, 0x6C, 0x93};
  ac.setCmd(kFujitsuAcCmdStepVert);
  ASSERT_EQ(kFujitsuAcStateLengthShort, ac.getStateLength());
  EXPECT_STATE_EQ(stepvert, ac.getRaw(), kFujitsuAcStateLengthShort * 8);
}

// https://github.com/crankyoldgit/IRremoteESP8266/discussions/1701#discussioncomment-1910164
TEST(TestIRFujitsuACClass, Discussion1701) {
  IRFujitsuAC ac(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  IRac irac(kGpioUnused);

  const String expected_raw_output =
      "f38000d50"
      "m3324s1574"
      "m448s390m448s390m448s1182m448s390m448s1182m448s390m448s390m448s390"
      "m448s1182m448s1182m448s390m448s390m448s390m448s1182m448s1182m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s1182m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s1182m448s390m448s390m448s390"
      "m448s390m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182"
      "m448s1182m448s390m448s390m448s1182m448s390m448s390m448s390m448s390"
      "m448s1182m448s390m448s390m448s390m448s1182m448s1182m448s390m448s390"
      "m448s1182m448s390m448s390m448s390m448s390m448s390m448s390m448s1182"
      "m448s1182m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s1182m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s1182m448s1182m448s390m448s390m448s1182m448s390"
      "m448s8100";
  const String expected_arrew4e_str =
      "Model: 6 (ARREW4E), Id: 0, Power: On, Mode: 1 (Cool), Temp: 24C, "
      "Fan: 1 (High), 10C Heat: Off, Swing: 0 (Off), Command: N/A, "
      "Outside Quiet: Off, Timer: Off";
  const uint8_t expected_arrew4e_state[kFujitsuAcStateLength] =
      {0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x31,
       0x81, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x4C};

  // Method used in `TurnOnFujitsuAC`
  ac.begin();
  ac.setModel(ARREW4E);
  ac.setSwing(kFujitsuAcSwingOff);
  ac.setMode(kFujitsuAcModeCool);
  ac.setFanSpeed(kFujitsuAcFanHigh);
  ac.setTemp(24);  // 24C
  ac.setCmd(kFujitsuAcCmdTurnOn);
  ASSERT_EQ(expected_arrew4e_str, ac.toString());
  ac.send();
  ac._irsend.makeDecodeResult();
  // 260 = 16 (bytes) * 8 (bits) * 2 (per bit) + kHeader (2) + kFooter (2)
  EXPECT_EQ(1 + 260, ac._irsend.capture.rawlen);
  EXPECT_TRUE(irrecv.decode(&ac._irsend.capture));
  ASSERT_EQ(FUJITSU_AC, ac._irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcStateLength * 8, ac._irsend.capture.bits);
  EXPECT_EQ(expected_arrew4e_str,
            IRAcUtils::resultAcToString(&ac._irsend.capture));
  EXPECT_STATE_EQ(expected_arrew4e_state, ac._irsend.capture.state,
                  ac._irsend.capture.bits);
  EXPECT_EQ(expected_raw_output, ac._irsend.outputStr());

  // Now try to reproduce it via the IRac class.
  ac._irsend.reset();
  ac.stateReset();
  ASSERT_NE(expected_arrew4e_str, ac.toString());

  irac.fujitsu(&ac,
               ARREW4E,                     // Model
               true,                        // Power
               stdAc::opmode_t::kCool,      // Mode
               true,                        // Celsius
               24,                          // Degrees
               stdAc::fanspeed_t::kHigh,    // Fan speed
               stdAc::swingv_t::kOff,       // Vertical swing
               stdAc::swingh_t::kOff,       // Horizontal swing
               false,                       // Quiet
               false,                       // Turbo (Powerful)
               false,                       // Econo
               false,                       // Filter
               false);                      // Clean
  ASSERT_EQ(expected_arrew4e_str, ac.toString());
  ac._irsend.makeDecodeResult();
  // 260 = 16 (bytes) * 8 (bits) * 2 (per bit) + kHeader (2) + kFooter (2)
  EXPECT_EQ(1 + 260, ac._irsend.capture.rawlen);
  EXPECT_TRUE(irrecv.decode(&ac._irsend.capture));
  ASSERT_EQ(FUJITSU_AC, ac._irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcStateLength * 8, ac._irsend.capture.bits);
  EXPECT_EQ(expected_arrew4e_str,
            IRAcUtils::resultAcToString(&ac._irsend.capture));
  EXPECT_STATE_EQ(expected_arrew4e_state, ac._irsend.capture.state,
                  ac._irsend.capture.bits);
  EXPECT_EQ(expected_raw_output, ac._irsend.outputStr());
  // Success.
}

TEST(TestIRFujitsuACClass, toCommon_Issue1780HandlePrev) {
  IRFujitsuAC ac(kGpioUnused);
  ac.setMode(kFujitsuAcModeCool);
  ac.setTemp(20);
  ac.setFanSpeed(kFujitsuAcFanQuiet);
  ac.setSwing(kFujitsuAcSwingBoth);
  ac.on();
  ASSERT_TRUE(ac.toCommon().power);
  stdAc::state_t prev = ac.toCommon();  // Copy in the state.
  ac.off();
  ASSERT_FALSE(ac.toCommon().power);
  ac.send();  // This should send a short code.
  prev.degrees = 27;
  ac.stateReset();
  IRrecv irrecv(kGpioUnused);
  ac._irsend.makeDecodeResult();
  EXPECT_TRUE(irrecv.decode(&ac._irsend.capture));
  ASSERT_EQ(FUJITSU_AC, ac._irsend.capture.decode_type);
  ac.setRaw(ac._irsend.capture.state, ac._irsend.capture.bits / 8);
  ASSERT_EQ(decode_type_t::FUJITSU_AC, ac.toCommon().protocol);
  ASSERT_EQ(fujitsu_ac_remote_model_t::ARRAH2E, ac.toCommon().model);
  ASSERT_FALSE(ac.toCommon().power);
  ASSERT_TRUE(ac.toCommon().celsius);
  ASSERT_EQ(16, ac.toCommon().degrees);
  ASSERT_EQ(27, ac.toCommon(&prev).degrees);
  ASSERT_FALSE(ac.toCommon().quiet);

  ASSERT_EQ(stdAc::opmode_t::kAuto, ac.toCommon().mode);
  ASSERT_EQ(stdAc::opmode_t::kCool, ac.toCommon(&prev).mode);
  ASSERT_EQ(stdAc::fanspeed_t::kAuto, ac.toCommon().fanspeed);
  ASSERT_EQ(stdAc::fanspeed_t::kMin, ac.toCommon(&prev).fanspeed);
  ASSERT_EQ(stdAc::swingv_t::kOff, ac.toCommon().swingv);
  ASSERT_EQ(stdAc::swingh_t::kOff, ac.toCommon().swingh);
  // Unsupported.
  ASSERT_FALSE(ac.toCommon().filter);
  ASSERT_FALSE(ac.toCommon().clean);
  ASSERT_FALSE(ac.toCommon().turbo);
  ASSERT_FALSE(ac.toCommon().light);
  ASSERT_FALSE(ac.toCommon().econo);
  ASSERT_FALSE(ac.toCommon().beep);
  ASSERT_EQ(-1, ac.toCommon().sleep);
  ASSERT_EQ(-1, ac.toCommon().clock);

  stdAc::state_t result_inc_prev;
  ASSERT_TRUE(IRAcUtils::decodeToState(&ac._irsend.capture, &result_inc_prev,
                                       &prev));
  ASSERT_EQ(27, result_inc_prev.degrees);
  ASSERT_EQ(stdAc::opmode_t::kCool, result_inc_prev.mode);
  ASSERT_EQ(stdAc::fanspeed_t::kMin, result_inc_prev.fanspeed);
}

TEST(TestIRFujitsuACClass, Improve10CHeat) {
  IRFujitsuAC ac(kGpioUnused);
  // Data from https://docs.google.com/spreadsheets/d/1RdmJdOZ3zxYlLXzluKTp4L6VVdjDXKgizwwIyTTG8MA/edit#gid=0&range=G2
  const uint8_t Arrah2u_10CHeatOn[16] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
      0x41, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x20, 0x64};
  ASSERT_FALSE(ac.get10CHeat());
  ac.setRaw(Arrah2u_10CHeatOn, 16);
  ASSERT_TRUE(ac.get10CHeat());
  ASSERT_EQ(
      "Model: 1 (ARRAH2E), Id: 0, Power: On, Mode: 3 (Fan), Temp: 10C, "
      "Fan: 0 (Auto), Clean: Off, Filter: Off, 10C Heat: On, Swing: 0 (Off), "
      "Command: N/A, Timer: Off",
      ac.toString());
  EXPECT_EQ(decode_type_t::FUJITSU_AC, ac.toCommon().protocol);
  ASSERT_TRUE(ac.get10CHeat());
  EXPECT_EQ(fujitsu_ac_remote_model_t::ARRAH2E, ac.toCommon().model);
  EXPECT_EQ(kFujitsuAcMinHeat, ac.toCommon().degrees);

  ac.stateReset();
  // Data from https://docs.google.com/spreadsheets/d/1RdmJdOZ3zxYlLXzluKTp4L6VVdjDXKgizwwIyTTG8MA/edit#gid=0&range=G8
  const uint8_t Arreg1u_10CHeatOn[16] = {
      0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0x30,
      0x61, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x20, 0x44};
  ASSERT_FALSE(ac.get10CHeat());
  ac.setRaw(Arreg1u_10CHeatOn, 16);
  ASSERT_TRUE(ac.get10CHeat());
  ASSERT_EQ(
      "Model: 1 (ARRAH2E), Id: 0, Power: On, Mode: 3 (Fan), Temp: 10C, "
      "Fan: 0 (Auto), Clean: Off, Filter: Off, 10C Heat: On, Swing: 0 (Off), "
      "Command: N/A, Timer: Off",
      ac.toString());
  EXPECT_EQ(decode_type_t::FUJITSU_AC, ac.toCommon().protocol);
  ASSERT_TRUE(ac.get10CHeat());
  EXPECT_EQ(fujitsu_ac_remote_model_t::ARRAH2E, ac.toCommon().model);
  EXPECT_EQ(kFujitsuAcMinHeat, ac.toCommon().degrees);
}

// Tests for Fujitsu A/C 264 methods.
TEST(TestDecodeFujitsuAc264, RealExample) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  uint16_t rawData[531] = {3378, 1534, 498, 322, 526, 294, 524, 1116, 522, 302,
    492, 1166, 472, 350, 470, 352, 466, 352, 468, 1172, 468, 1176, 466, 352,
    466, 354, 466, 356, 466, 1176, 464, 1174, 468, 354, 466, 356, 464, 352,
    468, 356, 466, 352, 466, 354, 466, 356, 464, 354, 468, 352, 466, 356, 466,
    352, 466, 356, 464, 356, 464, 1178, 464, 356, 466, 356, 464, 356, 462, 356,
    466, 356, 464, 356, 462, 356, 464, 1178, 464, 356, 466, 356, 462, 358, 464,
    354, 464, 1176, 464, 1178, 464, 1180, 462, 1176, 454, 1188, 442, 1202, 440,
    1198, 442, 380, 464, 1176, 440, 382, 442, 1200, 438, 1202, 440, 380, 440,
    382, 438, 382, 440, 404, 416, 382, 440, 380, 440, 382, 438, 380, 440, 380,
    440, 1202, 440, 404, 414, 1202, 440, 382, 440, 380, 438, 1204, 440, 380,
    440, 382, 436, 400, 422, 1226, 416, 1226, 416, 404, 416, 406, 414, 406,
    414, 406, 414, 406, 416, 404, 416, 404, 418, 404, 414, 406, 416, 404, 418,
    404, 416, 404, 416, 404, 416, 404, 414, 404, 416, 406, 416, 404, 414, 404,
    416, 406, 414, 406, 414, 406, 416, 404, 414, 406, 414, 408, 414, 404, 414,
    408, 412, 406, 416, 404, 414, 406, 414, 408, 414, 406, 414, 408, 412, 406,
    414, 408, 412, 408, 414, 404, 414, 408, 412, 406, 414, 404, 416, 406, 412,
    408, 414, 404, 414, 410, 412, 408, 412, 406, 412, 408, 414, 408, 414, 406,
    412, 1230, 412, 408, 414, 408, 412, 1228, 414, 408, 412, 408, 412, 406,
    414, 408, 412, 1228, 414, 1228, 412, 408, 416, 408, 410, 408, 412, 408,
    414, 408, 412, 408, 412, 410, 410, 408, 410, 408, 412, 408, 414, 408, 412,
    408, 414, 408, 412, 1228, 414, 408, 412, 408, 410, 410, 410, 410, 412, 406,
    412, 408, 412, 410, 410, 1232, 410, 408, 410, 410, 410, 412, 386, 1254,
    412, 410, 408, 412, 412, 408, 410, 1230, 412, 1232, 410, 1230, 408, 1232,
    410, 1232, 386, 432, 386, 436, 408, 410, 386, 434, 404, 418, 386, 434, 386,
    432, 390, 432, 386, 1254, 388, 1254, 386, 434, 388, 432, 388, 434, 386,
    436, 384, 456, 362, 1256, 386, 434, 386, 434, 386, 434, 384, 436, 386, 434,
    386, 1256, 386, 458, 360, 434, 388, 1254, 386, 434, 388, 434, 386, 434,
    388, 434, 384, 460, 360, 436, 384, 458, 360, 436, 386, 460, 362, 434, 386,
    458, 360, 460, 360, 460, 362, 460, 360, 458, 362, 460, 360, 458, 360, 460,
    362, 460, 360, 460, 360, 460, 362, 456, 362, 458, 360, 462, 360, 458, 362,
    458, 362, 456, 364, 458, 362, 460, 360, 460, 362, 458, 362, 458, 360, 460,
    360, 458, 360, 1282, 360, 1280, 362, 1280, 360, 1282, 360, 1280, 360, 1280,
    360, 1282, 360, 1278, 362, 1280, 360, 1282, 360, 1280, 362, 1280, 360,
    1282, 360, 1280, 362, 1280, 360, 1282, 360, 1282, 362, 1280, 360, 1280,
    360, 1282, 358, 1282, 360, 1282, 360, 1282, 358, 1282, 360, 462, 358, 462,
    358, 462, 360, 460, 360, 462, 360, 484, 334, 462, 360, 462, 358, 458, 360,
    462, 356, 1282, 360, 1282, 360, 1308, 332, 486, 334, 1284, 358, 462, 360
    };  // FUJITSU_AC264

  uint8_t expectedState[kFujitsuAc264StateLength] = {
    0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x1A, 0x40,
    0x89, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12,
    0x06, 0x00, 0x01, 0x11, 0x1F, 0x60, 0x10, 0x24,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00,
    0x5C};

  irsend.begin();
  irsend.reset();
  irsend.sendRaw(rawData, 531, 38000);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC264, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAc264Bits, irsend.capture.bits);
  EXPECT_STATE_EQ(expectedState, irsend.capture.state, irsend.capture.bits);
  IRFujitsuAC264 ac(kGpioUnused);
  ac.setRaw(irsend.capture.state, irsend.capture.bits / 8);
  EXPECT_EQ("Power: On, Mode: 1 (Cool), Temp: 25C, Temp (Auto): 0C, "
    "Fan: 0 (Auto), Fan Angle: 15 (Stay), Swing: Off, Economy: Off, "
    "Clean: Off, Command: Cool, Current Time: 17:31, Sleep Timer: Off, "
    "On Timer: Off, Off Timer: Off",
    ac.toString());
}

TEST(TestDecodeFujitsuAc264, SyntheticExample) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);

  uint8_t sendCode[kFujitsuAc264StateLength] = {
    0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x1A, 0x40,
    0x89, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12,
    0x06, 0x00, 0x01, 0x11, 0x1F, 0x60, 0x10, 0x24,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00,
    0x5C};

  irsend.begin();
  irsend.reset();
  irsend.sendFujitsuAC264(sendCode);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  ASSERT_EQ(FUJITSU_AC264, irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAc264Bits, irsend.capture.bits);
  EXPECT_STATE_EQ(sendCode, irsend.capture.state, irsend.capture.bits);
  EXPECT_EQ(
      "f38000d50"
      "m3324s1574"
      "m448s390m448s390m448s1182m448s390m448s1182m448s390m448s390m448s390"
      "m448s1182m448s1182m448s390m448s390m448s390m448s1182m448s1182m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s1182m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s1182m448s390m448s390m448s390"
      "m448s390m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182"
      "m448s390m448s1182m448s390m448s1182m448s1182m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s1182m448s390"
      "m448s1182m448s390m448s390m448s1182m448s390m448s390m448s390m448s1182"
      "m448s1182m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s1182m448s390m448s390m448s1182m448s390m448s390m448s390"
      "m448s390m448s1182m448s1182m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s1182m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s1182m448s390m448s390m448s390m448s1182m448s390m448s390m448s390"
      "m448s1182m448s1182m448s1182m448s1182m448s1182m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s1182m448s1182m448s390"
      "m448s390m448s390m448s390m448s390m448s1182m448s390m448s390m448s390"
      "m448s390m448s390m448s1182m448s390m448s390m448s1182m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182"
      "m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182"
      "m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182m448s1182"
      "m448s390m448s390m448s390m448s390m448s390m448s390m448s390m448s390"
      "m448s390m448s390m448s1182m448s1182m448s1182m448s390m448s1182m448s390"
      "m448s8100",
      irsend.outputStr());
}

TEST(TestFujitsuAc264Class, toCommon) {
  IRFujitsuAC264 ac(kGpioUnused);
  ac.setPower(true);
  ac.setMode(kFujitsuAc264ModeCool);
  ac.setTemp(20);
  ac.setFanSpeed(kFujitsuAc264FanSpeedHigh);
  ac.setSwing(true);
  ac.setEcoFan(false);
  ac.setClock(1 * 60 + 23);   // "1:23"
  ac.setSleepTimer(2 * 60);
  // Now test it.
  ASSERT_EQ(decode_type_t::FUJITSU_AC264, ac.toCommon().protocol);
  ASSERT_EQ(-1, ac.toCommon().model);
  ASSERT_TRUE(ac.toCommon().power);
  ASSERT_TRUE(ac.toCommon().celsius);
  ASSERT_EQ(20, ac.toCommon().degrees);
  ASSERT_FALSE(ac.toCommon().quiet);
  ASSERT_EQ(stdAc::opmode_t::kCool, ac.toCommon().mode);
  ASSERT_EQ(stdAc::fanspeed_t::kMax, ac.toCommon().fanspeed);
  ASSERT_EQ(stdAc::swingv_t::kAuto, ac.toCommon().swingv);
  ASSERT_EQ(83, ac.toCommon().clock);
  ASSERT_EQ(120, ac.toCommon().sleep);
  // Unsupported.
  ASSERT_EQ(stdAc::swingh_t::kOff, ac.toCommon().swingh);
  ASSERT_FALSE(ac.toCommon().turbo);
  ASSERT_FALSE(ac.toCommon().clean);
  ASSERT_FALSE(ac.toCommon().econo);
  ASSERT_FALSE(ac.toCommon().light);
  ASSERT_FALSE(ac.toCommon().filter);
  ASSERT_FALSE(ac.toCommon().beep);
}

TEST(TestFujitsuAc264Class, Power) {
  IRFujitsuAC264 ac(kGpioUnused);
  ac.begin();

  // Value from getPower is not updated untill ac.send()
  ac.on();
  EXPECT_FALSE(ac.getPower());

  ac.on();
  ac.send();
  EXPECT_TRUE(ac.getPower());

  // Value from getPower is not updated untill ac.send()
  ac.off();
  EXPECT_TRUE(ac.getPower());

  ac.off();
  ac.send();
  EXPECT_FALSE(ac.getPower());

  // Value from getPower is not updated untill ac.send()
  ac.setPower(true);
  EXPECT_FALSE(ac.getPower());

  ac.setPower(true);
  ac.send();
  EXPECT_TRUE(ac.getPower());

  // Value from getPower is not updated untill ac.send()
  ac.setPower(false);
  EXPECT_TRUE(ac.getPower());

  ac.setPower(false);
  ac.send();
  EXPECT_FALSE(ac.getPower());
}

TEST(TestFujitsuAc264Class, Temperature) {
  IRFujitsuAC264 ac(kGpioUnused);
  ac.begin();

  ac.setMode(kFujitsuAc264ModeHeat);
  // Value over maximum is fixed to maximum (30C)
  ac.setTemp(40);
  EXPECT_EQ(30, ac.getTemp());

  // Value under minimum is fixed to minimum (16C in Heat)
  ac.setTemp(10);
  EXPECT_EQ(16, ac.getTemp());

  ac.setMode(kFujitsuAc264ModeCool);
  // Value over maximum is fixed to maximum (30C)
  ac.setTemp(40);
  EXPECT_EQ(30, ac.getTemp());

  // Value under minimum is fixed to minimum (18C)
  ac.setTemp(10);
  EXPECT_EQ(18, ac.getTemp());

  // Valid values in suppoerted range
  ac.setMode(kFujitsuAc264ModeHeat);
  for (float i = 16; i <= 30; i += 0.5) {
    ac.setTemp(i);
    EXPECT_EQ(i, ac.getTemp());
  }

  // Valid values in suppoerted range
  ac.setMode(kFujitsuAc264ModeCool);
  for (float i = 18; i <= 30; i += 0.5) {
    ac.setTemp(i);
    EXPECT_EQ(i, ac.getTemp());
  }

  // Value in supported range, but not multiple of 0.5
  // Fractional part of the value which is not multiple of 0.5 is truncated.
  ac.setTemp(22.9);
  EXPECT_EQ(22.5, ac.getTemp());

  ac.setTemp(20.1);
  EXPECT_EQ(20, ac.getTemp());
}

TEST(TestFujitsuAc264Class, TemperatureAuto) {
  IRFujitsuAC264 ac(kGpioUnused);
  ac.begin();

  // Value over maximum is fixed to maximum (+2C)
  ac.setTempAuto(10);
  EXPECT_EQ(2, ac.getTempAuto());

  // Value under minimum is fixed to minimum (-2C)
  ac.setTempAuto(-10);
  EXPECT_EQ(-2, ac.getTempAuto());

  // Valid values in suppoerted range
  for (float i = -2; i <= 2; i += 0.5) {
    ac.setTempAuto(i);
    EXPECT_EQ(i, ac.getTempAuto());
  }

  // Value in supported range, but not multiple of 0.5
  // Fractional part of the value which is not multiple of 0.5 is truncated.
  ac.setTempAuto(0.8);
  EXPECT_EQ(0.5, ac.getTempAuto());

  ac.setTempAuto(1.2);
  EXPECT_EQ(1, ac.getTempAuto());

  ac.setTempAuto(-1.4);
  EXPECT_EQ(-1, ac.getTempAuto());
}

TEST(TestFujitsuAc264Class, Mode) {
  IRFujitsuAC264 ac(kGpioUnused);
  ac.begin();

  // Unexpected value should default to Auto.
  ac.setMode(2);
  EXPECT_EQ(kFujitsuAc264ModeAuto, ac.getMode());
  EXPECT_FALSE(ac.isWeakDry());

  // Unexpected value should default to Auto.
  ac.setMode(255);
  EXPECT_EQ(kFujitsuAc264ModeAuto, ac.getMode());
  EXPECT_FALSE(ac.isWeakDry());

  ac.setMode(kFujitsuAc264ModeAuto);
  EXPECT_EQ(kFujitsuAc264ModeAuto, ac.getMode());
  EXPECT_FALSE(ac.isWeakDry());

  ac.setMode(kFujitsuAc264ModeCool);
  EXPECT_EQ(kFujitsuAc264ModeCool, ac.getMode());
  EXPECT_FALSE(ac.isWeakDry());

  ac.setMode(kFujitsuAc264ModeFan);
  EXPECT_EQ(kFujitsuAc264ModeFan, ac.getMode());
  EXPECT_FALSE(ac.isWeakDry());

  ac.setMode(kFujitsuAc264ModeHeat);
  EXPECT_EQ(kFujitsuAc264ModeHeat, ac.getMode());
  EXPECT_FALSE(ac.isWeakDry());

  ac.setMode(kFujitsuAc264ModeDry);
  EXPECT_EQ(kFujitsuAc264ModeDry, ac.getMode());
  EXPECT_FALSE(ac.isWeakDry());

  ac.setMode(kFujitsuAc264ModeDry, true);
  EXPECT_EQ(kFujitsuAc264ModeDry, ac.getMode());
  EXPECT_TRUE(ac.isWeakDry());
}

TEST(TestFujitsuAc264Class, FanSpeed) {
  IRFujitsuAC264 ac(kGpioUnused);
  ac.begin();

  // Unexpected value should default to Auto.
  ac.setFanSpeed(0);
  EXPECT_EQ(kFujitsuAc264FanSpeedAuto, ac.getFanSpeed());

  // Unexpected value should default to Auto.
  ac.setFanSpeed(255);
  EXPECT_EQ(kFujitsuAc264FanSpeedAuto, ac.getFanSpeed());

  // Unexpected value should default to Auto.
  ac.setFanSpeed(2);
  EXPECT_EQ(kFujitsuAc264FanSpeedAuto, ac.getFanSpeed());

  // Unexpected value should default to Auto.
  ac.setFanSpeed(4);
  EXPECT_EQ(kFujitsuAc264FanSpeedAuto, ac.getFanSpeed());

  // Unexpected value should default to Auto.
  ac.setFanSpeed(5);
  EXPECT_EQ(kFujitsuAc264FanSpeedAuto, ac.getFanSpeed());

  // Unexpected value should default to Auto.
  ac.setFanSpeed(7);
  EXPECT_EQ(kFujitsuAc264FanSpeedAuto, ac.getFanSpeed());

  // Beyond Max should default to Auto.
  ac.setFanSpeed(kFujitsuAc264FanSpeedHigh + 1);
  EXPECT_EQ(kFujitsuAc264FanSpeedAuto, ac.getFanSpeed());

  ac.setFanSpeed(kFujitsuAc264FanSpeedAuto);
  EXPECT_EQ(kFujitsuAc264FanSpeedAuto, ac.getFanSpeed());

  ac.setFanSpeed(kFujitsuAc264FanSpeedQuiet);
  EXPECT_EQ(kFujitsuAc264FanSpeedQuiet, ac.getFanSpeed());

  ac.setFanSpeed(kFujitsuAc264FanSpeedLow);
  EXPECT_EQ(kFujitsuAc264FanSpeedLow, ac.getFanSpeed());

  ac.setFanSpeed(kFujitsuAc264FanSpeedMed);
  EXPECT_EQ(kFujitsuAc264FanSpeedMed, ac.getFanSpeed());

  ac.setFanSpeed(kFujitsuAc264FanSpeedHigh);
  EXPECT_EQ(kFujitsuAc264FanSpeedHigh, ac.getFanSpeed());
}

TEST(TestFujitsuAc264Class, FanAngle) {
  IRFujitsuAC264 ac(kGpioUnused);
  ac.begin();

  // Unexpected value should default to Stay.
  ac.setFanAngle(0);
  EXPECT_EQ(kFujitsuAc264FanAngleStay, ac.getFanAngle());

  // Unexpected value should default to Stay.
  ac.setFanAngle(255);
  EXPECT_EQ(kFujitsuAc264FanAngleStay, ac.getFanAngle());

  // Unexpected value should default to Stay.
  ac.setFanAngle(8);
  EXPECT_EQ(kFujitsuAc264FanAngleStay, ac.getFanAngle());

  // Unexpected value should default to Stay.
  ac.setFanAngle(13);
  EXPECT_EQ(kFujitsuAc264FanAngleStay, ac.getFanAngle());

  // Unexpected value should default to Stay.
  ac.setFanAngle(32);
  EXPECT_EQ(kFujitsuAc264FanAngleStay, ac.getFanAngle());

  ac.setFanAngle(kFujitsuAc264FanAngle1);
  EXPECT_EQ(kFujitsuAc264FanAngle1, ac.getFanAngle());

  ac.setFanAngle(kFujitsuAc264FanAngle2);
  EXPECT_EQ(kFujitsuAc264FanAngle2, ac.getFanAngle());

  ac.setFanAngle(kFujitsuAc264FanAngle3);
  EXPECT_EQ(kFujitsuAc264FanAngle3, ac.getFanAngle());

  ac.setFanAngle(kFujitsuAc264FanAngle4);
  EXPECT_EQ(kFujitsuAc264FanAngle4, ac.getFanAngle());

  ac.setFanAngle(kFujitsuAc264FanAngle5);
  EXPECT_EQ(kFujitsuAc264FanAngle5, ac.getFanAngle());

  ac.setFanAngle(kFujitsuAc264FanAngle6);
  EXPECT_EQ(kFujitsuAc264FanAngle6, ac.getFanAngle());

  ac.setFanAngle(kFujitsuAc264FanAngle7);
  EXPECT_EQ(kFujitsuAc264FanAngle7, ac.getFanAngle());

  ac.setFanAngle(kFujitsuAc264FanAngleStay);
  EXPECT_EQ(kFujitsuAc264FanAngleStay, ac.getFanAngle());
}

TEST(TestFujitsuAc264Class, Swing) {
  IRFujitsuAC264 ac(kGpioUnused);
  ac.begin();

  ac.setSwing(true);
  EXPECT_TRUE(ac.getSwing());

  ac.setSwing(false);
  EXPECT_FALSE(ac.getSwing());
}

TEST(TestFujitsuAc264Class, Economy) {
  IRFujitsuAC264 ac(kGpioUnused);
  ac.begin();

  ac.setEconomy(true);
  EXPECT_TRUE(ac.getEconomy());

  ac.setEconomy(false);
  EXPECT_FALSE(ac.getEconomy());
}

TEST(TestFujitsuAc264Class, Clean) {
  IRFujitsuAC264 ac(kGpioUnused);
  ac.begin();

  ac.setClean(true);
  EXPECT_TRUE(ac.getClean());

  ac.setClean(false);
  EXPECT_FALSE(ac.getClean());
}

TEST(TestFujitsuAc264Class, Clock) {
  IRFujitsuAC264 ac(kGpioUnused);
  ac.begin();

  // Unexpected value should default to 0.
  ac.setClock(2000);
  EXPECT_EQ(0, ac.getClock());

  // Valid values in suppoerted range
  for (uint16_t i = 0; i < 1440; ++i) {
    ac.setClock(i);
    EXPECT_EQ(i, ac.getClock());
  }
}

TEST(TestFujitsuAc264Class, SleepTimer) {
  IRFujitsuAC264 ac(kGpioUnused);
  ac.begin();

  // Unexpected value should default to 0 (invalid).
  ac.setSleepTimer(12 * 60 + 1);
  EXPECT_EQ(0, ac.getSleepTimer());

  // Valid values in suppoerted range
  for (uint16_t i = 0; i <= 12; ++i) {
    ac.setSleepTimer(i * 60);
    EXPECT_EQ(i * 60, ac.getSleepTimer());
  }
}

TEST(TestFujitsuAc264Class, OnTimer) {
  IRFujitsuAC264 ac(kGpioUnused);
  ac.begin();

  // Unexpected value is ignored.
  ac.setOnTimer(144);
  EXPECT_EQ(0, ac.getOnTimer());

  // Valid values in suppoerted range
  for (uint8_t i = 0; i < 144; ++i) {
    ac.setOnTimer(i);
    EXPECT_EQ(i, ac.getOnTimer());
  }

  // Unexpected value is ignored.
  ac.setOnTimer(255);
  EXPECT_EQ(143, ac.getOnTimer());

  // On timer enabled
  ac.setTimerEnable(kFujitsuAc264OnTimerEnable);
  EXPECT_EQ(kFujitsuAc264OnTimerEnable, ac.getTimerEnable());

  // On timer disabled
  ac.setTimerEnable(kFujitsuAc264OnOffTimerDisable);
  EXPECT_EQ(kFujitsuAc264OnOffTimerDisable, ac.getTimerEnable());
}

TEST(TestFujitsuAc264Class, OffTimer) {
  IRFujitsuAC264 ac(kGpioUnused);
  ac.begin();

  // Unexpected value is ignored.
  ac.setOffTimer(144);
  EXPECT_EQ(0, ac.getOffTimer());

  // Valid values in suppoerted range
  for (uint8_t i = 0; i < 144; ++i) {
    ac.setOffTimer(i);
    EXPECT_EQ(i, ac.getOffTimer());
  }

  // Unexpected value is ignored.
  ac.setOffTimer(255);
  EXPECT_EQ(143, ac.getOffTimer());

  // Off timer enabled
  ac.setTimerEnable(kFujitsuAc264OffTimerEnable);
  EXPECT_EQ(kFujitsuAc264OffTimerEnable, ac.getTimerEnable());

  // On & off timer enabled
  ac.setTimerEnable(kFujitsuAc264OnOffTimerEnable);
  EXPECT_EQ(kFujitsuAc264OnOffTimerEnable, ac.getTimerEnable());

  // On & Off timer disabled
  ac.setTimerEnable(kFujitsuAc264OnOffTimerDisable);
  EXPECT_EQ(kFujitsuAc264OnOffTimerDisable, ac.getTimerEnable());
}

TEST(TestFujitsuAc264Class, Sterilization) {
  IRFujitsuAC264 ac(kGpioUnused);
  ac.begin();

  ac.on();
  ac.send();

  // When AC is powered on, sterilization is ignored.
  ac.toggleSterilization();
  EXPECT_EQ(kFujitsuAc264CmdCool, ac.getCmd());

  ac.off();
  ac.send();

  // When AC is powered off, sterilization is accepted.
  ac.toggleSterilization();
  EXPECT_EQ(kFujitsuAc264SpCmdToggleSterilization, ac.getCmd());
}

TEST(TestFujitsuAc264Class, OutsideQuiet) {
  IRFujitsuAC264 ac(kGpioUnused);
  ac.begin();

  ac.on();
  ac.send();

  // When AC is powered on, outside quiet is ignored.
  ac.setOutsideQuiet(true);
  ac.send();
  EXPECT_FALSE(ac.getOutsideQuiet());

  ac.off();
  ac.send();

  // When AC is powered off, outside quiet is accepted.
  ac.setOutsideQuiet(true);
  ac.send();
  EXPECT_TRUE(ac.getOutsideQuiet());

  ac.setOutsideQuiet(false);
  ac.send();
  EXPECT_FALSE(ac.getOutsideQuiet());
}

TEST(TestFujitsuAc264Class, EcoFan) {
  IRFujitsuAC264 ac(kGpioUnused);
  ac.begin();

  ac.on();
  ac.send();

  // When AC is powered on, eco fan is ignored.
  ac.setEcoFan(true);
  ac.send();
  EXPECT_FALSE(ac.getEcoFan());

  ac.off();
  ac.send();

  // When AC is powered off, eco fan is accepted.
  ac.setEcoFan(true);
  ac.send();
  EXPECT_TRUE(ac.getEcoFan());

  ac.setEcoFan(false);
  ac.send();
  EXPECT_FALSE(ac.getEcoFan());
}

TEST(TestFujitsuAc264Class, Powerful) {
  IRFujitsuAC264 ac(kGpioUnused);
  ac.begin();

  ac.off();
  ac.send();

  // When AC is powered off, powerful is ignored.
  ac.togglePowerful();
  ac.send();
  EXPECT_EQ(kFujitsuAc264SpCmdTurnOff, ac.getCmd());

  ac.on();
  ac.send();

  // When AC is powered on, powerful is accepted.
  ac.togglePowerful();
  ac.send();
  EXPECT_EQ(kFujitsuAc264SpCmdTogglePowerful, ac.getCmd());

  ac.togglePowerful();
  ac.send();
  EXPECT_EQ(kFujitsuAc264SpCmdTogglePowerful, ac.getCmd());
}

TEST(TestFujitsuAc264Class, NormalCommands) {
  IRFujitsuAC264 ac(kGpioUnused);
  ac.begin();

  ac.on();
  ac.send();

  // Special commands are ignored.
  ac.setCmd(kFujitsuAc264SpCmdTogglePowerful);
  EXPECT_EQ(kFujitsuAc264CmdCool, ac.getCmd());

  ac.setCmd(kFujitsuAc264SpCmdTurnOff);
  EXPECT_EQ(kFujitsuAc264CmdCool, ac.getCmd());

  ac.setCmd(kFujitsuAc264SpCmdEcoFanOff);
  EXPECT_EQ(kFujitsuAc264CmdCool, ac.getCmd());

  ac.setCmd(kFujitsuAc264SpCmdEcoFanOn);
  EXPECT_EQ(kFujitsuAc264CmdCool, ac.getCmd());

  ac.setCmd(kFujitsuAc264SpCmdOutsideQuietOff);
  EXPECT_EQ(kFujitsuAc264CmdCool, ac.getCmd());

  ac.setCmd(kFujitsuAc264SpCmdOutsideQuietOn);
  EXPECT_EQ(kFujitsuAc264CmdCool, ac.getCmd());

  ac.setCmd(kFujitsuAc264SpCmdToggleSterilization);
  EXPECT_EQ(kFujitsuAc264CmdCool, ac.getCmd());

  // Normal commands can be set.
  ac.setCmd(kFujitsuAc264CmdCool);
  EXPECT_EQ(kFujitsuAc264CmdCool, ac.getCmd());

  ac.setCmd(kFujitsuAc264CmdHeat);
  EXPECT_EQ(kFujitsuAc264CmdHeat, ac.getCmd());

  ac.setCmd(kFujitsuAc264CmdDry);
  EXPECT_EQ(kFujitsuAc264CmdDry, ac.getCmd());

  ac.setCmd(kFujitsuAc264CmdAuto);
  EXPECT_EQ(kFujitsuAc264CmdAuto, ac.getCmd());

  ac.setCmd(kFujitsuAc264CmdFan);
  EXPECT_EQ(kFujitsuAc264CmdFan, ac.getCmd());

  ac.setCmd(kFujitsuAc264CmdTemp);
  EXPECT_EQ(kFujitsuAc264CmdTemp, ac.getCmd());

  ac.setCmd(kFujitsuAc264CmdSwing);
  EXPECT_EQ(kFujitsuAc264CmdSwing, ac.getCmd());

  ac.setCmd(kFujitsuAc264CmdSleepTime);
  EXPECT_EQ(kFujitsuAc264CmdSleepTime, ac.getCmd());

  ac.setCmd(kFujitsuAc264CmdEconomy);
  EXPECT_EQ(kFujitsuAc264CmdEconomy, ac.getCmd());

  ac.setCmd(kFujitsuAc264CmdClean);
  EXPECT_EQ(kFujitsuAc264CmdClean, ac.getCmd());

  ac.setCmd(kFujitsuAc264CmdFanSpeed);
  EXPECT_EQ(kFujitsuAc264CmdFanSpeed, ac.getCmd());

  ac.setCmd(kFujitsuAc264CmdFanAngle);
  EXPECT_EQ(kFujitsuAc264CmdFanAngle, ac.getCmd());

  ac.setCmd(kFujitsuAc264CmdCancelSleepTimer);
  EXPECT_EQ(kFujitsuAc264CmdCancelSleepTimer, ac.getCmd());

  ac.setCmd(kFujitsuAc264CmdOnTimer);
  EXPECT_EQ(kFujitsuAc264CmdOnTimer, ac.getCmd());

  ac.setCmd(kFujitsuAc264CmdOffTimer);
  EXPECT_EQ(kFujitsuAc264CmdOffTimer, ac.getCmd());

  ac.setCmd(kFujitsuAc264CmdCancelOnOffTimer);
  EXPECT_EQ(kFujitsuAc264CmdCancelOnOffTimer, ac.getCmd());
}

TEST(TestFujitsuAc264Class, SpecialCommands) {
  IRFujitsuAC264 ac(kGpioUnused);
  ac.begin();

  uint8_t expected_turnoff[7] = {0x14, 0x63, 0x0, 0x10, 0x10, 0x02, 0xFD};
  uint8_t expected_powerful[7] = {0x14, 0x63, 0x00, 0x10, 0x10, 0x39, 0xC6};
  uint8_t expected_ecofanoff[7] = {0x14, 0x63, 0x00, 0x10, 0x10, 0x51, 0xAE};
  uint8_t expected_ecofanon[7] = {0x14, 0x63, 0x00, 0x10, 0x10, 0x50, 0xAF};
  uint8_t expected_outsidequietoff[16] = {
    0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0xC1,
    0x40, 0x01, 0x00, 0x00, 0xFE, 0xBF, 0x00, 0x41};
  uint8_t expected_outsidequieton[16] = {
    0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0xC1,
    0x40, 0x00, 0x00, 0x00, 0xFF, 0xBF, 0x00, 0x41};
  uint8_t expected_sterilization[16] = {
    0x14, 0x63, 0x00, 0x10, 0x10, 0xFE, 0x09, 0xC1,
    0x60, 0x03, 0x00, 0x00, 0xFC, 0x9F, 0x00, 0x41};

  ac.on();
  ac.send();

  ac.togglePowerful();
  ac.send();
  EXPECT_STATE_EQ(expected_powerful, ac.getRaw(), 7 * 8);
  EXPECT_EQ(kFujitsuAc264StateLengthShort, ac.getStateLength());
  EXPECT_EQ("Command: Powerful", ac.toString());

  ac.off();
  ac.send();
  EXPECT_STATE_EQ(expected_turnoff, ac.getRaw(), 7 * 8);
  EXPECT_EQ(kFujitsuAc264StateLengthShort, ac.getStateLength());
  EXPECT_EQ("Command: Power Off", ac.toString());

  ac.setEcoFan(true);
  ac.send();
  EXPECT_STATE_EQ(expected_ecofanon, ac.getRaw(), 7 * 8);
  EXPECT_EQ(kFujitsuAc264StateLengthShort, ac.getStateLength());
  EXPECT_EQ("Command: Eco Fan On", ac.toString());

  ac.setEcoFan(false);
  ac.send();
  EXPECT_STATE_EQ(expected_ecofanoff, ac.getRaw(), 7 * 8);
  EXPECT_EQ(kFujitsuAc264StateLengthShort, ac.getStateLength());
  EXPECT_EQ("Command: Eco Fan Off", ac.toString());

  ac.setOutsideQuiet(true);
  ac.send();
  EXPECT_STATE_EQ(expected_outsidequieton, ac.getRaw(), 16 * 8);
  EXPECT_EQ(kFujitsuAc264StateLengthMiddle, ac.getStateLength());
  EXPECT_EQ("Command: Outside Quiet On", ac.toString());

  ac.setOutsideQuiet(false);
  ac.send();
  EXPECT_STATE_EQ(expected_outsidequietoff, ac.getRaw(), 16 * 8);
  EXPECT_EQ(kFujitsuAc264StateLengthMiddle, ac.getStateLength());
  EXPECT_EQ("Command: Outside Quiet Off", ac.toString());

  ac.toggleSterilization();
  ac.send();
  EXPECT_STATE_EQ(expected_sterilization, ac.getRaw(), 16 * 8);
  EXPECT_EQ(kFujitsuAc264StateLengthMiddle, ac.getStateLength());
  EXPECT_EQ("Command: Sterilization", ac.toString());
}
