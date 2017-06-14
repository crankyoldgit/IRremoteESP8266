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
#define DENON_MANUFACTURER       0x2A4CULL

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
//   Some Denon devices use a Kaseikyo/Panasonic 48-bit format
//   Others use the Sharp protocol.
// Ref:
//   https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Denon.cpp
//   http://assets.denon.com/documentmaster/us/denon%20master%20ir%20hex.xls
void IRsend::sendDenon(uint64_t data, uint16_t nbits, uint16_t repeat) {
  if (nbits >= PANASONIC_BITS)  // Is this really Panasonic?
    sendPanasonic64(data, nbits, repeat);
  else if (nbits == DENON_LEGACY_BITS)
    // Support legacy (broken) calls of sendDenon().
    sendSharpRaw(data & (~0x2000ULL), nbits + 1, repeat);
  else
    sendSharpRaw(data, nbits, repeat);
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
// Status: BETA / Should work fine.
//
// Ref:
//   https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Denon.cpp
bool IRrecv::decodeDenon(decode_results *results, uint16_t nbits, bool strict) {
  // Compliance
  if (strict) {
    switch (nbits) {
      case DENON_BITS:
      case DENON_48_BITS:
      case DENON_LEGACY_BITS:
        break;
      default:
        return false;
    }
  }

  // Denon uses the Sharp & Panasonic(Kaseikyo) protocol for some
  // devices, so check for those first.
  // It is not exactly like Sharp's protocols, but close enough.
  // e.g. The expansion bit is not set for Denon vs. set for Sharp.
  // Ditto for Panasonic, it's the same except for a different
  // manufacturer code.

  if (!decodeSharp(results, nbits, true, false) &&
      !decodePanasonic(results, nbits, true, DENON_MANUFACTURER)) {
    // We couldn't decode it as expected, so try the old legacy method.
    // NOTE: I don't this following protocol actually exists.
    //       Looks like a partial version of the Sharp protocol.
    // Check we have enough data
    if (results->rawlen < 2 * nbits + HEADER + FOOTER - 1)
      return false;
    if (strict && nbits != DENON_LEGACY_BITS)
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
    results->address = 0;
    results->command = 0;
  }  // Legacy decode.

  // Compliance
  if (strict && nbits != results->bits) return false;

  // Success
  results->decode_type = DENON;
  return true;
}
#endif
