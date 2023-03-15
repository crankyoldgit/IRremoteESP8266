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


#if SEND_DAIKIN2
/// Send a Daikin2 (312-bit) A/C formatted message.
/// Status: STABLE / Expected to work.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/582
void IRsend::sendDaikin2(const unsigned char data[], const uint16_t nbytes,
                         const uint16_t repeat) {
  if (nbytes < kDaikin2Section1Length)
    return;  // Not enough bytes to send a partial message.

  for (uint16_t r = 0; r <= repeat; r++) {
    // Leader
    sendGeneric(kDaikin2LeaderMark, kDaikin2LeaderSpace,
                0, 0, 0, 0, 0, 0, (uint64_t) 0,  // No data payload.
                0, kDaikin2Freq, false, 0, 50);
    // Section #1
    sendGeneric(kDaikin2HdrMark, kDaikin2HdrSpace, kDaikin2BitMark,
                kDaikin2OneSpace, kDaikin2BitMark, kDaikin2ZeroSpace,
                kDaikin2BitMark, kDaikin2Gap, data, kDaikin2Section1Length,
                kDaikin2Freq, false, 0, 50);
    // Section #2
    sendGeneric(kDaikin2HdrMark, kDaikin2HdrSpace, kDaikin2BitMark,
                kDaikin2OneSpace, kDaikin2BitMark, kDaikin2ZeroSpace,
                kDaikin2BitMark, kDaikin2Gap, data + kDaikin2Section1Length,
                nbytes - kDaikin2Section1Length,
                kDaikin2Freq, false, 0, 50);
  }
}
#endif  // SEND_DAIKIN2

/// Class constructor.
/// @param[in] pin GPIO to be used when sending.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRDaikin2::IRDaikin2(const uint16_t pin, const bool inverted,
                     const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) { stateReset(); }

/// Set up hardware to be able to send a message.
void IRDaikin2::begin(void) { _irsend.begin(); }

#if SEND_DAIKIN2
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRDaikin2::send(const uint16_t repeat) {
  _irsend.sendDaikin2(getRaw(), kDaikin2StateLength, repeat);
}
#endif  // SEND_DAIKIN2

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length of the state array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool IRDaikin2::validChecksum(uint8_t state[], const uint16_t length) {
  // Validate the checksum of section #1.
  if (length <= kDaikin2Section1Length - 1 ||
      state[kDaikin2Section1Length - 1] != sumBytes(state,
                                                    kDaikin2Section1Length - 1))
    return false;
  // Validate the checksum of section #2 (a.k.a. the rest)
  if (length <= kDaikin2Section1Length + 1 ||
      state[length - 1] != sumBytes(state + kDaikin2Section1Length,
                                    length - kDaikin2Section1Length - 1))
    return false;
  return true;
}

/// Calculate and set the checksum values for the internal state.
void IRDaikin2::checksum(void) {
  _.Sum1 = sumBytes(_.raw, kDaikin2Section1Length - 1);
  _.Sum2 = sumBytes(_.raw + kDaikin2Section1Length, kDaikin2Section2Length - 1);
}

