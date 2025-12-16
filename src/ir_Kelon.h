// Copyright 2021 Davide Depau
// Copyright 2022 Leonardo Ascione (leonardfactory)

/// @file
/// @brief Support for Kelan AC protocol.
/// @note Both sending and decoding should be functional for models of series
/// KELON ON/OFF 9000-12000.
/// All features of the standard remote are implemented.
///
/// @note Unsupported:
///    - Explicit on/off due to AC unit limitations
///    - Explicit swing position due to AC unit limitations
///    - Fahrenheit.
///
/// For KELON168:
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1745
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1903
/// The specifics of reverse engineering the protocols details:
/// * DG11R2-01 by mp3-10
/// * RCH-R0Y3 by countrysideboy

// Supports:
//   Brand: Kelon,  Model: ON/OFF 9000-12000 (KELON)
//   Brand: Kelon,  Model: DG11R2-01 remote (KELON168)
//   Brand: Kelon,  Model: RCH-R0Y3 remote (KELON168)
//   Brand: Kelon,  Model: AST-09UW4RVETG00A A/C (KELON168)
//   Brand: Hisense,  Model: AST-09UW4RVETG00A A/C (KELON168)

#ifndef IR_KELON_H_
#define IR_KELON_H_

#ifdef UNIT_TEST
#include "IRsend_test.h"
#endif

#include <algorithm>
#include <cstring>
#ifndef ARDUINO
#include <string>
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
const uint8_t kKelonModeHeat = 0;
const uint8_t kKelonModeSmart = 1;  // (temp = 26C, but not shown)
const uint8_t kKelonModeCool = 2;
const uint8_t kKelonModeDry = 3;    // (temp = 25C, but not shown)
const uint8_t kKelonModeFan = 4;    // (temp = 25C, but not shown)
const uint8_t kKelonFanAuto = 0;
// Note! Kelon fan speeds are actually 0:AUTO, 1:MAX, 2:MED, 3:MIN
// Since this is insane, I decided to invert them in the public API, they are
// converted back in setFan/getFan
const uint8_t kKelonFanMin = 1;
const uint8_t kKelonFanMedium = 2;
const uint8_t kKelonFanMax = 3;

const int8_t kKelonDryGradeMin = -2;
const int8_t kKelonDryGradeMax = +2;
const uint8_t kKelonMinTemp = 18;
const uint8_t kKelonMaxTemp = 32;


class IRKelonAc {
 public:
  explicit IRKelonAc(uint16_t pin, bool inverted = false,
                     bool use_modulation = true);
  void stateReset(void);
  #if SEND_KELON
  void send(const uint16_t repeat = kNoRepeat);
  /// Run the calibration to calculate uSec timing offsets for this platform.
  /// @return The uSec timing offset needed per modulation of the IR Led.
  /// @note This will produce a 65ms IR signal pulse at 38kHz.
  ///   Only ever needs to be run once per object instantiation, if at all.
  int8_t calibrate(void) { return _irsend.calibrate(); }
  /// Since the AC does not support actually setting the power state to a known
  /// value, this utility allow ensuring the AC is on or off by exploiting
  /// the fact that the AC, according to the user manual, will always turn on
  /// when setting it to "smart" or "super" mode.
  void ensurePower(const bool on);
  #endif  // SEND_KELON


  void begin(void);
  void setTogglePower(const bool toggle);
  bool getTogglePower(void) const;
  void setTemp(const uint8_t degrees);
  uint8_t getTemp(void) const;
  void setFan(const uint8_t speed);
  uint8_t getFan(void) const;
  void setDryGrade(const int8_t grade);
  int8_t getDryGrade(void) const;
  void setMode(const uint8_t mode);
  uint8_t getMode(void) const;
  void setToggleSwingVertical(const bool toggle);
  bool getToggleSwingVertical(void) const;
  void setSleep(const bool on);
  bool getSleep(void) const;
  void setSupercool(const bool on);
  bool getSupercool(void) const;
  void setTimer(const uint16_t mins);
  uint16_t getTimer(void) const;
  void setTimerEnabled(const bool on);
  bool getTimerEnabled(void) const;
  uint64_t getRaw(void) const;
  void setRaw(const uint64_t new_code);
  static uint8_t convertMode(const stdAc::opmode_t mode);
  static uint8_t convertFan(const stdAc::fanspeed_t fan);
  static stdAc::opmode_t toCommonMode(const uint8_t mode);
  static stdAc::fanspeed_t toCommonFanSpeed(const uint8_t speed);
  stdAc::state_t toCommon(const stdAc::state_t *prev = nullptr) const;
  String toString(void) const;

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
  uint8_t _previousMode = 0;
  uint8_t _previousTemp = kKelonMinTemp;
  uint8_t _previousFan = kKelonFanAuto;
};


