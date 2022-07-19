// Copyright 2022 David Conran
/// @file
/// @brief Support for the Bosch A/C / heatpump protocol
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1787

// Supports:
//   Brand: Bosch,  Model: CL3000i-Set 26 E A/C
//   Brand: Bosch,  Model: RG10A(G2S)BGEF remote

#include <algorithm>
/*#ifndef ARDUINO
#include <string>
#endif*/
#include <cstring>
#include "ir_Bosch.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "IRtext.h"
#include "IRutils.h"

// Constants
const uint16_t kBoschHdrMark = 4366;
const uint16_t kBoschBitMark = 502;
const uint16_t kBoschHdrSpace = 4415;
const uint16_t kBoschOneSpace = 1645;
const uint16_t kBoschZeroSpace = 571;
const uint16_t kBoschFooterSpace = 5235;
const uint16_t kBoschFreq = 38000;  // Hz. (Guessing the most common frequency.)
const uint16_t kBosch144NrOfSections = 3;
const uint16_t kBosch144BytesPerSection = 6;

using irutils::addBoolToString;
using irutils::addModeToString;
using irutils::addFanToString;
using irutils::addTempToString;

#if SEND_BOSCH144
/// Send a Bosch 144-bit / 18-byte message (96-bit message are also possible)
/// Status: BETA / Probably Working.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void IRsend::sendBosch144(const unsigned char data[], const uint16_t nbytes,
                          const uint16_t repeat) {
  // nbytes is required to be a multiple of kBosch144BytesPerSection.
  if (nbytes % kBosch144BytesPerSection != 0)return;
  // Set IR carrier frequency
  enableIROut(kBoschFreq);

  for (uint16_t r = 0; r <= repeat; r++) {
    for (uint16_t offset=0; offset < nbytes; offset += kBosch144BytesPerSection)
      // Section Header + Data + Footer
      sendGeneric(kBoschHdrMark, kBoschHdrSpace,
                  kBoschBitMark, kBoschOneSpace,
                  kBoschBitMark, kBoschZeroSpace,
                  kBoschBitMark, kBoschFooterSpace,
                  data + offset, kBosch144BytesPerSection,
                  kBoschFreq, true, 0, kDutyDefault);
    space(kDefaultMessageGap);  // Complete guess
  }
}

#endif  // SEND_BOSCH144

/// Class constructor.
/// @param[in] pin GPIO to be used when sending.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRBosch144AC::IRBosch144AC(const uint16_t pin, const bool inverted,
                           const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) { stateReset(); }

/// Reset the internal state to a fixed known good state.
void IRBosch144AC::stateReset(void) { setRaw(kBosch144DefaultState); }

/// Set up hardware to be able to send a message.
void IRBosch144AC::begin(void) { _irsend.begin(); }

#if SEND_BOSCH144
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRBosch144AC::send(const uint16_t repeat) {
  if (!powerFlag) {
    _irsend.sendBosch144(kBosch144Off, sizeof(kBosch144Off), repeat);
  }   // "Off" is a 96bit message
  else _irsend.sendBosch144(getRaw(), kBosch144StateLength, repeat);
  // other 96bit messages are not yet supported
}
#endif  // SEND_BOSCH144

/// Get a copy of the internal state as a valid code for this protocol.
/// @return A valid code for this protocol based on the current internal state.
uint8_t* IRBosch144AC::getRaw(void) { 
  return _.raw; 
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] new_code A valid code for this protocol.
void IRBosch144AC::setRaw(const uint8_t new_code[]) {
  std::memcpy(_.raw, new_code, kBosch144StateLength);
}

void IRBosch144AC::setPower(const bool on) {
  powerFlag = on;
}

bool IRBosch144AC::getPower(void) const {
  return true;
}

void IRBosch144AC::setTempRaw(const uint8_t code) {
  _.TempS1 = _.TempS2 = code >> 1; // save 4 bits in S1 and S2
  _.TempS3 = code & 1;             // save 1 bit in Section3
}

