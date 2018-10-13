// Copyright 2009 Ken Shirriff
// Copyright 2017, 2018 David Conran
// Copyright 2018 Kamil Palczewski

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"
#include "ir_NEC.h"

//                        PPPP  III  OOO  N   N EEEE EEEE RRRR
//                        P   P  I  O   O NN  N E    E    R   R
//                        PPPP   I  O   O N N N EEE  EEE  RRRR
//                        P      I  O   O N  NN E    E    R R
//                        P     III  OOO  N   N EEEE EEEE R  RR

// Constants
// Ref:
//  http://adrian-kingston.com/IRFormatPioneer.htm
const uint16_t kPioneerSectionGap = 989;


#if SEND_PIONEER
// Send a raw Pioneer formatted message.
//
// Args:
//   data:   The message to be sent.
//   nbits:  The number of bits of the message to be sent.
//           Typically kPioneerBits.
//   repeat: The number of times the command is to be repeated.
//
// Status: BETA / Expected to be working.
//
// Ref:
//  http://adrian-kingston.com/IRFormatPioneer.htm
void IRsend::sendPioneer(const uint64_t data, const uint16_t nbits,
                         const uint16_t repeat) {
  // If nbits is to big, or is odd, abort.
  if (nbits > sizeof(data) * 8 || nbits % 2 == 1)  return;

  // send 1st part of the code
  sendNEC(data >> (nbits / 2), nbits / 2, 0);

  // send space between the codes
  sendGeneric(kNecBitMark, kPioneerSectionGap,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 38, true, 0, 33);  // No data sent.

  // send 2nd part of the code
  sendNEC(data & ((1UL << (nbits / 2)) - 1), nbits / 2, repeat);
}

// Calculate the raw Pioneer data based on address and command.
// Args:
//   address: A 32-bit address value.
//   command: A 32-bit command value.
// Returns:
//   A raw 64-bit Pioneer message.
//
// Status: BETA / Expected to work.
//
uint64_t IRsend::encodePioneer(const uint32_t address, const uint32_t command) {
  return (((uint64_t) address) << 32) | command;
}
#endif  // SEND_PIONEER

#if DECODE_PIONEER
// Decode the supplied Pioneer message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect. Typically kPioneerBits.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: BETA / Should be working. (Self decodes)
//
bool IRrecv::decodePioneer(decode_results *results, const uint16_t nbits,
                           const bool strict) {
  if (results->rawlen < 2 * nbits + 3 * (kHeader + kFooter) - 1)
    return false;  // Can't possibly be a valid Pioneer message.
  if (strict && nbits != kPioneerBits)
    return false;  // Not strictly an Pioneer message.

  uint64_t data = 0;
  uint16_t offset = kStartOffset;

  for (uint16_t section = 0; section < 2; section++) {
    // Header
    if (!matchMark(results->rawbuf[offset], kNecHdrMark)) return false;
    // Calculate how long the lowest tick time is based on the header mark.
    uint32_t mark_tick = results->rawbuf[offset++] * kRawTick /
        kNecHdrMarkTicks;
    if (!matchSpace(results->rawbuf[offset], kNecHdrSpace)) return false;
    // Calculate how long the common tick time is based on the header space.
    uint32_t space_tick = results->rawbuf[offset++] * kRawTick /
        kNecHdrSpaceTicks;
    //
    // Data
    match_result_t data_result = matchData(&(results->rawbuf[offset]),
                                           nbits / 2,
                                           kNecBitMarkTicks * mark_tick,
                                           kNecOneSpaceTicks * space_tick,
                                           kNecBitMarkTicks * mark_tick,
                                           kNecZeroSpaceTicks * space_tick);
    if (data_result.success == false) return false;
    data = (data << (nbits / 2)) + data_result.data;
    offset += data_result.used;

    // Footer
    if (!matchMark(results->rawbuf[offset++], kNecBitMarkTicks * mark_tick))
        return false;
    if (offset < results->rawlen &&
        !matchAtLeast(results->rawbuf[offset++], kNecMinGapTicks * space_tick))
      return false;
    // Inter-section marker.
    if (section == 0) {
      if (!matchMark(results->rawbuf[offset++], kNecBitMarkTicks * mark_tick))
          return false;
      if (!matchSpace(results->rawbuf[offset++], kPioneerSectionGap))
          return false;
    }
  }

  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = PIONEER;
  results->command = data & ((1UL << (nbits / 2)) - 1);
  results->address = data >> (nbits / 2);
  return true;
}
#endif  // DECODE_PIONEER
