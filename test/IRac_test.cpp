// Copyright 2019 David Conran

#include "ir_Argo.h"
#include "ir_Daikin.h"
#include "ir_Fujitsu.h"
#include "ir_Gree.h"
#include "ir_Haier.h"
#include "ir_Hitachi.h"
#include "ir_Kelvinator.h"
#include "ir_Midea.h"
#include "ir_Mitsubishi.h"
#include "IRac.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"

// Tests for IRac class.

TEST(TestIRac, Argo) {
  IRArgoAC ac(0);
  IRac irac(0);

  ac.begin();
  irac.argo(&ac,
            true,                        // Power
            stdAc::opmode_t::kHeat,      // Mode
            21,                          // Celsius
            stdAc::fanspeed_t::kHigh,    // Fan speed
            stdAc::swingv_t::kOff,       // Veritcal swing
            false,                       // Turbo
            -1);                         // Sleep
  EXPECT_TRUE(ac.getPower());
  EXPECT_EQ(1, ac.getMode());
  EXPECT_EQ(21, ac.getTemp());
  EXPECT_EQ(kArgoFlapAuto, ac.getFlap());
  EXPECT_FALSE(ac.getMax());  // Turbo
  EXPECT_FALSE(ac.getNight());  // Sleep
}

TEST(TestIRac, Coolix) {
  IRCoolixAC ac(0);
  IRac irac(0);
  IRrecv capture(0);
  char expected[] =
      "Power: On, Mode: 3 (HEAT), Fan: 1 (MAX), Temp: 21C, Zone Follow: Off, "
      "Sensor Temp: Ignored";

  ac.begin();
  irac.coolix(&ac,
              true,                        // Power
              stdAc::opmode_t::kHeat,      // Mode
              21,                          // Celsius
              stdAc::fanspeed_t::kHigh,    // Fan speed
              stdAc::swingv_t::kOff,       // Veritcal swing
              stdAc::swingh_t::kOff,       // Horizontal swing
              false,                       // Turbo
              false,                       // Light
              false,                       // Clean
              -1);                         // Sleep
  ASSERT_EQ(expected, ac.toString());
  ac._irsend.makeDecodeResult();
  EXPECT_TRUE(capture.decode(&ac._irsend.capture));
  ASSERT_EQ(COOLIX, ac._irsend.capture.decode_type);
  ASSERT_EQ(kCoolixBits, ac._irsend.capture.bits);
  ac.setRaw(ac._irsend.capture.value);
  ASSERT_EQ(expected, ac.toString());
}

TEST(TestIRac, Daikin) {
  IRDaikinESP ac(0);
  IRac irac(0);
  char expected[] =
      "Power: On, Mode: 3 (COOL), Temp: 19C, Fan: 2, Powerful: Off, "
      "Quiet: Off, Sensor: Off, Eye: Off, Mold: On, Swing (Horizontal): Off, "
      "Swing (Vertical): Off, Current Time: 0:00, On Time: Off, Off Time: Off";

  ac.begin();
  irac.daikin(&ac,
              true,                        // Power
              stdAc::opmode_t::kCool,      // Mode
              19,                          // Celsius
              stdAc::fanspeed_t::kMedium,  // Fan speed
              stdAc::swingv_t::kOff,       // Veritcal swing
              stdAc::swingh_t::kOff,       // Horizontal swing
              false,                       // Quiet
              false,                       // Turbo
              true,                        // Filter
              true);                       // Clean
  ASSERT_EQ(expected, ac.toString());
}

