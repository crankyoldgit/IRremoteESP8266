// Copyright 2017 Jonny Graham
// Copyright 2017-2022 David Conran
// Copyright 2021 siriuslzx
// Copyright 2023 Takeshi Shimizu

/// @file
/// @brief Support for Fujitsu A/C protocols.
/// Fujitsu A/C support added by Jonny Graham & David Conran
/// @warning Use of incorrect model may cause the A/C unit to lock up.
/// e.g. An A/C that uses an AR-RAH1U remote may lock up requiring a physical
///      power rest, if incorrect model (ARRAH2E) is used with a Swing command.
///      The correct model for it is ARREB1E.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1376

#include "ir_Fujitsu.h"
#include <algorithm>
#ifndef ARDUINO
#include <string>
#endif
#include "IRsend.h"
#include "IRtext.h"
#include "IRutils.h"

// Ref:
// These values are based on averages of measurements
const uint16_t kFujitsuAcHdrMark = 3324;
const uint16_t kFujitsuAcHdrSpace = 1574;
const uint16_t kFujitsuAcBitMark = 448;
const uint16_t kFujitsuAcOneSpace = 1182;
const uint16_t kFujitsuAcZeroSpace = 390;
const uint16_t kFujitsuAcMinGap = 8100;
const uint8_t  kFujitsuAcExtraTolerance = 5;  // Extra tolerance percentage.

using irutils::addBoolToString;
using irutils::addIntToString;
using irutils::addLabeledString;
using irutils::addModeToString;
using irutils::addModelToString;
using irutils::addFanToString;
using irutils::addTempFloatToString;
using irutils::minsToString;
using irutils::addSignedIntToString;

#if SEND_FUJITSU_AC
/// Send a Fujitsu A/C formatted message.
/// Status: STABLE / Known Good.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
///   Typically one of:
///          kFujitsuAcStateLength,
///          kFujitsuAcStateLength - 1,
///          kFujitsuAcStateLengthShort,
///          kFujitsuAcStateLengthShort - 1
/// @param[in] repeat The number of times the command is to be repeated.
void IRsend::sendFujitsuAC(const unsigned char data[], const uint16_t nbytes,
                           const uint16_t repeat) {
  sendGeneric(kFujitsuAcHdrMark, kFujitsuAcHdrSpace, kFujitsuAcBitMark,
              kFujitsuAcOneSpace, kFujitsuAcBitMark, kFujitsuAcZeroSpace,
              kFujitsuAcBitMark, kFujitsuAcMinGap, data, nbytes, 38, false,
              repeat, 50);
}
#endif  // SEND_FUJITSU_AC

// Code to emulate Fujitsu A/C IR remote control unit.

/// Class Constructor
/// @param[in] pin GPIO to be used when sending.
/// @param[in] model The enum for the model of A/C to be emulated.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRFujitsuAC::IRFujitsuAC(const uint16_t pin,
                         const fujitsu_ac_remote_model_t model,
                         const bool inverted, const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) {
  setModel(model);
  stateReset();
}

/// Set the currently emulated model of the A/C.
/// @param[in] model An enum representing the model to support/emulate.
void IRFujitsuAC::setModel(const fujitsu_ac_remote_model_t model) {
  _model = model;
  switch (model) {
    case fujitsu_ac_remote_model_t::ARDB1:
    case fujitsu_ac_remote_model_t::ARJW2:
      _state_length = kFujitsuAcStateLength - 1;
      _state_length_short = kFujitsuAcStateLengthShort - 1;
      break;
    case fujitsu_ac_remote_model_t::ARRY4:
    case fujitsu_ac_remote_model_t::ARRAH2E:
    case fujitsu_ac_remote_model_t::ARREB1E:
    default:
      _state_length = kFujitsuAcStateLength;
      _state_length_short = kFujitsuAcStateLengthShort;
  }
}

/// Get the currently emulated/detected model of the A/C.
/// @return The enum representing the model of A/C.
fujitsu_ac_remote_model_t IRFujitsuAC::getModel(void) const { return _model; }

/// Reset the state of the remote to a known good state/sequence.
void IRFujitsuAC::stateReset(void) {
  for (size_t i = 0; i < kFujitsuAcStateLength; i++) {
    _.longcode[i] = 0;
  }
  setTemp(24);
  _.Fan = kFujitsuAcFanHigh;
  _.Mode = kFujitsuAcModeCool;
  _.Swing = kFujitsuAcSwingBoth;
  _cmd = kFujitsuAcCmdTurnOn;
  _.Filter = false;
  _.Clean = false;
  _.TimerType = kFujitsuAcStopTimers;
  _.OnTimer = 0;
  _.OffTimer = 0;
  _.longcode[0] = 0x14;
  _.longcode[1] = 0x63;
  _.longcode[3] = 0x10;
  _.longcode[4] = 0x10;
  _rawstatemodified = true;
}

/// Set up hardware to be able to send a message.
void IRFujitsuAC::begin(void) { _irsend.begin(); }

#if SEND_FUJITSU_AC
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRFujitsuAC::send(const uint16_t repeat) {
  _irsend.sendFujitsuAC(getRaw(), getStateLength(), repeat);
}
#endif  // SEND_FUJITSU_AC

/// Update the length (size) of the state code for the current configuration.
/// @return true, if use long codes; false, use short codes.
bool IRFujitsuAC::updateUseLongOrShort(void) {
  bool fullCmd = false;
  switch (_cmd) {
    case kFujitsuAcCmdTurnOff:     // 0x02
    case kFujitsuAcCmdEcono:       // 0x09
    case kFujitsuAcCmdPowerful:    // 0x39
    case kFujitsuAcCmdStepVert:    // 0x6C
    case kFujitsuAcCmdToggleSwingVert:   // 0x6D
    case kFujitsuAcCmdStepHoriz:   // 0x79
    case kFujitsuAcCmdToggleSwingHoriz:  // 0x7A
      _.Cmd = _cmd;
      _rawstatemodified = true;
      break;
    default:
      switch (_model) {
        case fujitsu_ac_remote_model_t::ARRY4:
        case fujitsu_ac_remote_model_t::ARRAH2E:
        case fujitsu_ac_remote_model_t::ARREB1E:
        case fujitsu_ac_remote_model_t::ARREW4E:
          _.Cmd = 0xFE;
          _rawstatemodified = true;
          break;
        case fujitsu_ac_remote_model_t::ARDB1:
        case fujitsu_ac_remote_model_t::ARJW2:
          _.Cmd = 0xFC;
          _rawstatemodified = true;
          break;
      }
      fullCmd = true;
      break;
  }
  return fullCmd;
}

/// Calculate and set the checksum values for the internal state.
void IRFujitsuAC::checkSum(void) {
  _rawstatemodified = true;
  if (updateUseLongOrShort()) {  // Is it going to be a long code?
    // Nr. of bytes in the message after this byte.
    _.RestLength = _state_length - 7;
    _.Protocol = (_model == fujitsu_ac_remote_model_t::ARREW4E) ? 0x31 : 0x30;
    _.Power = (_cmd == kFujitsuAcCmdTurnOn) || get10CHeat();

    // These values depend on model
    if (_model != fujitsu_ac_remote_model_t::ARREB1E &&
        _model != fujitsu_ac_remote_model_t::ARREW4E) {
      _.OutsideQuiet = 0;
      if (_model != fujitsu_ac_remote_model_t::ARRAH2E) {
        _.TimerType = kFujitsuAcStopTimers;
      }
    }
    if (_model != fujitsu_ac_remote_model_t::ARRY4) {
      switch (_model) {
        case fujitsu_ac_remote_model_t::ARRAH2E:
        case fujitsu_ac_remote_model_t::ARREW4E:
          break;
        default:
          _.Clean = false;
      }
      _.Filter = false;
    }
    // Set the On/Off/Sleep timer Nr of mins.
    _.OffTimer = getOffSleepTimer();
    _.OnTimer = getOnTimer();
    // Enable bit for the Off/Sleep timer
    _.OffTimerEnable = _.OffTimer > 0;
    // Enable bit for the On timer
    _.OnTimerEnable = _.OnTimer > 0;

    uint8_t checksum = 0;
    uint8_t checksum_complement = 0;
    switch (_model) {
      case fujitsu_ac_remote_model_t::ARDB1:
      case fujitsu_ac_remote_model_t::ARJW2:
        _.Swing = kFujitsuAcSwingOff;
        checksum = sumBytes(_.longcode, _state_length - 1);
        checksum_complement = 0x9B;
        break;
      case fujitsu_ac_remote_model_t::ARREB1E:
      case fujitsu_ac_remote_model_t::ARRAH2E:
      case fujitsu_ac_remote_model_t::ARRY4:
        _.unknown = 1;
        // FALL THRU
      default:
        checksum = sumBytes(_.longcode + _state_length_short,
                            _state_length - _state_length_short - 1);
    }
    // and negate the checksum and store it in the last byte.
    _.longcode[_state_length - 1] = checksum_complement - checksum;
  } else {  // short codes
    for (size_t i = 0; i < _state_length_short; i++) {
      _.shortcode[i] = _.longcode[i];
    }
    switch (_model) {
      case fujitsu_ac_remote_model_t::ARRY4:
      case fujitsu_ac_remote_model_t::ARRAH2E:
      case fujitsu_ac_remote_model_t::ARREB1E:
      case fujitsu_ac_remote_model_t::ARREW4E:
        // The last byte is the inverse of penultimate byte
        _.shortcode[_state_length_short - 1] =
            ~_.shortcode[_state_length_short - 2];
        break;
      default:
        {};  // We don't need to do anything for the others.
    }
  }
}

/// Get the length (size) of the state code for the current configuration.
/// @return The length of the state array required for this config.
uint8_t IRFujitsuAC::getStateLength(void) {
  return updateUseLongOrShort() ? _state_length : _state_length_short;
}

