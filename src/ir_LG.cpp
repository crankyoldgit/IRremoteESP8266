// Copyright 2015 Darryl Smith
// Copyright 2015 cheaplin
// Copyright 2017, 2018 David Conran

/// @file
/// @brief Support for LG protocols.
/// LG decode originally added by Darryl Smith (based on the JVC protocol)
/// LG send originally added by https://github.com/chaeplin
/// @see https://github.com/arendst/Tasmota/blob/54c2eb283a02e4287640a4595e506bc6eadbd7f2/sonoff/xdrv_05_irremote.ino#L327-438

#include "ir_LG.h"
#include <algorithm>
#include "IRac.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "IRtext.h"
#include "IRutils.h"

using irutils::addBoolToString;
using irutils::addModeToString;
using irutils::addModelToString;
using irutils::addFanToString;
using irutils::addTempToString;
using irutils::setBit;
using irutils::setBits;


// Constants
const uint16_t kLgTick = 50;
const uint16_t kLgHdrMarkTicks = 170;
const uint16_t kLgHdrMark = kLgHdrMarkTicks * kLgTick;  // 8500
const uint16_t kLgHdrSpaceTicks = 85;
const uint16_t kLgHdrSpace = kLgHdrSpaceTicks * kLgTick;  // 4250
const uint16_t kLgBitMarkTicks = 11;
const uint16_t kLgBitMark = kLgBitMarkTicks * kLgTick;  // 550
const uint16_t kLgOneSpaceTicks = 32;
const uint16_t kLgOneSpace = kLgOneSpaceTicks * kLgTick;  // 1600
const uint16_t kLgZeroSpaceTicks = 11;
const uint16_t kLgZeroSpace = kLgZeroSpaceTicks * kLgTick;  // 550
const uint16_t kLgRptSpaceTicks = 45;
const uint16_t kLgRptSpace = kLgRptSpaceTicks * kLgTick;  // 2250
const uint16_t kLgMinGapTicks = 795;
const uint16_t kLgMinGap = kLgMinGapTicks * kLgTick;  // 39750
const uint16_t kLgMinMessageLengthTicks = 2161;
const uint32_t kLgMinMessageLength = kLgMinMessageLengthTicks * kLgTick;

const uint16_t kLg32HdrMarkTicks = 90;
const uint16_t kLg32HdrMark = kLg32HdrMarkTicks * kLgTick;  // 4500
const uint16_t kLg32HdrSpaceTicks = 89;
const uint16_t kLg32HdrSpace = kLg32HdrSpaceTicks * kLgTick;  // 4450
const uint16_t kLg32RptHdrMarkTicks = 179;
const uint16_t kLg32RptHdrMark = kLg32RptHdrMarkTicks * kLgTick;  // 8950

const uint16_t kLg2HdrMarkTicks = 64;
const uint16_t kLg2HdrMark = kLg2HdrMarkTicks * kLgTick;  // 3200
const uint16_t kLg2HdrSpaceTicks = 197;
const uint16_t kLg2HdrSpace = kLg2HdrSpaceTicks * kLgTick;  // 9850
const uint16_t kLg2BitMarkTicks = 10;
const uint16_t kLg2BitMark = kLg2BitMarkTicks * kLgTick;  // 500

#if SEND_LG
/// Send an LG formatted message. (LG)
/// Status: Beta / Should be working.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
///   Typically kLgBits or kLg32Bits.
/// @param[in] repeat The number of times the command is to be repeated.
/// @note LG has a separate message to indicate a repeat, like NEC does.
void IRsend::sendLG(uint64_t data, uint16_t nbits, uint16_t repeat) {
  uint16_t repeatHeaderMark = 0;
  uint8_t duty = kDutyDefault;

  if (nbits >= kLg32Bits) {
    // LG 32bit protocol is near identical to Samsung except for repeats.
    sendSAMSUNG(data, nbits, 0);  // Send it as a single Samsung message.
    repeatHeaderMark = kLg32RptHdrMark;
    duty = 33;
    repeat++;
  } else {
    // LG (28-bit) protocol.
    repeatHeaderMark = kLgHdrMark;
    sendGeneric(kLgHdrMark, kLgHdrSpace, kLgBitMark, kLgOneSpace, kLgBitMark,
                kLgZeroSpace, kLgBitMark, kLgMinGap, kLgMinMessageLength, data,
                nbits, 38, true, 0,  // Repeats are handled later.
                duty);
  }

  // Repeat
  // Protocol has a mandatory repeat-specific code sent after every command.
  if (repeat)
    sendGeneric(repeatHeaderMark, kLgRptSpace, 0, 0, 0, 0,  // No data is sent.
                kLgBitMark, kLgMinGap, kLgMinMessageLength, 0, 0,  // No data.
                38, true, repeat - 1, duty);
}

