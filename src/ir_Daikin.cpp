// Copyright 2016 sillyfrog
// Copyright 2017 sillyfrog, crankyoldgit
// Copyright 2018-2022 crankyoldgit
// Copyright 2019 pasna (IRDaikin160 class / Daikin176 class)

/// @file
/// @brief Support for 280-bit Daikin A/C protocols.
/// @see Daikin http://harizanov.com/2012/02/control-daikin-air-conditioner-over-the-internet/
/// @see Daikin https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/tree/master/IRremote
/// @see Daikin http://rdlab.cdmt.vn/project-2013/daikin-ir-protocol
/// @see Daikin https://github.com/blafois/Daikin-IR-Reverse

// Supports:
//   Brand: Daikin,  Model: ARC433** remote (DAIKIN)
//   Brand: Daikin,  Model: FTE12HV2S A/C
//   Brand: Daikin,  Model: M Series A/C (DAIKIN)
//   Brand: Daikin,  Model: FTXM-M A/C (DAIKIN)
//   Brand: Daikin,  Model: ARC466A12 remote (DAIKIN)
//   Brand: Daikin,  Model: ARC466A33 remote (DAIKIN)

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
using irutils::addTempFloatToString;
using irutils::addFanToString;
using irutils::bcdToUint8;
using irutils::minsToString;
using irutils::setBit;
using irutils::setBits;
using irutils::sumNibbles;
using irutils::uint8ToBcd;

#if SEND_DAIKIN
/// Send a Daikin 280-bit A/C formatted message.
/// Status: STABLE
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
/// @see https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/tree/master/IRremote
/// @see https://github.com/blafois/Daikin-IR-Reverse
void IRsend::sendDaikin(const unsigned char data[], const uint16_t nbytes,
                        const uint16_t repeat) {
  if (nbytes < kDaikinStateLengthShort)
    return;  // Not enough bytes to send a proper message.

  for (uint16_t r = 0; r <= repeat; r++) {
    uint16_t offset = 0;
    // Send the header, 0b00000
    sendGeneric(0, 0,  // No header for the header
                kDaikinBitMark, kDaikinOneSpace, kDaikinBitMark,
                kDaikinZeroSpace, kDaikinBitMark, kDaikinZeroSpace + kDaikinGap,
                (uint64_t)0b00000, kDaikinHeaderLength, 38, false, 0, 50);
    // Data #1
    if (nbytes < kDaikinStateLength) {  // Are we using the legacy size?
      // Do this as a constant to save RAM and keep in flash memory
      sendGeneric(kDaikinHdrMark, kDaikinHdrSpace, kDaikinBitMark,
                  kDaikinOneSpace, kDaikinBitMark, kDaikinZeroSpace,
                  kDaikinBitMark, kDaikinZeroSpace + kDaikinGap,
                  kDaikinFirstHeader64, 64, 38, false, 0, 50);
    } else {  // We are using the newer/more correct size.
      sendGeneric(kDaikinHdrMark, kDaikinHdrSpace, kDaikinBitMark,
                  kDaikinOneSpace, kDaikinBitMark, kDaikinZeroSpace,
                  kDaikinBitMark, kDaikinZeroSpace + kDaikinGap,
                  data, kDaikinSection1Length, 38, false, 0, 50);
      offset += kDaikinSection1Length;
    }
    // Data #2
    sendGeneric(kDaikinHdrMark, kDaikinHdrSpace, kDaikinBitMark,
                kDaikinOneSpace, kDaikinBitMark, kDaikinZeroSpace,
                kDaikinBitMark, kDaikinZeroSpace + kDaikinGap,
                data + offset, kDaikinSection2Length, 38, false, 0, 50);
    offset += kDaikinSection2Length;
    // Data #3
    sendGeneric(kDaikinHdrMark, kDaikinHdrSpace, kDaikinBitMark,
                kDaikinOneSpace, kDaikinBitMark, kDaikinZeroSpace,
                kDaikinBitMark, kDaikinZeroSpace + kDaikinGap,
                data + offset, nbytes - offset, 38, false, 0, 50);
  }
}
#endif  // SEND_DAIKIN

