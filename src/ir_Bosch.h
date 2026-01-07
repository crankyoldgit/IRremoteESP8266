// Copyright 2022 Nico Thien
/// @file
/// @brief Support for Bosch A/C protocol
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1787

// Supports:
//   Brand: Bosch,  Model: CL3000i-Set 26 E A/C
//   Brand: Bosch,  Model: RG10A(G2S)BGEF remote
//   Brand: Durastar, Model: RG10R(M2S)/BGEFU1 remote


#ifndef IR_BOSCH_H_
#define IR_BOSCH_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <algorithm>
#include <cstring>
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "IRrecv.h"
#include "IRtext.h"
#include "IRutils.h"
#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#ifdef UNIT_TEST
#include "IRsend_test.h"
#endif

// Constants
const uint16_t kBoschHdrMark = 4366;
const uint16_t kBoschBitMark = 456;
const uint16_t kBoschHdrSpace = 4415;
const uint16_t kBoschOneSpace = 1645;
const uint16_t kBoschZeroSpace = 610;
const uint16_t kBoschFooterSpace = 5235;
const uint16_t kBoschFreq = 38000;  // Hz. (Guessing the most common frequency.)
const uint16_t kBosch144NrOfSections = 3;
const uint16_t kBosch144BytesPerSection = 6;

using irutils::addBoolToString;
using irutils::addModeToString;
using irutils::addFanToString;
using irutils::addTempToString;
using std::min;
using std::max;
using std::memcpy;
using std::memcmp;

// Modes                Bit[0] to Section 3    Bit[1-2] to Section 1
//                                ModeS3                   ModeS1
const uint8_t kBosch144Cool = 0b000;
const uint8_t kBosch144Dry =  0b011;
const uint8_t kBosch144Auto = 0b101;
const uint8_t kBosch144Heat = 0b110;
const uint8_t kBosch144Fan =  0b010;

// Fan Control          Bit[0-5] to Section 3    Bit[6-8] to Section 1
//                                  FanS3                    FanS1
const uint16_t kBosch144Fan20 = 0b111001010;
const uint16_t kBosch144Fan40 = 0b100010100;
const uint16_t kBosch144Fan60 = 0b010011110;
const uint16_t kBosch144Fan80 = 0b001101000;
const uint16_t kBosch144Fan100 = 0b001110010;
const uint16_t kBosch144FanAuto = 0b101110011;
const uint16_t kBosch144FanAuto0 = 0b000110011;

// Temperature
const uint8_t kBosch144CelsiusMin = 16;
const uint8_t kBosch144CelsiusMax = 30;
const uint8_t kBosch144CelsiusMap[] = {
    // Bit[0] to Section 3: TempS4 (the "half-degree" bit)
    // Bit[1] to Section 3: TempS3
    // Bit[1-4] to Section 1: TempS1
    0b000010,  // 16C
    0b000000,  // 17C
    0b000100,  // 18C
    0b001100,  // 19C
    0b001000,  // 20C
    0b011000,  // 21C
    0b011100,  // 22C
    0b010100,  // 23C
    0b010000,  // 24C
    0b110000,  // 25C
    0b110100,  // 26C
    0b100100,  // 27C
    0b100000,  // 28C
    0b101000,  // 29C
    0b101100   // 30C
};

const uint8_t kBosch144FahrenheitMin = 60;
const uint8_t kBosch144FahrenheitMax = 86;
const uint8_t kBosch144FahrenheitMap[] = {
    // Bit[0] to Section 3: TempS4
    // Bit[1] to Section 3: TempS3
    // Bit[1-4] to Section 1: TempS1
    0b000010,  // 60F
    0b000011,  // 61F
    0b000000,  // 62F
    0b000001,  // 63F
    0b000100,  // 64F
    0b000101,  // 65F
    0b001100,  // 66F
    0b001101,  // 67F
    0b001000,  // 68F
    0b001001,  // 69F
    0b011000,  // 70F
    0b011001,  // 71F
    0b011100,  // 72F
    0b010100,  // 73F
    0b010101,  // 74F
    0b010000,  // 75F
    0b010001,  // 76F
    0b110000,  // 77F
    0b110001,  // 78F
    0b110100,  // 79F
    0b110101,  // 80F
    0b100100,  // 81F
    0b100000,  // 82F
    0b100001,  // 83F
    0b101000,  // 84F
    0b101001,  // 85F
    0b101100   // 86F
};

// "OFF" is a 96bit-message    the same as Coolix protocol
const uint8_t kBosch144Off[] = {0xB2, 0x4D, 0x7B, 0x84, 0xE0, 0x1F,
                                0xB2, 0x4D, 0x7B, 0x84, 0xE0, 0x1F};

