// Copyright 2020 Fabien G. (AtomBaf)
/// @file
/// @brief Support for the Lumene Projector Screen IR protocols.
/// @see https://en.lumene-screens.com/ecrans-motorises

// Supports:
//   Brand: Lumene,  Model: Embassy

#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

// Constants
const uint16_t kLumeneHdrMark = 400;
const uint16_t kLumeneHdrSpace = 0;
const uint16_t kLumeneOneMark = 800;
const uint16_t kLumeneOneSpace = 0;
const uint16_t kLumeneZeroMark = 0;
const uint16_t kLumeneZeroSpace = 800;
const uint16_t kLumeneFtrMark = 400;
const uint16_t kLumeneGap = 21000;
const uint16_t kLumeneFreq = 38000;

#if SEND_LUMENE
/// Send a Lumene formatted message.
/// Status: ALPHA / Not tested.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void IRsend::sendLumene(const uint64_t data, const uint16_t nbits,
                        const uint16_t repeat) {
  sendGeneric(kLumeneHdrMark, kLumeneHdrSpace,
              kLumeneOneMark, kLumeneOneSpace,
              kLumeneZeroMark, kLumeneZeroSpace,
              kLumeneFtrMark, kLumeneGap,
              data, nbits, kLumeneFreq, false, repeat, kDutyDefault);
}

#endif  // SEND_LUMENE

#if DECODE_LUMENE
/// Decode the supplied Lumene message.
/// Status: ALPHA / Not tested.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return True if it can decode it, false if it can't.
bool IRrecv::decodeLumene(decode_results *results, uint16_t offset,
                          const uint16_t nbits, const bool strict) {
  if (strict && nbits != kLumeneBits)
    return false;  // We expect Lumene to be a certain sized message.

  uint64_t data = 0;

  // Match Header + Data + Footer
  if (!matchGeneric(results->rawbuf + offset, &data,
                    results->rawlen - offset, nbits,
                    kLumeneHdrMark, kLumeneHdrSpace,
                    kLumeneOneMark, kLumeneOneSpace,
                    kLumeneZeroMark, kLumeneZeroSpace,
                    kLumeneFtrMark, kLumeneGap, false)) return false;
  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = decode_type_t::LUMENE;
  results->command = 0;
  results->address = 0;
  return true;
}
#endif  // DECODE_LUMENE
