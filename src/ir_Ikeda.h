// Copyright 2022 Onepamopa

/// @file
/// @brief Support for Ikeda protocol.
// Supports:
//   Brand: Ikeda,  Model: ASU-09HRA A/C

#ifndef IR_IKEDA_H_
#define IR_IKEDA_H_

#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include "IRremoteESP8266.h"
#include "IRsend.h"
#ifdef UNIT_TEST
#include "IRsend_test.h"
#endif

// /// Native representation of a Ikeda A/C message.
// union IkedaProtocol{
//   uint64_t remote_state;  ///< The state in native IR code form
//   struct {
//     // Byte 0
//     uint8_t Checksum      :8;

//     // Byte 1
//     uint8_t Temperature   :8;		// DEC 16-30 C -> @ HEX

//     // Byte 2
//     uint8_t               :1; // Empty 0x0
//     uint8_t Fan           :2; // 3=Low, 2=Medium, 1=High, 0=Auto
//     uint8_t Flap          :1; // 0=Off, 1=On
//     uint8_t Fp            :1; // 0=Off, 1=On // FORCE RUN (Turbo?)
//     uint8_t FanOnly       :1; // 0=Off, 1=On (with mode Auto)
//     uint8_t Mode          :2; // 0b10001 (17) 0x11=Heat, 0b00 (0) 0x00=Auto(& Fan), 0b0001 (1) 0x01=Cool, 0b00010000 (16) 0x10=Dry

//     // Byte 3
//     uint8_t Sleep         :1; // 0=Off, 1=On
//     uint8_t OnT_Enable    :1; // 0=Off, 1=On
//     uint8_t OffT_Enable   :1; // 0=Off, 1=On
//     uint8_t Power         :1; // 0=Off, 1=On
//     uint8_t Timer         :4; // 1-12h
// 		// timer hours 1-9, 0xA=10, 0xB=11 0xC=12

//     // Byte 4
//     uint8_t               :8;  // Always 0xAA
//   };
// };


// /// Native representation of a Ikeda A/C message.
// union IkedaProtocol{
//   uint64_t remote_state;  ///< The state in native IR code form
//   struct {
//       // Byte 4
//     uint8_t               :8;  // Always 0xAA

//     // Byte 3
//     uint8_t Sleep         :1; // 0=Off, 1=On
//     uint8_t OnT_Enable    :1; // 0=Off, 1=On
//     uint8_t OffT_Enable   :1; // 0=Off, 1=On
//     uint8_t Power         :1; // 0=Off, 1=On
//     uint8_t Timer         :4; // 1-12h
// 		// timer hours 1-9, 0xA=10, 0xB=11 0xC=12

//     // Byte 2
//     uint8_t               :1; // Empty 0x0
//     uint8_t Fan           :2; // 3=Low, 2=Medium, 1=High, 0=Auto
//     uint8_t Flap          :1; // 0=Off, 1=On
//     uint8_t Fp            :1; // 0=Off, 1=On // FORCE RUN (Turbo?)
//     uint8_t FanOnly       :1; // 0=Off, 1=On (with mode Auto)
//     uint8_t Mode          :2; // 0b10001 (17) 0x11=Heat, 0b00 (0) 0x00=Auto(& Fan), 0b0001 (1) 0x01=Cool, 0b00010000 (16) 0x10=Dry

//     // Byte 1
//     uint8_t Temperature   :8;		// DEC 16-30 C -> @ HEX

//     // Byte 0
//     uint8_t Checksum      :8;
//   };
// };



// Works OK but the degrees are not correct
union IkedaProtocol{
  uint64_t remote_state;  ///< The state in native IR code form
  struct {
    // Byte 4
    uint8_t               :8;  // Always 0xAA

    // Byte 3
    uint8_t Timer         :4; // 1-12h
    uint8_t Power         :1; // 0=Off, 1=On
    uint8_t OffT_Enable   :1; // 0=Off, 1=On
    uint8_t OnT_Enable    :1; // 0=Off, 1=On
    uint8_t Sleep         :1; // 0=Off, 1=On
		// timer hours 1-9, 0xA=10, 0xB=11 0xC=12

