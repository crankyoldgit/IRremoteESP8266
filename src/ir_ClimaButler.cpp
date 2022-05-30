// Copyright 2020 David Conran (crankyoldgit)
/// @file
/// @brief Support for Clima-Butler protocol
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1812

// Supports:
//   Brand: Clima-Butler,  Model: AR-715 remote
//   Brand: Clima-Butler,  Model: RCS-SD43UWI A/C

#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

// WARNING: This probably isn't directly usable. It's a guide only.

// See https://github.com/crankyoldgit/IRremoteESP8266/wiki/Adding-support-for-a-new-IR-protocol
// for details of how to include this in the library.
const uint16_t kClimaButlerHdrMark = 0;
const uint16_t kClimaButlerBitMark = 511;
const uint16_t kClimaButlerHdrSpace = 3492;
const uint16_t kClimaButlerOneSpace = 1540;
const uint16_t kClimaButlerZeroSpace = 548;
const uint16_t kClimaButlerFreq = 38000;  // Hz. (Guessing the most common frequency.)
const uint16_t kClimaButlerOverhead = 5;

#if SEND_CLIMABUTLER
// Function should be safe up to 64 bits.
/// Send a ClimaButler formatted message.
/// Status: ALPHA / Untested.
/// @param[in] data containing the IR command.
/// @param[in] nbits Nr. of bits to send. usually kClimaButlerBits
/// @param[in] repeat Nr. of times the message is to be repeated.
void IRsend::sendClimaButler(const uint64_t data, const uint16_t nbits, const uint16_t repeat) {
  enableIROut(kClimaButlerFreq);
  for (uint16_t r = 0; r <= repeat; r++) {
    // Header + Data
    sendGeneric(kClimaButlerBitMark, kClimaButlerHdrSpace,
                kClimaButlerBitMark, kClimaButlerOneSpace,
                kClimaButlerBitMark, kClimaButlerZeroSpace,
                kClimaButlerBitMark, kClimaButlerHdrSpace,
                data, nbits, kClimaButlerFreq, true, 0, kDutyDefault);
    // Footer
    mark(kClimaButlerBitMark);
    space(kDefaultMessageGap);  // A 100% made up guess of the gap between messages.
  }
}
#endif  // SEND_CLIMABUTLER

#if DECODE_CLIMABUTLER
// Function should be safe up to 64 bits.
/// Decode the supplied ClimaButler message.
/// Status: ALPHA / Untested.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool IRrecv::decodeClimaButler(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * nbits + kClimaButlerOverhead - offset)
    return false;  // Too short a message to match.
  if (strict && nbits != kClimaButlerBits)
    return false;

  uint64_t data = 0;
  match_result_t data_result;
  if (!matchMark(results->rawbuf[offset++], kClimaButlerBitMark))
    return false;
  if (!matchSpace(results->rawbuf[offset++], kClimaButlerHdrSpace))
    return false;

  // Data Section #1
  // e.g. data_result.data = 0x830000005880F, nbits = 52
  data_result = matchData(&(results->rawbuf[offset]), 52,
                          kClimaButlerBitMark, kClimaButlerOneSpace,
                          kClimaButlerBitMark, kClimaButlerZeroSpace);
  offset += data_result.used;
  if (data_result.success == false) return false;  // Fail
  data <<= 52;  // Make room for the new bits of data.
  data |= data_result.data;

  // Footer
  if (!matchMark(results->rawbuf[offset++], kClimaButlerBitMark))
    return false;
  if (!matchSpace(results->rawbuf[offset++], kClimaButlerHdrSpace))
    return false;

  // Success
  results->decode_type = decode_type_t::CLIMABUTLER;
  results->bits = nbits;
  results->value = data;
  results->command = 0;
  results->address = 0;
  return true;
}
#endif  // DECODE_CLIMABUTLER
