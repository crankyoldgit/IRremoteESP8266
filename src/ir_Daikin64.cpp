// Copyright 2016 sillyfrog
// Copyright 2017 sillyfrog, crankyoldgit
// Copyright 2018-2022 crankyoldgit
// Copyright 2019 pasna (IRDaikin160 class / Daikin176 class)

/// @file
/// @brief Support for Daikin A/C protocols.
/// @see Daikin http://harizanov.com/2012/02/control-daikin-air-conditioner-over-the-internet/
/// @see Daikin https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/tree/master/IRremote
/// @see Daikin http://rdlab.cdmt.vn/project-2013/daikin-ir-protocol
/// @see Daikin https://github.com/blafois/Daikin-IR-Reverse
/// @see Daikin128 https://github.com/crankyoldgit/IRremoteESP8266/issues/827
/// @see Daikin152 https://github.com/crankyoldgit/IRremoteESP8266/issues/873
/// @see Daikin152 https://github.com/ToniA/arduino-heatpumpir/blob/master/DaikinHeatpumpARC480A14IR.cpp
/// @see Daikin152 https://github.com/ToniA/arduino-heatpumpir/blob/master/DaikinHeatpumpARC480A14IR.h
/// @see Daikin160 https://github.com/crankyoldgit/IRremoteESP8266/issues/731
/// @see Daikin2 https://docs.google.com/spreadsheets/d/1f8EGfIbBUo2B-CzUFdrgKQprWakoYNKM80IKZN4KXQE/edit#gid=236366525&range=B25:D32
/// @see Daikin2 https://github.com/crankyoldgit/IRremoteESP8266/issues/582
/// @see Daikin2 https://github.com/crankyoldgit/IRremoteESP8266/issues/1535
/// @see Daikin2 https://www.daikin.co.nz/sites/default/files/daikin-split-system-US7-FTXZ25-50NV1B.pdf
/// @see Daikin216 https://github.com/crankyoldgit/IRremoteESP8266/issues/689
/// @see Daikin216 https://github.com/danny-source/Arduino_DY_IRDaikin
/// @see Daikin64 https://github.com/crankyoldgit/IRremoteESP8266/issues/1064
/// @see Daikin200 https://github.com/crankyoldgit/IRremoteESP8266/issues/1802

#include "ir_Daikin.h"
#include <algorithm>
#include <cstring>
#ifndef ARDUINO
#include <string>
#endif
#include "IRrecv.h"
#include "IRremoteESP8266.h"
#include "IRsend.h"
#ifdef UNIT_TEST
#include "IRsend_test.h"
#endif
#include "IRtext.h"
#include "IRutils.h"

using irutils::addBoolToString;
using irutils::addDayToString;
using irutils::addIntToString;
using irutils::addLabeledString;
using irutils::addModeToString;
using irutils::addSwingHToString;
using irutils::addSwingVToString;
using irutils::addTempToString;
using irutils::addFanToString;
using irutils::bcdToUint8;
using irutils::minsToString;
using irutils::setBit;
using irutils::setBits;
using irutils::sumNibbles;
using irutils::uint8ToBcd;

#if SEND_DAIKIN64
/// Send a Daikin64 (64-bit) A/C formatted message.
/// Status: Beta / Probably Working.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1064
void IRsend::sendDaikin64(const uint64_t data, const uint16_t nbits,
                          const uint16_t repeat) {
  enableIROut(kDaikin64Freq);
  for (uint16_t r = 0; r <= repeat; r++) {
    for (uint8_t i = 0; i < 2; i++) {
      // Leader
      mark(kDaikin64LdrMark);
      space(kDaikin64LdrSpace);
    }
    // Header + Data + Footer #1
    sendGeneric(kDaikin64HdrMark, kDaikin64HdrSpace,
                kDaikin64BitMark, kDaikin64OneSpace,
                kDaikin64BitMark, kDaikin64ZeroSpace,
                kDaikin64BitMark, kDaikin64Gap,
                data, nbits, kDaikin64Freq, false, 0, 50);
    // Footer #2
    mark(kDaikin64HdrMark);
    space(kDefaultMessageGap);  // A guess of the gap between messages.
  }
}
#endif  // SEND_DAIKIN64

