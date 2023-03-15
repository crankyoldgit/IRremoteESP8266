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

#if SEND_DAIKIN176
/// Send a Daikin176 (176-bit) A/C formatted message.
/// Status: STABLE / Working on a real device.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void IRsend::sendDaikin176(const unsigned char data[], const uint16_t nbytes,
                           const uint16_t repeat) {
  if (nbytes < kDaikin176Section1Length)
    return;  // Not enough bytes to send a partial message.

  for (uint16_t r = 0; r <= repeat; r++) {
    // Section #1
    sendGeneric(kDaikin176HdrMark, kDaikin176HdrSpace, kDaikin176BitMark,
                kDaikin176OneSpace, kDaikin176BitMark, kDaikin176ZeroSpace,
                kDaikin176BitMark, kDaikin176Gap, data,
                kDaikin176Section1Length,
                kDaikin176Freq, false, 0, kDutyDefault);
    // Section #2
    sendGeneric(kDaikin176HdrMark, kDaikin176HdrSpace, kDaikin176BitMark,
                kDaikin176OneSpace, kDaikin176BitMark, kDaikin176ZeroSpace,
                kDaikin176BitMark, kDaikin176Gap,
                data + kDaikin176Section1Length,
                nbytes - kDaikin176Section1Length,
                kDaikin176Freq, false, 0, kDutyDefault);
  }
}
#endif  // SEND_DAIKIN176

/// Class constructor
/// @param[in] pin GPIO to be used when sending.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRDaikin176::IRDaikin176(const uint16_t pin, const bool inverted,
                         const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) { stateReset(); }

/// Set up hardware to be able to send a message.
void IRDaikin176::begin(void) { _irsend.begin(); }

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length of the state array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool IRDaikin176::validChecksum(uint8_t state[], const uint16_t length) {
  // Validate the checksum of section #1.
  if (length <= kDaikin176Section1Length - 1 ||
      state[kDaikin176Section1Length - 1] != sumBytes(
          state, kDaikin176Section1Length - 1))
    return false;
  // Validate the checksum of section #2 (a.k.a. the rest)
  if (length <= kDaikin176Section1Length + 1 ||
      state[length - 1] != sumBytes(state + kDaikin176Section1Length,
                                    length - kDaikin176Section1Length - 1))
    return false;
  return true;
}

/// Calculate and set the checksum values for the internal state.
void IRDaikin176::checksum(void) {
  _.Sum1 = sumBytes(_.raw, kDaikin176Section1Length - 1);
  _.Sum2 = sumBytes(_.raw + kDaikin176Section1Length,
                    kDaikin176Section2Length - 1);
}

/// Reset the internal state to a fixed known good state.
void IRDaikin176::stateReset(void) {
  for (uint8_t i = 0; i < kDaikin176StateLength; i++) _.raw[i] = 0x00;
  _.raw[0] =  0x11;
  _.raw[1] =  0xDA;
  _.raw[2] =  0x17;
  _.raw[3] =  0x18;
  _.raw[4] =  0x04;
  // _.raw[6] is a checksum byte, it will be set by checksum().
  _.raw[7] =  0x11;
  _.raw[8] =  0xDA;
  _.raw[9] =  0x17;
  _.raw[10] = 0x18;
  _.raw[12] = 0x73;
  _.raw[14] = 0x20;
  _.raw[18] = 0x16;  // Fan speed and swing
  _.raw[20] = 0x20;
  // _.raw[21] is a checksum byte, it will be set by checksum().
  _saved_temp = getTemp();
}

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *IRDaikin176::getRaw(void) {
  checksum();  // Ensure correct settings before sending.
  return _.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
void IRDaikin176::setRaw(const uint8_t new_code[]) {
  std::memcpy(_.raw, new_code, kDaikin176StateLength);
  _saved_temp = getTemp();
}

#if SEND_DAIKIN176
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRDaikin176::send(const uint16_t repeat) {
  _irsend.sendDaikin176(getRaw(), kDaikin176StateLength, repeat);
}
#endif  // SEND_DAIKIN176

/// Change the power setting to On.
void IRDaikin176::on(void) { setPower(true); }

/// Change the power setting to Off..
void IRDaikin176::off(void) { setPower(false); }

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin176::setPower(const bool on) {
  _.ModeButton = 0;
  _.Power = on;
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin176::getPower(void) const { return _.Power; }

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t IRDaikin176::getMode(void) const { return _.Mode; }

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void IRDaikin176::setMode(const uint8_t mode) {
  uint8_t altmode = 0;
  // Set the mode bits.
  _.Mode = mode;
  // Daikin172 has some alternate/additional mode bits that need to be changed
  // in line with the operating mode. The following few lines match up these
  // bits with the corresponding operating bits.
  switch (mode) {
    case kDaikin176Dry:  altmode = 2; break;
    case kDaikin176Fan:  altmode = 6; break;
    case kDaikin176Auto:
    case kDaikin176Cool:
    case kDaikin176Heat: altmode = 7; break;
    default: _.Mode = kDaikin176Cool; altmode = 7; break;
  }
  // Set the additional mode bits.
  _.AltMode = altmode;
  setTemp(_saved_temp);
  // Needs to happen after setTemp() as it will clear it.
  _.ModeButton = kDaikin176ModeButton;
}

/// Convert a stdAc::opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRDaikin176::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kDry:   return kDaikin176Dry;
    case stdAc::opmode_t::kHeat:  return kDaikin176Heat;
    case stdAc::opmode_t::kFan:   return kDaikin176Fan;
    case stdAc::opmode_t::kAuto:  return kDaikin176Auto;
    default:                      return kDaikin176Cool;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::opmode_t IRDaikin176::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kDaikin176Dry:  return stdAc::opmode_t::kDry;
    case kDaikin176Heat: return stdAc::opmode_t::kHeat;
    case kDaikin176Fan:  return stdAc::opmode_t::kFan;
    case kDaikin176Auto: return stdAc::opmode_t::kAuto;
    default: return stdAc::opmode_t::kCool;
  }
}

