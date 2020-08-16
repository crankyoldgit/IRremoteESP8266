// Copyright 2020 David Conran (crankyoldgit)
// Copyright 2020 manj9501
/// @file
/// @brief Support for Voltas A/C protocol
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1238

#include "ir_Voltas.h"
#include <cstring>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRtext.h"
#include "IRutils.h"

using irutils::addBoolToString;
using irutils::addModeToString;
using irutils::addFanToString;
using irutils::addTempToString;
using irutils::minsToString;

// Constants
const uint16_t kVoltasBitMark = 1026;   ///< uSeconds.
const uint16_t kVoltasOneSpace = 2553;  ///< uSeconds.
const uint16_t kVoltasZeroSpace = 554;  ///< uSeconds.
const uint16_t kVoltasFreq = 38000;     ///< Hz.

#if SEND_VOLTAS
/// Send a Voltas formatted message.
/// Status: ALPHA / Untested.
/// @param[in] data An array of bytes containing the IR command.
///                 It is assumed to be in MSB order for this code.
/// e.g.
/// @code
///   uint8_t data[kVoltasStateLength] = {0x33, 0x28, 0x88, 0x1A, 0x3B, 0x3B,
///                                       0x3B, 0x11, 0x00, 0x40};
/// @endcode
/// @param[in] nbytes Nr. of bytes of data in the array. (>=kVoltasStateLength)
/// @param[in] repeat Nr. of times the message is to be repeated.
void IRsend::sendVoltas(const uint8_t data[], const uint16_t nbytes,
                        const uint16_t repeat) {
  sendGeneric(0, 0,
              kVoltasBitMark, kVoltasOneSpace,
              kVoltasBitMark, kVoltasZeroSpace,
              kVoltasBitMark, kDefaultMessageGap,
              data, nbytes,
              kVoltasFreq, true, repeat, kDutyDefault);
}
#endif  // SEND_VOLTAS

#if DECODE_VOLTAS
/// Decode the supplied Voltas message.
/// Status: ALPHA / Untested.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool IRrecv::decodeVoltas(decode_results *results, uint16_t offset,
                          const uint16_t nbits, const bool strict) {
  if (strict && nbits != kVoltasBits) return false;

  // Data + Footer
  if (!matchGeneric(results->rawbuf + offset, results->state,
                    results->rawlen - offset, nbits,
                    0, 0,  // No header
                    kVoltasBitMark, kVoltasOneSpace,
                    kVoltasBitMark, kVoltasZeroSpace,
                    kVoltasBitMark, kDefaultMessageGap, true)) return false;

  // Compliance
  if (strict && !IRVoltas::validChecksum(results->state, nbits / 8))
    return false;
  // Success
  results->decode_type = decode_type_t::VOLTAS;
  results->bits = nbits;
  return true;
}
#endif  // DECODE_VOLTAS

/// Class constructor
/// @param[in] pin GPIO to be used when sending.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRVoltas::IRVoltas(const uint16_t pin, const bool inverted,
                   const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) {
  stateReset();
}

// Reset the internal state to a fixed known good state.
void IRVoltas::stateReset() {
  // This resets to a known-good state.
  std::memset(_.raw, 0, sizeof _.raw);
}

/// Set up hardware to be able to send a message.
void IRVoltas::begin() { _irsend.begin(); }

#if SEND_VOLTAS
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRVoltas::send(const uint16_t repeat) {
  _irsend.sendVoltas(getRaw(), kVoltasStateLength, repeat);
}
#endif  // SEND_VOLTAS

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t* IRVoltas::getRaw(void) {
  checksum();  // Ensure correct settings before sending.
  return _.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
void IRVoltas::setRaw(const uint8_t new_code[]) {
  std::memcpy(_.raw, new_code, kVoltasStateLength);
}

/// Calculate and set the checksum values for the internal state.
void IRVoltas::checksum(void) {
  _.Checksum = calcChecksum(_.raw, kVoltasStateLength - 1);
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length of the state array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool IRVoltas::validChecksum(const uint8_t state[], const uint16_t length) {
  if (length) return state[length - 1] == calcChecksum(state, length);
  return true;
}

/// Calculate the checksum is valid for a given state.
/// @param[in] state The array to calculate the checksum of.
/// @param[in] length The length of the state array.
/// @return The valid checksum value for the state.
uint8_t IRVoltas::calcChecksum(const uint8_t state[], const uint16_t length) {
  uint8_t result = 0;
  if (length)
    result = sumBytes(state, length - 1);
  return ~result;
}

/// Change the power setting to On.
void IRVoltas::on() { setPower(true); }

/// Change the power setting to Off.
void IRVoltas::off() { setPower(false); }

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRVoltas::setPower(const bool on) { _.Power = on; }

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool IRVoltas::getPower(void) const { return _.Power; }

/// Convert the current internal state into its stdAc::state_t equivilant.
/// @return The stdAc equivilant of the native settings.
stdAc::state_t IRVoltas::toCommon() {
  stdAc::state_t result;
  result.protocol = decode_type_t::VOLTAS;
  result.power = _.Power;
  // result.mode = toCommonMode(getMode());
  result.celsius = true;
  /*
  result.degrees = getTemp();
  result.fanspeed = toCommonFanSpeed(getFan());
  if (getSwingVerticalAuto())
    result.swingv = stdAc::swingv_t::kAuto;
  else
    result.swingv = toCommonSwingV(getSwingVerticalPosition());
  */
  result.turbo = _.Turbo;
  result.econo = _.Econo;
  result.light = _.Light;
  /*
  result.clean = getXFan();
  result.sleep = getSleep() ? 0 : -1;
  */
  // Not supported.
  result.model = -1;
  result.swingh = stdAc::swingh_t::kOff;
  result.quiet = false;
  result.filter = false;
  result.beep = false;
  result.clock = -1;
  return result;
}

/// Convert the current internal state into a human readable string.
/// @return A human readable string.
String IRVoltas::toString() {
  String result = "";
  result.reserve(80);  // Reserve some heap for the string to reduce fragging.
  result += addBoolToString(_.Power, kPowerStr, false);
  /*
  result += addModeToString(getMode(), kVoltasAuto, kVoltasCool, kVoltasHeat,
                            kVoltasDry, kVoltasFan);
  */
  result += addBoolToString(_.Turbo, kTurboStr);
  result += addBoolToString(_.Wifi, kWifiStr);
  result += addBoolToString(_.Light, kLightStr);
  return result;
}
