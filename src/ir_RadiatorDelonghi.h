// Copyright 2023 Thierry Houdoin

/// @file
/// @brief Delonghi Radiator

// Supports:
//   Brand: Delonghi

#ifndef IR_DELONGHI_RADIATOR_H_
#define IR_DELONGHI_RADIATOR_H_

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

/// Native representation of a Delonghi Heater single word. A full sequence is 10 word long
union DelonghiRadiatorProtocol{
  uint64_t raw;         ///< The state of the IR remote, one 64 bit word
  uint8_t data8[8];     ///< The same expressed as byte array 
  struct {
    // Byte 7, the byte order is reversed to cope with ESP little endianess
    uint8_t CheckSum:8;   //  CRC 8 (Maxim)
    // Byte 6
    uint8_t Second:8;    //  00 to 59 decimal
    // Byte 5
    uint8_t Minute :8;    //  00 to 59 decimal 
    // Byte 4
    uint8_t Hour:5;      //  00 to 23 decimal
    uint8_t Day:3;       //  0 : Monday .. 6 Sunday
    // Byte 3
    uint8_t Type:8;       //  00 for this structure
    // Byte 2 (Fake)
    uint8_t b2:8;
    // Byte 1 (Fake)
    uint8_t b1:8;
    // Byte 0 (Fake)
    uint8_t b0:8;
  } WordHour;            ///<  First word transmitted (Date/Hour)
  struct  {
      // Byte 7
      uint8_t CheckSum:8;             //  CRC 8 (Maxim)
      // Byte 6
      uint8_t TempReduced : 8;        //  Reduced temperature expressed in 1/10 of °C --> 14°c --> 140 decimal or 0x8c
      //  Byte 4-5
      uint16_t TempComfort : 10;      //  Comfort temperature expressed in 1/20 of °C --> 20°c --> 400 decimal or 0x190
      uint8_t Mode:6;                 // See the moes below
      // Byte 3
      uint8_t Type:8;                 //  01 for this structure
      // Byte 2 (Fake)
      uint8_t b2:8;
      // Byte 1 (Fake)
      uint8_t b1:8;
      // Byte 0 (Fake)
      uint8_t b0:8;
  } WordMode;
  struct {
      // Byte 7
      uint8_t CheckSum:8;             //  CRC 8 (Maxim)
      // Byte 4-6
      uint32_t SlotHour : 24;         //  Each bit set when temperature should be set to comfort (clock mode). First bit for 00H-01H slot
      // Byte 3
      uint8_t Type:8;                 //  0x08 (Monday) to 0x0E (Sunday)
      // Byte 2 (Fake)
      uint8_t b2:8;
      // Byte 1 (Fake)
      uint8_t b1:8;
      // Byte 0 (Fake)
      uint8_t b0:8;
  } WordDay;
  struct {
      // Byte 7
      uint8_t CheckSum:8;           //  CRC 8 (Maxim)
      // Byte 4-6
      uint32_t FanTime:3;           //  Fan time duration (1 means 15mn, 2 30mn), max is 60mn (4)
      uint32_t :21;                 //  Not used
      // Byte 3
      uint8_t Type:8;               //  02 for this word
      // Byte 2 (Fake)
      uint8_t b2:8;
      // Byte 1 (Fake)
      uint8_t b1:8;
      // Byte 0 (Fake)
      uint8_t b0:8;
  } LastWord;
};


// Constants
const uint8_t kDelonghiRadiatorTempMinC = 7;  // Deg C
const uint8_t kDelonghiRadiatorTempMaxC = 32;  // Deg C

#define MODE_FORCED       0
#define MODE_COMFORT      1
#define MODE_NIGHT        2
#define MODE_STANDBY      4
#define MODE_ANTIFREEZE   8  
#define MODE_PILOT        16
#define MODE_CHRONO       0x2F
const uint8_t kDelonghiRadiatorClockMode   = 0b101111;
const uint8_t kDelonghiRadiatorOffMode     = 0b000100;
const uint8_t kDelonghiRadiatorNightMode  = 0b000010;
const uint8_t kDelonghiRadiatorComfortMode = 0b000001;
const uint8_t kDelonghiRadiatorForcedMode  = 0b000000;
const uint8_t kDelonghiRadiatorPilotMode =  0b010000;
const uint8_t kDelonghiRadiatorAntoiFreezeMode =    0b001000;

const uint8_t kDelonghiRadiatorChecksumOffset = 32;

// Classes

/// Class for handling detailed Delonghi Heater messages.
class IRDelonghiRadiator {
 public:
  explicit IRDelonghiRadiator(const uint16_t pin, const bool inverted = false, const bool use_modulation = true);
  void stateReset(void);
#if SEND_DELONGHI_RADIATOR
  void FullSequenceSend();
  /// Run the calibration to calculate uSec timing offsets for this platform.
  /// @return The uSec timing offset needed per modulation of the IR Led.
  /// @note This will produce a 65ms IR signal pulse at 38kHz.
  ///   Only ever needs to be run once per object instantiation, if at all.
  int8_t calibrate(void) { return _irsend.calibrate(); }
#endif  // SEND_DELONGHI_RADIATOR
  void begin(void);
  static uint8_t calcChecksum(const uint64_t state);
  static bool validChecksum(const uint64_t state);

  void setTemp(const float degrees_confort, const float degrees_reduced, const bool force=false);
  float getTempComfort(void) const;
  float getTempReduced(void) const;

  void setFanDuration(uint8_t duration);
  uint8_t getFanDuration(void) const;

  uint64_t getRaw(int Index);

  void StoreSequence(const uint64_t state);
  bool IsTransmissionComplete();

  void setDateTime(const uint8_t Day, const uint8_t Hour, const uint8_t Minute, const uint8_t Second);
  void getDateTime(uint8_t &Day, uint8_t &Hour, uint8_t &Minute, uint8_t &Second);

  void setSlotDay(const uint8_t Day, const uint32_t SlotVal);
  uint32_t getSlotDay(const uint8_t Day) const;

  void setMode(const uint8_t mode);
  uint8_t getMode(void) const;
  char *SingleWordtoString(uint64_t State);
  String FullSequencetoString();

#ifndef UNIT_TEST

 private:
  IRsend _irsend;  ///< instance of the IR send class
#else
  /// @cond IGNORE
  IRsendTest _irsend;  ///< instance of the testing IR send class
  /// @endcond
#endif
  DelonghiRadiatorProtocol RecWord;

  DelonghiRadiatorProtocol FullSequence[10];      //  Full transmission sequence (ten words)
  uint16_t ReceiveSequence;
  void checksum(int Index);
  const char *Mode2Str(const uint8_t  Mode);
  char *OutputSlot(uint32_t Val);
};
#endif  // IR_DELONGHI_RADIATOR_H_
