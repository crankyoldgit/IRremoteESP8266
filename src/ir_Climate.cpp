// Copyright 2020 David Conran (crankyoldgit)
/// @file
/// @brief Support for TestExample protocol

// Supports:
//   Brand: TestExample,  Model: TODO add device and remote

#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

// WARNING: This probably isn't directly usable. It's a guide only.

// See https://github.com/crankyoldgit/IRremoteESP8266/wiki/Adding-support-for-a-new-IR-protocol
// for details of how to include this in the library.
const uint16_t kTestExampleHdrMark = 0;
const uint16_t kTestExampleBitMark = 511;
const uint16_t kTestExampleHdrSpace = 3492;
const uint16_t kTestExampleOneSpace = 1540;
const uint16_t kTestExampleZeroSpace = 548;
const uint16_t kTestExampleFreq = 38000;  // Hz. (Guessing the most common frequency.)
const uint16_t kTestExampleOverhead = 5;

#if SEND_TESTEXAMPLE
// Function should be safe up to 64 bits.
/// Send a TestExample formatted message.
/// Status: ALPHA / Untested.
/// @param[in] data containing the IR command.
/// @param[in] nbits Nr. of bits to send. usually kTestExampleBits
/// @param[in] repeat Nr. of times the message is to be repeated.
void IRsend::sendTestExample(const uint64_t data, const uint16_t nbits, const uint16_t repeat) {
  enableIROut(kTestExampleFreq);
  for (uint16_t r = 0; r <= repeat; r++) {
    uint64_t send_data = data;
    space(kTestExampleHdrSpace);
    // Data Section #1
    // e.g. data = 0x830000005880F, nbits = 52
    sendData(kTestExampleBitMark, kTestExampleOneSpace, kTestExampleBitMark, kTestExampleZeroSpace, send_data, 52, true);
    send_data >>= 52;
    // Footer
    mark(kTestExampleBitMark);
    space(kTestExampleHdrSpace);
    space(kDefaultMessageGap);  // A 100% made up guess of the gap between messages.
  }
}
#endif  // SEND_TESTEXAMPLE

#if DECODE_TESTEXAMPLE
// Function should be safe up to 64 bits.
/// Decode the supplied TestExample message.
/// Status: ALPHA / Untested.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool IRrecv::decodeTestExample(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * nbits + kTestExampleOverhead - offset)
    return false;  // Too short a message to match.
  if (strict && nbits != kTestExampleBits)
    return false;

  uint64_t data = 0;
  match_result_t data_result;
  if (!matchSpace(results->rawbuf[offset++], kTestExampleHdrSpace))
    return false;

  // Data Section #1
  // e.g. data_result.data = 0x830000005880F, nbits = 52
  data_result = matchData(&(results->rawbuf[offset]), 52,
                          kTestExampleBitMark, kTestExampleOneSpace,
                          kTestExampleBitMark, kTestExampleZeroSpace);
  offset += data_result.used;
  if (data_result.success == false) return false;  // Fail
  data <<= 52;  // Make room for the new bits of data.
  data |= data_result.data;

  // Footer
  if (!matchMark(results->rawbuf[offset++], kTestExampleBitMark))
    return false;
  if (!matchSpace(results->rawbuf[offset++], kTestExampleHdrSpace))
    return false;

  // Success
  results->decode_type = decode_type_t::TESTEXAMPLE;
  results->bits = nbits;
  results->value = data;
  results->command = 0;
  results->address = 0;
  return true;
}
#endif  // DECODE_TESTEXAMPLE