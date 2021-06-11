// Copyright 2021 Davide Depau

/// @file
/// @brief Support for Kelan AC protocol.
/// Both sending and decoding should be functional for models of series KELON ON/OFF 9000-12000.
/// All features of the standard remote are implemented.
///
/// @note Unsupported:
///    - Explicit on/off due to AC unit limitations
///    - Explicit swing position due to AC unit limitations
///    - Fahrenheit.
// Supports:
//   Brand: Kelon,  Model: ON/OFF 9000-12000

#ifndef IR_KELON_H_
#define IR_KELON_H_

#ifdef UNIT_TEST
#include "IRsend_test.h"
#endif

#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "IRutils.h"

union KelonProtocol {
  uint64_t raw;

  struct {
    uint8_t preamble[2];
    uint8_t Fan: 2;
    uint8_t PowerToggle: 1;
    uint8_t SleepEnabled: 1;
    uint8_t DehumidifierGrade: 3;
    uint8_t SwingVToggle: 1;
    uint8_t Mode: 3;
    uint8_t TimerEnabled: 1;
    uint8_t Temperature: 4;
    uint8_t TimerHalfHour: 1;
    uint8_t TimerHours: 6;
    uint8_t SmartModeEnabled: 1;
    uint8_t pad1: 4;
    uint8_t SuperCoolEnabled1: 1;
    uint8_t pad2: 2;
    uint8_t SuperCoolEnabled2: 1;
  };
};

// Constants
const uint8_t kKelonModeHeat{0};
const uint8_t kKelonModeSmart{1};  // (temp = 26C, but not shown)
const uint8_t kKelonModeCool{2};
const uint8_t kKelonModeDry{3};    // (temp = 25C, but not shown)
const uint8_t kKelonModeFan{4};    // (temp = 25C, but not shown)
const uint8_t kKelonFanAuto{0};
// Note! Kelon fan speeds are actually 0:AUTO, 1:MAX, 2:MED, 3:MIN
// Since this is insane, I decided to invert them in the public API, they are converted back in setFan/getFan
const uint8_t kKelonFanMin{1};
const uint8_t kKelonFanMedium{2};
const uint8_t kKelonFanMax{3};

const int8_t kKelonDryGradeMax{2};
const int8_t kKelonDryGradeMin{-2};
const uint8_t kKelonMinTemp{18};
const uint8_t kKelonMaxTemp{32};


class IRKelonAC {
public:
  explicit IRKelonAC(uint16_t pin, bool inverted = false, bool use_modulation = true);

  void stateReset();

  #if SEND_KELON

  void send(uint16_t repeat = kNoRepeat);

  /// Run the calibration to calculate uSec timing offsets for this platform.
  /// @return The uSec timing offset needed per modulation of the IR Led.
  /// @note This will produce a 65ms IR signal pulse at 38kHz.
  ///   Only ever needs to be run once per object instantiation, if at all.
  int8_t calibrate() { return _irsend.calibrate(); }

  #endif


  void begin();

  void setTogglePower(bool toggle);

  bool getTogglePower() const;

  void setTemp(uint8_t degrees);

  uint8_t getTemp() const;

  void setFan(uint8_t speed);

  uint8_t getFan() const;

  void setDryGrade(int8_t grade);

  int8_t getDryGrade() const;

  void setMode(uint8_t mode);

  uint8_t getMode() const;

  void setToggleSwingVertical(bool toggle);

  bool getToggleSwingVertical() const;

  void setSleep(bool on);

  bool getSleep() const;

  void setSupercool(bool on);

  bool getSupercool() const;

  void setTimer(uint16_t mins);

  uint16_t getTimer() const;

  void setTimerEnabled(bool on);

  bool getTimerEnabled() const;

  uint64_t getRaw() const;

  void setRaw(const uint64_t new_code);

  static uint8_t convertMode(stdAc::opmode_t mode);

  static uint8_t convertFan(stdAc::fanspeed_t fan);

  static stdAc::opmode_t toCommonMode(uint8_t mode);

  static stdAc::fanspeed_t toCommonFanSpeed(uint8_t speed);

  stdAc::state_t toCommon() const;

  String toString() const;

private:
#ifndef UNIT_TEST
  IRsend _irsend;  ///< Instance of the IR send class
#else  // UNIT_TEST
  /// @cond IGNORE
  IRsendTest _irsend;  ///< Instance of the testing IR send class
  /// @endcond
#endif  // UNIT_TEST
  KelonProtocol _;

  // Used when exiting supercool mode
  uint8_t _previousMode{0};
  uint8_t _previousTemp{kKelonMinTemp};
  uint8_t _previousFan{kKelonFanAuto};
};

#endif // IR_KELON_H_
