// Goodweather A/C
//
// Copyright 2019 ribeirodanielf
// Copyright 2019 David Conran

#ifndef IR_GOODWEATHER_H_
#define IR_GOODWEATHER_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#ifndef UNIT_TEST
#include <Arduino.h>
#else
#include <string>
#endif
#include "IRremoteESP8266.h"
#include "IRsend.h"
#ifdef UNIT_TEST
#include "IRsend_test.h"
#endif

// Supports:
//   ZH/JT-03 remote controller
// Ref:
//   https://github.com/markszabo/IRremoteESP8266/issues/697

// Constants

// Timing
const uint16_t kGoodweatherTick = 620;
const uint16_t kGoodweatherBitMarkTicks = 1;
const uint16_t kGoodweatherBitMark = kGoodweatherBitMarkTicks *
    kGoodweatherTick;
const uint16_t kGoodweatherOneSpaceTicks = 1;
const uint16_t kGoodweatherOneSpace = kGoodweatherOneSpaceTicks *
    kGoodweatherTick;
const uint16_t kGoodweatherZeroSpaceTicks = 3;
const uint16_t kGoodweatherZeroSpace = kGoodweatherZeroSpaceTicks *
    kGoodweatherTick;
const uint16_t kGoodweatherHdrMarkTicks = 11;
const uint16_t kGoodweatherHdrMark = kGoodweatherHdrMarkTicks *
    kGoodweatherTick;
const uint16_t kGoodweatherHdrSpaceTicks = 11;
const uint16_t kGoodweatherHdrSpace = kGoodweatherHdrSpaceTicks *
    kGoodweatherTick;

// Masks
const uint8_t kGoodweatherBitLight = 8;
const uint64_t kGoodweatherLightMask = 0x1ULL << kGoodweatherBitLight;
const uint8_t kGoodweatherBitTurbo = 11;
const uint64_t kGoodweatherTurboMask = 0x1ULL << kGoodweatherBitTurbo;
const uint8_t kGoodweatherBitSleep = 24;
const uint64_t kGoodweatherSleepMask = 0x1ULL << kGoodweatherBitSleep;
const uint8_t kGoodweatherBitPower = kGoodweatherBitSleep + 1;  // 25
const uint64_t kGoodweatherPowerMask = 0x1ULL << kGoodweatherBitPower;
const uint8_t kGoodweatherBitSwing = kGoodweatherBitPower + 1;  // 26
const uint64_t kGoodweatherSwingMask = 0x3ULL << kGoodweatherBitSwing;
const uint8_t kGoodweatherBitFan = kGoodweatherBitSwing + 3;  // 29
const uint64_t kGoodweatherFanMask = 0x3ULL << kGoodweatherBitFan;
const uint8_t kGoodweatherBitTemp = kGoodweatherBitFan + 3;  // 32
const uint64_t kGoodweatherTempMask = 0xFULL << kGoodweatherBitTemp;
const uint8_t kGoodweatherBitMode = kGoodweatherBitFan + 8;  // 37
const uint64_t kGoodweatherModeMask = 0x7ULL << kGoodweatherBitMode;

// Modes
const uint8_t kGoodweatherAuto = 0b000;
const uint8_t kGoodweatherCool = 0b001;
const uint8_t kGoodweatherDry =  0b010;
const uint8_t kGoodweatherFan =  0b011;
const uint8_t kGoodweatherHeat = 0b100;
const uint8_t kGoodweatherSwingFast = 0b00;
const uint8_t kGoodweatherSwingSlow = 0b01;
const uint8_t kGoodweatherSwingOff =  0b10;
// Fan Control
const uint8_t kGoodweatherFanAuto = 0b00;
const uint8_t kGoodweatherFanHigh = 0b01;
const uint8_t kGoodweatherFanMed =  0b10;
const uint8_t kGoodweatherFanLow =  0b11;

// Temperature
const uint8_t kGoodweatherTempMin = 16;  // Celsius
const uint8_t kGoodweatherTempMax = 31;  // Celsius


// Classes
class IRGoodweatherAc {
 public:
  explicit IRGoodweatherAc(uint16_t pin);

  void stateReset(void);
#if SEND_GOODWEATHER
  void send(const uint16_t repeat = kGoodweatherMinRepeat);
#endif  // SEND_GOODWEATHER
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
  uint8_t getMode();
  void setSwing(const uint8_t speed);
  uint8_t getSwing(void);
  void setSleep(const bool toggle);
  bool getSleep(void);
  void setTurbo(const bool toggle);
  bool getTurbo(void);
  void setLight(const bool toggle);
  bool getLight(void);
  uint64_t getRaw(void);
  void setRaw(const uint64_t state);
  uint8_t convertMode(const stdAc::opmode_t mode);
  uint8_t convertFan(const stdAc::fanspeed_t speed);
  uint8_t convertSwingV(const stdAc::swingv_t swingv);
#ifdef ARDUINO
  String toString();
#else
  std::string toString();
#endif
#ifndef UNIT_TEST

 private:
  IRsend _irsend;
#else
  IRsendTest _irsend;
#endif
  uint64_t remote;  // The state of the IR remote in IR code form.
};
#endif  // IR_GOODWEATHER_H_
