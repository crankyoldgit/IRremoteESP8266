// Copyright 2018-2019 David Conran
//
// Code to emulate Hitachi protocol compatible devices.
// Should be compatible with:
// * Hitachi RAS-35THA6 remote
//

#include "ir_Hitachi.h"
#include <algorithm>
#include <cstring>
#ifndef ARDUINO
#include <string>
#endif
#include "IRrecv.h"
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "IRtext.h"
#include "IRutils.h"

// Constants
// Ref: https://github.com/crankyoldgit/IRremoteESP8266/issues/417
const uint16_t kHitachiAcHdrMark = 3300;
const uint16_t kHitachiAcHdrSpace = 1700;
const uint16_t kHitachiAc1HdrMark = 3400;
const uint16_t kHitachiAc1HdrSpace = 3400;
const uint16_t kHitachiAcBitMark = 400;
const uint16_t kHitachiAcOneSpace = 1250;
const uint16_t kHitachiAcZeroSpace = 500;
const uint32_t kHitachiAcMinGap = kDefaultMessageGap;  // Just a guess.
// Support for HitachiAc424 protocol
// Ref: https://github.com/crankyoldgit/IRremoteESP8266/issues/973
const uint16_t kHitachiAc424LdrMark = 29784;   // Leader
const uint16_t kHitachiAc424LdrSpace = 49290;  // Leader
const uint16_t kHitachiAc424HdrMark = 3416;    // Header
const uint16_t kHitachiAc424HdrSpace = 1604;   // Header
const uint16_t kHitachiAc424BitMark = 463;
const uint16_t kHitachiAc424OneSpace = 1208;
const uint16_t kHitachiAc424ZeroSpace = 372;

using irutils::addBoolToString;
using irutils::addIntToString;
using irutils::addLabeledString;
using irutils::addModeToString;
using irutils::addFanToString;
using irutils::addTempToString;
using irutils::setBit;
using irutils::setBits;

#if (SEND_HITACHI_AC || SEND_HITACHI_AC2)
// Send a Hitachi A/C message.
//
// Args:
//   data: An array of bytes containing the IR command.
//   nbytes: Nr. of bytes of data in the array. (>=kHitachiAcStateLength)
//   repeat: Nr. of times the message is to be repeated. (Default = 0).
//
// Status: ALPHA / Untested.
//
// Ref:
//   https://github.com/crankyoldgit/IRremoteESP8266/issues/417
void IRsend::sendHitachiAC(const unsigned char data[], const uint16_t nbytes,
                           const uint16_t repeat) {
  if (nbytes < kHitachiAcStateLength)
    return;  // Not enough bytes to send a proper message.
  sendGeneric(kHitachiAcHdrMark, kHitachiAcHdrSpace, kHitachiAcBitMark,
              kHitachiAcOneSpace, kHitachiAcBitMark, kHitachiAcZeroSpace,
              kHitachiAcBitMark, kHitachiAcMinGap, data, nbytes, 38, true,
              repeat, 50);
}
#endif  // (SEND_HITACHI_AC || SEND_HITACHI_AC2)

#if SEND_HITACHI_AC1
// Send a Hitachi A/C 13-byte message.
//
// For devices:
//  Hitachi A/C Series VI (Circa 2007) / Remote: LT0541-HTA
//
// Args:
//   data: An array of bytes containing the IR command.
//   nbytes: Nr. of bytes of data in the array. (>=kHitachiAc1StateLength)
//   repeat: Nr. of times the message is to be repeated. (Default = 0).
//
// Status: BETA / Appears to work.
//
// Ref:
//   https://github.com/crankyoldgit/IRremoteESP8266/issues/453
//   Basically the same as sendHitatchiAC() except different size and header.
void IRsend::sendHitachiAC1(const unsigned char data[], const uint16_t nbytes,
                            const uint16_t repeat) {
  if (nbytes < kHitachiAc1StateLength)
    return;  // Not enough bytes to send a proper message.
  sendGeneric(kHitachiAc1HdrMark, kHitachiAc1HdrSpace, kHitachiAcBitMark,
              kHitachiAcOneSpace, kHitachiAcBitMark, kHitachiAcZeroSpace,
              kHitachiAcBitMark, kHitachiAcMinGap, data, nbytes, kHitachiAcFreq,
              true, repeat, kDutyDefault);
}
#endif  // SEND_HITACHI_AC1

