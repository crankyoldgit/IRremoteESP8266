// Delonghi A/C
//
// Copyright 2020 David Conran

#ifndef IR_DELONGHI_H_
#define IR_DELONGHI_H_

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
//   Brand: Delonghi,  Model: PAC A95

// Ref:
//   https://github.com/crankyoldgit/IRremoteESP8266/issues/1096

// Kudos:
//   TheMaxxz: For the breakdown and mapping of the bit values.

// Constants

// Classes
class IRDelonghiAc {
 public:
  explicit IRDelonghiAc(const uint16_t pin, const bool inverted = false,
                        const bool use_modulation = true);

  void stateReset();
#if SEND_DELONGHI_AC
  void send(const uint16_t repeat = kDelonghiAcDefaultRepeat);
  int8_t calibrate(void) { return _irsend.calibrate(); }
#endif  // SEND_DELONGHI_AC
  void begin();
  static uint8_t calcChecksum(const uint64_t state);
  static bool validChecksum(const uint64_t state);
  void setPower(const bool state);
  bool getPower();
  void on();
  void off();
  void setTemp(const uint8_t temp);
  uint8_t getTemp();
  void setFan(const uint8_t speed);
  uint8_t getFan();
  void setMode(const uint8_t mode);
  uint8_t getMode();
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
  void checksum(void);
};
#endif  // IR_DELONGHI_H_
