// Copyright 2017 Ville Skyttä (scop)
// Copyright 2017, 2018 David Conran

/// @file
/// @brief Support for Green A/C protocols.
/// @see https://github.com/ToniA/arduino-heatpumpir/blob/master/GreenHeatpumpIR.h

#include "ir_Green.h"
#include <algorithm>
#include <cstring>
#ifndef ARDUINO
#include <string>
#endif
#include "IRrecv.h"
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "IRtext.h"
#include "IRutils.h"
//#include "ir_Kelvinator.h"

// Constants
const uint16_t kGreenHdrMark = 3860;
const uint16_t kGreenHdrSpace = 1440;  ///< See #684 & real example in unit tests
const uint16_t kGreenBitMark = 540;  //570
const uint16_t kGreenOneSpace = 1170;
const uint16_t kGreenZeroSpace = 460;
const uint16_t kGreenMsgSpace = 19000;
const uint8_t kGreenBlockFooter = 0b010;
const uint8_t kGreenBlockFooterBits = 3;

using irutils::addBoolToString;
using irutils::addIntToString;
using irutils::addLabeledString;
using irutils::addModeToString;
using irutils::addModelToString;
using irutils::addFanToString;
using irutils::addTempToString;
using irutils::minsToString;
using irutils::setBit;
using irutils::setBits;

#if SEND_GREEN
/// Send a Green Heat Pump formatted message.
/// Status: STABLE / Working.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void IRsend::sendGreen(const uint8_t data[], const uint16_t nbytes,
                      const uint16_t repeat) {
  if (nbytes < kGreenStateLength)
    return;  // Not enough bytes to send a proper message.

  for (uint16_t r = 0; r <= repeat; r++) {

        sendGeneric(kGreenHdrMark, kGreenHdrSpace, 
//                kGreenBitMark, 
				600, kGreenOneSpace, kGreenBitMark, kGreenZeroSpace,
                kGreenBitMark, kGreenMsgSpace, data , nbytes , 38, false, 0,
                25);
    /*
    // Block #1
    sendGeneric(kGreenHdrMark, kGreenHdrSpace, kGreenBitMark, kGreenOneSpace,
                kGreenBitMark, kGreenZeroSpace, 0, 0,  // No Footer.
                data, 4, 37, false, 0, 50);
				
    // Footer #1
    sendGeneric(0, 0,  // No Header
                kGreenBitMark, kGreenOneSpace, kGreenBitMark, kGreenZeroSpace,
                kGreenBitMark, kGreenMsgSpace, 0b010, 3, 38, false, 0, 50);

    // Block #2
    sendGeneric(0, 0,  // No Header for Block #2
                kGreenBitMark, kGreenOneSpace, kGreenBitMark, kGreenZeroSpace,
                kGreenBitMark, kGreenMsgSpace, data + 4, nbytes - 4, 38, false, 0,
                50);
    */           
  }
}

/// Send a Green Heat Pump formatted message.
/// Status: STABLE / Working.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void IRsend::sendGreen(const uint64_t data, const uint16_t nbits,
                      const uint16_t repeat) {
  if (nbits != kGreenBits)
    return;  // Wrong nr. of bits to send a proper message.
  // Set IR carrier frequency
  enableIROut(38);

  for (uint16_t r = 0; r <= repeat; r++) {
    // Header
    mark(kGreenHdrMark);
    space(kGreenHdrSpace);

    // Data
    for (int16_t i = 8; i <= nbits; i += 8) {
      sendData(kGreenBitMark, kGreenOneSpace, kGreenBitMark, kGreenZeroSpace,
               (data >> (nbits - i)) & 0xFF, 8, false);
      if (i == nbits / 2) {
        // Send the mid-message Footer.
        sendData(kGreenBitMark, kGreenOneSpace, kGreenBitMark, kGreenZeroSpace,
                 0b010, 3);
        mark(kGreenBitMark);
        space(kGreenMsgSpace);
      }
    }
    // Footer
    mark(kGreenBitMark);
    space(kGreenMsgSpace);
  }
}
#endif  // SEND_GREEN

