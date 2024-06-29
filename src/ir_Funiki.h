// Copyright 2024 QuangThai2297

/// @file
/// @brief Support for FUNIKI A/C protocols.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/2112



#ifndef IR_FUNIKI_H_
#define IR_FUNIKI_H_

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

/// Native representation of a Funiki A/C message.
union FunikiProtocol{
  uint8_t remote_state[kFunikiStateLength];
  ///< The state in native IR code form
  struct {
    // BYte 0
    uint8_t Mode      :2;
    uint8_t Sleep     :1;
    uint8_t Power     :1;
    uint8_t Fan       :2;
    uint8_t TimerOnEnable :1;
    uint8_t :1;
    // Byte 1
    uint8_t Temp      :4;
    uint8_t TimerOnHours1 :2;
    uint8_t :2;
    // Byte 2
    uint8_t TimerOnHours2 :4;
    uint8_t TimerOnMinutes :3;
    uint8_t :1;
    // Byte 3
    uint8_t unknown1;
    // Byte 4
    uint8_t :4;
    uint8_t :1;
    uint8_t TimerOffEnable :1;
    uint8_t AutoMode   :1;
    uint8_t :1;
    // Byte 5
    uint8_t SwingV     :3;
    uint8_t :5;
    // Byte 6
    uint8_t TimerOffHours1:4;
    uint8_t TimerOffHours2:4;
    // Byte 7
    uint8_t TimerOffMinutes:3;
    uint8_t :5;
    // Byte 8
    uint8_t Hours1:4;
    uint8_t Hours2:4;
    // Byte 9
    uint8_t Minutes1:4;
    uint8_t Minutes2:4;
  };
};

// Constants

const uint8_t kFunikiCool  = 0;
const uint8_t kFunikiDry   = 3;
const uint8_t kFunikiFan   = 1;
const uint8_t kFunikiAuto  = 2;
const uint8_t kFunikiHeat  = 4;

const uint8_t kFunikiAutoModeOff  = 0;
const uint8_t kFunikiAutoModeOn   = 1;

const uint8_t kFunikiFanAuto = 0;
const uint8_t kFunikiFanMin  = 1;
const uint8_t kFunikiFanMed  = 2;
const uint8_t kFunikiFanMax  = 3;

const uint8_t kFunikiMinTempC = 16;  // Celsius
const uint8_t kFunikiMaxTempC = 31;  // Celsius
const uint8_t kFunikiMinTempF = 61;  // Fahrenheit
const uint8_t kFunikiMaxTempF = 86;  // Fahrenheit
const uint16_t kFunikiTimerMax = 24 * 60;

const uint8_t kFunikiSwingLastPos    = 0b000;  // 0
const uint8_t kFunikiSwingAuto       = 0b001;  // 1
const uint8_t kFunikiSwingUp         = 0b010;  // 2
const uint8_t kFunikiSwingMiddleUp   = 0b011;  // 3
const uint8_t kFunikiSwingMiddle     = 0b100;  // 4
const uint8_t kFunikiSwingMiddleDown = 0b101;  // 5
const uint8_t kFunikiSwingDown       = 0b110;  // 6

const uint8_t kFunikiSwingHOff        = 0b000;  // 0
const uint8_t kFunikiSwingHAuto       = 0b001;  // 1
const uint8_t kFunikiSwingHMaxLeft    = 0b010;  // 2
const uint8_t kFunikiSwingHLeft       = 0b011;  // 3
const uint8_t kFunikiSwingHMiddle     = 0b100;  // 4
const uint8_t kFunikiSwingHRight      = 0b101;  // 5
const uint8_t kFunikiSwingHMaxRight   = 0b110;  // 6

const uint8_t kFunikiDisplayTempOff     = 0b00;  // 0
const uint8_t kFunikiDisplayTempSet     = 0b01;  // 1
const uint8_t kFunikiDisplayTempInside  = 0b10;  // 2
const uint8_t kFunikiDisplayTempOutside = 0b11;  // 3

