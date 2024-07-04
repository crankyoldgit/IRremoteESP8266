// Copyright 2024 QuangThai2297

/// @file
/// @brief Support for FUNIKI A/C protocols.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/2112


#include "ir_Funiki.h"
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
#include "ir_Kelvinator.h"

// Constants
const uint16_t kFunikiHdrMark = 9000;
///< See #684 & real example in unit tests
const uint16_t kFunikiHdrSpace = 4500;
const uint16_t kFunikiBitMark = 620;
const uint16_t kFunikiOneSpace = 1600;
const uint16_t kFunikiZeroSpace = 540;
///< See #1508, #386, & Kelvinator
const uint16_t kFunikiMsgSpace = 19980;
const uint8_t kFunikiBlockFooter = 0b101;
const uint8_t kFunikiBlockFooterBits = 3;

using irutils::addBoolToString;
using irutils::addIntToString;
using irutils::addLabeledString;
using irutils::addModeToString;
using irutils::addModelToString;
using irutils::addFanToString;
using irutils::addSwingHToString;
using irutils::addTempToString;
using irutils::minsToString;
using irutils::bcdToUint8;
using irutils::uint8ToBcd;

#if SEND_FUNIKI
/// Send a Funiki Heat Pump formatted message.
/// Status: STABLE / Working.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void IRsend::sendFuniki(const uint8_t data[], const uint16_t nbytes,
                      const uint16_t repeat) {
  if (nbytes < kFunikiStateLength)
    return;  // Not enough bytes to send a proper message.

  for (uint16_t r = 0; r <= repeat; r++) {
    // Block #1
    sendGeneric(kFunikiHdrMark, kFunikiHdrSpace, kFunikiBitMark,
                kFunikiOneSpace,
                kFunikiBitMark, kFunikiZeroSpace, 0, 0,  // No Footer.
                data, nbytes, 38, false, 0, 50);
    // Footer #1
    sendGeneric(0, 0,  // No Header
                kFunikiBitMark, kFunikiOneSpace, kFunikiBitMark,
                kFunikiZeroSpace,
                kFunikiBitMark, kFunikiMsgSpace, 0b101, 3, 38, false, 0, 50);
  }
}
#endif  // SEND_FUNIKI

/// Class constructor
/// @param[in] pin GPIO to be used when sending.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRFunikiAC::IRFunikiAC(const uint16_t pin,
                   const bool inverted, const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) {
  stateReset();
}

/// Reset the internal state to a fixed known good state.
void IRFunikiAC::stateReset(void) {
  // This resets to a known-good state to Power Off, Fan Auto, Mode Auto, 25C.
  std::memset(_.remote_state, 0, sizeof _.remote_state);
  _.Temp = 9;  // _.remote_state[1] = 0x09;
  _.unknown1 = 0x60;  // _.remote_state[5] = 0x20;
}

/// Fix up the internal state so it is correct.
/// @note Internal use only.
void IRFunikiAC::fixup(void) {
  setPower(getPower());
}

/// Set up hardware to be able to send a message.
void IRFunikiAC::begin(void) { _irsend.begin(); }

