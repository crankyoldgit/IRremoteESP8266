// Copyright 2009 Ken Shirriff
// Copyright 2016 marcosamarinho
// Copyright 2017-2020 David Conran

/// @file
/// @brief Support for Sanyo protocols.
/// Sanyo LC7461 support originally by marcosamarinho
/// Sanyo SA 8650B originally added from
///   https://github.com/shirriff/Arduino-IRremote/
/// @see https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Sanyo.cpp
/// @see http://pdf.datasheetcatalog.com/datasheet/sanyo/LC7461.pdf
/// @see https://github.com/marcosamarinho/IRremoteESP8266/blob/master/ir_Sanyo.cpp
/// @see http://slydiman.narod.ru/scr/kb/sanyo.htm
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1211

#include "ir_Sanyo.h"
#include <algorithm>
#include <cstring>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRtext.h"
#include "IRutils.h"

using irutils::addBoolToString;
using irutils::addFanToString;
using irutils::addIntToString;
using irutils::addModeToString;
using irutils::addTempToString;
using irutils::sumNibbles;
using irutils::setBit;
using irutils::setBits;

// Constants
// Sanyo SA 8650B
const uint16_t kSanyoSa8650bHdrMark = 3500;  // seen range 3500
const uint16_t kSanyoSa8650bHdrSpace = 950;  // seen 950
const uint16_t kSanyoSa8650bOneMark = 2400;  // seen 2400
const uint16_t kSanyoSa8650bZeroMark = 700;  // seen 700
// usually see 713 - not using ticks as get number wrapround
const uint16_t kSanyoSa8650bDoubleSpaceUsecs = 800;
const uint16_t kSanyoSa8650bRptLength = 45000;

// Sanyo LC7461
const uint16_t kSanyoLc7461AddressMask = (1 << kSanyoLC7461AddressBits) - 1;
const uint16_t kSanyoLc7461CommandMask = (1 << kSanyoLC7461CommandBits) - 1;
const uint16_t kSanyoLc7461HdrMark = 9000;
const uint16_t kSanyoLc7461HdrSpace = 4500;
const uint16_t kSanyoLc7461BitMark = 560;    // 1T
const uint16_t kSanyoLc7461OneSpace = 1690;  // 3T
const uint16_t kSanyoLc7461ZeroSpace = 560;  // 1T
const uint32_t kSanyoLc7461MinCommandLength = 108000;

const uint16_t kSanyoLc7461MinGap =
    kSanyoLc7461MinCommandLength -
    (kSanyoLc7461HdrMark + kSanyoLc7461HdrSpace +
     kSanyoLC7461Bits * (kSanyoLc7461BitMark +
                         (kSanyoLc7461OneSpace + kSanyoLc7461ZeroSpace) / 2) +
     kSanyoLc7461BitMark);

const uint16_t kSanyoAcHdrMark = 8500;   ///< uSeconds
const uint16_t kSanyoAcHdrSpace = 4200;  ///< uSeconds
const uint16_t kSanyoAcBitMark = 500;    ///< uSeconds
const uint16_t kSanyoAcOneSpace = 1600;  ///< uSeconds
const uint16_t kSanyoAcZeroSpace = 550;  ///< uSeconds
const uint32_t kSanyoAcGap = kDefaultMessageGap;  ///< uSeconds (Guess only)
const uint16_t kSanyoAcFreq = 38000;  ///< Hz. (Guess only)

#if SEND_SANYO
/// Construct a Sanyo LC7461 message.
/// @param[in] address The 13 bit value of the address(Custom) portion of the
///   protocol.
/// @param[in] command The 8 bit value of the command(Key) portion of the
///   protocol.
/// @return An uint64_t with the encoded raw 42 bit Sanyo LC7461 data value.
/// @note This protocol uses the NEC protocol timings. However, data is
///  formatted as : address(13 bits), !address, command(8 bits), !command.
///  According with LIRC, this protocol is used on Sanyo, Aiwa and Chinon
uint64_t IRsend::encodeSanyoLC7461(uint16_t address, uint8_t command) {
  // Mask our input values to ensure the correct bit sizes.
  address &= kSanyoLc7461AddressMask;
  command &= kSanyoLc7461CommandMask;

  uint64_t data = address;
  address ^= kSanyoLc7461AddressMask;  // Invert the 13 LSBs.
  // Append the now inverted address.
  data = (data << kSanyoLC7461AddressBits) | address;
  // Append the command.
  data = (data << kSanyoLC7461CommandBits) | command;
  command ^= kSanyoLc7461CommandMask;  // Invert the command.
  // Append the now inverted command.
  data = (data << kSanyoLC7461CommandBits) | command;

  return data;
}