/// Is the current binary state representation a long or a short code?
/// @return true, if long; false, if short.
bool IRFujitsuAC::isLongCode(void) const {
  return _.Cmd == 0xFE || _.Cmd == 0xFC;
}

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t* IRFujitsuAC::getRaw(void) {
  checkSum();
  return isLongCode() ? _.longcode : _.shortcode;
}

/// Build the internal state/config from the current (raw) A/C message.
/// @param[in] length Size of the current/used (raw) A/C message array.
void IRFujitsuAC::buildFromState(const uint16_t length) {
  switch (length) {
    case kFujitsuAcStateLength - 1:
    case kFujitsuAcStateLengthShort - 1:
      setModel(fujitsu_ac_remote_model_t::ARDB1);
      // ARJW2 has horizontal swing.
      if (_.Swing > kFujitsuAcSwingVert)
        setModel(fujitsu_ac_remote_model_t::ARJW2);
      break;
    default:
      switch (_.Cmd) {
        case kFujitsuAcCmdEcono:
        case kFujitsuAcCmdPowerful:
          setModel(fujitsu_ac_remote_model_t::ARREB1E);
          break;
        default:
          setModel(fujitsu_ac_remote_model_t::ARRAH2E);
      }
  }
  switch (_.RestLength) {
    case 8:
      if (_model != fujitsu_ac_remote_model_t::ARJW2)
        setModel(fujitsu_ac_remote_model_t::ARDB1);
      break;
    case 9:
      if (_model != fujitsu_ac_remote_model_t::ARREB1E)
        setModel(fujitsu_ac_remote_model_t::ARRAH2E);
      break;
  }
  if (_.Power)
    setCmd(kFujitsuAcCmdTurnOn);
  else
    setCmd(kFujitsuAcCmdStayOn);
  // Currently the only way we know how to tell ARRAH2E & ARRY4 apart is if
  // either the raw Filter or Clean setting is on.
  if (_model == fujitsu_ac_remote_model_t::ARRAH2E && (_.Filter || _.Clean) &&
      !get10CHeat())
    setModel(fujitsu_ac_remote_model_t::ARRY4);
  if (_state_length == kFujitsuAcStateLength && _.OutsideQuiet)
    setModel(fujitsu_ac_remote_model_t::ARREB1E);
  switch (_.Cmd) {
    case kFujitsuAcCmdTurnOff:
    case kFujitsuAcCmdStepHoriz:
    case kFujitsuAcCmdToggleSwingHoriz:
    case kFujitsuAcCmdStepVert:
    case kFujitsuAcCmdToggleSwingVert:
    case kFujitsuAcCmdEcono:
    case kFujitsuAcCmdPowerful:
      setCmd(_.Cmd);
      break;
  }
  if (_.Protocol == 0x31) setModel(fujitsu_ac_remote_model_t::ARREW4E);
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] newState A valid code for this protocol.
/// @param[in] length Size of the newState array.
/// @return true, if successful; Otherwise false. (i.e. size check)
bool IRFujitsuAC::setRaw(const uint8_t newState[], const uint16_t length) {
  if (length > kFujitsuAcStateLength) return false;
  for (uint16_t i = 0; i < kFujitsuAcStateLength; i++) {
    if (i < length)
      _.longcode[i] = newState[i];
    else
      _.longcode[i] = 0;
  }
  buildFromState(length);
  _rawstatemodified = false;
  return true;
}

/// Request the A/C to step the Horizontal Swing.
void IRFujitsuAC::stepHoriz(void) { setCmd(kFujitsuAcCmdStepHoriz); }

/// Request the A/C to toggle the Horizontal Swing mode.
/// @param[in] update Do we need to update the general swing config?
void IRFujitsuAC::toggleSwingHoriz(const bool update) {
  // Toggle the current setting.
  if (update) setSwing(getSwing() ^ kFujitsuAcSwingHoriz);
  // and set the appropriate special command.
  setCmd(kFujitsuAcCmdToggleSwingHoriz);
}

/// Request the A/C to step the Vertical Swing.
void IRFujitsuAC::stepVert(void) { setCmd(kFujitsuAcCmdStepVert); }

/// Request the A/C to toggle the Vertical Swing mode.
/// @param[in] update Do we need to update the general swing config?
void IRFujitsuAC::toggleSwingVert(const bool update) {
  // Toggle the current setting.
  if (update) setSwing(getSwing() ^ kFujitsuAcSwingVert);
  // and set the appropriate special command.
  setCmd(kFujitsuAcCmdToggleSwingVert);
}

/// Set the requested (special) command part for the A/C message.
/// @param[in] cmd The special command code.
void IRFujitsuAC::setCmd(const uint8_t cmd) {
  switch (cmd) {
    case kFujitsuAcCmdTurnOff:
    case kFujitsuAcCmdTurnOn:
    case kFujitsuAcCmdStayOn:
    case kFujitsuAcCmdStepVert:
    case kFujitsuAcCmdToggleSwingVert:
      _cmd = cmd;
      break;
    case kFujitsuAcCmdStepHoriz:
    case kFujitsuAcCmdToggleSwingHoriz:
      switch (_model) {
        // Only these remotes have horizontal.
        case fujitsu_ac_remote_model_t::ARRAH2E:
        case fujitsu_ac_remote_model_t::ARJW2:
          _cmd = cmd;
          break;
        default:
          _cmd = kFujitsuAcCmdStayOn;
      }
      break;
    case kFujitsuAcCmdEcono:
    case kFujitsuAcCmdPowerful:
      switch (_model) {
        // Only these remotes have these commands.
        case ARREB1E:
        case ARREW4E:
          _cmd = cmd;
        break;
      default:
        _cmd = kFujitsuAcCmdStayOn;
      }
      break;
    default:
      _cmd = kFujitsuAcCmdStayOn;
  }
}

/// Set the requested (special) command part for the A/C message.
/// @return The special command code.
uint8_t IRFujitsuAC::getCmd(void) const {
  return _cmd;
}

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRFujitsuAC::setPower(const bool on) {
  setCmd(on ? kFujitsuAcCmdTurnOn : kFujitsuAcCmdTurnOff);
}

/// Set the requested power state of the A/C to off.
void IRFujitsuAC::off(void) { setPower(false); }

/// Set the requested power state of the A/C to on.
void IRFujitsuAC::on(void) { setPower(true); }

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool IRFujitsuAC::getPower(void) const { return _cmd != kFujitsuAcCmdTurnOff; }

/// Set the Outside Quiet mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRFujitsuAC::setOutsideQuiet(const bool on) {
  _.OutsideQuiet = on;
  _rawstatemodified = true;
  setCmd(kFujitsuAcCmdStayOn);  // No special command involved.
}

/// Get the Outside Quiet mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRFujitsuAC::getOutsideQuiet(void) const {
  switch (_model) {
    // Only ARREB1E & ARREW4E seems to have this mode.
    case fujitsu_ac_remote_model_t::ARREB1E:
    case fujitsu_ac_remote_model_t::ARREW4E:
      return _.OutsideQuiet;
    default: return false;
  }
}

/// Set the temperature.
/// @param[in] temp The temperature in degrees.
/// @param[in] useCelsius Use Celsius or Fahrenheit?
void IRFujitsuAC::setTemp(const float temp, const bool useCelsius) {
  float mintemp;
  float maxtemp;
  uint8_t offset;
  bool _useCelsius;
  float _temp;

  switch (_model) {
    // These models have native Fahrenheit & Celsius upport.
    case fujitsu_ac_remote_model_t::ARREW4E:
      _useCelsius = useCelsius;
      _temp = temp;
      break;
    // Make sure everything else uses Celsius.
    default:
      _useCelsius = true;
      _temp = useCelsius ? temp : fahrenheitToCelsius(temp);
  }
  setCelsius(_useCelsius);
  if (_useCelsius) {
    mintemp = kFujitsuAcMinTemp;
    maxtemp = kFujitsuAcMaxTemp;
    offset = kFujitsuAcTempOffsetC;
  } else {
    mintemp = kFujitsuAcMinTempF;
    maxtemp = kFujitsuAcMaxTempF;
    offset = kFujitsuAcTempOffsetF;
  }
  _temp = std::max(mintemp, _temp);
  _temp = std::min(maxtemp, _temp);
  if (_useCelsius) {
    if (_model == fujitsu_ac_remote_model_t::ARREW4E)
      _.Temp = (_temp - (offset / 2)) * 2;
    else
      _.Temp = (_temp - offset) * 4;
  } else {
    _.Temp = _temp - offset;
  }
  _rawstatemodified = true;
  setCmd(kFujitsuAcCmdStayOn);  // No special command involved.
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees of the currently set units.
float IRFujitsuAC::getTemp(void) const {
  if (_model == fujitsu_ac_remote_model_t::ARREW4E) {
    if (_.Fahrenheit)  // Currently only ARREW4E supports native Fahrenheit.
      return _.Temp + kFujitsuAcTempOffsetF;
    else
      return (_.Temp / 2.0) + (kFujitsuAcMinTemp / 2);
  } else {
    return _.Temp / 4 + kFujitsuAcMinTemp;
  }
}

/// Set the speed of the fan.
/// @param[in] fanSpeed The desired setting.
void IRFujitsuAC::setFanSpeed(const uint8_t fanSpeed) {
  if (fanSpeed > kFujitsuAcFanQuiet)
    _.Fan = kFujitsuAcFanHigh;  // Set the fan to maximum if out of range.
  else
    _.Fan = fanSpeed;
  _rawstatemodified = true;
  setCmd(kFujitsuAcCmdStayOn);  // No special command involved.
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t IRFujitsuAC::getFanSpeed(void) const { return _.Fan; }

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void IRFujitsuAC::setMode(const uint8_t mode) {
  if (mode > kFujitsuAcModeHeat)
    _.Mode = kFujitsuAcModeHeat;  // Set the mode to maximum if out of range.
  else
    _.Mode = mode;
  _rawstatemodified = true;
  setCmd(kFujitsuAcCmdStayOn);  // No special command involved.
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t IRFujitsuAC::getMode(void) const { return _.Mode; }

/// Set the requested swing operation mode of the A/C unit.
/// @param[in] swingMode The swingMode code for the A/C.
///   Vertical, Horizon, or Both. See constants for details.
/// @note Not all models support all possible swing modes.
void IRFujitsuAC::setSwing(const uint8_t swingMode) {
  _.Swing = swingMode;
  _rawstatemodified = true;
  switch (_model) {
    // No Horizontal support.
    case fujitsu_ac_remote_model_t::ARDB1:
    case fujitsu_ac_remote_model_t::ARREB1E:
    case fujitsu_ac_remote_model_t::ARRY4:
      // Set the mode to max if out of range
      if (swingMode > kFujitsuAcSwingVert) _.Swing = kFujitsuAcSwingVert;
      break;
    // Has Horizontal support.
    case fujitsu_ac_remote_model_t::ARRAH2E:
    case fujitsu_ac_remote_model_t::ARJW2:
    default:
      // Set the mode to max if out of range
      if (swingMode > kFujitsuAcSwingBoth) _.Swing = kFujitsuAcSwingBoth;
  }
  setCmd(kFujitsuAcCmdStayOn);  // No special command involved.
}

/// Get the requested swing operation mode of the A/C unit.
/// @return The contents of the swing state/mode.
uint8_t IRFujitsuAC::getSwing(void) const { return _.Swing; }

/// Set the Clean mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRFujitsuAC::setClean(const bool on) {
  _.Clean = on;
  _rawstatemodified = true;
  setCmd(kFujitsuAcCmdStayOn);  // No special command involved.
}

/// Get the Clean mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRFujitsuAC::getClean(void) const {
  switch (_model) {
    case fujitsu_ac_remote_model_t::ARRY4: return _.Clean;
    default: return false;
  }
}

/// Set the Filter mode status of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRFujitsuAC::setFilter(const bool on) {
  _.Filter = on;
  _rawstatemodified = true;
  setCmd(kFujitsuAcCmdStayOn);  // No special command involved.
}