/// Send an LG Variant-2 formatted message. (LG2)
/// Status: Beta / Should be working.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
///   Typically kLgBits or kLg32Bits.
/// @param[in] repeat The number of times the command is to be repeated.
/// @note LG has a separate message to indicate a repeat, like NEC does.
void IRsend::sendLG2(uint64_t data, uint16_t nbits, uint16_t repeat) {
  if (nbits >= kLg32Bits) {
    // Let the original routine handle it.
    sendLG(data, nbits, repeat);  // Send it as a single Samsung message.
    return;
  }

  // LGv2 (28-bit) protocol.
  sendGeneric(kLg2HdrMark, kLg2HdrSpace, kLgBitMark, kLgOneSpace, kLgBitMark,
              kLgZeroSpace, kLgBitMark, kLgMinGap, kLgMinMessageLength, data,
              nbits, 38, true, 0,  // Repeats are handled later.
              50);

  // TODO(crackn): Verify the details of what repeat messages look like.
  // Repeat
  // Protocol has a mandatory repeat-specific code sent after every command.
  if (repeat)
    sendGeneric(kLg2HdrMark, kLgRptSpace, 0, 0, 0, 0,  // No data is sent.
                kLgBitMark, kLgMinGap, kLgMinMessageLength, 0, 0,  // No data.
                38, true, repeat - 1, 50);
}

/// Construct a raw 28-bit LG message code from the supplied address & command.
/// Status: STABLE / Works.
/// @param[in] address The address code.
/// @param[in] command The command code.
/// @return A raw 28-bit LG message code suitable for sendLG() etc.
/// @note Sequence of bits = address + command + checksum.
uint32_t IRsend::encodeLG(uint16_t address, uint16_t command) {
  return ((address << 20) | (command << 4) | irutils::sumNibbles(command, 4));
}
#endif  // SEND_LG