/// Send a Sanyo LC7461 message.
/// Status: BETA / Probably works.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
/// @note Based on \@marcosamarinho's work.
///   This protocol uses the NEC protocol timings. However, data is
///   formatted as : address(13 bits), !address, command (8 bits), !command.
///   According with LIRC, this protocol is used on Sanyo, Aiwa and Chinon
///   Information for this protocol is available at the Sanyo LC7461 datasheet.
///   Repeats are performed similar to the NEC method of sending a special
///   repeat message, rather than duplicating the entire message.
/// @see https://github.com/marcosamarinho/IRremoteESP8266/blob/master/ir_Sanyo.cpp
/// @see http://pdf.datasheetcatalog.com/datasheet/sanyo/LC7461.pdf
void IRsend::sendSanyoLC7461(const uint64_t data, const uint16_t nbits,
                             const uint16_t repeat) {
  // This protocol appears to be another 42-bit variant of the NEC protocol.
  sendNEC(data, nbits, repeat);
}
#endif  // SEND_SANYO

#if DECODE_SANYO
/// Decode the supplied SANYO LC7461 message.
/// Status: BETA / Probably works.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return True if it can decode it, false if it can't.
/// @note Based on \@marcosamarinho's work.
///   This protocol uses the NEC protocol. However, data is
///   formatted as : address(13 bits), !address, command (8 bits), !command.
///   According with LIRC, this protocol is used on Sanyo, Aiwa and Chinon
///   Information for this protocol is available at the Sanyo LC7461 datasheet.
/// @see http://slydiman.narod.ru/scr/kb/sanyo.htm
/// @see https://github.com/marcosamarinho/IRremoteESP8266/blob/master/ir_Sanyo.cpp
/// @see http://pdf.datasheetcatalog.com/datasheet/sanyo/LC7461.pdf
bool IRrecv::decodeSanyoLC7461(decode_results *results, uint16_t offset,
                               const uint16_t nbits, const bool strict) {
  if (strict && nbits != kSanyoLC7461Bits)
    return false;  // Not strictly in spec.
  // This protocol is basically a 42-bit variant of the NEC protocol.
  if (!decodeNEC(results, offset, nbits, false))
    return false;  // Didn't match a NEC format (without strict)

  // Bits 30 to 42+.
  uint16_t address =
      results->value >> (kSanyoLC7461Bits - kSanyoLC7461AddressBits);
  // Bits 9 to 16.
  uint8_t command =
      (results->value >> kSanyoLC7461CommandBits) & kSanyoLc7461CommandMask;
  // Compliance
  if (strict) {
    if (results->bits != nbits) return false;
    // Bits 17 to 29.
    uint16_t inverted_address =
        (results->value >> (kSanyoLC7461CommandBits * 2)) &
        kSanyoLc7461AddressMask;
    // Bits 1-8.
    uint8_t inverted_command = results->value & kSanyoLc7461CommandMask;
    if ((address ^ kSanyoLc7461AddressMask) != inverted_address)
      return false;  // Address integrity check failed.
    if ((command ^ kSanyoLc7461CommandMask) != inverted_command)
      return false;  // Command integrity check failed.
  }

  // Success
  results->decode_type = SANYO_LC7461;
  results->address = address;
  results->command = command;
  return true;
}

