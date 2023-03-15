// Copyright 2016 sillyfrog
// Copyright 2017 sillyfrog, crankyoldgit
// Copyright 2018-2022 crankyoldgit
// Copyright 2019 pasna (IRDaikin160 class / Daikin176 class)

/// @file
/// @brief Support for 128-bit Daikin A/C protocols.
/// @see Daikin128 https://github.com/crankyoldgit/IRremoteESP8266/issues/827

// Supports:
//   Brand: Daikin,  Model: 17 Series FTXB09AXVJU A/C (DAIKIN128)
//   Brand: Daikin,  Model: 17 Series FTXB12AXVJU A/C (DAIKIN128)
//   Brand: Daikin,  Model: 17 Series FTXB24AXVJU A/C (DAIKIN128)
//   Brand: Daikin,  Model: BRC52B63 remote (DAIKIN128)

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

#if SEND_DAIKIN128
/// Send a Daikin128 (128-bit) A/C formatted message.
/// Status: STABLE / Known Working.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/827
void IRsend::sendDaikin128(const unsigned char data[], const uint16_t nbytes,
                           const uint16_t repeat) {
  if (nbytes < kDaikin128SectionLength)
    return;  // Not enough bytes to send a partial message.

  for (uint16_t r = 0; r <= repeat; r++) {
    enableIROut(kDaikin128Freq);
    // Leader
    for (uint8_t i = 0; i < 2; i++) {
      mark(kDaikin128LeaderMark);
      space(kDaikin128LeaderSpace);
    }
    // Section #1 (Header + Data)
    sendGeneric(kDaikin128HdrMark, kDaikin128HdrSpace, kDaikin128BitMark,
                kDaikin128OneSpace, kDaikin128BitMark, kDaikin128ZeroSpace,
                kDaikin128BitMark, kDaikin128Gap, data,
                kDaikin128SectionLength,
                kDaikin128Freq, false, 0, kDutyDefault);
    // Section #2 (Data + Footer)
    sendGeneric(0, 0, kDaikin128BitMark,
                kDaikin128OneSpace, kDaikin128BitMark, kDaikin128ZeroSpace,
                kDaikin128FooterMark, kDaikin128Gap,
                data + kDaikin128SectionLength,
                nbytes - kDaikin128SectionLength,
                kDaikin128Freq, false, 0, kDutyDefault);
  }
}
#endif  // SEND_DAIKIN128

/// Class constructor
/// @param[in] pin GPIO to be used when sending.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRDaikin128::IRDaikin128(const uint16_t pin, const bool inverted,
                         const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) { stateReset(); }

/// Set up hardware to be able to send a message.
void IRDaikin128::begin(void) { _irsend.begin(); }

uint8_t IRDaikin128::calcFirstChecksum(const uint8_t state[]) {
  return sumNibbles(state, kDaikin128SectionLength - 1,
                    state[kDaikin128SectionLength - 1] & 0x0F) & 0x0F;
}

