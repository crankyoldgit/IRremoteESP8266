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


#if SEND_DAIKIN216
/// Send a Daikin216 (216-bit) A/C formatted message.
/// Status: Alpha / Untested on a real device.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/689
/// @see https://github.com/danny-source/Arduino_DY_IRDaikin
void IRsend::sendDaikin216(const unsigned char data[], const uint16_t nbytes,
                           const uint16_t repeat) {
  if (nbytes < kDaikin216Section1Length)
    return;  // Not enough bytes to send a partial message.

  for (uint16_t r = 0; r <= repeat; r++) {
    // Section #1
    sendGeneric(kDaikin216HdrMark, kDaikin216HdrSpace, kDaikin216BitMark,
                kDaikin216OneSpace, kDaikin216BitMark, kDaikin216ZeroSpace,
                kDaikin216BitMark, kDaikin216Gap, data,
                kDaikin216Section1Length,
                kDaikin216Freq, false, 0, kDutyDefault);
    // Section #2
    sendGeneric(kDaikin216HdrMark, kDaikin216HdrSpace, kDaikin216BitMark,
                kDaikin216OneSpace, kDaikin216BitMark, kDaikin216ZeroSpace,
                kDaikin216BitMark, kDaikin216Gap,
                data + kDaikin216Section1Length,
                nbytes - kDaikin216Section1Length,
                kDaikin216Freq, false, 0, kDutyDefault);
  }
}
#endif  // SEND_DAIKIN216

/// Class Constructor
/// @param[in] pin GPIO to be used when sending.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRDaikin216::IRDaikin216(const uint16_t pin, const bool inverted,
                         const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) { stateReset(); }

/// Set up hardware to be able to send a message.
void IRDaikin216::begin(void) { _irsend.begin(); }

#if SEND_DAIKIN216
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRDaikin216::send(const uint16_t repeat) {
  _irsend.sendDaikin216(getRaw(), kDaikin216StateLength, repeat);
}
#endif  // SEND_DAIKIN216

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length of the state array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool IRDaikin216::validChecksum(uint8_t state[], const uint16_t length) {
  // Validate the checksum of section #1.
  if (length <= kDaikin216Section1Length - 1 ||
      state[kDaikin216Section1Length - 1] != sumBytes(
          state, kDaikin216Section1Length - 1))
    return false;
  // Validate the checksum of section #2 (a.k.a. the rest)
  if (length <= kDaikin216Section1Length + 1 ||
      state[length - 1] != sumBytes(state + kDaikin216Section1Length,
                                    length - kDaikin216Section1Length - 1))
    return false;
  return true;
}

/// Calculate and set the checksum values for the internal state.
void IRDaikin216::checksum(void) {
  _.Sum1 = sumBytes(_.raw, kDaikin216Section1Length - 1);
  _.Sum2 = sumBytes(_.raw + kDaikin216Section1Length,
                    kDaikin216Section2Length - 1);
}

/// Reset the internal state to a fixed known good state.
void IRDaikin216::stateReset(void) {
  for (uint8_t i = 0; i < kDaikin216StateLength; i++) _.raw[i] = 0x00;
  _.raw[0] =  0x11;
  _.raw[1] =  0xDA;
  _.raw[2] =  0x27;
  _.raw[3] =  0xF0;
  // _.raw[7] is a checksum byte, it will be set by checksum().
  _.raw[8] =  0x11;
  _.raw[9] =  0xDA;
  _.raw[10] = 0x27;
  _.raw[23] = 0xC0;
  // _.raw[26] is a checksum byte, it will be set by checksum().
}

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *IRDaikin216::getRaw(void) {
  checksum();  // Ensure correct settings before sending.
  return _.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
void IRDaikin216::setRaw(const uint8_t new_code[]) {
  std::memcpy(_.raw, new_code, kDaikin216StateLength);
}

/// Change the power setting to On.
void IRDaikin216::on(void) { setPower(true); }

/// Change the power setting to Off.
void IRDaikin216::off(void) { setPower(false); }

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin216::setPower(const bool on) { _.Power = on; }

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin216::getPower(void) const { return _.Power; }

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t IRDaikin216::getMode(void) const { return _.Mode; }

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void IRDaikin216::setMode(const uint8_t mode) {
  switch (mode) {
    case kDaikinAuto:
    case kDaikinCool:
    case kDaikinHeat:
    case kDaikinFan:
    case kDaikinDry:
      _.Mode = mode;
      break;
    default:
      _.Mode = kDaikinAuto;
  }
}

/// Convert a stdAc::opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRDaikin216::convertMode(const stdAc::opmode_t mode) {
  return IRDaikinESP::convertMode(mode);
}

/// Set the temperature.
/// @param[in] temp The temperature in degrees celsius.
void IRDaikin216::setTemp(const uint8_t temp) {
  uint8_t degrees = std::max(temp, kDaikinMinTemp);
  degrees = std::min(degrees, kDaikinMaxTemp);
  _.Temp = degrees;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t IRDaikin216::getTemp(void) const { return _.Temp; }

/// Set the speed of the fan.
/// @param[in] fan The desired setting.
/// @note 1-5 or kDaikinFanAuto or kDaikinFanQuiet
void IRDaikin216::setFan(const uint8_t fan) {
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
uint8_t IRDaikin216::getFan(void) const {
  uint8_t fan = _.Fan;
  if (fan != kDaikinFanQuiet && fan != kDaikinFanAuto) fan -= 2;
  return fan;
}

/// Convert a stdAc::fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRDaikin216::convertFan(const stdAc::fanspeed_t speed) {
  return IRDaikinESP::convertFan(speed);
}

/// Set the Vertical Swing mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin216::setSwingVertical(const bool on) {
  _.SwingV = (on ? kDaikin216SwingOn : kDaikin216SwingOff);
}

