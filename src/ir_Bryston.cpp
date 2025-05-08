/// @file
/// @brief Bryston


// Supports:
//   Brand: Bryston

#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"


// Constants
const uint16_t kBrystonTick = 315;
const uint16_t kBrystonHdrMarkTicks = 1;
const uint16_t kBrystonHdrMark = kBrystonHdrMarkTicks * kBrystonTick;
const uint16_t kBrystonHdrSpaceTicks = 3;
const uint16_t kBrystonHdrSpace = kBrystonHdrSpaceTicks * kBrystonTick;
const uint16_t kBrystonBitMarkTicks = 6;
const uint16_t kBrystonBitMark = kBrystonBitMarkTicks * kBrystonTick;
const uint16_t kBrystonOneSpaceTicks = 1;
const uint16_t kBrystonOneSpace = kBrystonOneSpaceTicks * kBrystonTick;
const uint16_t kBrystonZeroSpaceTicks = 6;
const uint16_t kBrystonZeroSpace = kBrystonZeroSpaceTicks * kBrystonTick;
const uint16_t kBrystonMinGapTicks = 18;
const uint16_t kBrystonMinGap = kBrystonMinGapTicks * kBrystonTick;

#if SEND_BRYSTON
/// Send a Bryston formatted message.
/// Status: STABLE / Working.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void IRsend::sendBryston(uint64_t data, uint16_t nbits, uint16_t repeat) {
  sendGeneric(kBrystonHdrMark, kBrystonHdrSpace, kBrystonBitMark, kBrystonOneSpace,
              kBrystonBitMark, kBrystonZeroSpace, kBrystonBitMark, kBrystonMinGap, data,
              nbits, 38, true, repeat, 33);
}
#endif  // SEND_Bryston

#if DECODE_BRYSTON
/// Decode the supplied Bryston message.
/// Status: STABLE / Working.
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
                    kBrystonBitMark, kBrystonOneSpace,
                    kBrystonBitMark, kBrystonZeroSpace,
                    kBrystonBitMark, kBrystonMinGap, true)) return false;
  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = decode_type_t::BRYSTON;
  results->command = 0;
  results->address = 0;
  return true;
}
#endif  // DECODE_Bryston
