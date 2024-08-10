// Copyright 2024 Andrey Kravchenko (stellalupus)
/// @file
/// @brief Support for the Electrolux EACM protocols.

// Supports:
//   Brand: Electrolux,  Model: Electrolux EACM EZ/N3

#ifndef IR_ELECTROLUX_AC_H_
#define IR_ELECTROLUX_AC_H_

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

union ElectroluxAcProtocol{
  uint64_t raw;  // The state of the IR remote in native IR code form.
  struct {
    uint8_t PowerToggle         :1;
    uint8_t Fan                 :2;
    uint8_t Temp                :5;
    uint8_t Mode                :3;
    uint8_t TimerEnable         :1;
    uint8_t Timer               :4;
    uint8_t Quiet               :1;
    uint8_t                     :1;
    uint8_t TempModeFahrenheit  :1;
    uint8_t                     :5;
    uint8_t                     :4;
    uint8_t Sum                 :4;
  };
};

// Constants
const uint8_t kElectroluxAcMinTemp = 16;    // 16C
const uint8_t kElectroluxAcMaxTemp = 32;    // 32C
const uint8_t kElectroluxAcMinFTemp = 60;   // 60F
const uint8_t kElectroluxAcMaxFTemp = 90;   // 90F
const uint8_t kElectroluxTimerMax = 12;     // 12H
const uint8_t kElectroluxTimerMin = 1;      // 1H
const uint64_t kElectroluxAcKnownGoodState = 0xF3008005;
const uint8_t kElectroluxAcChecksumOffset = 28;
const uint8_t kElectroluxAcChecksumSize = 4;

// Fan
const uint8_t kElectroluxFanLow = 2;     // 0b11
const uint8_t kElectroluxFanMedium = 1;  // 0b01
const uint8_t kElectroluxFanHigh = 0;    // 0b00
const uint8_t kElectroluxFanAuto = 3;    // 0b11

// Modes
const uint8_t kElectroluxModeCool = 0;  // 0b000
const uint8_t kElectroluxModeDry = 1;   // 0b001
const uint8_t kElectroluxModeFan = 2;   // 0b010
const uint8_t kElectroluxModeAuto = 4;  // 0b100


class IRElectroluxAc {
    public:
    explicit IRElectroluxAc(const uint16_t pin, const bool inverted = false,
                            const bool use_modulation = true);
    void stateReset();
#if SEND_ELECTROLUX_AC
    void send(const uint16_t repeat = kElectroluxAcDefaultRepeat);
    /// Run the calibration to calculate uSec timing offsets for this platform.
    /// @return The uSec timing offset needed per modulation of the IR Led.
    /// @note This will produce a 65ms IR signal pulse at 38kHz.
    ///   Only ever needs to be run once per object instantiation, if at all.
    int8_t calibrate(void) { return _irsend.calibrate(); }
#endif  // SEND_ELECTROLUX_AC
    void begin();
    void setPowerToggle(const bool on);
    bool getPowerToggle(void) const;
    void setTemp(const uint8_t temp);
    uint8_t getTemp(void) const;
    void setFan(const uint8_t speed);
    uint8_t getFan(void) const;
    void setMode(const uint8_t mode);
    uint8_t getMode(void) const;
    void setOnOffTimer(const uint16_t nr_of_mins);
    uint16_t getOnOffTimer(void) const;
    void setQuiet(const bool on);
    bool getQuiet(void) const;
    void setTempModeFahrenheit(const bool on);
    bool getTempModeFahrenheit(void) const;
    uint64_t getRaw(void);
    void setRaw(const uint64_t state);
    static uint8_t calcChecksum(const uint64_t state);
    static bool validChecksum(const uint64_t state);
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
    ElectroluxAcProtocol _;
    void checksum(void);
};

#endif  // IR_ELECTROLUX_AC_H_