#if SEND_HITACHI_AC2
// Send a Hitachi A/C 53-byte message.
//
// For devices:
//  Hitachi A/C Series VI (Circa 2007) / Remote: LT0541-HTA
//
// Args:
//   data: An array of bytes containing the IR command.
//   nbytes: Nr. of bytes of data in the array. (>=kHitachiAc2StateLength)
//   repeat: Nr. of times the message is to be repeated. (Default = 0).
//
// Status: BETA / Appears to work.
//
// Ref:
//   https://github.com/crankyoldgit/IRremoteESP8266/issues/417
//   Basically the same as sendHitatchiAC() except different size.
void IRsend::sendHitachiAC2(const unsigned char data[], const uint16_t nbytes,
                            const uint16_t repeat) {
  if (nbytes < kHitachiAc2StateLength)
    return;  // Not enough bytes to send a proper message.
  sendHitachiAC(data, nbytes, repeat);
}
#endif  // SEND_HITACHI_AC2

// Class for handling the remote control on a Hitachi 28 byte A/C message.
// Inspired by:
// https://github.com/ToniA/arduino-heatpumpir/blob/master/HitachiHeatpumpIR.cpp

IRHitachiAc::IRHitachiAc(const uint16_t pin, const bool inverted,
                         const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) { stateReset(); }

void IRHitachiAc::stateReset(void) {
  remote_state[0] = 0x80;
  remote_state[1] = 0x08;
  remote_state[2] = 0x0C;
  remote_state[3] = 0x02;
  remote_state[4] = 0xFD;
  remote_state[5] = 0x80;
  remote_state[6] = 0x7F;
  remote_state[7] = 0x88;
  remote_state[8] = 0x48;
  remote_state[9] = 0x10;
  for (uint8_t i = 10; i < kHitachiAcStateLength; i++) remote_state[i] = 0x00;
  remote_state[14] = 0x60;
  remote_state[15] = 0x60;
  remote_state[24] = 0x80;
  setTemp(23);
}

void IRHitachiAc::begin(void) { _irsend.begin(); }

uint8_t IRHitachiAc::calcChecksum(const uint8_t state[],
                                  const uint16_t length) {
  uint8_t sum = 62;
  for (uint16_t i = 0; i < length - 1; i++) sum -= reverseBits(state[i], 8);
  return reverseBits(sum, 8);
}

void IRHitachiAc::checksum(const uint16_t length) {
  remote_state[length - 1] = calcChecksum(remote_state, length);
}

bool IRHitachiAc::validChecksum(const uint8_t state[], const uint16_t length) {
  if (length < 2) return true;  // Assume true for lengths that are too short.
  return (state[length - 1] == calcChecksum(state, length));
}

uint8_t *IRHitachiAc::getRaw(void) {
  checksum();
  return remote_state;
}

void IRHitachiAc::setRaw(const uint8_t new_code[], const uint16_t length) {
  memcpy(remote_state, new_code, std::min(length, kHitachiAcStateLength));
}

#if SEND_HITACHI_AC
void IRHitachiAc::send(const uint16_t repeat) {
  _irsend.sendHitachiAC(getRaw(), kHitachiAcStateLength, repeat);
}
#endif  // SEND_HITACHI_AC

bool IRHitachiAc::getPower(void) {
  return GETBIT8(remote_state[17], kHitachiAcPowerOffset);
}

void IRHitachiAc::setPower(const bool on) {
  setBit(&remote_state[17], kHitachiAcPowerOffset, on);
}

void IRHitachiAc::on(void) { setPower(true); }

void IRHitachiAc::off(void) { setPower(false); }

uint8_t IRHitachiAc::getMode(void) { return reverseBits(remote_state[10], 8); }

