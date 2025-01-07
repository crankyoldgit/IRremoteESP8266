// Copyright 2021 Davide Depau
// Copyright 2022 David Conran

/// @file
/// @brief Support for Kelon AC protocols.
/// Both sending and decoding should be functional for models of series
/// KELON ON/OFF 9000-12000.
/// All features of the standard remote are implemented.
///
/// @note Unsupported:
///    - Explicit on/off due to AC unit limitations
///    - Explicit swing position due to AC unit limitations
///    - Fahrenheit.

#include <algorithm>
#include <cassert>

#include "ir_Kelon.h"

#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"
#include "IRtext.h"


using irutils::addBoolToString;
using irutils::addIntToString;
using irutils::addSignedIntToString;
using irutils::addModeToString;
using irutils::addFanToString;
using irutils::addTempToString;
using irutils::addLabeledString;
using irutils::minsToString;

// Constants
const uint16_t kKelonHdrMark = 9000;
const uint16_t kKelonHdrSpace = 4600;
const uint16_t kKelonBitMark = 560;
const uint16_t kKelonOneSpace = 1680;
const uint16_t kKelonZeroSpace = 600;
const uint32_t kKelonGap = 2 * kDefaultMessageGap;
const uint16_t kKelonFreq = 38000;

const uint32_t kKelon168FooterSpace = 8000;
const uint16_t kKelon168Section1Size = 6;
const uint16_t kKelon168Section2Size = 8;
const uint16_t kKelon168Section3Size = 7;

#if SEND_KELON
/// Send a Kelon 48-bit message.
/// Status: STABLE / Working.
/// @param[in] data The data to be transmitted.
/// @param[in] nbits Nr. of bits of data to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void IRsend::sendKelon(const uint64_t data, const uint16_t nbits,
                       const uint16_t repeat) {
  sendGeneric(kKelonHdrMark, kKelonHdrSpace,
              kKelonBitMark, kKelonOneSpace,
              kKelonBitMark, kKelonZeroSpace,
              kKelonBitMark, kKelonGap,
              data, nbits, kKelonFreq, false,  // LSB First.
              repeat, kDutyDefault);
}
#endif  // SEND_KELON

#if DECODE_KELON
/// Decode the supplied Kelon 48-bit message.
/// Status: STABLE / Working.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return True if it can decode it, false if it can't.
bool IRrecv::decodeKelon(decode_results *results, uint16_t offset,
                         const uint16_t nbits, const bool strict) {
  if (strict && nbits != kKelonBits) return false;

  if (!matchGeneric(results->rawbuf + offset, &(results->value),
                    results->rawlen - offset, nbits,
                    kKelonHdrMark, kKelonHdrSpace,
                    kKelonBitMark, kKelonOneSpace,
                    kKelonBitMark, kKelonZeroSpace,
                    kKelonBitMark, kKelonGap, true,
                    _tolerance, 0, false)) return false;

  results->decode_type = decode_type_t::KELON;
  results->address = 0;
  results->command = 0;
  results->bits = nbits;
  return true;
}
#endif  // DECODE_KELON

/// Class constructor
/// @param[in] pin GPIO to be used when sending.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRKelonAc::IRKelonAc(const uint16_t pin, const bool inverted,
                     const bool use_modulation)
    : _irsend{pin, inverted, use_modulation}, _{} { stateReset(); }

/// Reset the internals of the object to a known good state.
void IRKelonAc::stateReset() {
  _.raw = 0L;
  _.preamble[0] = 0b10000011;
  _.preamble[1] = 0b00000110;
}

#if SEND_KELON

/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRKelonAc::send(const uint16_t repeat) {
  _irsend.sendKelon(getRaw(), kKelonBits, repeat);

  // Reset toggle flags
  _.PowerToggle = false;
  _.SwingVToggle = false;

  // Remove the timer time setting
  _.TimerHours = 0;
  _.TimerHalfHour = 0;
}

/// Ensures the AC is on or off by exploiting the fact that setting
/// it to "smart" will always turn it on if it's off.
/// This method will send 2 commands to the AC to do the trick
/// @param[in] on Whether to ensure the AC is on or off
void IRKelonAc::ensurePower(bool on) {
  // Try to avoid turning on the compressor for this operation.
  // "Dry grade", when in "smart" mode, acts as a temperature offset that
  // the user can configure if they feel too cold or too hot. By setting it
  // to +2 we're setting the temperature to ~28°C, which will effectively
  // set the AC to fan mode.
  int8_t previousDry = getDryGrade();
  setDryGrade(2);
  setMode(kKelonModeSmart);
  send();

  setDryGrade(previousDry);
  setMode(_previousMode);
  send();

  // Now we're sure it's on. Turn it back off. The AC seems to turn back on if
  // we don't send this separately
  if (!on) {
    setTogglePower(true);
    send();
  }
}

#endif  // SEND_KELON

/// Set up hardware to be able to send a message.
void IRKelonAc::begin() { _irsend.begin(); }

