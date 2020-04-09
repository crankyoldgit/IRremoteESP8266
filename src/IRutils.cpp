// Copyright 2017 David Conran

#include "IRutils.h"
#ifndef UNIT_TEST
#include <Arduino.h>
#endif

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <string.h>
#include <algorithm>
#ifndef ARDUINO
#include <string>
#endif
#include "IRrecv.h"
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "IRtext.h"

// Reverse the order of the requested least significant nr. of bits.
// Args:
//   input: Bit pattern/integer to reverse.
//   nbits: Nr. of bits to reverse.
// Returns:
//   The reversed bit pattern.
uint64_t reverseBits(uint64_t input, uint16_t nbits) {
  if (nbits <= 1) return input;  // Reversing <= 1 bits makes no change at all.
  // Cap the nr. of bits to rotate to the max nr. of bits in the input.
  nbits = std::min(nbits, (uint16_t)(sizeof(input) * 8));
  uint64_t output = 0;
  for (uint16_t i = 0; i < nbits; i++) {
    output <<= 1;
    output |= (input & 1);
    input >>= 1;
  }
  // Merge any remaining unreversed bits back to the top of the reversed bits.
  return (input << nbits) | output;
}

// Convert a uint64_t (unsigned long long) to a string.
// Arduino String/toInt/Serial.print() can't handle printing 64 bit values.
//
// Args:
//   input: The value to print
//   base:  The output base.
// Returns:
//   A string representation of the integer.
// Note: Based on Arduino's Print::printNumber()
String uint64ToString(uint64_t input, uint8_t base) {
  String result = "";
  // prevent issues if called with base <= 1
  if (base < 2) base = 10;
  // Check we have a base that we can actually print.
  // i.e. [0-9A-Z] == 36
  if (base > 36) base = 10;

  // Reserve some string space to reduce fragmentation.
  // 16 bytes should store a uint64 in hex text which is the likely worst case.
  // 64 bytes would be the worst case (base 2).
  result.reserve(16);

  do {
    char c = input % base;
    input /= base;

    if (c < 10)
      c += '0';
    else
      c += 'A' - 10;
    result = c + result;
  } while (input);
  return result;
}

#ifdef ARDUINO
// Print a uint64_t/unsigned long long to the Serial port
// Serial.print() can't handle printing long longs. (uint64_t)
//
// Args:
//   input: The value to print
//   base: The output base.
void serialPrintUint64(uint64_t input, uint8_t base) {
  Serial.print(uint64ToString(input, base));
}
#endif