#if SEND_FUNIKI
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRFunikiAC::send(const uint16_t repeat) {
  _irsend.sendFuniki(getRaw(), kFunikiStateLength, repeat);
}
#endif  // SEND_FUNIKI

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t* IRFunikiAC::getRaw(void) {
  fixup();  // Ensure correct settings before sending.
  return _.remote_state;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
void IRFunikiAC::setRaw(const uint8_t new_code[]) {
  std::memcpy(_.remote_state, new_code, kFunikiStateLength);
}

/// Change the power setting to On.
void IRFunikiAC::on(void) { setPower(true); }

/// Change the power setting to Off.
void IRFunikiAC::off(void) { setPower(false); }

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRFunikiAC::setPower(const bool on) {
  _.Power = on;
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool IRFunikiAC::getPower(void) const {
  return _.Power;
}


/// Set the temp. in degrees
/// @param[in] temp Desired temperature in Degrees.
/// @param[in] fahrenheit Use units of Fahrenheit and set that as units used.
///   false is Celsius (Default), true is Fahrenheit.
/// @note The unit actually works in Celsius with a special optional
///   "extra degree" when sending Fahrenheit.
void IRFunikiAC::setTemp(const uint8_t temp, const bool fahrenheit) {
  float safecelsius = temp;
  if (fahrenheit)
    // Covert to F, and add a fudge factor to round to the expected degree.
    // Why 0.6 you ask?! Because it works. Ya'd thing 0.5 would be good for
    // rounding, but Noooooo!
    safecelsius = fahrenheitToCelsius(temp + 0.6);

  // Make sure we have desired temp in the correct range.
  safecelsius = std::max(static_cast<float>(kFunikiMinTempC), safecelsius);
  safecelsius = std::min(static_cast<float>(kFunikiMaxTempC), safecelsius);
  // An operating mode of Auto locks the temp to a specific value. Do so.
  // if (_.Mode == kFunikiAuto) safecelsius = 25;
  if (_.Mode != kFunikiCool || _.AutoMode == kFunikiAutoModeOn)
    safecelsius = kFunikiMinTempC;
  // Set the "main" Celsius degrees.
  _.Temp = safecelsius - kFunikiMinTempC;
  // Deal with the extra degree fahrenheit difference.
  // _.TempExtraDegreeF = (static_cast<uint8_t>(safecelsius * 2) & 1);
}

/// Get the set temperature
/// @return The temperature in degrees in the current units (C/F) set.
uint8_t IRFunikiAC::getTemp(void) const {
  uint8_t deg = kFunikiMinTempC + _.Temp;
  return deg;
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting. 0 is auto, 1-3 is the speed.
void IRFunikiAC::setFan(const uint8_t speed) {
  uint8_t fan = std::min(kFunikiFanMax, speed);  // Bounds check
  if (_.Mode == kFunikiDry) fan = 1;  // DRY mode is always locked to fan 1.
  // Set the basic fan values.
  _.Fan = fan;
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t IRFunikiAC::getFan(void) const { return _.Fan; }

/// Set the operating mode of the A/C.
/// @param[in] new_mode The desired operating mode.
void IRFunikiAC::setMode(const uint8_t new_mode) {
  uint8_t mode = new_mode;
  switch (mode) {
    // AUTO is locked to 25C
    case kFunikiAuto: setTemp(25); break;
    // DRY always sets the fan to 1.
    case kFunikiDry: setFan(1); break;
    case kFunikiCool:
    case kFunikiFan: break;
    // If we get an unexpected mode, default to AUTO.
    default: mode = kFunikiAuto;
  }
  if (mode == kFunikiAuto) {
    _.Mode = kFunikiCool;
    _.AutoMode = kFunikiAutoModeOn;
  } else {
    _.Mode = mode;
    _.AutoMode = kFunikiAutoModeOff;
  }
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t IRFunikiAC::getMode(void) const { return _.Mode; }

/// Set the Sleep setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRFunikiAC::setSleep(const bool on) { _.Sleep = on; }

/// Get the Sleep setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRFunikiAC::getSleep(void) const { return _.Sleep; }


/// Set the Vertical Swing mode of the A/C.
/// @param[in] automatic Do we use the automatic setting?
/// @param[in] position The position/mode to set the vanes to.
void IRFunikiAC::setSwingVertical(const bool automatic,
                const uint8_t position) {
  uint8_t new_position = position;
  if (!automatic) {
    switch (position) {
      case kFunikiSwingUp:
      case kFunikiSwingMiddleUp:
      case kFunikiSwingMiddle:
      case kFunikiSwingMiddleDown:
      case kFunikiSwingDown:
        break;
      default:
        new_position = kFunikiSwingLastPos;
    }
  } else {
      new_position = kFunikiSwingAuto;
  }
  _.SwingV = new_position;
}

/// Get the Vertical Swing Automatic mode setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRFunikiAC::getSwingVerticalAuto(void) const {
  return (_.SwingV == kFunikiSwingAuto);
}

/// Get the Vertical Swing position setting of the A/C.
/// @return The native position/mode.
uint8_t IRFunikiAC::getSwingVerticalPosition(void) const { return _.SwingV; }



/// Get the clock time of the A/C unit.
/// @return Nr. of seconds past midnight.
int16_t IRFunikiAC::getClock(void) const {
  return (bcdToUint8((_.Hours1 << kNibbleSize) |  _.Hours2) * 60)
  + bcdToUint8((_.Minutes1 << kNibbleSize) |  _.Minutes2);
}

/// Set the clock of the A/C.
/// @param[in] nr_of_minutes Number of minutes in a day
void IRFunikiAC::setClock(const int16_t nr_of_minutes) {
  uint32_t remaining = nr_of_minutes;
  _.Minutes1 = uint8ToBcd(remaining % 60)>>4;
  _.Minutes2 = uint8ToBcd(remaining % 60) & 0x0F;
  remaining /= 60;
  _.Hours1 = uint8ToBcd(remaining % 60)>>4;
  _.Hours2 = uint8ToBcd(remaining % 60) & 0x0F;
}


// /// Get the timer on enabled setting of the A/C.
// /// @return true, the setting is on. false, the setting is off.
bool IRFunikiAC::getTimerOnEnabled(void) const { return _.TimerOnEnable; }

// /// Get the timer ofd enabled setting of the A/C.
// /// @return true, the setting is on. false, the setting is off.
bool IRFunikiAC::getTimerOffEnabled(void) const { return _.TimerOffEnable; }

// /// Get the timer on time value from the A/C.
// /// @return The number of minutes the timer is set for.
uint16_t IRFunikiAC::getTimerOn(void) const {
  uint16_t hrs = irutils::bcdToUint8((_.TimerOnHours1 << kNibbleSize) |
    _.TimerOnHours2);
  return hrs * 60 + (_.TimerOnMinutes * 10);
}
// /// Get the timer off time value from the A/C.
// /// @return The number of minutes the timer is set for.
uint16_t IRFunikiAC::getTimerOff(void) const {
  uint16_t hrs = irutils::bcdToUint8((_.TimerOffHours1 << kNibbleSize) |
    _.TimerOffHours2);
  return hrs * 60 + (_.TimerOffMinutes * 10);
}

/// Convert a stdAc::opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRFunikiAC::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kCool: return kFunikiCool;
    case stdAc::opmode_t::kDry:  return kFunikiDry;
    case stdAc::opmode_t::kFan:  return kFunikiFan;
    default:                     return kFunikiAuto;
  }
}

/// Convert a stdAc::fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRFunikiAC::convertFan(const stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin:    return kFunikiFanMin;
    case stdAc::fanspeed_t::kLow:
    case stdAc::fanspeed_t::kMedium: return kFunikiFanMax - 1;
    case stdAc::fanspeed_t::kHigh:
    case stdAc::fanspeed_t::kMax:    return kFunikiFanMax;
    default:                         return kFunikiFanAuto;
  }
}