TEST(TestIRac, Daikin2) {
  IRDaikin2 ac(0);
  IRac irac(0);
  IRrecv capture(0);
  char expected[] =
      "Power: On, Mode: 3 (COOL), Temp: 19C, Fan: 2, Swing (V): 14 (Auto), "
      "Swing (H): 0, Clock: 0:00, On Time: Off, Off Time: Off, "
      "Sleep Time: Off, Beep: 1 (Quiet), Light: 1 (Bright), Mold: On, "
      "Clean: Off, Fresh Air: Off, Eye: Off, Eye Auto: Off, Quiet: Off, "
      "Powerful: Off, Purify: On, Econo: Off";

  ac.begin();
  irac.daikin2(&ac,
               true,                        // Power
               stdAc::opmode_t::kCool,      // Mode
               19,                          // Celsius
               stdAc::fanspeed_t::kMedium,  // Fan speed
               stdAc::swingv_t::kOff,       // Veritcal swing
               stdAc::swingh_t::kOff,       // Horizontal swing
               false,                       // Quiet
               false,                       // Turbo
               true,                        // Light
               false,                       // Econo
               true,                        // Filter
               true,                        // Clean (aka Mold)
               -1,                          // Sleep time
               -1);                         // Current time
  ASSERT_EQ(expected, ac.toString());
  ac._irsend.makeDecodeResult();
  EXPECT_TRUE(capture.decode(&ac._irsend.capture));
  ASSERT_EQ(DAIKIN2, ac._irsend.capture.decode_type);
  ASSERT_EQ(kDaikin2Bits, ac._irsend.capture.bits);
  ac.setRaw(ac._irsend.capture.state);
  ASSERT_EQ(expected, ac.toString());
}

TEST(TestIRac, Fujitsu) {
  IRFujitsuAC ac(0);
  IRac irac(0);
  IRrecv capture(0);
  char expected[] =
      "Power: On, Mode: 1 (COOL), Temp: 19C, Fan: 2 (MED), "
      "Swing: Off, Command: N/A";

  ac.begin();
  irac.fujitsu(&ac,
               ARDB1,                       // Model
               true,                        // Power
               stdAc::opmode_t::kCool,      // Mode
               19,                          // Celsius
               stdAc::fanspeed_t::kMedium,  // Fan speed
               stdAc::swingv_t::kOff,       // Veritcal swing
               stdAc::swingh_t::kOff,       // Horizontal swing
               false);                      // Quiet
  ASSERT_EQ(expected, ac.toString());
  ac._irsend.makeDecodeResult();
  EXPECT_TRUE(capture.decode(&ac._irsend.capture));
  ASSERT_EQ(FUJITSU_AC, ac._irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcBits - 8, ac._irsend.capture.bits);
  ac.setRaw(ac._irsend.capture.state, ac._irsend.capture.bits / 8);
  ASSERT_EQ(expected, ac.toString());

  ac._irsend.reset();
  irac.fujitsu(&ac,
               ARRAH2E,                     // Model
               true,                        // Power
               stdAc::opmode_t::kCool,      // Mode
               19,                          // Celsius
               stdAc::fanspeed_t::kMedium,  // Fan speed
               stdAc::swingv_t::kOff,       // Veritcal swing
               stdAc::swingh_t::kOff,       // Horizontal swing
               false);                      // Quiet
  ASSERT_EQ(expected, ac.toString());
  ac._irsend.makeDecodeResult();
  EXPECT_TRUE(capture.decode(&ac._irsend.capture));
  ASSERT_EQ(FUJITSU_AC, ac._irsend.capture.decode_type);
  ASSERT_EQ(kFujitsuAcBits, ac._irsend.capture.bits);
  ac.setRaw(ac._irsend.capture.state, ac._irsend.capture.bits / 8);
  ASSERT_EQ(expected, ac.toString());
}

TEST(TestIRac, Gree) {
  IRGreeAC ac(0);
  IRac irac(0);
  IRrecv capture(0);
  char expected[] =
      "Power: On, Mode: 1 (COOL), Temp: 22C, Fan: 2, Turbo: Off, XFan: On, "
      "Light: On, Sleep: On, Swing Vertical Mode: Manual, "
      "Swing Vertical Pos: 3";

  ac.begin();
  irac.gree(&ac,
            true,                        // Power
            stdAc::opmode_t::kCool,      // Mode
            22,                          // Celsius
            stdAc::fanspeed_t::kMedium,  // Fan speed
            stdAc::swingv_t::kHigh,      // Veritcal swing
            false,                       // Turbo
            true,                        // Light
            true,                        // Clean (aka Mold/XFan)
            8 * 60 + 0);                 // Sleep time
  ASSERT_EQ(expected, ac.toString());
  ac._irsend.makeDecodeResult();
  EXPECT_TRUE(capture.decode(&ac._irsend.capture));
  ASSERT_EQ(GREE, ac._irsend.capture.decode_type);
  ASSERT_EQ(kGreeBits, ac._irsend.capture.bits);
  ac.setRaw(ac._irsend.capture.state);
  ASSERT_EQ(expected, ac.toString());
}