/// Class constructor
/// @param[in] pin GPIO to be used when sending.
/// @param[in] model The enum of the model to be emulated.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRGreenAC::IRGreenAC(const uint16_t pin, const green_ac_remote_model_t model,
                   const bool inverted, const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) {
  stateReset();
  setModel(model);
}

/// Reset the internal state to a fixed known good state.
void IRGreenAC::stateReset(void) {
  // This resets to a known-good state to Power Off, Fan Auto, Mode Auto, 25C.
  for (uint8_t i = 0; i < kGreenStateLength; i++) remote_state[i] = 0x0;
  remote_state[0] = 0x23;
  remote_state[1] = 0xcb;
  remote_state[2] = 0x26;
  remote_state[3] = 0x01;
  remote_state[4] = 0x00;
  remote_state[5] = 0x24;
  remote_state[6] = 0x01;
  remote_state[7] = 0x06;
  remote_state[8] = 0x12;
  remote_state[9] = 0x00;
  remote_state[10] = 0x00;
  remote_state[11] = 0x01;
  remote_state[12] = 0x04;
 // remote_state[13] = 0x63;
}

/// Fix up the internal state so it is correct.
/// @note Internal use only.
void IRGreenAC::fixup(void) {
  setPower(getPower());  // Redo the power bits as they differ between models.
  checksum();  // Calculate the checksums
}

/// Set up hardware to be able to send a message.
void IRGreenAC::begin(void) { _irsend.begin(); }