#if DECODE_DAIKIN64
/// Decode the supplied Daikin 64-bit message. (DAIKIN64)
/// Status: Beta / Probably Working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1064
bool IRrecv::decodeDaikin64(decode_results *results, uint16_t offset,
                            const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * nbits + kDaikin64Overhead - offset)
    return false;  // Too short a message to match.
  // Compliance
  if (strict && nbits != kDaikin64Bits)
    return false;

  // Leader
  for (uint8_t i = 0; i < 2; i++) {
    if (!matchMark(results->rawbuf[offset++], kDaikin64LdrMark))
      return false;
    if (!matchSpace(results->rawbuf[offset++], kDaikin64LdrSpace))
      return false;
  }
  // Header + Data + Footer #1
  uint16_t used = matchGeneric(results->rawbuf + offset, &results->value,
                               results->rawlen - offset, nbits,
                               kDaikin64HdrMark, kDaikin64HdrSpace,
                               kDaikin64BitMark, kDaikin64OneSpace,
                               kDaikin64BitMark, kDaikin64ZeroSpace,
                               kDaikin64BitMark, kDaikin64Gap,
                               false, _tolerance + kDaikin64ToleranceDelta,
                               kMarkExcess, false);
  if (used == 0) return false;
  offset += used;
  // Footer #2
  if (!matchMark(results->rawbuf[offset++], kDaikin64HdrMark))
    return false;

  // Compliance
  if (strict && !IRDaikin64::validChecksum(results->value)) return false;
  // Success
  results->decode_type = decode_type_t::DAIKIN64;
  results->bits = nbits;
  results->command = 0;
  results->address = 0;
  return true;
}
#endif  // DAIKIN64

/// Class constructor
/// @param[in] pin GPIO to be used when sending.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRDaikin64::IRDaikin64(const uint16_t pin, const bool inverted,
                         const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) { stateReset(); }

/// Set up hardware to be able to send a message.
void IRDaikin64::begin(void) { _irsend.begin(); }

#if SEND_DAIKIN64
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRDaikin64::send(const uint16_t repeat) {
  _irsend.sendDaikin64(getRaw(), kDaikin64Bits, repeat);
}
#endif  // SEND_DAIKIN64

