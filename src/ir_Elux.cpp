// Copyright 2017 Ville Skyttä (scop)
// Copyright 2017, 2018 David Conran

/// @file
/// @brief Support for Elux A/C protocols.
/// @see https://github.com/ToniA/arduino-heatpumpir/blob/master/EluxHeatpumpIR.h

#include "ir_Elux.h"
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
const uint16_t kEluxHdrMark = 8860;
const uint16_t kEluxHdrSpace = 4440;  ///< See #684 & real example in unit tests
const uint16_t kEluxBitMark = 520;  //570
const uint16_t kEluxOneSpace = 1670;
const uint16_t kEluxZeroSpace = 560;
const uint16_t kEluxMsgSpace = 8200;
const uint8_t kEluxBlockFooter = 0b010;
const uint8_t kEluxBlockFooterBits = 3;

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

#if SEND_ELUX
/// Send a Elux Heat Pump formatted message.
/// Status: STABLE / Working.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void IRsend::sendElux(const uint8_t data[], const uint16_t nbytes,
                      const uint16_t repeat) {
  if (nbytes < kEluxStateLength)
    return;  // Not enough bytes to send a proper message.

  for (uint16_t r = 0; r <= repeat; r++) {
		
        sendGeneric(kEluxHdrMark, kEluxHdrSpace, kEluxBitMark, kEluxOneSpace,
                kEluxBitMark, kEluxZeroSpace, 0, 0,	// No Footer.
                data , 6, 38, false, 0,
                50);
//		space(kEluxZeroSpace);mark(50);
		sendGeneric(0, 0, kEluxBitMark, kEluxHdrMark, 
                kEluxBitMark, kEluxZeroSpace, 0, 0,	// No Footer.
                0b01 , 1, 38, true, 0,
                50);
		
/*    sendGeneric(kGreeHdrMark, kGreeHdrSpace, kGreeBitMark, kGreeOneSpace,
                kGreeBitMark, kGreeZeroSpace, 0, 0,  // No Footer.
                data, 4, 38, false, 0, 50);
*/				
    // Block #1
    sendGeneric(0, 0, kEluxBitMark, kEluxOneSpace, 
				kEluxBitMark, kEluxZeroSpace, 0, 0,
                data+6, nbytes-6, 38, false, 0, 50);
				
		sendGeneric(0, 0, kEluxBitMark, kEluxOneSpace, 
                kEluxBitMark, kEluxZeroSpace, 0, 0,	// No Footer.
                0b010 , 1, 38, false, 0,
                50);				
    /*				
    // Footer #1
    sendGeneric(0, 0,  // No Header
                kEluxBitMark, kEluxOneSpace, kEluxBitMark, kEluxZeroSpace,
                kEluxBitMark, kEluxMsgSpace, 0b010, 3, 38, false, 0, 50);

    // Block #2
    sendGeneric(0, 0,  // No Header for Block #2
                kEluxBitMark, kEluxOneSpace, kEluxBitMark, kEluxZeroSpace,
                kEluxBitMark, kEluxMsgSpace, data + 4, nbytes - 4, 38, false, 0,
                50);
    */           
  }
}

/// Send a Elux Heat Pump formatted message.
/// Status: STABLE / Working.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void IRsend::sendElux(const uint64_t data, const uint16_t nbits,
                      const uint16_t repeat) {
  if (nbits != kEluxBits)
    return;  // Wrong nr. of bits to send a proper message.
  // Set IR carrier frequency
  enableIROut(38);

  for (uint16_t r = 0; r <= repeat; r++) {
    // Header
    mark(kEluxHdrMark);
    space(kEluxHdrSpace);

    // Data
    for (int16_t i = 8; i <= nbits; i += 8) {
      sendData(kEluxBitMark, kEluxOneSpace, kEluxBitMark, kEluxZeroSpace,
               (data >> (nbits - i)) & 0xFF, 8, false);
      if (i == nbits / 2) {
        // Send the mid-message Footer.
        sendData(kEluxBitMark, kEluxOneSpace, kEluxBitMark, kEluxZeroSpace,
                 0b010, 3);
        mark(kEluxBitMark);
        space(kEluxMsgSpace);
      }
    }
    // Footer
    mark(kEluxBitMark);
    space(kEluxMsgSpace);
  }
}
#endif  // SEND_ELUX

