// Copyright 2016 sillyfrog
// Copyright 2017 sillyfrog, crankyoldgit
// Copyright 2018-2022 crankyoldgit
// Copyright 2019 pasna (IRDaikin160 class / Daikin176 class)

/// @file
/// @brief Support for 152-bit Daikin A/C protocols.
/// @see Daikin152 https://github.com/crankyoldgit/IRremoteESP8266/issues/873
/// @see Daikin152 https://github.com/ToniA/arduino-heatpumpir/blob/master/DaikinHeatpumpARC480A14IR.cpp
/// @see Daikin152 https://github.com/ToniA/arduino-heatpumpir/blob/master/DaikinHeatpumpARC480A14IR.h

// Supports:
//   Brand: Daikin,  Model: ARC480A5 remote (DAIKIN152)

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

#if SEND_DAIKIN152
/// Send a Daikin152 (152-bit) A/C formatted message.
/// Status: STABLE / Known Working.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/873
void IRsend::sendDaikin152(const unsigned char data[], const uint16_t nbytes,
                           const uint16_t repeat) {
  for (uint16_t r = 0; r <= repeat; r++) {
    // Leader
    sendGeneric(0, 0, kDaikin152BitMark, kDaikin152OneSpace,
                kDaikin152BitMark, kDaikin152ZeroSpace,
                kDaikin152BitMark, kDaikin152Gap,
                (uint64_t)0, kDaikin152LeaderBits,
                kDaikin152Freq, false, 0, kDutyDefault);
    // Header + Data + Footer
    sendGeneric(kDaikin152HdrMark, kDaikin152HdrSpace, kDaikin152BitMark,
                kDaikin152OneSpace, kDaikin152BitMark, kDaikin152ZeroSpace,
                kDaikin152BitMark, kDaikin152Gap, data,
                nbytes, kDaikin152Freq, false, 0, kDutyDefault);
  }
}
#endif  // SEND_DAIKIN152

#if DECODE_DAIKIN152
/// Decode the supplied Daikin 152-bit message. (DAIKIN152)
/// Status: STABLE / Known Working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/873
bool IRrecv::decodeDaikin152(decode_results *results, uint16_t offset,
                             const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * (5 + nbits + kFooter) + kHeader - 1 + offset)
    return false;
  if (nbits / 8 < kDaikin152StateLength) return false;

  // Compliance
  if (strict && nbits != kDaikin152Bits) return false;

  uint16_t used;

  // Leader
  uint64_t leader = 0;
  used = matchGeneric(results->rawbuf + offset, &leader,
                      results->rawlen - offset, kDaikin152LeaderBits,
                      0, 0,  // No Header
                      kDaikin152BitMark, kDaikin152OneSpace,
                      kDaikin152BitMark, kDaikin152ZeroSpace,
                      kDaikin152BitMark, kDaikin152Gap,  // Footer gap
                      false, _tolerance, kMarkExcess, false);
  if (used == 0 || leader != 0) return false;
  offset += used;

  // Header + Data + Footer
  used = matchGeneric(results->rawbuf + offset, results->state,
                      results->rawlen - offset, nbits,
                      kDaikin152HdrMark, kDaikin152HdrSpace,
                      kDaikin152BitMark, kDaikin152OneSpace,
                      kDaikin152BitMark, kDaikin152ZeroSpace,
                      kDaikin152BitMark, kDaikin152Gap,
                      true, _tolerance, kMarkExcess, false);
  if (used == 0) return false;

  // Compliance
  if (strict) {
    if (!IRDaikin152::validChecksum(results->state)) return false;
  }

  // Success
  results->decode_type = decode_type_t::DAIKIN152;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // DECODE_DAIKIN152

/// Class constructor
/// @param[in] pin GPIO to be used when sending.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRDaikin152::IRDaikin152(const uint16_t pin, const bool inverted,
                         const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) { stateReset(); }

/// Set up hardware to be able to send a message.
void IRDaikin152::begin(void) { _irsend.begin(); }

