// Copyright 2018 David Conran
//
// Code to emulate Hitachi protocol compatible devices.
// Should be compatible with:
// * Hitachi RAS-35THA6 remote
//

#include <algorithm>
#ifndef ARDUINO
#include <string>
#endif
#include "IRremoteESP8266.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

//              HH   HH IIIII TTTTTTT   AAA    CCCCC  HH   HH IIIII
//              HH   HH  III    TTT    AAAAA  CC    C HH   HH  III
//              HHHHHHH  III    TTT   AA   AA CC      HHHHHHH  III
//              HH   HH  III    TTT   AAAAAAA CC    C HH   HH  III
//              HH   HH IIIII   TTT   AA   AA  CCCCC  HH   HH IIIII

// Constants
// Ref: https://github.com/markszabo/IRremoteESP8266/issues/417
const uint16_t kHitachiACHdrMark = 3300;
const uint16_t kHitachiACHdrSpace = 1700;
const uint16_t kHitachiAC1HdrMark = 3400;
const uint16_t kHitachiAC1HdrSpace = 3400;
const uint16_t kHitachiACBitMark = 400;
const uint16_t kHitachiACOneSpace = 1250;
const uint16_t kHitachiACZeroSpace = 500;
const uint32_t kHitachiACMinGap = 100000;  // Completely made up value.

#if (SEND_HITACHI_AC || SEND_HITACHI_AC2)
// Send a Hitachi A/C message.
//
// Args:
//   data: An array of bytes containing the IR command.
//   nbytes: Nr. of bytes of data in the array. (>=kHitachiACStateLength)
//   repeat: Nr. of times the message is to be repeated. (Default = 0).
//
// Status: ALPHA / Untested.
//
// Ref:
//   https://github.com/markszabo/IRremoteESP8266/issues/417
void IRsend::sendHitachiAC(unsigned char data[], uint16_t nbytes,
                           uint16_t repeat) {
  if (nbytes < kHitachiACStateLength)
    return;  // Not enough bytes to send a proper message.
  sendGeneric(kHitachiACHdrMark, kHitachiACHdrSpace,
              kHitachiACBitMark, kHitachiACOneSpace,
              kHitachiACBitMark, kHitachiACZeroSpace,
              kHitachiACBitMark, kHitachiACMinGap,
              data, nbytes, 38, true, repeat, 50);
}
#endif  // (SEND_HITACHI_AC || SEND_HITACHI_AC2)

#if SEND_HITACHI_AC1
// Send a Hitachi A/C 13-byte message.
//
// For devices:
//  Hitachi A/C Series VI (Circa 2007) / Remote: LT0541-HTA
//
// Args:
//   data: An array of bytes containing the IR command.
//   nbytes: Nr. of bytes of data in the array. (>=kHitachiAC1StateLength)
//   repeat: Nr. of times the message is to be repeated. (Default = 0).
//
// Status: BETA / Appears to work.
//
// Ref:
//   https://github.com/markszabo/IRremoteESP8266/issues/453
//   Basically the same as sendHitatchiAC() except different size and header.
void IRsend::sendHitachiAC1(unsigned char data[], uint16_t nbytes,
                            uint16_t repeat) {
  if (nbytes < kHitachiAC1StateLength)
    return;  // Not enough bytes to send a proper message.
  sendGeneric(kHitachiAC1HdrMark, kHitachiAC1HdrSpace,
              kHitachiACBitMark, kHitachiACOneSpace,
              kHitachiACBitMark, kHitachiACZeroSpace,
              kHitachiACBitMark, kHitachiACMinGap,
              data, nbytes, 38, true, repeat, 50);
}
#endif  // SEND_HITACHI_AC1

