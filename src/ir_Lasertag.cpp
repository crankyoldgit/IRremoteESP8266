// Copyright 2017 David Conran

#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

//   LL        AAA    SSSSS  EEEEEEE RRRRRR  TTTTTTT   AAA     GGGG
//   LL       AAAAA  SS      EE      RR   RR   TTT    AAAAA   GG  GG
//   LL      AA   AA  SSSSS  EEEEE   RRRRRR    TTT   AA   AA GG
//   LL      AAAAAAA      SS EE      RR  RR    TTT   AAAAAAA GG   GG
//   LLLLLLL AA   AA  SSSSS  EEEEEEE RR   RR   TTT   AA   AA  GGGGGG

// Constants
#define MIN_LASERTAG_SAMPLES       13U
#define LASERTAG_TICK             333U
#define LASERTAG_MIN_GAP       100000UL  // Completely made up amount.
#define LASERTAG_TOLERANCE          0U   // Percentage error margin
#define LASERTAG_EXCESS             0U   // See MARK_EXCESS
#define LASERTAG_DELTA            150U   // Use instead of EXCESS and TOLERANCE.
const int16_t kSPACE = 1;
const int16_t kMARK = 0;

#if SEND_LASERTAG
// Send a Lasertag packet.
// This protocol is pretty much just raw Manchester encoding.
//
// Args:
//   data:    The message you wish to send.
//   nbits:   Bit size of the protocol you want to send.
//   repeat:  Nr. of extra times the data will be sent.
//
// Status: STABLE / Working.
//

void IRsend::sendLasertag(uint64_t data, uint16_t nbits, uint16_t repeat) {
  if (nbits > sizeof(data) * 8)
    return;  // We can't send something that big.

  // Set 36kHz IR carrier frequency & a 1/4 (25%) duty cycle.
  // NOTE: duty cycle is not confirmed. Just guessing based on RC5/6 protocols.
  enableIROut(36, 25);

  for (uint16_t i = 0; i <= repeat; i++) {
    // Data
    for (uint64_t mask = 1ULL << (nbits - 1); mask; mask >>= 1)
      if (data & mask) {  // 1
        space(LASERTAG_TICK);  // 1 is space, then mark.
        mark(LASERTAG_TICK);
      } else {  // 0
        mark(LASERTAG_TICK);  // 0 is mark, then space.
        space(LASERTAG_TICK);
      }
    // Footer
    space(LASERTAG_MIN_GAP);
  }
}
#endif  // SEND_LASERTAG

#if DECODE_LASERTAG
// Decode the supplied Lasertag message.
// This protocol is pretty much just raw Manchester encoding.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: BETA / Appears to be working 90% of the time.
//
// Ref:
//   http://www.sbprojects.com/knowledge/ir/rc5.php
//   https://en.wikipedia.org/wiki/RC-5
//   https://en.wikipedia.org/wiki/Manchester_code
bool IRrecv::decodeLasertag(decode_results *results, uint16_t nbits,
                            bool strict) {
  if (results->rawlen < MIN_LASERTAG_SAMPLES) return false;

  // Compliance
  if (strict && nbits != LASERTAG_BITS) return false;

  uint16_t offset = OFFSET_START;
  uint16_t used = 0;
  uint64_t data = 0;
  uint16_t actual_bits = 0;

  // No Header

  // Data
  for (; offset <= results->rawlen; actual_bits++) {
    int16_t levelA = getRClevel(results, &offset, &used, LASERTAG_TICK,
                                LASERTAG_TOLERANCE, LASERTAG_EXCESS,
                                LASERTAG_DELTA);
    int16_t levelB = getRClevel(results, &offset, &used, LASERTAG_TICK,
                                LASERTAG_TOLERANCE, LASERTAG_EXCESS,
                                LASERTAG_DELTA);
    if (levelA == kSPACE && levelB == kMARK) {
      data = (data << 1) | 1;  // 1
    } else {
      if (levelA == kMARK && levelB == kSPACE) {
        data <<= 1;  // 0
      } else {
      break;
      }
    }
  }
  // Footer (None)

  // Compliance
  if (actual_bits < nbits) return false;  // Less data than we expected.
  if (strict && actual_bits != LASERTAG_BITS) return false;

  // Success
  results->decode_type = LASERTAG;
  results->value = data;
  results->address = data & 0xF;  // Unit
  results->command = data >> 4;   // Team
  results->repeat = false;
  results->bits = actual_bits;
  return true;
}
#endif  // DECODE_LASERTAG