/// Class constructor
/// @param[in] pin GPIO to be used when sending.
/// @param[in] model The enum of the model to be emulated.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IREluxAC::IREluxAC(const uint16_t pin, const elux_ac_remote_model_t model,
                   const bool inverted, const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) {
  stateReset();
  setModel(model);
}

/// Reset the internal state to a fixed known good state.
void IREluxAC::stateReset(void) {
  // This resets to a known-good state to Power Off, Fan Auto, Mode Auto, 25C.
  for (uint8_t i = 0; i < kEluxStateLength; i++) remote_state[i] = 0x0;
  remote_state[0] = 0x83;
  remote_state[1] = 0x06;
  remote_state[2] = 0x03;
  remote_state[3] = 0x60;
  remote_state[4] = 0x00;
  remote_state[5] = 0x00;
  remote_state[6] = 0x00;
  remote_state[7] = 0x00;
  remote_state[8] = 0x00;
  remote_state[9] = 0x00;
  remote_state[10] = 0x0C;
  remote_state[11] = 0x00;
  remote_state[12] = 0x00;
  remote_state[13] = 0x6B;
}

/// Fix up the internal state so it is correct.
/// @note Internal use only.
void IREluxAC::fixup(void) {
  setPower(getPower());  // Redo the power bits as they differ between models.
  checksum();  // Calculate the checksums
}

/// Set up hardware to be able to send a message.
void IREluxAC::begin(void) { _irsend.begin(); }

