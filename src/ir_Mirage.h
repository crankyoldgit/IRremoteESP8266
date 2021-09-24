// Copyright 2020-2021 David Conran (crankyoldgit)
/// @file
/// @brief Support for Mirage protocol
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1289
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1573


// Supports:
//   Brand: Mirage,  Model: VLU series A/C
//   Brand: Maxell,  Model: MX-CH18CF A/C
//   Brand: Maxell,  Model: KKG9A-C1 remote

#ifndef IR_MIRAGE_H_
#define IR_MIRAGE_H_

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

/// Native representation of a Mirage 120-bit A/C message.
/// @see https://docs.google.com/spreadsheets/d/1Ucu9mOOIIJoWQjUJq_VCvwgV3EwKaRk8K2AuZgccYEk/edit#gid=0
union Mirage120Protocol{
  uint8_t raw[kMirageStateLength];  ///< The state in code form.
  struct {
    // Byte 0
    uint8_t         :8;  // Header. (0x56)
    // Byte 1
    uint8_t Temp    :8;  // Celsius minus 0x5C.
    // Byte 2
    uint8_t         :8;  // Unknown / Unused. Typically 0x00
    // Byte 3
    uint8_t         :3;  // Unknown / Unused. Typically 0x0
    uint8_t Light   :1;  // Aka. Display. Seems linked to Sleep mode.
    uint8_t         :4;  // Unknown / Unused. Typically 0x0
    // Byte 4
    uint8_t Fan     :2;  // Fan Speed.
    uint8_t         :2;  // Unknown / Unused. Typically 0x0
    uint8_t Mode    :4;  // Cool, Heat, Dry, Fan, Recycle
    // Byte 5
    uint8_t Swing   :8;
    // Byte 6
    uint8_t         :7;  // Unknown / Unused. Typically 0x00
    uint8_t Sleep   :1;  // Sleep mode on or off.
    // Byte 7
    uint8_t         :3;  // Unknown / Unused. Typically 0x0
    uint8_t Turbo   :1;  // Sleep mode on or off. Only works in Cool mode.
    uint8_t         :4;  // Unknown / Unused. Typically 0x0
    // Byte 8
    uint8_t         :8;  // Unknown / Unused. Typically 0xC0
    // Byte 9
    uint8_t         :8;  // Unknown / Unused. Typically 0x00
    // Byte 10
    uint8_t         :8;  // Unknown / Unused.
    // Byte 11
    uint8_t Seconds :8;  // Nr. of Seconds in BCD.
    // Byte 12
    uint8_t Minutes :8;  // Nr. of Minutes in BCD.
    // Byte 13
    uint8_t Hours   :8;  // Nr. of Hours in BCD.
    // Byte 14
    uint8_t Sum     :8;  // Sum of all the previous nibbles.
  };
};

// Constants
const uint8_t kMirageAcHeat =    0b001;  // 1
const uint8_t kMirageAcCool =    0b010;  // 2
const uint8_t kMirageAcDry =     0b011;  // 3
const uint8_t kMirageAcRecycle = 0b100;  // 4
const uint8_t kMirageAcFan =     0b101;  // 5

const uint8_t kMirageAcFanAuto = 0b00;  // 0
const uint8_t kMirageAcFanHigh = 0b01;  // 1
const uint8_t kMirageAcFanMed =  0b10;  // 2
const uint8_t kMirageAcFanLow =  0b11;  // 3

const uint8_t kMirageAcMinTemp = 16;  // 16C
const uint8_t kMirageAcMaxTemp = 32;  // 32C
const uint8_t kMirageAcTempOffset = 0x5C;

/// Class for handling detailed Mirage 120-bit A/C messages.
/// @note Inspired and derived from the work done at: https://github.com/r45635/HVAC-IR-Control
/// @warning Consider this very alpha code. Seems to work, but not validated.
class IRMirageAc {
 public:
  explicit IRMirageAc(const uint16_t pin, const bool inverted = false,
                      const bool use_modulation = true);
  void stateReset(void);
  static bool validChecksum(const uint8_t* data);
#if SEND_MIRAGE
  void send(const uint16_t repeat = kMirageMinRepeat);
  /// Run the calibration to calculate uSec timing offsets for this platform.
  /// @return The uSec timing offset needed per modulation of the IR Led.
  /// @note This will produce a 65ms IR signal pulse at 38kHz.
  ///   Only ever needs to be run once per object instantiation, if at all.
  int8_t calibrate(void) { return _irsend.calibrate(); }
#endif  // SEND_MIRAGE
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
  uint8_t* getRaw(void);
  void setRaw(const uint8_t* data);
  uint32_t getClock(void) const;
  void setClock(const uint32_t nr_of_seconds);
  void setTurbo(const bool on);
  bool getTurbo(void) const;
  void setLight(const bool on);
  bool getLight(void) const;
  void setSleep(const bool on);
  bool getSleep(void) const;

  static uint8_t convertMode(const stdAc::opmode_t mode);
  static uint8_t convertFan(const stdAc::fanspeed_t speed);
  static uint8_t convertSwingV(const stdAc::swingv_t position);
  static uint8_t convertSwingH(const stdAc::swingh_t position);
  static stdAc::opmode_t toCommonMode(const uint8_t mode);
  static stdAc::fanspeed_t toCommonFanSpeed(const uint8_t speed);
  static stdAc::swingv_t toCommonSwingV(const uint8_t pos);
  static stdAc::swingh_t toCommonSwingH(const uint8_t pos);
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
  Mirage120Protocol _;
  void checksum(void);
  static uint8_t calculateChecksum(const uint8_t* data);
};
#endif  // IR_MIRAGE_H_