// Legacy defines.
#define FUINIKI_AUTO kFunikiAuto
#define FUINIKI_COOL kFunikiCool
#define FUINIKI_DRY kFunikiDry
#define FUINIKI_FAN kFunikiFan
#define FUINIKI_HEAT kFunikiHeat
#define FUINIKI_MIN_TEMP kFunikiMinTempC
#define FUINIKI_MAX_TEMP kFunikiMaxTempC
#define FUINIKI_FAN_MAX kFunikiFanMax
#define FUINIKI_SWING_LAST_POS kFunikiSwingLastPos
#define FUINIKI_SWING_AUTO kFunikiSwingAuto
#define FUINIKI_SWING_UP kFunikiSwingUp
#define FUINIKI_SWING_MIDDLE_UP kFunikiSwingMiddleUp
#define FUINIKI_SWING_MIDDLE kFunikiSwingMiddle
#define FUINIKI_SWING_MIDDLE_DOWN kFunikiSwingMiddleDown
#define FUINIKI_SWING_DOWN kFunikiSwingDown
#define FUINIKI_SWING_DOWN_AUTO kFunikiSwingDownAuto
#define FUINIKI_SWING_MIDDLE_AUTO kFunikiSwingMiddleAuto
#define FUINIKI_SWING_UP_AUTO kFunikiSwingUpAuto

// Classes
/// Class for handling detailed Funiki A/C messages.
class IRFunikiAC {
 public:
  explicit IRFunikiAC(
      const uint16_t pin,
      const funiki_ac_remote_model_t model = funiki_ac_remote_model_t::UNKOWN,
      const bool inverted = false, const bool use_modulation = true);
  void stateReset(void);
#if SEND_FUNIKI
  void send(const uint16_t repeat = kFunikiDefaultRepeat);
  /// Run the calibration to calculate uSec timing offsets for this platform.
  /// @return The uSec timing offset needed per modulation of the IR Led.
  /// @note This will produce a 65ms IR signal pulse at 38kHz.
  ///   Only ever needs to be run once per object instantiation, if at all.
  int8_t calibrate(void) { return _irsend.calibrate(); }
#endif  // SEND_FUNIKI
  void begin(void);
  void on(void);
  void off(void);
  void setModel(const funiki_ac_remote_model_t model);
  funiki_ac_remote_model_t getModel(void) const;
  void setPower(const bool on);
  bool getPower(void) const;
  void setTemp(const uint8_t temp, const bool fahrenheit = false);
  uint8_t getTemp(void) const;
  void setFan(const uint8_t speed);
  uint8_t getFan(void) const;
  void setMode(const uint8_t new_mode);
  uint8_t getMode(void) const;
  void setSleep(const bool on);
  bool getSleep(void) const;
  void setSwingVertical(const bool automatic, const uint8_t position);
  bool getSwingVerticalAuto(void) const;
  uint8_t getSwingVerticalPosition(void) const;
  int16_t getClock(void) const;
  void setClock(const int16_t nr_of_minutes);
  uint16_t getTimerOn(void) const;
  uint16_t getTimerOff(void) const;
  bool getTimerOnEnabled(void) const;
  bool getTimerOffEnabled(void) const;
  // void setTimer(const uint16_t minutes);

  static uint8_t convertMode(const stdAc::opmode_t mode);
  static uint8_t convertFan(const stdAc::fanspeed_t speed);
  static uint8_t convertSwingV(const stdAc::swingv_t swingv);
  static stdAc::opmode_t toCommonMode(const uint8_t mode, uint8_t isAutoMode);
  static stdAc::fanspeed_t toCommonFanSpeed(const uint8_t speed);
  static stdAc::swingv_t toCommonSwingV(const uint8_t pos);
  stdAc::state_t toCommon(void);
  uint8_t* getRaw(void);
  void setRaw(const uint8_t new_code[]);
  static bool validChecksum(const uint8_t state[],
                            const uint16_t length = kFunikiStateLength);
  String toString(void);
#ifndef UNIT_TEST

 private:
  IRsend _irsend;  ///< Instance of the IR send class
#else  // UNIT_TEST
  /// @cond IGNORE
  IRsendTest _irsend;  ///< Instance of the testing IR send class
  /// @endcond
#endif  // UNIT_TEST
  FunikiProtocol _;
  funiki_ac_remote_model_t _model;
  void checksum(const uint16_t length = kFunikiStateLength);
  void fixup(void);
  // void setTimerEnabled(const bool on);
  // bool getTimerEnabled(void) const;
};

#endif  // IR_FUNIKI_H_