/// Get the Filter mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRFujitsuAC::getFilter(void) const {
  switch (_model) {
    case fujitsu_ac_remote_model_t::ARRY4: return _.Filter;
    default:                               return false;
  }
}

/// Set the 10C heat status of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRFujitsuAC::set10CHeat(const bool on) {
  switch (_model) {
    // Only selected models support this.
    case fujitsu_ac_remote_model_t::ARRAH2E:
    case fujitsu_ac_remote_model_t::ARREW4E:
      setClean(on);  // 10C Heat uses the same bit as Clean
      if (on) {
        _.Mode = kFujitsuAcModeFan;
        _.Power = true;
        _.Fan = kFujitsuAcFanAuto;
        _.Swing = kFujitsuAcSwingOff;
        _rawstatemodified = true;
      }
    default:
      break;
  }
}

/// Get the 10C heat status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRFujitsuAC::get10CHeat(void) const {
  switch (_model) {
    case fujitsu_ac_remote_model_t::ARRAH2E:
    case fujitsu_ac_remote_model_t::ARREW4E:
      return (_.Clean && _.Power && _.Mode == kFujitsuAcModeFan &&
              _.Fan == kFujitsuAcFanAuto && _.Swing == kFujitsuAcSwingOff);
    default: return false;
  }
}

/// Get the Timer type of the A/C message.
/// @return The current timer type in numeric form.
uint8_t IRFujitsuAC::getTimerType(void) const {
  switch (_model) {
    // These models seem to have timer support.
    case fujitsu_ac_remote_model_t::ARRAH2E:
    case fujitsu_ac_remote_model_t::ARREB1E: return _.TimerType;
    default:                                 return kFujitsuAcStopTimers;
  }
}

/// Set the Timer type of the A/C message.
/// @param[in] timertype The kind of timer to use for the message.
void IRFujitsuAC::setTimerType(const uint8_t timertype) {
  switch (timertype) {
    case kFujitsuAcSleepTimer:
    case kFujitsuAcOnTimer:
    case kFujitsuAcOffTimer:
    case kFujitsuAcStopTimers:
      _.TimerType = timertype;
      break;
    default: _.TimerType = kFujitsuAcStopTimers;
  }
  _rawstatemodified = true;
}

/// Get the On Timer setting of the A/C.
/// @return nr of minutes left on the timer. 0 means disabled/not supported.
uint16_t IRFujitsuAC::getOnTimer(void) const {
  if (getTimerType() == kFujitsuAcOnTimer)
    return _.OnTimer;
  return 0;
}

/// Set the On Timer setting of the A/C.
/// @param[in] nr_mins Nr. of minutes to set the timer to. 0 means disabled.
void IRFujitsuAC::setOnTimer(const uint16_t nr_mins) {
  _.OnTimer = std::min(kFujitsuAcTimerMax, nr_mins);  // Bounds check.
  _rawstatemodified = true;
  if (_.OnTimer) {
    _.TimerType = kFujitsuAcOnTimer;
  } else if (getTimerType() == kFujitsuAcOnTimer) {
    _.TimerType = kFujitsuAcStopTimers;
  }
}

/// Get the Off/Sleep Timer setting of the A/C.
/// @return nr of minutes left on the timer. 0 means disabled/not supported.
uint16_t IRFujitsuAC::getOffSleepTimer(void) const {
  switch (getTimerType()) {
    case kFujitsuAcOffTimer:
    case kFujitsuAcSleepTimer: return _.OffTimer;
    default:                   return 0;
  }
}

/// Set the Off/Sleep Timer time for the A/C.
/// @param[in] nr_mins Nr. of minutes to set the timer to. 0 means disabled.
inline void IRFujitsuAC::setOffSleepTimer(const uint16_t nr_mins) {
  _.OffTimer = std::min(kFujitsuAcTimerMax, nr_mins);  // Bounds check.
  _rawstatemodified = true;
}

/// Set the Off Timer time for the A/C.
/// @param[in] nr_mins Nr. of minutes to set the timer to. 0 means disabled.
void IRFujitsuAC::setOffTimer(const uint16_t nr_mins) {
  setOffSleepTimer(nr_mins);  // This will also set _rawstatemodified to true.
  if (nr_mins)
    _.TimerType = kFujitsuAcOffTimer;
  else if (getTimerType() != kFujitsuAcOnTimer)
    _.TimerType = kFujitsuAcStopTimers;
}

/// Set the Sleep Timer time for the A/C.
/// @param[in] nr_mins Nr. of minutes to set the timer to. 0 means disabled.
void IRFujitsuAC::setSleepTimer(const uint16_t nr_mins) {
  setOffSleepTimer(nr_mins);  // This will also set _rawstatemodified to true.
  if (nr_mins)
    _.TimerType = kFujitsuAcSleepTimer;
  else if (getTimerType() != kFujitsuAcOnTimer)
    _.TimerType = kFujitsuAcStopTimers;
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length of the state array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool IRFujitsuAC::validChecksum(uint8_t state[], const uint16_t length) {
  uint8_t sum = 0;
  uint8_t sum_complement = 0;
  uint8_t checksum = state[length - 1];
  switch (length) {
    case kFujitsuAcStateLengthShort:  // ARRAH2E, ARREB1E, & ARRY4
      return state[length - 1] == (uint8_t)~state[length - 2];
    case kFujitsuAcStateLength - 1:  // ARDB1 & ARJW2
      sum = sumBytes(state, length - 1);
      sum_complement = 0x9B;
      break;
    case kFujitsuAcStateLength:  // ARRAH2E, ARRY4, & ARREB1E
      sum = sumBytes(state + kFujitsuAcStateLengthShort,
                     length - 1 - kFujitsuAcStateLengthShort);
      break;
    default:        // Includes ARDB1 & ARJW2 short.
      return true;  // Assume the checksum is valid for other lengths.
  }
  return checksum == (uint8_t)(sum_complement - sum);  // Does it match?
}

/// Set the device's remote ID number.
/// @param[in] num The ID for the remote. Valid number range is 0 to 3.
void IRFujitsuAC::setId(const uint8_t num) {
  _.Id = num;
  _rawstatemodified = true;
}

/// Get the current device's remote ID number.
/// @return The current device's remote ID number.
uint8_t IRFujitsuAC::getId(void) const { return _.Id; }

/// Set the Temperature units for the A/C.
/// @param[in] on true, use Celsius. false, use Fahrenheit.
void IRFujitsuAC::setCelsius(const bool on) {
  _.Fahrenheit = !on;
  _rawstatemodified = true;
}

/// Get the Clean mode status of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRFujitsuAC::getCelsius(void) const { return !_.Fahrenheit; }

/// Convert a stdAc::opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRFujitsuAC::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kCool: return kFujitsuAcModeCool;
    case stdAc::opmode_t::kHeat: return kFujitsuAcModeHeat;
    case stdAc::opmode_t::kDry:  return kFujitsuAcModeDry;
    case stdAc::opmode_t::kFan:  return kFujitsuAcModeFan;
    default:                     return kFujitsuAcModeAuto;
  }
}

/// Convert a stdAc::fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRFujitsuAC::convertFan(stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin:    return kFujitsuAcFanQuiet;
    case stdAc::fanspeed_t::kLow:    return kFujitsuAcFanLow;
    case stdAc::fanspeed_t::kMedium: return kFujitsuAcFanMed;
    case stdAc::fanspeed_t::kHigh:
    case stdAc::fanspeed_t::kMax:    return kFujitsuAcFanHigh;
    default:                         return kFujitsuAcFanAuto;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::opmode_t IRFujitsuAC::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kFujitsuAcModeCool: return stdAc::opmode_t::kCool;
    case kFujitsuAcModeHeat: return stdAc::opmode_t::kHeat;
    case kFujitsuAcModeDry:  return stdAc::opmode_t::kDry;
    case kFujitsuAcModeFan:  return stdAc::opmode_t::kFan;
    default:                 return stdAc::opmode_t::kAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::fanspeed_t IRFujitsuAC::toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kFujitsuAcFanHigh:  return stdAc::fanspeed_t::kMax;
    case kFujitsuAcFanMed:   return stdAc::fanspeed_t::kMedium;
    case kFujitsuAcFanLow:   return stdAc::fanspeed_t::kLow;
    case kFujitsuAcFanQuiet: return stdAc::fanspeed_t::kMin;
    default:                 return stdAc::fanspeed_t::kAuto;
  }
}