/// Class constructor.
/// @param[in] pin GPIO to be used when sending.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRDaikinESP::IRDaikinESP(const uint16_t pin, const bool inverted,
                         const bool use_modulation)
      : _irsend(pin, inverted, use_modulation) { stateReset(); }

/// Set up hardware to be able to send a message.
void IRDaikinESP::begin(void) { _irsend.begin(); }

#if SEND_DAIKIN
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRDaikinESP::send(const uint16_t repeat) {
  _irsend.sendDaikin(getRaw(), kDaikinStateLength, repeat);
}
#endif  // SEND_DAIKIN

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length of the state array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool IRDaikinESP::validChecksum(uint8_t state[], const uint16_t length) {
  // Data #1
  if (length < kDaikinSection1Length ||
      state[kDaikinByteChecksum1] != sumBytes(state, kDaikinSection1Length - 1))
    return false;
  // Data #2
  if (length < kDaikinSection1Length + kDaikinSection2Length ||
      state[kDaikinByteChecksum2] != sumBytes(state + kDaikinSection1Length,
                                              kDaikinSection2Length - 1))
    return false;
  // Data #3
  if (length < kDaikinSection1Length + kDaikinSection2Length + 2 ||
      state[length - 1] != sumBytes(state + kDaikinSection1Length +
                                    kDaikinSection2Length,
                                    length - (kDaikinSection1Length +
                                              kDaikinSection2Length) - 1))
    return false;
  return true;
}

/// Calculate and set the checksum values for the internal state.
void IRDaikinESP::checksum(void) {
  _.Sum1 = sumBytes(_.raw, kDaikinSection1Length - 1);
  _.Sum2 = sumBytes(_.raw + kDaikinSection1Length, kDaikinSection2Length - 1);
  _.Sum3 = sumBytes(_.raw + kDaikinSection1Length + kDaikinSection2Length,
                    kDaikinSection3Length - 1);
}

/// Reset the internal state to a fixed known good state.
void IRDaikinESP::stateReset(void) {
  for (uint8_t i = 0; i < kDaikinStateLength; i++) _.raw[i] = 0x0;

  _.raw[0] = 0x11;
  _.raw[1] = 0xDA;
  _.raw[2] = 0x27;
  _.raw[4] = 0xC5;
  // _.raw[7] is a checksum byte, it will be set by checksum().
  _.raw[8] = 0x11;
  _.raw[9] = 0xDA;
  _.raw[10] = 0x27;
  _.raw[12] = 0x42;
  // _.raw[15] is a checksum byte, it will be set by checksum().
  _.raw[16] = 0x11;
  _.raw[17] = 0xDA;
  _.raw[18] = 0x27;
  _.raw[21] = 0x49;
  _.raw[22] = 0x1E;
  _.raw[24] = 0xB0;
  _.raw[27] = 0x06;
  _.raw[28] = 0x60;
  _.raw[31] = 0xC0;
  // _.raw[34] is a checksum byte, it will be set by checksum().
  checksum();
}

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *IRDaikinESP::getRaw(void) {
  checksum();  // Ensure correct settings before sending.
  return _.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
/// @param[in] length Length of the code in bytes.
void IRDaikinESP::setRaw(const uint8_t new_code[], const uint16_t length) {
  uint8_t offset = 0;
  if (length == kDaikinStateLengthShort) {  // Handle the "short" length case.
    offset = kDaikinStateLength - kDaikinStateLengthShort;
    stateReset();
  }
  for (uint8_t i = 0; i < length && i < kDaikinStateLength; i++)
    _.raw[i + offset] = new_code[i];
}

/// Change the power setting to On.
void IRDaikinESP::on(void) { setPower(true); }

/// Change the power setting to Off.
void IRDaikinESP::off(void) { setPower(false); }

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikinESP::setPower(const bool on) {
  _.Power = on;
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikinESP::getPower(void) const {
  return _.Power;
}

/// Set the temperature.
/// @param[in] temp The temperature in degrees celsius.
void IRDaikinESP::setTemp(const float temp) {
  float degrees = std::max(temp, static_cast<float>(kDaikinMinTemp));
  degrees = std::min(degrees, static_cast<float>(kDaikinMaxTemp));
  _.Temp = degrees * 2.0f;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
float IRDaikinESP::getTemp(void) const { return _.Temp / 2.0f; }

/// Set the speed of the fan.
/// @param[in] fan The desired setting.
/// @note 1-5 or kDaikinFanAuto or kDaikinFanQuiet
void IRDaikinESP::setFan(const uint8_t fan) {
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
uint8_t IRDaikinESP::getFan(void) const {
  uint8_t fan = _.Fan;
  if (fan != kDaikinFanQuiet && fan != kDaikinFanAuto) fan -= 2;
  return fan;
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t IRDaikinESP::getMode(void) const {
  return _.Mode;
}

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void IRDaikinESP::setMode(const uint8_t mode) {
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

/// Set the Vertical Swing mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikinESP::setSwingVertical(const bool on) {
  _.SwingV = (on ? kDaikinSwingOn : kDaikinSwingOff);
}

/// Get the Vertical Swing mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikinESP::getSwingVertical(void) const {
  return _.SwingV;
}

/// Set the Horizontal Swing mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikinESP::setSwingHorizontal(const bool on) {
  _.SwingH = (on ? kDaikinSwingOn : kDaikinSwingOff);
}

/// Get the Horizontal Swing mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikinESP::getSwingHorizontal(void) const {
  return _.SwingH;
}

/// Set the Quiet mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikinESP::setQuiet(const bool on) {
  _.Quiet = on;
  // Powerful & Quiet mode being on are mutually exclusive.
  if (on) setPowerful(false);
}

/// Get the Quiet mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikinESP::getQuiet(void) const {
  return _.Quiet;
}

