// Copyright 2018 David Conran
//
// Code to emulate Whirlpool protocol compatible devices.
// Should be compatible with:
// * SPIS409L, SPIS412L, SPIW409L, SPIW412L, SPIW418L
//

#include "ir_Whirlpool.h"
#include <algorithm>
#ifndef ARDUINO
#include <string>
#endif
#include "IRrecv.h"
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "IRutils.h"

//    WW      WW HH   HH IIIII RRRRRR  LL      PPPPPP   OOOOO   OOOOO  LL
//    WW      WW HH   HH  III  RR   RR LL      PP   PP OO   OO OO   OO LL
//    WW   W  WW HHHHHHH  III  RRRRRR  LL      PPPPPP  OO   OO OO   OO LL
//     WW WWW WW HH   HH  III  RR  RR  LL      PP      OO   OO OO   OO LL
//      WW   WW  HH   HH IIIII RR   RR LLLLLLL PP       OOOO0   OOOO0  LLLLLLL

// Constants
// Ref: https://github.com/markszabo/IRremoteESP8266/issues/509
const uint16_t kWhirlpoolAcHdrMark = 8950;
const uint16_t kWhirlpoolAcHdrSpace = 4484;
const uint16_t kWhirlpoolAcBitMark = 597;
const uint16_t kWhirlpoolAcOneSpace = 1649;
const uint16_t kWhirlpoolAcZeroSpace = 533;
const uint16_t kWhirlpoolAcGap = 7920;
const uint32_t kWhirlpoolAcMinGap = 100000;  // Completely made up value.
const uint8_t kWhirlpoolAcSections = 3;

#if SEND_WHIRLPOOL_AC
// Send a Whirlpool A/C message.
//
// Args:
//   data: An array of bytes containing the IR command.
//   nbytes: Nr. of bytes of data in the array. (>=kWhirlpoolAcStateLength)
//   repeat: Nr. of times the message is to be repeated. (Default = 0).
//
// Status: ALPHA / Untested.
//
// Ref:
//   https://github.com/markszabo/IRremoteESP8266/issues/509
void IRsend::sendWhirlpoolAC(unsigned char data[], uint16_t nbytes,
                             uint16_t repeat) {
  if (nbytes < kWhirlpoolAcStateLength)
    return;  // Not enough bytes to send a proper message.
  for (uint16_t r = 0; r <= repeat; r++) {
    // Section 1
    sendGeneric(kWhirlpoolAcHdrMark, kWhirlpoolAcHdrSpace, kWhirlpoolAcBitMark,
                kWhirlpoolAcOneSpace, kWhirlpoolAcBitMark,
                kWhirlpoolAcZeroSpace, kWhirlpoolAcBitMark, kWhirlpoolAcGap,
                data, 6,  // 6 bytes == 48 bits
                38000,    // Complete guess of the modulation frequency.
                false, 0, 50);
    // Section 2
    sendGeneric(0, 0, kWhirlpoolAcBitMark, kWhirlpoolAcOneSpace,
                kWhirlpoolAcBitMark, kWhirlpoolAcZeroSpace, kWhirlpoolAcBitMark,
                kWhirlpoolAcGap, data + 6, 8,  // 8 bytes == 64 bits
                38000,  // Complete guess of the modulation frequency.
                false, 0, 50);
    // Section 3
    sendGeneric(0, 0, kWhirlpoolAcBitMark, kWhirlpoolAcOneSpace,
                kWhirlpoolAcBitMark, kWhirlpoolAcZeroSpace, kWhirlpoolAcBitMark,
                kWhirlpoolAcMinGap, data + 14, 7,  // 7 bytes == 56 bits
                38000,  // Complete guess of the modulation frequency.
                false, 0, 50);
  }
}
#endif  // SEND_WHIRLPOOL_AC

IRWhirlpoolAc::IRWhirlpoolAc(uint16_t pin) : _irsend(pin) { stateReset(); }

void IRWhirlpoolAc::stateReset() {
  for (uint8_t i = 2; i < kWhirlpoolAcStateLength; i++) remote_state[i] = 0x0;
  remote_state[0] = 0x83;
  remote_state[1] = 0x06;
}

void IRWhirlpoolAc::begin() { _irsend.begin(); }

bool IRWhirlpoolAc::validChecksum(uint8_t state[], const uint16_t length) {
  if (length > kWhirlpoolAcChecksumByte1 &&
      state[kWhirlpoolAcChecksumByte1] !=
          xorBytes(state + 2, kWhirlpoolAcChecksumByte1 - 1 - 2)) {
    DPRINTLN("DEBUG: First Whirlpool AC checksum failed.");
    return false;
  }
  if (length > kWhirlpoolAcChecksumByte2 &&
      state[kWhirlpoolAcChecksumByte2] !=
          xorBytes(state + kWhirlpoolAcChecksumByte1 + 1,
                   kWhirlpoolAcChecksumByte2 - kWhirlpoolAcChecksumByte1 - 1)) {
    DPRINTLN("DEBUG: Second Whirlpool AC checksum failed.");
    return false;
  }
  // State is too short to have a checksum or everything checked out.
  return true;
}