// Convert a C-style str to a decode_type_t
//
// Args:
//   str:  A C-style string containing a protocol name or number.
// Returns:
//  A decode_type_t enum.
decode_type_t strToDecodeType(const char * const str) {
  if (!strcasecmp(str, kUnknownStr))
    return decode_type_t::UNKNOWN;
  else if (!strcasecmp(str, kUnusedStr))
    return decode_type_t::UNUSED;
  else if (!strcasecmp(str, kAirwellStr))
    return decode_type_t::AIRWELL;
  else if (!strcasecmp(str, kAiwaRcT501Str))
    return decode_type_t::AIWA_RC_T501;
  else if (!strcasecmp(str, kAmcorStr))
    return decode_type_t::AMCOR;
  else if (!strcasecmp(str, kArgoStr))
    return decode_type_t::ARGO;
  else if (!strcasecmp(str, kCarrierAcStr))
    return decode_type_t::CARRIER_AC;
  else if (!strcasecmp(str, kCoolixStr))
    return decode_type_t::COOLIX;
  else if (!strcasecmp(str, kDaikinStr))
    return decode_type_t::DAIKIN;
  else if (!strcasecmp(str, kDaikin128Str))
    return decode_type_t::DAIKIN128;
  else if (!strcasecmp(str, kDaikin152Str))
    return decode_type_t::DAIKIN152;
  else if (!strcasecmp(str, kDaikin160Str))
    return decode_type_t::DAIKIN160;
  else if (!strcasecmp(str, kDaikin176Str))
    return decode_type_t::DAIKIN176;
  else if (!strcasecmp(str, kDaikin2Str))
    return decode_type_t::DAIKIN2;
  else if (!strcasecmp(str, kDaikin216Str))
    return decode_type_t::DAIKIN216;
  else if (!strcasecmp(str, kDaikin64Str))
    return decode_type_t::DAIKIN64;
  else if (!strcasecmp(str, kDenonStr))
    return decode_type_t::DENON;
  else if (!strcasecmp(str, kDishStr))
    return decode_type_t::DISH;
  else if (!strcasecmp(str, kElectraAcStr))
    return decode_type_t::ELECTRA_AC;
  else if (!strcasecmp(str, kEpsonStr))
    return decode_type_t::EPSON;
  else if (!strcasecmp(str, kFujitsuAcStr))
    return decode_type_t::FUJITSU_AC;
  else if (!strcasecmp(str, kGICableStr))
    return decode_type_t::GICABLE;
  else if (!strcasecmp(str, kGlobalCacheStr))
    return decode_type_t::GLOBALCACHE;
  else if (!strcasecmp(str, kGoodweatherStr))
    return decode_type_t::GOODWEATHER;
  else if (!strcasecmp(str, kGreeStr))
    return decode_type_t::GREE;
  else if (!strcasecmp(str, kHaierAcStr))
    return decode_type_t::HAIER_AC;
  else if (!strcasecmp(str, kHaierAcYrw02Str))
    return decode_type_t::HAIER_AC_YRW02;
  else if (!strcasecmp(str, kHitachiAcStr))
    return decode_type_t::HITACHI_AC;
  else if (!strcasecmp(str, kHitachiAc1Str))
    return decode_type_t::HITACHI_AC1;
  else if (!strcasecmp(str, kHitachiAc2Str))
    return decode_type_t::HITACHI_AC2;
  else if (!strcasecmp(str, kHitachiAc3Str))
    return decode_type_t::HITACHI_AC3;
  else if (!strcasecmp(str, kHitachiAc424Str))
    return decode_type_t::HITACHI_AC424;
  else if (!strcasecmp(str, kInaxStr))
    return decode_type_t::INAX;
  else if (!strcasecmp(str, kJvcStr))
    return decode_type_t::JVC;
  else if (!strcasecmp(str, kKelvinatorStr))
    return decode_type_t::KELVINATOR;
  else if (!strcasecmp(str, kLasertagStr))
    return decode_type_t::LASERTAG;
  else if (!strcasecmp(str, kLegopfStr))
    return decode_type_t::LEGOPF;
  else if (!strcasecmp(str, kLgStr))
    return decode_type_t::LG;
  else if (!strcasecmp(str, kLg2Str))
    return decode_type_t::LG2;
  else if (!strcasecmp(str, kLutronStr))
    return decode_type_t::LUTRON;
  else if (!strcasecmp(str, kMagiquestStr))
    return decode_type_t::MAGIQUEST;
  else if (!strcasecmp(str, kMideaStr))
    return decode_type_t::MIDEA;
  else if (!strcasecmp(str, kMitsubishiStr))
    return decode_type_t::MITSUBISHI;
  else if (!strcasecmp(str, kMitsubishi2Str))
    return decode_type_t::MITSUBISHI2;
  else if (!strcasecmp(str, kMitsubishiAcStr))
    return decode_type_t::MITSUBISHI_AC;
  else if (!strcasecmp(str, kMitsubishi136Str))
    return decode_type_t::MITSUBISHI136;
  else if (!strcasecmp(str, kMitsubishi112Str))
    return decode_type_t::MITSUBISHI112;
  else if (!strcasecmp(str, kMitsubishiHeavy88Str))
    return decode_type_t::MITSUBISHI_HEAVY_88;
  else if (!strcasecmp(str, kMitsubishiHeavy152Str))
    return decode_type_t::MITSUBISHI_HEAVY_152;
  else if (!strcasecmp(str, kMwmStr))
    return decode_type_t::MWM;
  else if (!strcasecmp(str, kNeoclimaStr))
    return decode_type_t::NEOCLIMA;
  else if (!strcasecmp(str, kNecStr))
    return decode_type_t::NEC;
  else if (!strcasecmp(str, kNecLikeStr) ||
           !strcasecmp(str, kNecNonStrictStr))
    return decode_type_t::NEC_LIKE;
  else if (!strcasecmp(str, kNikaiStr))
    return decode_type_t::NIKAI;
  else if (!strcasecmp(str, kPanasonicStr))
    return decode_type_t::PANASONIC;
  else if (!strcasecmp(str, kPanasonicAcStr))
    return decode_type_t::PANASONIC_AC;
  else if (!strcasecmp(str, kPioneerStr))
    return decode_type_t::PIONEER;
  else if (!strcasecmp(str, kProntoStr))
    return decode_type_t::PRONTO;
  else if (!strcasecmp(str, kRawStr))
    return decode_type_t::RAW;
  else if (!strcasecmp(str, kRc5Str))
    return decode_type_t::RC5;
  else if (!strcasecmp(str, kRc5XStr))
    return decode_type_t::RC5X;
  else if (!strcasecmp(str, kRc6Str))
    return decode_type_t::RC6;
  else if (!strcasecmp(str, kRcmmStr))
    return decode_type_t::RCMM;
  else if (!strcasecmp(str, kSamsungStr))
    return decode_type_t::SAMSUNG;
  else if (!strcasecmp(str, kSamsung36Str))
    return decode_type_t::SAMSUNG36;
  else if (!strcasecmp(str, kSamsungAcStr))
    return decode_type_t::SAMSUNG_AC;
  else if (!strcasecmp(str, kSanyoStr))
    return decode_type_t::SANYO;
  else if (!strcasecmp(str, kSanyoLc7461Str))
    return decode_type_t::SANYO_LC7461;
  else if (!strcasecmp(str, kSharpStr))
    return decode_type_t::SHARP;
  else if (!strcasecmp(str, kSharpAcStr))
    return decode_type_t::SHARP_AC;
  else if (!strcasecmp(str, kSherwoodStr))
    return decode_type_t::SHERWOOD;
  else if (!strcasecmp(str, kSonyStr))
    return decode_type_t::SONY;
  else if (!strcasecmp(str, kSony38KStr))
    return decode_type_t::SONY_38K;
  else if (!strcasecmp(str, kSymphonyStr))
    return decode_type_t::SYMPHONY;
  else if (!strcasecmp(str, kTcl112AcStr))
    return decode_type_t::TCL112AC;
  else if (!strcasecmp(str, kTecoStr))
    return decode_type_t::TECO;
  else if (!strcasecmp(str, kToshibaAcStr))
    return decode_type_t::TOSHIBA_AC;
  else if (!strcasecmp(str, kTrotecStr))
    return decode_type_t::TROTEC;
  else if (!strcasecmp(str, kVestelAcStr))
    return decode_type_t::VESTEL_AC;
  else if (!strcasecmp(str, kWhirlpoolAcStr))
    return decode_type_t::WHIRLPOOL_AC;
  else if (!strcasecmp(str, kWhynterStr))
    return decode_type_t::WHYNTER;
  // Handle integer values of the type by converting to a string and back again.
  decode_type_t result = strToDecodeType(
      typeToString((decode_type_t)atoi(str)).c_str());
  if (result > 0)
    return result;
  else
    return decode_type_t::UNKNOWN;
}