/// Convert the current internal state into its stdAc::state_t equivalent.
/// @param[in] prev Ptr to a previous state.
/// @return The stdAc equivalent of the native settings.
stdAc::state_t IRFujitsuAC::toCommon(const stdAc::state_t *prev) {
  stdAc::state_t result{};
  if (prev != NULL) result = *prev;
  result.protocol = decode_type_t::FUJITSU_AC;
  checkSum();
  result.model = _model;
  result.power = getPower();
  // Only update these settings if it is a long message, or we have no previous
  // state info for those settings.
  if (isLongCode() || prev == NULL) {
    result.mode = toCommonMode(_.Mode);
    result.celsius = getCelsius();
    {
      const float minHeat = result.celsius ? kFujitsuAcMinHeat
                                           : kFujitsuAcMinHeatF;
      result.degrees = get10CHeat() ? minHeat : getTemp();
    }
    result.fanspeed = toCommonFanSpeed(_.Fan);
    uint8_t swing = _.Swing;
    switch (result.model) {
      case fujitsu_ac_remote_model_t::ARREB1E:
      case fujitsu_ac_remote_model_t::ARRAH2E:
      case fujitsu_ac_remote_model_t::ARRY4:
        result.clean = _.Clean;
        result.filter = _.Filter;
        result.swingv = (swing & kFujitsuAcSwingVert) ? stdAc::swingv_t::kAuto
                                                      : stdAc::swingv_t::kOff;
        result.swingh = (swing & kFujitsuAcSwingHoriz) ? stdAc::swingh_t::kAuto
                                                       : stdAc::swingh_t::kOff;
        break;
      case fujitsu_ac_remote_model_t::ARDB1:
      case fujitsu_ac_remote_model_t::ARJW2:
      default:
        result.swingv = stdAc::swingv_t::kOff;
        result.swingh = stdAc::swingh_t::kOff;
    }
  }
  result.quiet = _.Fan == kFujitsuAcFanQuiet;
  result.turbo = _cmd == kFujitsuAcCmdPowerful;
  result.econo = _cmd == kFujitsuAcCmdEcono;
  // Not supported.
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
String IRFujitsuAC::toString(void) const {
  String result = "";
  result.reserve(180);  // Reserve some heap for the string to reduce fragging.
  fujitsu_ac_remote_model_t model = _model;
  result += addModelToString(decode_type_t::FUJITSU_AC, model, false);
  result += addIntToString(_.Id, kIdStr);
  result += addBoolToString(getPower(), kPowerStr);
  if (_rawstatemodified || isLongCode()) {
    result += addModeToString(_.Mode, kFujitsuAcModeAuto, kFujitsuAcModeCool,
                              kFujitsuAcModeHeat, kFujitsuAcModeDry,
                              kFujitsuAcModeFan);
    {
      const bool isCelsius = getCelsius();
      const float minHeat = isCelsius ? kFujitsuAcMinHeat : kFujitsuAcMinHeatF;
      result += addTempFloatToString(get10CHeat() ? minHeat : getTemp(),
                                     isCelsius);
    }
    result += addFanToString(_.Fan, kFujitsuAcFanHigh, kFujitsuAcFanLow,
                             kFujitsuAcFanAuto, kFujitsuAcFanQuiet,
                             kFujitsuAcFanMed);
    switch (model) {
      // These models have no internal swing, clean. or filter state.
      case fujitsu_ac_remote_model_t::ARDB1:
      case fujitsu_ac_remote_model_t::ARJW2:
        break;
      // These models have Clean & Filter, plus Swing (via fall thru)
      case fujitsu_ac_remote_model_t::ARRAH2E:
      case fujitsu_ac_remote_model_t::ARREB1E:
      case fujitsu_ac_remote_model_t::ARRY4:
        result += addBoolToString(getClean(), kCleanStr);
        result += addBoolToString(getFilter(), kFilterStr);
        // FALL THRU
      default:   // e.g. ARREW4E
        switch (model) {
          case fujitsu_ac_remote_model_t::ARRAH2E:
          case fujitsu_ac_remote_model_t::ARREW4E:
            result += addBoolToString(get10CHeat(), k10CHeatStr);
            break;
          default:
            break;
        }
        result += addIntToString(_.Swing, kSwingStr);
        result += kSpaceLBraceStr;
        switch (_.Swing) {
          case kFujitsuAcSwingOff:
            result += kOffStr;
            break;
          case kFujitsuAcSwingVert:
            result += kSwingVStr;
            break;
          case kFujitsuAcSwingHoriz:
            result += kSwingHStr;
            break;
          case kFujitsuAcSwingBoth:
            result += kSwingVStr;
            result += '+';
            result += kSwingHStr;
            break;
          default:
            result += kUnknownStr;
        }
        result += ')';
    }
  }
  result += kCommaSpaceStr;
  result += kCommandStr;
  result += kColonSpaceStr;
  switch (_cmd) {
    case kFujitsuAcCmdStepHoriz:
      result += kStepStr;
      result += ' ';
      result += kSwingHStr;
      break;
    case kFujitsuAcCmdStepVert:
      result += kStepStr;
      result += ' ';
      result += kSwingVStr;
      break;
    case kFujitsuAcCmdToggleSwingHoriz:
      result += kToggleStr;
      result += ' ';
      result += kSwingHStr;
      break;
    case kFujitsuAcCmdToggleSwingVert:
      result += kToggleStr;
      result += ' ';
      result += kSwingVStr;
      break;
    case kFujitsuAcCmdEcono:
      result += kEconoStr;
      break;
    case kFujitsuAcCmdPowerful:
      result += kPowerfulStr;
      break;
    default:
      result += kNAStr;
  }
  if (_rawstatemodified || isLongCode()) {
    uint16_t mins = 0;
    String type_str = kTimerStr;
    switch (model) {
      case fujitsu_ac_remote_model_t::ARREB1E:
      case fujitsu_ac_remote_model_t::ARREW4E:
        result += addBoolToString(getOutsideQuiet(), kOutsideQuietStr);
        // FALL THRU
      // These models seem to have timer support.
      case fujitsu_ac_remote_model_t::ARRAH2E:
        switch (getTimerType()) {
          case kFujitsuAcOnTimer:
            type_str = kOnTimerStr;
            mins = getOnTimer();
            break;
          case kFujitsuAcOffTimer:
            type_str = kOffTimerStr;
            mins = getOffSleepTimer();
            break;
          case kFujitsuAcSleepTimer:
            type_str = kSleepTimerStr;
            mins = getOffSleepTimer();
            break;
        }
        result += addLabeledString(mins ? minsToString(mins) : kOffStr,
                                   type_str);
        break;
      default:
        break;
    }
  }
  return result;
}

#if DECODE_FUJITSU_AC
/// Decode the supplied Fujitsu AC IR message if possible.
/// Status: STABLE / Working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool IRrecv::decodeFujitsuAC(decode_results* results, uint16_t offset,
                             const uint16_t nbits,
                             const bool strict) {
  uint16_t dataBitsSoFar = 0;

  // Have we got enough data to successfully decode?
  if (results->rawlen < (2 * kFujitsuAcMinBits) + kHeader + kFooter - 1 +
      offset)
    return false;  // Can't possibly be a valid message.

  // Compliance
  if (strict) {
    switch (nbits) {
      case kFujitsuAcBits:
      case kFujitsuAcBits - 8:
      case kFujitsuAcMinBits:
      case kFujitsuAcMinBits + 8: break;
      default: return false;  // Must be called with the correct nr. of bits.
    }
  }

  // Header / Some of the Data
  uint16_t used = matchGeneric(results->rawbuf + offset, results->state,
                               results->rawlen - offset, kFujitsuAcMinBits - 8,
                               kFujitsuAcHdrMark, kFujitsuAcHdrSpace,  // Header
                               kFujitsuAcBitMark, kFujitsuAcOneSpace,  // Data
                               kFujitsuAcBitMark, kFujitsuAcZeroSpace,
                               0, 0,  // No Footer (yet)
                               false, _tolerance + kFujitsuAcExtraTolerance, 0,
                               false);  // LSBF
  if (!used) return false;
  offset += used;
  // Check we have the typical data header.
  if (results->state[0] != 0x14 || results->state[1] != 0x63) return false;
  dataBitsSoFar += kFujitsuAcMinBits - 8;

  // Keep reading bytes until we either run out of message or state to fill.
  match_result_t data_result;
  for (uint16_t i = 5;
       offset <= results->rawlen - 16 && i < kFujitsuAcStateLength;
       i++, dataBitsSoFar += 8, offset += data_result.used) {
    data_result = matchData(
        &(results->rawbuf[offset]), 8, kFujitsuAcBitMark, kFujitsuAcOneSpace,
        kFujitsuAcBitMark, kFujitsuAcZeroSpace,
        _tolerance + kFujitsuAcExtraTolerance, 0, false);
    if (data_result.success == false) break;  // Fail
    results->state[i] = data_result.data;
  }

  // Footer
  if (offset > results->rawlen ||
      !matchMark(results->rawbuf[offset++], kFujitsuAcBitMark))
    return false;
  // The space is optional if we are out of capture.
  if (offset < results->rawlen &&
      !matchAtLeast(results->rawbuf[offset], kFujitsuAcMinGap))
    return false;

  // Compliance
  if (strict) {
    if (dataBitsSoFar != nbits) return false;
  }

  results->decode_type = FUJITSU_AC;
  results->bits = dataBitsSoFar;

  // Compliance
  switch (dataBitsSoFar) {
    case kFujitsuAcMinBits:
      // Check if this values indicate that this should have been a long state
      // message.
      if (results->state[5] == 0xFC) return false;
      return true;  // Success
    case kFujitsuAcMinBits + 8:
      // Check if this values indicate that this should have been a long state
      // message.
      if (results->state[5] == 0xFE) return false;
      // The last byte needs to be the inverse of the penultimate byte.
      if (results->state[5] != (uint8_t)~results->state[6]) return false;
      return true;  // Success
    case kFujitsuAcBits - 8:
      // Long messages of this size require this byte be correct.
      if (results->state[5] != 0xFC) return false;
      break;
    case kFujitsuAcBits:
      // Long messages of this size require this byte be correct.
      if (results->state[5] != 0xFE) return false;
      break;
    default:
      return false;  // Unexpected size.
  }
  if (!IRFujitsuAC::validChecksum(results->state, dataBitsSoFar / 8))
    return false;

  // Success
  return true;  // All good.
}
#endif  // DECODE_FUJITSU_AC

