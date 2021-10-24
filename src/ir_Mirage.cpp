// Copyright 2020-2021 David Conran (crankyoldgit)
/// @file
/// @brief Support for Mirage protocol
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1289
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1573


#include "ir_Mirage.h"
#include <algorithm>
#include <cstring>
#ifndef ARDUINO
#include <string>
#endif
#include "IRrecv.h"
#include "IRsend.h"
#include "IRtext.h"
#include "IRutils.h"

using irutils::addBoolToString;
using irutils::addFanToString;
using irutils::addIntToString;
using irutils::addLabeledString;
using irutils::addModeToString;
using irutils::addSwingHToString;
using irutils::addSwingVToString;
using irutils::addTempToString;
using irutils::minsToString;
using irutils::bcdToUint8;
using irutils::uint8ToBcd;
using irutils::sumNibbles;

// Constants
const uint16_t kMirageHdrMark = 8360;            ///< uSeconds
const uint16_t kMirageBitMark = 554;             ///< uSeconds
const uint16_t kMirageHdrSpace = 4248;           ///< uSeconds
const uint16_t kMirageOneSpace = 1592;           ///< uSeconds
const uint16_t kMirageZeroSpace = 545;           ///< uSeconds
const uint32_t kMirageGap = kDefaultMessageGap;  ///< uSeconds (just a guess)
const uint16_t kMirageFreq = 38000;              ///< Hz. (Just a guess)


#if SEND_MIRAGE
/// Send a Mirage formatted message.
/// Status: STABLE / Reported as working.
/// @param[in] data An array of bytes containing the IR command.
/// @param[in] nbytes Nr. of bytes of data in the array. (>=kMirageStateLength)
/// @param[in] repeat Nr. of times the message is to be repeated.
void IRsend::sendMirage(const uint8_t data[], const uint16_t nbytes,
                        const uint16_t repeat) {
  sendGeneric(kMirageHdrMark, kMirageHdrSpace,
              kMirageBitMark, kMirageOneSpace,
              kMirageBitMark, kMirageZeroSpace,
              kMirageBitMark, kMirageGap,
              data, nbytes, kMirageFreq, false,  // LSB
              repeat, kDutyDefault);
}
#endif  // SEND_MIRAGE

#if DECODE_MIRAGE
/// Decode the supplied Mirage message.
/// Status: STABLE / Reported as working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool IRrecv::decodeMirage(decode_results *results, uint16_t offset,
                          const uint16_t nbits, const bool strict) {
  if (strict && nbits != kMirageBits) return false;  // Compliance.

  if (!matchGeneric(results->rawbuf + offset, results->state,
                    results->rawlen - offset, nbits,
                    kMirageHdrMark, kMirageHdrSpace,
                    kMirageBitMark, kMirageOneSpace,
                    kMirageBitMark, kMirageZeroSpace,
                    kMirageBitMark, kMirageGap, true,
                    kUseDefTol, kMarkExcess, false)) return false;
  // Compliance
  if (strict && !IRMirageAc::validChecksum(results->state)) return false;

  // Success
  results->decode_type = decode_type_t::MIRAGE;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}

// Code to emulate Mirage A/C IR remote control unit.

/// Class constructor
/// @param[in] pin GPIO to be used when sending.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRMirageAc::IRMirageAc(const uint16_t pin, const bool inverted,
                               const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) { stateReset(); }

/// Reset the state of the remote to a known good state/sequence.
void IRMirageAc::stateReset(void) {
  // The state of the IR remote in IR code form.
  static const uint8_t kReset[kMirageStateLength] = {
      0x56, 0x6C, 0x00, 0x00, 0x20, 0x1A, 0x00, 0x00,
      0x0C, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x42};
  setRaw(kReset);
}

/// Set up hardware to be able to send a message.
void IRMirageAc::begin(void) { _irsend.begin(); }

#if SEND_MITSUBISHI_AC
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRMirageAc::send(const uint16_t repeat) {
  _irsend.sendMirage(getRaw(), kMirageStateLength, repeat);
}
#endif  // SEND_MITSUBISHI_AC

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *IRMirageAc::getRaw(void) {
  checksum();
  return _.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] data A valid code for this protocol.
void IRMirageAc::setRaw(const uint8_t *data) {
  std::memcpy(_.raw, data, kMirageStateLength);
}

/// Calculate and set the checksum values for the internal state.
void IRMirageAc::checksum(void) { _.Sum = calculateChecksum(_.raw); }

/// Verify the checksum is valid for a given state.
/// @param[in] data The array to verify the checksum of.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool IRMirageAc::validChecksum(const uint8_t *data) {
  return calculateChecksum(data) == data[kMirageStateLength - 1];
}

/// Calculate the checksum for a given state.
/// @param[in] data The value to calc the checksum of.
/// @return The calculated checksum value.
uint8_t IRMirageAc::calculateChecksum(const uint8_t *data) {
  return sumNibbles(data, kMirageStateLength - 1);
}

/// Set the requested power state of the A/C to on.
void IRMirageAc::on(void) { setPower(true); }

