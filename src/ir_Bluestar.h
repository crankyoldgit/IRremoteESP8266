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
  uint8_t getEffectiveTemp() const;  ///< Includes standalone Turbo override.
  void setFan(const uint8_t speed);
  uint8_t getFan() const;  ///< Native stored fan code; 0 for Convertible Turbo.
  uint8_t getEffectiveFan() const;  ///< Includes both Turbo mechanisms.
  void setSwingH(const bool on);
  bool getSwingH() const;
  void setSwingV(const bool on);
  bool getSwingV() const;
  void setLight(const bool on);
  bool getLight() const;
  void setSleep(const bool on);
  bool getSleep() const;
  /// Standalone toggle, independent of capacity.
  void setTurbo(const bool on);
  bool getTurbo() const;
  void setEcono(const bool on);
  bool getEcono() const;
  void setCapacity(const uint8_t capacity);
  uint8_t getCapacity() const;
  uint8_t getClockMinute() const;  ///< Only confirmed portion of clock data.
  static uint8_t convertMode(const stdAc::opmode_t mode);
  static uint8_t convertFan(const stdAc::fanspeed_t speed);
  static stdAc::opmode_t toCommonMode(const uint8_t mode);
  static stdAc::fanspeed_t toCommonFanSpeed(const uint8_t speed);
  stdAc::state_t toCommon() const;
  String toString() const;
#ifdef UNIT_TEST
  IRsendTest _irsend;
#else

 private:
  IRsend _irsend;
#endif

 private:
  uint8_t _state[kBluestarStateLength];
  uint8_t _normalFan;  ///< Fan selected before a locally set capacity override.
  void setField(const uint8_t byte, const uint8_t mask, const uint8_t value);
};

#endif  // IR_BLUESTAR_H_