#if SEND_DAIKIN152
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRDaikin152::send(const uint16_t repeat) {
  _irsend.sendDaikin152(getRaw(), kDaikin152StateLength, repeat);
}
#endif  // SEND_DAIKIN152

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length of the state array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool IRDaikin152::validChecksum(uint8_t state[], const uint16_t length) {
  // Validate the checksum of the given state.
  if (length <= 1 || state[length - 1] != sumBytes(state, length - 1))
    return false;
  else
    return true;
}

/// Calculate and set the checksum values for the internal state.
void IRDaikin152::checksum(void) {
  _.Sum = sumBytes(_.raw, kDaikin152StateLength - 1);
}

/// Reset the internal state to a fixed known good state.
void IRDaikin152::stateReset(void) {
  for (uint8_t i = 3; i < kDaikin152StateLength; i++) _.raw[i] = 0x00;
  _.raw[0] =  0x11;
  _.raw[1] =  0xDA;
  _.raw[2] =  0x27;
  _.raw[15] = 0xC5;
  // _.raw[19] is a checksum byte, it will be set by checksum().
}

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *IRDaikin152::getRaw(void) {
  checksum();  // Ensure correct settings before sending.
  return _.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
void IRDaikin152::setRaw(const uint8_t new_code[]) {
  std::memcpy(_.raw, new_code, kDaikin152StateLength);
}

/// Change the power setting to On.
void IRDaikin152::on(void) { setPower(true); }

/// Change the power setting to Off.
void IRDaikin152::off(void) { setPower(false); }

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin152::setPower(const bool on) { _.Power = on; }

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin152::getPower(void) const { return _.Power; }

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t IRDaikin152::getMode(void) const { return _.Mode; }

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void IRDaikin152::setMode(const uint8_t mode) {
  switch (mode) {
    case kDaikinFan:
      setTemp(kDaikin152FanTemp);  // Handle special temp for fan mode.
      break;
    case kDaikinDry:
      setTemp(kDaikin152DryTemp);  // Handle special temp for dry mode.
      break;
    case kDaikinAuto:
    case kDaikinCool:
    case kDaikinHeat:
      break;
    default:
      _.Mode = kDaikinAuto;
      return;
  }
  _.Mode = mode;
}

/// Convert a stdAc::opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRDaikin152::convertMode(const stdAc::opmode_t mode) {
  return IRDaikinESP::convertMode(mode);
}

/// Set the temperature.
/// @param[in] temp The temperature in degrees celsius.
void IRDaikin152::setTemp(const uint8_t temp) {
  uint8_t degrees = std::max(
      temp, (_.Mode == kDaikinHeat) ? kDaikinMinTemp : kDaikin2MinCoolTemp);
  degrees = std::min(degrees, kDaikinMaxTemp);
  if (temp == kDaikin152FanTemp) degrees = temp;  // Handle fan only temp.
  _.Temp = degrees;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t IRDaikin152::getTemp(void) const { return _.Temp; }

/// Set the speed of the fan.
/// @param[in] fan The desired setting.
/// @note 1-5 or kDaikinFanAuto or kDaikinFanQuiet
void IRDaikin152::setFan(const uint8_t fan) {
  // Set the fan speed bits, leave low 4 bits alone
  uint8_t fanset;
  if (fan == kDaikinFanQuiet || fan == kDaikinFanAuto)
    fanset = fan;
  else if (fan < kDaikinFanMin || fan > kDaikinFanMax)
    fanset = kDaikinFanAuto;
  else
    fanset = 2 + fan;
  _.Fan = fanset;
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t IRDaikin152::getFan(void) const {
  const uint8_t fan = _.Fan;
  switch (fan) {
    case kDaikinFanAuto:
    case kDaikinFanQuiet: return fan;
    default: return fan - 2;
  }
}

/// Convert a stdAc::fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRDaikin152::convertFan(const stdAc::fanspeed_t speed) {
  return IRDaikinESP::convertFan(speed);
}

/// Set the Vertical Swing mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin152::setSwingV(const bool on) {
  _.SwingV = (on ? kDaikinSwingOn : kDaikinSwingOff);
}