/// Reset the internal state to a fixed known good state.
void IRDaikin2::stateReset(void) {
  for (uint8_t i = 0; i < kDaikin2StateLength; i++) _.raw[i] = 0x0;

  _.raw[0] = 0x11;
  _.raw[1] = 0xDA;
  _.raw[2] = 0x27;
  _.raw[4] = 0x01;
  _.raw[6] = 0xC0;
  _.raw[7] = 0x70;
  _.raw[8] = 0x08;
  _.raw[9] = 0x0C;
  _.raw[10] = 0x80;
  _.raw[11] = 0x04;
  _.raw[12] = 0xB0;
  _.raw[13] = 0x16;
  _.raw[14] = 0x24;
  _.raw[17] = 0xBE;
  _.raw[18] = 0xD0;
  // _.raw[19] is a checksum byte, it will be set by checksum().
  _.raw[20] = 0x11;
  _.raw[21] = 0xDA;
  _.raw[22] = 0x27;
  _.raw[25] = 0x08;
  _.raw[28] = 0xA0;
  _.raw[35] = 0xC1;
  _.raw[36] = 0x80;
  _.raw[37] = 0x60;
  // _.raw[38] is a checksum byte, it will be set by checksum().
  disableOnTimer();
  disableOffTimer();
  disableSleepTimer();
  checksum();
}

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t *IRDaikin2::getRaw(void) {
  checksum();  // Ensure correct settings before sending.
  return _.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
void IRDaikin2::setRaw(const uint8_t new_code[]) {
  std::memcpy(_.raw, new_code, kDaikin2StateLength);
}

/// Change the power setting to On.
void IRDaikin2::on(void) { setPower(true); }

/// Change the power setting to Off.
void IRDaikin2::off(void) { setPower(false); }

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin2::setPower(const bool on) {
  _.Power = on;
  _.Power2 = !on;
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin2::getPower(void) const { return _.Power && !_.Power2; }

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t IRDaikin2::getMode(void) const { return _.Mode; }

/// Set the operating mode of the A/C.
/// @param[in] desired_mode The desired operating mode.
void IRDaikin2::setMode(const uint8_t desired_mode) {
  uint8_t mode = desired_mode;
  switch (mode) {
    case kDaikinCool:
    case kDaikinHeat:
    case kDaikinFan:
    case kDaikinDry: break;
    default: mode = kDaikinAuto;
  }
  _.Mode = mode;
  // Redo the temp setting as Cool mode has a different min temp.
  if (mode == kDaikinCool) setTemp(getTemp());
  setHumidity(getHumidity());  // Make sure the humidity is okay for this mode.
}

/// Set the temperature.
/// @param[in] desired The temperature in degrees celsius.
void IRDaikin2::setTemp(const uint8_t desired) {
  // The A/C has a different min temp if in cool mode.
  uint8_t temp = std::max(
      (_.Mode == kDaikinCool) ? kDaikin2MinCoolTemp : kDaikinMinTemp,
      desired);
  _.Temp = std::min(kDaikinMaxTemp, temp);
  // If the humidity setting is in use, the temp is a fixed value.
  if (_.HumidOn) _.Temp = kDaikinMaxTemp;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t IRDaikin2::getTemp(void) const { return _.Temp; }

/// Set the speed of the fan.
/// @param[in] fan The desired setting.
/// @note 1-5 or kDaikinFanAuto or kDaikinFanQuiet
void IRDaikin2::setFan(const uint8_t fan) {
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
uint8_t IRDaikin2::getFan(void) const {
  const uint8_t fan = _.Fan;
  switch (fan) {
    case kDaikinFanAuto:
    case kDaikinFanQuiet: return fan;
    default: return fan - 2;
  }
}

/// Set the Vertical Swing mode of the A/C.
/// @param[in] position The position/mode to set the swing to.
void IRDaikin2::setSwingVertical(const uint8_t position) {
  switch (position) {
    case kDaikin2SwingVHighest:
    case kDaikin2SwingVHigh:
    case kDaikin2SwingVUpperMiddle:
    case kDaikin2SwingVLowerMiddle:
    case kDaikin2SwingVLow:
    case kDaikin2SwingVLowest:
    case kDaikin2SwingVOff:
    case kDaikin2SwingVBreeze:
    case kDaikin2SwingVCirculate:
    case kDaikin2SwingVAuto:
      _.SwingV = position;
  }
}

/// Get the Vertical Swing mode of the A/C.
/// @return The native position/mode setting.
uint8_t IRDaikin2::getSwingVertical(void) const { return _.SwingV; }

/// Convert a stdAc::swingv_t enum into it's native setting.
/// @param[in] position The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRDaikin2::convertSwingV(const stdAc::swingv_t position) {
  switch (position) {
    case stdAc::swingv_t::kHighest:
    case stdAc::swingv_t::kHigh:
    case stdAc::swingv_t::kMiddle:
    case stdAc::swingv_t::kLow:
    case stdAc::swingv_t::kLowest:
      return (uint8_t)position + kDaikin2SwingVHighest;
    case stdAc::swingv_t::kOff:
      return kDaikin2SwingVOff;
    default:
      return kDaikin2SwingVAuto;
  }
}

/// Convert a native vertical swing postion to it's common equivalent.
/// @param[in] setting A native position to convert.
/// @return The common vertical swing position.
stdAc::swingv_t IRDaikin2::toCommonSwingV(const uint8_t setting) {
  switch (setting) {
    case kDaikin2SwingVHighest:     return stdAc::swingv_t::kHighest;
    case kDaikin2SwingVHigh:        return stdAc::swingv_t::kHigh;
    case kDaikin2SwingVUpperMiddle:
    case kDaikin2SwingVLowerMiddle: return stdAc::swingv_t::kMiddle;
    case kDaikin2SwingVLow:         return stdAc::swingv_t::kLow;
    case kDaikin2SwingVLowest:      return stdAc::swingv_t::kLowest;
    case kDaikin2SwingVOff:         return stdAc::swingv_t::kOff;
    default:                        return stdAc::swingv_t::kAuto;
  }
}

/// Set the Horizontal Swing mode of the A/C.
/// @param[in] position The position/mode to set the swing to.
void IRDaikin2::setSwingHorizontal(const uint8_t position) {
  _.SwingH = position;
}

/// Get the Horizontal Swing mode of the A/C.
/// @return The native position/mode setting.
uint8_t IRDaikin2::getSwingHorizontal(void) const { return _.SwingH; }

/// Set the clock on the A/C unit.
/// @param[in] numMins Nr. of minutes past midnight.
void IRDaikin2::setCurrentTime(const uint16_t numMins) {
  uint16_t mins = numMins;
  if (numMins > 24 * 60) mins = 0;  // If > 23:59, set to 00:00
  _.CurrentTime = mins;
}

/// Get the clock time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t IRDaikin2::getCurrentTime(void) const { return _.CurrentTime; }

/// Set the enable status & time of the On Timer.
/// @param[in] starttime The number of minutes past midnight.
/// @note Timer location is shared with sleep timer.
void IRDaikin2::enableOnTimer(const uint16_t starttime) {
  clearSleepTimerFlag();
  _.OnTimer = true;
  _.OnTime = starttime;
}

/// Clear the On Timer flag.
void IRDaikin2::clearOnTimerFlag(void) { _.OnTimer = false; }

/// Disable the On timer.
void IRDaikin2::disableOnTimer(void) {
  _.OnTime = kDaikinUnusedTime;
  clearOnTimerFlag();
  clearSleepTimerFlag();
}

/// Get the On Timer time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t IRDaikin2::getOnTime(void) const { return _.OnTime; }

/// Get the enable status of the On Timer.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin2::getOnTimerEnabled(void) const { return _.OnTimer; }

/// Set the enable status & time of the Off Timer.
/// @param[in] endtime The number of minutes past midnight.
void IRDaikin2::enableOffTimer(const uint16_t endtime) {
  // Set the Off Timer flag.
  _.OffTimer = true;
  _.OffTime = endtime;
}

/// Disable the Off timer.
void IRDaikin2::disableOffTimer(void) {
  _.OffTime = kDaikinUnusedTime;
  // Clear the Off Timer flag.
  _.OffTimer = false;
}

/// Get the Off Timer time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t IRDaikin2::getOffTime(void) const { return _.OffTime; }

/// Get the enable status of the Off Timer.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin2::getOffTimerEnabled(void) const { return _.OffTimer; }

/// Get the Beep status of the A/C.
/// @return true, the setting is on. false, the setting is off.
uint8_t IRDaikin2::getBeep(void) const { return _.Beep; }

/// Set the Beep mode of the A/C.
/// @param[in] beep true, the setting is on. false, the setting is off.
void IRDaikin2::setBeep(const uint8_t beep) { _.Beep = beep; }

/// Get the Light status of the A/C.
/// @return true, the setting is on. false, the setting is off.
uint8_t IRDaikin2::getLight(void) const { return _.Light; }

/// Set the Light (LED) mode of the A/C.
/// @param[in] light true, the setting is on. false, the setting is off.
void IRDaikin2::setLight(const uint8_t light) { _.Light = light; }

/// Set the Mould (filter) mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin2::setMold(const bool on) { _.Mold = on; }

/// Get the Mould (filter) mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin2::getMold(void) const { return _.Mold; }

/// Set the Auto clean mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin2::setClean(const bool on) { _.Clean = on; }

/// Get the Auto Clean mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin2::getClean(void) const { return _.Clean; }

/// Set the Fresh Air mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin2::setFreshAir(const bool on) { _.FreshAir = on; }

/// Get the Fresh Air mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin2::getFreshAir(void) const { return _.FreshAir; }

/// Set the (High) Fresh Air mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin2::setFreshAirHigh(const bool on) { _.FreshAirHigh = on; }

/// Get the (High) Fresh Air mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin2::getFreshAirHigh(void) const { return _.FreshAirHigh; }

/// Set the Automatic Eye (Sensor) mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin2::setEyeAuto(bool on) { _.EyeAuto = on; }

/// Get the Automaitc Eye (Sensor) mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin2::getEyeAuto(void) const { return _.EyeAuto; }

/// Set the Eye (Sensor) mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin2::setEye(bool on) { _.Eye = on; }

/// Get the Eye (Sensor) mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin2::getEye(void) const { return _.Eye; }

/// Set the Economy mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin2::setEcono(bool on) { _.Econo = on; }

/// Get the Economical mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin2::getEcono(void) const { return _.Econo; }

/// Set the enable status & time of the Sleep Timer.
/// @param[in] sleeptime The number of minutes past midnight.
/// @note The Timer location is shared with On Timer.
void IRDaikin2::enableSleepTimer(const uint16_t sleeptime) {
  enableOnTimer(sleeptime);
  clearOnTimerFlag();
  _.SleepTimer = true;
}

/// Clear the sleep timer flag.
void IRDaikin2::clearSleepTimerFlag(void) { _.SleepTimer = false; }

/// Disable the sleep timer.
void IRDaikin2::disableSleepTimer(void) { disableOnTimer(); }

/// Get the Sleep Timer time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t IRDaikin2::getSleepTime(void) const { return getOnTime(); }

/// Get the Sleep timer enabled status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin2::getSleepTimerEnabled(void) const { return _.SleepTimer; }

/// Set the Quiet mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin2::setQuiet(const bool on) {
  _.Quiet = on;
  // Powerful & Quiet mode being on are mutually exclusive.
  if (on) setPowerful(false);
}

/// Get the Quiet mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin2::getQuiet(void) const { return _.Quiet; }

/// Set the Powerful (Turbo) mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin2::setPowerful(const bool on) {
  _.Powerful = on;
  // Powerful & Quiet mode being on are mutually exclusive.
  if (on) setQuiet(false);
}

/// Get the Powerful (Turbo) mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin2::getPowerful(void) const { return _.Powerful; }

/// Set the Purify (Filter) mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRDaikin2::setPurify(const bool on) { _.Purify = on; }

/// Get the Purify (Filter) mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRDaikin2::getPurify(void) const { return _.Purify; }

/// Get the Humidity percentage setting of the A/C.
/// @return The setting percentage. 255 is Automatic. 0 is Off.
uint8_t IRDaikin2::getHumidity(void) const { return _.Humidity; }

/// Set the Humidity percentage setting of the A/C.
/// @param[in] percent Percentage humidty. 255 is Auto. 0 is Off.
/// @note Only available in Dry & Heat modes, otherwise it is Off.
void IRDaikin2::setHumidity(const uint8_t percent) {
  _.Humidity = kDaikin2HumidityOff;  // Default to off.
  switch (getMode()) {
    case kDaikinHeat:
      switch (percent) {
        case kDaikin2HumidityOff:
        case kDaikin2HumidityHeatLow:
        case kDaikin2HumidityHeatMedium:
        case kDaikin2HumidityHeatHigh:
        case kDaikin2HumidityAuto:
          _.Humidity = percent;
      }
      break;
    case kDaikinDry:
      switch (percent) {
        case kDaikin2HumidityOff:
        case kDaikin2HumidityDryLow:
        case kDaikin2HumidityDryMedium:
        case kDaikin2HumidityDryHigh:
        case kDaikin2HumidityAuto:
          _.Humidity = percent;
      }
      break;
  }
  _.HumidOn = (_.Humidity != kDaikin2HumidityOff);  // Enabled?
  setTemp(getTemp());  // Adjust the temperature if we need to.
}

/// Convert a stdAc::opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRDaikin2::convertMode(const stdAc::opmode_t mode) {
  return IRDaikinESP::convertMode(mode);
}