// Update the checksum for the internal state.
void IRWhirlpoolAc::checksum(uint16_t length) {
  if (length >= kWhirlpoolAcChecksumByte1)
    remote_state[kWhirlpoolAcChecksumByte1] =
        xorBytes(remote_state + 2, kWhirlpoolAcChecksumByte1 - 1 - 2);
  if (length >= kWhirlpoolAcChecksumByte2)
    remote_state[kWhirlpoolAcChecksumByte2] =
        xorBytes(remote_state + kWhirlpoolAcChecksumByte1 + 1,
                 kWhirlpoolAcChecksumByte2 - kWhirlpoolAcChecksumByte1 - 1);
}

#if SEND_WHIRLPOOL_AC
void IRWhirlpoolAc::send(const bool calcchecksum) {
  if (calcchecksum) checksum();
  _irsend.sendWhirlpoolAC(remote_state);
}
#endif  // SEND_WHIRLPOOL_AC

uint8_t *IRWhirlpoolAc::getRaw(const bool calcchecksum) {
  if (calcchecksum) checksum();
  return remote_state;
}

void IRWhirlpoolAc::setRaw(const uint8_t new_code[], const uint16_t length) {
  for (uint8_t i = 0; i < length && i < kWhirlpoolAcStateLength; i++)
    remote_state[i] = new_code[i];
}

// Set the temp. in deg C
void IRWhirlpoolAc::setTemp(const uint8_t temp) {
  uint8_t newtemp = std::max(kWhirlpoolAcMinTemp, temp);
  newtemp = std::min(kWhirlpoolAcMaxTemp, newtemp);
  remote_state[3] = (remote_state[3] & 0b00001111) |
                     ((newtemp - kWhirlpoolAcMinTemp) << 4);
}

// Return the set temp. in deg C
uint8_t IRWhirlpoolAc::getTemp() {
  return ((remote_state[3] & kWhirlpoolAcTempMask) >> 4) + kWhirlpoolAcMinTemp;
}

void IRWhirlpoolAc::setMode(const uint8_t mode) {
  switch (mode) {
    case kWhirlpoolAcHeat:
    case kWhirlpoolAcAuto:
    case kWhirlpoolAcCool:
    case kWhirlpoolAcDry:
    case kWhirlpoolAcFan:
      remote_state[3] &= kWhirlpoolAcModeMask;
      remote_state[3] |= mode;
      break;
  }
}

uint8_t IRWhirlpoolAc::getMode() {
  return remote_state[3] & ~kWhirlpoolAcModeMask;
}

void IRWhirlpoolAc::setFan(const uint8_t speed) {
  switch (speed) {
    case kWhirlpoolAcFanAuto:
    case kWhirlpoolAcFanLow:
    case kWhirlpoolAcFanMedium:
    case kWhirlpoolAcFanHigh:
      remote_state[2] = (remote_state[2] & kWhirlpoolAcFanMask) | (speed << 4);
      break;
  }
}

uint8_t IRWhirlpoolAc::getFan() {
  return (remote_state[2] & ~kWhirlpoolAcFanMask) >> 4;
}

void IRWhirlpoolAc::setSwing(const bool on) {
  if (on) {
    remote_state[2] |= 0b00000001;
    remote_state[11] |= 0b00000010;
  } else {
    remote_state[2] &= 0b11111110;
    remote_state[11] &= 0b11111101;
  }
}

bool IRWhirlpoolAc::getSwing() {
  return (remote_state[2] & 0b00000001) && (remote_state[11] & 0b00000010);
}

void IRWhirlpoolAc::setLight(const bool on) {
  if (on)
    remote_state[6] &= ~kWhirlpoolAcLightMask;
  else
    remote_state[6] |= kWhirlpoolAcLightMask;
}

bool IRWhirlpoolAc::getLight() {
  return !(remote_state[6] & kWhirlpoolAcLightMask);
}

void IRWhirlpoolAc::setClock(const uint16_t minspastmidnight) {
  remote_state[6] &= ~kWhirlpoolAcClockHourMask;
  remote_state[6] |= (minspastmidnight / 60) % 24;  // Hours
  remote_state[7] = minspastmidnight % 60;  // Minutes
}

uint16_t IRWhirlpoolAc::getClock() {
  return (remote_state[6] & kWhirlpoolAcClockHourMask) * 60 + remote_state[7];
}

