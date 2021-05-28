// Copyright 2021 David Conran (crankyoldgit)
/// @file
/// @brief Support for the Teknopoint protocol
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1486

// Supports:
//   Brand: Teknopoint,  Model: Allegro SSA-09H A/C
//   Brand: Teknopoint,  Model: GZ-055B-E1 remote

#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

// Protocol timings
const uint16_t kTeknopointHdrMark = 3614;
const uint16_t kTeknopointBitMark = 439;
const uint16_t kTeknopointHdrSpace = 1610;
const uint16_t kTeknopointOneSpace = 1238;
const uint16_t kTeknopointZeroSpace = 567;
const uint16_t kTeknopointFreq = 38000;  // Hz. (Guess Only)
const uint16_t kTeknopointOverhead = 3;

#if SEND_TEKNOPOINT
/// Send a Teknopoint formatted message.
/// Status: BETA / Probably works however the bit order is not yet determined.
/// @param[in] data An array of bytes containing the IR command.
///                 It is assumed to be in MSB order for this code.
/// e.g.
/// @code
///   uint8_t data[kTeknopointStateLength] = {
///       0xC4, 0xD3, 0x64, 0x80, 0x00, 0x24, 0xC0,
///       0xF0, 0x10, 0x00, 0x00, 0x00, 0x00, 0xCA};
/// @endcode
/// @param[in] nbytes Nr. of bytes of data in the array.
/// @param[in] repeat Nr. of times the message is to be repeated.
void IRsend::sendTeknopoint(const uint8_t data[], const uint16_t nbytes,
                            const uint16_t repeat) {
  sendGeneric(kTeknopointHdrMark, kTeknopointHdrSpace,
              kTeknopointBitMark, kTeknopointOneSpace,
              kTeknopointBitMark, kTeknopointZeroSpace,
              kTeknopointBitMark, kDefaultMessageGap,
              data, nbytes,  // Bytes
              kTeknopointFreq, true, repeat, kDutyDefault);
}
#endif  // SEND_TEKNOPOINT

#if DECODE_TEKNOPOINT
/// Decode the supplied Teknopoint message.
/// Status: Alpha / Probably works however the bit order is not yet determined.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool IRrecv::decodeTeknopoint(decode_results *results, uint16_t offset,
                              const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * nbits + kHeader + kFooter - 1 - offset)
    return false;  // Too short a message to match.
  if (strict && nbits != kTeknopointBits)
    return false;

  if (!matchGeneric(results->rawbuf + offset, results->state,
                    results->rawlen - offset, nbits,
                    kTeknopointHdrMark, kTeknopointHdrSpace,
                    kTeknopointBitMark, kTeknopointOneSpace,
                    kTeknopointBitMark, kTeknopointZeroSpace,
                    kTeknopointBitMark, kDefaultMessageGap, true)) return false;
  // Success
  results->decode_type = decode_type_t::TEKNOPOINT;
  results->bits = nbits;
  return true;
}
#endif  // DECODE_TEKNOPOINT
