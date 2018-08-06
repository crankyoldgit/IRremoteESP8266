// Copyright 2018 David Conran

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

//              GGGG      IIIII       CCCCC    AAA   BBBBB   LL      EEEEEEE
//             GG  GG      III       CC    C  AAAAA  BB   B  LL      EE
//            GG           III       CC      AA   AA BBBBBB  LL      EEEEE
//            GG   GG ...  III  ...  CC    C AAAAAAA BB   BB LL      EE
//             GGGGGG ... IIIII ...   CCCCC  AA   AA BBBBBB  LLLLLLL EEEEEEE
//
// Ref:
//   https://github.com/cyborg5/IRLib2/blob/master/IRLibProtocols/IRLib_P09_GICable.h
//   https://github.com/markszabo/IRremoteESP8266/issues/447

// Constants
const uint16_t kGICableHdrMark = 9000;
const uint16_t kGICableHdrSpace = 4400;
const uint16_t kGICableBitMark = 550;
const uint16_t kGICableOneSpace = 4400;
const uint16_t kGICableZeroSpace = 2200;
const uint16_t kGICableRptSpace = 2200;
const uint32_t kGICableMinCommandLength = 99600;
const uint32_t kGICableMinGap = kGICableMinCommandLength -
    (kGICableHdrMark + kGICableHdrSpace +
     kGICableBits * (kGICableBitMark + kGICableOneSpace) + kGICableBitMark);

#if SEND_GICABLE
// Send a raw G.I. Cable formatted message.
//
// Args:
//   data:   The message to be sent.
//   nbits:  The number of bits of the message to be sent.
//           Typically kGICableBits.
//   repeat: The number of times the command is to be repeated.
//
// Status: Alpha / Untested.
//
// Ref:
void IRsend::sendGICable(uint64_t data, uint16_t nbits, uint16_t repeat) {
  sendGeneric(kGICableHdrMark, kGICableHdrSpace,
              kGICableBitMark, kGICableOneSpace,
              kGICableBitMark, kGICableZeroSpace,
              kGICableBitMark, kGICableMinGap, kGICableMinCommandLength,
              data, nbits, 39, true, 0,  // Repeats are handled later.
              50);
  // Message repeat sequence.
  if (repeat)
    sendGeneric(kGICableHdrMark, kGICableRptSpace,
                0, 0, 0, 0,  // No actual data sent.
                kGICableBitMark, kGICableMinGap, kGICableMinCommandLength,
                0, 0,  // No data to be sent.
                39, true, repeat - 1, 50);
}
#endif  // SEND_GICABLE

#if DECODE_GICABLE
// Decode the supplied G.I. Cable message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect. Typically kGICableBits.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: Alpha / Not tested against a real device.
bool IRrecv::decodeGICable(decode_results *results, uint16_t nbits,
                           bool strict) {
  if (results->rawlen < 2 * (nbits + kHeader + kFooter) - 1)
    return false;  // Can't possibly be a valid GICABLE message.
  if (strict && nbits != kGICableBits)
    return false;  // Not strictly an GICABLE message.

  uint64_t data = 0;
  uint16_t offset = kStartOffset;

  // Header
  if (!matchMark(results->rawbuf[offset++], kGICableHdrMark)) return false;
  if (!matchSpace(results->rawbuf[offset++], kGICableHdrSpace)) return false;

  // Data
  match_result_t data_result = matchData(&(results->rawbuf[offset]), nbits,
                                         kGICableBitMark,
                                         kGICableOneSpace,
                                         kGICableBitMark,
                                         kGICableZeroSpace);
  if (data_result.success == false) return false;
  data = data_result.data;
  offset += data_result.used;

  // Footer
  if (!matchMark(results->rawbuf[offset++], kGICableBitMark)) return false;
  if (offset < results->rawlen &&
      !matchAtLeast(results->rawbuf[offset++], kGICableMinGap))
    return false;

  // Compliance
  if (strict) {
    // We expect a repeat frame.
    if (!matchMark(results->rawbuf[offset++], kGICableHdrMark)) return false;
    if (!matchSpace(results->rawbuf[offset++], kGICableRptSpace)) return false;
    if (!matchMark(results->rawbuf[offset++], kGICableBitMark)) return false;
  }

  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = GICABLE;
  results->command = 0;
  results->address = 0;
  return true;
}
#endif  // DECODE_GICABLE
