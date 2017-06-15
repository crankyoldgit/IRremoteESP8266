// Copyright 2015 Darryl Smith
// Copyright 2015 cheaplin
// Copyright 2017 David Conran

#include "ir_LG.h"
#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRtimer.h"

//                               L       GGGG
//                               L      G
//                               L      G  GG
//                               L      G   G
//                               LLLLL   GGG

// LG decode originally added by Darryl Smith (based on the JVC protocol)
// LG send originally added by https://github.com/chaeplin

// Constants
#define LG_HDR_MARK             8000U
#define LG_HDR_SPACE            4000U
#define LG_BIT_MARK              560U
#define LG_ONE_SPACE            1690U
#define LG_ZERO_SPACE            560U
#define LG_RPT_SPACE            2250U
#define LG_MIN_GAP             40000U
#define LG_MIN_MESSAGE_LENGTH 108000UL
#define LG32_HDR_MARK           4500U
#define LG32_HDR_SPACE          4500U
#define LG32_RPT_HDR_MARK       9000U

#if (SEND_LG || DECODE_LG)
// Calculate the rolling 4-bit wide checksum over all of the data.
//  Args:
//    data: The value to be checksum'ed.
//  Returns:
//    A 4-bit checksum.
uint8_t calcLGChecksum(uint16_t data) {
  return(((data >> 12) + ((data >> 8) & 0xF) + ((data >> 4) & 0xF) +
         (data & 0xF)) & 0xF);
}
#endif

#if SEND_LG
// Send an LG formatted message.
//
// Args:
//   data:   The contents of the message you want to send.
//   nbits:  The bit size of the message being sent.
//           Typically LG_BITS or LG32_BITS.
//   repeat: The number of times you want the message to be repeated.
//
// Status: Beta / Should be working.
//
// Notes:
//   LG has a separate message to indicate a repeat, like NEC does.
void IRsend::sendLG(uint64_t data, uint16_t nbits, uint16_t repeat) {
  // Set IR carrier frequency
  enableIROut(38);

  uint16_t repeatHeaderMark = 0;
  IRtimer usecTimer = IRtimer();

  if (nbits >= LG32_BITS) {
    // LG 32bit protocol is near identical to Samsung except for repeats.
    sendSAMSUNG(data, nbits, 0);  // Send it as a single Samsung message.
    repeatHeaderMark = LG32_RPT_HDR_MARK;
    repeat++;
  } else {
    // LG (28-bit) protocol.
    repeatHeaderMark = LG_HDR_MARK;
    // Header
    usecTimer.reset();
    mark(LG_HDR_MARK);
    space(LG_HDR_SPACE);
    // Data
    sendData(LG_BIT_MARK, LG_ONE_SPACE, LG_BIT_MARK, LG_ZERO_SPACE,
             data, nbits, true);
    // Footer
    mark(LG_BIT_MARK);
    space(std::max((uint32_t) (LG_MIN_MESSAGE_LENGTH - usecTimer.elapsed()),
                   (uint32_t) LG_MIN_GAP));
  }

  // Repeat
  // Protocol has a mandatory repeat-specific code sent after every command.
  for (uint16_t i = 0; i < repeat; i++) {
    usecTimer.reset();
    mark(repeatHeaderMark);
    space(LG_RPT_SPACE);
    mark(LG_BIT_MARK);
    space(std::max((uint32_t) LG_MIN_MESSAGE_LENGTH - usecTimer.elapsed(),
                   (uint32_t) LG_MIN_GAP));
  }
}

// Construct a raw 28-bit LG message from the supplied address & command.
//
// Args:
//   address: The address code.
//   command: The command code.
// Returns:
//   A raw 28-bit LG message suitable for sendLG().
//
// Status: BETA / Should work.
//
// Notes:
//   e.g. Sequence of bits = address + command + checksum.
uint32_t IRsend::encodeLG(uint16_t address, uint16_t command) {
  return ((address << 20) | (command << 4) | calcLGChecksum(command));
}
#endif

