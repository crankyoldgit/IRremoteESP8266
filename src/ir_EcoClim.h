// Copyright 2021 David Conran

/// @file
/// @brief EcoClim A/C protocol.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1397

// Supports:
//   Brand: EcoClim,  Model: HYSFR-P348 remote
//   Brand: EcoClim,  Model: ZC200DPO A/C

#ifndef IR_ECOCLIM_H_
#define IR_ECOCLIM_H_

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
// Modes
const uint8_t kEcoclimAuto =    0b000;  ///< 0. a.k.a Slave
const uint8_t kEcoclimCool =    0b001;  ///< 1
const uint8_t kEcoclimDry =     0b010;  ///< 2
const uint8_t kEcoclimRecycle = 0b011;  ///< 3
const uint8_t kEcoclimFan =     0b100;  ///< 4
const uint8_t kEcoclimHeat =    0b101;  ///< 5
const uint8_t kEcoclimSleep =   0b111;  ///< 7
// Fan Control
const uint8_t kEcoclimFanMin =  0b00;  ///< 0
const uint8_t kEcoclimFanMed =  0b01;  ///< 1
const uint8_t kEcoclimFanMax =  0b10;  ///< 2
const uint8_t kEcoclimFanAuto = 0b11;  ///< 3
// Temperature
const uint8_t kEcoclimTempMin = 5;  // Celsius
const uint8_t kEcoclimTempMax = kEcoclimTempMin + 31;  // Celsius

const uint8_t kEcoclimSensorTempMax = 30;  // Celsius
const uint8_t kEcoclimSensorTempIgnoreCode = 0b11111;  // 0x1F / 31 (DEC)

// Power: On, Mode: Auto, Temp: 11C, Sensor: 22C, Fan: Auto, Clock: 15:42
const uint64_t kEcoclimDefaultState = 0x110673AEFFFF72;

/// Native representation of a Ecoclim A/C message.
union EcoclimProtocol {
  uint64_t raw;  ///< The state in IR code form.
  struct {  // Only 56 bits (7 bytes are used.
    // Byte
    uint64_t            :3;  ///< Fixed 0b010
    uint64_t            :1;  ///< Unknown
    uint64_t DipConfig  :4;
    // Byte
    uint64_t OffMins    :3;  ///< Off Timer minutes (in tens of mins)
    uint64_t OffHours   :5;  ///< Off Timer nr of Hours
    // Byte
    uint64_t OnMins     :3;  ///< On Timer minutes (in tens of mins)
    uint64_t OnHours    :5;  ///< On Timer nr of Hours
    // Byte+Byte
    uint64_t Clock      :11;
    uint64_t            :1;  ///< Unknown
    uint64_t Fan        :2;  ///< Fan Speed
    uint64_t Power      :1;  ///< Power control
    uint64_t Clear      :1;  // Not sure what this is
    // Byte
    uint64_t Temp       :5;  ///< Desired Temperature (Celsius)
    uint64_t Mode       :3;  ///< Operating Mode
    // Byte
    uint64_t SensorTemp :5;  ///< Sensed Temperature (Celsius)
    uint64_t            :3;  ///< Fixed
  };
};

// Classes

/// Class for handling detailed EcoClim A/C 56 bit messages.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1397
class IREcoclimAc {
 public:
  explicit IREcoclimAc(const uint16_t pin, const bool inverted = false,
                       const bool use_modulation = true);
  void stateReset(void);
#if SEND_ECOCLIM
  void send(const uint16_t repeat = kNoRepeat);
  /// Run the calibration to calculate uSec timing offsets for this platform.
  /// @return The uSec timing offset needed per modulation of the IR Led.
  /// @note This will produce a 65ms IR signal pulse at 38kHz.
  ///   Only ever needs to be run once per object instantiation, if at all.
  int8_t calibrate(void) { return _irsend.calibrate(); }
#endif  // SEND_ECOCLIM
  void begin(void);
  void on(void);
  void off(void);
  void setPower(const bool on);
  bool getPower(void) const;
  void setTemp(const uint8_t celsius);
  uint8_t getTemp(void) const;
  void setSensorTemp(const uint8_t celsius);
  uint8_t getSensorTemp(void) const;
  void setFan(const uint8_t speed);
  uint8_t getFan(void) const;
  void setMode(const uint8_t mode);
  uint8_t getMode(void) const;
  void setClock(const uint16_t nr_of_mins);
  uint16_t getClock(void) const;
  uint64_t getRaw(void) const;
  void setRaw(const uint64_t new_code);
  static uint8_t convertMode(const stdAc::opmode_t mode);
  static uint8_t convertFan(const stdAc::fanspeed_t speed);
  static stdAc::opmode_t toCommonMode(const uint8_t mode);
  static stdAc::fanspeed_t toCommonFanSpeed(const uint8_t speed);
  stdAc::state_t toCommon(void) const;
  String toString(void) const;
#ifndef UNIT_TEST

 private:
  IRsend _irsend;  ///< Instance of the IR send class
#else  // UNIT_TEST
  /// @cond IGNORE
  IRsendTest _irsend;  ///< Instance of the testing IR send class
  /// @endcond
#endif  // UNIT_TEST
  EcoclimProtocol _;  ///< The state of the IR remote in IR code form.
};

#endif  // IR_ECOCLIM_H_
