// Copyright 2021 David Conran (crankyoldgit)

/// @file
/// @brief Support for Truma protocol.
/// This protocol uses mark length bit encoding.
/// @warning Binary data format not yet stablished. May be inverted or reversed.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1440

// Supports:
//   Brand: Truma,  Model: Aventa A/C
//   Brand: Truma,  Model: 40091-86700 remote

#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

// Constants

const uint16_t kTrumaLdrMark = 20200;
const uint16_t kTrumaLdrSpace = 1000;
const uint16_t kTrumaHdrMark = 1800;
const uint16_t kTrumaSpace = 630;
const uint16_t kTrumaOneMark = 600;
const uint16_t kTrumaZeroMark = 1200;
const uint16_t kTrumaFooterMark = kTrumaOneMark;
const uint32_t kTrumaGap = kDefaultMessageGap;  // Just a guess.


#if SEND_TRUMA
/// Send a Truma formatted message.
/// Status: ALPHA / Probably okay. Bit order etc not yet established.
/// @param[in] data The message to be sent.
/// @warning Bit format is not yet extablished. May be invert or order changed
///   in the future.
/// @param[in] nbits The bit size of the message being sent.
/// @param[in] repeat The number of times the message is to be repeated.
void IRsend::sendTruma(const uint64_t data, const uint16_t nbits,
                       const uint16_t repeat) {
  for (uint16_t r = 0; r <= repeat; r++) {
    enableIROut(38000);
    mark(kTrumaLdrMark);
    space(kTrumaLdrSpace);
    sendGeneric(kTrumaHdrMark, kTrumaSpace,   // Header
                kTrumaOneMark, kTrumaSpace,   // Data
                kTrumaZeroMark, kTrumaSpace,
                kTrumaFooterMark, kTrumaGap,  // Footer
                data, nbits, 38, false, 0, kDutyDefault);
  }
}
#endif  // SEND_TRUMA

#if DECODE_TRUMA
/// Decode the supplied Truma message.
/// Status: ALPHA / Probably okay. Bit order etc not yet established.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @warning Bit format is not yet extablished. May be invert or order changed
///   in the future.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect. Typically kTrumaBits.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool IRrecv::decodeTruma(decode_results *results, uint16_t offset,
                         const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * nbits + kHeader - 1 + offset)
    return false;  // Can't possibly be a valid message.
  if (strict && nbits != kTrumaBits)
    return false;  // Not strictly a message.

  // Leader.
  if (!matchMark(results->rawbuf[offset++], kTrumaLdrMark)) return false;
  if (!matchSpace(results->rawbuf[offset++], kTrumaLdrSpace)) return false;

  uint64_t data = 0;
  uint16_t used;
  used = matchGeneric(results->rawbuf + offset, &data,
                      results->rawlen - offset, nbits,
                      kTrumaHdrMark, kTrumaSpace,
                      kTrumaOneMark, kTrumaSpace,
                      kTrumaZeroMark, kTrumaSpace,
                      kTrumaFooterMark, kTrumaGap,
                      true, kUseDefTol, kMarkExcess, false);
  if (!used) return false;

  // Success
  results->value = data;
  results->decode_type = decode_type_t::TRUMA;
  results->bits = nbits;
  results->address = 0;
  results->command = 0;
  return true;
}
#endif  // DECODE_TRUMA