/// Set the requested power state of the A/C to off.
void IRMirageAc::off(void) { setPower(false); }

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRMirageAc::setPower(bool on) {
  // In order to change the power setting, it seems must be less than
  // kMirageAcPowerOff. kMirageAcPowerOff is larger than half of the possible
  // value stored in the allocated bit space.
  // Thus if the value is larger than kMirageAcPowerOff the power is off.
  // Less than, then power is on.
  // We can't just aribitarily add or subtract the value (which analysis
  // indicates is how the power status changes. Very weird, I know!) as that is
  // not an idempotent action, we must check if the addition or substraction is
  // needed first. e.g. via getPower()
  // i.e. If we added or subtracted twice, we would cause a wrap of the integer
  // and not get the desired result.
  if (on)
    _.SwingAndPower -= getPower() ? 0 : kMirageAcPowerOff;
  else
    _.SwingAndPower += getPower() ? kMirageAcPowerOff : 0;
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool IRMirageAc::getPower(void) const {
  return _.SwingAndPower < kMirageAcPowerOff;
}


/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t IRMirageAc::getMode(void) const { return _.Mode; }

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void IRMirageAc::setMode(const uint8_t mode) {
  switch (mode) {
    case kMirageAcCool:
    case kMirageAcDry:
    case kMirageAcHeat:
    case kMirageAcFan:
    case kMirageAcRecycle:
      _.Mode = mode;
      // Reset turbo if we have to.
      setTurbo(getTurbo());
      break;
    default:  // Default to cool mode for anything else.
      setMode(kMirageAcCool);
  }
}

/// Set the temperature.
/// @param[in] degrees The temperature in degrees celsius.
void IRMirageAc::setTemp(const uint8_t degrees) {
  // Make sure we have desired temp in the correct range.
  uint8_t celsius = std::max(degrees, kMirageAcMinTemp);
  _.Temp = std::min(celsius, kMirageAcMaxTemp) + kMirageAcTempOffset;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t IRMirageAc::getTemp(void) const { return _.Temp - kMirageAcTempOffset; }

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
void IRMirageAc::setFan(const uint8_t speed) {
  _.Fan = (speed <= kMirageAcFanLow) ? speed : kMirageAcFanAuto;
}

/// Get the current fan speed setting.
/// @return The current fan speed/mode.
uint8_t IRMirageAc::getFan(void) const { return _.Fan; }

/// Change the Turbo setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRMirageAc::setTurbo(bool on) {
  _.Turbo = (on && (getMode() == kMirageAcCool));
}

/// Get the value of the current Turbo setting.
/// @return true, the setting is on. false, the setting is off.
bool IRMirageAc::getTurbo(void) const { return _.Turbo; }

/// Change the Sleep setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRMirageAc::setSleep(bool on) { _.Sleep = on; }

/// Get the value of the current Sleep setting.
/// @return true, the setting is on. false, the setting is off.
bool IRMirageAc::getSleep(void) const { return _.Sleep; }

/// Change the Light/Display setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRMirageAc::setLight(bool on) { _.Light = on; }

/// Get the value of the current Light/Display setting.
/// @return true, the setting is on. false, the setting is off.
bool IRMirageAc::getLight(void) const { return _.Light; }

/// Get the clock time of the A/C unit.
/// @return Nr. of seconds past midnight.
uint32_t IRMirageAc::getClock(void) const {
  return ((bcdToUint8(_.Hours) * 60) + bcdToUint8(_.Minutes)) * 60 +
      bcdToUint8(_.Seconds);
}

/// Set the clock time on the A/C unit.
/// @param[in] nr_of_seconds Nr. of seconds past midnight.
void IRMirageAc::setClock(const uint32_t nr_of_seconds) {
  uint32_t remaining = std::min(
      nr_of_seconds, (uint32_t)(24 * 60 * 60 - 1));  // Limit to 23:59:59.
  _.Seconds = uint8ToBcd(remaining % 60);
  remaining /= 60;
  _.Minutes = uint8ToBcd(remaining % 60);
  remaining /= 60;
  _.Hours = uint8ToBcd(remaining);
}

/// Set the Vertical Swing setting/position of the A/C.
/// @param[in] position The desired swing setting.
void IRMirageAc::setSwingV(const uint8_t position) {
  const bool power = getPower();
  switch (position) {
    case kMirageAcSwingVLowest:
    case kMirageAcSwingVLow:
    case kMirageAcSwingVMiddle:
    case kMirageAcSwingVHigh:
    case kMirageAcSwingVHighest:
    case kMirageAcSwingVAuto:
      _.SwingAndPower = position;
      // Power needs to be reapplied after overwriting SwingAndPower
      setPower(power);
      break;
    default:  // Default to Auto for anything else.
      setSwingV(kMirageAcSwingVAuto);
  }
}

/// Get the Vertical Swing setting/position of the A/C.
/// @return The desired Vertical Swing setting/position.
uint8_t IRMirageAc::getSwingV(void) const {
  return _.SwingAndPower - (getPower() ? 0 : kMirageAcPowerOff);
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::opmode_t IRMirageAc::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kMirageAcHeat: return stdAc::opmode_t::kHeat;
    case kMirageAcDry:  return stdAc::opmode_t::kDry;
    case kMirageAcFan:  return stdAc::opmode_t::kFan;
    default:            return stdAc::opmode_t::kCool;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::fanspeed_t IRMirageAc::toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kMirageAcFanHigh:  return stdAc::fanspeed_t::kHigh;
    case kMirageAcFanMed:   return stdAc::fanspeed_t::kMedium;
    case kMirageAcFanLow:   return stdAc::fanspeed_t::kLow;
    default:                return stdAc::fanspeed_t::kAuto;
  }
}