#if SEND_GREEN
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRGreenAC::send(const uint16_t repeat) {
  fixup();  // Ensure correct settings before sending.
  _irsend.sendGreen(remote_state, kGreenStateLength, repeat);
}
#endif  // SEND_GREEN

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t* IRGreenAC::getRaw(void) {
  fixup();  // Ensure correct settings before sending.
  return remote_state;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
void IRGreenAC::setRaw(const uint8_t new_code[]) {
  memcpy(remote_state, new_code, kGreenStateLength);
  // We can only detect the difference between models when the power is on.
  if (getPower()) {
    if (GETBIT8(remote_state[5], kGreenPower2Offset))
      _model = green_ac_remote_model_t::YAW1FG;
    else
      _model = green_ac_remote_model_t::YBOFBG;
  }
}

/// Calculate and set the checksum values for the internal state.
/// @param[in] length The size/length of the state array to fix the checksum of.
void IRGreenAC::checksum(const uint16_t length) {

  uint8_t sum = 0;
  // Sum the lower half of the first 4 bytes of this block.
  for (uint8_t i = 0; i < length-1; i++)
    sum += remote_state[i];

  setBits(&remote_state[length - 1], kLowNibble, 8, sum);
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length of the state array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool IRGreenAC::validChecksum(const uint8_t state[], const uint16_t length) {
  // Top 4 bits of the last byte in the state is the state's checksum.
    uint8_t sum = 0;
  // Sum the lower half of the first 4 bytes of this block.
  for (uint8_t i = 0; i < length-1; i++)
    sum += state[i];
  return GETBITS8(state[length - 1], kLowNibble, 8) == sum;
}

/// Set the model of the A/C to emulate.
/// @param[in] model The enum of the appropriate model.
void IRGreenAC::setModel(const green_ac_remote_model_t model) {
  switch (model) {
    case green_ac_remote_model_t::YAW1FG:
    case green_ac_remote_model_t::YBOFBG: _model = model; break;
    default: setModel(green_ac_remote_model_t::YAW1FG);
  }
}

/// Get/Detect the model of the A/C.
/// @return The enum of the compatible model.
green_ac_remote_model_t IRGreenAC::getModel(void) { return _model; }

/// Change the power setting to On.
void IRGreenAC::on(void) { setPower(true); }

/// Change the power setting to Off.
void IRGreenAC::off(void) { setPower(false); }

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/814
void IRGreenAC::setPower(const bool on) {
  setBit(&remote_state[5], kGreenPower1Offset, on);
  // May not be needed. See #814
  //setBit(&remote_state[2], kGreenPower2Offset,
  //       on && _model != green_ac_remote_model_t::YBOFB);
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/814
bool IRGreenAC::getPower(void) {
  //  See #814. Not checking/requiring: (remote_state[2] & kGreenPower2Mask)
  return GETBIT8(remote_state[5], kGreenPower1Offset);
}

/// Set the default temperature units to use.
/// @param[in] on Use Fahrenheit as the units.
///   true is Fahrenheit, false is Celsius.
void IRGreenAC::setUseFahrenheit(const bool on) {
//  setBit(&remote_state[5], kGreenUseFahrenheitOffset, on);
;
}

/// Get the default temperature units in use.
/// @return true is Fahrenheit, false is Celsius.
bool IRGreenAC::getUseFahrenheit(void) {
  return GETBIT8(remote_state[5], kGreenUseFahrenheitOffset);
}

/// Set the temp. in degrees
/// @param[in] temp Desired temperature in Degrees.
/// @param[in] fahrenheit Use units of Fahrenheit and set that as units used.
///   false is Celsius (Default), true is Fahrenheit.
/// @note The unit actually works in Celsius with a special optional
///   "extra degree" when sending Fahrenheit.
void IRGreenAC::setTemp(const uint8_t temp, const bool fahrenheit) {
  float safecelsius = temp;
  if (fahrenheit)
    // Covert to F, and add a fudge factor to round to the expected degree.
    // Why 0.6 you ask?! Because it works. Ya'd thing 0.5 would be good for
    // rounding, but Noooooo!
    safecelsius = fahrenheitToCelsius(temp + 0.6);
  setUseFahrenheit(fahrenheit);  // Set the correct Temp units.

  // Make sure we have desired temp in the correct range.
  safecelsius = std::max(static_cast<float>(kGreenMinTempC), safecelsius);
  safecelsius = std::min(static_cast<float>(kGreenMaxTempC), safecelsius);
  // An operating mode of Auto locks the temp to a specific value. Do so.
  if (getMode() == kGreenAuto) safecelsius = 25;

  // Set the "main" Celsius degrees.
  setBits(&remote_state[7], kGreenTempOffset, kGreenTempSize,
           kGreenMaxTempC - safecelsius);
  // Deal with the extra degree fahrenheit difference.
 // setBit(&remote_state[3], kGreenTempExtraDegreeFOffset,
  //       (uint8_t)(safecelsius * 2) & 1);
}

/// Get the set temperature
/// @return The temperature in degrees in the current units (C/F) set.
uint8_t IRGreenAC::getTemp(void) {
  uint8_t deg = kGreenMaxTempC - GETBITS8(remote_state[7], kGreenTempOffset,
                                         kGreenTempSize);
  if (getUseFahrenheit()) {
    deg = celsiusToFahrenheit(deg);
    // Retrieve the "extra" fahrenheit from elsewhere in the code.
    ///if (GETBIT8(remote_state[3], kGreenTempExtraDegreeFOffset)) deg++;
    //deg = std::max(deg, kGreenMinTempF);  // Cover the fact that 61F is < 16C
  }
  return deg;
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting. 0 is auto, 1-3 is the speed.
void IRGreenAC::setFan(const uint8_t speed) {
  uint8_t fan = std::min((uint8_t)kGreenFanMax, speed);  // Bounds check
  if (getMode() == kGreenDry) fan = kGreenFanMin;  // DRY mode is always locked to fan 1.
  // Set the basic fan values.
  switch (speed) {
  case 1: fan = kGreenFanMin; break;
  case 2: fan = kGreenFanMed; break;
  case 3: fan = kGreenFanMax; break;
  }
  setBits(&remote_state[8], kGreenFanOffset, kGreenFanSize, fan);
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t IRGreenAC::getFan(void) {
  return GETBITS8(remote_state[8], kGreenFanOffset, kGreenFanSize);
}

/// Set the operating mode of the A/C.
/// @param[in] new_mode The desired operating mode.
void IRGreenAC::setMode(const uint8_t new_mode) {
  uint8_t mode = new_mode;
  switch (mode) {
    // AUTO is locked to 25C
    case kGreenAuto: setTemp(25); break;
    // DRY always sets the fan to 1.
    case kGreenDry: setFan(1); break;
    case kGreenCool:
    case kGreenFan:
    case kGreenHeat: break;
    // If we get an unexpected mode, default to AUTO.
    default: mode = kGreenAuto;
  }
  setBits(&remote_state[6], kLowNibble, kModeBitsSize, mode);
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t IRGreenAC::getMode(void) {
  return GETBITS8(remote_state[6], kLowNibble, kModeBitsSize);
}

/// Set the Light (LED) setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRGreenAC::setLight(const bool on) {
  setBit(&remote_state[7], kGreenLightOffset, on);
}

/// Get the Light (LED) setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRGreenAC::getLight(void) {
  return GETBIT8(remote_state[7], kGreenLightOffset);
}
/*
/// Set the IFeel setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRGreenAC::setIFeel(const bool on) {
  setBit(&remote_state[5], kGreenIFeelOffset, on);
}

/// Get the IFeel setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRGreenAC::getIFeel(void) {
  return GETBIT8(remote_state[5], kGreenIFeelOffset);
}

/// Set the Wifi (enabled) setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRGreenAC::setWiFi(const bool on) {
  setBit(&remote_state[5], kGreenWiFiOffset, on);
}

/// Get the Wifi (enabled) setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRGreenAC::getWiFi(void) {
  return GETBIT8(remote_state[5], kGreenWiFiOffset);
}
*/
/// Set the XFan (Mould) setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRGreenAC::setXFan(const bool on) {
//  setBit(&remote_state[2], kGreenXfanOffset, on);
}

/// Get the XFan (Mould) setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRGreenAC::getXFan(void) {
  return GETBIT8(remote_state[2], kGreenXfanOffset);
}

/// Set the Sleep setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRGreenAC::setSleep(const bool on) {
  setBit(&remote_state[9], kGreenSleepOffset, on);
}

/// Get the Sleep setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRGreenAC::getSleep(void) {
  return GETBIT8(remote_state[9], kGreenSleepOffset);
}