/// Convert a stdAc::fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRDaikin2::convertFan(const stdAc::fanspeed_t speed) {
  return IRDaikinESP::convertFan(speed);
}

/// Convert a stdAc::swingh_t enum into it's native setting.
/// @param[in] position The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRDaikin2::convertSwingH(const stdAc::swingh_t position) {
  switch (position) {
    case stdAc::swingh_t::kAuto:     return kDaikin2SwingHSwing;
    case stdAc::swingh_t::kLeftMax:  return kDaikin2SwingHLeftMax;
    case stdAc::swingh_t::kLeft:     return kDaikin2SwingHLeft;
    case stdAc::swingh_t::kMiddle:   return kDaikin2SwingHMiddle;
    case stdAc::swingh_t::kRight:    return kDaikin2SwingHRight;
    case stdAc::swingh_t::kRightMax: return kDaikin2SwingHRightMax;
    case stdAc::swingh_t::kWide:     return kDaikin2SwingHWide;
    default:                         return kDaikin2SwingHAuto;
  }
}

/// Convert a native horizontal swing postion to it's common equivalent.
/// @param[in] setting A native position to convert.
/// @return The common horizontal swing position.
stdAc::swingh_t IRDaikin2::toCommonSwingH(const uint8_t setting) {
  switch (setting) {
    case kDaikin2SwingHSwing:    return stdAc::swingh_t::kAuto;
    case kDaikin2SwingHLeftMax:  return stdAc::swingh_t::kLeftMax;
    case kDaikin2SwingHLeft:     return stdAc::swingh_t::kLeft;
    case kDaikin2SwingHMiddle:   return stdAc::swingh_t::kMiddle;
    case kDaikin2SwingHRight:    return stdAc::swingh_t::kRight;
    case kDaikin2SwingHRightMax: return stdAc::swingh_t::kRightMax;
    case kDaikin2SwingHWide:     return stdAc::swingh_t::kWide;
    default:                     return stdAc::swingh_t::kOff;
  }
}