/// Request toggling power - will be reset to false after sending
/// @param[in] toggle Whether to toggle the power state
void IRKelonAc::setTogglePower(const bool toggle) { _.PowerToggle = toggle; }

/// Get whether toggling power will be requested
/// @return The power toggle state
bool IRKelonAc::getTogglePower() const { return _.PowerToggle; }

/// Set the temperature setting.
/// @param[in] degrees The temperature in degrees celsius.
void IRKelonAc::setTemp(const uint8_t degrees) {
  uint8_t temp = std::max(kKelonMinTemp, degrees);
  temp = std::min(kKelonMaxTemp, temp);
  _previousTemp = _.Temperature;
  _.Temperature = temp - kKelonMinTemp;
}

/// Get the current temperature setting.
/// @return Get current setting for temp. in degrees celsius.
uint8_t IRKelonAc::getTemp() const { return _.Temperature + kKelonMinTemp; }

/// Set the speed of the fan.
/// @param[in] speed 0 is auto, 1-5 is the speed
void IRKelonAc::setFan(const uint8_t speed) {
  uint8_t fan = std::min(speed, kKelonFanMax);

  _previousFan = _.Fan;
  // Note: Kelon fan speeds are backwards! This code maps the range 0,1:3 to
  // 0,3:1 to save the API's user's sanity.
  _.Fan = ((static_cast<int16_t>(fan) - 4) * -1) % 4;
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t IRKelonAc::getFan() const {
  return ((static_cast<int16_t>(_.Fan) - 4) * -1) % 4;;
}

/// Set the dehumidification intensity.
/// @param[in] grade has to be in the range [-2 : +2]
void IRKelonAc::setDryGrade(const int8_t grade) {
  int8_t drygrade = std::max(kKelonDryGradeMin, grade);
  drygrade = std::min(kKelonDryGradeMax, drygrade);

  // Two's complement is clearly too bleeding edge for this manufacturer
  uint8_t outval;
  if (drygrade < 0)
    outval = 0b100 | (-drygrade & 0b011);
  else
    outval = drygrade & 0b011;
  _.DehumidifierGrade = outval;
}

/// Get the current dehumidification intensity setting. In smart mode, this
/// controls the temperature adjustment.
/// @return The current dehumidification intensity.
int8_t IRKelonAc::getDryGrade() const {
  return static_cast<int8_t>(_.DehumidifierGrade & 0b011) *
         ((_.DehumidifierGrade & 0b100) ? -1 : 1);
}

/// Set the desired operation mode.
/// @param[in] mode The desired operation mode.
void IRKelonAc::setMode(const uint8_t mode) {
  if (_.Mode == kKelonModeSmart || _.Mode == kKelonModeFan ||
      _.Mode == kKelonModeDry) {
    _.Temperature = _previousTemp;
  }
  if (_.SuperCoolEnabled1) {
    // Cancel supercool
    _.SuperCoolEnabled1 = false;
    _.SuperCoolEnabled2 = false;
    _.Temperature = _previousTemp;
    _.Fan = _previousFan;
  }
  _previousMode = _.Mode;

  switch (mode) {
    case kKelonModeSmart:
      setTemp(26);
      _.SmartModeEnabled = true;
      _.Mode = mode;
      break;
    case kKelonModeDry:
    case kKelonModeFan:
      setTemp(25);
      // fallthrough
    case kKelonModeCool:
    case kKelonModeHeat:
      _.Mode = mode;
      // fallthrough
    default:
      _.SmartModeEnabled = false;
  }
}

/// Get the current operation mode setting.
/// @return The current operation mode.
uint8_t IRKelonAc::getMode() const { return _.Mode; }

/// Request toggling the vertical swing - will be reset to false after sending
/// @param[in] toggle If true, the swing mode will be toggled when sent.
void IRKelonAc::setToggleSwingVertical(const bool toggle) {
  _.SwingVToggle = toggle;
}

/// Get whether the swing mode is set to be toggled
/// @return Whether the toggle bit is set
bool IRKelonAc::getToggleSwingVertical() const { return _.SwingVToggle; }

/// Control the current sleep (quiet) setting.
/// @param[in] on The desired setting.
void IRKelonAc::setSleep(const bool on) { _.SleepEnabled = on; }

/// Is the sleep setting on?
/// @return The current value.
bool IRKelonAc::getSleep() const { return _.SleepEnabled; }

/// Control the current super cool mode setting.
/// @param[in] on The desired setting.
void IRKelonAc::setSupercool(const bool on) {
  if (on) {
    setTemp(kKelonMinTemp);
    setMode(kKelonModeCool);
    setFan(kKelonFanMax);
  } else {
    // All reverts to previous are handled by setMode as needed
    setMode(_previousMode);
  }
  _.SuperCoolEnabled1 = on;
  _.SuperCoolEnabled2 = on;
}

/// Is the super cool mode setting on?
/// @return The current value.
bool IRKelonAc::getSupercool() const { return _.SuperCoolEnabled1; }

/// Set the timer time and enable it. Timer is an off timer if the unit is on,
/// it is an on timer if the unit is off.
/// Only multiples of 30m are supported for < 10h, then only multiples of 60m
/// @param[in] mins Nr. of minutes
void IRKelonAc::setTimer(uint16_t mins) {
  const uint16_t minutes = std::min(static_cast<int>(mins), 24 * 60);

  if (minutes / 60 >= 10) {
    uint8_t hours = minutes / 60 + 10;
    _.TimerHalfHour = hours & 1;
    _.TimerHours = hours >> 1;
  } else {
    _.TimerHalfHour = (minutes % 60) >= 30 ? 1 : 0;
    _.TimerHours = minutes / 60;
  }

  setTimerEnabled(true);
}

/// Get the set timer. Timer set time is deleted once the command is sent, so
/// calling this after send() will return 0.
/// The AC unit will continue keeping track of the remaining time unless it is
/// later disabled.
/// @return The timer set minutes
uint16_t IRKelonAc::getTimer() const {
  if (_.TimerHours >= 10)
    return (static_cast<uint16_t>((_.TimerHours << 1) | _.TimerHalfHour) -
            10) * 60;
  return static_cast<uint16_t>(_.TimerHours) * 60 + (_.TimerHalfHour ? 30 : 0);
}

/// Enable or disable the timer. Note that in order to enable the timer the
/// minutes must be set with setTimer().
/// @param[in] on Whether to enable or disable the timer
void IRKelonAc::setTimerEnabled(bool on) { _.TimerEnabled = on; }

/// Get the current timer status
/// @return Whether the timer is enabled.
bool IRKelonAc::getTimerEnabled() const { return _.TimerEnabled; }

/// Get the raw state of the object, suitable to be sent with the appropriate
/// IRsend object method.
/// @return A PTR to the internal state.
uint64_t IRKelonAc::getRaw() const { return _.raw; }

/// Set the raw state of the object.
/// @param[in] new_code The raw state from the native IR message.
void IRKelonAc::setRaw(const uint64_t new_code) { _.raw = new_code; }

/// Convert a standard A/C mode (stdAc::opmode_t) into it a native mode.
/// @param[in] mode A stdAc::opmode_t operation mode.
/// @return The native mode equivalent.
uint8_t IRKelonAc::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kCool: return kKelonModeCool;
    case stdAc::opmode_t::kHeat: return kKelonModeHeat;
    case stdAc::opmode_t::kDry:  return kKelonModeDry;
    case stdAc::opmode_t::kFan:  return kKelonModeFan;
    default:                     return kKelonModeSmart;  // aka Auto.
  }
}