/// Set the Turbo setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRGreenAC::setTurbo(const bool on) {
  setBit(&remote_state[9], kGreenTurboOffset, on);
}

/// Get the Turbo setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRGreenAC::getTurbo(void) {
  return GETBIT8(remote_state[9], kGreenTurboOffset);
}

/// Set the Vertical Swing mode of the A/C.
/// @param[in] automatic Do we use the automatic setting?
/// @param[in] position The position/mode to set the vanes to.
void IRGreenAC::setSwingVertical(const bool automatic, const uint8_t position) {
  //setBit(&remote_state[8], kGreenSwingAutoOffset, automatic);
  uint8_t new_position = position;
  if (!automatic) {
    switch (position) {
      case kGreenSwingUp:
      case kGreenSwingMiddleUp:
      case kGreenSwingMiddle:
      case kGreenSwingMiddleDown:
      case kGreenSwingDown:
        break;
      default:
        new_position = kGreenSwingLastPos;
    }
  } else {
    switch (position) {
      case kGreenSwingAuto:
    // case kGreenSwingDownAuto:
    //  case kGreenSwingMiddleAuto:
    //  case kGreenSwingUpAuto:
        break;
      default:
        new_position = kGreenSwingAuto;
    }
  }
  setBits(&remote_state[8], kGreenSwingOffset, kGreenSwingSize, new_position);
}

/// Get the Vertical Swing Automatic mode setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRGreenAC::getSwingVerticalAuto(void) {
  return GETBIT8(remote_state[8], kGreenSwingAutoOffset);
}

/// Get the Vertical Swing position setting of the A/C.
/// @return The native position/mode.
uint8_t IRGreenAC::getSwingVerticalPosition(void) {
  return GETBITS8(remote_state[8], kGreenSwingOffset, kGreenSwingSize);
}

