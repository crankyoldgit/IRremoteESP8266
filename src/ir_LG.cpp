// Copyright 2015 Darryl Smith
// Copyright 2015 cheaplin
// Copyright 2017 David Conran

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
#define LG_BIT_MARK              600U
#define LG_ONE_SPACE            1600U
#define LG_ZERO_SPACE            550U
#define LG_RPT_SPACE            2250U
#define LG_MIN_GAP             20000U  // Completely made up figure.
#define LG_MIN_MESSAGE_LENGTH 108000UL
#define LG32_HDR_MARK           4500U

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
//   nbits:  The bit size of the message being sent. Typically LG_BITS.
//   repeat: The number of times you want the message to be repeated.
//
// Status: STABLE (< 32 bits), ALPHA (>= 32 bits)
//
// Notes:
//   LG has a separate message to indicate a repeat, like NEC does.
void IRsend::sendLG(uint64_t data, uint16_t nbits, uint16_t repeat) {
  // Set IR carrier frequency
  enableIROut(38);

  uint16_t repeatHeaderMark = 0;
  IRtimer usecTimer = IRtimer();

  if (nbits >= 32) {
    // LG 32bit protocol is near identical to Samsung except for repeats.
    sendSAMSUNG(data, nbits, 0);  // Send it as a single Samsung message.
    repeatHeaderMark = LG32_HDR_MARK;
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
uint32_t IRsend::encodeLG(uint8_t address, uint16_t command) {
  return ((address << 20) | (command << 4) | calcLGChecksum(command));
}
#endif

#if DECODE_LG
// Decode the supplied LG message.
// LG protocol has a repeat code which is 4 items long.
// Even though the protocol has 28 bits of data, only 16 bits are distinct.
// In transmission order, the 28 bits are constructed as follows:
//   8 bits of address + 16 bits of command + 4 bits of checksum.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of bits to expect in the data portion. Typically LG_BITS.
//   strict:  Flag to indicate if we strictly adhere to the specification.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: BETA  Strict mode is ALPHA / Untested.
//
// Note:
//   LG 32bit protocol appears near identical to the Samsung protocol.
//   They differ on their compliance criteria and how they repeat.
// Ref:
//   https://funembedded.wordpress.com/2014/11/08/ir-remote-control-for-lg-conditioner-using-stm32f302-mcu-on-mbed-platform/
bool IRrecv::decodeLG(decode_results *results, uint16_t nbits, bool strict) {
  if (results->rawlen < 2 * nbits + 4 && results->rawlen != 4)
    return false;  // Can't possibly be a valid LG message.
  if (strict && nbits != LG_BITS)
    return false;  // Doesn't comply with expected LG protocol.

  uint64_t data = 0;
  uint16_t offset = OFFSET_START;

  // Header
  if (!matchMark(results->rawbuf[offset++], LG_HDR_MARK))
    return false;
  if (!matchSpace(results->rawbuf[offset++], LG_HDR_SPACE))
    return false;
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
  if (!matchMark(results->rawbuf[offset], LG_BIT_MARK))
    return false;

  // Compliance

  uint8_t address = (data >> 16) & 0xFF;  // Address is the first 8 bits sent.
  uint16_t command = (data >> 4) & 0xFFFF;  // Command is the next 16 bits sent.
  // The last 4 bits sent are the expected checksum.
  if (strict && (data & 0xF) != calcLGChecksum(command))
    return false;

  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = LG;
  results->address = address;
  results->command = command;
  return true;
}
#endif
