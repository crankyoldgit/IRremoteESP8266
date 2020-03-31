// Samsung A/C
//
// Copyright 2018 David Conran

#ifndef IR_SAMSUNG_H_
#define IR_SAMSUNG_H_

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
//   Brand: Samsung,  Model: UA55H6300 TV
//   Brand: Samsung,  Model: DB63-03556X003 remote
//   Brand: Samsung,  Model: DB93-16761C remote
//   Brand: Samsung,  Model: IEC-R03 remote
//   Brand: Samsung,  Model: AR09FSSDAWKNFA A/C
//   Brand: Samsung,  Model: AR12KSFPEWQNET A/C
//   Brand: Samsung,  Model: AR12HSSDBWKNEU A/C
//   Brand: Samsung,  Model: AR12NXCXAWKXEU A/C

// Ref:
//   https://github.com/crankyoldgit/IRremoteESP8266/issues/505
//   https://github.com/crankyoldgit/IRremoteESP8266/issues/1062

// Constants
// Byte[1]
// Checksum                                        0b11110000 ???
const uint8_t kSamsungAcPower1Offset = 5;  // Mask 0b00100000
const uint8_t kSamsungAcQuiet1Offset = 4;  // Mask 0b00010000
// Byte[5]
const uint8_t kSamsungAcQuiet5Offset = 5;
// Byte[6]
const uint8_t kSamsungAcPower6Offset = 4;  // Mask 0b00110000
const uint8_t kSamsungAcPower6Size = 2;  // Bits
// Byte[8]
// Checksum                             0b11110000 ???
const uint8_t kSamsungAcPowerfulMask8 = 0b01010000;
// Byte[9]
const uint8_t kSamsungAcSwingOffset = 4;  // Mask 0b01110000
const uint8_t kSamsungAcSwingSize = 3;  // Bits
const uint8_t kSamsungAcSwingMove =                0b010;
const uint8_t kSamsungAcSwingStop =                0b111;
// Byte[10]
const uint8_t kSamsungAcPowerful10Offset = 1;  // Mask 0b00001110
const uint8_t kSamsungAcPowerful10Size = 3;    // Mask 0b00001110
const uint8_t kSamsungAcPowerful10On =                     0b011;
// Breeze (aka. WindFree)
const uint8_t kSamsungAcBreezeOffset = kSamsungAcPowerful10Offset;
const uint8_t kSamsungAcBreezeSize = kSamsungAcPowerful10Size;
const uint8_t kSamsungAcBreezeOn =                         0b101;
const uint8_t kSamsungAcDisplayOffset = 4;     // Mask 0b00010000
const uint8_t kSamsungAcClean10Offset = 7;     // Mask 0b10000000
// Byte[11]
const uint8_t kSamsungAcIonOffset = 0;      // Mask 0b00000001
const uint8_t kSamsungAcClean11Offset = 1;  // Mask 0b00000010
const uint8_t kSamsungAcMinTemp = 16;   // C   Mask 0b11110000
const uint8_t kSamsungAcMaxTemp = 30;   // C   Mask 0b11110000
const uint8_t kSamsungAcAutoTemp = 25;  // C   Mask 0b11110000
// Byte[12]
const uint8_t kSamsungAcModeOffset = 4;  // Mask 0b01110000
const uint8_t kSamsungAcAuto = 0;
const uint8_t kSamsungAcCool = 1;
const uint8_t kSamsungAcDry = 2;
const uint8_t kSamsungAcFan = 3;
const uint8_t kSamsungAcHeat = 4;
const uint8_t kSamsungAcFanOffest = 1;   // Mask 0b00001110
const uint8_t kSamsungAcFanSize = 3;  // Bits
const uint8_t kSamsungAcFanAuto = 0;
const uint8_t kSamsungAcFanLow = 2;
const uint8_t kSamsungAcFanMed = 4;
const uint8_t kSamsungAcFanHigh = 5;
const uint8_t kSamsungAcFanAuto2 = 6;
const uint8_t kSamsungAcFanTurbo = 7;
// Byte[13]
const uint8_t kSamsungAcBeepOffset = 1;  // Mask 0b00000010

const uint16_t kSamsungAcSectionLength = 7;
const uint64_t kSamsungAcPowerSection = 0x1D20F00000000;

// Classes
class IRSamsungAc {
 public:
  explicit IRSamsungAc(const uint16_t pin, const bool inverted = false,
                       const bool use_modulation = true);

  void stateReset(const bool forcepower = true, const bool initialPower = true);
#if SEND_SAMSUNG_AC
  void send(const uint16_t repeat = kSamsungAcDefaultRepeat,
            const bool calcchecksum = true);
  void sendExtended(const uint16_t repeat = kSamsungAcDefaultRepeat,
                    const bool calcchecksum = true);
  void sendOn(const uint16_t repeat = kSamsungAcDefaultRepeat);
  void sendOff(const uint16_t repeat = kSamsungAcDefaultRepeat);
  uint8_t calibrate(void) { return _irsend.calibrate(); }
#endif  // SEND_SAMSUNG_AC
  void begin(void);
  void on(void);
  void off(void);
  void setPower(const bool on);
  bool getPower(void);
  void setTemp(const uint8_t temp);
  uint8_t getTemp(void);
  void setFan(const uint8_t speed);
  uint8_t getFan(void);
  void setMode(const uint8_t mode);
  uint8_t getMode(void);
  void setSwing(const bool on);
  bool getSwing(void);
  void setBeep(const bool on);
  bool getBeep(void);
  void setClean(const bool on);
  bool getClean(void);
  void setQuiet(const bool on);
  bool getQuiet(void);
  void setPowerful(const bool on);
  bool getPowerful(void);
  void setBreeze(const bool on);
  bool getBreeze(void);
  void setDisplay(const bool on);
  bool getDisplay(void);
  void setIon(const bool on);
  bool getIon(void);

  uint8_t* getRaw(void);
  void setRaw(const uint8_t new_code[],
              const uint16_t length = kSamsungAcStateLength);
  static bool validChecksum(const uint8_t state[],
                            const uint16_t length = kSamsungAcStateLength);
  static uint8_t calcChecksum(const uint8_t state[],
                              const uint16_t length = kSamsungAcStateLength);
  uint8_t convertMode(const stdAc::opmode_t mode);
  uint8_t convertFan(const stdAc::fanspeed_t speed);
  static stdAc::opmode_t toCommonMode(const uint8_t mode);
  static stdAc::fanspeed_t toCommonFanSpeed(const uint8_t speed);
  stdAc::state_t toCommon(void);
  String toString(void);
#ifndef UNIT_TEST

 private:
  IRsend _irsend;
#else
  IRsendTest _irsend;
#endif
  // The state of the IR remote in IR code form.
  uint8_t remote_state[kSamsungAcExtendedStateLength];
  bool _forcepower;  // Hack to know when we need to send a special power mesg.
  bool _lastsentpowerstate;
  void checksum(const uint16_t length = kSamsungAcStateLength);
};

#endif  // IR_SAMSUNG_H_
