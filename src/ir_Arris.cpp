// Copyright 2021 David Conran
#include "IRrecv.h"
#include "IRsend.h"

/// @file
/// @brief Arris "Manchester code" based protocol.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1595

// Supports:
//   Brand: Arris,  Model: VIP1113M Set-top box

const uint8_t kArrisOverhead = 2;
const uint16_t kArrisHalfClockPeriod = 320;  // uSeconds
const uint16_t kArrisHdrMark = 8 * kArrisHalfClockPeriod;  // uSeconds
const uint16_t kArrisHdrSpace = 6 * kArrisHalfClockPeriod;  // uSeconds
const uint16_t kArrisGapSpace = 18.5 * kArrisHalfClockPeriod;  // uSeconds

#if SEND_ARRIS
/// Send an Arris Manchester Code formatted message.
/// Status: ALPHA / Untested.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of the message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1595
void IRsend::sendArris(const uint64_t data, const uint16_t nbits,
                       const uint16_t repeat) {
  enableIROut(38);
  for (uint16_t r = 0; r <= repeat; r++) {
    // Header (part 1)
    mark(kArrisHdrMark);
    space(kArrisHdrSpace);
    // Header (part 2) + Data + Footer
    sendManchester(kArrisHalfClockPeriod * 2, 0, kArrisHalfClockPeriod,
                   0, kArrisGapSpace, data, nbits);
  }
}
#endif  // SEND_ARRIS

#if DECODE_ARRIS
/// Decode the supplied Arris "Manchester code" message.
///
/// Status: ALPHA / Untested.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1595
bool IRrecv::decodeArris(decode_results *results, uint16_t offset,
                         const uint16_t nbits, const bool strict) {
  if (results->rawlen < nbits + kArrisOverhead - offset)
    return false;  // Too short a message to match.

  // Compliance
  if (strict && nbits != kArrisBits)
    return false;  // Doesn't match our protocol defn.

  // Header (part 1)
  if (!matchMark(results->rawbuf[offset++], kArrisHdrMark)) return false;
  if (!matchSpace(results->rawbuf[offset++], kArrisHdrSpace)) return false;

  // Header (part 2) + Data
  if (!matchManchester(results->rawbuf + offset, &results->value,
                       results->rawlen - offset, nbits,
                       kArrisHalfClockPeriod * 2, 0,
                       kArrisHalfClockPeriod, 0, 0,
                       false, kUseDefTol, kMarkExcess, true, false))
    return false;

  // Success
  results->decode_type = decode_type_t::ARRIS;
  results->bits = nbits;
  results->address = 0;
  results->command = 0;
  return true;
}
#endif  // DECODE_ARRIS