#if DECODE_LG
/// Decode the supplied LG message.
/// Status: STABLE / Working.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
///   Typically kLgBits or kLg32Bits.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return True if it can decode it, false if it can't.
/// @note LG protocol has a repeat code which is 4 items long.
///   Even though the protocol has 28/32 bits of data, only 24/28 bits are
///   distinct.
///   In transmission order, the 28/32 bits are constructed as follows:
///     8/12 bits of address + 16 bits of command + 4 bits of checksum.
/// @note LG 32bit protocol appears near identical to the Samsung protocol.
///   They possibly differ on how they repeat and initial HDR mark.
/// @see https://funembedded.wordpress.com/2014/11/08/ir-remote-control-for-lg-conditioner-using-stm32f302-mcu-on-mbed-platform/
bool IRrecv::decodeLG(decode_results *results, uint16_t offset,
                      const uint16_t nbits, const bool strict) {
  if (nbits >= kLg32Bits) {
    if (results->rawlen <= 2 * nbits + 2 * (kHeader + kFooter) - 1 + offset)
      return false;  // Can't possibly be a valid LG32 message.
  } else {
    if (results->rawlen <= 2 * nbits + kHeader - 1 + offset)
      return false;  // Can't possibly be a valid LG message.
  }
  if (strict && nbits != kLgBits && nbits != kLg32Bits)
    return false;  // Doesn't comply with expected LG protocol.
  uint64_t data = 0;
  bool isLg2 = false;

  // Header
  uint32_t m_tick;
  if (matchMark(results->rawbuf[offset], kLgHdrMark)) {
    m_tick = results->rawbuf[offset++] * kRawTick / kLgHdrMarkTicks;
  } else if (matchMark(results->rawbuf[offset], kLg2HdrMark)) {
    m_tick = results->rawbuf[offset++] * kRawTick / kLg2HdrMarkTicks;
    isLg2 = true;
  } else if (matchMark(results->rawbuf[offset], kLg32HdrMark)) {
    m_tick = results->rawbuf[offset++] * kRawTick / kLg32HdrMarkTicks;
  } else {
    return false;
  }
  uint32_t s_tick;
  if (isLg2) {
    if (matchSpace(results->rawbuf[offset], kLg2HdrSpace))
      s_tick = results->rawbuf[offset++] * kRawTick / kLg2HdrSpaceTicks;
    else
      return false;
  } else {
    if (matchSpace(results->rawbuf[offset], kLgHdrSpace))
      s_tick = results->rawbuf[offset++] * kRawTick / kLgHdrSpaceTicks;
    else if (matchSpace(results->rawbuf[offset], kLg2HdrSpace))
      s_tick = results->rawbuf[offset++] * kRawTick / kLg32HdrSpaceTicks;
    else
      return false;
  }

  // Set up the expected tick sizes based on variant.
  uint16_t bitmarkticks;
  if (isLg2) {
    bitmarkticks = kLg2BitMarkTicks;
  } else {
    bitmarkticks = kLgBitMarkTicks;
  }

  // Data
  match_result_t data_result =
      matchData(&(results->rawbuf[offset]), nbits, bitmarkticks * m_tick,
                kLgOneSpaceTicks * s_tick, bitmarkticks * m_tick,
                kLgZeroSpaceTicks * s_tick, _tolerance, 0);
  if (data_result.success == false) return false;
  data = data_result.data;
  offset += data_result.used;

  // Footer
  if (!matchMark(results->rawbuf[offset++], bitmarkticks * m_tick))
    return false;
  if (offset < results->rawlen &&
      !matchAtLeast(results->rawbuf[offset], kLgMinGapTicks * s_tick))
    return false;

  // Repeat
  if (nbits >= kLg32Bits) {
    // If we are expecting the LG 32-bit protocol, there is always
    // a repeat message. So, check for it.
    offset++;
    if (!matchMark(results->rawbuf[offset++], kLg32RptHdrMarkTicks * m_tick))
      return false;
    if (!matchSpace(results->rawbuf[offset++], kLgRptSpaceTicks * s_tick))
      return false;
    if (!matchMark(results->rawbuf[offset++], bitmarkticks * m_tick))
      return false;
    if (offset < results->rawlen &&
        !matchAtLeast(results->rawbuf[offset], kLgMinGapTicks * s_tick))
      return false;
  }

  // Compliance
  uint16_t command = (data >> 4) & 0xFFFF;  // The 16 bits before the checksum.

  if (strict && (data & 0xF) != irutils::sumNibbles(command, 4))
    return false;  // The last 4 bits sent are the expected checksum.
  // Success
  if (isLg2)
    results->decode_type = LG2;
  else
    results->decode_type = LG;
  results->bits = nbits;
  results->value = data;
  results->command = command;
  results->address = data >> 20;  // The bits before the command.
  return true;
}
#endif

// LG A/C Class

/// Class constructor
/// @param[in] pin GPIO to be used when sending.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRLgAc::IRLgAc(const uint16_t pin, const bool inverted,
               const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) { this->stateReset(); }

/// Reset the internals of the object to a known good state.
void IRLgAc::stateReset(void) {
  setRaw(kLgAcOffCommand);
  setModel(lg_ac_remote_model_t::GE6711AR2853M);
}

/// Set up hardware to be able to send a message.
void IRLgAc::begin(void) { _irsend.begin(); }

#if SEND_LG
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRLgAc::send(const uint16_t repeat) {
  if (this->getPower())
    _irsend.send(this->_protocol, this->getRaw(), kLgBits, repeat);
  else
    // Always send the special Off command if the power is set to off.
    // Ref: https://github.com/crankyoldgit/IRremoteESP8266/issues/1008#issuecomment-570763580
    _irsend.send(this->_protocol, kLgAcOffCommand, kLgBits, repeat);
}
#endif  // SEND_LG