/// Set the temperature.
/// @param[in] temp The temperature in degrees celsius.
void IRDaikin176::setTemp(const uint8_t temp) {
  uint8_t degrees = std::min(kDaikinMaxTemp, std::max(temp, kDaikinMinTemp));
  _saved_temp = degrees;
  switch (_.Mode) {
    case kDaikin176Dry:
    case kDaikin176Fan:
      degrees = kDaikin176DryFanTemp; break;
  }
  _.Temp = degrees - 9;
  _.ModeButton = 0;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t IRDaikin176::getTemp(void) const { return _.Temp + 9; }

/// Set the speed of the fan.
/// @param[in] fan The desired setting.
/// @note 1 for Min or 3 for Max
void IRDaikin176::setFan(const uint8_t fan) {
  switch (fan) {
    case kDaikinFanMin:
    case kDaikin176FanMax:
      _.Fan = fan;
      break;
    default:
      _.Fan = kDaikin176FanMax;
      break;
  }
  _.ModeButton = 0;
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t IRDaikin176::getFan(void) const { return _.Fan; }

/// Convert a stdAc::fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRDaikin176::convertFan(const stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin:
    case stdAc::fanspeed_t::kLow: return kDaikinFanMin;
    default: return kDaikin176FanMax;
  }
}

/// Set the Horizontal Swing mode of the A/C.
/// @param[in] position The position/mode to set the swing to.
void IRDaikin176::setSwingHorizontal(const uint8_t position) {
  switch (position) {
    case kDaikin176SwingHOff:
    case kDaikin176SwingHAuto:
      _.SwingH = position;
      break;
    default: _.SwingH = kDaikin176SwingHAuto;
  }
}

/// Get the Horizontal Swing mode of the A/C.
/// @return The native position/mode setting.
uint8_t IRDaikin176::getSwingHorizontal(void) const { return _.SwingH; }

/// Get the Unit Id of the A/C.
/// @return The Unit Id the A/C is to use.
uint8_t IRDaikin176::getId(void) const { return _.Id1; }

/// Set the Unit Id of the A/C.
/// @param[in] num The Unit Id the A/C is to use.
/// @note 0 for Unit A; 1 for Unit B
void IRDaikin176::setId(const uint8_t num) { _.Id1 = _.Id2 = num; }

/// Convert a stdAc::swingh_t enum into it's native setting.
/// @param[in] position The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRDaikin176::convertSwingH(const stdAc::swingh_t position) {
  switch (position) {
    case stdAc::swingh_t::kOff:  return kDaikin176SwingHOff;
    case stdAc::swingh_t::kAuto: return kDaikin176SwingHAuto;
    default: return kDaikin176SwingHAuto;
  }
}