/// Convert a stdAc::swingv_t enum into it's native setting.
/// @param[in] swingv The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRFunikiAC::convertSwingV(const stdAc::swingv_t swingv) {
  switch (swingv) {
    case stdAc::swingv_t::kHighest: return kFunikiSwingUp;
    case stdAc::swingv_t::kHigh:    return kFunikiSwingMiddleUp;
    case stdAc::swingv_t::kMiddle:  return kFunikiSwingMiddle;
    case stdAc::swingv_t::kLow:     return kFunikiSwingMiddleDown;
    case stdAc::swingv_t::kLowest:  return kFunikiSwingDown;
    default:                        return kFunikiSwingAuto;
  }
}


/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @param[in] isAutoMode The addition info setting mode to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::opmode_t IRFunikiAC::toCommonMode(const uint8_t mode,
                                    uint8_t isAutoMode) {
  if (isAutoMode == kFunikiAutoModeOn) {
    return stdAc::opmode_t::kAuto;
  }
  switch (mode) {
    case kFunikiCool: return stdAc::opmode_t::kCool;
    // case kFunikiHeat: return stdAc::opmode_t::kHeat;
    case kFunikiDry:  return stdAc::opmode_t::kDry;
    case kFunikiFan:  return stdAc::opmode_t::kFan;
    default:        return stdAc::opmode_t::kAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::fanspeed_t IRFunikiAC::toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kFunikiFanMax:     return stdAc::fanspeed_t::kMax;
    case kFunikiFanMax - 1: return stdAc::fanspeed_t::kMedium;
    case kFunikiFanMin:     return stdAc::fanspeed_t::kMin;
    default:              return stdAc::fanspeed_t::kAuto;
  }
}

/// Convert a native Vertical Swing into its stdAc equivalent.
/// @param[in] pos The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::swingv_t IRFunikiAC::toCommonSwingV(const uint8_t pos) {
  switch (pos) {
    case kFunikiSwingUp:         return stdAc::swingv_t::kHighest;
    case kFunikiSwingMiddleUp:   return stdAc::swingv_t::kHigh;
    case kFunikiSwingMiddle:     return stdAc::swingv_t::kMiddle;
    case kFunikiSwingMiddleDown: return stdAc::swingv_t::kLow;
    case kFunikiSwingDown:       return stdAc::swingv_t::kLowest;
    default:                   return stdAc::swingv_t::kAuto;
  }
}