/// Set the model of the A/C to emulate.
/// @param[in] model The enum of the appropriate model.
void IRLgAc::setModel(const lg_ac_remote_model_t model) {
  switch (model) {
    case lg_ac_remote_model_t::AKB75215403:
      _protocol = decode_type_t::LG2;
      break;
    case lg_ac_remote_model_t::GE6711AR2853M:
      // FALL THRU
    default:
      _protocol = decode_type_t::LG;
  }
}

/// Get the model of the A/C.
/// @return The enum of the compatible model.
lg_ac_remote_model_t IRLgAc::getModel(void) {
  switch (_protocol) {
    case LG2:
      return lg_ac_remote_model_t::AKB75215403;
    case LG:
      // FALL THRU
    default:
      return lg_ac_remote_model_t::GE6711AR2853M;
  }
}

/// Get a copy of the internal state/code for this protocol.
/// @return The code for this protocol based on the current internal state.
uint32_t IRLgAc::getRaw(void) {
  checksum();
  return remote_state;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
void IRLgAc::setRaw(const uint32_t new_code) {
  remote_state = new_code;
  _temp = 15;  // Ensure there is a "sane" previous temp.
  _temp = getTemp();
}

/// Calculate the checksum for a given state.
/// @param[in] state The value to calc the checksum of.
/// @return The calculated checksum value.
uint8_t IRLgAc::calcChecksum(const uint32_t state) {
  return irutils::sumNibbles(state >> 4, 4);
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The value to verify the checksum of.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool IRLgAc::validChecksum(const uint32_t state) {
  return calcChecksum(state) == GETBITS32(state, kLgAcChecksumOffset,
                                          kLgAcChecksumSize);
}

/// Calculate and set the checksum values for the internal state.
void IRLgAc::checksum(void) {
  setBits(&remote_state, kLgAcChecksumOffset, kLgAcChecksumSize,
          calcChecksum(remote_state));
}

/// Change the power setting to On.
void IRLgAc::on(void) { setPower(true); }

/// Change the power setting to Off.
void IRLgAc::off(void) { setPower(false); }

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRLgAc::setPower(const bool on) {
  setBits(&remote_state, kLgAcPowerOffset, kLgAcPowerSize,
          on ? kLgAcPowerOn : kLgAcPowerOff);
  if (on)
    setTemp(_temp);  // Reset the temp if we are on.
  else
    _setTemp(0);  // Off clears the temp.
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool IRLgAc::getPower(void) {
  return GETBITS32(remote_state, kLgAcPowerOffset, kLgAcPowerSize) ==
      kLgAcPowerOn;
}

/// Set the temperature.
/// @param[in] value The native temperature.
/// @note Internal use only.
void IRLgAc::_setTemp(const uint8_t value) {
  setBits(&remote_state, kLgAcTempOffset, kLgAcTempSize, value);
}

/// Set the temperature.
/// @param[in] degrees The temperature in degrees celsius.
void IRLgAc::setTemp(const uint8_t degrees) {
  uint8_t temp = std::max(kLgAcMinTemp, degrees);
  temp = std::min(kLgAcMaxTemp, temp);
  _temp = temp;
  _setTemp(temp - kLgAcTempAdjust);
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t IRLgAc::getTemp(void) {
  if (getPower())
    return GETBITS32(remote_state, kLgAcTempOffset, kLgAcTempSize) +
        kLgAcTempAdjust;
  else
    return _temp;
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
void IRLgAc::setFan(const uint8_t speed) {
  switch (speed) {
    case kLgAcFanAuto:
    case kLgAcFanLowest:
    case kLgAcFanLow:
    case kLgAcFanMedium:
    case kLgAcFanHigh:
      setBits(&remote_state, kLgAcFanOffset, kLgAcFanSize, speed);
      break;
    default:
      setFan(kLgAcFanAuto);
  }
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t IRLgAc::getFan(void) {
  return GETBITS32(remote_state, kLgAcFanOffset, kLgAcFanSize);
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t IRLgAc::getMode(void) {
  return GETBITS32(remote_state, kLgAcModeOffset, kLgAcModeSize);
}

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
void IRLgAc::setMode(const uint8_t mode) {
  switch (mode) {
    case kLgAcAuto:
    case kLgAcDry:
    case kLgAcHeat:
    case kLgAcCool:
    case kLgAcFan:
      setBits(&remote_state, kLgAcModeOffset, kLgAcModeSize, mode);
      break;
    default:  // If we get an unexpected mode, default to AUTO.
      this->setMode(kLgAcAuto);
  }
}

/// Convert a stdAc::opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivilant of the enum.
uint8_t IRLgAc::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kCool: return kLgAcCool;
    case stdAc::opmode_t::kHeat: return kLgAcHeat;
    case stdAc::opmode_t::kFan:  return kLgAcFan;
    case stdAc::opmode_t::kDry:  return kLgAcDry;
    default:                     return kLgAcAuto;
  }
}

/// Convert a native mode into its stdAc equivilant.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivilant of the native setting.
stdAc::opmode_t IRLgAc::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kLgAcCool: return stdAc::opmode_t::kCool;
    case kLgAcHeat: return stdAc::opmode_t::kHeat;
    case kLgAcDry:  return stdAc::opmode_t::kDry;
    case kLgAcFan:  return stdAc::opmode_t::kFan;
    default:        return stdAc::opmode_t::kAuto;
  }
}

/// Convert a stdAc::fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivilant of the enum.
uint8_t IRLgAc::convertFan(const stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin:    return kLgAcFanLowest;
    case stdAc::fanspeed_t::kLow:    return kLgAcFanLow;
    case stdAc::fanspeed_t::kMedium: return kLgAcFanMedium;
    case stdAc::fanspeed_t::kHigh:
    case stdAc::fanspeed_t::kMax:    return kLgAcFanHigh;
    default:                         return kLgAcFanAuto;
  }
}

/// Convert a native fan speed into its stdAc equivilant.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivilant of the native setting.
stdAc::fanspeed_t IRLgAc::toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kLgAcFanHigh:    return stdAc::fanspeed_t::kMax;
    case kLgAcFanMedium:  return stdAc::fanspeed_t::kMedium;
    case kLgAcFanLow:     return stdAc::fanspeed_t::kLow;
    case kLgAcFanLowest:  return stdAc::fanspeed_t::kMin;
    default:              return stdAc::fanspeed_t::kAuto;
  }
}