uint8_t IRDaikin128::calcSecondChecksum(const uint8_t state[]) {
  return sumNibbles(state + kDaikin128SectionLength,
                    kDaikin128SectionLength - 1);
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool IRDaikin128::validChecksum(uint8_t state[]) {
  // Validate the checksum of section #1.
  if (state[kDaikin128SectionLength - 1] >> 4 != calcFirstChecksum(state))
    return false;
  // Validate the checksum of section #2
  if (state[kDaikin128StateLength - 1] != calcSecondChecksum(state))
    return false;
  return true;
}

/// Calculate and set the checksum values for the internal state.
void IRDaikin128::checksum(void) {
  _.Sum1 = calcFirstChecksum(_.raw);
  _.Sum2 = calcSecondChecksum(_.raw);
}

/// Reset the internal state to a fixed known good state.
void IRDaikin128::stateReset(void) {
  for (uint8_t i = 0; i < kDaikin128StateLength; i++) _.raw[i] = 0x00;
  _.raw[0] = 0x16;
  _.raw[7] = 0x04;  // Most significant nibble is a checksum.
  _.raw[8] = 0xA1;
  // _.raw[15] is a checksum byte, it will be set by checksum().
}

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *IRDaikin128::getRaw(void) {
  checksum();  // Ensure correct settings before sending.
  return _.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
void IRDaikin128::setRaw(const uint8_t new_code[]) {
  std::memcpy(_.raw, new_code, kDaikin128StateLength);
}

#if SEND_DAIKIN128
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRDaikin128::send(const uint16_t repeat) {
  _irsend.sendDaikin128(getRaw(), kDaikin128StateLength, repeat);
}
#endif  // SEND_DAIKIN128

/// Set the Power toggle setting of the A/C.
/// @param[in] toggle true, the setting is on. false, the setting is off.
void IRDaikin128::setPowerToggle(const bool toggle) { _.Power = toggle; }

/// Get the Power toggle setting of the A/C.
/// @return The current operating mode setting.
bool IRDaikin128::getPowerToggle(void) const { return _.Power; }

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t IRDaikin128::getMode(void) const { return _.Mode; }

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void IRDaikin128::setMode(const uint8_t mode) {
  switch (mode) {
    case kDaikin128Auto:
    case kDaikin128Cool:
    case kDaikin128Heat:
    case kDaikin128Fan:
    case kDaikin128Dry:
      _.Mode = mode;
      break;
    default:
      _.Mode = kDaikin128Auto;
      break;
  }
  // Force a reset of mode dependant things.
  setFan(getFan());  // Covers Quiet & Powerful too.
  setEcono(getEcono());
}

/// Convert a stdAc::opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRDaikin128::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kCool: return kDaikin128Cool;
    case stdAc::opmode_t::kHeat: return kDaikin128Heat;
    case stdAc::opmode_t::kDry: return kDaikinDry;
    case stdAc::opmode_t::kFan: return kDaikin128Fan;
    default: return kDaikin128Auto;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::opmode_t IRDaikin128::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kDaikin128Cool: return stdAc::opmode_t::kCool;
    case kDaikin128Heat: return stdAc::opmode_t::kHeat;
    case kDaikin128Dry: return stdAc::opmode_t::kDry;
    case kDaikin128Fan: return stdAc::opmode_t::kFan;
    default: return stdAc::opmode_t::kAuto;
  }
}

/// Set the temperature.
/// @param[in] temp The temperature in degrees celsius.
void IRDaikin128::setTemp(const uint8_t temp) {
  _.Temp = uint8ToBcd(std::min(kDaikin128MaxTemp,
                              std::max(temp, kDaikin128MinTemp)));
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t IRDaikin128::getTemp(void) const { return bcdToUint8(_.Temp); }

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t IRDaikin128::getFan(void) const { return _.Fan; }

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
void IRDaikin128::setFan(const uint8_t speed) {
  uint8_t new_speed = speed;
  uint8_t mode = _.Mode;
  switch (speed) {
    case kDaikin128FanQuiet:
    case kDaikin128FanPowerful:
      if (mode == kDaikin128Auto) new_speed = kDaikin128FanAuto;
      // FALL-THRU
    case kDaikin128FanAuto:
    case kDaikin128FanHigh:
    case kDaikin128FanMed:
    case kDaikin128FanLow:
      _.Fan = new_speed;
      break;
    default:
      _.Fan = kDaikin128FanAuto;
      return;
  }
}

/// Convert a stdAc::fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRDaikin128::convertFan(const stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin: return kDaikinFanQuiet;
    case stdAc::fanspeed_t::kLow: return kDaikin128FanLow;
    case stdAc::fanspeed_t::kMedium: return kDaikin128FanMed;
    case stdAc::fanspeed_t::kHigh: return kDaikin128FanHigh;
    case stdAc::fanspeed_t::kMax: return kDaikin128FanPowerful;
    default: return kDaikin128FanAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::fanspeed_t IRDaikin128::toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kDaikin128FanPowerful: return stdAc::fanspeed_t::kMax;
    case kDaikin128FanHigh: return stdAc::fanspeed_t::kHigh;
    case kDaikin128FanMed: return stdAc::fanspeed_t::kMedium;
    case kDaikin128FanLow: return stdAc::fanspeed_t::kLow;
    case kDaikinFanQuiet: return stdAc::fanspeed_t::kMin;
    default: return stdAc::fanspeed_t::kAuto;
  }
}

/// Set the Vertical Swing mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin128::setSwingVertical(const bool on) { _.SwingV = on; }

/// Get the Vertical Swing mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin128::getSwingVertical(void) const { return _.SwingV; }

/// Set the Sleep mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin128::setSleep(const bool on) { _.Sleep = on; }

/// Get the Sleep mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin128::getSleep(void) const { return _.Sleep; }

/// Set the Economy mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin128::setEcono(const bool on) {
  uint8_t mode = _.Mode;
  _.Econo = (on && (mode == kDaikin128Cool || mode == kDaikin128Heat));
}

/// Get the Economical mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin128::getEcono(void) const { return _.Econo; }

