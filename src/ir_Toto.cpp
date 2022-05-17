// Copyright 2022 David Conran (crankyoldgit)
/// @file
/// @brief Support for the Toto Toilet IR protocols.
/// @see https://www.d-resi.jp/dt/nishi-shinjuku/limited/imgs/pdf/book6.pdf
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1806

// Supports:
//   Brand: Toto,  Model: Washlet Toilet NJ

#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

// Constants
const uint16_t kTotoHdrMark = 6197;
const uint16_t kTotoHdrSpace = 2754;
const uint16_t kTotoBitMark = 600;
const uint16_t kTotoOneSpace = 1634;
const uint16_t kTotoZeroSpace = 516;
const uint16_t kTotoGap = 38000;
const uint16_t kTotoSpecialGap = 42482;

#if SEND_TOTO
/// Send a Toto Toilet formatted message.
/// Status: ALPHA / Untested.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1806
void IRsend::sendToto(const uint64_t data, const uint16_t nbits,
                      const uint16_t repeat) {
  sendGeneric(kTotoHdrMark, kTotoHdrSpace,
              kTotoBitMark, kTotoOneSpace,
              kTotoBitMark, kTotoZeroSpace,
              kTotoBitMark, kTotoGap,
              data, nbits, 38, true, repeat, kDutyDefault);
}
#endif  // SEND_TOTO

#if DECODE_TOTO
/// Decode the supplied Toto Toilet message.
/// Status: ALPHA / Untested.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return True if it can decode it, false if it can't.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1806
bool IRrecv::decodeToto(decode_results *results, uint16_t offset,
                        const uint16_t nbits, const bool strict) {
  if (strict && nbits != kTotoBits)
    return false;  // We expect Toto to be a certain sized message.

  if (results->rawlen < (2 * nbits + kHeader + kFooter) *
                        (kTotoDefaultRepeat + 1) - 1 + offset)
    return false;  // We don't have enough entries to possibly match.


  uint64_t data = 0;
  uint16_t used = 0;

  for (uint16_t r = 0; r <= kTotoDefaultRepeat; r++) {  // We expect a repeat.
    // Match Header + Data + Footer + Gap
    used = matchGeneric(results->rawbuf + offset, &data,
                        results->rawlen - offset, nbits,
                        kTotoHdrMark, kTotoHdrSpace,
                        kTotoBitMark, kTotoOneSpace,
                        kTotoBitMark, kTotoZeroSpace,
                        kTotoBitMark, kTotoGap, true);
    if (!used) return false;  // Didn't match, so fail.
    offset += used;
    if (r && data != results->value) return false;  // The repeat didn't match.
    results->value = data;
  }
  // Success
  results->bits = nbits;
  results->decode_type = decode_type_t::TOTO;
  results->command = 0;
  results->address = 0;
  return true;
}
#endif  // DECODE_TOTO