/// KELON168
/// --------

/// Native representation of a Kelon 168 bit A/C message.
union Kelon168Protocol {
  uint8_t raw[kKelon168StateLength];  ///< The state in native form
  struct {
    // Byte 0~1
    uint8_t preamble[2];  // Fixed to 0x83, 0x06
    // Byte 2
    uint8_t Fan     :2;
    uint8_t Power   :1;
    uint8_t Sleep   :1;
    uint8_t         :3;   // Not encoded / used, should be dry?
    uint8_t Swing1  :1;
    // Byte 3
    uint8_t Mode  :3;
    uint8_t       :1;
    uint8_t Temp  :4;
    // Byte 4
    uint8_t       :8;
    // Byte 5
    uint8_t         :4;
    uint8_t Super1  :1;
    uint8_t         :2;
    uint8_t Super2  :1;
    // Byte 6
    uint8_t ClockHours  :5;
    uint8_t LightOff    :1;
    uint8_t             :2;
    // Byte 7
    uint8_t ClockMins       :6;
    uint8_t                 :1;
    uint8_t OffTimerEnabled :1;
    // Byte 8
    uint8_t OffHours  :5;
    uint8_t           :1;
    uint8_t Swing2    :1;
    uint8_t           :1;
    // Byte 9
    uint8_t OffMins         :6;
    uint8_t                 :1;
    uint8_t OnTimerEnabled  :1;
    // Byte 10
    uint8_t OnHours :5;
    uint8_t         :3;
    // Byte 11
    uint8_t OnMins  :6;
    uint8_t         :2;
    // Byte 12
    uint8_t       :8;
    // Byte 13
    uint8_t Sum1  :8;
    // Byte 14
    uint8_t       :8;
    // Byte 15
    uint8_t Cmd   :8;
    // Byte 16
    uint8_t       :1;
    uint8_t Fan2  :1;
    uint8_t       :6;
    // Byte 17
    uint8_t pad1;
    // Byte 18 (Model1 & Model2 are some fixed bits.
    // On Whirlpool remotes they indicate the remote model)
    uint8_t Model1 :4;
    uint8_t On     :1;
    uint8_t Model2 :3;
    // Byte 19
    uint8_t       :8;
    // Byte 20
    uint8_t Sum2  :8;
  };
};

// Constants
const uint8_t kKelon168ModeHeat = 0;
const uint8_t kKelon168ModeSmart = 1;  // (temp = 26C, but not shown)
const uint8_t kKelon168ModeCool = 2;
const uint8_t kKelon168ModeDry = 3;    // (temp = 25C, but not shown)
const uint8_t kKelon168ModeFan = 4;    // (temp = 25C, but not shown)
const uint8_t kKelon168ChecksumByte1 = 13;
const uint8_t kKelon168ChecksumByte2 = kKelon168StateLength - 1;
const uint8_t kKelon168Heat = 0;
const uint8_t kKelon168Auto = 1;
const uint8_t kKelon168Cool = 2;
const uint8_t kKelon168Dry = 3;
const uint8_t kKelon168Fan = 4;
const uint8_t kKelon168FanAuto = 0;
const uint8_t kKelon168FanMin = 1;      // 0b001
const uint8_t kKelon168FanLow = 2;      // 0b010
const uint8_t kKelon168FanMedium = 3;   // 0b011
const uint8_t kKelon168FanHigh = 4;     // 0b100
const uint8_t kKelon168FanMax = 5;      // 0b101
const uint8_t kKelon168MinTemp = 16;     // 16C (DG11R2-01)
const uint8_t kKelon168MaxTemp = 32;     // 30C (DG11R2-01)
const uint8_t kKelon168AutoTemp = 23;    // 23C
const uint8_t kKelon168CommandLight = 0x00;
const uint8_t kKelon168CommandPower = 0x01;
const uint8_t kKelon168CommandTemp = 0x02;
const uint8_t kKelon168CommandSleep = 0x03;
const uint8_t kKelon168CommandSuper = 0x04;
const uint8_t kKelon168CommandOnTimer = 0x05;
const uint8_t kKelon168CommandMode = 0x06;
const uint8_t kKelon168CommandSwing = 0x07;
const uint8_t kKelon168CommandIFeel = 0x0D;
const uint8_t kKelon168CommandFanSpeed = 0x11;
// const uint8_t kKelon168CommandIFeel = 0x17; // ?
const uint8_t kKelon168CommandOffTimer = 0x1D;