/// Convert a standard A/C fan speed (stdAc::fanspeed_t) into it a native speed.
/// @param[in] fan A stdAc::fanspeed_t fan speed
/// @return The native speed equivalent.
uint8_t IRKelonAc::convertFan(stdAc::fanspeed_t fan) {
  switch (fan) {
    case stdAc::fanspeed_t::kMin:
    case stdAc::fanspeed_t::kLow:    return kKelonFanMin;
    case stdAc::fanspeed_t::kMedium: return kKelonFanMedium;
    case stdAc::fanspeed_t::kHigh:
    case stdAc::fanspeed_t::kMax:    return kKelonFanMax;
    default:                         return kKelonFanAuto;
  }
}

/// Convert a native mode to it's stdAc::opmode_t equivalent.
/// @param[in] mode A native operating mode value.
/// @return The stdAc::opmode_t equivalent.
stdAc::opmode_t IRKelonAc::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kKelonModeCool: return stdAc::opmode_t::kCool;
    case kKelonModeHeat: return stdAc::opmode_t::kHeat;
    case kKelonModeDry:  return stdAc::opmode_t::kDry;
    case kKelonModeFan:  return stdAc::opmode_t::kFan;
    default:             return stdAc::opmode_t::kAuto;
  }
}

/// Convert a native fan speed to it's stdAc::fanspeed_t equivalent.
/// @param[in] speed A native fan speed value.
/// @return The stdAc::fanspeed_t equivalent.
stdAc::fanspeed_t IRKelonAc::toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kKelonFanMin:    return stdAc::fanspeed_t::kLow;
    case kKelonFanMedium: return stdAc::fanspeed_t::kMedium;
    case kKelonFanMax:    return stdAc::fanspeed_t::kHigh;
    default:              return stdAc::fanspeed_t::kAuto;
  }
}