void IRHitachiAc::setMode(const uint8_t mode) {
  uint8_t newmode = mode;
  switch (mode) {
    // Fan mode sets a special temp.
    case kHitachiAcFan: setTemp(64); break;
    case kHitachiAcAuto:
    case kHitachiAcHeat:
    case kHitachiAcCool:
    case kHitachiAcDry: break;
    default: newmode = kHitachiAcAuto;
  }
  remote_state[10] = reverseBits(newmode, 8);
  if (mode != kHitachiAcFan) setTemp(_previoustemp);
  setFan(getFan());  // Reset the fan speed after the mode change.
}

uint8_t IRHitachiAc::getTemp(void) {
  return reverseBits(remote_state[11], 8) >> 1;
}

void IRHitachiAc::setTemp(const uint8_t celsius) {
  uint8_t temp;
  if (celsius != 64) _previoustemp = celsius;
  switch (celsius) {
    case 64:
      temp = celsius;
      break;
    default:
      temp = std::min(celsius, kHitachiAcMaxTemp);
      temp = std::max(temp, kHitachiAcMinTemp);
  }
  remote_state[11] = reverseBits(temp << 1, 8);
  if (temp == kHitachiAcMinTemp)
    remote_state[9] = 0x90;
  else
    remote_state[9] = 0x10;
}

uint8_t IRHitachiAc::getFan(void) { return reverseBits(remote_state[13], 8); }

void IRHitachiAc::setFan(const uint8_t speed) {
  uint8_t fanmin = kHitachiAcFanAuto;
  uint8_t fanmax = kHitachiAcFanHigh;
  switch (getMode()) {
    case kHitachiAcDry:  // Only 2 x low speeds in Dry mode.
      fanmin = kHitachiAcFanLow;
      fanmax = kHitachiAcFanLow + 1;
      break;
    case kHitachiAcFan:
      fanmin = kHitachiAcFanLow;  // No Auto in Fan mode.
      break;
  }
  uint8_t newspeed = std::max(speed, fanmin);
  newspeed = std::min(newspeed, fanmax);
  remote_state[13] = reverseBits(newspeed, 8);
}

bool IRHitachiAc::getSwingVertical(void) {
  return GETBIT8(remote_state[14], kHitachiAcSwingOffset);
}

void IRHitachiAc::setSwingVertical(const bool on) {
  setBit(&remote_state[14], kHitachiAcSwingOffset, on);
}

bool IRHitachiAc::getSwingHorizontal(void) {
  return GETBIT8(remote_state[15], kHitachiAcSwingOffset);
}

void IRHitachiAc::setSwingHorizontal(const bool on) {
  setBit(&remote_state[15], kHitachiAcSwingOffset, on);
}

// Convert a standard A/C mode into its native mode.
uint8_t IRHitachiAc::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kCool: return kHitachiAcCool;
    case stdAc::opmode_t::kHeat: return kHitachiAcHeat;
    case stdAc::opmode_t::kDry:  return kHitachiAcDry;
    case stdAc::opmode_t::kFan:  return kHitachiAcFan;
    default:                     return kHitachiAcAuto;
  }
}

// Convert a standard A/C Fan speed into its native fan speed.
uint8_t IRHitachiAc::convertFan(const stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin:
    case stdAc::fanspeed_t::kLow:    return kHitachiAcFanLow;
    case stdAc::fanspeed_t::kMedium: return kHitachiAcFanLow + 1;
    case stdAc::fanspeed_t::kHigh:   return kHitachiAcFanHigh - 1;
    case stdAc::fanspeed_t::kMax:    return kHitachiAcFanHigh;
    default:                         return kHitachiAcFanAuto;
  }
}

// Convert a native mode to it's common equivalent.
stdAc::opmode_t IRHitachiAc::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kHitachiAcCool: return stdAc::opmode_t::kCool;
    case kHitachiAcHeat: return stdAc::opmode_t::kHeat;
    case kHitachiAcDry:  return stdAc::opmode_t::kDry;
    case kHitachiAcFan:  return stdAc::opmode_t::kFan;
    default:             return stdAc::opmode_t::kAuto;
  }
}