    // Byte 2
    uint8_t Mode          :2; // 0b10001 (17) 0x11=Heat, 0b00 (0) 0x00=Auto(& Fan), 0b0001 (1) 0x01=Cool, 0b00010000 (16) 0x10=Dry
    uint8_t FanOnly       :1; // 0=Off, 1=On (with mode Auto)
    uint8_t Fp            :1; // 0=Off, 1=On // FORCE RUN (Turbo?)
    uint8_t Flap          :1; // 0=Off, 1=On
    uint8_t Fan           :2; // 3=Low, 2=Medium, 1=High, 0=Auto
    uint8_t               :1; // Empty 0x0

    // Byte 1
    //uint8_t               :3; // :2
    //uint8_t Temperature   :6;		// :5  DEC 16-30 C -> @ HEX
    //uint8_t               :2; // :3
    uint8_t Temperature   :8;

    // Byte 0
    uint8_t Checksum      :8;
  };
};

// Constants

// ------------------------------- Temperature
const uint8_t ikedaMinTemp 		= 16;
const uint8_t ikedaMaxTemp 		= 30;

// ------------------------------- Fan Speed & Flap
const uint8_t ikedaFanAuto 		  = 0;
const uint8_t ikedaFanHigh		  = 1;
const uint8_t ikedaFanMed 		  = 2;
const uint8_t ikedaFanLow 		  = 3;

// ------------------------------- Operating Mode
const uint8_t ikedaAuto 		  = 0;
const uint8_t ikedaCool 		  = 1;
const uint8_t ikedaDry			  = 2;
const uint8_t ikedaHeat 		  = 3;
const uint8_t ikedaFan			  = 4; // Auto 

// Class
/// Class for handling detailed Truma A/C messages.
class IRIkedaAc {
 public:
  explicit IRIkedaAc(const uint16_t pin, const bool inverted = false,
                     const bool use_modulation = true);
#if SEND_IKEDA
  void send(const uint16_t repeat = kIkedaMinRepeat);
  /// Run the calibration to calculate uSec timing offsets for this platform.
  /// @return The uSec timing offset needed per modulation of the IR Led.
  /// @note This will produce a 65ms IR signal pulse at 38kHz.
  ///   Only ever needs to be run once per object instantiation, if at all.
  int8_t calibrate(void) { return _irsend.calibrate(); }
#endif  // SEND_IKEDA
  void begin(void);
  void stateReset(void);

  void on(void);
  void off(void);
  void setPower(const bool on);
  bool getPower(void) const;

  void setTemp(const uint8_t celsius);
  uint8_t getTemp(void) const;

  void setFan(const uint8_t speed);
  uint8_t getFan(void) const;

  void setSwingV(const bool on);
  bool getSwingV(void) const;

  void setTurbo(const bool on);
  bool getTurbo(void) const;

  uint8_t getMode(void) const;
  void setMode(const uint8_t mode);

  void setSleep(const bool on);
  bool getSleep(void) const;

  void setQuiet(const bool on); // Mode: FAN?
  bool getQuiet(void) const; // Mode: FAN?

  void setRaw(const uint64_t newState);
  uint64_t getRaw(void);

  // static bool validChecksum(const uint64_t state);
  static uint8_t convertMode(const stdAc::opmode_t mode);
  static uint8_t convertFan(const stdAc::fanspeed_t speed);
  static stdAc::opmode_t toCommonMode(const uint8_t mode);
  static stdAc::fanspeed_t toCommonFanSpeed(const uint8_t speed);
  stdAc::state_t toCommon(const stdAc::state_t *prev = NULL);
  String toString(void) const;
  
#ifndef UNIT_TEST

 private:
  IRsend _irsend;  ///< Instance of the IR send class
#else  // UNIT_TEST
  /// @cond IGNORE
  IRsendTest _irsend;  ///< Instance of the testing IR send class
  /// @endcond
#endif  // UNIT_TEST
  IkedaProtocol _;
 
  uint8_t _lastfan;  // Last user chosen/valid fan speed.
  uint8_t _lastmode;  // Last user chosen operation mode.
  // static uint8_t calcChecksum(const uint64_t state);
  // void checksum(void);
};
#endif  // IR_TRUMA_H_
