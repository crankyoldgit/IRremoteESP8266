// Copyright 2018 Erdem U. Altinyurt
// Copyright 2019 David Conran

#ifndef IR_VESTEL_H_
#define IR_VESTEL_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#ifdef ARDUINO
#include <Arduino.h>
#else
#include <string>
#endif
#include "IRremoteESP8266.h"
#include "IRsend.h"

//                 VV     VV  EEEEEEE   SSSSS  TTTTTTTT  EEEEEEE  LL
//                 VV     VV  EE       S          TT     EE       LL
//                  VV   VV   EEEEE     SSSS      TT     EEEEE    LL
//                   VV VV    EE            S     TT     EE       LL
//                    VVV     EEEEEEE  SSSSS      TT     EEEEEEE  LLLLLLL

// Vestel added by Erdem U. Altinyurt

// Structure of a Command message (56 bits)
//   Signature: 12 bits. e.g. 0x201
//   Checksum: 8 bits
//   Swing: 4 bits. (auto 0xA, stop 0xF)
//   turbo_sleep_normal: 4bits. (normal 0x1, sleep 0x3, turbo 0x7)
//   Unused: 8 bits. (0x00)
//   Temperature: 4 bits. (Celcius, but offset by -16 degrees. e.g. 0x0 = 16C)
//   Fan Speed: 4 bits (auto 0x1, low 0x5, mid 0x9, high 0xB, 0xD auto hot,
//                    0xC auto cool)
//   Mode: 3 bits. (auto 0x0, cold 0x1, dry 0x2, fan 0x3, hot 0x4)
//   unknown/unused: 6 bits.
//   Ion flag: 1 bit.
//   unknown/unused: 1 bit.
//   Power/message type: 4 bits. (on 0xF, off 0xC, 0x0 == Timer mesage)
//
// Structure of a Time(r) message (56 bits)
//   Signature: 12 bits. e.g. 0x201
//   Checksum: 8 bits
//   Off Minutes: 3 bits. (Stored in 10 min increments. eg. xx:20 is 0x2)
//   Off Hours: 5 bits. (0x17 == 11PM / 23:00)
//   On Minutes: 3 bits. (Stored in 10 min increments. eg. xx:20 is 0x2)
//   On Hours: 5 bits. (0x9 == 9AM / 09:00)
//   Clock Hours: 5 bits.
//   On Timer flag: 1 bit.
//   Off Timer flag: 1 bit.
//   Timer mode flag: 1 bit. (Off after X many hours/mins, not at clock time.)
//   Clock Minutes: 8 bits. (0-59)
//   Power/message type: 4 bits. (0x0 == Timer mesage, else see Comman message)

// Constants
const uint16_t kVestelACHdrMark = 3110;
const uint16_t kVestelACHdrSpace = 9066;
const uint16_t kVestelACBitMark = 520;
const uint16_t kVestelACOneSpace = 1535;
const uint16_t kVestelACZeroSpace = 480;
const uint16_t kVestelACTolerance = 30;

const uint8_t kVestelACMinTempH = 16;
const uint8_t kVestelACMinTempC = 18;
const uint8_t kVestelACMaxTemp = 30;

const uint64_t kVestelACCRCMask = 0xFFFFFFFFFFF00000;

const uint8_t kVestelACAuto = 0;
const uint8_t kVestelACCool = 1;
const uint8_t kVestelACDry = 2;
const uint8_t kVestelACFan = 3;
const uint8_t kVestelACHeat = 4;

const uint8_t kVestelACFanAuto = 1;
const uint8_t kVestelACFanLow = 5;
const uint8_t kVestelACFanMed = 9;
const uint8_t kVestelACFanHigh = 0xB;
const uint8_t kVestelACFanAutoCool = 0xC;
const uint8_t kVestelACFanAutoHot = 0xD;

const uint8_t kVestelACNormal = 1;
const uint8_t kVestelACSleep = 3;
const uint8_t kVestelACTurbo = 7;
const uint8_t kVestelACIon = 4;
const uint8_t kVestelACSwing = 0xA;

const uint8_t kVestelACChecksumOffset = 12;
const uint8_t kVestelACSwingOffset = 20;
const uint8_t kVestelACTurboSleepOffset = 24;
const uint8_t kVestelACTempOffset = 36;
const uint8_t kVestelACFanOffset = 40;
const uint8_t kVestelACModeOffset = 44;
const uint8_t kVestelACIonOffset = 50;
const uint8_t kVestelACPowerOffset = 52;
const uint8_t kVestelACOffTimeOffset = 20;
const uint8_t kVestelACOnTimeOffset = 28;
const uint8_t kVestelACHourOffset = 36;  // 5 bits
const uint8_t kVestelACOnTimerFlagOffset = kVestelACHourOffset + 5;
const uint8_t kVestelACOffTimerFlagOffset = kVestelACHourOffset + 6;
const uint8_t kVestelACTimerFlagOffset = kVestelACHourOffset + 7;
const uint8_t kVestelACMinuteOffset = 44;


class IRVestelAC {
 public:
  explicit IRVestelAC(uint16_t pin);

  void stateReset();
#if SEND_VESTEL_AC
  void send();
#endif  // SEND_VESTEL_AC
  void begin(void);
  void on(void);
  void off(void);
  void setPower(const bool state);
  bool getPower();
  void setAuto(const int8_t autoLevel);
  void setTimer(const uint16_t minutes);
  uint16_t getTimer(void);
  void setTime(const uint16_t minutes);
  uint16_t getTime(void);
  void setOnTimer(const uint16_t minutes);
  uint16_t getOnTimer(void);
  void setOffTimer(const uint16_t minutes);
  uint16_t getOffTimer(void);
  void setTemp(const uint8_t temp);
  uint8_t getTemp(void);
  void setFan(const uint8_t fan);
  uint8_t getFan(void);
  void setMode(const uint8_t mode);
  uint8_t getMode(void);
  void setRaw(uint8_t* newState);
  void setRaw(const uint64_t newState);
  uint64_t getRaw(void);
  static bool validChecksum(const uint64_t state);
  void setSwing(const bool state);
  bool getSwing(void);
  void setSleep(const bool state);
  bool getSleep(void);
  void setTurbo(const bool state);
  bool getTurbo(void);
  void setIon(const bool state);
  bool getIon(void);
  bool isTimeCommand(void);
  bool isOnTimerActive(void);
  void setOnTimerActive(const bool on);
  bool isOffTimerActive(void);
  void setOffTimerActive(const bool on);
  bool isTimerActive(void);
  void setTimerActive(const bool on);
  static uint8_t calcChecksum(const uint64_t state);
#ifdef ARDUINO
  String toString();
#else
  std::string toString();
#endif

 private:
  uint64_t remote_state;
  uint64_t remote_time_state;
  bool use_time_state = false;
  void checksum();
  IRsend _irsend;
};

#endif  // IR_VESTEL_H_