/// Get the Vertical Swing mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin216::getSwingVertical(void) const { return _.SwingV; }

/// Set the Horizontal Swing mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin216::setSwingHorizontal(const bool on) {
  _.SwingH = (on ? kDaikin216SwingOn : kDaikin216SwingOff);
}

/// Get the Horizontal Swing mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin216::getSwingHorizontal(void) const { return _.SwingH; }

/// Set the Quiet mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
/// @note This is a horrible hack till someone works out the quiet mode bit.
void IRDaikin216::setQuiet(const bool on) {
  if (on) {
    setFan(kDaikinFanQuiet);
    // Powerful & Quiet mode being on are mutually exclusive.
    setPowerful(false);
  } else if (getFan() == kDaikinFanQuiet) {
    setFan(kDaikinFanAuto);
  }
}

/// Get the Quiet mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
/// @note This is a horrible hack till someone works out the quiet mode bit.
bool IRDaikin216::getQuiet(void) const { return getFan() == kDaikinFanQuiet; }

/// Set the Powerful (Turbo) mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin216::setPowerful(const bool on) {
  _.Powerful = on;
  // Powerful & Quiet mode being on are mutually exclusive.
  if (on) setQuiet(false);
}

/// Get the Powerful (Turbo) mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin216::getPowerful(void) const { return _.Powerful; }

/// Convert the current internal state into its stdAc::state_t equivalent.
/// @return The stdAc equivalent of the native settings.
stdAc::state_t IRDaikin216::toCommon(void) const {
  stdAc::state_t result{};
  result.protocol = decode_type_t::DAIKIN216;
  result.model = -1;  // No models used.
  result.power = _.Power;
  result.mode = IRDaikinESP::toCommonMode(_.Mode);
  result.celsius = true;
  result.degrees = _.Temp;
  result.fanspeed = IRDaikinESP::toCommonFanSpeed(getFan());
  result.swingv = _.SwingV ? stdAc::swingv_t::kAuto :
                              stdAc::swingv_t::kOff;
  result.swingh = _.SwingH ? stdAc::swingh_t::kAuto :
                              stdAc::swingh_t::kOff;
  result.quiet = getQuiet();
  result.turbo = _.Powerful;
  // Not supported.
  result.light = false;
  result.clean = false;
  result.econo = false;
  result.filter = false;
  result.beep = false;
  result.sleep = -1;
  result.clock = -1;
  return result;
}

/// Convert the current internal state into a human readable string.
/// @return A human readable string.
String IRDaikin216::toString(void) const {
  String result = "";
  result.reserve(120);  // Reserve some heap for the string to reduce fragging.
  result += addBoolToString(_.Power, kPowerStr, false);
  result += addModeToString(_.Mode, kDaikinAuto, kDaikinCool, kDaikinHeat,
                            kDaikinDry, kDaikinFan);
  result += addTempToString(_.Temp);
  result += addFanToString(getFan(), kDaikinFanMax, kDaikinFanMin,
                           kDaikinFanAuto, kDaikinFanQuiet, kDaikinFanMed);
  result += addBoolToString(_.SwingH, kSwingHStr);
  result += addBoolToString(_.SwingV, kSwingVStr);
  result += addBoolToString(getQuiet(), kQuietStr);
  result += addBoolToString(_.Powerful, kPowerfulStr);
  return result;
}

#if DECODE_DAIKIN216
/// Decode the supplied Daikin 216-bit message. (DAIKIN216)
/// Status: STABLE / Should be working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/689
/// @see https://github.com/danny-source/Arduino_DY_IRDaikin
bool IRrecv::decodeDaikin216(decode_results *results, uint16_t offset,
                             const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * (nbits + kHeader + kFooter) - 1 + offset)
    return false;

  // Compliance
  if (strict && nbits != kDaikin216Bits) return false;

  const uint8_t ksectionSize[kDaikin216Sections] = {kDaikin216Section1Length,
                                                    kDaikin216Section2Length};
  // Sections
  uint16_t pos = 0;
  for (uint8_t section = 0; section < kDaikin216Sections; section++) {
    uint16_t used;
    // Section Header + Section Data + Section Footer
    used = matchGeneric(results->rawbuf + offset, results->state + pos,
                        results->rawlen - offset, ksectionSize[section] * 8,
                        kDaikin216HdrMark, kDaikin216HdrSpace,
                        kDaikin216BitMark, kDaikin216OneSpace,
                        kDaikin216BitMark, kDaikin216ZeroSpace,
                        kDaikin216BitMark, kDaikin216Gap,
                        section >= kDaikin216Sections - 1,
                        kDaikinTolerance, kDaikinMarkExcess, false);
    if (used == 0) return false;
    offset += used;
    pos += ksectionSize[section];
  }
  // Compliance
  if (strict) {
    if (pos * 8 != kDaikin216Bits) return false;
    // Validate the checksum.
    if (!IRDaikin216::validChecksum(results->state)) return false;
  }

  // Success
  results->decode_type = decode_type_t::DAIKIN216;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // DECODE_DAIKIN216