#if SEND_ELUX
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void IREluxAC::send(const uint16_t repeat) {
  fixup();  // Ensure correct settings before sending.
  _irsend.sendElux(remote_state, kEluxStateLength, repeat);
}
#endif  // SEND_ELUX

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t* IREluxAC::getRaw(void) {
  fixup();  // Ensure correct settings before sending.
  return remote_state;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
void IREluxAC::setRaw(const uint8_t new_code[]) {
  memcpy(remote_state, new_code, kEluxStateLength);
  // We can only detect the difference between models when the power is on.
  if (getPower()) {
    if (GETBIT8(remote_state[5], kEluxPower2Offset))
      _model = elux_ac_remote_model_t::EAW1F;
    else
      _model = elux_ac_remote_model_t::EBOFB;
  }
}

/// Calculate and set the checksum values for the internal state.
/// @param[in] length The size/length of the state array to fix the checksum of.
void IREluxAC::checksum(const uint16_t length) {

  uint8_t sum = 0;
  // Sum the lower half of the first 4 bytes of this block.
  for (uint8_t i = 2; i < length-1; i++)
    sum ^= remote_state[i];

  remote_state[length - 1]= sum;
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length of the state array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool IREluxAC::validChecksum(const uint8_t state[], const uint16_t length) {
  // Top 4 bits of the last byte in the state is the state's checksum.
    uint8_t sum = 0;
  // Sum the lower half of the first 4 bytes of this block.
  for (uint8_t i = 0; i < length-1; i++)
    sum += state[i];
  return GETBITS8(state[length - 1], kLowNibble, 8) == sum;
}

/// Set the model of the A/C to emulate.
/// @param[in] model The enum of the appropriate model.
void IREluxAC::setModel(const elux_ac_remote_model_t model) {
  switch (model) {
    case elux_ac_remote_model_t::EAW1F:
    case elux_ac_remote_model_t::EBOFB: _model = model; break;
    default: setModel(elux_ac_remote_model_t::EAW1F);
  }
}

/// Get/Detect the model of the A/C.
/// @return The enum of the compatible model.
elux_ac_remote_model_t IREluxAC::getModel(void) { return _model; }

/// Change the power setting to On.
void IREluxAC::on(void) { setPower(true); }

/// Change the power setting to Off.
void IREluxAC::off(void) { setPower(false); }

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/814
void IREluxAC::setPower(const bool on) {
  setBit(&remote_state[2], kEluxPower1Offset, on);
  // May not be needed. See #814
  //setBit(&remote_state[2], kEluxPower2Offset,
  //       on && _model != elux_ac_remote_model_t::YBOFB);
//    setBit(&remote_state[2], 3, on);
//	setBit(&remote_state[2], 4, on);
//    setBit(&remote_state[2], 5, on);
//	setBit(&remote_state[2], 6, on);	
//	setBit(&remote_state[2], 7, on);
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/814
bool IREluxAC::getPower(void) {
  //  See #814. Not checking/requiring: (remote_state[2] & kEluxPower2Mask)
  return GETBIT8(remote_state[2], kEluxPower1Offset);
}

/// Set the default temperature units to use.
/// @param[in] on Use Fahrenheit as the units.
///   true is Fahrenheit, false is Celsius.
void IREluxAC::setUseFahrenheit(const bool on) {
//  setBit(&remote_state[5], kEluxUseFahrenheitOffset, on);
;
}

/// Get the default temperature units in use.
/// @return true is Fahrenheit, false is Celsius.
bool IREluxAC::getUseFahrenheit(void) {
  return GETBIT8(remote_state[5], kEluxUseFahrenheitOffset);
}

/// Set the temp. in degrees
/// @param[in] temp Desired temperature in Deeluxs.
/// @param[in] fahrenheit Use units of Fahrenheit and set that as units used.
///   false is Celsius (Default), true is Fahrenheit.
/// @note The unit actually works in Celsius with a special optional
///   "extra deelux" when sending Fahrenheit.
void IREluxAC::setTemp(const uint8_t temp, const bool fahrenheit) {
  float safecelsius = temp;
  if (fahrenheit)
    // Covert to F, and add a fudge factor to round to the expected deelux.
    // Why 0.6 you ask?! Because it works. Ya'd thing 0.5 would be good for
    // rounding, but Noooooo!
    safecelsius = fahrenheitToCelsius(temp + 0.6);
  setUseFahrenheit(fahrenheit);  // Set the correct Temp units.

  // Make sure we have desired temp in the correct range.
  safecelsius = std::max(static_cast<float>(kEluxMinTempC), safecelsius);
  safecelsius = std::min(static_cast<float>(kEluxMaxTempC), safecelsius);
  // An operating mode of Auto locks the temp to a specific value. Do so.
  if (getMode() == kEluxAuto) safecelsius = 22;

  // Set the "main" Celsius degrees.
  setBits(&remote_state[3], kEluxTempOffset, kEluxTempSize,
            safecelsius - kEluxMinTempC);
  // Deal with the extra degree fahrenheit difference.
 // setBit(&remote_state[3], kEluxTempExtraDeeluxFOffset,
  //       (uint8_t)(safecelsius * 2) & 1);
}

/// Get the set temperature
/// @return The temperature in degrees in the current units (C/F) set.
uint8_t IREluxAC::getTemp(void) {
  uint8_t deg = kEluxMaxTempC - GETBITS8(remote_state[3], kEluxTempOffset,
                                         kEluxTempSize);
  if (getUseFahrenheit()) {
    deg = celsiusToFahrenheit(deg);
    // Retrieve the "extra" fahrenheit from elsewhere in the code.
    ///if (GETBIT8(remote_state[3], kEluxTempExtraDeeluxFOffset)) deg++;
    //deg = std::max(deg, kEluxMinTempF);  // Cover the fact that 61F is < 16C
  }
  return deg;
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting. 0 is auto, 1-3 is the speed.
void IREluxAC::setFan(const uint8_t speed) {
  uint8_t fan = std::min((uint8_t)kEluxFanMax, speed);  // Bounds check
  if (getMode() == kEluxDry) fan = kEluxFanMin;  // DRY mode is always locked to fan 1.
  // Set the basic fan values.
  switch (speed) {
  case 0: fan = kEluxFanAuto; break;
  case 3: fan = kEluxFanMin; break;
  case 2: fan = kEluxFanMed; break;
  case 1: fan = kEluxFanMax; break;
  }
  setBits(&remote_state[2], kEluxFanOffset, kEluxFanSize, fan);
}


/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t IREluxAC::getFan(void) {
  return GETBITS8(remote_state[2], kEluxFanOffset, kEluxFanSize);
}

/// Set the operating mode of the A/C.
/// @param[in] new_mode The desired operating mode.
void IREluxAC::setMode(const uint8_t new_mode) {
  uint8_t mode = new_mode;
  switch (mode) {
    // AUTO is locked to 22C
    case kEluxAuto: setTemp(22); break;
    // DRY always sets the fan to 1.
    case kEluxDry: setFan(1); break;
    case kEluxCool:
    case kEluxFan:
    case kEluxHeat: break;
    // If we get an unexpected mode, default to AUTO.
    default: mode = kEluxAuto;
  }
  setBits(&remote_state[3], kLowNibble, kModeBitsSize, mode);
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t IREluxAC::getMode(void) {
  return GETBITS8(remote_state[3], kLowNibble, kModeBitsSize);
}

/// Set the Light (LED) setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IREluxAC::setLight(const bool on) {
  setBit(&remote_state[4], kEluxLightOffset, on);
}

/// Get the Light (LED) setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IREluxAC::getLight(void) {
  return GETBIT8(remote_state[4], kEluxLightOffset);
}
/*
/// Set the IFeel setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IREluxAC::setIFeel(const bool on) {
  setBit(&remote_state[5], kEluxIFeelOffset, on);
}

/// Get the IFeel setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IREluxAC::getIFeel(void) {
  return GETBIT8(remote_state[5], kEluxIFeelOffset);
}

/// Set the Wifi (enabled) setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IREluxAC::setWiFi(const bool on) {
  setBit(&remote_state[5], kEluxWiFiOffset, on);
}

/// Get the Wifi (enabled) setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IREluxAC::getWiFi(void) {
  return GETBIT8(remote_state[5], kEluxWiFiOffset);
}
*/
/// Set the XFan (Mould) setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IREluxAC::setXFan(const bool on) {
//  setBit(&remote_state[2], kEluxXfanOffset, on);
}

/// Get the XFan (Mould) setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IREluxAC::getXFan(void) {
  return GETBIT8(remote_state[2], kEluxXfanOffset);
}

/// Set the Sleep setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IREluxAC::setSleep(const bool on) {
  setBit(&remote_state[4], kEluxSleepOffset, on);
}