/// Convert the current internal state into its stdAc::state_t equivalent.
/// @return The stdAc equivalent of the native settings.
stdAc::state_t IRFunikiAC::toCommon(void) {
  stdAc::state_t result{};
  result.protocol = decode_type_t::FUNIKI;
  result.model = -1;
  result.power = _.Power;
  result.mode = toCommonMode(_.Mode, _.AutoMode);
  result.degrees = getTemp();
  // no support for Sensor temp.
  result.fanspeed = toCommonFanSpeed(_.Fan);
  if (_.SwingV == kFunikiSwingAuto)
    result.swingv = stdAc::swingv_t::kAuto;
  else
    result.swingv = toCommonSwingV(_.SwingV);

  result.sleep = _.Sleep ? 0 : -1;
  // Not supported.
  // result.quiet = false;
  // result.filter = false;
  // result.beep = false;
  result.clock = getClock();
  return result;
}

/// Convert the current internal state into a human readable string.
/// @return A human readable string.
String IRFunikiAC::toString(void) {
  String result = "";
  result.reserve(220);  // Reserve some heap for the string to reduce fragging.
  result += addBoolToString(_.Power, kPowerStr);
  if (_.AutoMode == kFunikiAutoModeOn) {
    result += addModeToString(kFunikiAuto, kFunikiAuto, kFunikiCool,
                              kFunikiHeat, kFunikiDry, kFunikiFan);
  } else {
    result += addModeToString(_.Mode, kFunikiAuto, kFunikiCool, kFunikiHeat,
                                kFunikiDry, kFunikiFan);
  }
  result += addTempToString(getTemp(), true);
  result += addFanToString(_.Fan, kFunikiFanMax, kFunikiFanMin, kFunikiFanAuto,
                           kFunikiFanAuto, kFunikiFanMed);
  result += addBoolToString(_.Sleep, kSleepStr);
  result += addIntToString(_.SwingV, kSwingVStr);
  result += kSpaceLBraceStr;
  switch (_.SwingV) {
    case kFunikiSwingLastPos:
      result += kLastStr;
      break;
    case kFunikiSwingAuto:
      result += kAutoStr;
      break;
    default: result += kUnknownStr;
  }
  result += ')';
  result += addLabeledString(
      getTimerOnEnabled() ?
      minsToString(getTimerOn()) : kOffStr, kOnTimerStr);

  result += addLabeledString(
      getTimerOffEnabled() ?
      minsToString(getTimerOff()) : kOffStr, kOffTimerStr);
  result += addLabeledString(minsToString(getClock()), kClockStr);
  return result;
}

#if DECODE_FUNIKI
/// Decode the supplied Gree HVAC message.
/// Status: STABLE / Working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool IRrecv::decodeFuniki(decode_results* results, uint16_t offset,
                        const uint16_t nbits, bool const strict) {
  if (results->rawlen <=
      2 * (nbits + kFunikiBlockFooterBits) + (kHeader) - 1 + offset)
    return false;  // Can't possibly be a valid Funiki message.
  if (strict && nbits != kFunikiBits)
    return false;  // Not strictly a Funiki message.
  // There are two blocks back-to-back in a full Funiki IR message
  // sequence.
  uint16_t used;
  // Header + Data Block #1 (80 bits)
  used = matchGeneric(results->rawbuf + offset, results->state,
                      results->rawlen - offset, nbits,
                      kFunikiHdrMark, kFunikiHdrSpace,
                      kFunikiBitMark, kFunikiOneSpace,
                      kFunikiBitMark, kFunikiZeroSpace,
                      0, 0, false,
                      _tolerance, kMarkExcess, false);
  if (used == 0) return false;
  offset += used;
  // Block #1 footer (3 bits, B101)
  match_result_t data_result;
  data_result = matchData(&(results->rawbuf[offset]), kFunikiBlockFooterBits,
                          kFunikiBitMark, kFunikiOneSpace, kFunikiBitMark,
                          kFunikiZeroSpace, _tolerance, kMarkExcess, false);
  if (data_result.success == false) return false;
  if (data_result.data != kFunikiBlockFooter) return false;
  offset += data_result.used;
  // Success
  results->decode_type = FUNIKI;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // DECODE_FUNIKI