#if DECODE_LG
// Decode the supplied LG message.
// LG protocol has a repeat code which is 4 items long.
// Even though the protocol has 28/32 bits of data, only 24/28 bits are
// distinct.
// In transmission order, the 28/32 bits are constructed as follows:
//   8/12 bits of address + 16 bits of command + 4 bits of checksum.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of bits to expect in the data portion.
//            Typically LG_BITS or LG32_BITS.
//   strict:  Flag to indicate if we strictly adhere to the specification.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: BETA / Should work.
//
// Note:
//   LG 32bit protocol appears near identical to the Samsung protocol.
//   They possibly differ on how they repeat and initial HDR mark.

// Ref:
//   https://funembedded.wordpress.com/2014/11/08/ir-remote-control-for-lg-conditioner-using-stm32f302-mcu-on-mbed-platform/
bool IRrecv::decodeLG(decode_results *results, uint16_t nbits, bool strict) {
  if (results->rawlen < 2 * nbits + HEADER + FOOTER - 1 && results->rawlen != 4)
    return false;  // Can't possibly be a valid LG message.
  if (strict && nbits != LG_BITS && nbits != LG32_BITS)
    return false;  // Doesn't comply with expected LG protocol.

  uint64_t data = 0;
  uint16_t offset = OFFSET_START;

  // Header
  if (!matchMark(results->rawbuf[offset], LG_HDR_MARK) &&
      !matchMark(results->rawbuf[offset], LG32_HDR_MARK))
    return false;
  offset++;
  if (!matchSpace(results->rawbuf[offset], LG_HDR_SPACE) &&
      !matchMark(results->rawbuf[offset], LG32_HDR_SPACE))
    return false;
  offset++;
  // Data
  for (uint16_t i = 0; i < nbits; i++, offset++) {
    if (!matchMark(results->rawbuf[offset++], LG_BIT_MARK))
      return false;
    if (matchSpace(results->rawbuf[offset], LG_ONE_SPACE))
      data = (data << 1) | 1;  // 1
    else if (matchSpace(results->rawbuf[offset], LG_ZERO_SPACE))
      data <<= 1;  // 0
    else
      return false;
  }
  // Footer
  if (!matchMark(results->rawbuf[offset++], LG_BIT_MARK))
    return false;
  if (offset < results->rawlen &&
      !matchAtLeast(results->rawbuf[offset], LG_MIN_GAP))
    return false;

  // Repeat
  if (nbits >= LG32_BITS) {
    // If we are expecting the LG 32-bit protocol, there is always
    // a repeat message. So, check for it.
#ifndef UNIT_TEST
    if (!matchSpace(results->rawbuf[offset], LG_MIN_GAP))
#else
    if (!(matchSpace(results->rawbuf[offset], LG_MIN_MESSAGE_LENGTH) ||
          matchSpace(results->rawbuf[offset], 65500) ||
          matchSpace(results->rawbuf[offset], LG_MIN_GAP)))
#endif  // UNIT_TEST
      return false;
    offset++;
    if (!matchMark(results->rawbuf[offset++], LG32_RPT_HDR_MARK))
      return false;
    if (!matchSpace(results->rawbuf[offset++], LG_RPT_SPACE))
      return false;
    if (!matchMark(results->rawbuf[offset++], LG_BIT_MARK))
      return false;
    if (offset < results->rawlen &&
        !matchAtLeast(results->rawbuf[offset], LG_MIN_GAP))
      return false;
  }

  // Compliance
  uint16_t command = (data >> 4) & 0xFFFF;  // The 16 bits before the checksum.

  if (strict && (data & 0xF) != calcLGChecksum(command))
    return false;  // The last 4 bits sent are the expected checksum.

  // Success
  results->decode_type = LG;
  results->bits = nbits;
  results->value = data;
  results->command = command;
  results->address = data >> 20;  // The bits before the command.
  return true;
}
#endif