/// Set the temperature.
/// @param[in] degrees The temperature in degrees celsius.
void IRBosch144AC::setTemp(const uint8_t degrees) {
  uint8_t temp = std::max(kBosch144TempMin, degrees);
  temp = std::min(kBosch144TempMax, temp);
  setTempRaw(kBosch144TempMap[temp - kBosch144TempMin]);
}

uint8_t IRBosch144AC::getTemp(void) const {
  uint8_t temp = (_.TempS1 << 1) + _.TempS3;
  for (uint8_t i = 0; i < 16; i++) {
    if (temp == kBosch144TempMap[i]) return kBosch144TempMin + i;
  }
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
void IRBosch144AC::setFan(const uint16_t speed) {
  _.FanS1 = _.FanS2 = speed >> 6; // save 3 bits in S1 and S2
  _.FanS3 = speed & 0b111111;     // save 6 bits in Section3
}

/// Set the desired operation mode.
/// @param[in] mode The desired operation mode.
void IRBosch144AC::setMode(const uint8_t mode) {
  _.ModeS1 = _.ModeS2 = mode >> 1; // save 2 bits in S1 and S2
  _.ModeS3 = mode & 0b1;           // save 1 bit in Section3
  if (mode == kBosch144Auto || mode == kBosch144Dry) {
    _.FanS1 = _.FanS2 = 0b000;     // save 3 bits in S1 and S2
    _.FanS3 = kBosch144FanAuto0;   // save 6 bits in Section3
  }
}

/// Set the Quiet mode of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRBosch144AC::setQuiet(const bool on) {
  _.Quiet = on;                 // save 1 bit in Section3
  setFan(kBosch144FanAuto);     // set Fan -> Auto
}

/// Get the Quiet mode of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRBosch144AC::getQuiet(void) const { return _.Quiet; }

/// Convert a stdAc::opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRBosch144AC::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kCool:
      return kBosch144Cool;
    case stdAc::opmode_t::kHeat:
      return kBosch144Heat;
    case stdAc::opmode_t::kDry:
      return kBosch144Dry;
    case stdAc::opmode_t::kFan:
      return kBosch144Fan;
    default:
      return kBosch144Auto;
  }
}

/// Convert a stdAc::fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint16_t IRBosch144AC::convertFan(const stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin:
      return kBosch144Fan20;
    case stdAc::fanspeed_t::kLow:
      return kBosch144Fan40;
    case stdAc::fanspeed_t::kMedium:
      return kBosch144Fan60;
    case stdAc::fanspeed_t::kHigh:
      return kBosch144Fan80;
    case stdAc::fanspeed_t::kMax:
      return kBosch144Fan100;
    default:
      return kBosch144FanAuto;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::opmode_t IRBosch144AC::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kBosch144Cool: return stdAc::opmode_t::kCool;
    case kBosch144Heat: return stdAc::opmode_t::kHeat;
    case kBosch144Dry: return stdAc::opmode_t::kDry;
    case kBosch144Fan: return stdAc::opmode_t::kFan;
    default: return stdAc::opmode_t::kAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::fanspeed_t IRBosch144AC::toCommonFanSpeed(const uint16_t speed) {
  switch (speed) {
    case kBosch144Fan100: return stdAc::fanspeed_t::kMax;
    case kBosch144Fan80: return stdAc::fanspeed_t::kHigh;
    case kBosch144Fan60: return stdAc::fanspeed_t::kMedium;
    case kBosch144Fan40: return stdAc::fanspeed_t::kLow;
    case kBosch144Fan20: return stdAc::fanspeed_t::kMin;
    default: return stdAc::fanspeed_t::kAuto;
  }
}