/// Get the Sleep setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IREluxAC::getSleep(void) {
  return GETBIT8(remote_state[4], kEluxSleepOffset);
}

/// Set the Turbo setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IREluxAC::setTurbo(const bool on) {
  setBit(&remote_state[4], kEluxTurboOffset, on);
}

/// Get the Turbo setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IREluxAC::getTurbo(void) {
  return GETBIT8(remote_state[4], kEluxTurboOffset);
}

/// Set the Vertical Swing mode of the A/C.
/// @param[in] automatic Do we use the automatic setting?
/// @param[in] position The position/mode to set the vanes to.
void IREluxAC::setSwingVertical(const bool automatic, const uint8_t position) {
  //setBit(&remote_state[8], kEluxSwingAutoOffset, automatic);
  uint8_t new_position = position;
  if (!automatic) {
    switch (position) {
      case kEluxSwingUp:
      case kEluxSwingMiddleUp:
      case kEluxSwingMiddle:
      case kEluxSwingMiddleDown:
      case kEluxSwingDown:
        break;
      default:
        new_position = kEluxSwingLastPos;
    }
  } else {
    switch (position) {
      case kEluxSwingAuto:
    // case kEluxSwingDownAuto:
    //  case kEluxSwingMiddleAuto:
    //  case kEluxSwingUpAuto:
        break;
      default:
        new_position = kEluxSwingAuto;
    }
  }
  setBits(&remote_state[4], kEluxSwingOffset, kEluxSwingSize, new_position);
}

/// Get the Vertical Swing Automatic mode setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IREluxAC::getSwingVerticalAuto(void) {
  return GETBIT8(remote_state[4], kEluxSwingAutoOffset);
}

/// Get the Vertical Swing position setting of the A/C.
/// @return The native position/mode.
uint8_t IREluxAC::getSwingVerticalPosition(void) {
  return GETBITS8(remote_state[4], kEluxSwingOffset, kEluxSwingSize);
}

/// Set the timer enable setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IREluxAC::setTimerEnabled(const bool on) {
  setBit(&remote_state[4], kEluxTimerEnabledOffset, on);
}

/// Get the timer enabled setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IREluxAC::getTimerEnabled(void) {
  return GETBIT8(remote_state[4], kEluxTimerEnabledOffset);
}

/// Get the timer time value from the A/C.
/// @return The number of minutes the timer is set for.
uint16_t IREluxAC::getTimer(void) {
  uint16_t hrs = irutils::bcdToUint8(
      (GETBITS8(remote_state[4], kEluxTimerTensHrOffset,
                kEluxTimerTensHrSize) << kNibbleSize) |
      GETBITS8(remote_state[4], kEluxTimerHoursOffset, kEluxTimerHoursSize));
  return hrs * 60 + (GETBIT8(remote_state[4], kEluxTimerHalfHrOffset) ? 30 : 0);
}

