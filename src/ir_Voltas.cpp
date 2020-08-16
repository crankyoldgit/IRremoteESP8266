// Copyright 2020 David Conran (crankyoldgit)
// Copyright 2020 manj9501
/// @file
/// @brief Support for Voltas A/C protocol
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1238

// Supports:
//   Brand: Voltas,  Model: 122LZF 4011252 Window A/C

#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

// Constants
const uint16_t kVoltasBitMark = 1026;   ///< uSeconds.
const uint16_t kVoltasOneSpace = 2553;  ///< uSeconds.
const uint16_t kVoltasZeroSpace = 554;  ///< uSeconds.
const uint16_t kVoltasFreq = 38000;     ///< Hz.

#if SEND_VOLTAS
/// Send a Voltas formatted message.
/// Status: ALPHA / Untested.
/// @param[in] data An array of bytes containing the IR command.
///                 It is assumed to be in MSB order for this code.
/// e.g.
/// @code
///   uint8_t data[kVoltasStateLength] = {0x33, 0x28, 0x88, 0x1A, 0x3B, 0x3B,
///                                       0x3B, 0x11, 0x00, 0x40};
/// @endcode
/// @param[in] nbytes Nr. of bytes of data in the array. (>=kVoltasStateLength)
/// @param[in] repeat Nr. of times the message is to be repeated.
void IRsend::sendVoltas(const uint8_t data[], const uint16_t nbytes,
                        const uint16_t repeat) {
  sendGeneric(0, 0,
              kVoltasBitMark, kVoltasOneSpace,
              kVoltasBitMark, kVoltasZeroSpace,
              kVoltasBitMark, kDefaultMessageGap,
              data, nbytes,
              kVoltasFreq, true, repeat, kDutyDefault);
}
#endif  // SEND_VOLTAS

#if DECODE_VOLTAS
/// Decode the supplied Voltas message.
/// Status: ALPHA / Untested.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool IRrecv::decodeVoltas(decode_results *results, uint16_t offset,
                          const uint16_t nbits, const bool strict) {
  if (strict && nbits != kVoltasBits) return false;

  // Data + Footer
  if (!matchGeneric(results->rawbuf + offset, results->state,
                    results->rawlen - offset, nbits,
                    0, 0,  // No header
                    kVoltasBitMark, kVoltasOneSpace,
                    kVoltasBitMark, kVoltasZeroSpace,
                    kVoltasBitMark, kDefaultMessageGap, true)) return false;

  // Success
  results->decode_type = decode_type_t::VOLTAS;
  results->bits = nbits;
  return true;
}
#endif  // DECODE_VOLTAS