/// Set the Quiet mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin128::setQuiet(const bool on) {
  uint8_t mode = _.Mode;
  if (on && (mode == kDaikin128Cool || mode == kDaikin128Heat))
    setFan(kDaikin128FanQuiet);
  else if (_.Fan == kDaikin128FanQuiet)
    setFan(kDaikin128FanAuto);
}

/// Get the Quiet mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin128::getQuiet(void) const { return _.Fan == kDaikin128FanQuiet; }

/// Set the Powerful (Turbo) mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin128::setPowerful(const bool on) {
  uint8_t mode = _.Mode;
  if (on && (mode == kDaikin128Cool || mode == kDaikin128Heat))
    setFan(kDaikin128FanPowerful);
  else if (_.Fan == kDaikin128FanPowerful)
    setFan(kDaikin128FanAuto);
}

/// Get the Powerful (Turbo) mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin128::getPowerful(void) const {
  return _.Fan == kDaikin128FanPowerful;
}

/// Set the clock on the A/C unit.
/// @param[in] mins_since_midnight Nr. of minutes past midnight.
void IRDaikin128::setClock(const uint16_t mins_since_midnight) {
  uint16_t mins = mins_since_midnight;
  if (mins_since_midnight >= 24 * 60) mins = 0;  // Bounds check.
  // Hours.
  _.ClockHours = uint8ToBcd(mins / 60);
  // Minutes.
  _.ClockMins = uint8ToBcd(mins % 60);
}

/// Get the clock time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t IRDaikin128::getClock(void) const {
  return bcdToUint8(_.ClockHours) * 60 + bcdToUint8(_.ClockMins);
}

/// Set the enable status of the On Timer.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin128::setOnTimerEnabled(const bool on) { _.OnTimer = on; }

/// Get the enable status of the On Timer.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin128::getOnTimerEnabled(void) const { return _.OnTimer; }

#define SETTIME(x, n) do { \
  uint16_t mins = n;\
  if (n >= 24 * 60) mins = 0;\
  _.x##HalfHour = (mins % 60) >= 30;\
  _.x##Hours = uint8ToBcd(mins / 60);\
} while (0)

#define GETTIME(x) bcdToUint8(_.x##Hours) * 60 + (_.x##HalfHour ? 30 : 0)

/// Set the On Timer time for the A/C unit.
/// @param[in] mins_since_midnight Nr. of minutes past midnight.
void IRDaikin128::setOnTimer(const uint16_t mins_since_midnight) {
  SETTIME(On, mins_since_midnight);
}

/// Get the On Timer time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t IRDaikin128::getOnTimer(void) const { return GETTIME(On); }

/// Set the enable status of the Off Timer.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin128::setOffTimerEnabled(const bool on) { _.OffTimer = on; }

/// Get the enable status of the Off Timer.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin128::getOffTimerEnabled(void) const { return _.OffTimer; }

/// Set the Off Timer time for the A/C unit.
/// @param[in] mins_since_midnight Nr. of minutes past midnight.
void IRDaikin128::setOffTimer(const uint16_t mins_since_midnight) {
  SETTIME(Off, mins_since_midnight);
}

/// Get the Off Timer time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t IRDaikin128::getOffTimer(void) const { return GETTIME(Off); }

/// Set the Light toggle setting of the A/C.
/// @param[in] unit Device to show the LED (Light) Display info about.
/// @note 0 is off.
void IRDaikin128::setLightToggle(const uint8_t unit) {
  _.Ceiling = 0;
  _.Wall = 0;
  switch (unit) {
    case kDaikin128BitCeiling:
      _.Ceiling = 1;
      break;
    case kDaikin128BitWall:
      _.Wall = 1;
      break;
  }
}

/// Get the Light toggle setting of the A/C.
/// @return The current operating mode setting.
uint8_t IRDaikin128::getLightToggle(void) const {
  uint8_t code = 0;
  if (_.Ceiling) {
    code = kDaikin128BitCeiling;
  } else if (_.Wall) {
    code = kDaikin128BitWall;
  }

  return code;
}

