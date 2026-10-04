// Copyright 2026
/// @file
/// @brief Support for Islandaire PTAC A/C protocols.
/// @note The framing & timings are the same as TEKNOPOINT, so this protocol
///   must be decoded before it. The 0x22 x 5 preamble tells them apart.

#include "ir_Islandaire.h"
#include <algorithm>
#include <cstring>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRtext.h"
#include "IRutils.h"

// Constants
const uint16_t kIslandaireHdrMark = 3608;
const uint16_t kIslandaireHdrSpace = 1520;
const uint16_t kIslandaireBitMark = 560;
const uint16_t kIslandaireOneSpace = 1208;
const uint16_t kIslandaireZeroSpace = 408;
const uint16_t kIslandaireFreq = 38000;  // Hz.
const uint8_t kIslandaireExtraTolerance = 5;  // Extra tolerance percentage.

using irutils::addBoolToString;
using irutils::addFanToString;
using irutils::addLabeledString;
using irutils::addModeToString;
using irutils::addTempToString;
using irutils::minsToString;

#if SEND_ISLANDAIRE_AC
/// Send an Islandaire A/C formatted message.
/// Status: STABLE / Working on real device.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void IRsend::sendIslandaireAc(const uint8_t data[], const uint16_t nbytes,
                              const uint16_t repeat) {
  if (nbytes < kIslandaireStateLength) return;  // Not enough bytes to send.
  sendGeneric(kIslandaireHdrMark, kIslandaireHdrSpace,
              kIslandaireBitMark, kIslandaireOneSpace,
              kIslandaireBitMark, kIslandaireZeroSpace,
              kIslandaireBitMark, kDefaultMessageGap,
              data, nbytes, kIslandaireFreq, false, repeat, kDutyDefault);
}
#endif  // SEND_ISLANDAIRE_AC

#if DECODE_ISLANDAIRE_AC
/// Decode the supplied Islandaire A/C message.
/// Status: STABLE / Working on real device.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool IRrecv::decodeIslandaireAc(decode_results *results, uint16_t offset,
                                const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * nbits + kHeader + kFooter - 1 + offset)
    return false;  // Too short a message to match.
  if (strict && nbits != kIslandaireBits)
    return false;

  if (!matchGeneric(results->rawbuf + offset, results->state,
                    results->rawlen - offset, nbits,
                    kIslandaireHdrMark, kIslandaireHdrSpace,
                    kIslandaireBitMark, kIslandaireOneSpace,
                    kIslandaireBitMark, kIslandaireZeroSpace,
                    kIslandaireBitMark, kDefaultMessageGap,
                    true, _tolerance + kIslandaireExtraTolerance,
                    kMarkExcess, false)) return false;
  // Compliance
  for (uint8_t i = 0; i < kIslandairePreambleLength; i++)
    if (results->state[i] != kIslandairePreamble) return false;
  if (strict && !IRIslandaireAc::validChecksum(results->state, nbits / 8))
    return false;
  // Success
  results->decode_type = decode_type_t::ISLANDAIRE_AC;
  results->bits = nbits;
  return true;
}
#endif  // DECODE_ISLANDAIRE_AC

/// Class constructor
/// @param[in] pin GPIO to be used when sending.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRIslandaireAc::IRIslandaireAc(const uint16_t pin, const bool inverted,
                               const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) { stateReset(); }

/// Reset the internal state to a fixed known good state.
/// @note Power on, Cool, 72F, Fan Auto, Timer off.
void IRIslandaireAc::stateReset(void) {
  static const uint8_t kReset[kIslandaireStateLength] = {
      0x22, 0x22, 0x22, 0x22, 0x22, 0x24, 0x03, 0x48, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00};
  setRaw(kReset);
  checksum();
}

/// Set up hardware to be able to send a message.
void IRIslandaireAc::begin(void) { _irsend.begin(); }

#if SEND_ISLANDAIRE_AC
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRIslandaireAc::send(const uint16_t repeat) {
  _irsend.sendIslandaireAc(getRaw(), kIslandaireStateLength, repeat);
}
#endif  // SEND_ISLANDAIRE_AC

