// Copyright 2015 Kristian Lauszus
// Copyright 2017 David Conran

#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRtimer.h"
#include "IRutils.h"

//                             JJJJJ  V   V   CCCC
//                               J    V   V  C
//                               J     V V   C
//                             J J     V V   C
//                              J       V     CCCC

// JVC originally added by Kristian Lauszus
// (Thanks to zenwheel and other people at the original blog post)

// Constants
// Ref:
//   http://www.sbprojects.com/knowledge/ir/jvc.php
#define JVC_HDR_MARK    8400U
#define JVC_HDR_SPACE   4200U
#define JVC_BIT_MARK     525U
#define JVC_ONE_SPACE   1725U
#define JVC_ZERO_SPACE   525U
#define JVC_RPT_LENGTH 60000U
#define JVC_MIN_GAP    11400U  // 60000 - 16 * (1725 + 525) - 8400 - 4200

#if SEND_JVC
// Send a JVC message.
//
// Args:
//   data:   The contents of the command you want to send.
//   nbits:  The bit size of the command being sent. (JVC_BITS)
//   repeat: The number of times you want the command to be repeated.
//
// Status: STABLE.
//
// Ref:
//   http://www.sbprojects.com/knowledge/ir/jvc.php
void IRsend::sendJVC(uint64_t data, uint16_t nbits, uint16_t repeat) {
  // Set 38kHz IR carrier frequency & a 1/3 (33%) duty cycle.
  enableIROut(38, 33);

  IRtimer usecs = IRtimer();
  // Header
  // Only sent for the first message.
  mark(JVC_HDR_MARK);
  space(JVC_HDR_SPACE);

  // We always send the data & footer at least once, hence '<= repeat'.
  for (uint16_t i = 0; i <= repeat; i++) {
    // Data
    sendData(JVC_BIT_MARK, JVC_ONE_SPACE, JVC_BIT_MARK, JVC_ZERO_SPACE,
             data, nbits, true);
    // Footer
    mark(JVC_BIT_MARK);
    // Wait till the end of the repeat time window before we send another code.
    space(std::max(JVC_MIN_GAP, JVC_RPT_LENGTH - usecs.elapsed()));
    usecs.reset();
  }
}

// Calculate the raw JVC data based on address and command.
//
// Args:
//   address: An 8-bit address value.
//   command: An 8-bit command value.
// Returns:
//   A raw JVC message.
//
// Status: BETA / Should work fine.
//
// Ref:
//   http://www.sbprojects.com/knowledge/ir/jvc.php
uint16_t IRsend::encodeJVC(uint8_t address, uint8_t command) {
  return reverseBits((command << 8) | address, 16);
}
#endif

#if DECODE_JVC
// Decode the supplied JVC message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of bits of data to expect. Typically JVC_BITS.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: STABLE
//
// Note:
//   JVC repeat codes don't have a header.
// Ref:
//   http://www.sbprojects.com/knowledge/ir/jvc.php
bool IRrecv::decodeJVC(decode_results *results, uint16_t nbits,  bool strict) {
  if (strict && nbits != JVC_BITS)
    return false;  // Must be called with the correct nr. of bits.
  if (results->rawlen < 2 * nbits + FOOTER - 1)
    return false;  // Can't possibly be a valid JVC message.

  uint64_t data = 0;
  uint16_t offset = OFFSET_START;
  bool isRepeat = true;

  // Header
  // (Optional as repeat codes don't have the header)
  if (matchMark(results->rawbuf[offset], JVC_HDR_MARK)) {
    isRepeat = false;
    offset++;
    if (results->rawlen < 2 * nbits + 4)
      return false;  // Can't possibly be a valid JVC message with a header.
    if (!matchSpace(results->rawbuf[offset++], JVC_HDR_SPACE))
      return false;
  }

  // Data
  for (uint16_t i = 0; i < nbits; i++) {
    if (!matchMark(results->rawbuf[offset++], JVC_BIT_MARK))
      return false;
    if (matchSpace(results->rawbuf[offset], JVC_ONE_SPACE))
      data = (data << 1) | 1;  // 1
    else if (matchSpace(results->rawbuf[offset], JVC_ZERO_SPACE))
      data <<= 1;  // 0
    else
      return false;
    offset++;
  }

  // Footer
  if (!matchMark(results->rawbuf[offset++], JVC_BIT_MARK))
    return false;
  if (offset < results->rawlen &&
      !matchAtLeast(results->rawbuf[offset], JVC_MIN_GAP))
    return false;

  // Success
  results->decode_type = JVC;
  results->bits = nbits;
  results->value = data;
  // command & address are transmitted LSB first, so we need to reverse them.
  results->address = reverseBits(data >> 8, 8);  // The first 8 bits sent.
  results->command = reverseBits(data & 0xFF, 8);  // The last 8 bits sent.
  results->repeat = isRepeat;
  return true;
}
#endif