// On, 25C, Mode: Auto
const uint8_t kBosch144DefaultState[kBosch144StateLength] = {
  0xB2, 0x4D, 0x1F, 0xE0, 0xC8, 0x37,
  0xB2, 0x4D, 0x1F, 0xE0, 0xC8, 0x37,
  0xD5, 0x65, 0x00, 0x00, 0x00, 0x3A};

union Bosch144Protocol {
  uint8_t raw[kBosch144StateLength];  ///< The state in IR code form.
  struct {
    uint8_t               :8;   // Fixed value 0b10110010 / 0xB2.   ############
    uint8_t InnvertS1_1   :8;   // Invert byte  0b01001101 / 0x4D   #
    uint8_t               :5;   // not used (without timer use)     #
    uint8_t FanS1         :3;   // Fan speed bits in Section 1      #
    uint8_t InnvertS1_2   :8;   // Invert byte                      # Section 1
    uint8_t               :2;   // not used (without timer use)     # =
    uint8_t ModeS1        :2;   // Operation mode bits S1           # Section 2
    uint8_t TempS1        :4;   // Desired temperature (Celsius) S2 #
    uint8_t InnvertS1_3   :8;   // Invert byte (without timer use)  ############

    uint8_t               :8;   // Fixed value 0b10110010 / 0xB2.   ############
    uint8_t InnvertS2_1   :8;   // Invert byte  0b01001101 / 0x4D   #
    uint8_t               :5;   // not used (without timer use)     #
    uint8_t FanS2         :3;   // Fan speed bits in Section 2      #
    uint8_t InnvertS2_2   :8;   // Invert byte                      # Section 2
    uint8_t               :2;   // not used (without timer use)     # =
    uint8_t ModeS2        :2;   // Operation mode bits S2           # Section 1
    uint8_t TempS2        :4;   // Desired temperature (Celsius) S2 #
    uint8_t InnvertS2_3   :8;   // Invert byte (without timer use)  ###########

    uint8_t               :8;   // Fixed value 0b11010101 / 0xD5    ###########
    uint8_t ModeS3        :1;   // ModeBit in Section 3             #
    uint8_t FanS3         :6;   // Fan speed bits in Section 3      #
    uint8_t               :1;   // Unknown                          #
    uint8_t               :5;   // Unknown                          #
    uint8_t TempS4        :1;   // Desired temp                     #
    uint8_t               :1;   // Unknown                          #
    uint8_t Quiet         :1;   // Silent-Mode                      # Section 3
    uint8_t UseFahrenheit :1;   // Fahrenheit or Celcius            #
    uint8_t               :3;   // Unknown                          #
    uint8_t TempS3        :1;   // Desired temp. Bit in Section3    #
    uint8_t               :3;   // Unknown                          #
    uint8_t               :8;   // Unknown                          #
    uint8_t ChecksumS3    :8;   // Checksum from byte 13-17         ###########
  };
};

// Classes

/// Class for handling detailed Bosch144 A/C messages.
class IRBosch144AC {
 public:
  explicit IRBosch144AC(const uint16_t pin, const bool inverted = false,
                      const bool use_modulation = true);
  void stateReset(void);
#if SEND_BOSCH144
  void send(const uint16_t repeat = 0);
  /// Run the calibration to calculate uSec timing offsets for this platform.
  /// @return The uSec timing offset needed per modulation of the IR Led.
  /// @note This will produce a 65ms IR signal pulse at 38kHz.
  ///   Only ever needs to be run once per object instantiation, if at all.
  int8_t calibrate(void) { return _irsend.calibrate(); }
#endif  // SEND_BOSCH144
  void begin();
  void setPower(const bool state);
  bool getPower(void) const;
  void setTemp(const uint8_t temp, const bool fahrenheit = false);
  uint8_t getTemp(void) const;
  void setUseFahrenheit(const bool on);
  bool getUseFahrenheit(void) const;
  void setFan(const uint16_t speed);
  uint16_t getFan(void) const;
  void setMode(const uint8_t mode);
  uint8_t getMode(void) const;
  void setQuiet(const bool on);
  bool getQuiet(void) const;
  uint8_t* getRaw(void);
  void setRaw(const uint8_t new_code[],
              const uint16_t length = kBosch144StateLength);
  static uint8_t convertMode(const stdAc::opmode_t mode);
  static uint16_t convertFan(const stdAc::fanspeed_t speed);
  static stdAc::opmode_t toCommonMode(const uint8_t mode);
  static stdAc::fanspeed_t toCommonFanSpeed(const uint16_t speed);
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
  Bosch144Protocol _;  ///< The state of the IR remote in IR code form.

  // Internal State settings
  bool powerFlag;

  void setInvertBytes();
  void setCheckSumS3();
  void setTempRaw(const uint8_t code);
  uint8_t getTempRaw(void) const;
};

#endif  // IR_BOSCH_H_