/// Convert the internal A/C object state to it's stdAc::state_t equivalent.
/// @return A stdAc::state_t containing the current settings.
stdAc::state_t IRKelonAc::toCommon(const stdAc::state_t *prev) const {
  stdAc::state_t result{};
  result.protocol = decode_type_t::KELON;
  result.model = -1;  // Unused.
  // AC only supports toggling it
  result.power = (prev == nullptr || prev->power) ^ _.PowerToggle;
  result.mode = toCommonMode(getMode());
  result.celsius = true;
  result.degrees = getTemp();
  result.fanspeed = toCommonFanSpeed(getFan());
  // AC only supports toggling it
  result.swingv = stdAc::swingv_t::kAuto;
  if (prev != nullptr &&
      (prev->swingv != stdAc::swingv_t::kAuto) ^ _.SwingVToggle)
    result.swingv = stdAc::swingv_t::kOff;
  result.turbo = getSupercool();
  result.sleep = getSleep() ? 0 : -1;
  // Not supported.
  result.swingh = stdAc::swingh_t::kOff;
  result.light = true;
  result.beep = true;
  result.quiet = false;
  result.filter = false;
  result.clean = false;
  result.econo = false;
  result.clock = -1;
  return result;
}

/// Convert the internal settings into a human readable string.
/// @return A String.
String IRKelonAc::toString() const {
  String result = "";
  // Reserve some heap for the string to reduce fragging.
  result.reserve(160);
  result += addTempToString(getTemp(), true, false);
  result += addModeToString(_.Mode, kKelonModeSmart, kKelonModeCool,
                            kKelonModeHeat, kKelonModeDry, kKelonModeFan);
  result += addFanToString(_.Fan, kKelonFanMax, kKelonFanMin, kKelonFanAuto,
                           -1, kKelonFanMedium, kKelonFanMax);
  result += addBoolToString(_.SleepEnabled, kSleepStr);
  result += addSignedIntToString(getDryGrade(), kDryStr);
  result += addLabeledString(
      getTimerEnabled() ? (getTimer() > 0 ? minsToString(getTimer()) : kOnStr)
                        : kOffStr,
      kTimerStr);
  result += addBoolToString(getSupercool(), kTurboStr);
  if (getTogglePower())
    result += addBoolToString(true, kPowerToggleStr);
  if (getToggleSwingVertical())
    result += addBoolToString(true, kSwingVToggleStr);
  return result;
}

// KELON168

#if SEND_KELON168
/// Send a Kelon 168 bit / 21 byte message.
/// Status: BETA / Probably works.
/// @param[in] data The data to be transmitted.
/// @param[in] nbytes Nr. of bytes of data to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void IRsend::sendKelon168(const uint8_t data[], const uint16_t nbytes,
                          const uint16_t repeat) {
  assert(kKelon168StateLength == kKelon168Section1Size + kKelon168Section2Size +
                                 kKelon168Section3Size);
  // Enough bytes to send a proper message?
  if (nbytes < kKelon168StateLength) return;

  for (uint16_t r = 0; r <= repeat; r++) {
    // Section #1 (48 bits)
    sendGeneric(kKelonHdrMark, kKelonHdrSpace,
                kKelonBitMark, kKelonOneSpace,
                kKelonBitMark, kKelonZeroSpace,
                kKelonBitMark, kKelon168FooterSpace,
                data, kKelon168Section1Size, kKelonFreq, false,  // LSB First.
                0,  // No repeats here
                kDutyDefault);
    // Section #2 (64 bits)
    sendGeneric(0, 0,
                kKelonBitMark, kKelonOneSpace,
                kKelonBitMark, kKelonZeroSpace,
                kKelonBitMark, kKelon168FooterSpace,
                data + kKelon168Section1Size, kKelon168Section2Size,
                kKelonFreq, false,  // LSB First.
                0,  // No repeats here
                kDutyDefault);
    // Section #3 (56 bits)
    sendGeneric(0, 0,
                kKelonBitMark, kKelonOneSpace,
                kKelonBitMark, kKelonZeroSpace,
                kKelonBitMark, kKelonGap,
                data + kKelon168Section1Size + kKelon168Section2Size,
                nbytes - (kKelon168Section1Size + kKelon168Section2Size),
                kKelonFreq, false,  // LSB First.
                0,  // No repeats here
                kDutyDefault);
  }
}
#endif  // SEND_KELON168