// Convert a native fan speed to it's common equivalent.
stdAc::fanspeed_t IRHitachiAc::toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kHitachiAcFanHigh:     return stdAc::fanspeed_t::kMax;
    case kHitachiAcFanHigh - 1: return stdAc::fanspeed_t::kHigh;
    case kHitachiAcFanLow + 1:  return stdAc::fanspeed_t::kMedium;
    case kHitachiAcFanLow:      return stdAc::fanspeed_t::kLow;
    default:                    return stdAc::fanspeed_t::kAuto;
  }
}

// Convert the A/C state to it's common equivalent.
stdAc::state_t IRHitachiAc::toCommon(void) {
  stdAc::state_t result;
  result.protocol = decode_type_t::HITACHI_AC;
  result.model = -1;  // No models used.
  result.power = this->getPower();
  result.mode = this->toCommonMode(this->getMode());
  result.celsius = true;
  result.degrees = this->getTemp();
  result.fanspeed = this->toCommonFanSpeed(this->getFan());
  result.swingv = this->getSwingVertical() ? stdAc::swingv_t::kAuto :
                                             stdAc::swingv_t::kOff;
  result.swingh = this->getSwingHorizontal() ? stdAc::swingh_t::kAuto :
                                               stdAc::swingh_t::kOff;
  // Not supported.
  result.quiet = false;
  result.turbo = false;
  result.clean = false;
  result.econo = false;
  result.filter = false;
  result.light = false;
  result.beep = false;
  result.sleep = -1;
  result.clock = -1;
  return result;
}

// Convert the internal state into a human readable string.
String IRHitachiAc::toString(void) {
  String result = "";
  result.reserve(110);  // Reserve some heap for the string to reduce fragging.
  result += addBoolToString(getPower(), kPowerStr, false);
  result += addModeToString(getMode(), kHitachiAcAuto, kHitachiAcCool,
                            kHitachiAcHeat, kHitachiAcDry, kHitachiAcFan);
  result += addTempToString(getTemp());
  result += addFanToString(getFan(), kHitachiAcFanHigh, kHitachiAcFanLow,
                           kHitachiAcFanAuto, kHitachiAcFanAuto,
                           kHitachiAcFanMed);
  result += addBoolToString(getSwingVertical(), kSwingVStr);
  result += addBoolToString(getSwingHorizontal(), kSwingHStr);
  return result;
}

