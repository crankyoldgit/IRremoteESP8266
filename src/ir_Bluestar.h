// Copyright 2026 IRremoteESP8266 contributors
/// @file
/// @brief Blue Star IE318YKU A/C support. Distinct from BluestarHeavy.

// Supports:
//   Brand: Blue Star, Model: IE318YKU A/C

#ifndef IR_BLUESTAR_H_
#define IR_BLUESTAR_H_

#include <stdint.h>
#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include "IRremoteESP8266.h"
#include "IRsend.h"
#ifdef UNIT_TEST
#include "IRsend_test.h"
#endif

/// Native representation of a Blue Star A/C message.
/// @note Bytes 10-11 store the right-aligned 11-bit secondary command;
/// the upper five bits of byte 10 are not transmitted.
union BluestarProtocol {
  uint8_t raw[kBluestarStateLength];
  struct {
    // Byte 0
    uint8_t             :8;  // Signature: 0x33.
    // Byte 1
    uint8_t Mode        :4;
    uint8_t             :1;
    uint8_t Fan         :3;
    // Byte 2
    uint8_t SwingH      :1;  // 1 is On; confirmed by testing on the A/C.
    uint8_t SwingV      :1;  // 1 is On; confirmed by testing on the A/C.
    uint8_t             :1;
    uint8_t Light       :1;  // Inverted: 0 is On.
    uint8_t Turbo       :1;
    uint8_t             :1;
    uint8_t Sleep       :1;
    uint8_t Power       :1;
    // Byte 3
    uint8_t Temp        :5;
    uint8_t Econo       :1;
    uint8_t             :2;
    // Bytes 4-7: unresolved clock/timer fields. Byte 6 contains minutes.
    uint8_t             :8;
    uint8_t             :8;
    uint8_t             :8;
    uint8_t             :8;
    // Byte 8
    uint8_t Capacity    :3;
    uint8_t             :5;
    // Byte 9: complement of the sum of bytes 0-8.
    uint8_t Sum         :8;
    // Bytes 10-11: secondary command, most significant byte first.
    uint8_t CommandHigh :3;
    uint8_t             :5;
    uint8_t CommandLow  :8;
  };
};

const uint8_t kBluestarFanOnly = 1;
const uint8_t kBluestarAuto = 2;
const uint8_t kBluestarDry = 4;
const uint8_t kBluestarCool = 8;
const uint8_t kBluestarFanTurbo = 0;  ///< Convertible 110%, displayed as High.
const uint8_t kBluestarFanHigh = 1;
const uint8_t kBluestarFanMed = 2;
const uint8_t kBluestarFanLow = 4;
const uint8_t kBluestarFanAuto = 7;
const uint8_t kBluestarMinTemp = 16;
const uint8_t kBluestarMaxTemp = 30;
const uint8_t kBluestarFanModeTemp = 24;  ///< Observed hidden Fan-only value.
const uint8_t kBluestarCapacityTurbo = 1;  ///< 110%
const uint8_t kBluestarCapacity80 = 2;
const uint8_t kBluestarCapacity60 = 3;
const uint8_t kBluestarCapacity40 = 4;
const uint8_t kBluestarCapacityDefault = 5;  ///< 100%

// Observed secondary section values. A command can accompany different states.
const uint16_t kBluestarCommandLight = 0;
const uint16_t kBluestarCommandCapacity = 0;
const uint16_t kBluestarCommandMode = 0x5C0;
const uint16_t kBluestarCommandTemp = 0x5C4;
const uint16_t kBluestarCommandSwing = 0x5C8;
const uint16_t kBluestarCommandPower = 0x5CD;
const uint16_t kBluestarCommandEcono = 0x7C1;
const uint16_t kBluestarCommandTurbo = 0x7C9;
const uint16_t kBluestarCommandSleep = 0x7CC;
const uint16_t kBluestarCommandFan = 0x7D0;

/// An 80-bit A/C state and its separate 11-bit secondary command.
/// @note Clock/timer bytes and undocumented bits are preserved. Clock and
/// timer setters are deliberately absent because their encoding is unresolved.
/// @note Mode setters select the native mode; callers explicitly select the
/// new mode's temperature/fan. Per-mode remote history is not synthesized.
class IRBluestarAc {
 public:
  explicit IRBluestarAc(const uint16_t pin, const bool inverted = false,
                       const bool use_modulation = true);
  void begin();
  void stateReset();
#if SEND_BLUESTAR
  void send(const uint16_t repeat = kNoRepeat);
  int8_t calibrate() { return _irsend.calibrate(); }
#endif
  uint8_t *getRaw();
  bool setRaw(const uint8_t state[],
              const uint16_t length = kBluestarStateLength);
  static uint8_t calcChecksum(const uint8_t state[],
                             const uint16_t length = kBluestarMainStateLength);
  static bool validChecksum(const uint8_t state[],
                            const uint16_t length = kBluestarStateLength);
  uint16_t getCommand() const;
  void setCommand(const uint16_t command);
  void on();
  void off();
  void setPower(const bool on);
  bool getPower() const;
  void setMode(const uint8_t mode);
  uint8_t getMode() const;
  void setTemp(const uint8_t celsius);
  uint8_t getTemp() const;  ///< Stored temperature, including hidden Fan value.
  void setFan(const uint8_t speed);
  uint8_t getFan() const;  ///< Native stored fan code; 0 for Convertible Turbo.
  void setSwingH(const bool on);
  bool getSwingH() const;
  void setSwingV(const bool on);
  bool getSwingV() const;
  void setLight(const bool on);
  bool getLight() const;
  void setSleep(const bool on);
  bool getSleep() const;
  /// Standalone toggle, independent of capacity.
  /// @note The remote displays 16 C/High while On, but retains the stored
  /// temperature/fan values in the packet and restores them when Off.
  void setTurbo(const bool on);
  bool getTurbo() const;
  void setEcono(const bool on);
  bool getEcono() const;
  void setCapacity(const uint8_t capacity);
  uint8_t getCapacity() const;
  static uint8_t convertMode(const stdAc::opmode_t mode);
  static uint8_t convertFan(const stdAc::fanspeed_t speed);
  static stdAc::opmode_t toCommonMode(const uint8_t mode);
  static stdAc::fanspeed_t toCommonFanSpeed(const uint8_t speed);
  stdAc::state_t toCommon() const;
  String toString() const;
#ifdef UNIT_TEST
  /// @cond IGNORE
  IRsendTest _irsend;
  /// @endcond
#else

 private:
  IRsend _irsend;
#endif

 private:
  BluestarProtocol _;
  uint8_t _normalFan;  ///< Fan selected before a locally set capacity override.
};

#endif  // IR_BLUESTAR_H_
