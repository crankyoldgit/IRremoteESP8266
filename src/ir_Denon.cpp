// Copyright 2016 Massimiliano Pinto
// Copyright 2017 David Conran

#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRtimer.h"

//                    DDDD   EEEEE  N   N   OOO   N   N
//                     D  D  E      NN  N  O   O  NN  N
//                     D  D  EEE    N N N  O   O  N N N
//                     D  D  E      N  NN  O   O  N  NN
//                    DDDD   EEEEE  N   N   OOO   N   N

// Original Denon support added by https://github.com/csBlueChip
// Ported over by Massimiliano Pinto

// Constants
// Ref:
//   https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Denon.cpp
#define DENON_HDR_MARK              263U  // The length of the Header:Mark
#define DENON_HDR_SPACE             789U  // The length of the Header:Space
#define DENON_BIT_MARK              263U  // The length of a Bit:Mark
#define DENON_ONE_SPACE            1842U  // The length of a Bit:Space for 1's
#define DENON_ZERO_SPACE            789U  // The length of a Bit:Space for 0's
#define DENON_MIN_COMMAND_LENGTH 134052UL
#define DENON_MIN_GAP DENON_MIN_COMMAND_LENGTH - \
    (DENON_HDR_MARK + DENON_HDR_SPACE + DENON_BITS * \
     (DENON_BIT_MARK + DENON_ONE_SPACE) + DENON_BIT_MARK)

#if SEND_DENON
// Send a Denon message
//
// Args:
//   data:   Contents of the message to be sent.
//   nbits:  Nr. of bits of data to be sent. Typically DENON_BITS.
//   repeat: Nr. of additional times the message is to be sent.
//
// Status: BETA / Should be working.
//
// Notes:
//   Some Denon devices use a Kaseikyo/Panasonic 48-bit format.
// Ref:
//   https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Denon.cpp
//   http://assets.denon.com/documentmaster/us/denon%20master%20ir%20hex.xls
void IRsend::sendDenon(uint64_t data, uint16_t nbits, uint16_t repeat) {
  enableIROut(38);  // Set IR carrier frequency
  IRtimer usecTimer = IRtimer();

  for (uint16_t i = 0; i <= repeat; i++) {
    usecTimer.reset();
    // Header
    mark(DENON_HDR_MARK);
    space(DENON_HDR_SPACE);
    // Data
    sendData(DENON_BIT_MARK, DENON_ONE_SPACE, DENON_BIT_MARK, DENON_ZERO_SPACE,
             data, nbits, true);
    // Footer
    mark(DENON_BIT_MARK);
    space(std::max(DENON_MIN_COMMAND_LENGTH - usecTimer.elapsed(),
                   DENON_MIN_GAP));
  }
}
#endif

#if DECODE_DENON
// Decode a Denon message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Expected nr. of data bits. (Typically DENON_BITS)
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: STABLE
//
//
// Ref:
//   https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Denon.cpp
bool IRrecv::decodeDenon(decode_results *results, uint16_t nbits, bool strict) {
  // Check we have enough data
  if (results->rawlen < 2 * nbits + HEADER + FOOTER)
    return false;
  if (strict && nbits != DENON_BITS)
    return false;

  uint64_t data = 0;
  uint16_t offset = OFFSET_START;

  // Header
  if (!matchMark(results->rawbuf[offset++], DENON_HDR_MARK))
    return false;
  if (!matchSpace(results->rawbuf[offset++], DENON_HDR_SPACE))
    return false;
  // Data
  for (uint16_t i = 0; i < nbits; i++, offset++) {
    if (!matchMark(results->rawbuf[offset++], DENON_BIT_MARK))
      return false;
    if (matchSpace(results->rawbuf[offset], DENON_ONE_SPACE))
      data = (data << 1) | 1;  // 1
    else if (matchSpace(results->rawbuf[offset], DENON_ZERO_SPACE))
      data = (data << 1);  // 0
    else
      return false;
  }
  // Footer
  if (!matchMark(results->rawbuf[offset++], DENON_BIT_MARK))
    return false;

  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = DENON;
  results->address = 0;
  results->command = 0;
  return true;
}
#endif
