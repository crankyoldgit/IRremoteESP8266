// Whirlpool A/C
//
// Copyright 2018 David Conran

#ifndef IR_WHIRLPOOL_H_
#define IR_WHIRLPOOL_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#ifndef UNIT_TEST
#include <Arduino.h>
#else
#include <string>
#endif
#include "IRremoteESP8266.h"
#include "IRsend.h"

//    WW      WW HH   HH IIIII RRRRRR  LL      PPPPPP   OOOOO   OOOOO  LL
//    WW      WW HH   HH  III  RR   RR LL      PP   PP OO   OO OO   OO LL
//    WW   W  WW HHHHHHH  III  RRRRRR  LL      PPPPPP  OO   OO OO   OO LL
//     WW WWW WW HH   HH  III  RR  RR  LL      PP      OO   OO OO   OO LL
//      WW   WW  HH   HH IIIII RR   RR LLLLLLL PP       OOOO0   OOOO0  LLLLLLL

// Ref:
//   https://github.com/markszabo/IRremoteESP8266/issues/509

// Constants
const uint8_t kWhirlpoolAcChecksumByte1 = 13;
const uint8_t kWhirlpoolAcChecksumByte2 = kWhirlpoolAcStateLength - 1;
const uint8_t kWhirlpoolAcHeat = 0;
const uint8_t kWhirlpoolAcAuto = 1;
const uint8_t kWhirlpoolAcCool = 2;
const uint8_t kWhirlpoolAcDry = 3;
const uint8_t kWhirlpoolAcFan = 4;
const uint8_t kWhirlpoolAcModeMask = 0b11111000;
const uint8_t kWhirlpoolAcFanAuto = 0;
const uint8_t kWhirlpoolAcFanHigh = 1;
const uint8_t kWhirlpoolAcFanMedium = 2;
const uint8_t kWhirlpoolAcFanLow = 3;
const uint8_t kWhirlpoolAcFanMask = 0b11001111;
const uint8_t kWhirlpoolAcMinTemp = 18;   // 18C
const uint8_t kWhirlpoolAcMaxTemp = 32;   // 32C
const uint8_t kWhirlpoolAcAutoTemp = 25;  // 25C
const uint8_t kWhirlpoolAcTempMask = 0b11110000;
const uint8_t kWhirlpoolAcLightMask = 0b00000100;
const uint8_t kWhirlpoolAcClockHourMask = 0b00011111;


// Classes
class IRWhirlpoolAc {
 public:
  explicit IRWhirlpoolAc(uint16_t pin);

  void stateReset();
#if SEND_WHIRLPOOL_AC
  void send(const bool calcchecksum = true);
#endif  // SEND_WHIRLPOOL_AC
  void begin();
  void on();
  void off();
  void setPower(const bool state);
  bool getPower();
  void setTemp(const uint8_t temp);
  uint8_t getTemp();
  void setFan(const uint8_t speed);
  uint8_t getFan();
  void setMode(const uint8_t mode);
  uint8_t getMode();
  void setSwing(const bool on);
  bool getSwing();
  void setLight(const bool on);
  bool getLight();
  uint16_t getClock();
  void setClock(uint16_t minspastmidnight);
  uint8_t* getRaw(const bool calcchecksum = true);
  void setRaw(const uint8_t new_code[],
              const uint16_t length = kWhirlpoolAcStateLength);
  static bool validChecksum(uint8_t state[],
                            const uint16_t length = kWhirlpoolAcStateLength);
#ifdef ARDUINO
  String toString();
#else
  std::string toString();
#endif

#ifndef UNIT_TEST
 private:
#endif
  // The state of the IR remote in IR code form.
  uint8_t remote_state[kWhirlpoolAcStateLength];
  void checksum(const uint16_t length = kWhirlpoolAcStateLength);
  IRsend _irsend;
#ifdef ARDUINO
  String timeToString(uint16_t minspastmidnight);
#else
  std::string timeToString(uint16_t minspastmidnight);
#endif
};

#endif  // IR_WHIRLPOOL_H_