#if SEND_FUJITSU_AC264
/// Send a Fujitsu 264 bit A/C formatted message.
/// Status: STABLE / Known Good.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void IRsend::sendFujitsuAC264(const unsigned char data[], const uint16_t nbytes,
                              const uint16_t repeat) {
  sendGeneric(kFujitsuAcHdrMark, kFujitsuAcHdrSpace, kFujitsuAcBitMark,
              kFujitsuAcOneSpace, kFujitsuAcBitMark, kFujitsuAcZeroSpace,
              kFujitsuAcBitMark, kFujitsuAcMinGap, data, nbytes, 38, false,
              repeat, 50);
}
#endif  // SEND_FUJITSU_AC264

// Code to emulate Fujitsu 264 bit A/C IR remote control unit.

/// Class Constructor
/// @param[in] pin GPIO to be used when sending.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRFujitsuAC264::IRFujitsuAC264(const uint16_t pin,
                         const bool inverted, const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) {
  stateReset();
}

/// Set up hardware to be able to send a message.
void IRFujitsuAC264::begin(void) { _irsend.begin(); }

/// Reset the state of the remote to a known good state/sequence.
void IRFujitsuAC264::stateReset(void) {
  for (size_t i = 0; i < kFujitsuAc264StateLength; i++) {
    _.raw[i] = 0;
  }
  _ispoweredon = false;
  _isecofan = false;
  _isoutsidequiet = false;
  _settemp = 0;
  setTemp(24);
  _cmd = _.Cmd = kFujitsuAc264CmdCool;
  _.TempAuto = 0;
  _.Mode = kFujitsuAc264ModeCool;
  _.FanSpeed = kFujitsuAc264FanSpeedHigh;
  _.FanAngle = kFujitsuAc264FanAngleStay;
  _.Swing = false;
  _.Economy = false;
  _.Clean = false;
  _.ClockHours = 0;
  _.ClockMins = 0;
  _.SleepTimerEnable = false;
  _.SleepTimer = 0;
  _.TimerEnable = kFujitsuAc264OnOffTimerDisable;
  _.OnTimer = 0;
  _.OffTimer = 0;
  _.raw[0] = 0x14;
  _.raw[1] = 0x63;
  _.raw[2] = 0x00;
  _.raw[3] = 0x10;
  _.raw[4] = 0x10;
}

#if SEND_FUJITSU_AC264
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRFujitsuAC264::send(const uint16_t repeat) {
  _irsend.sendFujitsuAC(getRaw(), getStateLength(), repeat);
  _settemp = _.Temp;        // Preserve the sent setting
  _ispoweredon = (_cmd < kFujitsuAc264SpCmdTurnOff);
  if (_cmd == kFujitsuAc264SpCmdEcoFanOn)
    _isecofan = true;
  if (_cmd == kFujitsuAc264SpCmdEcoFanOff)
    _isecofan = false;
  if (_cmd == kFujitsuAc264SpCmdOutsideQuietOn)
    _isoutsidequiet = true;
  if (_cmd == kFujitsuAc264SpCmdOutsideQuietOff)
    _isoutsidequiet = false;
}
#endif  // SEND_FUJITSU_AC264

/// Get a PTR to the internal state/code for this protocol.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t* IRFujitsuAC264::getRaw(void) {
  checkSum();
  return _.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] newState A valid code for this protocol.