/// Set the timer enable setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRGreenAC::setTimerEnabled(const bool on) {
  setBit(&remote_state[9], kGreenTimerEnabledOffset, on);
}

/// Get the timer enabled setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRGreenAC::getTimerEnabled(void) {
  return GETBIT8(remote_state[9], kGreenTimerEnabledOffset);
}

/// Get the timer time value from the A/C.
/// @return The number of minutes the timer is set for.
uint16_t IRGreenAC::getTimer(void) {
  uint16_t hrs = irutils::bcdToUint8(
      (GETBITS8(remote_state[9], kGreenTimerTensHrOffset,
                kGreenTimerTensHrSize) << kNibbleSize) |
      GETBITS8(remote_state[9], kGreenTimerHoursOffset, kGreenTimerHoursSize));
  return hrs * 60 + (GETBIT8(remote_state[9], kGreenTimerHalfHrOffset) ? 30 : 0);
}

/// Set the A/C's timer to turn off in X many minutes.
/// @param[in] minutes The number of minutes the timer should be set for.
/// @note Stores time internally in 30 min units.
///  e.g. 5 mins means 0 (& Off), 95 mins is  90 mins (& On). Max is 24 hours.
void IRGreenAC::setTimer(const uint16_t minutes) {
  uint16_t mins = std::min(kGreenTimerMax, minutes);  // Bounds check.
  setTimerEnabled(mins >= 30);  // Timer is enabled when >= 30 mins.
  uint8_t hours = mins / 60;
  // Set the half hour bit.
  setBit(&remote_state[9], kGreenTimerHalfHrOffset, !((mins % 60) < 30));
  // Set the "tens" digit of hours.
  setBits(&remote_state[9], kGreenTimerTensHrOffset, kGreenTimerTensHrSize,
          hours / 10);
  // Set the "units" digit of hours.
  setBits(&remote_state[9], kGreenTimerHoursOffset, kGreenTimerHoursSize,
          hours % 10);
}

/// Set temperature display mode.
/// i.e. Internal, External temperature sensing.
/// @param[in] mode The desired temp source to display.
/// @note In order for the A/C unit properly accept these settings. You must
///   cycle (send) in the following order:
///   kGreenDisplayTempOff(0) -> kGreenDisplayTempSet(1) ->
///   kGreenDisplayTempInside(2) ->kGreenDisplayTempOutside(3) ->
///   kGreenDisplayTempOff(0).
///   The unit will no behave correctly if the changes of this setting are sent
///   out of order.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1118#issuecomment-628242152
void IRGreenAC::setDisplayTempSource(const uint8_t mode) {
  setBits(&remote_state[9], kGreenDisplayTempOffset, kGreenDisplayTempSize, mode);
}

/// Get the temperature display mode.
/// i.e. Internal, External temperature sensing.
/// @return The current temp source being displayed.
uint8_t IRGreenAC::getDisplayTempSource(void) {
  return GETBITS8(remote_state[9], kGreenDisplayTempOffset,
                  kGreenDisplayTempSize);
}

/// Convert a stdAc::opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivilant of the enum.
uint8_t IRGreenAC::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kCool: return kGreenCool;
    case stdAc::opmode_t::kHeat: return kGreenHeat;
    case stdAc::opmode_t::kDry:  return kGreenDry;
    case stdAc::opmode_t::kFan:  return kGreenFan;
    default:                     return kGreenAuto;
  }
}

/// Convert a stdAc::fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivilant of the enum.
uint8_t IRGreenAC::convertFan(const stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin:    return kGreenFanMin;
    case stdAc::fanspeed_t::kLow:
    case stdAc::fanspeed_t::kMedium: return kGreenFanMax - 1;
    case stdAc::fanspeed_t::kHigh:
    case stdAc::fanspeed_t::kMax:    return kGreenFanMax;
    default:                         return kGreenFanAuto;
  }
}