class IRKelon168Ac {
 public:
  explicit IRKelon168Ac(uint16_t pin, bool inverted = false,
                     bool use_modulation = true);

  #if SEND_KELON168
  void send(const uint16_t repeat = kKelon168DefaultRepeat,
            const bool calcChecksum = true);
  /// Run the calibration to calculate uSec timing offsets for this platform.
  /// @return The uSec timing offset needed per modulation of the IR Led.
  /// @note This will produce a 65ms IR signal pulse at 38kHz.
  ///   Only ever needs to be run once per object instantiation, if at all.
  int8_t calibrate(void) { return _irsend.calibrate(); }
  #endif  // SEND_KELON168

  void begin(void);
  void stateReset(void);

  void setPower(const bool on);
  bool getPower(void) const;
  void setSleep(const bool on);
  bool getSleep(void) const;
  void setSuper(const bool on);
  bool getSuper(void) const;
  void setTemp(const uint8_t temp);
  uint8_t getTemp(void) const;
  void setFan(const uint8_t speed);
  uint8_t getFan(void) const;
  void setMode(const uint8_t mode);
  uint8_t getMode(void) const;
  void setSwing(const bool on);
  bool getSwing(void) const;
  void setLight(const bool on);
  bool getLight(void) const;
  uint16_t getClock(void) const;
  void setClock(const uint16_t minsPastMidight);
  uint16_t getOnTimer(void) const;
  void setOnTimer(const uint16_t minsPastMidight);
  void enableOnTimer(const bool on);
  bool isOnTimerEnabled(void) const;
  uint16_t getOffTimer(void) const;
  void setOffTimer(const uint16_t minsPastMidight);
  void enableOffTimer(const bool on);
  bool isOffTimerEnabled(void) const;
  void setCommand(const uint8_t code);
  uint8_t getCommand(void) const;
  kelon168_ac_remote_model_t getModel(void) const;
  void setModel(const kelon168_ac_remote_model_t model);
  uint8_t* getRaw(const bool calcChecksum = true);
  void setRaw(const uint8_t newCode[],
              const uint16_t length = kKelon168StateLength);
  static bool validChecksum(const uint8_t state[],
                            const uint16_t length = kKelon168StateLength);
  static uint8_t convertMode(const stdAc::opmode_t mode);
  static uint8_t convertFan(const stdAc::fanspeed_t speed);
  static stdAc::opmode_t toCommonMode(const uint8_t mode);
  static stdAc::fanspeed_t toCommonFanSpeed(const uint8_t speed);
  stdAc::state_t toCommon(const stdAc::state_t *prev = NULL) const;
  String toString(void) const;
#ifndef UNIT_TEST

 private:
  IRsend _irsend;  ///< Instance of the IR send class
#else  // UNIT_TEST
  /// @cond IGNORE
  IRsendTest _irsend;  ///< Instance of the testing IR send class
  /// @endcond
#endif  // UNIT_TEST
  Kelon168Protocol _;

  uint8_t _desiredtemp;  ///< The last user explicitly set temperature.
  void checksum(const uint16_t length = kWhirlpoolAcStateLength);
  void _setTemp(const uint8_t temp, const bool remember = true);
  void _setMode(const uint8_t mode);
  int8_t getTempOffset(void) const;
};

#endif  // IR_KELON_H_