/* NOTE: Disabled due to poor quality.
/// Decode the supplied Sanyo SA 8650B message.
/// Status: Depricated.
/// @depricated Disabled due to poor quality.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return True if it can decode it, false if it can't.
/// @warning This decoder looks like rubbish. Only keeping it for compatibility
///   with the Arduino IRremote library. Seriously, don't trust it.
///   If someone has a device that this is supposed to be for, please log an
///   Issue on github with a rawData dump please. We should probably remove it.
///   We think this is a Sanyo decoder - serial = SA 8650B
/// @see https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Sanyo.cpp
bool IRrecv::decodeSanyo(decode_results *results, uint16_t nbits, bool strict) {
  if (results->rawlen < 2 * nbits + kHeader - 1)
    return false;  // Shorter than shortest possible.
  if (strict && nbits != kSanyoSA8650BBits)
    return false;  // Doesn't match the spec.

  uint16_t offset = 0;

  // TODO(crankyoldgit): This repeat code looks like garbage, it should never
  //   match or if it does, it won't be reliable. We should probably just
  //   remove it.
  if (results->rawbuf[offset++] < kSanyoSa8650bDoubleSpaceUsecs) {
    results->bits = 0;
    results->value = kRepeat;
    results->decode_type = SANYO;
    results->address = 0;
    results->command = 0;
    results->repeat = true;
    return true;
  }

  // Header
  if (!matchMark(results->rawbuf[offset++], kSanyoSa8650bHdrMark))
    return false;
  // NOTE: These next two lines look very wrong. Treat as suspect.
  if (!matchMark(results->rawbuf[offset++], kSanyoSa8650bHdrMark))
    return false;
  // Data
  uint64_t data = 0;
  while (offset + 1 < results->rawlen) {
    if (!matchSpace(results->rawbuf[offset], kSanyoSa8650bHdrSpace))
      break;
    offset++;
    if (matchMark(results->rawbuf[offset], kSanyoSa8650bOneMark))
      data = (data << 1) | 1;  // 1
    else if (matchMark(results->rawbuf[offset], kSanyoSa8650bZeroMark))
      data <<= 1;  // 0
    else
      return false;
    offset++;
  }

  if (strict && kSanyoSA8650BBits > (offset - 1U) / 2U)
    return false;

  // Success
  results->bits = (offset - 1) / 2;
  results->decode_type = SANYO;
  results->value = data;
  results->address = 0;
  results->command = 0;
  return true;
}
*/
#endif  // DECODE_SANYO


#if SEND_SANYO_AC
/// Send a SanyoAc formatted message.
/// Status: ALPHA / Untested.
/// @param[in] data An array of bytes containing the IR command.
/// @param[in] nbytes Nr. of bytes of data in the array.
/// @param[in] repeat Nr. of times the message is to be repeated.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1211
void IRsend::sendSanyoAc(const uint8_t data[], const uint16_t nbytes,
                         const uint16_t repeat) {
  // Header + Data + Footer
  sendGeneric(kSanyoAcHdrMark, kSanyoAcHdrSpace,
              kSanyoAcBitMark, kSanyoAcOneSpace,
              kSanyoAcBitMark, kSanyoAcZeroSpace,
              kSanyoAcBitMark, kSanyoAcGap,
              data, nbytes, kSanyoAcFreq, false, repeat, kDutyDefault);
}
#endif  // SEND_SANYO_AC

#if DECODE_SANYO_AC
/// Decode the supplied SanyoAc message.
/// Status: BETA / Probably works.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1211
bool IRrecv::decodeSanyoAc(decode_results *results, uint16_t offset,
                           const uint16_t nbits, const bool strict) {
  if (strict && nbits != kSanyoAcBits)
    return false;

  // Header + Data + Footer
  if (!matchGeneric(results->rawbuf + offset, results->state,
                    results->rawlen - offset, nbits,
                    kSanyoAcHdrMark, kSanyoAcHdrSpace,
                    kSanyoAcBitMark, kSanyoAcOneSpace,
                    kSanyoAcBitMark, kSanyoAcZeroSpace,
                    kSanyoAcBitMark, kSanyoAcGap,
                    true, kUseDefTol, kMarkExcess, false)) return false;
  // Compliance
  if (strict)
    if (!IRSanyoAc::validChecksum(results->state, nbits / 8)) return false;

  // Success
  results->decode_type = decode_type_t::SANYO_AC;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // DECODE_SANYO_AC

/// Class constructor
/// @param[in] pin GPIO to be used when sending.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRSanyoAc::IRSanyoAc(const uint16_t pin, const bool inverted,
                         const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) { stateReset(); }

