// Copyright 2016 David Conran

/// @file
/// @brief Support for Green A/C protocols.
/// @see https://github.com/ToniA/arduino-heatpumpir/blob/master/GreeHeatpumpIR.h

// Supports:
//   Brand: Green,  Model: YBOFB remote
//   Brand: Green,  Model: GZ01-BEJ0 remote


#ifndef IR_GREEN_H_
#define IR_GREEN_H_

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

// Constants

const uint8_t kGreenAuto = 0;
const uint8_t kGreenCool = 3;
const uint8_t kGreenDry = 2;
const uint8_t kGreenFan = 7;
const uint8_t kGreenHeat = 1;
//const uint8_t kGreenAcModeOffset = 5;

// Byte[5]
const uint8_t kGreenPower1Offset = 2;


const uint8_t kGreenSleepOffset = 7;
// Byte[7]
const uint8_t kGreenTempOffset = 0;
const uint8_t kGreenTempSize = 4;            // Mask 0b0000xxxx
const uint8_t kGreenMinTempC = 16;  // Celsius
const uint8_t kGreenMaxTempC = 31;  // Celsius
const uint8_t kGreenMinTempF = 61;  // Fahrenheit
const uint8_t kGreenMaxTempF = 86;  // Fahrenheit
const uint8_t kGreenTimerHalfHrOffset = 4;   // Mask 0b000x0000
const uint8_t kGreenTimerTensHrOffset = 5;
const uint8_t kGreenTimerTensHrSize = 2;     // Mask 0b0xx00000
const uint16_t kGreenTimerMax = 24 * 60;
const uint8_t kGreenTimerEnabledOffset = 7;  // Mask 0bx0000000
// Byte[8]
const uint8_t kGreenFanOffset = 0;
const uint8_t kGreenFanSize = 3;  // Bits
const uint8_t kGreenFanAuto = 0;
const uint8_t kGreenFanMin = 2;
const uint8_t kGreenFanMed = 3;
const uint8_t kGreenFanMax = 5;
const uint8_t kGreenSwingSize = 3;  // Bits
const uint8_t kGreenSwingOffset = 3;
const uint8_t kGreenSwingAutoOffset = 3;
const uint8_t kGreenSwingLastPos =    0b0000;
const uint8_t kGreenSwingAuto =       0b0111;
const uint8_t kGreenSwingUp =         0b0001;
const uint8_t kGreenSwingMiddleUp =   0b0010;
const uint8_t kGreenSwingMiddle =     0b0011;
const uint8_t kGreenSwingMiddleDown = 0b0100;
const uint8_t kGreenSwingDown =       0b0101;
const uint8_t kGreenSwingDownAuto =   0b0111;
//const uint8_t kGreenSwingMiddleAuto = 0b1001;
//const uint8_t kGreenSwingUpAuto =     0b1011;


const uint8_t kGreenTimerHoursOffset = 0;
const uint8_t kGreenTimerHoursSize = 4;  // Bits
const uint8_t kGreenTurboOffset = 4;
const uint8_t kGreenLightOffset = 5;
// This might not be used. See #814
const uint8_t kGreenPower2Offset = 6;
const uint8_t kGreenXfanOffset = 7;
// Byte[3]
const uint8_t kGreenTempExtraDegreeFOffset = 2;  // Mask 0b00000x00
const uint8_t kGreenUseFahrenheitOffset = 3;     // Mask 0b0000x000
// Byte[4]

// Byte[5]
const uint8_t kGreenWiFiOffset = 6;       // Mask 0b0x000000
const uint8_t kGreenIFeelOffset = 2;      // Mask 0b00000x00
const uint8_t kGreenDisplayTempOffset = 0;
const uint8_t kGreenDisplayTempSize = 2;  // Mask 0b000000xx
const uint8_t kGreenDisplayTempOff =                    0b00;  // 0
const uint8_t kGreenDisplayTempSet =                    0b01;  // 1
const uint8_t kGreenDisplayTempInside =                 0b10;  // 2
const uint8_t kGreenDisplayTempOutside =                0b11;  // 3