/// Convert the current internal state into its stdAc::state_t equivalent.
/// @return The stdAc equivalent of the native settings.
stdAc::state_t IRBosch144AC::toCommon(void) const {
  stdAc::state_t result{};
  result.protocol = decode_type_t::BOSCH144;
  result.power = getPower();
  result.mode = toCommonMode((_.ModeS1 << 1) + _.ModeS3);
  result.celsius = true;
  result.degrees = getTemp();
  result.fanspeed = toCommonFanSpeed((_.FanS1 << 6) + _.FanS3);
  result.quiet = getQuiet();
  // Not supported.
  result.model = -1;
  result.turbo = false;
  result.swingv = stdAc::swingv_t::kOff;
  result.swingh = stdAc::swingh_t::kOff;
  result.light = false;
  result.filter = false;
  result.econo = false;
  result.clean = false;
  result.beep = false;
  result.clock = -1;
  result.sleep = -1;
  return result;
}

/// Convert the current internal state into a human readable string.
/// @return A human readable string.
String IRBosch144AC::toString(void) const {
  uint8_t mode = (_.ModeS1 << 1)+_.ModeS3;
  uint16_t fan = (_.FanS1 << 6) + _.FanS3;
  String result = "";
  result.reserve(70);  // Reserve some heap for the string to reduce fragging.
  result += addBoolToString(getPower(), kPowerStr, false);
  result += addModeToString(mode, kBosch144Auto, kBosch144Cool,
                            kBosch144Heat, kBosch144Dry, kBosch144Fan);
  result += addFanToString(fan>>1, kBosch144Fan100>>1, kBosch144Fan20>>1,
                           kBosch144FanAuto>>1, kBosch144FanAuto>>1,
                           kBosch144Fan60>>1);
  result += addTempToString(getTemp());
  return result;
}

void IRBosch144AC::setInvertBytes() {
  _.InnvertS1_1 = ~_.raw[0];
  _.InnvertS1_2 = ~_.raw[2];
  _.InnvertS1_3 = ~_.raw[4];
  _.InnvertS2_1 = ~_.raw[6];
  _.InnvertS2_2 = ~_.raw[8];
  _.InnvertS2_3 = ~_.raw[10];
}

void IRBosch144AC::setCheckSumS3() {
  _.ChecksumS3 =  sumBytes(&(_.raw[12]), 5);
}

#if DECODE_BOSCH144
/// Decode the supplied Bosch 144-bit / 18-byte A/C message.
/// Status: STABLE / Confirmed Working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool IRrecv::decodeBosch144(decode_results *results, uint16_t offset,
                            const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * nbits +
                        kBosch144NrOfSections * (kHeader + kFooter) -
                        1 + offset)
    return false;  // Can't possibly be a valid BOSCH144 message.
  if (strict && nbits != kBosch144Bits)
    return false;      // Not strictly a BOSCH144 message.
  if (nbits % 8 != 0)  // nbits has to be a multiple of nr. of bits in a byte.
    return false;
  if (nbits % kBosch144NrOfSections != 0)
    return false;  // nbits has to be a multiple of kBosch144NrOfSections.
  const uint16_t kSectionBits = nbits / kBosch144NrOfSections;
  const uint16_t kSectionBytes = kSectionBits / 8;
  const uint16_t kNBytes = kSectionBytes * kBosch144NrOfSections;
  // Capture each section individually
  for (uint16_t pos = 0, section = 0;
       pos < kNBytes;
       pos += kSectionBytes, section++) {
    uint16_t used = 0;
    // Section Header + Section Data + Section Footer
    used = matchGeneric(results->rawbuf + offset, results->state + pos,
                        results->rawlen - offset, kSectionBits,
                        kBoschHdrMark, kBoschHdrSpace,
                        kBoschBitMark, kBoschOneSpace,
                        kBoschBitMark, kBoschZeroSpace,
                        kBoschBitMark, kBoschFooterSpace,
                        section >= kBosch144NrOfSections - 1,
                        _tolerance, kMarkExcess, true);
    if (!used) return false;  // Didn't match.
    offset += used;
  }

  // Compliance

  // Success
  results->decode_type = decode_type_t::BOSCH144;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // DECODE_BOSCH144