#ifdef ARDUINO
String IRWhirlpoolAc::timeToString(const uint16_t minspastmidnight) {
  String result = "";
#else
std::string IRWhirlpoolAc::timeToString(const uint16_t minspastmidnight) {
  std::string result = "";
#endif  // ARDUINO
  uint8_t hours = minspastmidnight / 60;
  if (hours < 10) result += "0";
  result += uint64ToString(hours);
  result += ":";
  uint8_t mins = minspastmidnight % 60;
  if (mins < 10) result += "0";
  result += uint64ToString(mins);
  return result;
}

// Convert the internal state into a human readable string.
#ifdef ARDUINO
String IRWhirlpoolAc::toString() {
  String result = "";
#else
std::string IRWhirlpoolAc::toString() {
  std::string result = "";
#endif  // ARDUINO
  result += "Mode: " + uint64ToString(getMode());
  switch (getMode()) {
    case kWhirlpoolAcHeat:
      result += " (HEAT)";
      break;
    case kWhirlpoolAcAuto:
      result += " (AUTO)";
      break;
    case kWhirlpoolAcCool:
      result += " (COOL)";
      break;
    case kWhirlpoolAcDry:
      result += " (DRY)";
      break;
    case kWhirlpoolAcFan:
      result += " (FAN)";
      break;
    default:
      result += " (UNKNOWN)";
  }
  result += ", Temp: " + uint64ToString(getTemp()) + "C";
  result += ", Fan: " + uint64ToString(getFan());
  switch (getFan()) {
    case kWhirlpoolAcFanAuto:
      result += " (AUTO)";
      break;
    case kWhirlpoolAcFanHigh:
      result += " (HIGH)";
      break;
    case kWhirlpoolAcFanMedium:
      result += " (MEDIUM)";
      break;
    case kWhirlpoolAcFanLow:
      result += " (LOW)";
      break;
    default:
      result += " (UNKNOWN)";
      break;
  }
  result += ", Swing: ";
  if (getSwing())
    result += "On";
  else
    result += "Off";
  result += ", Light: ";
  if (getLight())
    result += "On";
  else
    result += "Off";
  result += ", Time: ";
  result += timeToString(getClock());
  return result;
}

#if DECODE_WHIRLPOOL_AC
// Decode the supplied Whirlpool A/C message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect. Typically kWhirlpoolAcBits
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: ALPHA / Untested.
//
//
// Ref:
//   https://github.com/markszabo/IRremoteESP8266/issues/509
bool IRrecv::decodeWhirlpoolAC(decode_results *results, uint16_t nbits,
                               bool strict) {
  if (results->rawlen < 2 * nbits + 4 + kHeader + kFooter - 1)
    return false;  // Can't possibly be a valid Whirlpool A/C message.
  if (strict) {
    if (nbits != kWhirlpoolAcBits) return false;
  }

  uint16_t offset = kStartOffset;
  uint16_t dataBitsSoFar = 0;
  uint16_t i = 0;
  match_result_t data_result;
  uint8_t sectionSize[kWhirlpoolAcSections] = {6, 8, 7};

  // Header
  if (!matchMark(results->rawbuf[offset++], kWhirlpoolAcHdrMark)) return false;
  if (!matchSpace(results->rawbuf[offset++], kWhirlpoolAcHdrSpace))
    return false;

  // Data Section
  // Keep reading bytes until we either run out of section or state to fill.
  for (uint8_t section = 0, pos = 0; section < kWhirlpoolAcSections;
       section++) {
    pos += sectionSize[section];
    for (; offset <= results->rawlen - 16 && i < pos;
         i++, dataBitsSoFar += 8, offset += data_result.used) {
      data_result =
          matchData(&(results->rawbuf[offset]), 8, kWhirlpoolAcBitMark,
                    kWhirlpoolAcOneSpace, kWhirlpoolAcBitMark,
                    kWhirlpoolAcZeroSpace, kTolerance, kMarkExcess, false);
      if (data_result.success == false) break;  // Fail
      // Data is in LSB order. We need to reverse it.
      results->state[i] = (uint8_t)data_result.data;
    }
    // Section Footer
    if (!matchMark(results->rawbuf[offset++], kWhirlpoolAcBitMark))
      return false;
    if (section < kWhirlpoolAcSections - 1) {  // Inter-section gaps.
      if (!matchSpace(results->rawbuf[offset++], kWhirlpoolAcGap)) return false;
    } else {  // Last section / End of message gap.
      if (offset <= results->rawlen &&
          !matchAtLeast(results->rawbuf[offset++], kWhirlpoolAcGap))
        return false;
    }
  }

  // Compliance
  if (strict) {
    // Re-check we got the correct size/length due to the way we read the data.
    if (dataBitsSoFar != kWhirlpoolAcBits) return false;
    if (!IRWhirlpoolAc::validChecksum(results->state, dataBitsSoFar / 8))
      return false;
  }

  // Success
  results->decode_type = WHIRLPOOL_AC;
  results->bits = dataBitsSoFar;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // WHIRLPOOL_AC