/// @param[in] length Size of the newState array.
/// @return True, if successful; Otherwise false. (i.e. size check)
bool IRFujitsuAC264::setRaw(const uint8_t newState[], const uint16_t length) {
  if (length > kFujitsuAc264StateLength) return false;
  for (uint16_t i = 0; i < kFujitsuAc264StateLength; i++) {
    if (i < length)
      _.raw[i] = newState[i];
    else
      _.raw[i] = 0;
  }
  switch (length) {
    case kFujitsuAc264StateLengthShort:
      if (std::memcmp(_.raw, kFujitsuAc264StatesTurnOff,
                      kFujitsuAc264StateLengthShort) == 0)
        _cmd = kFujitsuAc264SpCmdTurnOff;
      if (std::memcmp(_.raw, kFujitsuAc264StatesTogglePowerful,
                      kFujitsuAc264StateLengthShort) == 0)
        _cmd = kFujitsuAc264SpCmdTogglePowerful;
      if (std::memcmp(_.raw, kFujitsuAc264StatesEcoFanOff,
                      kFujitsuAc264StateLengthShort) == 0)
        _cmd = kFujitsuAc264SpCmdEcoFanOff;
      if (std::memcmp(_.raw, kFujitsuAc264StatesEcoFanOn,
                      kFujitsuAc264StateLengthShort) == 0)
        _cmd = kFujitsuAc264SpCmdEcoFanOn;
      break;
    case kFujitsuAc264StateLengthMiddle:
      if (std::memcmp(_.raw, kFujitsuAc264StatesOutsideQuietOff,
                      kFujitsuAc264StateLengthMiddle) == 0)
        _cmd = kFujitsuAc264SpCmdOutsideQuietOff;
      if (std::memcmp(_.raw, kFujitsuAc264StatesOutsideQuietOn,
                      kFujitsuAc264StateLengthMiddle) == 0)
        _cmd = kFujitsuAc264SpCmdOutsideQuietOn;
      if (std::memcmp(_.raw, kFujitsuAc264StatesToggleSterilization,
                      kFujitsuAc264StateLengthMiddle) == 0)
        _cmd = kFujitsuAc264SpCmdToggleSterilization;
      break;
    case kFujitsuAc264StateLength:
      setPower(true);
      _cmd = _.Cmd;
      break;
    default:
      return false;
  }
  return true;
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length of the state array.
/// @return True, if the state has a valid checksum. Otherwise, false.
bool IRFujitsuAC264::validChecksum(uint8_t state[], const uint16_t length) {
  uint8_t sum = 0;
  uint8_t sum_complement = 0;
  uint8_t checksum = 0;

  if (length == kFujitsuAc264StateLengthShort) {
    checksum = state[kFujitsuAc264StateLengthShort - 1];
    sum = state[kFujitsuAc264StateLengthShort - 2];
    sum_complement = 0xFF;
  } else if (length == kFujitsuAc264StateLengthMiddle) {
    checksum = state[kFujitsuAc264StateLengthMiddle - 1];
    sum = sumBytes(state, kFujitsuAc264StateLengthMiddle - 1);
    sum_complement = 0x9E;
  // The current command is normal
  } else if (length == kFujitsuAc264StateLength) {
    checksum = state[kFujitsuAc264StateLength - 1];
    sum = sumBytes(state, kFujitsuAc264StateLength - 1);
    sum_complement = 0xAF;
  } else {
    return false;
  }
  return checksum == (uint8_t) (sum_complement - sum);  // Does it match?
}

/// Calculate and set the checksum values for the internal state.
void IRFujitsuAC264::checkSum(void) {
  if (!isSpecialCommand()) {   // The current command is not special
    _.raw[5] = 0xFE;
    _.RestLength = 0x1A;
    _.Protocol = 0x40;
    _.raw[13] = 0x00;
    _.raw[15] |= 0x12;
    _.raw[16] |= 0x06;
    _.raw[17] = 0x00;
    _.raw[21] |= 0x40;
    _.raw[22] |= 0x10;
    _.raw[25] = 0x00;
    _.raw[26] = 0x00;
    _.raw[27] = 0x00;
    _.raw[28] |= 0xF0;
    _.raw[29] = 0xFF;
    _.raw[30] = 0xFF;

    uint8_t checksum = 0;
    uint8_t checksum_complement = 0;
    checksum = sumBytes(_.raw, kFujitsuAc264StateLength - 1);
    checksum_complement = 0xAF;
    _.raw[kFujitsuAc264StateLength - 1] = checksum_complement - checksum;
  }
}

/// Is the current command a special command?
/// @return True, if special command (kFujitsuAc264SpCmd*);
///         false, if normal command (kFujitsuAc264Cmd*).
bool IRFujitsuAC264::isSpecialCommand(void) const {
  return (_cmd & 0xF0) == 0xF0;
}

/// Get the length (size) of the state code for the current configuration.
/// @return The length of the state array required for this config.
uint8_t IRFujitsuAC264::getStateLength(void) {
  uint8_t stateLength = 0;

  switch (_cmd) {
    case kFujitsuAc264SpCmdTurnOff:
    case kFujitsuAc264SpCmdTogglePowerful:
    case kFujitsuAc264SpCmdEcoFanOff:
    case kFujitsuAc264SpCmdEcoFanOn:
      stateLength = kFujitsuAc264StateLengthShort;
      break;
    case kFujitsuAc264SpCmdOutsideQuietOff:
    case kFujitsuAc264SpCmdOutsideQuietOn:
    case kFujitsuAc264SpCmdToggleSterilization:
      stateLength = kFujitsuAc264StateLengthMiddle;
      break;
    default:
      stateLength = kFujitsuAc264StateLength;
      break;
  }
  return stateLength;
}

/// Set the requested power state of the A/C to on.
/// @note Mode should be set after this function.
void IRFujitsuAC264::on(void) { setPower(true); }

/// Set the requested power state of the A/C to off.
void IRFujitsuAC264::off(void) { setPower(false); }

/// Change the power setting.
/// @param[in] on True, the setting is on. false, the setting is off.
/// @note If on = true, a mode should be set after calling this function.
void IRFujitsuAC264::setPower(const bool on) {
  if (on) {
    _cmd = kFujitsuAc264CmdCool;
  } else {
    _cmd = kFujitsuAc264SpCmdTurnOff;
    std::memcpy(_.raw, kFujitsuAc264StatesTurnOff,
                kFujitsuAc264StateLengthShort);
  }
}

/// Get the value of the current power setting.
/// @return True, the setting is on. false, the setting is off.
bool IRFujitsuAC264::getPower(void) const { return _ispoweredon; }

/// Check if the temperature setting is changed.
/// @return True if the temperature is not changed.
bool IRFujitsuAC264::isTempStayed(void) const { return _settemp == _.Temp; }

/// Set the temperature.
/// @param[in] temp The temperature in degrees Celcius.
/// @note The fractional part which is truncated to multiple of 0.5.
void IRFujitsuAC264::setTemp(const float temp) {
  float _temp;

  if (temp > kFujitsuAc264MaxTemp)
    _temp = kFujitsuAc264MaxTemp;
  else if ((temp < kFujitsuAc264MinTemp) && (_.Mode != kFujitsuAc264ModeHeat))
    _temp = kFujitsuAc264MinTemp;
  else if (temp < kFujitsuAc264MinHeat)
    _temp = kFujitsuAc264MinHeat;
  else
    _temp = temp;

  _.Temp = (_temp - (kFujitsuAc264TempOffsetC / 2)) * 2;
  _cmd = _.Cmd = kFujitsuAc264CmdTemp;
  _.SubCmd = isTempStayed();
}

/// Get the current temperature setting.
/// @return The current setting for temperature in degrees Celcius.
float IRFujitsuAC264::getTemp(void) const {
  return static_cast<float>(_.Temp / 2.0) + (kFujitsuAc264TempOffsetC / 2);
}

/// Set the temperature in auto mode.
/// @param[in] temp The temperature in auto mode in degrees Celcius.
/// @note The fractional part which is truncated to multiple of 0.5.
void IRFujitsuAC264::setTempAuto(const float temp) {
  int8_t _tempx10;

  _tempx10 = (int8_t) (temp * 10);
  _tempx10 -= _tempx10 % 5;
  if (temp > kFujitsuAc264MaxTempAuto)
    _tempx10 = kFujitsuAc264MaxTempAuto * 10;
  else if (temp < kFujitsuAc264MinTempAuto)
    _tempx10 = kFujitsuAc264MinTempAuto * 10;

  _.TempAuto = _tempx10;
  _cmd = _.Cmd = kFujitsuAc264CmdTemp;
}

/// Get the current temperature in auto mode setting.
/// @return The current setting for temp in auto mode in degrees Celcius.
float IRFujitsuAC264::getTempAuto(void) const {
  return static_cast<float>(static_cast<int8_t>(_.TempAuto) / 10.0);
}

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
/// @param[in] weakdry True if dry mode is in weak.
void IRFujitsuAC264::setMode(const uint8_t mode, const bool weakdry) {
  switch (mode) {
    case kFujitsuAc264ModeAuto:
      _.Mode = kFujitsuAc264ModeAuto;
      _cmd = _.Cmd = kFujitsuAc264CmdAuto;
      break;
    case kFujitsuAc264ModeCool:
      _.Mode = kFujitsuAc264ModeCool;
      _cmd = _.Cmd = kFujitsuAc264CmdCool;
      break;
    case kFujitsuAc264ModeFan:
      _.Mode = kFujitsuAc264ModeFan;
      _cmd = _.Cmd = kFujitsuAc264CmdFan;
      break;
    case kFujitsuAc264ModeHeat:
      _.Mode = kFujitsuAc264ModeHeat;
      _cmd = _.Cmd = kFujitsuAc264CmdHeat;
      break;
    case kFujitsuAc264ModeDry:
      _.Mode = kFujitsuAc264ModeDry;
      _.WeakDry = weakdry;
      _cmd = _.Cmd = kFujitsuAc264CmdDry;
      break;
    default:
      _.Mode = kFujitsuAc264ModeAuto;
      _cmd = _.Cmd = kFujitsuAc264CmdAuto;
      break;
  }
  _.SubCmd = 1;
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t IRFujitsuAC264::getMode(void) const { return _.Mode; }

/// Get the weak dry mode setting of the A/C.
/// @return The weak dry mode setting.
bool IRFujitsuAC264::isWeakDry(void) const { return _.WeakDry; }

/// Set the speed of the fan.
/// @param[in] fanSpeed The desired setting.
void IRFujitsuAC264::setFanSpeed(const uint8_t fanSpeed) {
  // Set the fan to auto if out of range.
  if ((fanSpeed == kFujitsuAc264FanSpeedQuiet) ||
      (fanSpeed == kFujitsuAc264FanSpeedLow) ||
      (fanSpeed == kFujitsuAc264FanSpeedMed) ||
      (fanSpeed == kFujitsuAc264FanSpeedHigh))
    _.FanSpeed = fanSpeed;
  else
    _.FanSpeed = kFujitsuAc264FanSpeedAuto;
  _cmd = _.Cmd = kFujitsuAc264CmdFanSpeed;
  _.SubCmd = 0;
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t IRFujitsuAC264::getFanSpeed(void) const { return _.FanSpeed; }

/// Set the angle of the fan.
/// @param[in] fanAngle The desired setting.
void IRFujitsuAC264::setFanAngle(const uint8_t fanAngle) {
  // Set the fan to stay if out of range.
  if ((fanAngle > kFujitsuAc264FanAngle7) ||
      (fanAngle < kFujitsuAc264FanAngle1)) {
    _.FanAngle = kFujitsuAc264FanAngleStay;
  } else {
    _.FanAngle = fanAngle;
  }
  _cmd = _.Cmd = kFujitsuAc264CmdFanAngle;
  _.SubCmd = 0;
}

/// Get the current fan angle setting.
/// @return The current fan angle.
uint8_t IRFujitsuAC264::getFanAngle(void) const { return _.FanAngle; }

/// Set weather the swing of fan is enabled or not.
/// @param[in] on True if swing is enabled, false if disabled.
void IRFujitsuAC264::setSwing(const bool on) {
  _.Swing = on;
  _.FanAngle = kFujitsuAc264FanAngleStay;  // Set the fan to stay.
  _cmd = _.Cmd = kFujitsuAc264CmdSwing;
  _.SubCmd = 0;
}

/// Get the requested swing operation mode of the A/C unit.
/// @return True if swing is enabled, false if disabled.
bool IRFujitsuAC264::getSwing(void) const { return _.Swing; }

/// Set weather economy mode is enabled or not.
/// @param[in] on True if economy mode is enabled, false if disabled.
void IRFujitsuAC264::setEconomy(const bool on) {
  _.Economy = on;
  _cmd = _.Cmd = kFujitsuAc264CmdEconomy;
  _.SubCmd = 0;
}

/// Get the requested economy mode of the A/C unit.
/// @return True if economy mode is enabled, false if disabled.
bool IRFujitsuAC264::getEconomy(void) const { return _.Economy; }

/// Set weather clean mode is enabled or not.
/// @param[in] on True if swing is enabled, false if disabled.
void IRFujitsuAC264::setClean(const bool on) {
  _.Clean = on;
  _cmd = _.Cmd = kFujitsuAc264CmdClean;
  _.SubCmd = 0;
}

/// Get the requested clean mode of the A/C unit.
/// @return True if clean is enabled, false if disabled.
bool IRFujitsuAC264::getClean(void) const { return _.Clean; }

/// Toggle the sterilization.
/// @note This command is valid only when AC's power is off.
void IRFujitsuAC264::toggleSterilization(void) {
  if (getPower())
    return;
  _cmd = kFujitsuAc264SpCmdToggleSterilization;
  std::memcpy(_.raw, kFujitsuAc264StatesToggleSterilization,
              kFujitsuAc264StateLengthMiddle);
}

/// Set weather outside quiet mode is enabled or not.
/// @param[in] on True if outside quiet is enabled, false if disabled.
/// @note This command is valid only when AC's power is off.
void IRFujitsuAC264::setOutsideQuiet(const bool on) {
  if (getPower())
    return;
  if (on) {
    _cmd = kFujitsuAc264SpCmdOutsideQuietOn;
    std::memcpy(_.raw, kFujitsuAc264StatesOutsideQuietOn,
                kFujitsuAc264StateLengthMiddle);
  } else {
    _cmd = kFujitsuAc264SpCmdOutsideQuietOff;
    std::memcpy(_.raw, kFujitsuAc264StatesOutsideQuietOff,
                kFujitsuAc264StateLengthMiddle);
  }
}

/// Get the requested outside quiet mode of the A/C unit.
/// @return True if outside quiet is enabled, false if disabled.
bool IRFujitsuAC264::getOutsideQuiet(void) const { return _isoutsidequiet; }

/// Set weather economy fan mode is enabled or not.
/// @param[in] on True if economy fan mode is enabled, false if disabled.
/// @note This command is valid only when AC's power is off.
void IRFujitsuAC264::setEcoFan(const bool on) {
  if (getPower())
    return;
  if (on) {
    _cmd = kFujitsuAc264SpCmdEcoFanOn;
    std::memcpy(_.raw, kFujitsuAc264StatesEcoFanOn,
                kFujitsuAc264StateLengthShort);
  } else {
    _cmd = kFujitsuAc264SpCmdEcoFanOff;
    std::memcpy(_.raw, kFujitsuAc264StatesEcoFanOff,
                kFujitsuAc264StateLengthShort);
  }
}

/// Get the requested economy fan mode of the A/C unit.
/// @return True if economy fan mode is enabled, false if disabled.
bool IRFujitsuAC264::getEcoFan(void) const { return _isecofan; }

/// Toggle the powerful mode.
/// @note This command is valid only when AC's power is on.
void IRFujitsuAC264::togglePowerful(void) {
  if (!getPower())
    return;
  _cmd = kFujitsuAc264SpCmdTogglePowerful;
  std::memcpy(_.raw, kFujitsuAc264StatesTogglePowerful,
              kFujitsuAc264StateLengthShort);
}

/// Set the clock on the A/C unit.
/// @param[in] mins_since_midnight Nr. of minutes past midnight.
void IRFujitsuAC264::setClock(const uint16_t mins_since_midnight) {
  uint16_t mins = mins_since_midnight;
  if (mins_since_midnight >= 24 * 60) mins = 0;  // Bounds check.
  // Hours.
  _.ClockHours = mins / 60;
  // Minutes.
  _.ClockMins = mins % 60;
}

/// Get the clock time to be sent to the A/C unit.
/// @return The number of minutes past midnight.
uint16_t IRFujitsuAC264::getClock(void) const {
  return (_.ClockHours * 60 + _.ClockMins);
}

/// Set the sleep timer setting of the A/C.
/// @param[in] mins Minutes to set the timer to. 0 means disabled.
void IRFujitsuAC264::setSleepTimer(const uint16_t mins) {
  if (mins == 0) {
    _.SleepTimerEnable = false;
    _.SleepTimer = 0;
    _cmd = _.Cmd = kFujitsuAc264CmdCancelSleepTimer;
    _.SubCmd = 0;
  } else if (mins <= kFujitsuAc264SleepTimerMax) {
    _.SleepTimerEnable = true;
    _.SleepTimer = 0x800 + mins;
    _cmd = _.Cmd = kFujitsuAc264CmdSleepTime;
    _.SubCmd = 1;
  }
}

/// Get the sleep timer setting of the A/C.
/// @return Minutes left on the timer. 0 means disabled/not supported.
uint16_t IRFujitsuAC264::getSleepTimer(void) const {
  if (_.SleepTimerEnable)
    return (_.SleepTimer - 0x800);
  return 0;
}

/// Set the Timer enable of the A/C message.
/// @param[in] timer_enable The kind of timer to enable for the message.
void IRFujitsuAC264::setTimerEnable(const uint8_t timer_enable) {
  switch (timer_enable) {
    case kFujitsuAc264OnTimerEnable:
      _.TimerEnable = timer_enable;
      _cmd = _.Cmd = kFujitsuAc264CmdOnTimer;
      _.SubCmd = 0;
      break;
    case kFujitsuAc264OffTimerEnable:
    case kFujitsuAc264OnOffTimerEnable:
      _.TimerEnable = timer_enable;
      _cmd = _.Cmd = kFujitsuAc264CmdOffTimer;
      _.SubCmd = 0;
      break;
    case kFujitsuAc264OnOffTimerDisable:
      _.TimerEnable = timer_enable;
      _cmd = _.Cmd = kFujitsuAc264CmdCancelOnOffTimer;
      _.SubCmd = 0;
      break;
    default:
      _.TimerEnable = kFujitsuAc264OnOffTimerDisable;
      _cmd = _.Cmd = kFujitsuAc264CmdCancelOnOffTimer;
      _.SubCmd = 0;
      break;
  }
}

/// Get the Timer enable of the A/C message.
/// @return The current timer enable in numeric form.
uint8_t IRFujitsuAC264::getTimerEnable(void) const { return _.TimerEnable; }

/// Set the on timer setting of the A/C.
/// @param[in] mins10 Time in 10 minutes unit, when the A/C will turn on.
///                   0 means 0:00 AM, 1 means 0:10 AM.
void IRFujitsuAC264::setOnTimer(const uint8_t mins10) {
  if (mins10 <= kFujitsuAc26OnOffTimerMax)
    _.OnTimer = mins10;
}

/// Get the on timer setting of the A/C.
/// @return Time in 10 minutes unit, when the A/C will turn on.
///         0 means 0:00 AM, 1 means 0:10 AM.
uint8_t IRFujitsuAC264::getOnTimer(void) const { return _.OnTimer; }

/// Set the off timer setting of the A/C.
/// @param[in] mins10 Time in 10 minutes unit, when the A/C will turn off.
///                   0 means 0:00 AM, 1 means 0:10 AM.
void IRFujitsuAC264::setOffTimer(const uint8_t mins10) {
  if (mins10 <= kFujitsuAc26OnOffTimerMax)
    _.OffTimer = mins10;
}

/// Get the off timer setting of the A/C.
/// @return Time in 10 minutes unit, when the A/C will turn off.
///         0 means 0:00 AM, 1 means 0:10 AM.
uint8_t IRFujitsuAC264::getOffTimer(void) const { return _.OffTimer; }

/// Set the requested (normal) command part for the A/C message.
/// @param[in] cmd Command to be set.
/// @note Only normal commands (=!isSpecialCommand()) can be set
///       with this function.
void IRFujitsuAC264::setCmd(const uint8_t cmd) {
  switch (cmd) {
    case kFujitsuAc264CmdCool:
    case kFujitsuAc264CmdHeat:
    case kFujitsuAc264CmdDry:
    case kFujitsuAc264CmdAuto:
    case kFujitsuAc264CmdFan:
    case kFujitsuAc264CmdSleepTime:
      _cmd = _.Cmd = cmd;
      _.SubCmd = 1;
      break;
    case kFujitsuAc264CmdTemp:
    case kFujitsuAc264CmdSwing:
    case kFujitsuAc264CmdEconomy:
    case kFujitsuAc264CmdClean:
    case kFujitsuAc264CmdFanSpeed:
    case kFujitsuAc264CmdFanAngle:
    case kFujitsuAc264CmdCancelSleepTimer:
    case kFujitsuAc264CmdOnTimer:
    case kFujitsuAc264CmdOffTimer:
    case kFujitsuAc264CmdCancelOnOffTimer:
      _cmd = _.Cmd = cmd;
      _.SubCmd = 0;
      break;
    default:
      break;
  }
}

/// Get the requested command part for the A/C message.
/// @return The command code.
uint8_t IRFujitsuAC264::getCmd(void) const {
  return _cmd;
}

/// Convert a stdAc::opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRFujitsuAC264::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kCool: return kFujitsuAc264ModeCool;
    case stdAc::opmode_t::kHeat: return kFujitsuAc264ModeHeat;
    case stdAc::opmode_t::kDry:  return kFujitsuAc264ModeDry;
    case stdAc::opmode_t::kFan:  return kFujitsuAc264ModeFan;
    default:                     return kFujitsuAc264ModeAuto;
  }
}

/// Convert a stdAc::fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRFujitsuAC264::convertFanSpeed(stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin:    return kFujitsuAc264FanSpeedQuiet;
    case stdAc::fanspeed_t::kLow:    return kFujitsuAc264FanSpeedLow;
    case stdAc::fanspeed_t::kMedium: return kFujitsuAc264FanSpeedMed;
    case stdAc::fanspeed_t::kHigh:
    case stdAc::fanspeed_t::kMax:    return kFujitsuAc264FanSpeedHigh;
    default:                         return kFujitsuAc264FanSpeedAuto;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::opmode_t IRFujitsuAC264::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kFujitsuAc264ModeCool: return stdAc::opmode_t::kCool;
    case kFujitsuAc264ModeHeat: return stdAc::opmode_t::kHeat;
    case kFujitsuAc264ModeDry:  return stdAc::opmode_t::kDry;
    case kFujitsuAc264ModeFan:  return stdAc::opmode_t::kFan;
    default:                    return stdAc::opmode_t::kAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::fanspeed_t IRFujitsuAC264::toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kFujitsuAc264FanSpeedHigh:   return stdAc::fanspeed_t::kMax;
    case kFujitsuAc264FanSpeedMed:    return stdAc::fanspeed_t::kMedium;
    case kFujitsuAc264FanSpeedLow:    return stdAc::fanspeed_t::kLow;
    case kFujitsuAc264FanSpeedQuiet:  return stdAc::fanspeed_t::kMin;
    default:                          return stdAc::fanspeed_t::kAuto;
  }
}