/// Reset the state of the remote to a known good state/sequence.
/// @see https://docs.google.com/spreadsheets/d/1dYfLsnYvpjV-SgO8pdinpfuBIpSzm8Q1R5SabrLeskw/edit?ts=5f0190a5#gid=1050142776&range=A2:B2
void IRSanyoAc::stateReset(void) {
  static const uint8_t kReset[kSanyoAcStateLength] = {
    0x6A, 0x6D, 0x51, 0x00, 0x10, 0x45, 0x00, 0x00, 0x33};
  memcpy(remote_state, kReset, kSanyoAcStateLength);
}

/// Set up hardware to be able to send a message.
void IRSanyoAc::begin(void) { _irsend.begin(); }

#if SEND_SANYO_AC
/// Send the current internal state as IR messages.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRSanyoAc::send(const uint16_t repeat) {
  _irsend.sendSanyoAc(getRaw(), kSanyoAcStateLength, repeat);
}
#endif  // SEND_SANYO_AC

/// Get a PTR to the internal state/code for this protocol with all integrity
///   checks passing.
/// @return PTR to a code for this protocol based on the current internal state.
uint8_t* IRSanyoAc::getRaw(void) {
  checksum();
  return remote_state;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] newState A valid code for this protocol.
void IRSanyoAc::setRaw(const uint8_t newState[]) {
  memcpy(remote_state, newState, kSanyoAcStateLength);
}

/// Calculate the checksum for a given state.
/// @param[in] state The array to calc the checksum of.
/// @param[in] length The length/size of the array.
/// @return The calculated checksum value.
uint8_t IRSanyoAc::calcChecksum(const uint8_t state[],
                                const uint16_t length) {
  return length ? sumNibbles(state, length - 1) : 0;
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length/size of the array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool IRSanyoAc::validChecksum(const uint8_t state[], const uint16_t length) {
  return length && state[length - 1] == IRSanyoAc::calcChecksum(state, length);
}

/// Calculate & set the checksum for the current internal state of the remote.
void IRSanyoAc::checksum(void) {
  // Stored the checksum value in the last byte.
  remote_state[kSanyoAcStateLength - 1] = calcChecksum(remote_state);
}


/// Set the requested power state of the A/C to on.
void IRSanyoAc::on(void) { setPower(true); }

/// Set the requested power state of the A/C to off.
void IRSanyoAc::off(void) { setPower(false); }

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRSanyoAc::setPower(const bool on) {
  setBits(&remote_state[kSanyoAcPowerByte], kSanyoAcPowerOffset,
          kSanyoAcPowerSize, on ? kSanyoAcPowerOn : kSanyoAcPowerOff);
}

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool IRSanyoAc::getPower(void) {
  return GETBITS8(remote_state[kSanyoAcPowerByte], kSanyoAcPowerOffset,
                  kSanyoAcPowerSize) == kSanyoAcPowerOn;
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t IRSanyoAc::getMode(void) {
  return GETBITS8(remote_state[kSanyoAcModeByte], kSanyoAcModeOffset,
                  kSanyoAcModeSize);
}

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
/// @note If we get an unexpected mode, default to AUTO.
void IRSanyoAc::setMode(const uint8_t mode) {
  switch (mode) {
    case kSanyoAcAuto:
    case kSanyoAcCool:
    case kSanyoAcDry:
    case kSanyoAcHeat:
      setBits(&remote_state[kSanyoAcModeByte], kSanyoAcModeOffset,
              kSanyoAcModeSize, mode);
      break;
    default: setMode(kSanyoAcAuto);
  }
}

/// Convert a stdAc::opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivilant of the enum.
uint8_t IRSanyoAc::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kCool: return kSanyoAcCool;
    case stdAc::opmode_t::kHeat: return kSanyoAcHeat;
    case stdAc::opmode_t::kDry:  return kSanyoAcDry;
    default:                     return kSanyoAcAuto;
  }
}