/// Convert a stdAc::swingv_t enum into it's native setting.
/// @param[in] swingv The enum to be converted.
/// @return The native equivilant of the enum.
uint8_t IRGreenAC::convertSwingV(const stdAc::swingv_t swingv) {
  switch (swingv) {
    case stdAc::swingv_t::kHighest: return kGreenSwingUp;
    case stdAc::swingv_t::kHigh:    return kGreenSwingMiddleUp;
    case stdAc::swingv_t::kMiddle:  return kGreenSwingMiddle;
    case stdAc::swingv_t::kLow:     return kGreenSwingMiddleDown;
    case stdAc::swingv_t::kLowest:  return kGreenSwingDown;
    default:                        return kGreenSwingAuto;
  }
}

/// Convert a native mode into its stdAc equivilant.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivilant of the native setting.
stdAc::opmode_t IRGreenAC::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kGreenCool: return stdAc::opmode_t::kCool;
    case kGreenHeat: return stdAc::opmode_t::kHeat;
    case kGreenDry: return stdAc::opmode_t::kDry;
    case kGreenFan: return stdAc::opmode_t::kFan;
    default: return stdAc::opmode_t::kAuto;
  }
}

/// Convert a native fan speed into its stdAc equivilant.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivilant of the native setting.
stdAc::fanspeed_t IRGreenAC::toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kGreenFanMax: return stdAc::fanspeed_t::kMax;
    case kGreenFanMax - 1: return stdAc::fanspeed_t::kMedium;
    case kGreenFanMin: return stdAc::fanspeed_t::kMin;
    default: return stdAc::fanspeed_t::kAuto;
  }
}

/// Convert a stdAc::swingv_t enum into it's native setting.
/// @param[in] pos The enum to be converted.
/// @return The native equivilant of the enum.
stdAc::swingv_t IRGreenAC::toCommonSwingV(const uint8_t pos) {
  switch (pos) {
    case kGreenSwingUp: return stdAc::swingv_t::kHighest;
    case kGreenSwingMiddleUp: return stdAc::swingv_t::kHigh;
    case kGreenSwingMiddle: return stdAc::swingv_t::kMiddle;
    case kGreenSwingMiddleDown: return stdAc::swingv_t::kLow;
    case kGreenSwingDown: return stdAc::swingv_t::kLowest;
    default: return stdAc::swingv_t::kAuto;
  }
}

/// Convert the current internal state into its stdAc::state_t equivilant.
/// @return The stdAc equivilant of the native settings.
stdAc::state_t IRGreenAC::toCommon(void) {
  stdAc::state_t result;
  result.protocol = decode_type_t::GREEN;
  result.model = this->getModel();
  result.power = this->getPower();
  result.mode = this->toCommonMode(this->getMode());
  result.celsius = !this->getUseFahrenheit();
  result.degrees = this->getTemp();
  result.fanspeed = this->toCommonFanSpeed(this->getFan());
  if (this->getSwingVerticalAuto())
    result.swingv = stdAc::swingv_t::kAuto;
  else
    result.swingv = this->toCommonSwingV(this->getSwingVerticalPosition());
  result.turbo = this->getTurbo();
  result.light = this->getLight();
 // result.clean = this->getXFan();
  result.sleep = this->getSleep() ? 0 : -1;
  // Not supported.
  result.swingh = stdAc::swingh_t::kOff;
  result.quiet = false;
  result.econo = false;
  result.filter = false;
  result.beep = false;
  result.clock = -1;
  return result;
}

