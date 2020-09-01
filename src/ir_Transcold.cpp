// Copyright 2020 David Conran (crankyoldgit)
/// @file
/// @brief Support for Transcold protocol.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1256

// Supports:
//   Brand: Transcold,  Model: M1-F-NO-6 A/C

#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

// Constants
const uint16_t kTranscoldHdrMark =   5944;  ///< uSeconds.
const uint16_t kTranscoldBitMark =    555;  ///< uSeconds.
const uint16_t kTranscoldHdrSpace =  7563;  ///< uSeconds.
const uint16_t kTranscoldOneSpace =  3556;  ///< uSeconds.
const uint16_t kTranscoldZeroSpace = 1526;  ///< uSeconds.
const uint16_t kTranscoldFreq =     38000;  ///< Hz.

#if SEND_TRANSCOLD
/// Send a Transcold formatted message.
/// Status: BETA / Probably works, needs to be tested on a real device.
/// @note Data bit ordering not yet confirmed. MSBF at present.
/// @param[in] data containing the IR command.
/// @param[in] nbits Nr. of bits to send. usually kTranscoldBits
/// @param[in] repeat Nr. of times the message is to be repeated.
void IRsend::sendTranscold(const uint64_t data, const uint16_t nbits,
                           const uint16_t repeat) {
  for (uint16_t r = 0; r <= repeat; r++) {
    sendGeneric(kTranscoldHdrMark, kTranscoldHdrSpace,   // Header
                kTranscoldBitMark, kTranscoldOneSpace,   // Data
                kTranscoldBitMark, kTranscoldZeroSpace,
                kTranscoldBitMark, kTranscoldHdrSpace,   // Footer (1 of 2).
                data, nbits,                             // Payload
                kTranscoldFreq, true, 0,   // Repeat handled by outer loop.
                kDutyDefault);
    // Footer (2 of 2)
    mark(kTranscoldBitMark);
    space(kDefaultMessageGap);  // A guess of the gap between messages.
  }
}
#endif  // SEND_TRANSCOLD

#if DECODE_TRANSCOLD
/// Decode the supplied Transcold message.
/// Status: BETA / Probably works.
/// @note Data bit ordering not yet confirmed. MSBF at present.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool IRrecv::decodeTranscold(decode_results *results, uint16_t offset,
                             const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * nbits + kHeader + (2 * kFooter) - offset)
    return false;  // Too short a message to match.
  if (strict && nbits != kTranscoldBits)
    return false;

  uint64_t data = 0;

  // Match Header + Data + Footer (1 of 2)
  uint16_t used = matchGeneric(results->rawbuf + offset, &data,
                               results->rawlen - offset, nbits,
                               // Header
                               kTranscoldHdrMark, kTranscoldHdrSpace,
                               // Data
                               kTranscoldBitMark, kTranscoldOneSpace,
                               kTranscoldBitMark, kTranscoldZeroSpace,
                               // Footer (1 of 2)
                               kTranscoldBitMark, kTranscoldHdrSpace,
                               false, _tolerance, 0, true);
  if (!used) return false;  // Didn't fully match.
  offset += used;

  // Footer (2 of 2)
  if (!matchMark(results->rawbuf[offset++], kTranscoldBitMark))
    return false;
  if (offset <= results->rawlen &&
      !matchAtLeast(results->rawbuf[offset], kDefaultMessageGap))
    return false;

  // Success
  results->decode_type = decode_type_t::TRANSCOLD;
  results->bits = nbits;
  results->value = data;
  results->command = 0;
  results->address = 0;
  return true;
}
#endif  // DECODE_TRANSCOLD