TEST(TestIRac, Haier) {
  IRHaierAC ac(0);
  IRac irac(0);
  IRrecv capture(0);
  char expected[] =
      "Command: 1 (On), Mode: 3 (HEAT), Temp: 24C, Fan: 2, Swing: 1 (Up), "
      "Sleep: On, Health: On, Current Time: 13:45, On Timer: Off, "
      "Off Timer: Off";

  ac.begin();
  irac.haier(&ac,
             true,                        // Power
             stdAc::opmode_t::kCool,      // Mode
             24,                          // Celsius
             stdAc::fanspeed_t::kMedium,  // Fan speed
             stdAc::swingv_t::kHigh,      // Veritcal swing
             true,                        // Filter
             8 * 60 + 0,                  // Sleep time
             13 * 60 + 45);               // Clock
  ASSERT_EQ(expected, ac.toString());
  ac._irsend.makeDecodeResult();
  EXPECT_TRUE(capture.decode(&ac._irsend.capture));
  ASSERT_EQ(HAIER_AC, ac._irsend.capture.decode_type);
  ASSERT_EQ(kHaierACBits, ac._irsend.capture.bits);
  ac.setRaw(ac._irsend.capture.state);
  ASSERT_EQ(expected, ac.toString());
}


TEST(TestIRac, HaierYrwo2) {
  IRHaierACYRW02 ac(0);
  IRac irac(0);
  IRrecv capture(0);
  char expected[] =
      "Power: On, Button: 5 (Power), Mode: 2 (Cool), Temp: 23C, Fan: 4 (Med), "
      "Turbo: 1 (High), Swing: 1 (Top), Sleep: On, Health: On";

  ac.begin();
  irac.haierYrwo2(&ac,
             true,                        // Power
             stdAc::opmode_t::kCool,      // Mode
             23,                          // Celsius
             stdAc::fanspeed_t::kMedium,  // Fan speed
             stdAc::swingv_t::kHigh,      // Veritcal swing
             true,                        // Turbo
             true,                        // Filter
             8 * 60 + 0);                 // Sleep time
  ASSERT_EQ(expected, ac.toString());
  ac._irsend.makeDecodeResult();
  EXPECT_TRUE(capture.decode(&ac._irsend.capture));
  ASSERT_EQ(HAIER_AC_YRW02, ac._irsend.capture.decode_type);
  ASSERT_EQ(kHaierACYRW02Bits, ac._irsend.capture.bits);
  ac.setRaw(ac._irsend.capture.state);
  ASSERT_EQ(expected, ac.toString());
}

TEST(TestIRac, Hitachi) {
  IRHitachiAc ac(0);
  IRac irac(0);
  IRrecv capture(0);
  char expected[] =
      "Power: On, Mode: 2 (AUTO), Temp: 22C, Fan: 3 (UNKNOWN), "
      "Swing (Vertical): Off, Swing (Horizontal): On";

  ac.begin();
  irac.hitachi(&ac,
               true,                        // Power
               stdAc::opmode_t::kAuto,      // Mode
               22,                          // Celsius
               stdAc::fanspeed_t::kMedium,  // Fan speed
               stdAc::swingv_t::kOff,       // Veritcal swing
               stdAc::swingh_t::kAuto);     // Horizontal swing

  ASSERT_EQ(expected, ac.toString());
  ac._irsend.makeDecodeResult();
  EXPECT_TRUE(capture.decode(&ac._irsend.capture));
  ASSERT_EQ(HITACHI_AC, ac._irsend.capture.decode_type);
  ASSERT_EQ(kHitachiAcBits, ac._irsend.capture.bits);
  ac.setRaw(ac._irsend.capture.state);
  ASSERT_EQ(expected, ac.toString());
}