// Convert a protocol type (enum etc) to a human readable string.
// Args:
//   protocol: Nr. (enum) of the protocol.
//   isRepeat: A flag indicating if it is a repeat message of the protocol.
// Returns:
//   A string containing the protocol name.
String typeToString(const decode_type_t protocol, const bool isRepeat) {
  String result = "";
  switch (protocol) {
    case UNUSED:
      result = kUnusedStr;
      break;
    case AIRWELL:
      result = kAirwellStr;
      break;
    case AIWA_RC_T501:
      result = kAiwaRcT501Str;
      break;
    case AMCOR:
      result = kAmcorStr;
      break;
    case ARGO:
      result = kArgoStr;
      break;
    case CARRIER_AC:
      result = kCarrierAcStr;
      break;
    case COOLIX:
      result = kCoolixStr;
      break;
    case DAIKIN:
      result = kDaikinStr;
      break;
    case DAIKIN128:
      result = kDaikin128Str;
      break;
    case DAIKIN152:
      result = kDaikin152Str;
      break;
    case DAIKIN160:
      result = kDaikin160Str;
      break;
    case DAIKIN176:
      result = kDaikin176Str;
      break;
    case DAIKIN2:
      result = kDaikin2Str;
      break;
    case DAIKIN216:
      result = kDaikin216Str;
      break;
    case DAIKIN64:
      result = kDaikin64Str;
      break;
    case DENON:
      result = kDenonStr;
      break;
    case DISH:
      result = kDishStr;
      break;
    case ELECTRA_AC:
      result = kElectraAcStr;
      break;
    case EPSON:
      result = kEpsonStr;
      break;
    case FUJITSU_AC:
      result = kFujitsuAcStr;
      break;
    case GICABLE:
      result = kGICableStr;
      break;
    case GLOBALCACHE:
      result = kGlobalCacheStr;
      break;
    case GOODWEATHER:
      result = kGoodweatherStr;
      break;
    case GREE:
      result = kGreeStr;
      break;
    case HAIER_AC:
      result = kHaierAcStr;
      break;
    case HAIER_AC_YRW02:
      result = kHaierAcYrw02Str;
      break;
    case HITACHI_AC:
      result = kHitachiAcStr;
      break;
    case HITACHI_AC1:
      result = kHitachiAc1Str;
      break;
    case HITACHI_AC2:
      result = kHitachiAc2Str;
      break;
    case HITACHI_AC3:
      result = kHitachiAc3Str;
      break;
    case HITACHI_AC424:
      result = kHitachiAc424Str;
      break;
    case INAX:
      result = kInaxStr;
      break;
    case JVC:
      result = kJvcStr;
      break;
    case KELVINATOR:
      result = kKelvinatorStr;
      break;
    case LEGOPF:
      result = kLegopfStr;
      break;
    case LG:
      result = kLgStr;
      break;
    case LG2:
      result = kLg2Str;
      break;
    case LASERTAG:
      result = kLasertagStr;
      break;
    case LUTRON:
      result = kLutronStr;
      break;
    case MAGIQUEST:
      result = kMagiquestStr;
      break;
    case MIDEA:
      result = kMideaStr;
      break;
    case MITSUBISHI:
      result = kMitsubishiStr;
      break;
    case MITSUBISHI2:
      result = kMitsubishi2Str;
      break;
    case MITSUBISHI_AC:
      result = kMitsubishiAcStr;
      break;
    case MITSUBISHI136:
      result = kMitsubishi136Str;
      break;
    case MITSUBISHI112:
      result = kMitsubishi112Str;
      break;
    case MITSUBISHI_HEAVY_88:
      result = kMitsubishiHeavy88Str;
      break;
    case MITSUBISHI_HEAVY_152:
      result = kMitsubishiHeavy152Str;
      break;
    case MWM:
      result = kMwmStr;
      break;
    case NEOCLIMA:
      result = kNeoclimaStr;
      break;
    case NEC:
      result = kNecStr;
      break;
    case NEC_LIKE:
      result = kNecNonStrictStr;
      break;
    case NIKAI:
      result = kNikaiStr;
      break;
    case PANASONIC:
      result = kPanasonicStr;
      break;
    case PANASONIC_AC:
      result = kPanasonicAcStr;
      break;
    case PIONEER:
      result = kPioneerStr;
      break;
    case PRONTO:
      result = kProntoStr;
      break;
    case RAW:
      result = kRawStr;
      break;
    case RC5:
      result = kRc5Str;
      break;
    case RC5X:
      result = kRc5XStr;
      break;
    case RC6:
      result = kRc6Str;
      break;
    case RCMM:
      result = kRcmmStr;
      break;
    case SAMSUNG:
      result = kSamsungStr;
      break;
    case SAMSUNG36:
      result = kSamsung36Str;
      break;
    case SAMSUNG_AC:
      result = kSamsungAcStr;
      break;
    case SANYO:
      result = kSanyoStr;
      break;
    case SANYO_LC7461:
      result = kSanyoLc7461Str;
      break;
    case SHARP:
      result = kSharpStr;
      break;
    case SHARP_AC:
      result = kSharpAcStr;
      break;
    case SHERWOOD:
      result = kSherwoodStr;
      break;
    case SONY:
      result = kSonyStr;
      break;
    case SONY_38K:
      result = kSony38KStr;
      break;
    case SYMPHONY:
      result = kSymphonyStr;
      break;
    case TCL112AC:
      result = kTcl112AcStr;
      break;
    case TECO:
      result = kTecoStr;
      break;
    case TOSHIBA_AC:
      result = kToshibaAcStr;
      break;
    case TROTEC:
      result = kTrotecStr;
      break;
    case VESTEL_AC:
      result = kVestelAcStr;
      break;
    case WHIRLPOOL_AC:
      result = kWhirlpoolAcStr;
      break;
    case WHYNTER:
      result = kWhynterStr;
      break;
    case UNKNOWN:
    default:
      result = kUnknownStr;
      break;
  }
  if (isRepeat) {
    result += kSpaceLBraceStr;
    result += kRepeatStr;
    result += ')';
  }
  return result;
}

