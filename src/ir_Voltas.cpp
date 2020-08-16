// Copyright 2020 David Conran (crankyoldgit)
/// @file
/// @brief Support for Voltas A/C protocol

// Supports:
//   Brand: Voltas,  Model: 122LZF 4011252 Window AC, Remote model not yet known
//	For a picture of the remote, see https://github.com/crankyoldgit/IRremoteESP8266/issues/1238 

#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

//Constants
const uint16_t kVoltasHdrMark = 0;
const uint16_t kVoltasBitMark = 1026;
const uint16_t kVoltasHdrSpace = 0;
const uint16_t kVoltasOneSpace = 2553;
const uint16_t kVoltasZeroSpace = 554;
const uint16_t kVoltasFreq = 38000;  // Hz. (Guessing the most common frequency.)
const uint16_t kVoltasOverhead = 1;

#if SEND_VOLTAS
// Alternative >64bit function to send VOLTAS messages
// Function should be safe over 64 bits.
/// Send a Voltas formatted message.
/// Status: ALPHA / Untested.
/// @param[in] data An array of bytes containing the IR command.
///                 It is assumed to be in MSB order for this code.
/// e.g.
/// @code
///   uint8_t data[kVoltasStateLength] = {0x33, 0x28, 0x88, 0x1A, 0x3B, 0x3B, 0x3B, 0x11, 0x00, 0x40};
/// @endcode
/// @param[in] nbytes Nr. of bytes of data in the array. (>=kVoltasStateLength)
/// @param[in] repeat Nr. of times the message is to be repeated.
void IRsend::sendVoltas(const uint8_t data[], const uint16_t nbytes, const uint16_t repeat) {
  for (uint16_t r = 0; r <= repeat; r++) {
    uint16_t pos = 0;
    // Data Section #1
    // e.g.
    //   bits = 80; bytes = 10;
    //   *(data + pos) = {0x33, 0x28, 0x88, 0x1A, 0x3B, 0x3B, 0x3B, 0x11, 0x00, 0x40};
    sendGeneric(0, 0,
                kVoltasBitMark, kVoltasOneSpace,
                kVoltasBitMark, kVoltasZeroSpace,
                kVoltasBitMark, kDefaultMessageGap,
                data + pos, 10,  // Bytes
                kVoltasFreq, true, kNoRepeat, kDutyDefault);
    pos += 10;  // Adjust by how many bytes of data we sent
  }
}
#endif  // SEND_VOLTAS

// DANGER: More than 64 bits detected. A uint64_t for 'data' won't work!

#if DECODE_VOLTAS
// Function should be safe over 64 bits.
/// Decode the supplied Voltas message.
/// Status: ALPHA / Untested.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool IRrecv::decodeVoltas(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * nbits + kVoltasOverhead - offset)
    return false;  // Too short a message to match.
  if (strict && nbits != kVoltasBits)
    return false;

  uint16_t pos = 0;
  uint16_t used = 0;

  // Data Section #1
  // e.g.
  //   bits = 80; bytes = 10;
  //   *(results->state + pos) = {0x33, 0x28, 0x88, 0x1A, 0x3B, 0x3B, 0x3B, 0x11, 0x00, 0x40};
  used = matchGeneric(results->rawbuf + offset, results->state + pos,
                      results->rawlen - offset, 80,
                      0, 0,
                      kVoltasBitMark, kVoltasOneSpace,
                      kVoltasBitMark, kVoltasZeroSpace,
                      kVoltasBitMark, kDefaultMessageGap, true);
  if (used == 0) return false;  // We failed to find any data.
  offset += used;  // Adjust for how much of the message we read.
  pos += 10;  // Adjust by how many bytes of data we read

  // Success
  results->decode_type = decode_type_t::VOLTAS;
  results->bits = nbits;
  return true;
}
#endif  // DECODE_VOLTAS