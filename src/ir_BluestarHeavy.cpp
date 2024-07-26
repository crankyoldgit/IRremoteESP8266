// Copyright 2024 Harsh Bhosale (harshbhosale01)
/// @file
/// @brief Support for BluestarHeavy protocol

// Supports:
// Brand: Bluestar,  Model: TODO add device and remote

#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

const uint16_t kBluestarHeavyHdrMark = 4912;
const uint16_t kBluestarHeavyBitMark = 465;
const uint16_t kBluestarHeavyHdrSpace = 5058;
const uint16_t kBluestarHeavyOneSpace = 1548;
const uint16_t kBluestarHeavyZeroSpace = 572;
const uint16_t kBluestarHeavyFreq = 38000;  
const uint16_t kBluestarHeavyOverhead = 3;

#if SEND_BLUESTARHEAVY
/// Send a BluestarHeavy formatted message.
/// Status: BETA / Tested.
/// @param[in] data An array of bytes containing the IR command.
///                 It is assumed to be in MSB order for this code.
/// e.g.
/// @code
///   uint8_t data[kBluestarHeavyStateLength] = {0xD5, 0xFE, 0xD7, 0x4F, 0xFA, 0x5F, 0xFA, 0x5F, 0xFF, 0x7F, 0x5C, 0xFD, 0xDC};
/// @endcode
/// @param[in] nbytes Nr. of bytes of data in the array. (>=kBluestarHeavyStateLength)
/// @param[in] repeat Nr. of times the message is to be repeated.
void IRsend::sendBluestarHeavy(const uint8_t data[], const uint16_t nbytes, const uint16_t repeat) {
  sendGeneric(kBluestarHeavyHdrMark, kBluestarHeavyHdrSpace,
                kBluestarHeavyBitMark, kBluestarHeavyOneSpace,
                kBluestarHeavyBitMark, kBluestarHeavyZeroSpace,
                kBluestarHeavyHdrMark, kDefaultMessageGap,
                data, nbytes,  // Bytes
                kBluestarHeavyFreq, true, repeat, kDutyDefault);
}
#endif  // SEND_BLUESTARHEAVY

#if DECODE_BLUESTARHEAVY
/// Decode the supplied BluestarHeavy message.
/// Status: BETA / Tested.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool IRrecv::decodeBluestarHeavy(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict) {
  if (strict && nbits != kBluestarHeavyBits)
    return false;

  uint16_t used = 0;

  used = matchGeneric(results->rawbuf + offset, results->state,
                      results->rawlen - offset, nbits,
                      kBluestarHeavyHdrMark, kBluestarHeavyHdrSpace,
                      kBluestarHeavyBitMark, kBluestarHeavyOneSpace,
                      kBluestarHeavyBitMark, kBluestarHeavyZeroSpace,
                      kBluestarHeavyHdrMark, kDefaultMessageGap, true);
  if (used == 0) return false;  // We failed to find any data.

  // Success
  results->decode_type = decode_type_t::BLUESTARHEAVY;
  results->bits = nbits;
  return true;
}
#endif  // DECODE_BLUESTARHEAVY
