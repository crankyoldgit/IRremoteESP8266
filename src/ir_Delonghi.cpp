// Copyright 2020 David Conran

#include "ir_Delonghi.h"
#include "IRrecv.h"
#include "IRsend.h"

// Delonghi based protocol.


const uint16_t kDelonghiAcHdrMark = 8984;
const uint16_t kDelonghiAcBitMark = 572;
const uint16_t kDelonghiAcHdrSpace = 4200;
const uint16_t kDelonghiAcOneSpace = 1558;
const uint16_t kDelonghiAcZeroSpace = 510;
const uint32_t kDelonghiAcGap = kDefaultMessageGap;  // A totally made-up guess.
const uint16_t kDelonghiAcFreq = 38000;  // Hz. (Guess: most common frequency.)
const uint16_t kDelonghiAcOverhead = 3;


#if SEND_DELONGHI_AC
// Send an Delonghi AC formatted message.
//
// Args:
//   data:   The message to be sent.
//   nbits:  The number of bits of the message to be sent.
//           Typically kDelonghiAcBits.
//   repeat: The number of times the command is to be repeated.
//
// Status: Alpha / Yet to be tested on a real device.
//
// Ref:
//  https://github.com/crankyoldgit/IRremoteESP8266/issues/1096
void IRsend::sendDelonghiAc(const uint64_t data, const uint16_t nbits,
                            const uint16_t repeat) {
  sendGeneric(kDelonghiAcHdrMark, kDelonghiAcHdrSpace,
              kDelonghiAcBitMark, kDelonghiAcOneSpace,
              kDelonghiAcBitMark, kDelonghiAcZeroSpace,
              kDelonghiAcBitMark, kDelonghiAcGap,
              data, nbits, kDelonghiAcFreq, false,  // LSB First.
              repeat, kDutyDefault);
}
#endif  // SEND_DELONGHI_AC

#if DECODE_DELONGHI_AC
// Decode the supplied DELONGHI_AC message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   offset:  The starting index to use when attempting to decode the raw data.
//            Typically/Defaults to kStartOffset.
//   nbits:   The number of data bits to expect. Typically kDelonghiAcBits.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: BETA / Appears to be working.
//
// Ref:
//   https://github.com/crankyoldgit/IRremoteESP8266/issues/1096
bool IRrecv::decodeDelonghiAc(decode_results *results, uint16_t offset,
                              const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * nbits + kDelonghiAcOverhead - offset)
    return false;  // Too short a message to match.
  if (strict && nbits != kDelonghiAcBits)
    return false;

  uint64_t data = 0;

  // Header + Data + Footer
  if (!matchGeneric(results->rawbuf + offset, &data,
                    results->rawlen - offset, nbits,
                    kDelonghiAcHdrMark, kDelonghiAcHdrSpace,
                    kDelonghiAcBitMark, kDelonghiAcOneSpace,
                    kDelonghiAcBitMark, kDelonghiAcZeroSpace,
                    kDelonghiAcBitMark, kDelonghiAcGap, true,
                    _tolerance, kMarkExcess, false)) return false;

  // Compliance
  // TODO(crankyoldgit): Enable this when written.
  // if (strict && !IRDelonghiAc::validChecksum(data)) return false;

  // Success
  results->decode_type = decode_type_t::DELONGHI_AC;
  results->bits = nbits;
  results->value = data;
  results->command = 0;
  results->address = 0;
  return true;
}
#endif  // DECODE_DELONGHI_AC
