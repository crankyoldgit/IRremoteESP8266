// Copyright 2026 bluebird
// Protocol for Mitsubishi Heavy Jinling series (RYD502A003B)
// See https://github.com/crankyoldgit/IRremoteESP8266/issues/2262
//
// Frame (8 bytes, 64-bit):
//   B0=FF B1=00 B2=fan B3=~B2 B4=(temp<<4)|mode B5=~B4 B6=2A B7=D5
// Timing: header 5950/7475, bit 508, one 3454, zero 1496, footer 508/7422

#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include "IRremoteESP8266.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

const uint16_t kMjHdrMark = 5950;
const uint16_t kMjHdrSpace = 7475;
const uint16_t kMjBitMark = 508;
const uint16_t kMjOneSpace = 3454;
const uint16_t kMjZeroSpace = 1496;
const uint16_t kMjFooterMark = 508;
const uint16_t kMjGap = 7422;
const uint16_t kMjMinGap = 7200;
const uint8_t kMjTolerance = 20;

void IRsend::sendMitsubishiHeavyJinling(const uint64_t data,
                                         const uint16_t nbits,
                                         const uint16_t repeat) {
  sendGeneric(kMjHdrMark, kMjHdrSpace,
              kMjBitMark, kMjOneSpace,
              kMjBitMark, kMjZeroSpace,
              kMjFooterMark, kMjGap,
              data, nbits, 38000, true,
              repeat, kDutyDefault);
}

bool IRrecv::decodeMitsubishiHeavyJinling(decode_results *results,
                                           uint16_t offset,
                                           const uint16_t nbits,
                                           const bool strict) {
  if (results->rawlen < 2 * nbits + kHeader + kFooter - 1)
    return false;

  uint64_t data = 0;
  // Match Header + Data
  if (!matchGeneric(results->rawbuf + offset, &data,
                    results->rawlen - offset, nbits,
                    kMjHdrMark, kMjHdrSpace,
                    kMjBitMark, kMjOneSpace,
                    kMjBitMark, kMjZeroSpace,
                    kMjFooterMark, kMjGap,
                    true, kMjTolerance, kMarkExcess, true))
    return false;

  if (strict) {
    uint8_t bytes[8];
    for (int i = 0; i < 8; i++)
      bytes[i] = (data >> (56 - i * 8)) & 0xFF;
    if (bytes[0] != 0xFF || bytes[1] != 0x00 ||
        bytes[6] != 0x2A || bytes[7] != 0xD5)
      return false;
    if ((uint8_t)(bytes[2] + bytes[3]) != 0xFF)
      return false;
    if ((uint8_t)(bytes[4] + bytes[5]) != 0xFF)
      return false;
  }

  results->decode_type = MITSUBISHI_HEAVY_JINLING;
  results->bits = nbits;
  results->value = data;
  results->command = 0;
  results->address = 0;
  return true;
}