TEST(TestIRac, Kelvinator) {
  IRKelvinatorAC ac(0);
  IRac irac(0);
  IRrecv capture(0);
  char expected[] =
      "Power: On, Mode: 1 (COOL), Temp: 19C, Fan: 3, Turbo: Off, Quiet: Off, "
      "XFan: On, IonFilter: On, Light: On, Swing (Horizontal): Off, "
      "Swing (Vertical): Off";

  ac.begin();
  irac.kelvinator(&ac,
                  true,                        // Power
                  stdAc::opmode_t::kCool,      // Mode
                  19,                          // Celsius
                  stdAc::fanspeed_t::kMedium,  // Fan speed
                  stdAc::swingv_t::kOff,       // Veritcal swing
                  stdAc::swingh_t::kOff,       // Horizontal swing
                  false,                       // Quiet
                  false,                       // Turbo
                  true,                        // Light
                  true,                        // Filter
                  true);                       // Clean

  ASSERT_EQ(expected, ac.toString());
  ac._irsend.makeDecodeResult();
  EXPECT_TRUE(capture.decode(&ac._irsend.capture));
  ASSERT_EQ(KELVINATOR, ac._irsend.capture.decode_type);
  ASSERT_EQ(kKelvinatorBits, ac._irsend.capture.bits);
  ac.setRaw(ac._irsend.capture.state);
  ASSERT_EQ(expected, ac.toString());
}

TEST(TestIRac, Midea) {
  IRMideaAC ac(0);
  IRac irac(0);
  IRrecv capture(0);
  char expected[] =
      "Power: On, Mode: 1 (DRY), Temp: 27C/81F, Fan: 2 (MED), Sleep: On";

  ac.begin();
  irac.midea(&ac,
             true,                        // Power
             stdAc::opmode_t::kDry,       // Mode
             27,                          // Celsius
             stdAc::fanspeed_t::kMedium,  // Fan speed
             8 * 60 + 0);                 // Sleep time

  ASSERT_EQ(expected, ac.toString());
  ac._irsend.makeDecodeResult();
  EXPECT_TRUE(capture.decode(&ac._irsend.capture));
  ASSERT_EQ(MIDEA, ac._irsend.capture.decode_type);
  ASSERT_EQ(kMideaBits, ac._irsend.capture.bits);
  ac.setRaw(ac._irsend.capture.value);
  ASSERT_EQ(expected, ac.toString());
}

TEST(TestIRac, Mitsubishi) {
  IRMitsubishiAC ac(0);
  IRac irac(0);
  IRrecv capture(0);
  char expected[] =
      "Power: On (COOL), Temp: 20C, FAN: 2, VANE: AUTO, Time: 14:30, "
      "On timer: 00:00, Off timer: 00:00, Timer: -";

  ac.begin();
  irac.mitsubishi(&ac,
                  true,                        // Power
                  stdAc::opmode_t::kCool,      // Mode
                  20,                          // Celsius
                  stdAc::fanspeed_t::kMedium,  // Fan speed
                  stdAc::swingv_t::kOff,       // Veritcal swing
                  false,                       // Silent
                  14 * 60 + 35);               // Clock
  ASSERT_EQ(expected, ac.toString());
  ac._irsend.makeDecodeResult();
  EXPECT_TRUE(capture.decode(&ac._irsend.capture));
  ASSERT_EQ(MITSUBISHI_AC, ac._irsend.capture.decode_type);
  ASSERT_EQ(kMitsubishiACBits, ac._irsend.capture.bits);
  ac.setRaw(ac._irsend.capture.state);
  ASSERT_EQ(expected, ac.toString());
}