/// Set the A/C's timer to turn off in X many minutes.
/// @param[in] minutes The number of minutes the timer should be set for.
/// @note Stores time internally in 30 min units.
///  e.g. 5 mins means 0 (& Off), 95 mins is  90 mins (& On). Max is 24 hours.
void IREluxAC::setTimer(const uint16_t minutes) {
  uint16_t mins = std::min(kEluxTimerMax, minutes);  // Bounds check.
  setTimerEnabled(mins >= 30);  // Timer is enabled when >= 30 mins.
  uint8_t hours = mins / 60;
  // Set the half hour bit.
  setBit(&remote_state[4], kEluxTimerHalfHrOffset, !((mins % 60) < 30));
  // Set the "tens" digit of hours.
  setBits(&remote_state[4], kEluxTimerTensHrOffset, kEluxTimerTensHrSize,
          hours / 10);
  // Set the "units" digit of hours.
  setBits(&remote_state[4], kEluxTimerHoursOffset, kEluxTimerHoursSize,
          hours % 10);
}

/// Set temperature display mode.
/// i.e. Internal, External temperature sensing.
/// @param[in] mode The desired temp source to display.
/// @note In order for the A/C unit properly accept these settings. You must
///   cycle (send) in the following order:
///   kEluxDisplayTempOff(0) -> kEluxDisplayTempSet(1) ->
///   kEluxDisplayTempInside(2) ->kEluxDisplayTempOutside(3) ->
///   kEluxDisplayTempOff(0).
///   The unit will no behave correctly if the changes of this setting are sent
///   out of order.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1118#issuecomment-628242152
void IREluxAC::setDisplayTempSource(const uint8_t mode) {
  setBits(&remote_state[4], kEluxDisplayTempOffset, kEluxDisplayTempSize, mode);
}

/// Get the temperature display mode.
/// i.e. Internal, External temperature sensing.
/// @return The current temp source being displayed.
uint8_t IREluxAC::getDisplayTempSource(void) {
  return GETBITS8(remote_state[4], kEluxDisplayTempOffset,
                  kEluxDisplayTempSize);
}

/// Convert a stdAc::opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivilant of the enum.
uint8_t IREluxAC::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kCool: return kEluxCool;
    case stdAc::opmode_t::kHeat: return kEluxHeat;
    case stdAc::opmode_t::kDry:  return kEluxDry;
    case stdAc::opmode_t::kFan:  return kEluxFan;
    default:                     return kEluxAuto;
  }
}

/// Convert a stdAc::fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivilant of the enum.
uint8_t IREluxAC::convertFan(const stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin:
    case stdAc::fanspeed_t::kLow:    return kEluxFanMin;
    case stdAc::fanspeed_t::kMedium: return kEluxFanMed;
    case stdAc::fanspeed_t::kHigh:
    case stdAc::fanspeed_t::kMax:    return kEluxFanMax;
    default:                         return kEluxFanAuto;
  }
}

/// Convert a stdAc::swingv_t enum into it's native setting.
/// @param[in] swingv The enum to be converted.
/// @return The native equivilant of the enum.
uint8_t IREluxAC::convertSwingV(const stdAc::swingv_t swingv) {
  switch (swingv) {
    case stdAc::swingv_t::kHighest: return kEluxSwingUp;
    case stdAc::swingv_t::kHigh:    return kEluxSwingMiddleUp;
    case stdAc::swingv_t::kMiddle:  return kEluxSwingMiddle;
    case stdAc::swingv_t::kLow:     return kEluxSwingMiddleDown;
    case stdAc::swingv_t::kLowest:  return kEluxSwingDown;
    default:                        return kEluxSwingAuto;
  }
}

/// Convert a native mode into its stdAc equivilant.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivilant of the native setting.
stdAc::opmode_t IREluxAC::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kEluxCool: return stdAc::opmode_t::kCool;
    case kEluxHeat: return stdAc::opmode_t::kHeat;
    case kEluxDry: return stdAc::opmode_t::kDry;
    case kEluxFan: return stdAc::opmode_t::kFan;
    default: return stdAc::opmode_t::kAuto;
  }
}

/// Convert a native fan speed into its stdAc equivilant.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivilant of the native setting.
stdAc::fanspeed_t IREluxAC::toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kEluxFanMax: return stdAc::fanspeed_t::kMax;
    case kEluxFanMin - 1: return stdAc::fanspeed_t::kMedium;
    case kEluxFanMin: return stdAc::fanspeed_t::kMin;
    default: return stdAc::fanspeed_t::kAuto;
  }
}