/// Set the Powerful (Turbo) mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikinESP::setPowerful(const bool on) {
  _.Powerful = on;
  if (on) {
    // Powerful, Quiet, & Econo mode being on are mutually exclusive.
    setQuiet(false);
    setEcono(false);
  }
}

/// Get the Powerful (Turbo) mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikinESP::getPowerful(void) const {
  return _.Powerful;
}

/// Set the Sensor mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikinESP::setSensor(const bool on) {
  _.Sensor = on;
}

/// Get the Sensor mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikinESP::getSensor(void) const {
  return _.Sensor;
}

/// Set the Economy mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikinESP::setEcono(const bool on) {
  _.Econo = on;
  // Powerful & Econo mode being on are mutually exclusive.
  if (on) setPowerful(false);
}

/// Get the Economical mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikinESP::getEcono(void) const {
  return _.Econo;
}

/// Set the Mould mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikinESP::setMold(const bool on) {
  _.Mold = on;
}

/// Get the Mould mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikinESP::getMold(void) const {
  return _.Mold;
}

/// Set the Comfort mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikinESP::setComfort(const bool on) {
  _.Comfort = on;
}

/// Get the Comfort mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikinESP::getComfort(void) const {
  return _.Comfort;
}

/// Set the enable status & time of the On Timer.
/// @param[in] starttime The number of minutes past midnight.
void IRDaikinESP::enableOnTimer(const uint16_t starttime) {
  _.OnTimer = true;
  _.OnTime = starttime;
}

/// Clear and disable the On timer.
void IRDaikinESP::disableOnTimer(void) {
  _.OnTimer = false;
  _.OnTime = kDaikinUnusedTime;
}

/// Get the On Timer time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t IRDaikinESP::getOnTime(void) const {
  return _.OnTime;
}

/// Get the enable status of the On Timer.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikinESP::getOnTimerEnabled(void) const {
  return _.OnTimer;
}

/// Set the enable status & time of the Off Timer.
/// @param[in] endtime The number of minutes past midnight.
void IRDaikinESP::enableOffTimer(const uint16_t endtime) {
  _.OffTimer = true;
  _.OffTime = endtime;
}

/// Clear and disable the Off timer.
void IRDaikinESP::disableOffTimer(void) {
  _.OffTimer = false;
  _.OffTime = kDaikinUnusedTime;
}

/// Get the Off Timer time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t IRDaikinESP::getOffTime(void) const {
  return _.OffTime;
}

/// Get the enable status of the Off Timer.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikinESP::getOffTimerEnabled(void) const {
  return _.OffTimer;
}