/// Calculate the checksum for a given state.
/// @param[in] state The array to calculate the checksum of.
/// @param[in] length The length/size of the array.
/// @return The calculated checksum value.
uint8_t IRIslandaireAc::calcChecksum(const uint8_t state[],
                                     const uint16_t length) {
  if (length == 0) return 0;
  return sumBytes(state, length - 1);
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length/size of the array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool IRIslandaireAc::validChecksum(const uint8_t state[],
                                   const uint16_t length) {
  if (length < kIslandaireStateLength) return false;
  return calcChecksum(state, length) == state[length - 1];
}

/// Calculate & set the checksum for the current internal state.
void IRIslandaireAc::checksum(void) {
  _.Sum = calcChecksum(_.raw, kIslandaireStateLength);
}

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t* IRIslandaireAc::getRaw(void) {
  checksum();
  return _.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
/// @param[in] length The length/size of the new_code array.
void IRIslandaireAc::setRaw(const uint8_t new_code[], const uint16_t length) {
  std::memcpy(_.raw, new_code, std::min(length, kIslandaireStateLength));
}

/// Set the requested power state of the A/C to on.
void IRIslandaireAc::on(void) { setPower(true); }

/// Set the requested power state of the A/C to off.
void IRIslandaireAc::off(void) { setPower(false); }

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRIslandaireAc::setPower(const bool on) { _.Power = on; }

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool IRIslandaireAc::getPower(void) const { return _.Power; }

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
/// @note Unknown modes default to Cool.
void IRIslandaireAc::setMode(const uint8_t mode) {
  switch (mode) {
    case kIslandaireHeat:
    case kIslandaireCool:
    case kIslandaireFan:
      _.Mode = mode;
      break;
    default:
      _.Mode = kIslandaireCool;
  }
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t IRIslandaireAc::getMode(void) const { return _.Mode; }

/// Set the temperature.
/// @param[in] degrees The temperature in degrees.
/// @param[in] fahrenheit true, if `degrees` is in Fahrenheit, else Celsius.
/// @note The A/C works in whole degrees Fahrenheit (61-88F).
void IRIslandaireAc::setTemp(const float degrees, const bool fahrenheit) {
  float temp_f = fahrenheit ? degrees : celsiusToFahrenheit(degrees);
  temp_f = std::max(static_cast<float>(kIslandaireMinTempF), temp_f);
  temp_f = std::min(static_cast<float>(kIslandaireMaxTempF), temp_f);
  _.Temp = static_cast<uint8_t>(temp_f + 0.5);
}

/// Get the current temperature setting.
/// @param[in] fahrenheit true, to return Fahrenheit, else Celsius.
/// @return The current setting for temp. in the requested units.
float IRIslandaireAc::getTemp(const bool fahrenheit) const {
  return fahrenheit ? _.Temp : fahrenheitToCelsius(_.Temp);
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
/// @note Unknown speeds default to Auto.
void IRIslandaireAc::setFan(const uint8_t speed) {
  switch (speed) {
    case kIslandaireFanAuto:
    case kIslandaireFanLow:
    case kIslandaireFanHigh:
      _.Fan = speed;
      break;
    default:
      _.Fan = kIslandaireFanAuto;
  }
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t IRIslandaireAc::getFan(void) const { return _.Fan; }

/// Set the auto-off timer.
/// @param[in] hours Nr. of hours until the A/C turns off. 0 disables it.
/// @note Values above 12 hours are capped at 12.
void IRIslandaireAc::setTimer(const uint8_t hours) {
  const uint8_t h = std::min(hours, kIslandaireTimerMax);
  _.TimerHours = h;
  _.TimerActive = (h > 0);
  _.TimerActive2 = (h > 0);
}

/// Get the auto-off timer setting.
/// @return Nr. of hours until the A/C turns off. 0 means disabled.
uint8_t IRIslandaireAc::getTimer(void) const {
  return _.TimerActive ? _.TimerHours : 0;
}

/// Convert a stdAc::opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRIslandaireAc::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kHeat: return kIslandaireHeat;
    case stdAc::opmode_t::kFan:  return kIslandaireFan;
    default:                     return kIslandaireCool;
  }
}

/// Convert a stdAc::fanspeed_t enum into its native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRIslandaireAc::convertFan(const stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin:
    case stdAc::fanspeed_t::kLow:    return kIslandaireFanLow;
    case stdAc::fanspeed_t::kMedium:
    case stdAc::fanspeed_t::kHigh:
    case stdAc::fanspeed_t::kMax:    return kIslandaireFanHigh;
    default:                         return kIslandaireFanAuto;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::opmode_t IRIslandaireAc::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kIslandaireHeat: return stdAc::opmode_t::kHeat;
    case kIslandaireFan:  return stdAc::opmode_t::kFan;
    default:              return stdAc::opmode_t::kCool;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::fanspeed_t IRIslandaireAc::toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kIslandaireFanLow:  return stdAc::fanspeed_t::kLow;
    case kIslandaireFanHigh: return stdAc::fanspeed_t::kHigh;
    default:                 return stdAc::fanspeed_t::kAuto;
  }
}

/// Convert the current internal state into its stdAc::state_t equivalent.
/// @return The stdAc equivalent of the native settings.
stdAc::state_t IRIslandaireAc::toCommon(void) const {
  stdAc::state_t result{};
  result.protocol = decode_type_t::ISLANDAIRE_AC;
  result.model = -1;  // Not supported.
  result.power = getPower();
  result.mode = toCommonMode(_.Mode);
  result.celsius = false;
  result.degrees = _.Temp;
  result.fanspeed = toCommonFanSpeed(_.Fan);
  // Not supported.
  result.swingv = stdAc::swingv_t::kOff;
  result.swingh = stdAc::swingh_t::kOff;
  result.quiet = false;
  result.turbo = false;
  result.econo = false;
  result.light = false;
  result.filter = false;
  result.clean = false;
  result.beep = false;
  result.sleep = -1;
  result.clock = -1;
  return result;
}

/// Convert the current internal state into a human readable string.
/// @return A human readable string.
String IRIslandaireAc::toString(void) const {
  String result = "";
  result.reserve(80);  // Reserve some heap for the string to reduce fragging.
  result += addBoolToString(_.Power, kPowerStr, false);
  result += addModeToString(_.Mode, 0xFF, kIslandaireCool, kIslandaireHeat,
                            0xFF, kIslandaireFan);
  result += addTempToString(_.Temp, false);
  result += addFanToString(_.Fan, kIslandaireFanHigh, kIslandaireFanLow,
                           kIslandaireFanAuto, 0xFF, 0xFF);
  const uint8_t timer = getTimer();
  result += addLabeledString(timer ? minsToString(timer * 60) : kOffStr,
                             kOffTimerStr);
  return result;
}
