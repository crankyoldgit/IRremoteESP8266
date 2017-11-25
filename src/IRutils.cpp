// Copyright 2017 David Conran

#include "IRutils.h"
#ifndef UNIT_TEST
#include <Arduino.h>
#endif

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <algorithm>
#ifndef ARDUINO
#include <string>
#endif
#include "IRrecv.h"
#include "IRremoteESP8266.h"

// Reverse the order of the requested least significant nr. of bits.
// Args:
//   input: Bit pattern/integer to reverse.
//   nbits: Nr. of bits to reverse.
// Returns:
//   The reversed bit pattern.
uint64_t reverseBits(uint64_t input, uint16_t nbits) {
  if (nbits <= 1)
    return input;  // Reversing <= 1 bits makes no change at all.
  // Cap the nr. of bits to rotate to the max nr. of bits in the input.
  nbits = std::min(nbits, (uint16_t) (sizeof(input) * 8));
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
#ifdef ARDUINO  // Arduino's & C++'s string implementations can't co-exist.
String uint64ToString(uint64_t input, uint8_t base) {
  String result = "";
#else
std::string uint64ToString(uint64_t input, uint8_t base) {
  std::string result = "";
#endif
  // prevent issues if called with base <= 1
  if (base < 2) base = 10;
  // Check we have a base that we can actually print.
  // i.e. [0-9A-Z] == 36
  if (base > 36) base = 10;

  do {
    char c = input % base;
    input /= base;

    if (c < 10)
      c +='0';
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

// Convert a protocol type (enum etc) to a human readable string.
// Args:
//   protocol: Nr. (enum) of the protocol.
//   isRepeat: A flag indicating if it is a repeat message of the protocol.
// Returns:
//   A string containing the protocol name.
#ifdef ARDUINO  // Arduino's & C++'s string implementations can't co-exist.
String typeToString(const decode_type_t protocol, const bool isRepeat) {
  String result = "";
#else
std::string typeToString(const decode_type_t protocol,
                         const bool isRepeat) {
  std::string result = "";
#endif
  switch (protocol) {
    default:
    case UNKNOWN:       result = "UNKNOWN";           break;
    case UNUSED:        result = "UNUSED";            break;
    case AIWA_RC_T501:  result = "AIWA_RC_T501";      break;
    case ARGO:          result = "ARGO";              break;
    case COOLIX:        result = "COOLIX";            break;
    case DAIKIN:        result = "DAIKIN";            break;
    case DENON:         result = "DENON";             break;
    case DISH:          result = "DISH";              break;
    case FUJITSU_AC:    result = "FUJITSU_AC";        break;
    case GLOBALCACHE:   result = "GLOBALCACHE";       break;
    case GREE:          result = "GREE";              break;
    case JVC:           result = "JVC";               break;
    case KELVINATOR:    result = "KELVINATOR";        break;
    case LG:            result = "LG";                break;
    case MIDEA:         result = "MIDEA";             break;
    case MITSUBISHI:    result = "MITSUBISHI";        break;
    case MITSUBISHI_AC: result = "MITSUBISHI_AC";     break;
    case NEC:           result = "NEC";               break;
    case NEC_LIKE:      result = "NEC (non-strict)";  break;
    case NIKAI:         result = "NIKAI";             break;
    case PANASONIC:     result = "PANASONIC";         break;
    case PRONTO:        result = "PRONTO";            break;
    case RAW:           result = "RAW";               break;
    case RC5:           result = "RC5";               break;
    case RC5X:          result = "RC5X";              break;
    case RC6:           result = "RC6";               break;
    case RCMM:          result = "RCMM";              break;
    case SAMSUNG:       result = "SAMSUNG";           break;
    case SANYO:         result = "SANYO";             break;
    case SANYO_LC7461:  result = "SANYO_LC7461";      break;
    case SHARP:         result = "SHARP";             break;
    case SHERWOOD:      result = "SHERWOOD";          break;
    case SONY:          result = "SONY";              break;
    case TOSHIBA_AC:    result = "TOSHIBA_AC";        break;
    case TROTEC:        result = "TROTEC";            break;
    case WHYNTER:       result = "WHYNTER";           break;
  }
  if (isRepeat) result += " (Repeat)";
  return result;
}

// Does the given protocol use a complex state as part of the decode?
bool hasACState(const decode_type_t protocol) {
  switch (protocol) {
    case DAIKIN:
    case KELVINATOR:
    case TOSHIBA_AC:
      return true;
    default:
      return false;
  }
}