// Does the given protocol use a complex state as part of the decode?
bool hasACState(const decode_type_t protocol) {
  switch (protocol) {
    case AMCOR:
    case ARGO:
    case DAIKIN:
    case DAIKIN128:
    case DAIKIN152:
    case DAIKIN160:
    case DAIKIN176:
    case DAIKIN2:
    case DAIKIN216:
    case ELECTRA_AC:
    case FUJITSU_AC:
    case GREE:
    case HAIER_AC:
    case HAIER_AC_YRW02:
    case HITACHI_AC:
    case HITACHI_AC1:
    case HITACHI_AC2:
    case HITACHI_AC3:
    case HITACHI_AC424:
    case KELVINATOR:
    case MITSUBISHI136:
    case MITSUBISHI112:
    case MITSUBISHI_AC:
    case MITSUBISHI_HEAVY_88:
    case MITSUBISHI_HEAVY_152:
    case MWM:
    case NEOCLIMA:
    case PANASONIC_AC:
    case SAMSUNG_AC:
    case SHARP_AC:
    case TCL112AC:
    case TOSHIBA_AC:
    case TROTEC:
    case WHIRLPOOL_AC:
      return true;
    default:
      return false;
  }
}

// Return the corrected length of a 'raw' format array structure
// after over-large values are converted into multiple entries.
// Args:
//   results: A ptr to a decode result.
// Returns:
//   A uint16_t containing the length.
uint16_t getCorrectedRawLength(const decode_results * const results) {
  uint16_t extended_length = results->rawlen - 1;
  for (uint16_t i = 0; i < results->rawlen - 1; i++) {
    uint32_t usecs = results->rawbuf[i] * kRawTick;
    // Add two extra entries for multiple larger than UINT16_MAX it is.
    extended_length += (usecs / (UINT16_MAX + 1)) * 2;
  }
  return extended_length;
}

// Return a string containing the key values of a decode_results structure
// in a C/C++ code style format.
String resultToSourceCode(const decode_results * const results) {
  String output = "";
  // Reserve some space for the string to reduce heap fragmentation.
  output.reserve(1536);  // 1.5KB should cover most cases.
  // Start declaration
  output += F("uint16_t ");  // variable type
  output += F("rawData[");   // array name
  output += uint64ToString(getCorrectedRawLength(results), 10);
  // array size
  output += F("] = {");  // Start declaration

  // Dump data
  for (uint16_t i = 1; i < results->rawlen; i++) {
    uint32_t usecs;
    for (usecs = results->rawbuf[i] * kRawTick; usecs > UINT16_MAX;
         usecs -= UINT16_MAX) {
      output += uint64ToString(UINT16_MAX);
      if (i % 2)
        output += F(", 0,  ");
      else
        output += F(",  0, ");
    }
    output += uint64ToString(usecs, 10);
    if (i < results->rawlen - 1)
      output += kCommaSpaceStr;            // ',' not needed on the last one
    if (i % 2 == 0) output += ' ';  // Extra if it was even.
  }

  // End declaration
  output += F("};");

  // Comment
  output += F("  // ");
  output += typeToString(results->decode_type, results->repeat);
  // Only display the value if the decode type doesn't have an A/C state.
  if (!hasACState(results->decode_type))
    output += ' ' + uint64ToString(results->value, 16);
  output += F("\n");

  // Now dump "known" codes
  if (results->decode_type != UNKNOWN) {
    if (hasACState(results->decode_type)) {
#if DECODE_AC
      uint16_t nbytes = results->bits / 8;
      output += F("uint8_t state[");
      output += uint64ToString(nbytes);
      output += F("] = {");
      for (uint16_t i = 0; i < nbytes; i++) {
        output += F("0x");
        if (results->state[i] < 0x10) output += '0';
        output += uint64ToString(results->state[i], 16);
        if (i < nbytes - 1) output += kCommaSpaceStr;
      }
      output += F("};\n");
#endif  // DECODE_AC
    } else {
      // Simple protocols
      // Some protocols have an address &/or command.
      // NOTE: It will ignore the atypical case when a message has been
      // decoded but the address & the command are both 0.
      if (results->address > 0 || results->command > 0) {
        output += F("uint32_t address = 0x");
        output += uint64ToString(results->address, 16);
        output += F(";\n");
        output += F("uint32_t command = 0x");
        output += uint64ToString(results->command, 16);
        output += F(";\n");
      }
      // Most protocols have data
      output += F("uint64_t data = 0x");
      output += uint64ToString(results->value, 16);
      output += F(";\n");
    }
  }
  return output;
}

