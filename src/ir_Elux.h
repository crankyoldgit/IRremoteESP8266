// Copyright 2016 David Conran

/// @file
/// @brief Support for Elux A/C protocols.
/// @see https://github.com/ToniA/arduino-heatpumpir/blob/master/EluxHeatpumpIR.h

// Supports:
//   Brand: Elux,  Model: YBOFB remote
//   Brand: Elux,  Model: GZ01-BEJ0 remote


#ifndef IR_ELUX_H_
#define IR_ELUX_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include "IRremoteESP8266.h"
#include "IRsend.h"
#ifdef UNIT_TEST
//#include "IRsend_test.h"
#endif

// Constants

const uint8_t kEluxAuto = 1;
const uint8_t kEluxCool = 2;
const uint8_t kEluxDry = 3;
const uint8_t kEluxFan = 7;
const uint8_t kEluxHeat = 0;
//const uint8_t kEluxAcModeOffset = 5;

// Byte[2]
const uint8_t kEluxPower1Offset = 2;
const uint8_t kEluxFanOffset = 0;
const uint8_t kEluxFanSize = 2;  // Bits
const uint8_t kEluxFanAuto = 0;
const uint8_t kEluxFanMin = 1;
const uint8_t kEluxFanMed = 2;  //was 1=Med is ok; 
const uint8_t kEluxFanMax = 3;  //was 2

// Byte[3]
const uint8_t kEluxTempOffset = 4;
const uint8_t kEluxTempSize = 4;            // Mask 0b0000xxxx
const uint8_t kEluxMinTempC = 18;  // Celsius
const uint8_t kEluxMaxTempC = 31;  // Celsius
const uint8_t kEluxMinTempF = 61;  // Fahrenheit
const uint8_t kEluxMaxTempF = 86;  // Fahrenheit

// Byte[4]  unknown

// Byte[5]
const uint8_t kEluxWiFiOffset = 6;       // Mask 0b0x000000
const uint8_t kEluxIFeelOffset = 2;      // Mask 0b00000x00
const uint8_t kEluxDisplayTempOffset = 0;
const uint8_t kEluxDisplayTempSize = 2;  // Mask 0b000000xx
const uint8_t kEluxDisplayTempOff =                    0b00;  // 0
const uint8_t kEluxDisplayTempSet =                    0b01;  // 1
const uint8_t kEluxDisplayTempInside =                 0b10;  // 2
const uint8_t kEluxDisplayTempOutside =                0b11;  // 3


const uint8_t kEluxSleepOffset = 7;
// Byte[7]

const uint8_t kEluxTimerHalfHrOffset = 4;   // Mask 0b000x0000
const uint8_t kEluxTimerTensHrOffset = 5;
const uint8_t kEluxTimerTensHrSize = 2;     // Mask 0b0xx00000
const uint16_t kEluxTimerMax = 24 * 60;
const uint8_t kEluxTimerEnabledOffset = 7;  // Mask 0bx0000000
// Byte[8]

const uint8_t kEluxSwingSize = 3;  // Bits
const uint8_t kEluxSwingOffset = 3;
const uint8_t kEluxSwingAutoOffset = 3;
const uint8_t kEluxSwingLastPos =    0b0000;
const uint8_t kEluxSwingAuto =       0b0111;
const uint8_t kEluxSwingUp =         0b0001;
const uint8_t kEluxSwingMiddleUp =   0b0010;
const uint8_t kEluxSwingMiddle =     0b0011;
const uint8_t kEluxSwingMiddleDown = 0b0100;
const uint8_t kEluxSwingDown =       0b0101;
const uint8_t kEluxSwingDownAuto =   0b0111;
//const uint8_t kEluxSwingMiddleAuto = 0b1001;
//const uint8_t kEluxSwingUpAuto =     0b1011;


const uint8_t kEluxTimerHoursOffset = 0;
const uint8_t kEluxTimerHoursSize = 4;  // Bits
const uint8_t kEluxTurboOffset = 4;
const uint8_t kEluxLightOffset = 5;
// This might not be used. See #814
const uint8_t kEluxPower2Offset = 6;
const uint8_t kEluxXfanOffset = 7;
// Byte[3]
const uint8_t kEluxTempExtraDeeluxFOffset = 2;  // Mask 0b00000x00
const uint8_t kEluxUseFahrenheitOffset = 3;     // Mask 0b0000x000



// Legacy defines.
#define ELUX_AUTO kEluxAuto
#define ELUX_COOL kEluxCool
#define ELUX_DRY kEluxDry
#define ELUX_FAN kEluxFan
#define ELUX_HEAT kEluxHeat
#define ELUX_MIN_TEMP kEluxMinTempC
#define ELUX_MAX_TEMP kEluxMaxTempC
#define ELUX_FAN_MAX kEluxFanMax
#define ELUX_SWING_LAST_POS kEluxSwingLastPos
#define ELUX_SWING_AUTO kEluxSwingAuto
#define ELUX_SWING_UP kEluxSwingUp
#define ELUX_SWING_MIDDLE_UP kEluxSwingMiddleUp
#define ELUX_SWING_MIDDLE kEluxSwingMiddle
#define ELUX_SWING_MIDDLE_DOWN kEluxSwingMiddleDown
#define ELUX_SWING_DOWN kEluxSwingDown
#define ELUX_SWING_DOWN_AUTO kEluxSwingDownAuto
#define ELUX_SWING_MIDDLE_AUTO kEluxSwingMiddleAuto
#define ELUX_SWING_UP_AUTO kEluxSwingUpAuto

// Classes
/// Class for handling detailed Elux A/C messages.
class IREluxAC {
 public:
  explicit IREluxAC(
      const uint16_t pin,
      const elux_ac_remote_model_t model = elux_ac_remote_model_t::EAW1F,
      const bool inverted = false, const bool use_modulation = true);
  void stateReset(void);
#if SEND_ELUX
  void send(const uint16_t repeat = kEluxDefaultRepeat);
  /// Run the calibration to calculate uSec timing offsets for this platform.
  /// @return The uSec timing offset needed per modulation of the IR Led.
  /// @note This will produce a 65ms IR signal pulse at 38kHz.
  ///   Only ever needs to be run once per object instantiation, if at all.
  int8_t calibrate(void) { return _irsend.calibrate(); }
#endif  // SEND_ELUX
  void begin(void);
  void on(void);
  void off(void);
  void setModel(const elux_ac_remote_model_t model);
  elux_ac_remote_model_t getModel(void);
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
                            const uint16_t length = kEluxStateLength);
  String toString(void);
#ifndef UNIT_TEST

 private:
  IRsend _irsend;  ///< Instance of the IR send class
#else  // UNIT_TEST
  /// @cond IGNORE
  IRsendTest _irsend;  ///< Instance of the testing IR send class
  /// @endcond
#endif  // UNIT_TEST
  uint8_t remote_state[kEluxStateLength];  ///< The state in native IR code form
  elux_ac_remote_model_t _model;
  void checksum(const uint16_t length = kEluxStateLength);
  void fixup(void);
  void setTimerEnabled(const bool on);
  bool getTimerEnabled(void);
};

#endif  // IR_ELUX_H_