/// Convert a native horizontal swing postion to it's common equivalent.
/// @param[in] setting A native position to convert.
/// @return The common horizontal swing position.
stdAc::swingh_t IRDaikin176::toCommonSwingH(const uint8_t setting) {
  switch (setting) {
    case kDaikin176SwingHOff: return stdAc::swingh_t::kOff;
    case kDaikin176SwingHAuto: return stdAc::swingh_t::kAuto;
    default:
      return stdAc::swingh_t::kAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::fanspeed_t IRDaikin176::toCommonFanSpeed(const uint8_t speed) {
  return (speed == kDaikinFanMin) ? stdAc::fanspeed_t::kMin
                                  : stdAc::fanspeed_t::kMax;
}

/// Convert the current internal state into its stdAc::state_t equivalent.
/// @return The stdAc equivalent of the native settings.
stdAc::state_t IRDaikin176::toCommon(void) const {
  stdAc::state_t result{};
  result.protocol = decode_type_t::DAIKIN176;
  result.model = -1;  // No models used.
  result.power = _.Power;
  result.mode = IRDaikin176::toCommonMode(_.Mode);
  result.celsius = true;
  result.degrees = getTemp();
  result.fanspeed = toCommonFanSpeed(_.Fan);
  result.swingh = toCommonSwingH(_.SwingH);

  // Not supported.
  result.swingv = stdAc::swingv_t::kOff;
  result.quiet = false;
  result.turbo = false;
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
String IRDaikin176::toString(void) const {
  String result = "";
  result.reserve(90);  // Reserve some heap for the string to reduce fragging.
  result += addBoolToString(_.Power, kPowerStr, false);
  result += addModeToString(_.Mode, kDaikin176Auto, kDaikin176Cool,
                            kDaikin176Heat, kDaikin176Dry, kDaikin176Fan);
  result += addTempToString(getTemp());
  result += addFanToString(_.Fan, kDaikin176FanMax, kDaikinFanMin,
                           kDaikinFanMin, kDaikinFanMin, kDaikinFanMin);
  result += addSwingHToString(_.SwingH, kDaikin176SwingHAuto,
                              kDaikin176SwingHAuto,  // maxleft Unused
                              kDaikin176SwingHAuto,  // left Unused
                              kDaikin176SwingHAuto,  // middle Unused
                              kDaikin176SwingHAuto,  // right Unused
                              kDaikin176SwingHAuto,  // maxright Unused
                              kDaikin176SwingHOff,
                              // Below are unused.
                              kDaikin176SwingHAuto,
                              kDaikin176SwingHAuto,
                              kDaikin176SwingHAuto,
                              kDaikin176SwingHAuto);
  result += addIntToString(_.Id1, kIdStr);
  return result;
}

#if DECODE_DAIKIN176
/// Decode the supplied Daikin 176-bit message. (DAIKIN176)
/// Status: STABLE / Expected to work.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool IRrecv::decodeDaikin176(decode_results *results, uint16_t offset,
                             const uint16_t nbits,
                             const bool strict) {
  if (results->rawlen < 2 * (nbits + kHeader + kFooter) - 1 + offset)
    return false;

  // Compliance
  if (strict && nbits != kDaikin176Bits) return false;

  const uint8_t ksectionSize[kDaikin176Sections] = {kDaikin176Section1Length,
                                                    kDaikin176Section2Length};

  // Sections
  uint16_t pos = 0;
  for (uint8_t section = 0; section < kDaikin176Sections; section++) {
    uint16_t used;
    // Section Header + Section Data (7 bytes) + Section Footer
    used = matchGeneric(results->rawbuf + offset, results->state + pos,
                        results->rawlen - offset, ksectionSize[section] * 8,
                        kDaikin176HdrMark, kDaikin176HdrSpace,
                        kDaikin176BitMark, kDaikin176OneSpace,
                        kDaikin176BitMark, kDaikin176ZeroSpace,
                        kDaikin176BitMark, kDaikin176Gap,
                        section >= kDaikin176Sections - 1,
                        kDaikinTolerance, kDaikinMarkExcess, false);
    if (used == 0) return false;
    offset += used;
    pos += ksectionSize[section];
  }
  // Compliance
  if (strict) {
    // Validate the checksum.
    if (!IRDaikin176::validChecksum(results->state)) return false;
  }

  // Success
  results->decode_type = decode_type_t::DAIKIN176;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // DECODE_DAIKIN176