// Dump out the decode_results structure.
//
String resultToTimingInfo(const decode_results * const results) {
  String output = "";
  String value = "";
  // Reserve some space for the string to reduce heap fragmentation.
  output.reserve(2048);  // 2KB should cover most cases.
  value.reserve(6);  // Max value should be 2^17 = 131072
  output += F("Raw Timing[");
  output += uint64ToString(results->rawlen - 1, 10);
  output += F("]:\n");

  for (uint16_t i = 1; i < results->rawlen; i++) {
    if (i % 2 == 0)
      output += '-';  // even
    else
      output += F("   +");  // odd
    value = uint64ToString(results->rawbuf[i] * kRawTick);
    // Space pad the value till it is at least 6 chars long.
    while (value.length() < 6) value = ' ' + value;
    output += value;
    if (i < results->rawlen - 1)
      output += kCommaSpaceStr;  // ',' not needed for last one
    if (!(i % 8)) output += '\n';  // Newline every 8 entries.
  }
  output += '\n';
  return output;
}

// Convert the decode_results structure's value/state to simple hexadecimal.
//
String resultToHexidecimal(const decode_results * const result) {
  String output = F("0x");
  // Reserve some space for the string to reduce heap fragmentation.
  output.reserve(2 * kStateSizeMax + 2);  // Should cover worst cases.
  if (hasACState(result->decode_type)) {
#if DECODE_AC
    for (uint16_t i = 0; result->bits > i * 8; i++) {
      if (result->state[i] < 0x10) output += '0';  // Zero pad
      output += uint64ToString(result->state[i], 16);
    }
#endif  // DECODE_AC
  } else {
    output += uint64ToString(result->value, 16);
  }
  return output;
}

// Dump out the decode_results structure.
//
String resultToHumanReadableBasic(const decode_results * const results) {
  String output = "";
  // Reserve some space for the string to reduce heap fragmentation.
  output.reserve(2 * kStateSizeMax + 50);  // Should cover most cases.
  // Show Encoding standard
  output += kProtocolStr;
  output += F("  : ");
  output += typeToString(results->decode_type, results->repeat);
  output += '\n';

  // Show Code & length
  output += kCodeStr;
  output += F("      : ");
  output += resultToHexidecimal(results);
  output += kSpaceLBraceStr;
  output += uint64ToString(results->bits);
  output += ' ';
  output += kBitsStr;
  output +=  F(")\n");
  return output;
}

// Convert a decode_results into an array suitable for `sendRaw()`.
// Args:
//   decode:  A pointer to an IR decode_results structure that contains a mesg.
// Returns:
//   A pointer to a dynamically allocated uint16_t sendRaw compatible array.
// Note:
//   Result needs to be delete[]'ed/free()'ed (deallocated) after use by caller.
uint16_t* resultToRawArray(const decode_results * const decode) {
  uint16_t *result = new uint16_t[getCorrectedRawLength(decode)];
  if (result != NULL) {  // The memory was allocated successfully.
    // Convert the decode data.
    uint16_t pos = 0;
    for (uint16_t i = 1; i < decode->rawlen; i++) {
      uint32_t usecs = decode->rawbuf[i] * kRawTick;
      while (usecs > UINT16_MAX) {  // Keep truncating till it fits.
        result[pos++] = UINT16_MAX;
        result[pos++] = 0;  // A 0 in a sendRaw() array basically means skip.
        usecs -= UINT16_MAX;
      }
      result[pos++] = usecs;
    }
  }
  return result;
}

uint8_t sumBytes(const uint8_t * const start, const uint16_t length,
                 const uint8_t init) {
  uint8_t checksum = init;
  const uint8_t *ptr;
  for (ptr = start; ptr - start < length; ptr++) checksum += *ptr;
  return checksum;
}

uint8_t xorBytes(const uint8_t * const start, const uint16_t length,
                 const uint8_t init) {
  uint8_t checksum = init;
  const uint8_t *ptr;
  for (ptr = start; ptr - start < length; ptr++) checksum ^= *ptr;
  return checksum;
}

// Count the number of bits of a certain type.
// Args:
//   start: Ptr to the start of data to count bits in.
//   length: How many bytes to count.
//   ones: Count the binary 1 bits. False for counting the 0 bits.
//   init: Start the counting from this value.
// Returns:
//   Nr. of bits found.
uint16_t countBits(const uint8_t * const start, const uint16_t length,
                   const bool ones, const uint16_t init) {
  uint16_t count = init;
  for (uint16_t offset = 0; offset < length; offset++)
    for (uint8_t currentbyte = *(start + offset);
         currentbyte;
         currentbyte >>= 1)
      if (currentbyte & 1) count++;
  if (ones || length == 0)
    return count;
  else
    return (length * 8) - count;
}

// Count the number of bits of a certain type.
// Args:
//   data: The value you want bits counted for, starting from the LSB.
//   length: How many bits to count.
//   ones: Count the binary 1 bits. False for counting the 0 bits.
//   init: Start the counting from this value.
// Returns:
//   Nr. of bits found.
uint16_t countBits(const uint64_t data, const uint8_t length, const bool ones,
                   const uint16_t init) {
  uint16_t count = init;
  uint8_t bitsSoFar = length;
  for (uint64_t remainder = data; remainder && bitsSoFar;
       remainder >>= 1, bitsSoFar--)
      if (remainder & 1) count++;
  if (ones || length == 0)
    return count;
  else
    return length - count;
}