/// Set the clock on the A/C unit.
/// @param[in] mins_since_midnight Nr. of minutes past midnight.
void IRDaikinESP::setCurrentTime(const uint16_t mins_since_midnight) {
  uint16_t mins = mins_since_midnight;
  if (mins > 24 * 60) mins = 0;  // If > 23:59, set to 00:00
  _.CurrentTime = mins;
}

/// Get the clock time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t IRDaikinESP::getCurrentTime(void) const {
  return _.CurrentTime;
}

/// Set the current day of the week to be sent to the A/C unit.
/// @param[in] day_of_week The numerical representation of the day of the week.
/// @note 1 is SUN, 2 is MON, ..., 7 is SAT
void IRDaikinESP::setCurrentDay(const uint8_t day_of_week) {
  _.CurrentDay = day_of_week;
}

/// Get the current day of the week to be sent to the A/C unit.
/// @return The numerical representation of the day of the week.
/// @note 1 is SUN, 2 is MON, ..., 7 is SAT
uint8_t IRDaikinESP::getCurrentDay(void) const {
  return _.CurrentDay;
}

/// Set the enable status of the Weekly Timer.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikinESP::setWeeklyTimerEnable(const bool on) {
  // Bit is cleared for `on`.
  _.WeeklyTimer = !on;
}

/// Get the enable status of the Weekly Timer.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikinESP::getWeeklyTimerEnable(void) const {
  return !_.WeeklyTimer;
}

/// Convert a stdAc::opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRDaikinESP::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kCool: return kDaikinCool;
    case stdAc::opmode_t::kHeat: return kDaikinHeat;
    case stdAc::opmode_t::kDry: return kDaikinDry;
    case stdAc::opmode_t::kFan: return kDaikinFan;
    default: return kDaikinAuto;
  }
}

/// Convert a stdAc::fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRDaikinESP::convertFan(const stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin: return kDaikinFanQuiet;
    case stdAc::fanspeed_t::kLow: return kDaikinFanMin;
    case stdAc::fanspeed_t::kMedium: return kDaikinFanMed;
    case stdAc::fanspeed_t::kHigh: return kDaikinFanMax - 1;
    case stdAc::fanspeed_t::kMax: return kDaikinFanMax;
    default: return kDaikinFanAuto;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::opmode_t IRDaikinESP::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kDaikinCool: return stdAc::opmode_t::kCool;
    case kDaikinHeat: return stdAc::opmode_t::kHeat;
    case kDaikinDry: return stdAc::opmode_t::kDry;
    case kDaikinFan: return stdAc::opmode_t::kFan;
    default: return stdAc::opmode_t::kAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::fanspeed_t IRDaikinESP::toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kDaikinFanMax: return stdAc::fanspeed_t::kMax;
    case kDaikinFanMax - 1: return stdAc::fanspeed_t::kHigh;
    case kDaikinFanMed:
    case kDaikinFanMin + 1: return stdAc::fanspeed_t::kMedium;
    case kDaikinFanMin: return stdAc::fanspeed_t::kLow;
    case kDaikinFanQuiet: return stdAc::fanspeed_t::kMin;
    default: return stdAc::fanspeed_t::kAuto;
  }
}

/// Convert the current internal state into its stdAc::state_t equivalent.
/// @return The stdAc equivalent of the native settings.
stdAc::state_t IRDaikinESP::toCommon(void) const {
  stdAc::state_t result{};
  result.protocol = decode_type_t::DAIKIN;
  result.model = -1;  // No models used.
  result.power = _.Power;
  result.mode = toCommonMode(_.Mode);
  result.celsius = true;
  result.degrees = getTemp();
  result.fanspeed = toCommonFanSpeed(getFan());
  result.swingv = _.SwingV ? stdAc::swingv_t::kAuto :
                                             stdAc::swingv_t::kOff;
  result.swingh = _.SwingH ? stdAc::swingh_t::kAuto :
                                               stdAc::swingh_t::kOff;
  result.quiet = _.Quiet;
  result.turbo = _.Powerful;
  result.clean = _.Mold;
  result.econo = _.Econo;
  // Not supported.
  result.filter = false;
  result.light = false;
  result.beep = false;
  result.sleep = -1;
  result.clock = -1;
  return result;
}