/// Convert the current internal state into its stdAc::state_t equivilant.
/// @return The stdAc equivilant of the native settings.
stdAc::state_t IRLgAc::toCommon(void) {
  stdAc::state_t result;
  result.protocol = decode_type_t::LG;
  result.model = this->getModel();
  result.power = this->getPower();
  result.mode = this->toCommonMode(this->getMode());
  result.celsius = true;
  result.degrees = this->getTemp();
  result.fanspeed = this->toCommonFanSpeed(this->getFan());
  // Not supported.
  result.swingv = stdAc::swingv_t::kOff;
  result.swingh = stdAc::swingh_t::kOff;
  result.quiet = false;
  result.turbo = false;
  result.light = false;
  result.filter = false;
  result.clean = false;
  result.econo = false;
  result.beep = false;
  result.sleep = -1;
  result.clock = -1;
  return result;
}

/// Convert the current internal state into a human readable string.
/// @return A human readable string.
String IRLgAc::toString(void) {
  String result = "";
  result.reserve(80);  // Reserve some heap for the string to reduce fragging.
  result += addModelToString(_protocol, getModel(), false);
  result += addBoolToString(getPower(), kPowerStr);
  if (getPower()) {  // Only display the rest if is in power on state.
    result += addModeToString(getMode(), kLgAcAuto, kLgAcCool,
                              kLgAcHeat, kLgAcDry, kLgAcFan);
    result += addTempToString(getTemp());
    result += addFanToString(getFan(), kLgAcFanHigh, kLgAcFanLow,
                             kLgAcFanAuto, kLgAcFanLowest, kLgAcFanMedium);
  }
  return result;
}

/// Check if the internal state looks like a valud LG A/C message.
/// @return true, the internal state is a valid LG A/C mesg. Otherwise, false.
bool IRLgAc::isValidLgAc(void) {
  return validChecksum(remote_state) &&
      (GETBITS32(remote_state, kLgAcSignatureOffset, kLgAcSignatureSize) ==
       kLgAcSignature);
}