/// Convert the current internal state into a human readable string.
/// @return A human readable string.
String IRDaikin128::toString(void) const {
  String result = "";
  result.reserve(240);  // Reserve some heap for the string to reduce fragging.
  result += addBoolToString(_.Power, kPowerToggleStr, false);
  result += addModeToString(_.Mode, kDaikin128Auto, kDaikin128Cool,
                            kDaikin128Heat, kDaikin128Dry, kDaikin128Fan);
  result += addTempToString(getTemp());
  result += addFanToString(_.Fan, kDaikin128FanHigh, kDaikin128FanLow,
                           kDaikin128FanAuto, kDaikin128FanQuiet,
                           kDaikin128FanMed);
  result += addBoolToString(getPowerful(), kPowerfulStr);
  result += addBoolToString(getQuiet(), kQuietStr);
  result += addBoolToString(_.SwingV, kSwingVStr);
  result += addBoolToString(_.Sleep, kSleepStr);
  result += addBoolToString(_.Econo, kEconoStr);
  result += addLabeledString(minsToString(getClock()), kClockStr);
  result += addBoolToString(_.OnTimer, kOnTimerStr);
  result += addLabeledString(minsToString(getOnTimer()), kOnTimerStr);
  result += addBoolToString(_.OffTimer, kOffTimerStr);
  result += addLabeledString(minsToString(getOffTimer()), kOffTimerStr);
  result += addIntToString(getLightToggle(), kLightToggleStr);
  result += kSpaceLBraceStr;
  switch (getLightToggle()) {
    case kDaikin128BitCeiling: result += kCeilingStr; break;
    case kDaikin128BitWall: result += kWallStr; break;
    case 0: result += kOffStr; break;
    default: result += kUnknownStr;
  }
  result += ')';
  return result;
}

/// Convert the current internal state into its stdAc::state_t equivalent.
/// @param[in] prev Ptr to a previous state.
/// @return The stdAc equivalent of the native settings.
stdAc::state_t IRDaikin128::toCommon(const stdAc::state_t *prev) const {
  stdAc::state_t result{};
  if (prev != NULL) result = *prev;
  result.protocol = decode_type_t::DAIKIN128;
  result.model = -1;  // No models used.
  result.power ^= _.Power;
  result.mode = toCommonMode(_.Mode);
  result.celsius = true;
  result.degrees = getTemp();
  result.fanspeed = toCommonFanSpeed(_.Fan);
  result.swingv = _.SwingV ? stdAc::swingv_t::kAuto : stdAc::swingv_t::kOff;
  result.quiet = getQuiet();
  result.turbo = getPowerful();
  result.econo = _.Econo;
  result.light ^= (getLightToggle() != 0);
  result.sleep = _.Sleep ? 0 : -1;
  result.clock = getClock();
  // Not supported.
  result.swingh = stdAc::swingh_t::kOff;
  result.clean = false;
  result.filter = false;
  result.beep = false;
  return result;
}

#if DECODE_DAIKIN128
/// Decode the supplied Daikin 128-bit message. (DAIKIN128)
/// Status: STABLE / Known Working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/827
bool IRrecv::decodeDaikin128(decode_results *results, uint16_t offset,
                             const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * (nbits + kHeader) + kFooter - 1 + offset)
    return false;
  if (nbits / 8 <= kDaikin128SectionLength) return false;

  // Compliance
  if (strict && nbits != kDaikin128Bits) return false;

  // Leader
  for (uint8_t i = 0; i < 2; i++) {
    if (!matchMark(results->rawbuf[offset++], kDaikin128LeaderMark,
                   kDaikinTolerance, kDaikinMarkExcess)) return false;
    if (!matchSpace(results->rawbuf[offset++], kDaikin128LeaderSpace,
                    kDaikinTolerance, kDaikinMarkExcess)) return false;
  }
  const uint16_t ksectionSize[kDaikin128Sections] = {
      kDaikin128SectionLength, (uint16_t)(nbits / 8 - kDaikin128SectionLength)};
  // Data Sections
  uint16_t pos = 0;
  for (uint8_t section = 0; section < kDaikin128Sections; section++) {
    uint16_t used;
    // Section Header (first section only) + Section Data (8 bytes) +
    //     Section Footer (Not for first section)
    used = matchGeneric(results->rawbuf + offset, results->state + pos,
                        results->rawlen - offset, ksectionSize[section] * 8,
                        section == 0 ? kDaikin128HdrMark : 0,
                        section == 0 ? kDaikin128HdrSpace : 0,
                        kDaikin128BitMark, kDaikin128OneSpace,
                        kDaikin128BitMark, kDaikin128ZeroSpace,
                        section > 0 ? kDaikin128FooterMark : kDaikin128BitMark,
                        kDaikin128Gap,
                        section > 0,
                        kDaikinTolerance, kDaikinMarkExcess, false);
    if (used == 0) return false;
    offset += used;
    pos += ksectionSize[section];
  }
  // Compliance
  if (strict) {
    if (!IRDaikin128::validChecksum(results->state)) return false;
  }

  // Success
  results->decode_type = decode_type_t::DAIKIN128;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // DECODE_DAIKIN128
