// Copyright 2020 David Conran (crankyoldgit)
/// @file
/// @brief Support for BluestarHeavy protocol

// Supports:
//   Brand: Bluestar,  Model: TODO add device and remote

#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

// WARNING: This probably isn't directly usable. It's a guide only.

// See https://github.com/crankyoldgit/IRremoteESP8266/wiki/Adding-support-for-a-new-IR-protocol
// for details of how to include this in the library.
const uint16_t kBluestarHeavyHdrMark = 4912;
const uint16_t kBluestarHeavyBitMark = 465;
const uint16_t kBluestarHeavyHdrSpace = 5058;
const uint16_t kBluestarHeavyOneSpace = 1548;
const uint16_t kBluestarHeavyZeroSpace = 572;
const uint16_t kBluestarHeavyFreq = 38000;  // Hz. (Guessing the most common frequency.)
//const uint16_t kBluestarHeavyBits = 104;  // Move to IRremoteESP8266.h
//const uint16_t kBluestarHeavyStateLength = 13;  // Move to IRremoteESP8266.h
const uint16_t kBluestarHeavyOverhead = 3;
// DANGER: More than 64 bits detected. A uint64_t for 'data' won't work!


#if SEND_BLUESTARHEAVY
// Alternative >64bit function to send BLUESTARHEAVY messages
// Function should be safe over 64 bits.
/// Send a BluestarHeavy formatted message.
/// Status: ALPHA / Untested.
/// @param[in] data An array of bytes containing the IR command.
///                 It is assumed to be in MSB order for this code.
/// e.g.
/// @code
///   uint8_t data[kBluestarHeavyStateLength] = {0xD5, 0xFE, 0xD7, 0x4F, 0xFA, 0x5F, 0xFA, 0x5F, 0xFF, 0x7F, 0x5C, 0xFD, 0xDC};
/// @endcode
/// @param[in] nbytes Nr. of bytes of data in the array. (>=kBluestarHeavyStateLength)
/// @param[in] repeat Nr. of times the message is to be repeated.
void IRsend::sendBluestarHeavy(const uint8_t data[], const uint16_t nbytes, const uint16_t repeat) {
  for (uint16_t r = 0; r <= repeat; r++) {
    uint16_t pos = 0;
    // Data Section #2
    // e.g.
    //   bits = 104; bytes = 13;
    //   *(data + pos) = {0xD5, 0xFE, 0xD7, 0x4F, 0xFA, 0x5F, 0xFA, 0x5F, 0xFF, 0x7F, 0x5C, 0xFD, 0xDC};
    sendGeneric(kBluestarHeavyHdrMark, kBluestarHeavyHdrSpace,
                kBluestarHeavyBitMark, kBluestarHeavyOneSpace,
                kBluestarHeavyBitMark, kBluestarHeavyZeroSpace,
                kBluestarHeavyHdrMark, kDefaultMessageGap,
                data + pos, 13,  // Bytes
                kBluestarHeavyFreq, true, kNoRepeat, kDutyDefault);
    pos += 13;  // Adjust by how many bytes of data we sent
  }
}
#endif  // SEND_BLUESTARHEAVY


// DANGER: More than 64 bits detected. A uint64_t for 'data' won't work!
// #if DECODE_BLUESTARHEAVY
// // Function should be safe up to 64 bits.
// /// Decode the supplied BluestarHeavy message.
// /// Status: ALPHA / Untested.
// /// @param[in,out] results Ptr to the data to decode & where to store the decode
// /// @param[in] offset The starting index to use when attempting to decode the
// ///   raw data. Typically/Defaults to kStartOffset.
// /// @param[in] nbits The number of data bits to expect.
// /// @param[in] strict Flag indicating if we should perform strict matching.
// /// @return A boolean. True if it can decode it, false if it can't.


// bool IRrecv::decodeBluestarHeavy(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict) {
//   if (results->rawlen < 2 * nbits + kBluestarHeavyOverhead - offset)
//     return false;  // Too short a message to match.
//   if (strict && nbits != kBluestarHeavyBits)
//     return false;

//   uint64_t data = 0;
//   match_result_t data_result;

//   // Header
//   if (!matchMark(results->rawbuf[offset++], kBluestarHeavyHdrMark))
//     return false;
//   if (!matchSpace(results->rawbuf[offset++], kBluestarHeavyHdrSpace))
//     return false;

//   // Data Section #1
//   // e.g. data_result.data = 0xD5FED74FFA5FFA5FFF7F5CFDDC, nbits = 104
//   data_result = matchData(&(results->rawbuf[offset]), 104,
//                           kBluestarHeavyBitMark, kBluestarHeavyOneSpace,
//                           kBluestarHeavyBitMark, kBluestarHeavyZeroSpace);
//   offset += data_result.used;
//   if (data_result.success == false) return false;  // Fail
//   data <<= 104;  // Make room for the new bits of data.
//   data |= data_result.data;

//   // Header
//   if (!matchMark(results->rawbuf[offset++], kBluestarHeavyHdrMark))
//     return false;

//   // Success
//   results->decode_type = decode_type_t::BLUESTARHEAVY;
//   results->bits = nbits;
//   results->value = data;
//   results->command = 0;
//   results->address = 0;
//   return true;
// }
// #endif  // DECODE_BLUESTARHEAVY

#if DECODE_BLUESTARHEAVY
// Function should be safe over 64 bits.
/// Decode the supplied BluestarHeavy message.
/// Status: ALPHA / Untested.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool IRrecv::decodeBluestarHeavy(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * nbits + kBluestarHeavyOverhead - offset)
    return false;  // Too short a message to match.
  if (strict && nbits != kBluestarHeavyBits)
    return false;

  uint16_t pos = 0;
  uint16_t used = 0;

  // Data Section #2
  // e.g.
  //   bits = 104; bytes = 13;
  //   *(results->state + pos) = {0xD5, 0xFE, 0xD7, 0x4F, 0xFA, 0x5F, 0xFA, 0x5F, 0xFF, 0x7F, 0x5C, 0xFD, 0xDC};
  used = matchGeneric(results->rawbuf + offset, results->state + pos,
                      results->rawlen - offset, 104,
                      kBluestarHeavyHdrMark, kBluestarHeavyHdrSpace,
                      kBluestarHeavyBitMark, kBluestarHeavyOneSpace,
                      kBluestarHeavyBitMark, kBluestarHeavyZeroSpace,
                      kBluestarHeavyHdrMark, kDefaultMessageGap, true);
  if (used == 0) return false;  // We failed to find any data.
  offset += used;  // Adjust for how much of the message we read.
  pos += 13;  // Adjust by how many bytes of data we read

  // Success
  results->decode_type = decode_type_t::BLUESTARHEAVY;
  results->bits = nbits;
  return true;
}
#endif  // DECODE_BLUESTARHEAVY