// Legacy defines.
#define GREEN_AUTO kGreenAuto
#define GREEN_COOL kGreenCool
#define GREEN_DRY kGreenDry
#define GREEN_FAN kGreenFan
#define GREEN_HEAT kGreenHeat
#define GREEN_MIN_TEMP kGreenMinTempC
#define GREEN_MAX_TEMP kGreenMaxTempC
#define GREEN_FAN_MAX kGreenFanMax
#define GREEN_SWING_LAST_POS kGreenSwingLastPos
#define GREEN_SWING_AUTO kGreenSwingAuto
#define GREEN_SWING_UP kGreenSwingUp
#define GREEN_SWING_MIDDLE_UP kGreenSwingMiddleUp
#define GREEN_SWING_MIDDLE kGreenSwingMiddle
#define GREEN_SWING_MIDDLE_DOWN kGreenSwingMiddleDown
#define GREEN_SWING_DOWN kGreenSwingDown
#define GREEN_SWING_DOWN_AUTO kGreenSwingDownAuto
#define GREEN_SWING_MIDDLE_AUTO kGreenSwingMiddleAuto
#define GREEN_SWING_UP_AUTO kGreenSwingUpAuto

// Classes
/// Class for handling detailed Green A/C messages.
class IRGreenAC {
 public:
  explicit IRGreenAC(
      const uint16_t pin,
      const green_ac_remote_model_t model = green_ac_remote_model_t::YAW1FG,
      const bool inverted = false, const bool use_modulation = true);
  void stateReset(void);
#if SEND_GREEN
  void send(const uint16_t repeat = kGreenDefaultRepeat);
  /// Run the calibration to calculate uSec timing offsets for this platform.
  /// @return The uSec timing offset needed per modulation of the IR Led.
  /// @note This will produce a 65ms IR signal pulse at 38kHz.
  ///   Only ever needs to be run once per object instantiation, if at all.
  int8_t calibrate(void) { return _irsend.calibrate(); }
#endif  // SEND_GREEN
  void begin(void);
  void on(void);
  void off(void);
  void setModel(const green_ac_remote_model_t model);
  green_ac_remote_model_t getModel(void);
  void setPower(const bool on);
  bool getPower(void);
  void setTemp(const uint8_t temp, const bool fahrenheit = false);
  uint8_t getTemp(void);
  void setUseFahrenheit(const bool on);
  bool getUseFahrenheit(void);
  void setFan(const uint8_t speed);
  uint8_t getFan(void);
  void setMode(const uint8_t new_mode);
  uint8_t getMode(void);
  void setLight(const bool on);
  bool getLight(void);
  void setXFan(const bool on);
  bool getXFan(void);
  void setSleep(const bool on);
  bool getSleep(void);
  void setTurbo(const bool on);
  bool getTurbo(void);
  //void setIFeel(const bool on);
  //bool getIFeel(void);
  //void setWiFi(const bool on);
  //bool getWiFi(void);
  void setSwingVertical(const bool automatic, const uint8_t position);
  bool getSwingVerticalAuto(void);
  uint8_t getSwingVerticalPosition(void);
  uint16_t getTimer(void);
  void setTimer(const uint16_t minutes);
  void setDisplayTempSource(const uint8_t mode);
  uint8_t getDisplayTempSource(void);
  uint8_t convertMode(const stdAc::opmode_t mode);
  uint8_t convertFan(const stdAc::fanspeed_t speed);
  uint8_t convertSwingV(const stdAc::swingv_t swingv);
  static stdAc::opmode_t toCommonMode(const uint8_t mode);
  static stdAc::fanspeed_t toCommonFanSpeed(const uint8_t speed);
  static stdAc::swingv_t toCommonSwingV(const uint8_t pos);
  stdAc::state_t toCommon(void);
  uint8_t* getRaw(void);
  void setRaw(const uint8_t new_code[]);
  static bool validChecksum(const uint8_t state[],
                            const uint16_t length = kGreenStateLength);
  String toString(void);
#ifndef UNIT_TEST

 private:
  IRsend _irsend;  ///< Instance of the IR send class
#else  // UNIT_TEST
  /// @cond IGNORE
  IRsendTest _irsend;  ///< Instance of the testing IR send class
  /// @endcond
#endif  // UNIT_TEST
  uint8_t remote_state[kGreenStateLength];  ///< The state in native IR code form
  green_ac_remote_model_t _model;
  void checksum(const uint16_t length = kGreenStateLength);
  void fixup(void);
  void setTimerEnabled(const bool on);
  bool getTimerEnabled(void);
};

#endif  // IR_GREEN_H_
