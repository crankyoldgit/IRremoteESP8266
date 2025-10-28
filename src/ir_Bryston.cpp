// Copyright 2025 Guillaume Giraudon - Colquhoun Audio Laboratories


/// @file
/// @brief Support for Bryston Protocols.
/// @note Currently only tested BP19/BR20 but should work for all Bryston Products.


// Supports:
//   Brand: Bryston

#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"


// Constants
const uint16_t kBrystonTicks = 315;  // Number of bits in a Bryston message.
const uint16_t kBrystonHdrMark = 0;
const uint16_t kBrystonHdrSpace = 0;
const uint16_t kBrystonOneMark = 6 * kBrystonTicks;
const uint16_t kBrystonOneSpace = 1 * kBrystonTicks;
const uint16_t kBrystonZeroMark = 1 * kBrystonTicks;
const uint16_t kBrystonZeroSpace = 6 * kBrystonTicks;
const uint16_t kBrystonMinGap = 0;
const uint16_t kBrystonFooterMark = 0;

#if SEND_BRYSTON
/// Send a Bryston formatted message.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void IRsend::sendBryston(uint64_t data, uint16_t nbits, uint16_t repeat) {
  Serial.printf("Sending Bryston: %016llX\n", data);

  sendGeneric(kBrystonHdrMark, kBrystonHdrSpace, kBrystonOneMark, kBrystonOneSpace,
              kBrystonZeroMark, kBrystonZeroSpace, kBrystonFooterMark, kBrystonMinGap, data,
              nbits, 38, true, repeat, 33);
}
#endif  // SEND_Bryston

#if DECODE_BRYSTON
/// Decode the supplied Bryston message.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
bool IRrecv::decodeBryston(decode_results *results, uint16_t offset,
                         const uint16_t nbits, const bool strict) {
  if (strict && nbits != kBrystonBits)
    return false;  // We expect Bryston to be a certain sized message.

  uint64_t data = 0;

  // Match Header + Data + Footer
  if (!matchGeneric(results->rawbuf + offset, &data,
                    results->rawlen - offset, nbits,
                    kBrystonHdrMark, kBrystonHdrSpace,
                    kBrystonOneMark, kBrystonOneSpace,
                    kBrystonZeroMark, kBrystonZeroSpace,
                    kBrystonFooterMark, kBrystonMinGap, true)) return false;
  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = decode_type_t::BRYSTON;
  results->command = 0;
  results->address = 0;
  return true;
}
#endif  // DECODE_Bryston