#if SEND_HITACHI_AC2
// Send a Hitachi A/C 53-byte message.
//
// For devices:
//  Hitachi A/C Series VI (Circa 2007) / Remote: LT0541-HTA
//
// Args:
//   data: An array of bytes containing the IR command.
//   nbytes: Nr. of bytes of data in the array. (>=kHitachiAC2StateLength)
//   repeat: Nr. of times the message is to be repeated. (Default = 0).
//
// Status: BETA / Appears to work.
//
// Ref:
//   https://github.com/markszabo/IRremoteESP8266/issues/417
//   Basically the same as sendHitatchiAC() except different size.
void IRsend::sendHitachiAC2(unsigned char data[], uint16_t nbytes,
                            uint16_t repeat) {
  if (nbytes < kHitachiAC2StateLength)
    return;  // Not enough bytes to send a proper message.
  sendHitachiAC(data, nbytes, repeat);
}
#endif  // SEND_HITACHI_AC2

#if (DECODE_HITACHI_AC || DECODE_HITACHI_AC1 || DECODE_HITACHI_AC2)
// Decode the supplied Hitachi A/C message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect.
//            Typically kHitachiACBits, kHitachiAC1Bits, kHitachiAC2Bits
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: ALPHA / Untested.
//
// Supported devices:
//  Hitachi A/C Series VI (Circa 2007) / Remote: LT0541-HTA
//
// Ref:
//   https://github.com/markszabo/IRremoteESP8266/issues/417
//   https://github.com/markszabo/IRremoteESP8266/issues/453
bool IRrecv::decodeHitachiAC(decode_results *results, uint16_t nbits,
                             bool strict) {
  const uint8_t kTolerance = 30;
  if (results->rawlen < 2 * nbits + kHeader + kFooter - 1)
    return false;  // Can't possibly be a valid HitachiAC message.
  if (strict) {
    switch (nbits) {
      case kHitachiACBits:
      case kHitachiAC1Bits:
      case kHitachiAC2Bits:
        break;  // Okay to continue.
      default:
        return false;  // Not strictly a Hitachi message.
    }
  }
  uint16_t offset = kStartOffset;
  uint16_t dataBitsSoFar = 0;
  match_result_t data_result;

  // Header
  if (nbits == kHitachiAC1Bits) {
    if (!matchMark(results->rawbuf[offset++], kHitachiAC1HdrMark, kTolerance))
      return false;
    if (!matchSpace(results->rawbuf[offset++], kHitachiAC1HdrSpace,
                    kTolerance))
      return false;
  } else {  // Everything else.
    if (!matchMark(results->rawbuf[offset++], kHitachiACHdrMark, kTolerance))
      return false;
    if (!matchSpace(results->rawbuf[offset++], kHitachiACHdrSpace,
                    kTolerance))
      return false;
  }
  // Data
  // Keep reading bytes until we either run out of message or state to fill.
  for (uint16_t i = 0;
      offset <= results->rawlen - 16 && i < nbits / 8;
      i++, dataBitsSoFar += 8, offset += data_result.used) {
    data_result = matchData(&(results->rawbuf[offset]), 8,
                            kHitachiACBitMark,
                            kHitachiACOneSpace,
                            kHitachiACBitMark,
                            kHitachiACZeroSpace,
                            kTolerance);
    if (data_result.success == false)  break;  // Fail
    results->state[i] = (uint8_t) data_result.data;
  }

  // Footer
  if (!matchMark(results->rawbuf[offset++], kHitachiACBitMark, kTolerance))
    return false;
  if (offset <= results->rawlen &&
      !matchAtLeast(results->rawbuf[offset], kHitachiACMinGap, kTolerance))
    return false;

  // Compliance
  if (strict) {
    // Re-check we got the correct size/length due to the way we read the data.
    switch (dataBitsSoFar / 8) {
      case kHitachiACStateLength:
      case kHitachiAC1StateLength:
      case kHitachiAC2StateLength:
        break;  // Continue
      default:
        return false;
    }
  }

  // Success
  switch (dataBitsSoFar) {
    case kHitachiAC1Bits:
      results->decode_type = HITACHI_AC1;
      break;
    case kHitachiAC2Bits:
      results->decode_type = HITACHI_AC2;
      break;
    case kHitachiACBits:
    default:
      results->decode_type = HITACHI_AC;
  }
  results->bits = dataBitsSoFar;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // (DECODE_HITACHI_AC || DECODE_HITACHI_AC1 || DECODE_HITACHI_AC2)