#if DECODE_KELON168
/// Decode the supplied Kelon 168 bit / 21 byte message.
/// Status: BETA / Probably Working.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return True if it can decode it, false if it can't.
bool IRrecv::decodeKelon168(decode_results *results, uint16_t offset,
                            const uint16_t nbits, const bool strict) {
  if (strict && nbits != kKelon168Bits) return false;
  if (results->rawlen <= 2 * nbits + kHeader + kFooter * 2 - 1 + offset)
    return false;  // Can't possibly be a valid Kelon 168 bit message.

  uint16_t used = 0;

  used = matchGeneric(results->rawbuf + offset, results->state,
                      results->rawlen - offset, kKelon168Section1Size * 8,
                      kKelonHdrMark, kKelonHdrSpace,
                      kKelonBitMark, kKelonOneSpace,
                      kKelonBitMark, kKelonZeroSpace,
                      kKelonBitMark, kKelon168FooterSpace,
                      false, _tolerance, 0, false);
  if (!used) return false;  // Failed to match.
  offset += used;

  used = matchGeneric(results->rawbuf + offset,
                      results->state + kKelon168Section1Size,
                      results->rawlen - offset, kKelon168Section2Size * 8,
                      0, 0,
                      kKelonBitMark, kKelonOneSpace,
                      kKelonBitMark, kKelonZeroSpace,
                      kKelonBitMark, kKelon168FooterSpace,
                      false, _tolerance, 0, false);
  if (!used) return false;  // Failed to match.
  offset += used;

  used = matchGeneric(results->rawbuf + offset,
                      results->state + (kKelon168Section1Size +
                                        kKelon168Section2Size),
                      results->rawlen - offset,
                      nbits - (kKelon168Section1Size +
                               kKelon168Section2Size) * 8,
                      0, 0,
                      kKelonBitMark, kKelonOneSpace,
                      kKelonBitMark, kKelonZeroSpace,
                      kKelonBitMark, kKelonGap,
                      true, _tolerance, 0, false);
  if (!used) return false;  // Failed to match.
  results->decode_type = decode_type_t::KELON168;
  results->bits = nbits;
  return true;
}
#endif

// Constants


using irutils::addBoolToString;
using irutils::addFanToString;
using irutils::addIntToString;
using irutils::addLabeledString;
using irutils::addModeToString;
using irutils::addModelToString;
using irutils::addTempToString;
using irutils::minsToString;

#define GETTIME(x) (_.x##Hours * 60 + _.x##Mins)
#define SETTIME(x, n) do { \
  uint16_t mins = n;\
  _.x##Hours = (mins / 60) % 24;\
  _.x##Mins = mins % 60;\
} while (0)

// Class for emulating a Kelon/Hisense (168bit) A/C remote.

/// Class constructor
/// @param[in] pin GPIO to be used when sending.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRKelon168Ac::IRKelon168Ac(const uint16_t pin, const bool inverted,
                             const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) { stateReset(); }

/// Reset the state of the remote to a known good state/sequence.
void IRKelon168Ac::stateReset(void) {
  for (uint8_t i = 2; i < kKelon168StateLength; i++) _.raw[i] = 0x0;
  _.raw[0]  = 0x83;
  _.raw[1]  = 0x06;
  _.raw[6]  = 0x80;
  _.raw[18] = 0x28;             // Remote model, device off
  _setTemp(kKelon168AutoTemp);  // Default to a sane value, 23C
}

/// Set up hardware to be able to send a message.
void IRKelon168Ac::begin(void) { _irsend.begin(); }

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length/size of the array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool IRKelon168Ac::validChecksum(const uint8_t state[],
                                  const uint16_t length) {
  if (length > kKelon168ChecksumByte1 &&
      state[kKelon168ChecksumByte1] !=
          xorBytes(state + 2, kKelon168ChecksumByte1 - 1 - 2)) {
    DPRINTLN("DEBUG: First Kelon168 checksum failed.");
    return false;
  }
  if (length > kKelon168ChecksumByte2 &&
      state[kKelon168ChecksumByte2] !=
          xorBytes(state + kKelon168ChecksumByte1 + 1,
                   kKelon168ChecksumByte2 - kKelon168ChecksumByte1 - 1)) {
    DPRINTLN("DEBUG: Second Kelon168 checksum failed.");
    return false;
  }
  // State is too short to have a checksum or everything checked out.
  return true;
}

/// Calculate & set the checksum for the current internal state of the remote.
/// @param[in] length The length/size of the internal state array.
void IRKelon168Ac::checksum(uint16_t length) {
  if (length >= kKelon168ChecksumByte1)
    _.Sum1 = xorBytes(_.raw + 2, kKelon168ChecksumByte1 - 1 - 2);
  if (length >= kKelon168ChecksumByte2)
    _.Sum2 = xorBytes(_.raw + kKelon168ChecksumByte1 + 1,
                 kKelon168ChecksumByte2 - kKelon168ChecksumByte1 - 1);
}

#if SEND_KELON168
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
/// @param[in] calcChecksum Do we need to calculate the checksum?.
void IRKelon168Ac::send(const uint16_t repeat, const bool calcChecksum) {
  _irsend.sendKelon168(getRaw(calcChecksum), kKelon168StateLength,
                          repeat);
}
#endif  // SEND_KELON168

/// Get a copy of the internal state/code for this protocol.
/// @param[in] calcChecksum Do we need to calculate the checksum?.
/// @return A code for this protocol based on the current internal state.
uint8_t *IRKelon168Ac::getRaw(const bool calcChecksum) {
  if (calcChecksum) checksum();
  return _.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] newCode A valid code for this protocol.
/// @param[in] length The length/size of the newCode array.
void IRKelon168Ac::setRaw(const uint8_t newCode[], const uint16_t length) {
  std::memcpy(_.raw, newCode, std::min(length, kKelon168StateLength));
}