/// Convert the current internal state into its stdAc::state_t equivalent.
/// @return The stdAc equivalent of the native settings.
stdAc::state_t IRDaikin2::toCommon(void) const {
  stdAc::state_t result{};
  result.protocol = decode_type_t::DAIKIN2;
  result.model = -1;  // No models used.
  result.power = getPower();
  result.mode = IRDaikinESP::toCommonMode(_.Mode);
  result.celsius = true;
  result.degrees = _.Temp;
  result.fanspeed = IRDaikinESP::toCommonFanSpeed(getFan());
  result.swingv = toCommonSwingV(_.SwingV);
  result.swingh = toCommonSwingH(_.SwingH);
  result.quiet = _.Quiet;
  result.light = _.Light != 3;  // 3 is Off, everything else is On.
  result.turbo = _.Powerful;
  result.clean = _.Mold;
  result.econo = _.Econo;
  result.filter = _.Purify;
  result.beep = _.Beep != 3;  // 3 is Off, everything else is On.
  result.sleep = _.SleepTimer ? getSleepTime() : -1;
  // Not supported.
  result.clock = -1;
  return result;
}

/// Convert the current internal state into a human readable string.
/// @return A human readable string.
String IRDaikin2::toString(void) const {
  String result = "";
  result.reserve(330);  // Reserve some heap for the string to reduce fragging.
  result += addBoolToString(getPower(), kPowerStr, false);
  result += addModeToString(_.Mode, kDaikinAuto, kDaikinCool, kDaikinHeat,
                            kDaikinDry, kDaikinFan);
  result += addTempToString(_.Temp);
  result += addFanToString(getFan(), kDaikinFanMax, kDaikinFanMin,
                           kDaikinFanAuto, kDaikinFanQuiet, kDaikinFanMed);
  result += addSwingVToString(_.SwingV, kDaikin2SwingVAuto,
                              kDaikin2SwingVHighest, kDaikin2SwingVHigh,
                              kDaikin2SwingVUpperMiddle,
                              kDaikin2SwingVAuto,  // Middle is unused.
                              kDaikin2SwingVLowerMiddle,
                              kDaikin2SwingVLow, kDaikin2SwingVLowest,
                              kDaikin2SwingVOff,  // Off is unused
                              kDaikin2SwingVSwing, kDaikin2SwingVBreeze,
                              kDaikin2SwingVCirculate);
  result += addSwingHToString(_.SwingH, kDaikin2SwingHAuto,
                              kDaikin2SwingHLeftMax,
                              kDaikin2SwingHLeft,
                              kDaikin2SwingHMiddle,
                              kDaikin2SwingHRight,
                              kDaikin2SwingHRightMax,
                              kDaikin2SwingHOff,
                              kDaikin2SwingHAuto,  // Unused
                              kDaikin2SwingHAuto,  // Unused
                              kDaikin2SwingHAuto,  // Unused
                              kDaikin2SwingHWide);
  result += addLabeledString(minsToString(_.CurrentTime), kClockStr);
  result += addLabeledString(
      _.OnTimer ? minsToString(_.OnTime) : kOffStr, kOnTimerStr);
  result += addLabeledString(
      _.OffTimer ? minsToString(_.OffTime) : kOffStr,
      kOffTimerStr);
  result += addLabeledString(
      _.SleepTimer ? minsToString(getSleepTime()) : kOffStr,
      kSleepTimerStr);
  result += addIntToString(_.Beep, kBeepStr);
  result += kSpaceLBraceStr;
  switch (_.Beep) {
    case kDaikinBeepLoud:
      result += kLoudStr;
      break;
    case kDaikinBeepQuiet:
      result += kQuietStr;
      break;
    case kDaikinBeepOff:
      result += kOffStr;
      break;
    default:
      result += kUnknownStr;
  }
  result += ')';
  result += addIntToString(_.Light, kLightStr);
  result += kSpaceLBraceStr;
  switch (_.Light) {
    case kDaikinLightBright:
      result += kHighStr;
      break;
    case kDaikinLightDim:
      result += kLowStr;
      break;
    case kDaikinLightOff:
      result += kOffStr;
      break;
    default:
      result += kUnknownStr;
  }
  result += ')';
  result += addBoolToString(_.Mold, kMouldStr);
  result += addBoolToString(_.Clean, kCleanStr);
  result += addLabeledString(
      _.FreshAir ? (_.FreshAirHigh ? kHighStr : kOnStr) : kOffStr,
      kFreshStr);
  result += addBoolToString(_.Eye, kEyeStr);
  result += addBoolToString(_.EyeAuto, kEyeAutoStr);
  result += addBoolToString(_.Quiet, kQuietStr);
  result += addBoolToString(_.Powerful, kPowerfulStr);
  result += addBoolToString(_.Purify, kPurifyStr);
  result += addBoolToString(_.Econo, kEconoStr);
  result += addIntToString(_.Humidity, kHumidStr);
  switch (_.Humidity) {
    case kDaikin2HumidityOff:
    case kDaikin2HumidityAuto:
      result += kSpaceLBraceStr;
      result += _.Humidity ? kAutoStr : kOffStr;
      result += ')';
      break;
    default:
      result += '%';
  }
  return result;
}