/// Calculate the checksum for a given state.
/// @param[in] state The value to calc the checksum of.
/// @return The 4-bit checksum stored in a uint_8.
uint8_t IRDaikin64::calcChecksum(const uint64_t state) {
  uint64_t data = GETBITS64(state, 0, kDaikin64ChecksumOffset);
  uint8_t result = 0;
  for (; data; data >>= 4)  // Add each nibble together.
    result += GETBITS64(data, 0, 4);
  return result & 0xF;
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The state to verify the checksum of.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool IRDaikin64::validChecksum(const uint64_t state) {
  // Validate the checksum of the given state.
  return (GETBITS64(state, kDaikin64ChecksumOffset,
                    kDaikin64ChecksumSize) == calcChecksum(state));
}

/// Calculate and set the checksum values for the internal state.
void IRDaikin64::checksum(void) { _.Sum = calcChecksum(_.raw); }

/// Reset the internal state to a fixed known good state.
void IRDaikin64::stateReset(void) { _.raw = kDaikin64KnownGoodState; }

/// Get a copy of the internal state as a valid code for this protocol.
/// @return A valid code for this protocol based on the current internal state.
uint64_t IRDaikin64::getRaw(void) {
  checksum();  // Ensure correct settings before sending.
  return _.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_state A valid code for this protocol.
void IRDaikin64::setRaw(const uint64_t new_state) { _.raw = new_state; }

/// Set the Power toggle setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin64::setPowerToggle(const bool on) { _.Power = on; }

/// Get the Power toggle setting of the A/C.
/// @return The current operating mode setting.
bool IRDaikin64::getPowerToggle(void) const { return _.Power; }

/// Set the temperature.
/// @param[in] temp The temperature in degrees celsius.
void IRDaikin64::setTemp(const uint8_t temp) {
  uint8_t degrees = std::max(temp, kDaikin64MinTemp);
  degrees = std::min(degrees, kDaikin64MaxTemp);
  _.Temp = uint8ToBcd(degrees);
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t IRDaikin64::getTemp(void) const { return bcdToUint8(_.Temp); }

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t IRDaikin64::getMode(void) const { return _.Mode; }

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void IRDaikin64::setMode(const uint8_t mode) {
  switch (mode) {
    case kDaikin64Fan:
    case kDaikin64Dry:
    case kDaikin64Cool:
    case kDaikin64Heat:
      _.Mode = mode;
      break;
    default:
      _.Mode = kDaikin64Cool;
  }
}

/// Convert a stdAc::opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRDaikin64::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kDry: return kDaikin64Dry;
    case stdAc::opmode_t::kFan: return kDaikin64Fan;
    case stdAc::opmode_t::kHeat: return kDaikin64Heat;
    default: return kDaikin64Cool;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::opmode_t IRDaikin64::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kDaikin64Cool: return stdAc::opmode_t::kCool;
    case kDaikin64Heat: return stdAc::opmode_t::kHeat;
    case kDaikin64Dry:  return stdAc::opmode_t::kDry;
    case kDaikin64Fan:  return stdAc::opmode_t::kFan;
    default: return stdAc::opmode_t::kAuto;
  }
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t IRDaikin64::getFan(void) const { return _.Fan; }

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
void IRDaikin64::setFan(const uint8_t speed) {
  switch (speed) {
    case kDaikin64FanQuiet:
    case kDaikin64FanTurbo:
    case kDaikin64FanAuto:
    case kDaikin64FanHigh:
    case kDaikin64FanMed:
    case kDaikin64FanLow:
      _.Fan = speed;
      break;
    default:
      _.Fan = kDaikin64FanAuto;
  }
}

/// Convert a stdAc::fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRDaikin64::convertFan(const stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin:    return kDaikin64FanQuiet;
    case stdAc::fanspeed_t::kLow:    return kDaikin64FanLow;
    case stdAc::fanspeed_t::kMedium: return kDaikin64FanMed;
    case stdAc::fanspeed_t::kHigh:   return kDaikin64FanHigh;
    case stdAc::fanspeed_t::kMax:    return kDaikin64FanTurbo;
    default:                         return kDaikin64FanAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::fanspeed_t IRDaikin64::toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kDaikin64FanTurbo: return stdAc::fanspeed_t::kMax;
    case kDaikin64FanHigh:  return stdAc::fanspeed_t::kHigh;
    case kDaikin64FanMed:   return stdAc::fanspeed_t::kMedium;
    case kDaikin64FanLow:   return stdAc::fanspeed_t::kLow;
    case kDaikinFanQuiet:   return stdAc::fanspeed_t::kMin;
    default:                return stdAc::fanspeed_t::kAuto;
  }
}

/// Get the Turbo (Powerful) mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin64::getTurbo(void) const { return _.Fan == kDaikin64FanTurbo; }

/// Set the Turbo (Powerful) mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin64::setTurbo(const bool on) {
  if (on) {
    setFan(kDaikin64FanTurbo);
  } else if (_.Fan == kDaikin64FanTurbo) {
     setFan(kDaikin64FanAuto);
  }
}

/// Get the Quiet mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin64::getQuiet(void) const { return _.Fan == kDaikin64FanQuiet; }

/// Set the Quiet mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin64::setQuiet(const bool on) {
  if (on) {
    setFan(kDaikin64FanQuiet);
  } else if (_.Fan == kDaikin64FanQuiet) {
     setFan(kDaikin64FanAuto);
  }
}

/// Set the Vertical Swing mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin64::setSwingVertical(const bool on) { _.SwingV = on; }

/// Get the Vertical Swing mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin64::getSwingVertical(void) const { return _.SwingV; }

/// Set the Sleep mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin64::setSleep(const bool on) { _.Sleep = on; }

/// Get the Sleep mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin64::getSleep(void) const { return _.Sleep; }

/// Set the clock on the A/C unit.
/// @param[in] mins_since_midnight Nr. of minutes past midnight.
void IRDaikin64::setClock(const uint16_t mins_since_midnight) {
  uint16_t mins = mins_since_midnight;
  if (mins_since_midnight >= 24 * 60) mins = 0;  // Bounds check.
  _.ClockMins = uint8ToBcd(mins % 60);
  _.ClockHours = uint8ToBcd(mins / 60);
}