/// Convert the current internal state into a human readable string.
/// @return A human readable string.
String IRGreenAC::toString(void) {
  String result = "";
  result.reserve(220);  // Reserve some heap for the string to reduce fragging.
  result += addModelToString(decode_type_t::GREEN, getModel(), false);
  result += addBoolToString(getPower(), kPowerStr);
  result += addModeToString(getMode(), kGreenAuto, kGreenCool, kGreenHeat,
                            kGreenDry, kGreenFan);
  result += addTempToString(getTemp(), !getUseFahrenheit());
  result += addFanToString(getFan(), kGreenFanMax, kGreenFanMin, kGreenFanAuto,
                           kGreenFanAuto, kGreenFanMed);
  result += addBoolToString(getTurbo(), kTurboStr);
  //result += addBoolToString(getIFeel(), kIFeelStr);
  //result += addBoolToString(getWiFi(), kWifiStr);
  //result += addBoolToString(getXFan(), kXFanStr);
  result += addBoolToString(getLight(), kLightStr);
  result += addBoolToString(getSleep(), kSleepStr);
  result += addLabeledString(getSwingVerticalAuto() ? kAutoStr : kManualStr,
                             kSwingVModeStr);
  result += addIntToString(getSwingVerticalPosition(), kSwingVStr);
  result += kSpaceLBraceStr;
  switch (getSwingVerticalPosition()) {
    case kGreenSwingLastPos:
      result += kLastStr;
      break;
    case kGreenSwingAuto:
      result += kAutoStr;
      break;
    default: result += kUnknownStr;
  }
  result += ')';
  result += addLabeledString(
      getTimerEnabled() ? minsToString(getTimer()) : kOffStr, kTimerStr);
  uint8_t src = getDisplayTempSource();
  result += addIntToString(src, kDisplayTempStr);
  result += kSpaceLBraceStr;
  switch (src) {
    case kGreenDisplayTempOff:
      result += kOffStr;
      break;
    case kGreenDisplayTempSet:
      result += kSetStr;
      break;
    case kGreenDisplayTempInside:
      result += kInsideStr;
      break;
    case kGreenDisplayTempOutside:
      result += kOutsideStr;
      break;
    default: result += kUnknownStr;
  }
  result += ')';
  return result;
}

#if DECODE_GREEN
/// Decode the supplied Green HVAC message.
/// Status: STABLE / Working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool IRrecv::decodeGreen(decode_results* results, uint16_t offset,
                        const uint16_t nbits, bool const strict) {
   DPRINT(" Green /n");
   DPRINT(results->rawlen);
   DPRINT(nbits);
  if (results->rawlen <=
      2 * (nbits + kGreenBlockFooterBits) + (kHeader + kFooter + 1) - 1 + offset)
    return false;  // Can't possibly be a valid Green message.
  if (strict && nbits != kGreenBits)
    return false;  // Not strictly a Green message.

  // There are two blocks back-to-back in a full Green IR message
  // sequence.

  uint16_t used;
  // Header + Data Block #1 (32 bits)
  used = matchGeneric(results->rawbuf + offset, results->state,
                      results->rawlen - offset, nbits / 2,
                      kGreenHdrMark, kGreenHdrSpace,
                      kGreenBitMark, kGreenOneSpace,
                      kGreenBitMark, kGreenZeroSpace,
                      0, 0, false,
                      _tolerance, kMarkExcess, false);
  if (used == 0) return false;
  offset += used;

  // Block #1 footer (3 bits, B010)
  match_result_t data_result;
  data_result = matchData(&(results->rawbuf[offset]), kGreenBlockFooterBits,
                          kGreenBitMark, kGreenOneSpace, kGreenBitMark,
                          kGreenZeroSpace, _tolerance, kMarkExcess, false);
  if (data_result.success == false) return false;
  if (data_result.data != kGreenBlockFooter) return false;
  offset += data_result.used;

  // Inter-block gap + Data Block #2 (32 bits) + Footer
  if (!matchGeneric(results->rawbuf + offset, results->state + 4,
                    results->rawlen - offset, nbits / 2,
                    kGreenBitMark, kGreenMsgSpace,
                    kGreenBitMark, kGreenOneSpace,
                    kGreenBitMark, kGreenZeroSpace,
                    kGreenBitMark, kGreenMsgSpace, true,
                    _tolerance, kMarkExcess, false)) return false;

  // Compliance
  if (strict) {
    // Verify the message's checksum is correct.
    if (!IRGreenAC::validChecksum(results->state)) return false;
  }

  // Success
  results->decode_type = GREEN;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // DECODE_GREEN