#if DECODE_DAIKIN2
/// Decode the supplied Daikin 312-bit message. (DAIKIN2)
/// Status: STABLE / Works as expected.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool IRrecv::decodeDaikin2(decode_results *results, uint16_t offset,
                           const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * (nbits + kHeader + kFooter) + kHeader - 1 + offset)
    return false;

  // Compliance
  if (strict && nbits != kDaikin2Bits) return false;

  const uint8_t ksectionSize[kDaikin2Sections] = {kDaikin2Section1Length,
                                                  kDaikin2Section2Length};

  // Leader
  if (!matchMark(results->rawbuf[offset++], kDaikin2LeaderMark,
                 _tolerance + kDaikin2Tolerance)) return false;
  if (!matchSpace(results->rawbuf[offset++], kDaikin2LeaderSpace,
                  _tolerance + kDaikin2Tolerance)) return false;

  // Sections
  uint16_t pos = 0;
  for (uint8_t section = 0; section < kDaikin2Sections; section++) {
    uint16_t used;
    // Section Header + Section Data + Section Footer
    used = matchGeneric(results->rawbuf + offset, results->state + pos,
                        results->rawlen - offset, ksectionSize[section] * 8,
                        kDaikin2HdrMark, kDaikin2HdrSpace,
                        kDaikin2BitMark, kDaikin2OneSpace,
                        kDaikin2BitMark, kDaikin2ZeroSpace,
                        kDaikin2BitMark, kDaikin2Gap,
                        section >= kDaikin2Sections - 1,
                        _tolerance + kDaikin2Tolerance, kDaikinMarkExcess,
                        false);
    if (used == 0) return false;
    offset += used;
    pos += ksectionSize[section];
  }
  // Compliance
  if (strict) {
    // Re-check we got the correct size/length due to the way we read the data.
    if (pos * 8 != kDaikin2Bits) return false;
    // Validate the checksum.
    if (!IRDaikin2::validChecksum(results->state)) return false;
  }

  // Success
  results->decode_type = DAIKIN2;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // DECODE_DAIKIN2