uint64_t invertBits(const uint64_t data, const uint16_t nbits) {
  // No change if we are asked to invert no bits.
  if (nbits == 0) return data;
  uint64_t result = ~data;
  // If we are asked to invert all the bits or more than we have, it's simple.
  if (nbits >= sizeof(data) * 8) return result;
  // Mask off any unwanted bits and return the result.
  return (result & ((1ULL << nbits) - 1));
}

float celsiusToFahrenheit(const float deg) { return (deg * 9.0) / 5.0 + 32.0; }

float fahrenheitToCelsius(const float deg) { return (deg - 32.0) * 5.0 / 9.0; }

namespace irutils {
  String addLabeledString(const String value, const String label,
                          const bool precomma) {
    String result = "";
    if (precomma) result += kCommaSpaceStr;
    result += label;
    result += kColonSpaceStr;
    return result + value;
  }

  String addBoolToString(const bool value, const String label,
                         const bool precomma) {
    return addLabeledString((value ? kOnStr : kOffStr), label, precomma);
  }

  String addIntToString(const uint16_t value, const String label,
                        const bool precomma) {
    return addLabeledString(uint64ToString(value), label, precomma);
  }

  String modelToStr(const decode_type_t protocol, const int16_t model) {
    switch (protocol) {
      case decode_type_t::FUJITSU_AC:
        switch (model) {
          case fujitsu_ac_remote_model_t::ARRAH2E: return F("ARRAH2E");
          case fujitsu_ac_remote_model_t::ARDB1: return F("ARDB1");
          case fujitsu_ac_remote_model_t::ARREB1E: return F("ARREB1E");
          case fujitsu_ac_remote_model_t::ARJW2: return F("ARJW2");
          case fujitsu_ac_remote_model_t::ARRY4: return F("ARRY4");
          default: return kUnknownStr;
        }
        break;
      case decode_type_t::GREE:
        switch (model) {
          case gree_ac_remote_model_t::YAW1F: return F("YAW1F");
          case gree_ac_remote_model_t::YBOFB: return F("YBOFB");
          default: return kUnknownStr;
        }
        break;
      case decode_type_t::HITACHI_AC1:
        switch (model) {
          case hitachi_ac1_remote_model_t::R_LT0541_HTA_A:
            return F("R-LT0541-HTA-A");
          case hitachi_ac1_remote_model_t::R_LT0541_HTA_B:
            return F("R-LT0541-HTA-B");
          default: return kUnknownStr;
        }
        break;
      case decode_type_t::LG:
      case decode_type_t::LG2:
        switch (model) {
          case lg_ac_remote_model_t::GE6711AR2853M: return F("GE6711AR2853M");
          case lg_ac_remote_model_t::AKB75215403: return F("AKB75215403");
          default: return kUnknownStr;
        }
        break;
      case decode_type_t::PANASONIC_AC:
        switch (model) {
          case panasonic_ac_remote_model_t::kPanasonicLke: return F("LKE");
          case panasonic_ac_remote_model_t::kPanasonicNke: return F("NKE");
          case panasonic_ac_remote_model_t::kPanasonicDke: return F("DKE");
          case panasonic_ac_remote_model_t::kPanasonicJke: return F("JKE");
          case panasonic_ac_remote_model_t::kPanasonicCkp: return F("CKP");
          case panasonic_ac_remote_model_t::kPanasonicRkr: return F("RKR");
          default: return kUnknownStr;
        }
        break;
      case decode_type_t::WHIRLPOOL_AC:
        switch (model) {
          case whirlpool_ac_remote_model_t::DG11J13A: return F("DG11J13A");
          case whirlpool_ac_remote_model_t::DG11J191: return F("DG11J191");
          default: return kUnknownStr;
        }
        break;
      default: return kUnknownStr;
    }
  }

  String addModelToString(const decode_type_t protocol, const int16_t model,
                          const bool precomma) {
    String result = addIntToString(model, kModelStr, precomma);
    result += kSpaceLBraceStr;
    result += modelToStr(protocol, model);
    return result + ')';
  }

  String addTempToString(const uint16_t degrees, const bool celsius,
                         const bool precomma) {
    String result = addIntToString(degrees, kTempStr, precomma);
    result += celsius ? 'C' : 'F';
    return result;
  }

  String addModeToString(const uint8_t mode, const uint8_t automatic,
                         const uint8_t cool, const uint8_t heat,
                         const uint8_t dry, const uint8_t fan) {
    String result = addIntToString(mode, kModeStr);
    result += kSpaceLBraceStr;
    if (mode == automatic) result += kAutoStr;
    else if (mode == cool) result += kCoolStr;
    else if (mode == heat) result += kHeatStr;
    else if (mode == dry) result += kDryStr;
    else if (mode == fan) result += kFanStr;
    else
      result += kUnknownStr;
    return result + ')';
  }

