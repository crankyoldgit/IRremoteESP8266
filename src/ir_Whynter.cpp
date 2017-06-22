// Copyright 2009 Ken Shirriff
// Copyright 2017 David Conran

#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRtimer.h"

//               W   W  H   H  Y   Y N   N TTTTT EEEEE  RRRRR
//               W   W  H   H   Y Y  NN  N   T   E      R   R
//               W W W  HHHHH    Y   N N N   T   EEE    RRRR
//               W W W  H   H    Y   N  NN   T   E      R  R
//                WWW   H   H    Y   N   N   T   EEEEE  R   R

// Whynter A/C ARC-110WD added by Francesco Meschia
// Whynter originally added from https://github.com/shirriff/Arduino-IRremote/

// Constants
#define WHYNTER_HDR_MARK             2850U
#define WHYNTER_HDR_SPACE            2850U
#define WHYNTER_BIT_MARK              750U
#define WHYNTER_ONE_SPACE            2150U
#define WHYNTER_ZERO_SPACE            750U
#define WHYNTER_MIN_COMMAND_LENGTH 108000UL  // Completely made up value.
#define WHYNTER_MIN_GAP WHYNTER_MIN_COMMAND_LENGTH - \
    (2 * (WHYNTER_BIT_MARK + WHYNTER_ZERO_SPACE) + \
     WHYNTER_BITS * (WHYNTER_BIT_MARK + WHYNTER_ONE_SPACE))

#if SEND_WHYNTER
// Send a Whynter message.
//
// Args:
//   data: message to be sent.
//   nbits: Nr. of bits of the message to be sent.
//   repeat: Nr. of additional times the message is to be sent.
//
// Status: STABLE
//
// Ref:
//   https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Whynter.cpp
void IRsend::sendWhynter(uint64_t data, uint16_t nbits, uint16_t repeat) {
  // Set IR carrier frequency
  enableIROut(38);
  IRtimer usecTimer = IRtimer();

  for (uint16_t i = 0; i <= repeat; i++) {
    usecTimer.reset();
    // Header
    mark(WHYNTER_BIT_MARK);
    space(WHYNTER_ZERO_SPACE);
    mark(WHYNTER_HDR_MARK);
    space(WHYNTER_HDR_SPACE);
    // Data
    sendData(WHYNTER_BIT_MARK, WHYNTER_ONE_SPACE, WHYNTER_BIT_MARK,
             WHYNTER_ZERO_SPACE, data, nbits, true);
    // Footer
    mark(WHYNTER_BIT_MARK);
    space(std::max(WHYNTER_MIN_COMMAND_LENGTH - usecTimer.elapsed(),
                   WHYNTER_MIN_GAP));
  }
}
#endif

#if DECODE_WHYNTER
// Decode the supplied Whynter message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of data bits to expect.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: BETA  Strict mode is ALPHA.
//
// Ref:
//   https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Whynter.cpp
bool IRrecv::decodeWhynter(decode_results *results, uint16_t nbits,
                           bool strict) {
  if (results->rawlen < 2 * nbits + 2 * HEADER + FOOTER - 1)
     return false;  // We don't have enough entries to possibly match.

  // Compliance
  if (strict && nbits != WHYNTER_BITS)
    return false;  // Incorrect nr. of bits per spec.

  uint16_t offset = OFFSET_START;

  // Header
  // Sequence begins with a bit mark and a zero space
  if (!matchMark(results->rawbuf[offset++], WHYNTER_BIT_MARK))
    return false;
  if (!matchSpace(results->rawbuf[offset++], WHYNTER_ZERO_SPACE))
    return false;
  // header mark and space
  if (!matchMark(results->rawbuf[offset++], WHYNTER_HDR_MARK))
    return false;
  if (!matchSpace(results->rawbuf[offset++], WHYNTER_HDR_SPACE))
    return false;

  // Data
  uint64_t data = 0;
  for (uint16_t i = 0; i < nbits; i++, offset++) {
    if (!matchMark(results->rawbuf[offset++], WHYNTER_BIT_MARK))
      return false;
    if (matchSpace(results->rawbuf[offset], WHYNTER_ONE_SPACE))
      data = (data << 1) | 1;  // 1
    else if (matchSpace(results->rawbuf[offset], WHYNTER_ZERO_SPACE))
      data <<= 1;  // 0
    else
      return false;
  }

  // Footer
  if (!matchMark(results->rawbuf[offset++], WHYNTER_BIT_MARK))
    return false;
  if (offset < results->rawlen &&
      !matchAtLeast(results->rawbuf[offset], WHYNTER_MIN_GAP))
    return false;

  // Success
  results->decode_type = WHYNTER;
  results->bits = nbits;
  results->value = data;
  results->address = 0;
  results->command = 0;
  return true;
}
#endif
