// Copyright 2026
/// @file
/// @brief Support for Islandaire PTAC A/C protocols.
/// @note The framing & timings are the same as TEKNOPOINT, so this protocol
///   must be decoded before it. The 0x22 x 5 preamble tells them apart.

// Supports:
//   Brand: Islandaire,  Model: EZ12A1GSP1S10AL PTAC

#ifndef IR_ISLANDAIRE_H_
#define IR_ISLANDAIRE_H_

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

/// Native representation of an Islandaire A/C message.
union IslandaireProtocol {
  uint8_t raw[kIslandaireStateLength];  ///< The state in IR code form.
  struct {
    // Bytes 0-4: Preamble (0x22 x 5)
    uint8_t                :8;
    uint8_t                :8;
    uint8_t                :8;
    uint8_t                :8;
    uint8_t                :8;
    // Byte 5
    uint8_t                :2;
    uint8_t Power          :1;
    uint8_t TimerActive    :1;
    uint8_t                :4;  // Always 0b0010
    // Byte 6
    uint8_t Mode           :4;
    uint8_t                :4;
    // Byte 7
    uint8_t Temp           :8;  // Degrees Fahrenheit
    // Byte 8
    uint8_t Fan            :3;
    uint8_t                :3;
    uint8_t TimerActive2   :1;  // Set together with TimerActive.
    uint8_t                :1;
    // Byte 9
    uint8_t                :8;
    // Byte 10
    uint8_t TimerHours     :8;
    // Bytes 11-12
    uint8_t                :8;
    uint8_t                :8;
    // Byte 13
    uint8_t Sum            :8;
  };
};

// Constants
const uint8_t kIslandairePreamble = 0x22;
const uint8_t kIslandairePreambleLength = 5;

const uint8_t kIslandaireHeat = 0x1;
const uint8_t kIslandaireCool = 0x3;
const uint8_t kIslandaireFan =  0x4;

const uint8_t kIslandaireFanAuto = 0x0;
const uint8_t kIslandaireFanLow =  0x2;
const uint8_t kIslandaireFanHigh = 0x4;

const uint8_t kIslandaireMinTempF = 61;  // 61F
const uint8_t kIslandaireMaxTempF = 88;  // 88F

const uint8_t kIslandaireTimerMax = 12;  // Hours

// Classes
/// Class for handling detailed Islandaire A/C messages.
class IRIslandaireAc {
 public:
  explicit IRIslandaireAc(const uint16_t pin, const bool inverted = false,
                          const bool use_modulation = true);
  void stateReset(void);
#if SEND_ISLANDAIRE_AC
  void send(const uint16_t repeat = kNoRepeat);
  /// Run the calibration to calculate uSec timing offsets for this platform.
  /// @return The uSec timing offset needed per modulation of the IR Led.
  /// @note This will produce a 65ms IR signal pulse at 38kHz.
  ///   Only ever needs to be run once per object instantiation, if at all.
  int8_t calibrate(void) { return _irsend.calibrate(); }
#endif  // SEND_ISLANDAIRE_AC
  void begin(void);
  static uint8_t calcChecksum(const uint8_t state[],
                              const uint16_t length = kIslandaireStateLength);
  static bool validChecksum(const uint8_t state[],
                            const uint16_t length = kIslandaireStateLength);
  uint8_t* getRaw(void);
  void setRaw(const uint8_t new_code[],
              const uint16_t length = kIslandaireStateLength);
  void on(void);
  void off(void);
  void setPower(const bool on);
  bool getPower(void) const;
  void setMode(const uint8_t mode);
  uint8_t getMode(void) const;
  void setTemp(const float degrees, const bool fahrenheit = false);
  float getTemp(const bool fahrenheit = false) const;
  void setFan(const uint8_t speed);
  uint8_t getFan(void) const;
  void setTimer(const uint8_t hours);
  uint8_t getTimer(void) const;
  static uint8_t convertMode(const stdAc::opmode_t mode);
  static uint8_t convertFan(const stdAc::fanspeed_t speed);
  static stdAc::opmode_t toCommonMode(const uint8_t mode);
  static stdAc::fanspeed_t toCommonFanSpeed(const uint8_t speed);
  stdAc::state_t toCommon(void) const;
  String toString(void) const;
#ifndef UNIT_TEST

 private:
  IRsend _irsend;  ///< Instance of the IR send class
#else
  /// @cond IGNORE
  IRsendTest _irsend;  ///< Instance of the testing IR send class
  /// @endcond
#endif
  IslandaireProtocol _;
  void checksum(void);
};
#endif  // IR_ISLANDAIRE_H_
