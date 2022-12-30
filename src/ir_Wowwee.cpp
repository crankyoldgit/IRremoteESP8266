// Copyright 2022 David Conran

/// @file
/// @brief Support for WowWee RoboRapter protocol
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues1938

// Supports:
//   Brand: WowWee,  Model: RoboRapter-X

#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

// Constants
const uint16_t kWowweeHdrMark   = 6684;
const uint16_t kWowweeHdrSpace  = 723;
const uint16_t kWowweeBitMark   = 912;
const uint16_t kWowweeOneSpace  = 3259;
const uint16_t kWowweeZeroSpace = kWowweeHdrSpace;
const uint16_t kWowweeFreq = 38000;  // Hz. (Just a guess)


#if SEND_WOWWEE
/// Send a WowWee formatted message.
/// Status: BETA / Untested on a real device.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void IRsend::sendWowwee(uint64_t data, uint16_t nbits, uint16_t repeat) {
  sendGeneric(kWowweeHdrMark, kWowweeHdrSpace,
              kWowweeBitMark, kWowweeOneSpace,
              kWowweeBitMark, kWowweeZeroSpace,
              kWowweeBitMark, kDefaultMessageGap, data,
              nbits, kWowweeFreq, true, repeat, 33);
}
#endif  // SEND_WOWWEE

#if DECODE_WOWWEE
/// Decode the supplied WowWee message.
/// Status: BETA / Untested on a real device.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
bool IRrecv::decodeWowwee(decode_results *results, uint16_t offset,
                         const uint16_t nbits, const bool strict) {
  if (strict && nbits != kWowweeBits)
    return false;  // We expect Wowwee to be a certain sized message.

  uint64_t data = 0;

  // Match Header + Data + Footer
  if (!matchGeneric(results->rawbuf + offset, &data,
                    results->rawlen - offset, nbits,
                    kWowweeHdrMark, kWowweeHdrSpace,
                    kWowweeBitMark, kWowweeOneSpace,
                    kWowweeBitMark, kWowweeZeroSpace,
                    kWowweeBitMark, kDefaultMessageGap, true)) return false;
  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = WOWWEE;
  results->command = 0;
  results->address = 0;
  return true;
}
#endif  // DECODE_WOWWEE