/// Convert a native mode into its stdAc equivilant.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivilant of the native setting.
stdAc::opmode_t IRSanyoAc::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kSanyoAcCool: return stdAc::opmode_t::kCool;
    case kSanyoAcHeat: return stdAc::opmode_t::kHeat;
    case kSanyoAcDry:  return stdAc::opmode_t::kDry;
    default:           return stdAc::opmode_t::kAuto;
  }
}

/// Set the temperature.
/// @param[in] degrees The temperature in degrees celsius.
void IRSanyoAc::setTemp(const uint8_t degrees) {
  uint8_t temp = std::max((uint8_t)kSanyoAcTempMin, degrees);
  temp = std::min((uint8_t)kSanyoAcTempMax, temp);
  setBits(&remote_state[kSanyoAcTempByte], kSanyoAcTempOffset, kSanyoAcTempSize,
          temp - kSanyoAcTempDelta);
}

/// Get the current temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t IRSanyoAc::getTemp(void) {
  return GETBITS8(remote_state[kSanyoAcTempByte], kSanyoAcTempOffset,
                  kSanyoAcTempSize) + kSanyoAcTempDelta;
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
void IRSanyoAc::setFan(const uint8_t speed) {
  setBits(&remote_state[kSanyoAcModeByte], kSanyoAcFanOffset, kSanyoAcFanSize,
          speed);
}

/// Get the current fan speed setting.
/// @return The current fan speed/mode.
uint8_t IRSanyoAc::getFan(void) {
  return GETBITS8(remote_state[kSanyoAcModeByte], kSanyoAcFanOffset,
                  kSanyoAcFanSize);
}

/// Convert a stdAc::fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivilant of the enum.
uint8_t IRSanyoAc::convertFan(const stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin:
    case stdAc::fanspeed_t::kLow:    return kSanyoAcFanLow;
    case stdAc::fanspeed_t::kMedium: return kSanyoAcFanMedium;
    case stdAc::fanspeed_t::kHigh:
    case stdAc::fanspeed_t::kMax:    return kSanyoAcFanHigh;
    default:                         return kSanyoAcFanAuto;
  }
}

/// Convert a native fan speed into its stdAc equivilant.
/// @param[in] spd The native setting to be converted.
/// @return The stdAc equivilant of the native setting.
stdAc::fanspeed_t IRSanyoAc::toCommonFanSpeed(const uint8_t spd) {
  switch (spd) {
    case kSanyoAcFanHigh:   return stdAc::fanspeed_t::kHigh;
    case kSanyoAcFanMedium: return stdAc::fanspeed_t::kMedium;
    case kSanyoAcFanLow:    return stdAc::fanspeed_t::kLow;
    default:                return stdAc::fanspeed_t::kAuto;
  }
}

/// Get the vertical swing setting of the A/C.
/// @return The current swing mode setting.
uint8_t IRSanyoAc::getSwingV(void) {
  return GETBITS8(remote_state[kSanyoAcPowerByte], kSanyoAcSwingVOffset,
                  kSanyoAcSwingVSize);
}

/// Set the vertical swing setting of the A/C.
/// @param[in] setting The value of the desired setting.
void IRSanyoAc::setSwingV(const uint8_t setting) {
  if (setting == kSanyoAcSwingVAuto ||
      (setting >= kSanyoAcSwingVLowest && setting <= kSanyoAcSwingVHighest))
    setBits(&remote_state[kSanyoAcPowerByte], kSanyoAcSwingVOffset,
            kSanyoAcSwingVSize, setting);

  else
    setSwingV(kSanyoAcSwingVAuto);
}