/// Get/Detect the model of the A/C. Actually only one remote type is supported.
/// @return The enum of the compatible model.
kelon168_ac_remote_model_t IRKelon168Ac::getModel(void) const {
  return DG11R201;
}

/// Set the model of the A/C to emulate.
/// @param[in] model The enum of the appropriate model.
void IRKelon168Ac::setModel(const kelon168_ac_remote_model_t model) {
  switch (model) {
    case kelon168_ac_remote_model_t::DG11R201:
    default:
      _.Model1 = 0b1000;
      _.Model2 = 0b001;
  }
}

/// Calculate the temp. offset in deg C for the current model.
/// Actually not used, but left for eventual support.
/// @return The temperature offset.
int8_t IRKelon168Ac::getTempOffset(void) const {
  return 0;
}

/// Set the temperature.
/// @param[in] temp The temperature in degrees celsius.
/// @param[in] remember Do we save this temperature?
/// @note Internal use only.
void IRKelon168Ac::_setTemp(const uint8_t temp, const bool remember) {
  if (remember) _desiredtemp = temp;
  int8_t offset = getTempOffset();  // Cache the min temp for the model.
  uint8_t newtemp = std::max((uint8_t)(kKelon168MinTemp + offset), temp);
  newtemp = std::min((uint8_t)(kKelon168MaxTemp + offset), newtemp);
  _.Temp = newtemp - (kKelon168MinTemp + offset);
}

/// Set the temperature.
/// @param[in] temp The temperature in degrees celsius.
void IRKelon168Ac::setTemp(const uint8_t temp) {
  _setTemp(temp);
  setSuper(false);  // Changing temp cancels Super/Jet mode.
  _.Cmd = kKelon168CommandTemp;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t IRKelon168Ac::getTemp(void) const {
  return _.Temp + kKelon168MinTemp + getTempOffset();
}

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
/// @note Internal use only.
void IRKelon168Ac::_setMode(const uint8_t mode) {
  switch (mode) {
    case kKelon168Auto:
      setFan(kKelonFanAuto);
      _setTemp(kKelon168AutoTemp, false);
      setSleep(false);  // Cancel sleep mode when in auto/6thsense mode.
      // FALL THRU
    case kKelon168Heat:
    case kKelon168Cool:
    case kKelon168Dry:
    case kKelon168Fan:
      _.Mode = mode;
      _.Cmd = kKelon168CommandMode;
      break;
    default:
      return;
  }
  if (mode == kKelon168Auto) _.Cmd = kKelon168CommandIFeel;
}

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void IRKelon168Ac::setMode(const uint8_t mode) {
    setSuper(false);  // Changing mode cancels Super/Jet mode.
    _setMode(mode);
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t IRKelon168Ac::getMode(void) const {
  return _.Mode;
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
void IRKelon168Ac::setFan(const uint8_t speed) {
  switch (speed) {
    case kKelon168FanAuto:
      _.Fan = 0;
      _.Fan2 = 0;
      break;
    case kKelon168FanMin:
      _.Fan  = 0b11;
      _.Fan2 = 0;
      break;
    case kKelon168FanLow:
      _.Fan = 0b11;
      _.Fan2 = 1;
      break;
    case kKelon168FanMedium:
      _.Fan = 0b10;
      _.Fan2 = 0;
      break;
    case kKelon168FanHigh:
      _.Fan = 0b01;
      _.Fan2 = 1;
      break;
    case kKelon168FanMax:
      _.Fan = 0b01;
      _.Fan2 = 0;
      break;
  }
  setSuper(false);  // Changing fan speed cancels Super/Jet mode.
  _.Cmd = kKelon168CommandFanSpeed;
}

/// Get the current fan speed setting.
/// The encoding is distributed across two bits, middle values (2 Low & 4 High)
/// are encoded on the Fan2 bit. Normal values (1 Minimum, 3 Medium,
/// 5 High) are written as 3,2,1 (inverted scale)
/// @return The current fan speed/mode.
uint8_t IRKelon168Ac::getFan(void) const {
  switch (_.Fan) {
    case 0b01:
      return _.Fan2 == 0 ? kKelon168FanMax : kKelon168FanHigh;
    case 0b10:
      return kKelon168FanMedium;
    case 0b11:
      return _.Fan2 == 0 ? kKelon168FanMin : kKelon168FanLow;
    default:
      return kKelon168FanAuto;
  }
}

/// Set the (vertical) swing setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRKelon168Ac::setSwing(const bool on) {
  _.Swing1 = on;
  _.Swing2 = on;
  _.Cmd = kKelon168CommandSwing;
}

/// Get the (vertical) swing setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRKelon168Ac::getSwing(void) const {
  return _.Swing1 && _.Swing2;
}

/// Set the Light (Display/LED) setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRKelon168Ac::setLight(const bool on) {
  // Cleared when on.
  _.LightOff = !on;
}

/// Get the Light (Display/LED) setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRKelon168Ac::getLight(void) const {
  return !_.LightOff;
}