  String addDayToString(const uint8_t day_of_week, const int8_t offset,
                        const bool precomma) {
    String result = addIntToString(day_of_week, kDayStr, precomma);
    result += kSpaceLBraceStr;
    if ((uint8_t)(day_of_week + offset) < 7)
#if UNIT_TEST
      result += String(kThreeLetterDayOfWeekStr).substr(
          (day_of_week + offset) * 3, 3);
#else  // UNIT_TEST
      result += String(kThreeLetterDayOfWeekStr).substring(
          (day_of_week + offset) * 3, (day_of_week + offset) * 3 + 3);
#endif  // UNIT_TEST
    else
      result += kUnknownStr;
    return result + ')';
  }

  String addFanToString(const uint8_t speed, const uint8_t high,
                        const uint8_t low, const uint8_t automatic,
                        const uint8_t quiet, const uint8_t medium) {
    String result = addIntToString(speed, kFanStr);
    result += kSpaceLBraceStr;
    if (speed == high) result += kHighStr;
    else if (speed == low) result += kLowStr;
    else if (speed == automatic) result += kAutoStr;
    else if (speed == quiet) result += kQuietStr;
    else if (speed == medium) result += kMediumStr;
    else
     result += kUnknownStr;
    return result + ')';
  }

  // Escape any special HTML (unsafe) characters in a string. e.g. anti-XSS.
  // Args:
  //   unescaped: A string containing text to make HTML safe.
  // Returns:
  //   A string that is HTML safe.
  String htmlEscape(const String unescaped) {
    String result = "";
    uint16_t ulen = unescaped.length();
    result.reserve(ulen);  // The result will be at least the size of input.
    for (size_t i = 0; i < ulen; i++) {
      char c = unescaped[i];
      switch (c) {
        // ';!-"<>=&#{}() are all unsafe.
        case '\'':
          result += F("&apos;");
          break;
        case ';':
          result += F("&semi;");
          break;
        case '!':
          result += F("&excl;");
          break;
        case '-':
          result += F("&dash;");
          break;
        case '\"':
          result += F("&quot;");
          break;
        case '<':
          result += F("&lt;");
          break;
        case '>':
          result += F("&gt;");
          break;
        case '=':
          result += F("&#equals;");
          break;
        case '&':
          result += F("&amp;");
          break;
        case '#':
          result += F("&num;");
          break;
        case '{':
          result += F("&lcub;");
          break;
        case '}':
          result += F("&rcub;");
          break;
        case '(':
          result += F("&lpar;");
          break;
        case ')':
          result += F("&rpar;");
          break;
        default:
          result += c;
      }
    }
    return result;
  }

  String msToString(uint32_t const msecs) {
    uint32_t totalseconds = msecs / 1000;
    if (totalseconds == 0) return kNowStr;

    // Note: uint32_t can only hold up to 45 days, so uint8_t is safe.
    uint8_t days = totalseconds / (60 * 60 * 24);
    uint8_t hours = (totalseconds / (60 * 60)) % 24;
    uint8_t minutes = (totalseconds / 60) % 60;
    uint8_t seconds = totalseconds % 60;

    String result = "";
    if (days)
      result += uint64ToString(days) + ' ' + String((days > 1) ? kDaysStr
                                                               : kDayStr);
    if (hours) {
      if (result.length()) result += ' ';
      result += uint64ToString(hours) + ' ' + String((hours > 1) ? kHoursStr
                                                                 : kHourStr);
    }
    if (minutes) {
      if (result.length()) result += ' ';
      result += uint64ToString(minutes) + ' ' + String(
          (minutes > 1) ? kMinutesStr : kMinuteStr);
    }
    if (seconds) {
      if (result.length()) result += ' ';
      result += uint64ToString(seconds) + ' ' + String(
          (seconds > 1) ? kSecondsStr : kSecondStr);
    }
    return result;
  }

  String minsToString(const uint16_t mins) {
    String result = "";
    result.reserve(5);  // 23:59 is the typical worst case.
    if (mins / 60 < 10) result += '0';  // Zero pad the hours
    result += uint64ToString(mins / 60) + kTimeSep;
    if (mins % 60 < 10) result += '0';  // Zero pad the minutes.
    result += uint64ToString(mins % 60);
    return result;
  }

  // Sum all the nibbles together in a series of bytes.
  // Args:
  //   start: PTR to the start of the bytes.
  //   length: Nr of bytes to sum the nibbles of.
  //   init: Starting value of the sum.
  // Returns:
  //   A uint8_t sum of all the nibbles inc the init.
  uint8_t sumNibbles(const uint8_t * const start, const uint16_t length,
                     const uint8_t init) {
    uint8_t sum = init;
    const uint8_t *ptr;
    for (ptr = start; ptr - start < length; ptr++)
      sum += (*ptr >> 4) + (*ptr & 0xF);
    return sum;
  }

  uint8_t bcdToUint8(const uint8_t bcd) {
    if (bcd > 0x99) return 255;  // Too big.
    return (bcd >> 4) * 10 + (bcd & 0xF);
  }

  uint8_t uint8ToBcd(const uint8_t integer) {
    if (integer > 99) return 255;  // Too big.
    return ((integer / 10) << 4) + (integer % 10);
  }

  // Return the value of `position`th bit of `data`.
  // Args:
  //   data: Value to be examined.
  //   position: Nr. of the nth bit to be examined. `0` is the LSB.
  //   size: Nr. of bits in data.
  bool getBit(const uint64_t data, const uint8_t position, const uint8_t size) {
    if (position >= size) return false;  // Outside of range.
    return data & (1ULL << position);
  }

  // Return the value of `position`th bit of `data`.
  // Args:
  //   data: Value to be examined.
  //   position: Nr. of the nth bit to be examined. `0` is the LSB.
  bool getBit(const uint8_t data, const uint8_t position) {
    if (position >= 8) return false;  // Outside of range.
    return data & (1 << position);
  }