/// Convert the current internal state into its stdAc::state_t equivalent.
/// @param[in] prev Ptr to a previous state.
/// @return The stdAc equivalent of the native settings.
stdAc::state_t IRFujitsuAC264::toCommon(const stdAc::state_t *prev) {
  stdAc::state_t result{};
  if (prev != NULL) result = *prev;
  result.protocol = decode_type_t::FUJITSU_AC264;
  checkSum();
  result.power = _cmd != kFujitsuAc264SpCmdTurnOff;
  // Only update these settings if it is not a special command message,
  // or we have no previous state info for those settings.
  if (!isSpecialCommand() || prev == NULL) {
    result.mode = toCommonMode(_.Mode);
    result.celsius = true;
    result.degrees = getTemp();
    result.fanspeed = toCommonFanSpeed(_.FanSpeed);
    result.clean = getClean();
    result.swingv = getSwing()? stdAc::swingv_t::kAuto :
                             stdAc::swingv_t::kOff;
    result.econo = getEconomy();
    result.clock = getClock();
    uint16_t sleep_time = getSleepTimer();
    result.sleep = sleep_time? sleep_time: -1;
  }
  result.quiet = getEcoFan();
  // Not supported.
  result.turbo = false;
  result.swingh = stdAc::swingh_t::kOff;
  result.light = false;
  result.filter = false;
  result.beep = false;
  return result;
}

