// Copyright 2017 David Conran

#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRtimer.h"

//                RRRRRR   CCCCC          MM    MM MM    MM
//                RR   RR CC    C         MMM  MMM MMM  MMM
//                RRRRRR  CC       _____  MM MM MM MM MM MM
//                RR  RR  CC    C         MM    MM MM    MM
//                RR   RR  CCCCC          MM    MM MM    MM

// Send & decode support for RC-MM added by David Conran

// Constants
// Ref:
//   http://www.sbprojects.com/knowledge/ir/rcmm.php
#define RCMM_HDR_MARK      416U
#define RCMM_HDR_SPACE     277U
#define RCMM_BIT_MARK      166U
#define RCMM_BIT_SPACE_0   277U
#define RCMM_BIT_SPACE_1   444U
#define RCMM_BIT_SPACE_2   611U
#define RCMM_BIT_SPACE_3   777U
#define RCMM_RPT_LENGTH  27778U
#define RCMM_MIN_GAP      3360U
// Use a tolerance of +/-10% when matching some data spaces.
#define RCMM_TOLERANCE      10U
#define RCMM_EXCESS         50U

#if SEND_RCMM
// Send a Philips RC-MM packet.
// Based on http://www.sbprojects.com/knowledge/ir/rcmm.php
// Args:
//   data: The data we want to send. MSB first.
//   nbits: The number of bits of data to send. (Typically 12, 24, or 32[Nokia])
//
// Status:  ALPHA (untested and unconfirmed.)
void IRsend::sendRCMM(uint32_t data, uint8_t nbits) {
  // Set 36kHz IR carrier frequency & a 1/3 (33%) duty cycle.
  enableIROut(36, 33);
  IRtimer usecs = IRtimer();

  // Header
  mark(RCMM_HDR_MARK);
  space(RCMM_HDR_SPACE);
  // Data
  uint32_t mask = 0b11 << (nbits - 2);
  // RC-MM sends data 2 bits at a time.
  for (uint8_t i = nbits; i > 0; i -= 2) {
    mark(RCMM_BIT_MARK);
    // Grab the next Most Significant Bits to send.
    switch ((data & mask) >> (i - 2)) {
      case 0b00: space(RCMM_BIT_SPACE_0); break;
      case 0b01: space(RCMM_BIT_SPACE_1); break;
      case 0b10: space(RCMM_BIT_SPACE_2); break;
      case 0b11: space(RCMM_BIT_SPACE_3); break;
    }
    mask >>= 2;
  }
  // Footer
  mark(RCMM_BIT_MARK);
  // Protocol requires us to wait at least RCMM_RPT_LENGTH usecs from the start
  // or RCMM_MIN_GAP usecs.
  space(std::max(RCMM_RPT_LENGTH - usecs.elapsed(), RCMM_MIN_GAP));
}
#endif

#if DECODE_RCMM
// Decode a Philips RC-MM packet (between 12 & 32 bits) if possible.
// Places successful decode information in the results pointer.
// Returns:
//   The decode success status.
//
// Status:  ALPHA (untested and unconfirmed.)
// Ref:
//   http://www.sbprojects.com/knowledge/ir/rcmm.php
bool IRrecv::decodeRCMM(decode_results *results) {
  uint32_t data = 0;
  uint16_t offset = OFFSET_START;

  int16_t bitSize = results->rawlen - 4;
  if (bitSize < 12 || bitSize > 32)
    return false;
  // Header decode
  if (!matchMark(results->rawbuf[offset++], RCMM_HDR_MARK))
    return false;
  if (!matchSpace(results->rawbuf[offset++], RCMM_HDR_SPACE))
    return false;
  // Data decode
  // RC-MM has two bits of data per mark/space pair.
  for (uint16_t i = 0; i < bitSize; i += 2, offset++) {
    data <<= 2;
    // Use non-default tolerance & excess for matching some of the spaces as the
    // defaults are too generous and causes mis-matches in some cases.
    if (!matchMark(results->rawbuf[offset++], RCMM_BIT_MARK, RCMM_TOLERANCE))
      return false;
    if (matchSpace(results->rawbuf[offset],
                   RCMM_BIT_SPACE_0, TOLERANCE, RCMM_EXCESS))
      data += 0;
    else if (matchSpace(results->rawbuf[offset],
                        RCMM_BIT_SPACE_1, TOLERANCE, RCMM_EXCESS))
      data += 1;
    else if (matchSpace(results->rawbuf[offset],
                        RCMM_BIT_SPACE_2, RCMM_TOLERANCE, RCMM_EXCESS))
      data += 2;
    else if (matchSpace(results->rawbuf[offset],
                        RCMM_BIT_SPACE_3, RCMM_TOLERANCE, RCMM_EXCESS))
      data += 3;
    else
      return false;
  }
  // Footer decode
  if (!matchMark(results->rawbuf[offset], RCMM_BIT_MARK))
    return false;

  // Success
  results->value = data;
  results->decode_type = RCMM;
  results->bits = bitSize;
  results->address = 0;
  results->command = 0;
  return true;
}
#endif