/// Convert a stdAc::opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRMirageAc::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kHeat: return kMirageAcHeat;
    case stdAc::opmode_t::kDry:  return kMirageAcDry;
    case stdAc::opmode_t::kFan:  return kMirageAcFan;
    default:                     return kMirageAcCool;
  }
}

/// Convert a stdAc::fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRMirageAc::convertFan(const stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin:
    case stdAc::fanspeed_t::kLow:    return kMirageAcFanLow;
    case stdAc::fanspeed_t::kMedium: return kMirageAcFanMed;
    case stdAc::fanspeed_t::kHigh:
    case stdAc::fanspeed_t::kMax:    return kMirageAcFanHigh;
    default:                         return kMirageAcFanAuto;
  }
}

/// Convert a stdAc::swingv_t enum into it's native setting.
/// @param[in] position The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRMirageAc::convertSwingV(const stdAc::swingv_t position) {
  switch (position) {
    case stdAc::swingv_t::kHighest:     return kMirageAcSwingVHighest;
    case stdAc::swingv_t::kHigh:        return kMirageAcSwingVHigh;
    case stdAc::swingv_t::kMiddle:      return kMirageAcSwingVMiddle;
    case stdAc::swingv_t::kLow:         return kMirageAcSwingVLow;
    case stdAc::swingv_t::kLowest:      return kMirageAcSwingVLowest;
    default:                            return kMirageAcSwingVAuto;
  }
}

/// Convert a native vertical swing postion to it's common equivalent.
/// @param[in] pos A native position to convert.
/// @return The common vertical swing position.
stdAc::swingv_t IRMirageAc::toCommonSwingV(const uint8_t pos) {
  switch (pos) {
    case kMirageAcSwingVHighest: return stdAc::swingv_t::kHighest;
    case kMirageAcSwingVHigh:    return stdAc::swingv_t::kHigh;
    case kMirageAcSwingVMiddle:  return stdAc::swingv_t::kMiddle;
    case kMirageAcSwingVLow:     return stdAc::swingv_t::kLow;
    case kMirageAcSwingVLowest:  return stdAc::swingv_t::kLowest;
    default:                     return stdAc::swingv_t::kAuto;
  }
}

/// Convert the current internal state into its stdAc::state_t equivalent.
/// @return The stdAc equivalent of the native settings.
stdAc::state_t IRMirageAc::toCommon(void) const {
  stdAc::state_t result;
  result.protocol = decode_type_t::MIRAGE;
  result.model = -1;  // No models used.
  result.power = getPower();
  result.mode = toCommonMode(_.Mode);
  result.celsius = true;
  result.degrees = getTemp();
  result.fanspeed = toCommonFanSpeed(getFan());
  result.swingv = toCommonSwingV(getSwingV());
  result.turbo = getTurbo();
  result.light = getLight();
  result.sleep = getSleep() ? 0 : -1;
  // Not supported.
  result.swingh = stdAc::swingh_t::kOff;
  result.quiet = false;
  result.clean = false;
  result.econo = false;
  result.filter = false;
  result.beep = false;
  result.clock = -1;
  return result;
}

/// Convert the internal state into a human readable string.
/// @return A string containing the settings in human-readable form.
String IRMirageAc::toString(void) const {
  String result = "";
  result.reserve(110);  // Reserve some heap for the string to reduce fragging.
  result += addBoolToString(getPower(), kPowerStr, false);
  result += addModeToString(_.Mode, 0xFF, kMirageAcCool,
                            kMirageAcHeat, kMirageAcDry,
                            kMirageAcFan);
  result += addTempToString(getTemp());
  result += addFanToString(_.Fan, kMirageAcFanHigh,
                           kMirageAcFanLow,
                           kMirageAcFanAuto, kMirageAcFanAuto,
                           kMirageAcFanMed);
  result += addSwingVToString(getSwingV(),
                              kMirageAcSwingVAuto,
                              kMirageAcSwingVHighest,
                              kMirageAcSwingVHigh,
                              0xFF,  // Unused.
                              kMirageAcSwingVMiddle,
                              0xFF,  // Unused.
                              kMirageAcSwingVLow,
                              kMirageAcSwingVLowest,
                              0xFF, 0xFF, 0xFF, 0xFF);  // Unused.
  result += addBoolToString(_.Turbo, kTurboStr);
  result += addBoolToString(_.Light, kLightStr);
  result += addBoolToString(_.Sleep, kSleepStr);
  result += addLabeledString(minsToString(getClock() / 60), kClockStr);
  return result;
}
#endif  // DECODE_MIRAGE