/// Convert the current internal state into a human readable string.
/// @return A human readable string.
String IRFujitsuAC264::toString(void) const {
  String result = "";
  result.reserve(180);  // Reserve some heap for the string to reduce fragging.
  if (isSpecialCommand()) {   // Special commands
    result += kCommandStr;
    result += kColonSpaceStr;
    switch (_cmd) {
      case kFujitsuAc264SpCmdTurnOff:
        result += "Power Off";
        break;
      case kFujitsuAc264SpCmdTogglePowerful:
        result += kPowerfulStr;
        break;
      case kFujitsuAc264SpCmdEcoFanOff:
        result += "Eco Fan ";
        result += kOffStr;
        break;
      case kFujitsuAc264SpCmdEcoFanOn:
        result += "Eco Fan ";
        result += kOnStr;
        break;
      case kFujitsuAc264SpCmdOutsideQuietOff:
        result += "Outside Quiet ";
        result += kOffStr;
        break;
      case kFujitsuAc264SpCmdOutsideQuietOn:
        result += "Outside Quiet ";
        result += kOnStr;
        break;
      case kFujitsuAc264SpCmdToggleSterilization:
        result += "Sterilization";
        break;
      default:
        result += kNAStr;
    }
  } else {    // Normal commands
    result += addBoolToString(true, kPowerStr, false);
    // Mode
    result += addModeToString(_.Mode, kFujitsuAc264ModeAuto,
                kFujitsuAc264ModeCool, kFujitsuAc264ModeHeat,
                kFujitsuAc264ModeDry, kFujitsuAc264ModeFan);
    // Temp
    float degrees = getTemp();
    result += addTempFloatToString(getTemp());
    // Temp in Auto
    degrees = getTempAuto();
    String degrees_str = (degrees >= 0)? uint64ToString(degrees):
                          String(kDashStr) + uint64ToString(-degrees);
    result += addLabeledString(degrees_str, "Temp (Auto)");
    if (((uint16_t)(2 * degrees)) & 1) result += F(".5");
    result += 'C';
    // Fan Speed
    result += addFanToString(_.FanSpeed, kFujitsuAc264FanSpeedHigh,
                kFujitsuAc264FanSpeedLow, kFujitsuAc264FanSpeedAuto,
                kFujitsuAc264FanSpeedQuiet, kFujitsuAc264FanSpeedMed);
    // Fan Angle
    result += addIntToString(_.FanAngle, "Fan Angle");
    result += kSpaceLBraceStr;
    switch (_.FanAngle) {
      case kFujitsuAc264FanAngle1:
        result += kHighestStr;
        break;
      case kFujitsuAc264FanAngle2:
        result += kHighStr;
        break;
      case kFujitsuAc264FanAngle4:
        result += kMiddleStr;
        break;
      case kFujitsuAc264FanAngle6:
        result += kLowStr;
        break;
      case kFujitsuAc264FanAngle7:
        result += kLowestStr;
        break;
      case kFujitsuAc264FanAngle3:
        result += kUpperStr;
        result += ' ';
        result += kMiddleStr;
        break;
      case kFujitsuAc264FanAngle5:
        result += kLowerStr;
        result += ' ';
        result += kMiddleStr;
        break;
      case kFujitsuAc264FanAngleStay:
        result += "Stay";
        break;
      default:
        result += kUnknownStr;
    }
    result += ')';
    // Swing
    result += addBoolToString(getSwing(), kSwingStr);
    // Low Power
    result += addBoolToString(getEconomy(), "Economy");
    // Clean
    result += addBoolToString(getClean(), kCleanStr);
    // Cmd
    result += kCommaSpaceStr;
    result += kCommandStr;
    result += kColonSpaceStr;
    switch (_.Cmd) {
      case kFujitsuAc264CmdCool:
        result += kCoolStr;
        break;
      case kFujitsuAc264CmdHeat:
        result += kHeatStr;
        break;
      case kFujitsuAc264CmdDry:
        if (_.WeakDry)
          result += "Weak ";
        result += kDryStr;
        break;
      case kFujitsuAc264CmdAuto:
        result += kAutoStr;
        break;
      case kFujitsuAc264CmdFan:
        result += kFanStr;
        break;
      case kFujitsuAc264CmdTemp:
        result += kTempStr;
        break;
      case kFujitsuAc264CmdSwing:
        result += kSwingStr;
        break;
      case kFujitsuAc264CmdSleepTime:
        result += kSleepTimerStr;
        break;
      case kFujitsuAc264CmdEconomy:
        result += "Economy";
        break;
      case kFujitsuAc264CmdClean:
        result += kCleanStr;
        break;
      case kFujitsuAc264CmdFanSpeed:
        result += "Fan Speed";
        break;
      case kFujitsuAc264CmdFanAngle:
        result += "Fan Angle";
        break;
      case kFujitsuAc264CmdCancelSleepTimer:
        result += "Cancel Sleep Timer";
        break;
      case kFujitsuAc264CmdOffTimer:
        result += kOffTimerStr;
        break;
      case kFujitsuAc264CmdOnTimer:
        result += kOnTimerStr;
        break;
      case kFujitsuAc264CmdCancelOnOffTimer:
        result += "Cancel On/Off Timer";
        break;
      default:
        result += kNAStr;
    }
    // Clock
    uint16_t mins = 0;
    mins = getClock();
    result += addLabeledString(mins ? minsToString(mins) : kNAStr,
                               "Current Time");
    // Sleep Timer
    mins = getSleepTimer();
    result += addLabeledString(mins ? minsToString(mins) : kOffStr,
                               kSleepTimerStr);
    // On/Off Timer
    uint8_t timer_enable = getTimerEnable();
    mins = getOnTimer();
    result += addLabeledString((timer_enable & kFujitsuAc264OnTimerEnable)?
                                minsToString(mins * 10): kOffStr, kOnTimerStr);
    mins = getOffTimer();
    result += addLabeledString((timer_enable & kFujitsuAc264OffTimerEnable)?
                                minsToString(mins * 10): kOffStr, kOffTimerStr);
  }
  return result;
}

#if DECODE_FUJITSU_AC264
/// Decode the supplied Fujitsu 264 bit AC IR message if possible.
/// Status: STABLE / Working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool IRrecv::decodeFujitsuAC264(decode_results* results, uint16_t offset,
                                const uint16_t nbits,
                                const bool strict) {
  uint16_t dataBitsSoFar = 0;
  uint8_t restLength = 0;

  // Have we got enough data to successfully decode?
  if (results->rawlen < (2 * kFujitsuAc264BitsShort) + kHeader + kFooter - 1 +
      offset)
    return false;  // Can't possibly be a valid message.

  // Compliance
  if (strict) {
    switch (nbits) {
      case kFujitsuAc264Bits:
      case kFujitsuAc264BitsMiddle:
      case kFujitsuAc264BitsShort: break;
      default: return false;  // Must be called with the correct nr. of bits.
    }
  }

  // Header / Some of the Data
  uint16_t used = matchGeneric(results->rawbuf + offset, results->state,
                               results->rawlen - offset, kFujitsuAc264BitsShort,
                               kFujitsuAcHdrMark, kFujitsuAcHdrSpace,  // Header
                               kFujitsuAcBitMark, kFujitsuAcOneSpace,  // Data
                               kFujitsuAcBitMark, kFujitsuAcZeroSpace,
                               0, 0,  // No Footer (yet)
                               false, _tolerance + kFujitsuAcExtraTolerance, 0,
                               false);  // LSBF
  offset += used;
  dataBitsSoFar += kFujitsuAc264BitsShort;

  // Check if it has the Fujitsu AC264 protocol header
  if (results->state[0] != 0x14 || results->state[1] != 0x63 ||
      results->state[2] != 0x00 || results->state[3] != 0x10 ||
      results->state[4] != 0x10)
      return false;

  // Identify which command it is
  switch (results->state[5]) {
    case 0xFE:    // Command length is normal or middle
      restLength = results->state[6];
      // check the rest length
      if ((restLength != 0x1A) && (restLength != 0x09))
        return false;
      break;
    case 0x51:    // Command length is short
    case 0x50:    // Command length is short
    case 0x39:    // Command length is short
    case 0x02:    // Command length is short
      if (results->state[6] == (uint8_t)~results->state[5]) {    // checksum
        results->decode_type = FUJITSU_AC264;
        results->bits = dataBitsSoFar;
        return true;
      } else {
        return false;
      }
    default:
      return false;
  }

  // Keep reading bytes until we either run out of message or state to fill.
  match_result_t data_result;
  for (uint16_t i = kFujitsuAc264StateLengthShort;
       offset <= results->rawlen - 16 && i < kFujitsuAc264StateLengthShort +
       restLength; i++, dataBitsSoFar += 8, offset += data_result.used) {
    data_result = matchData(
        &(results->rawbuf[offset]), 8, kFujitsuAcBitMark, kFujitsuAcOneSpace,
        kFujitsuAcBitMark, kFujitsuAcZeroSpace,
        _tolerance + kFujitsuAcExtraTolerance, 0, false);
    if (data_result.success == false) break;  // Fail
    results->state[i] = data_result.data;
  }

  // Compliance
  if (strict) {
    if (dataBitsSoFar != nbits) return false;
  }

  // Compliance
  switch (dataBitsSoFar) {
    case kFujitsuAc264BitsMiddle:
    case kFujitsuAc264Bits:
      // Check if the state[5] is matched with the protocol.
      if (results->state[5] != 0xFE) return false;
      break;
    default:
      return false;  // Unexpected size.
  }

  if (!IRFujitsuAC264::validChecksum(results->state, dataBitsSoFar / 8))
    return false;

  // Success
  results->decode_type = FUJITSU_AC264;
  results->bits = dataBitsSoFar;
  return true;  // All good.
}
#endif  // DECODE_FUJITSU_AC264