/// Convert a stdAc::swingv_t enum into it's native setting.
/// @param[in] pos The enum to be converted.
/// @return The native equivilant of the enum.
stdAc::swingv_t IREluxAC::toCommonSwingV(const uint8_t pos) {
  switch (pos) {
    case kEluxSwingUp: return stdAc::swingv_t::kHighest;
    case kEluxSwingMiddleUp: return stdAc::swingv_t::kHigh;
    case kEluxSwingMiddle: return stdAc::swingv_t::kMiddle;
    case kEluxSwingMiddleDown: return stdAc::swingv_t::kLow;
    case kEluxSwingDown: return stdAc::swingv_t::kLowest;
    default: return stdAc::swingv_t::kAuto;
  }
}

/// Convert the current internal state into its stdAc::state_t equivilant.
/// @return The stdAc equivilant of the native settings.
stdAc::state_t IREluxAC::toCommon(void) {
  stdAc::state_t result;
  result.protocol = decode_type_t::ELUX;
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
String IREluxAC::toString(void) {
  String result = "";
  result.reserve(220);  // Reserve some heap for the string to reduce fragging.
  result += addModelToString(decode_type_t::ELUX, getModel(), false);
  result += addBoolToString(getPower(), kPowerStr);
  result += addModeToString(getMode(), kEluxAuto, kEluxCool, kEluxHeat,
                            kEluxDry, kEluxFan);
  result += addTempToString(getTemp(), !getUseFahrenheit());
  result += addFanToString(getFan(), kEluxFanMax, kEluxFanMin, kEluxFanAuto,
                           kEluxFanAuto, kEluxFanMed);
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
    case kEluxSwingLastPos:
      result += kLastStr;
      break;
    case kEluxSwingAuto:
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
    case kEluxDisplayTempOff:
      result += kOffStr;
      break;
    case kEluxDisplayTempSet:
      result += kSetStr;
      break;
    case kEluxDisplayTempInside:
      result += kInsideStr;
      break;
    case kEluxDisplayTempOutside:
      result += kOutsideStr;
      break;
    default: result += kUnknownStr;
  }
  result += ')';
  return result;
}

#if DECODE_ELUX
/// Decode the supplied Elux HVAC message.
/// Status: STABLE / Working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool IRrecv::decodeElux(decode_results* results, uint16_t offset,
                        const uint16_t nbits, bool const strict) {
   DPRINT(" Elux /n");
   DPRINT(results->rawlen);
   DPRINT(nbits);
  if (results->rawlen <=
      2 * (nbits + kEluxBlockFooterBits) + (kHeader + kFooter + 1) - 1 + offset)
    return false;  // Can't possibly be a valid Elux message.
  if (strict && nbits != kEluxBits)
    return false;  // Not strictly a Elux message.

  // There are two blocks back-to-back in a full Elux IR message
  // sequence.

  uint16_t used;
  // Header + Data Block #1 (32 bits)
  used = matchGeneric(results->rawbuf + offset, results->state,
                      results->rawlen - offset, nbits / 2,
                      kEluxHdrMark, kEluxHdrSpace,
                      kEluxBitMark, kEluxOneSpace,
                      kEluxBitMark, kEluxZeroSpace,
                      0, 0, false,
                      _tolerance, kMarkExcess, false);
  if (used == 0) return false;
  offset += used;

  // Block #1 footer (3 bits, B010)
  match_result_t data_result;
  data_result = matchData(&(results->rawbuf[offset]), kEluxBlockFooterBits,
                          kEluxBitMark, kEluxOneSpace, kEluxBitMark,
                          kEluxZeroSpace, _tolerance, kMarkExcess, false);
  if (data_result.success == false) return false;
  if (data_result.data != kEluxBlockFooter) return false;
  offset += data_result.used;

  // Inter-block gap + Data Block #2 (32 bits) + Footer
  if (!matchGeneric(results->rawbuf + offset, results->state + 4,
                    results->rawlen - offset, nbits / 2,
                    kEluxBitMark, kEluxMsgSpace,
                    kEluxBitMark, kEluxOneSpace,
                    kEluxBitMark, kEluxZeroSpace,
                    kEluxBitMark, kEluxMsgSpace, true,
                    _tolerance, kMarkExcess, false)) return false;

  // Compliance
  if (strict) {
    // Verify the message's checksum is correct.
    if (!IREluxAC::validChecksum(results->state)) return false;
  }

  // Success
  results->decode_type = ELUX;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // DECODE_ELUX
