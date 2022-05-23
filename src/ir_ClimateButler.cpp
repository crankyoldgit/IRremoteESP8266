// Copyright 2020 David Conran (crankyoldgit)
/// @file
/// @brief Support for ClimateButler protocol

// Supports:
//   Brand: ClimateButler,  Model: AR-715 remote
//   Brand: ClimateButler,  Model: RCS-SD43UWI A/C

#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

// WARNING: This probably isn't directly usable. It's a guide only.

// See https://github.com/crankyoldgit/IRremoteESP8266/wiki/Adding-support-for-a-new-IR-protocol
// for details of how to include this in the library.
const uint16_t kClimateButlerHdrMark = 0;
const uint16_t kClimateButlerBitMark = 511;
const uint16_t kClimateButlerHdrSpace = 3492;
const uint16_t kClimateButlerOneSpace = 1540;
const uint16_t kClimateButlerZeroSpace = 548;
const uint16_t kClimateButlerFreq = 38000;  // Hz. (Guessing the most common frequency.)
const uint16_t kClimateButlerOverhead = 5;

#if SEND_CLIMATEBUTLER
// Function should be safe up to 64 bits.
/// Send a ClimateButler formatted message.
/// Status: ALPHA / Untested.
/// @param[in] data containing the IR command.
/// @param[in] nbits Nr. of bits to send. usually kClimateButlerBits
/// @param[in] repeat Nr. of times the message is to be repeated.
void IRsend::sendClimateButler(const uint64_t data, const uint16_t nbits, const uint16_t repeat) {
  enableIROut(kClimateButlerFreq);
  for (uint16_t r = 0; r <= repeat; r++) {
    // Header + Data
    sendGeneric(kClimateButlerBitMark, kClimateButlerHdrSpace,
                kClimateButlerBitMark, kClimateButlerOneSpace,
                kClimateButlerBitMark, kClimateButlerZeroSpace,
                kClimateButlerBitMark, kClimateButlerHdrSpace,
                data, nbits, kClimateButlerFreq, true, 0, kDutyDefault);
    // Footer
    mark(kClimateButlerBitMark);
    space(kDefaultMessageGap);  // A 100% made up guess of the gap between messages.
  }
}
#endif  // SEND_CLIMATEBUTLER

#if DECODE_CLIMATEBUTLER
// Function should be safe up to 64 bits.
/// Decode the supplied ClimateButler message.
/// Status: ALPHA / Untested.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool IRrecv::decodeClimateButler(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * nbits + kClimateButlerOverhead - offset)
    return false;  // Too short a message to match.
  if (strict && nbits != kClimateButlerBits)
    return false;

  uint64_t data = 0;
  match_result_t data_result;
  if (!matchMark(results->rawbuf[offset++], kClimateButlerBitMark))
    return false;
  if (!matchSpace(results->rawbuf[offset++], kClimateButlerHdrSpace))
    return false;

  // Data Section #1
  // e.g. data_result.data = 0x830000005880F, nbits = 52
  data_result = matchData(&(results->rawbuf[offset]), 52,
                          kClimateButlerBitMark, kClimateButlerOneSpace,
                          kClimateButlerBitMark, kClimateButlerZeroSpace);
  offset += data_result.used;
  if (data_result.success == false) return false;  // Fail
  data <<= 52;  // Make room for the new bits of data.
  data |= data_result.data;

  // Footer
  if (!matchMark(results->rawbuf[offset++], kClimateButlerBitMark))
    return false;
  if (!matchSpace(results->rawbuf[offset++], kClimateButlerHdrSpace))
    return false;

  // Success
  results->decode_type = decode_type_t::CLIMATEBUTLER;
  results->bits = nbits;
  results->value = data;
  results->command = 0;
  results->address = 0;
  return true;
}
#endif  // DECODE_CLIMATEBUTLER