/// Get the Vertical Swing mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin152::getSwingV(void) const { return _.SwingV; }

/// Set the Quiet mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin152::setQuiet(const bool on) {
  _.Quiet = on;
  // Powerful & Quiet mode being on are mutually exclusive.
  if (on) setPowerful(false);
}

/// Get the Quiet mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin152::getQuiet(void) const { return _.Quiet; }

/// Set the Powerful (Turbo) mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin152::setPowerful(const bool on) {
  _.Powerful = on;
  if (on) {
    // Powerful, Quiet, Comfort & Econo mode being on are mutually exclusive.
    setQuiet(false);
    setComfort(false);
    setEcono(false);
  }
}

/// Get the Powerful (Turbo) mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin152::getPowerful(void) const { return _.Powerful; }

/// Set the Economy mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin152::setEcono(const bool on) {
  _.Econo = on;
  // Powerful & Econo mode being on are mutually exclusive.
  if (on) setPowerful(false);
}

/// Get the Economical mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin152::getEcono(void) const { return _.Econo; }

/// Set the Sensor mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin152::setSensor(const bool on) { _.Sensor = on; }

/// Get the Sensor mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin152::getSensor(void) const { return _.Sensor; }

/// Set the Comfort mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin152::setComfort(const bool on) {
  _.Comfort = on;
  if (on) {
    // Comfort mode is incompatible with Powerful mode.
    setPowerful(false);
    // It also sets the fan to auto and turns off swingv.
    setFan(kDaikinFanAuto);
    setSwingV(false);
  }
}

/// Get the Comfort mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin152::getComfort(void) const { return _.Comfort; }

/// Convert the current internal state into its stdAc::state_t equivalent.
/// @return The stdAc equivalent of the native settings.
stdAc::state_t IRDaikin152::toCommon(void) const {
  stdAc::state_t result{};
  result.protocol = decode_type_t::DAIKIN152;
  result.model = -1;  // No models used.
  result.power = _.Power;
  result.mode = IRDaikinESP::toCommonMode(_.Mode);
  result.celsius = true;
  result.degrees = _.Temp;
  result.fanspeed = IRDaikinESP::toCommonFanSpeed(getFan());
  result.swingv = _.SwingV ? stdAc::swingv_t::kAuto : stdAc::swingv_t::kOff;
  result.quiet = _.Quiet;
  result.turbo = _.Powerful;
  result.econo = _.Econo;
  // Not supported.
  result.swingh = stdAc::swingh_t::kOff;
  result.clean = false;
  result.filter = false;
  result.light = false;
  result.beep = false;
  result.sleep = -1;
  result.clock = -1;
  return result;
}

/// Convert the current internal state into a human readable string.
/// @return A human readable string.
String IRDaikin152::toString(void) const {
  String result = "";
  result.reserve(180);  // Reserve some heap for the string to reduce fragging.
  result += addBoolToString(_.Power, kPowerStr, false);
  result += addModeToString(_.Mode, kDaikinAuto, kDaikinCool, kDaikinHeat,
                            kDaikinDry, kDaikinFan);
  result += addTempToString(_.Temp);
  result += addFanToString(getFan(), kDaikinFanMax, kDaikinFanMin,
                           kDaikinFanAuto, kDaikinFanQuiet, kDaikinFanMed);
  result += addBoolToString(_.SwingV, kSwingVStr);
  result += addBoolToString(_.Powerful, kPowerfulStr);
  result += addBoolToString(_.Quiet, kQuietStr);
  result += addBoolToString(_.Econo, kEconoStr);
  result += addBoolToString(_.Sensor, kSensorStr);
  result += addBoolToString(_.Comfort, kComfortStr);
  return result;
}