/// Get the clock time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t IRDaikin64::getClock(void) const {
  return bcdToUint8(_.ClockHours) * 60 + bcdToUint8(_.ClockMins);
}

/// Set the enable status of the On Timer.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin64::setOnTimeEnabled(const bool on) { _.OnTimer = on; }

/// Get the enable status of the On Timer.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin64::getOnTimeEnabled(void) const { return _.OnTimer; }

/// Get the On Timer time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t IRDaikin64::getOnTime(void) const { return GETTIME(On); }

/// Set the On Timer time for the A/C unit.
/// @param[in] mins_since_midnight Nr. of minutes past midnight.
void IRDaikin64::setOnTime(const uint16_t mins_since_midnight) {
  SETTIME(On, mins_since_midnight);
}

/// Set the enable status of the Off Timer.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin64::setOffTimeEnabled(const bool on) { _.OffTimer = on; }

/// Get the enable status of the Off Timer.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin64::getOffTimeEnabled(void) const { return _.OffTimer; }

/// Get the Off Timer time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t IRDaikin64::getOffTime(void) const { return GETTIME(Off); }

/// Set the Off Timer time for the A/C unit.
/// @param[in] mins_since_midnight Nr. of minutes past midnight.
void IRDaikin64::setOffTime(const uint16_t mins_since_midnight) {
  SETTIME(Off, mins_since_midnight);
}

/// Convert the current internal state into a human readable string.
/// @return A human readable string.
String IRDaikin64::toString(void) const {
  String result = "";
  result.reserve(120);  // Reserve some heap for the string to reduce fragging.
  result += addBoolToString(_.Power, kPowerToggleStr, false);
  result += addModeToString(_.Mode, 0xFF, kDaikin64Cool,
                            kDaikin64Heat, kDaikin64Dry, kDaikin64Fan);
  result += addTempToString(getTemp());
  if (!getTurbo()) {
    result += addFanToString(_.Fan, kDaikin64FanHigh, kDaikin64FanLow,
                             kDaikin64FanAuto, kDaikin64FanQuiet,
                             kDaikin64FanMed);
  } else {
    result += addIntToString(_.Fan, kFanStr);
    result += kSpaceLBraceStr;
    result += kTurboStr;
    result += ')';
  }
  result += addBoolToString(getTurbo(), kTurboStr);
  result += addBoolToString(getQuiet(), kQuietStr);
  result += addBoolToString(_.SwingV, kSwingVStr);
  result += addBoolToString(_.Sleep, kSleepStr);
  result += addLabeledString(minsToString(getClock()), kClockStr);
  result += addLabeledString(_.OnTimer
                             ? minsToString(getOnTime()) : kOffStr,
                             kOnTimerStr);
  result += addLabeledString(_.OffTimer
                             ? minsToString(getOffTime()) : kOffStr,
                             kOffTimerStr);
  return result;
}

/// Convert the current internal state into its stdAc::state_t equivalent.
/// @param[in] prev Ptr to a previous state.
/// @return The stdAc equivalent of the native settings.
stdAc::state_t IRDaikin64::toCommon(const stdAc::state_t *prev) const {
  stdAc::state_t result{};
  if (prev != NULL) result = *prev;
  result.protocol = decode_type_t::DAIKIN64;
  result.model = -1;  // No models used.
  result.power ^= _.Power;
  result.mode = toCommonMode(_.Mode);
  result.celsius = true;
  result.degrees = getTemp();
  result.fanspeed = toCommonFanSpeed(_.Fan);
  result.swingv = _.SwingV ? stdAc::swingv_t::kAuto : stdAc::swingv_t::kOff;
  result.turbo = getTurbo();
  result.quiet = getQuiet();
  result.sleep = _.Sleep ? 0 : -1;
  result.clock = getClock();
  // Not supported.
  result.swingh = stdAc::swingh_t::kOff;
  result.clean = false;
  result.filter = false;
  result.beep = false;
  result.econo = false;
  result.light = false;
  return result;
}