/// Convert a stdAc::swingv_t enum into it's native setting.
/// @param[in] position The enum to be converted.
/// @return The native equivilant of the enum.
uint8_t IRSanyoAc::convertSwingV(const stdAc::swingv_t position) {
  switch (position) {
    case stdAc::swingv_t::kHighest: return kSanyoAcSwingVHighest;
    case stdAc::swingv_t::kHigh:    return kSanyoAcSwingVHigh;
    case stdAc::swingv_t::kMiddle:  return kSanyoAcSwingVUpperMiddle;
    case stdAc::swingv_t::kLow:     return kSanyoAcSwingVLow;
    case stdAc::swingv_t::kLowest:  return kSanyoAcSwingVLowest;
    default:                        return kSanyoAcSwingVAuto;
  }
}

/// Convert a native vertical swing postion to it's common equivalent.
/// @param[in] setting A native position to convert.
/// @return The common vertical swing position.
stdAc::swingv_t IRSanyoAc::toCommonSwingV(const uint8_t setting) {
  switch (setting) {
    case kSanyoAcSwingVHighest:     return stdAc::swingv_t::kHighest;
    case kSanyoAcSwingVHigh:        return stdAc::swingv_t::kHigh;
    case kSanyoAcSwingVUpperMiddle:
    case kSanyoAcSwingVLowerMiddle: return stdAc::swingv_t::kMiddle;
    case kSanyoAcSwingVLow:         return stdAc::swingv_t::kLow;
    case kSanyoAcSwingVLowest:      return stdAc::swingv_t::kLowest;
    default:                        return stdAc::swingv_t::kAuto;
  }
}

/// Set the Sleep (Night Setback) setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRSanyoAc::setSleep(const bool on) {
  setBit(&remote_state[kSanyoAcSleepByte], kSanyoAcSleepBitOffset, on);
}

/// Get the Sleep (Night Setback) setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRSanyoAc::getSleep(void) {
  return GETBIT8(remote_state[kSanyoAcSleepByte], kSanyoAcSleepBitOffset);
}

/// Convert the current internal state into its stdAc::state_t equivilant.
/// @return The stdAc equivilant of the native settings.
stdAc::state_t IRSanyoAc::toCommon(void) {
  stdAc::state_t result;
  result.protocol = decode_type_t::SANYO_AC;
  result.model = -1;  // Not supported.
  result.power = getPower();
  result.mode = toCommonMode(getMode());
  result.celsius = true;
  result.degrees = getTemp();
  result.fanspeed = toCommonFanSpeed(getFan());
  result.sleep = getSleep() ? 0 : -1;
  result.swingv = toCommonSwingV(getSwingV());
  // Not supported.
  result.swingh = stdAc::swingh_t::kOff;
  result.turbo = false;
  result.econo = false;
  result.light = false;
  result.filter = false;
  result.quiet = false;
  result.clean = false;
  result.beep = false;
  result.clock = -1;
  return result;
}

/// Convert the current internal state into a human readable string.
/// @return A human readable string.
String IRSanyoAc::toString(void) {
  String result = "";
  result.reserve(90);
  result += addBoolToString(getPower(), kPowerStr, false);
  result += addModeToString(getMode(), kSanyoAcAuto, kSanyoAcCool,
                            kSanyoAcHeat, kSanyoAcDry, kSanyoAcAuto);
  result += addTempToString(getTemp());
  result += addFanToString(getFan(), kSanyoAcFanHigh, kSanyoAcFanLow,
                           kSanyoAcFanAuto, kSanyoAcFanAuto,
                           kSanyoAcFanMedium);
  result += addBoolToString(getSleep(), kSleepStr);
  result += addIntToString(getSwingV(), kSwingVStr);
  result += kSpaceLBraceStr;
  switch (getSwingV()) {
    case kSanyoAcSwingVHighest: result += kHighestStr; break;
    case kSanyoAcSwingVHigh:    result += kHighStr; break;
    case kSanyoAcSwingVUpperMiddle:
      result += kUpperStr;
      result += kMiddleStr;
      break;
      case kSanyoAcSwingVLowerMiddle:
        result += kLowerStr;
        result += kMiddleStr;
        break;
    case kSanyoAcSwingVLow:     result += kLowStr; break;
    case kSanyoAcSwingVLowest:  result += kLowestStr; break;
    case kSanyoAcSwingVAuto:    result += kAutoStr;   break;
    default:                    result += kUnknownStr;
  }
  result += ')';
  return result;
}