/// Set the clock time in nr. of minutes past midnight.
/// @param[in] minsPastMidnight The time expressed as minutes past midnight.
void IRKelon168Ac::setClock(const uint16_t minsPastMidnight) {
  SETTIME(Clock, minsPastMidnight);
}

/// Get the clock time in nr. of minutes past midnight.
/// @return The time expressed as the Nr. of minutes past midnight.
uint16_t IRKelon168Ac::getClock(void) const {
  return GETTIME(Clock);
}

/// Set the Off Timer time.
/// @param[in] minsPastMidnight The time expressed as minutes past midnight.
void IRKelon168Ac::setOffTimer(const uint16_t minsPastMidnight) {
  SETTIME(Off, minsPastMidnight);
}

/// Get the Off Timer time..
/// @return The time expressed as the Nr. of minutes past midnight.
uint16_t IRKelon168Ac::getOffTimer(void) const {
  return GETTIME(Off);
}

/// Is the Off timer enabled?
/// @return true, the Timer is enabled. false, the Timer is disabled.
bool IRKelon168Ac::isOffTimerEnabled(void) const {
  return _.OffTimerEnabled;
}

/// Enable the Off Timer.
/// @param[in] on true, the timer is enabled. false, the timer is disabled.
void IRKelon168Ac::enableOffTimer(const bool on) {
  _.OffTimerEnabled = on;
  _.Cmd = kKelon168CommandOffTimer;
}

/// Set the On Timer time.
/// @param[in] minsPastMidnight The time expressed as minutes past midnight.
void IRKelon168Ac::setOnTimer(const uint16_t minsPastMidnight) {
  SETTIME(On, minsPastMidnight);
}

/// Get the On Timer time..
/// @return The time expressed as the Nr. of minutes past midnight.
uint16_t IRKelon168Ac::getOnTimer(void) const {
  return GETTIME(On);
}

/// Is the On timer enabled?
/// @return true, the Timer is enabled. false, the Timer is disabled.
bool IRKelon168Ac::isOnTimerEnabled(void) const {
  return _.OnTimerEnabled;
}

/// Enable the On Timer.
/// @param[in] on true, the timer is enabled. false, the timer is disabled.
void IRKelon168Ac::enableOnTimer(const bool on) {
  _.OnTimerEnabled = on;
  _.Cmd = kKelon168CommandOnTimer;
}

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRKelon168Ac::setPower(const bool on) {
  _.Power = true;
  _.On = on;
  setSuper(false);  // Changing power cancels Super/Jet mode.
  _.Cmd = kKelon168CommandPower;
}

/// Get the value of the current power toggle setting.
/// @return true, the setting is on. false, the setting is off.
bool IRKelon168Ac::getPower(void) const {
  return _.Power;
}

/// Get the Command (Button) setting of the A/C.
/// @return The current Command (Button) of the A/C.
uint8_t IRKelon168Ac::getCommand(void) const {
  return _.Cmd;
}

/// Set the Sleep setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRKelon168Ac::setSleep(const bool on) {
  _.Sleep = on;
  if (on) setFan(kKelon168FanLow);
  _.Cmd = kKelon168CommandSleep;
}

/// Get the Sleep setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRKelon168Ac::getSleep(void) const {
  return _.Sleep;
}

/// Set the Super (Turbo/Jet) setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRKelon168Ac::setSuper(const bool on) {
  if (on) {
    setFan(kKelon168FanHigh);
    switch (_.Mode) {
      case kKelon168Heat:
        setTemp(kKelonMaxTemp + getTempOffset());
        break;
      case kKelon168Cool:
      default:
        setTemp(kKelonMinTemp + getTempOffset());
        setMode(kKelon168Cool);
        break;
    }
    _.Super1 = 1;
    _.Super2 = 1;
  } else {
    _.Super1 = 0;
    _.Super2 = 0;
  }
  _.Cmd = kKelon168CommandSuper;
}

/// Get the Super (Turbo/Jet) setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRKelon168Ac::getSuper(void) const {
  return _.Super1 && _.Super2;
}

/// Set the Command (Button) setting of the A/C.
/// @param[in] code The current Command (Button) of the A/C.
void IRKelon168Ac::setCommand(const uint8_t code) {
  _.Cmd = code;
}

/// Convert a stdAc::opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRKelon168Ac::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kAuto: return kKelon168Auto;
    case stdAc::opmode_t::kHeat: return kKelon168Heat;
    case stdAc::opmode_t::kDry:  return kKelon168Dry;
    case stdAc::opmode_t::kFan:  return kKelon168Fan;
    // Default to Cool as some models don't have an Auto mode.
    // Ref: https://github.com/crankyoldgit/IRremoteESP8266/issues/1283
    default:                     return kKelon168Cool;
  }
}

