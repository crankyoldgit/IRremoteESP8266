// Copyright 2017, 2019 David Conran

/// @file
/// @brief Support for LG protocols.
/// @see https://github.com/arendst/Tasmota/blob/54c2eb283a02e4287640a4595e506bc6eadbd7f2/sonoff/xdrv_05_irremote.ino#L327-438


// Supports:
//   Brand: LG,  Model: 6711A20083V remote (LG)
//   Brand: LG,  Model: AKB74395308 remote (LG2)
//   Brand: LG,  Model: S4-W12JA3AA A/C (LG2)
//   Brand: LG,  Model: AKB75215403 remote (LG2)
//   Brand: LG,  Model: AKB74955603 remote (LG2 - AKB74955603)
//   Brand: LG,  Model: A4UW30GFA2 A/C (LG2 - AKB74955603)
//   Brand: LG,  Model: AMNW09GSJA0 A/C (LG2 - AKB74955603)
//   Brand: General Electric,  Model: AG1BH09AW101 Split A/C (LG)
//   Brand: General Electric,  Model: 6711AR2853M A/C Remote (LG)

#ifndef IR_LG_H_
#define IR_LG_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "IRutils.h"
#ifdef UNIT_TEST
#include "IRsend_test.h"
#endif

/// Native representation of a LG A/C message.
union LGProtocol{
  uint32_t raw;  ///< The state of the IR remote in IR code form.
  struct {
    uint32_t Sum  :4;
    uint32_t Fan  :4;
    uint32_t Temp :4;
    uint32_t Mode :3;
    uint32_t      :3;
    uint32_t Power:2;
    uint32_t Sign :8;
  };
};

const uint8_t kLgAcFanLowest = 0;  // 0b0000
const uint8_t kLgAcFanLow = 1;     // 0b0001
const uint8_t kLgAcFanMedium = 2;  // 0b0010
const uint8_t kLgAcFanMax = 4;     // 0b0100
const uint8_t kLgAcFanAuto = 5;    // 0b0101
const uint8_t kLgAcFanLowAlt = 9;  // 0b1001
const uint8_t kLgAcFanHigh = 10;   // 0b1010
// Nr. of slots in the look-up table
const uint8_t kLgAcFanEntries = kLgAcFanHigh + 1;
const uint8_t kLgAcTempAdjust = 15;
const uint8_t kLgAcMinTemp = 16;  // Celsius
const uint8_t kLgAcMaxTemp = 30;  // Celsius
const uint8_t kLgAcCool = 0;  // 0b000
const uint8_t kLgAcDry = 1;   // 0b001
const uint8_t kLgAcFan = 2;   // 0b010
const uint8_t kLgAcAuto = 3;  // 0b011
const uint8_t kLgAcHeat = 4;  // 0b100
const uint8_t kLgAcPowerOff = 3;  // 0b11
const uint8_t kLgAcPowerOn = 0;   // 0b00
const uint8_t kLgAcSignature = 0x88;

const uint32_t kLgAcOffCommand = 0x88C0051;

// Classes
/// Class for handling detailed LG A/C messages.
class IRLgAc {
 public:
  explicit IRLgAc(const uint16_t pin, const bool inverted = false,
                  const bool use_modulation = true);
  void stateReset(void);
  static uint8_t calcChecksum(const uint32_t state);
  static bool validChecksum(const uint32_t state);
  bool isValidLgAc(void) const;
#if SEND_LG
  void send(const uint16_t repeat = kLgDefaultRepeat);
  /// Run the calibration to calculate uSec timing offsets for this platform.
  /// @return The uSec timing offset needed per modulation of the IR Led.
  /// @note This will produce a 65ms IR signal pulse at 38kHz.
  ///   Only ever needs to be run once per object instantiation, if at all.
  int8_t calibrate(void) { return _irsend.calibrate(); }
#endif  // SEND_LG
  void begin(void);
  void on(void);
  void off(void);
  void setPower(const bool on);
  bool getPower(void) const;
  void setTemp(const uint8_t degrees);
  uint8_t getTemp(void) const;
  void setFan(const uint8_t speed);
  uint8_t getFan(void) const;
  void setMode(const uint8_t mode);
  uint8_t getMode(void) const;
  uint32_t getRaw(void);
  void setRaw(const uint32_t new_code,
              const decode_type_t protocol = decode_type_t::UNKNOWN);
  static uint8_t convertMode(const stdAc::opmode_t mode);
  static stdAc::opmode_t toCommonMode(const uint8_t mode);
  static stdAc::fanspeed_t toCommonFanSpeed(const uint8_t speed);
  static uint8_t convertFan(const stdAc::fanspeed_t speed);
  stdAc::state_t toCommon(void) const;
  String toString(void) const;
  void setModel(const lg_ac_remote_model_t model);
  lg_ac_remote_model_t getModel(void) const;
#ifndef UNIT_TEST

 private:
  IRsend _irsend;  ///< Instance of the IR send class
#else  // UNIT_TEST
  /// @cond IGNORE
  IRsendTest _irsend;  ///< Instance of the testing IR send class
  /// @endcond
#endif  // UNIT_TEST
  LGProtocol _;
  uint8_t _temp;
  decode_type_t _protocol;  ///< Protocol version
  lg_ac_remote_model_t _model;  ///< Model type
  void checksum(void);
  void _setTemp(const uint8_t value);
  bool _isAKB74955603(void) const;
};

#endif  // IR_LG_H_