  // Return the value of `data` with the `position`th bit changed to `on`
  // Args:
  //   data: Value to be changed.
  //   position: Nr. of the bit to be changed. `0` is the LSB.
  //   on: Value to set the position'th bit to.
  //   size: Nr. of bits in data.
  uint64_t setBit(const uint64_t data, const uint8_t position, const bool on,
                  const uint8_t size) {
    if (position >= size) return data;  // Outside of range.
    uint64_t mask = 1ULL << position;
    if (on)
      return data | mask;
    else
      return data & ~mask;
  }

  // Return the value of `data` with the `position`th bit changed to `on`
  // Args:
  //   data: Value to be changed.
  //   position: Nr. of the bit to be changed. `0` is the LSB.
  //   on: Value to set the position'th bit to.
  uint8_t setBit(const uint8_t data, const uint8_t position, const bool on) {
    if (position >= 8) return data;  // Outside of range.
    uint8_t mask = 1 << position;
    if (on)
      return data | mask;
    else
      return data & ~mask;
  }

  // Change the value at the location `data_ptr` with the `position`th bit
  //   changed to `on`
  // Args:
  //   data: Ptr to the data to be changed.
  //   position: Nr. of the bit to be changed. `0` is the LSB.
  //   on: Value to set the position'th bit to.
  void setBit(uint8_t * const data, const uint8_t position, const bool on) {
    uint8_t mask = 1 << position;
    if (on)
      *data |= mask;
    else
      *data &= ~mask;
  }

  // Change the value at the location `data_ptr` with the `position`th bit
  //   changed to `on`
  // Args:
  //   data: Ptr to the data to be changed.
  //   position: Nr. of the bit to be changed. `0` is the LSB.
  //   on: Value to set the position'th bit to.
  void setBit(uint32_t * const data, const uint8_t position, const bool on) {
    uint32_t mask = (uint32_t)1 << position;
    if (on)
      *data |= mask;
    else
      *data &= ~mask;
  }

  // Change the value at the location `data_ptr` with the `position`th bit
  //   changed to `on`
  // Args:
  //   data: Ptr to the data to be changed.
  //   position: Nr. of the bit to be changed. `0` is the LSB.
  //   on: Value to set the position'th bit to.
  void setBit(uint64_t * const data, const uint8_t position, const bool on) {
    uint64_t mask = (uint64_t)1 << position;
    if (on)
      *data |= mask;
    else
      *data &= ~mask;
  }

  // Change the uint8_t pointed to by `dst` starting at the `offset`th bit
  //   and for `nbits` bits, with the contents of `data`.
  // Args:
  //   dst: Ptr to the uint8_t to be changed.
  //   offset: Nr. of bits from the Least Significant Bit to be ignored.
  //   nbits: Nr of bits of `data` to be placed into the destination uint8_t.
  //   data: Value to be placed into dst.
  void setBits(uint8_t * const dst, const uint8_t offset, const uint8_t nbits,
               const uint8_t data) {
    if (offset >= 8 || !nbits) return;  // Short circuit as it won't change.
    // Calculate the mask for the supplied value.
    uint8_t mask = UINT8_MAX >> (8 - ((nbits > 8) ? 8 : nbits));
    // Calculate the mask & clear the space for the data.
    // Clear the destination bits.
    *dst &= ~(uint8_t)(mask << offset);
    // Merge in the data.
    *dst |= ((data & mask) << offset);
  }

  // Change the uint32_t pointed to by `dst` starting at the `offset`th bit
  //   and for `nbits` bits, with the contents of `data`.
  // Args:
  //   dst: Ptr to the uint32_t to be changed.
  //   offset: Nr. of bits from the Least Significant Bit to be ignored.
  //   nbits: Nr of bits of `data` to be placed into the destination uint32_t.
  //   data: Value to be placed into dst.
  void setBits(uint32_t * const dst, const uint8_t offset, const uint8_t nbits,
               const uint32_t data) {
    if (offset >= 32 || !nbits) return;  // Short circuit as it won't change.
    // Calculate the mask for the supplied value.
    uint32_t mask = UINT32_MAX >> (32 - ((nbits > 32) ? 32 : nbits));
    // Calculate the mask & clear the space for the data.
    // Clear the destination bits.
    *dst &= ~(mask << offset);
    // Merge in the data.
    *dst |= ((data & mask) << offset);
  }

  // Change the uint64_t pointed to by `dst` starting at the `offset`th bit
  //   and for `nbits` bits, with the contents of `data`.
  // Args:
  //   dst: Ptr to the uint64_t to be changed.
  //   offset: Nr. of bits from the Least Significant Bit to be ignored.
  //   nbits: Nr of bits of `data` to be placed into the destination uint64_t.
  //   data: Value to be placed into dst.
  void setBits(uint64_t * const dst, const uint8_t offset, const uint8_t nbits,
               const uint64_t data) {
    if (offset >= 64 || !nbits) return;  // Short circuit as it won't change.
    // Calculate the mask for the supplied value.
    uint64_t mask = UINT64_MAX >> (64 - ((nbits > 64) ? 64 : nbits));
    // Calculate the mask & clear the space for the data.
    // Clear the destination bits.
    *dst &= ~(mask << offset);
    // Merge in the data.
    *dst |= ((data & mask) << offset);
  }
}  // namespace irutils
