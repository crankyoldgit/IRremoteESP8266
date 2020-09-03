// Copyright 2020 Quentin Briollant

/// @file
/// @brief Support for Technibel protocol.

#ifndef IR_TECHNIBEL_H_
#define IR_TECHNIBEL_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include "IRremoteESP8266.h"
#include "IRsend.h"
#ifdef UNIT_TEST
#include "IRsend_test.h"
#endif

// Supports:
//   Brand: TECHNIBEL,  Model: IRO PLUS

// Ref:
//

// Kudos:
//   : For the breakdown and mapping of the bit values.

/* State bit map:

+--+--+--+--+--+--+--+--+--+------------+-----------+----------+--+--+--+--+
|     FIXED HEADER      |ON|TIMER CHANGE|TEMP CHANGE|FAN CHANGE|    MODE   |
+--+--+--+--+--+--+--+--+--+------------+-----------+----------+--+--+--+--+
  0  1  2  3  4  5  6  7  8      9            10          11    12 13 14 15

+-----+------+-----+-----+---+--+--+--+---+--+--+--+--+--+--+--+
|TIMER|C OR F|SWING|SLEEP| 0 |   FAN  | 0 |     TEMPERATURE    |
+-----+------+-----+-----+---+--+--+--+---+--+--+--+--+--+--+--+
   16    17     18    19   20 21 22 23  24 25 26 27 28 28 30 31

+---+---+---+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
| 0 | 0 | 0 | ON TIME HOUR |         FOOTER        |       CHECKSUM        |
+---+---+---+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  32  33  34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 48 50 51 52 53 54 55


*/

// Constants
const uint8_t kTechnibelAcPowerBit = 8;
const uint8_t kTechnibelAcTimerChangeBit = kTechnibelAcPowerBit + 1;  // 9
const uint8_t kTechnibelAcTempChangeBit = kTechnibelAcTimerChangeBit + 1;  // 10
const uint8_t kTechnibelAcFanChangeBit = kTechnibelAcTempChangeBit + 1;  // 11
const uint8_t kTechnibelAcModeOffset = kTechnibelAcFanChangeBit + 1;  // 12
const uint8_t kTechnibelAcModeSize = 4;
const uint8_t kTechnibelAcCool =   0b0001;
const uint8_t kTechnibelAcDry =    0b0010;
const uint8_t kTechnibelAcFan =    0b0100;
const uint8_t kTechnibelAcHeat =   0b1000;
const uint8_t kTechnibelAcTimerEnableBit = kTechnibelAcModeOffset
                                          + kTechnibelAcModeSize;  // 16
const uint8_t kTechnibelAcTempUnitBit = kTechnibelAcTimerEnableBit + 1;
                                      // 17 (0 = Celsius, 1 = Fahrenheit)
const uint8_t kTechnibelAcSwingBit = kTechnibelAcTempUnitBit + 1;  // 18
const uint8_t kTechnibelAcSleepBit = kTechnibelAcSwingBit + 1;  // 19
// '0' bit
const uint8_t kTechnibelAcFanOffset = kTechnibelAcSleepBit + 2;  // 21
const uint8_t kTechnibelAcFanSize = 3;
const uint8_t kTechnibelAcFanLow =    0b001;
const uint8_t kTechnibelAcFanMedium = 0b010;
const uint8_t kTechnibelAcFanHigh =   0b100;
// '0' bit
const uint8_t kTechnibelAcTempOffset = kTechnibelAcFanOffset
                                      + kTechnibelAcFanSize + 1;  // 25
const uint8_t kTechnibelAcTempSize = 7;
const uint8_t kTechnibelAcTempMinC = 16;  // Deg C
const uint8_t kTechnibelAcTempMaxC = 31;  // Deg C
const uint8_t kTechnibelAcTempMinF = 61;  // Deg F
const uint8_t kTechnibelAcTempMaxF = 88;  // Deg F
// '0' bit
// '0' bit
// '0' bit
const uint8_t kTechnibelAcTimerHoursOffset = kTechnibelAcTempOffset
                                            + kTechnibelAcTempSize + 3;  // 35
const uint8_t kTechnibelAcHoursSize = 5;  // Max 24 hrs
const uint8_t kTechnibelAcTimerMax = 24;
const uint8_t kTechnibelAcFooterOffset = kTechnibelAcTimerHoursOffset
                                        + kTechnibelAcHoursSize;  // 40
const uint8_t kTechnibelAcFooterSize = 8;
const uint8_t kTechnibelAcChecksumOffset = kTechnibelAcFooterOffset
                                          + kTechnibelAcFooterSize;  // 48
const uint8_t kTechnibelAcChecksumSize = 8;


// Classes
class IRTechnibelAc {
 public:
  explicit IRTechnibelAc(const uint16_t pin, const bool inverted = false,
                        const bool use_modulation = true);

  void stateReset();
#if SEND_TECHNIBEL_AC
  void send(const uint16_t repeat = kTechnibelAcDefaultRepeat);
  int8_t calibrate(void) { return _irsend.calibrate(); }
#endif  // SEND_TECHNIBEL_AC
  void begin();
  static uint8_t calcChecksum(const uint64_t state);
  static bool validChecksum(const uint64_t state);
  void setPower(const bool on);
  bool getPower();
  void on();
  void off();
  void setTempUnit(const bool celsius);
  bool getTempUnit(void);
  void setTemp(const uint8_t temp, const bool fahrenheit = false);
  uint8_t getTemp();
  void setFan(const uint8_t speed);
  uint8_t getFan();
  void setMode(const uint8_t mode);
  uint8_t getMode();
  void setSwing(const bool on);
  bool getSwing();
  bool convertSwing(const stdAc::swingv_t swing);
  stdAc::swingv_t toCommonSwing(const bool swing);
  void setSleep(const bool on);
  bool getSleep();
  void setTimerEnabled(const bool on);
  bool getTimerEnabled(void);
  void setTimer(const uint8_t nr_of_hours);
  uint8_t getTimer(void);
  uint64_t getRaw();
  void setRaw(const uint64_t state);
  uint8_t convertMode(const stdAc::opmode_t mode);
  uint8_t convertFan(const stdAc::fanspeed_t speed);
  static stdAc::opmode_t toCommonMode(const uint8_t mode);
  static stdAc::fanspeed_t toCommonFanSpeed(const uint8_t speed);
  stdAc::state_t toCommon(void);
  String toString();
#ifndef UNIT_TEST

 private:
  IRsend _irsend;
#else
  IRsendTest _irsend;
#endif
  uint64_t remote_state;  // The state of the IR remote.
  uint8_t _saved_temp;  // The previously user requested temp value.
  uint8_t _saved_temp_units;  // The previously user requested temp units.
  void checksum(void);
};
#endif  // IR_TECHNIBEL_H_