/// Convert the current internal state into a human readable string.
/// @return A human readable string.
String IRDaikinESP::toString(void) const {
  String result = "";
  result.reserve(230);  // Reserve some heap for the string to reduce fragging.
  result += addBoolToString(_.Power, kPowerStr, false);
  result += addModeToString(_.Mode, kDaikinAuto, kDaikinCool, kDaikinHeat,
                            kDaikinDry, kDaikinFan);
  result += addTempFloatToString(getTemp());
  result += addFanToString(getFan(), kDaikinFanMax, kDaikinFanMin,
                           kDaikinFanAuto, kDaikinFanQuiet, kDaikinFanMed);
  result += addBoolToString(_.Powerful, kPowerfulStr);
  result += addBoolToString(_.Quiet, kQuietStr);
  result += addBoolToString(getSensor(), kSensorStr);
  result += addBoolToString(_.Mold, kMouldStr);
  result += addBoolToString(_.Comfort, kComfortStr);
  result += addBoolToString(_.SwingH, kSwingHStr);
  result += addBoolToString(_.SwingV, kSwingVStr);
  result += addLabeledString(minsToString(_.CurrentTime), kClockStr);
  result += addDayToString(_.CurrentDay, -1);
  result += addLabeledString(_.OnTimer
                             ? minsToString(_.OnTime) : kOffStr,
                             kOnTimerStr);
  result += addLabeledString(_.OffTimer
                             ? minsToString(_.OffTime) : kOffStr,
                             kOffTimerStr);
  result += addBoolToString(getWeeklyTimerEnable(), kWeeklyTimerStr);
  return result;
}

#if DECODE_DAIKIN
/// Decode the supplied Daikin 280-bit message. (DAIKIN)
/// Status: STABLE / Reported as working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
/// @see https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/tree/master/IRremote
bool IRrecv::decodeDaikin(decode_results *results, uint16_t offset,
                          const uint16_t nbits, const bool strict) {
  // Is there enough data to match successfully?
  if (results->rawlen < (2 * (nbits + kDaikinHeaderLength) +
                         kDaikinSections * (kHeader + kFooter) + kFooter - 1) +
                         offset)
    return false;

  // Compliance
  if (strict && nbits != kDaikinBits) return false;

  match_result_t data_result;

  // Header #1 - Doesn't count as data.
  data_result = matchData(&(results->rawbuf[offset]), kDaikinHeaderLength,
                          kDaikinBitMark, kDaikinOneSpace,
                          kDaikinBitMark, kDaikinZeroSpace,
                          kDaikinTolerance, kDaikinMarkExcess, false);
  offset += data_result.used;
  if (data_result.success == false) return false;  // Fail
  if (data_result.data) return false;  // The header bits should be zero.
  // Footer
  if (!matchMark(results->rawbuf[offset++], kDaikinBitMark,
                 kDaikinTolerance, kDaikinMarkExcess)) return false;
  if (!matchSpace(results->rawbuf[offset++], kDaikinZeroSpace + kDaikinGap,
                  kDaikinTolerance, kDaikinMarkExcess)) return false;
  // Sections
  const uint8_t ksectionSize[kDaikinSections] = {
      kDaikinSection1Length, kDaikinSection2Length, kDaikinSection3Length};
  uint16_t pos = 0;
  for (uint8_t section = 0; section < kDaikinSections; section++) {
    uint16_t used;
    // Section Header + Section Data (7 bytes) + Section Footer
    used = matchGeneric(results->rawbuf + offset, results->state + pos,
                        results->rawlen - offset, ksectionSize[section] * 8,
                        kDaikinHdrMark, kDaikinHdrSpace,
                        kDaikinBitMark, kDaikinOneSpace,
                        kDaikinBitMark, kDaikinZeroSpace,
                        kDaikinBitMark, kDaikinZeroSpace + kDaikinGap,
                        section >= kDaikinSections - 1,
                        kDaikinTolerance, kDaikinMarkExcess, false);
    if (used == 0) return false;
    offset += used;
    pos += ksectionSize[section];
  }
  // Compliance
  if (strict) {
    // Re-check we got the correct size/length due to the way we read the data.
    if (pos * 8 != kDaikinBits) return false;
    // Validate the checksum.
    if (!IRDaikinESP::validChecksum(results->state)) return false;
  }

  // Success
  results->decode_type = DAIKIN;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // DECODE_DAIKIN