/// Convert a stdAc::fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRKelon168Ac::convertFan(const stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin:    return kKelon168FanMin;
    case stdAc::fanspeed_t::kLow:    return kKelon168FanLow;
    case stdAc::fanspeed_t::kMedium: return kKelon168FanMedium;
    case stdAc::fanspeed_t::kHigh:   return kKelon168FanHigh;
    case stdAc::fanspeed_t::kMax:    return kKelon168FanMax;
    default:                         return kKelon168FanAuto;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::opmode_t IRKelon168Ac::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kKelon168Cool: return stdAc::opmode_t::kCool;
    case kKelon168Heat: return stdAc::opmode_t::kHeat;
    case kKelon168Dry:  return stdAc::opmode_t::kDry;
    case kKelon168Fan:  return stdAc::opmode_t::kFan;
    default:            return stdAc::opmode_t::kAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::fanspeed_t IRKelon168Ac::toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kKelon168FanMax:    return stdAc::fanspeed_t::kMax;
    case kKelon168FanHigh:   return stdAc::fanspeed_t::kHigh;
    case kKelon168FanMedium: return stdAc::fanspeed_t::kMedium;
    case kKelon168FanLow:    return stdAc::fanspeed_t::kLow;
    case kKelon168FanMin:    return stdAc::fanspeed_t::kMin;
    default:                 return stdAc::fanspeed_t::kAuto;
  }
}

/// Convert the current internal state into its stdAc::state_t equivalent.
/// @param[in] prev Ptr to the previous state if required.
/// @return The stdAc equivalent of the native settings.
stdAc::state_t IRKelon168Ac::toCommon(const stdAc::state_t *prev) const {
  stdAc::state_t result{};
  // Start with the previous state if given it.
  if (prev != NULL) {
    result = *prev;
  } else {
    // Set defaults for non-zero values that are not implicitly set for when
    // there is no previous state.
    // e.g. Any setting that toggles should probably go here.
    result.power = false;
  }
  result.protocol = decode_type_t::KELON168;
  result.model = getModel();
  if (_.Power) result.power = !result.power;
  result.mode = toCommonMode(_.Mode);
  result.celsius = true;
  result.degrees = getTemp();
  result.fanspeed = toCommonFanSpeed(_.Fan);
  result.swingv = getSwing() ? stdAc::swingv_t::kAuto : stdAc::swingv_t::kOff;
  result.turbo = getSuper();
  result.light = getLight();
  result.sleep = _.Sleep ? 0 : -1;
  // TODO(leonardfactory) add encoding
  result.swingh = stdAc::swingh_t::kOff;
  result.quiet = false;
  result.filter = false;
  result.econo = false;
  result.clean = false;
  result.beep = false;
  result.clock = -1;
  return result;
}

/// Convert the current internal state into a human readable string.
/// @return A human readable string.
String IRKelon168Ac::toString(void) const {
  String result = "";
  result.reserve(200);  // Reserve some heap for the string to reduce fragging.
  result += addModelToString(decode_type_t::KELON168, getModel(), false);
  result += addBoolToString(_.Power, kPowerToggleStr);
  result += addModeToString(_.Mode, kKelon168Auto, kKelon168Cool,
                            kKelon168Heat, kKelon168Dry, kKelon168Fan);
  result += addTempToString(getTemp());
  result += addFanToString(_.Fan, kKelon168FanHigh, kKelon168FanLow,
                           kKelon168FanAuto, kKelon168FanAuto,
                           kKelon168FanMedium);
  result += addBoolToString(getSwing(), kSwingStr);
  result += addBoolToString(getLight(), kLightStr);
  result += addLabeledString(minsToString(getClock()), kClockStr);
  result += addLabeledString(
      _.OnTimerEnabled ? minsToString(getOnTimer()) : kOffStr, kOnTimerStr);
  result += addLabeledString(
      _.OffTimerEnabled ? minsToString(getOffTimer()) : kOffStr, kOffTimerStr);
  result += addBoolToString(_.Sleep, kSleepStr);
  result += addBoolToString(getSuper(), kSuperStr);
  result += addIntToString(_.Cmd, kCommandStr);
  result += kSpaceLBraceStr;
  switch (_.Cmd) {
    case kKelon168CommandLight:
      result += kLightStr;
      break;
    case kKelon168CommandPower:
      result += kPowerStr;
      break;
    case kKelon168CommandTemp:
      result += kTempStr;
      break;
    case kKelon168CommandSleep:
      result += kSleepStr;
      break;
    case kKelon168CommandSuper:
      result += kSuperStr;
      break;
    case kKelon168CommandOnTimer:
      result += kOnTimerStr;
      break;
    case kKelon168CommandMode:
      result += kModeStr;
      break;
    case kKelon168CommandSwing:
      result += kSwingStr;
      break;
    case kKelon168CommandIFeel:
      result += kIFeelStr;
      break;
    case kKelon168CommandFanSpeed:
      result += kFanStr;
      break;
    case kKelon168CommandOffTimer:
      result += kOffTimerStr;
      break;
    default:
      result += kUnknownStr;
      break;
  }
  result += ')';
  return result;
}