#if (DECODE_HITACHI_AC || DECODE_HITACHI_AC1 || DECODE_HITACHI_AC2)
// Decode the supplied Hitachi A/C message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect.
//            Typically kHitachiAcBits, kHitachiAc1Bits, kHitachiAc2Bits
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: ALPHA / Untested.
//
// Supported devices:
//  Hitachi A/C Series VI (Circa 2007) / Remote: LT0541-HTA
//
// Ref:
//   https://github.com/crankyoldgit/IRremoteESP8266/issues/417
//   https://github.com/crankyoldgit/IRremoteESP8266/issues/453
bool IRrecv::decodeHitachiAC(decode_results *results, const uint16_t nbits,
                             const bool strict) {
  const uint8_t k_tolerance = _tolerance + 5;
  if (results->rawlen < 2 * nbits + kHeader + kFooter - 1)
    return false;  // Can't possibly be a valid HitachiAC message.
  if (strict) {
    switch (nbits) {
      case kHitachiAcBits:
      case kHitachiAc1Bits:
      case kHitachiAc2Bits:
        break;  // Okay to continue.
      default:
        return false;  // Not strictly a Hitachi message.
    }
  }
  uint16_t offset = kStartOffset;
  uint16_t hmark;
  uint32_t hspace;
  if (nbits == kHitachiAc1Bits) {
    hmark = kHitachiAc1HdrMark;
    hspace = kHitachiAc1HdrSpace;
  } else {
    hmark = kHitachiAcHdrMark;
    hspace = kHitachiAcHdrSpace;
  }
  // Match Header + Data + Footer
  if (!matchGeneric(results->rawbuf + offset, results->state,
                    results->rawlen - offset, nbits,
                    hmark, hspace,
                    kHitachiAcBitMark, kHitachiAcOneSpace,
                    kHitachiAcBitMark, kHitachiAcZeroSpace,
                    kHitachiAcBitMark, kHitachiAcMinGap, true,
                    k_tolerance)) return false;

  // Compliance
  if (strict) {
    if (nbits / 8 == kHitachiAcStateLength &&
        !IRHitachiAc::validChecksum(results->state, kHitachiAcStateLength))
      return false;
  }

  // Success
  switch (nbits) {
    case kHitachiAc1Bits:
      results->decode_type = HITACHI_AC1;
      break;
    case kHitachiAc2Bits:
      results->decode_type = HITACHI_AC2;
      break;
    case kHitachiAcBits:
    default:
      results->decode_type = HITACHI_AC;
  }
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // (DECODE_HITACHI_AC || DECODE_HITACHI_AC1 || DECODE_HITACHI_AC2)

#if SEND_HITACHI_AC424
// Send HITACHI_AC424 messages
//
// Note: This protocol is almost exactly the same as HitachiAC2 except this
//       variant has a leader section as well, and subtle timing differences.
//       It is also in LSBF order (per byte), rather than MSBF order.
//
// Args:
//   data: An array of bytes containing the IR command.
//         It is assumed to be in LSBF order for this code.
//   nbytes: Nr. of bytes of data in the array. (>=kHitachiAc424StateLength)
//   repeat: Nr. of times the message is to be repeated.
//
// Status: STABLE / Reported as working.
void IRsend::sendHitachiAc424(const uint8_t data[], const uint16_t nbytes,
                              const uint16_t repeat) {
  enableIROut(kHitachiAcFreq);
  for (uint16_t r = 0; r <= repeat; r++) {
    // Leader
    mark(kHitachiAc424LdrMark);
    space(kHitachiAc424LdrSpace);
    // Header + Data + Footer
    sendGeneric(kHitachiAc424HdrMark, kHitachiAc424HdrSpace,
                kHitachiAc424BitMark, kHitachiAc424OneSpace,
                kHitachiAc424BitMark, kHitachiAc424ZeroSpace,
                kHitachiAc424BitMark, kHitachiAcMinGap,
                data, nbytes,  // Bytes
                kHitachiAcFreq, false, kNoRepeat, kDutyDefault);
  }
}
#endif  // SEND_HITACHI_AC424

#if DECODE_HITACHI_AC424
// Decode the supplied Hitachi 424 bit A/C message.
//
// Note: This protocol is almost exactly the same as HitachiAC2 except this
//       variant has a leader section as well, and subtle timing differences.
//       It is also in LSBF order (per byte), rather than MSBF order.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect. Typically kHitachiAc424Bits.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: STABLE / Reported as working.
//
// Supported devices:
//  Hitachi Shirokumakun / AC Model: RAS-AJ25H / AC Remote Model: RAR-8P2
//  Manual (Japanese):
//    https://kadenfan.hitachi.co.jp/support/raj/item/docs/ras_aj22h_a_tori.pdf
//
// Ref:
//   https://github.com/crankyoldgit/IRremoteESP8266/issues/973
bool IRrecv::decodeHitachiAc424(decode_results *results, const uint16_t nbits,
                                const bool strict) {
  if (results->rawlen < 2 * nbits + kHeader + kHeader + kFooter - 1)
    return false;  // Too short a message to match.
  if (strict && nbits != kHitachiAc424Bits)
    return false;

  uint16_t offset = kStartOffset;
  uint16_t used;

  // Leader
  if (!matchMark(results->rawbuf[offset++], kHitachiAc424LdrMark))
    return false;
  if (!matchSpace(results->rawbuf[offset++], kHitachiAc424LdrSpace))
    return false;

  // Header + Data + Footer
  used = matchGeneric(results->rawbuf + offset, results->state,
                      results->rawlen - offset, nbits,
                      kHitachiAc424HdrMark, kHitachiAc424HdrSpace,
                      kHitachiAc424BitMark, kHitachiAc424OneSpace,
                      kHitachiAc424BitMark, kHitachiAc424ZeroSpace,
                      kHitachiAc424BitMark, kHitachiAcMinGap, true,
                      kUseDefTol, 0, false);
  if (used == 0) return false;  // We failed to find any data.

  // Success
  results->decode_type = decode_type_t::HITACHI_AC424;
  results->bits = nbits;
  return true;
}
#endif  // DECODE_HITACHI_AC424

// Class for handling the remote control on a Hitachi_AC424 53 byte A/C message
IRHitachiAc424::IRHitachiAc424(const uint16_t pin, const bool inverted,
                         const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) { stateReset(); }

// Reset to auto fan, cooling, 23° Celcius
void IRHitachiAc424::stateReset(void) {
  for (uint8_t i = 0; i < kHitachiAc424StateLength; i++)
    remote_state[i] = 0x00;

  remote_state[0]  = 0x01;
  remote_state[1]  = 0x10;
  // The following bytes will also have a calculated bit inverted pair
  remote_state[3]  = 0x40;
  remote_state[5]  = 0xFF;
  remote_state[7]  = 0xCC;
  remote_state[9]  = 0x92;
  remote_state[11] = 0x13;  // Button Action
  remote_state[13] = 0x5C;  // 23C
  remote_state[25] = 0x53;  // Fan speed, mode
  remote_state[27] = 0xF1;  // Power
  remote_state[33] = 0x80;
  remote_state[35] = 0x03;
  remote_state[37] = 0x01;
  remote_state[39] = 0x88;
  remote_state[45] = 0xFF;
  remote_state[47] = 0xFF;
  remote_state[49] = 0xFF;
  remote_state[51] = 0xFF;
  setInvertedStates();
}

void IRHitachiAc424::setInvertedStates(void) {
  for (uint8_t i = 3; i < kHitachiAc424StateLength; i+=2)
    remote_state[i+1] = ~remote_state[i];
}

void IRHitachiAc424::begin(void) { _irsend.begin(); }

uint8_t *IRHitachiAc424::getRaw(void) {
  setInvertedStates();
  return remote_state;
}

void IRHitachiAc424::setRaw(const uint8_t new_code[], const uint16_t length) {
  memcpy(remote_state, new_code, std::min(length, kHitachiAc424StateLength));
}

#if SEND_HITACHI_AC424
void IRHitachiAc424::send(const uint16_t repeat) {
  _irsend.sendHitachiAc424(getRaw(), kHitachiAc424StateLength, repeat);
}
#endif  // SEND_HITACHI_AC424

bool IRHitachiAc424::getPower(void) {
  return GETBIT8(remote_state[kHitachiAc424BytePower],
    kHitachiAc424PowerOffset);
}

void IRHitachiAc424::setPower(const bool on) {
  setBit(&remote_state[kHitachiAc424BytePower], kHitachiAc424PowerOffset, on);
}

void IRHitachiAc424::on(void) { setPower(true); }

void IRHitachiAc424::off(void) { setPower(false); }

uint8_t IRHitachiAc424::getMode(void) {
  return GETBITS8(remote_state[25], kLowNibble, kNibbleSize);
}

void IRHitachiAc424::setMode(const uint8_t mode) {
  uint8_t newMode = mode;
  switch (mode) {
    // Fan mode sets a special temp.
    case kHitachiAc424Fan: setTemp(27, false); break;
    case kHitachiAc424Heat:
    case kHitachiAc424Cool:
    case kHitachiAc424Dry: break;
    default: newMode = kHitachiAc424Cool;
  }
  setBits(&remote_state[25], kLowNibble, kNibbleSize, newMode);
  if (newMode != kHitachiAc424Fan) setTemp(_previoustemp);
  setFan(getFan());  // Reset the fan speed after the mode change.
}

uint8_t IRHitachiAc424::getTemp(void) {
  return remote_state[13] >> 2;
}

void IRHitachiAc424::setTemp(const uint8_t celsius, bool setPrevious = true) {
  uint8_t temp;
  if (setPrevious) _previoustemp = celsius;
  temp = std::min(celsius, kHitachiAc424MaxTemp);
  temp = std::max(temp, kHitachiAc424MinTemp);
  remote_state[13] = temp << 2;
}
