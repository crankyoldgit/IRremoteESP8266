// Copyright 2009 Ken Shirriff
// Copyright 2017 David Conran

#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRtimer.h"

//                           N   N  EEEEE   CCCC
//                           NN  N  E      C
//                           N N N  EEE    C
//                           N  NN  E      C
//                           N   N  EEEEE   CCCC

// NEC originally added from https://github.com/shirriff/Arduino-IRremote/

// Constants
// Ref:
//  http://www.sbprojects.com/knowledge/ir/nec.php
#define NEC_HDR_MARK             9000U
#define NEC_HDR_SPACE            4500U
#define NEC_BIT_MARK              560U
#define NEC_ONE_SPACE            1690U
#define NEC_ZERO_SPACE            560U
#define NEC_RPT_SPACE            2250U
#define NEC_RPT_LENGTH              4U
#define NEC_MIN_COMMAND_LENGTH 108000UL
#define NEC_MIN_GAP NEC_MIN_COMMAND_LENGTH - \
    (NEC_HDR_MARK + NEC_HDR_SPACE + NEC_BITS * (NEC_BIT_MARK + NEC_ONE_SPACE) \
     + NEC_BIT_MARK)

#if (SEND_NEC || SEND_SHERWOOD)
// Send a raw NEC(Renesas) formatted message.
//
// Args:
//   data:   The message to be sent.
//   nbits:  The number of bits of the message to be sent. Typically NEC_BITS.
//   repeat: The number of times the command is to be repeated.
//
// Status: STABLE / Known working.
//
// Ref:
//  http://www.sbprojects.com/knowledge/ir/nec.php
void IRsend::sendNEC(uint64_t data, uint16_t nbits, uint16_t repeat) {
  // Set 38kHz IR carrier frequency & a 1/3 (33%) duty cycle.
  enableIROut(38, 33);
  IRtimer usecs = IRtimer();
  // Header
  mark(NEC_HDR_MARK);
  space(NEC_HDR_SPACE);
  // Data
  sendData(NEC_BIT_MARK, NEC_ONE_SPACE, NEC_BIT_MARK, NEC_ZERO_SPACE,
           data, nbits, true);
  // Footer
  mark(NEC_BIT_MARK);
  // Gap to next command.
  space(std::max(NEC_MIN_GAP, NEC_MIN_COMMAND_LENGTH - usecs.elapsed()));

  // Optional command repeat sequence.
  for (uint16_t i = 0; i < repeat; i++) {
    usecs.reset();
    mark(NEC_HDR_MARK);
    space(NEC_RPT_SPACE);
    mark(NEC_BIT_MARK);
    // Gap till next command.
    space(std::max(NEC_MIN_GAP, NEC_MIN_COMMAND_LENGTH - usecs.elapsed()));
  }
}

// Calculate the raw NEC data based on address and command.
// Args:
//   address: An address value.
//   command: An 8-bit command value.
// Returns:
//   A raw 32-bit NEC message.
//
// Status: ALPHA / Untested.
//
// Ref:
//  http://www.sbprojects.com/knowledge/ir/nec.php
uint32_t IRsend::encodeNEC(uint16_t address, uint16_t command) {
  command &= 0xFF;  // We only want the least significant byte of command.
  command = (command <<  8) + (command ^ 0xFF);  // Calculate the new command.
  if (address > 0xFF)  // Is it Extended NEC?
    return (address << 16) + command;  // Extended.
  else
    return (address << 24) + ((address ^ 0xFF) << 16) + command;  // Normal.
}
#endif

#if (DECODE_NEC || DECODE_SHERWOOD)
// Decode the supplied NEC message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect. Typically NEC_BITS.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: STABLE / Known good.
//
// Notes:
//   NEC protocol has three varients/forms.
//     Normal:   a 8 bit address & a 8 bit command in 32 bit data form.
//               i.e. address + inverted(address) + command + inverted(command)
//     Extended: a 16 bit address & a 8 bit command in 32 bit data form.
//               i.e. address + command + inverted(command)
//     Repeat:   a 0-bit code. i.e. No data bits. Just the header + footer.
//
// Ref:
//   http://www.sbprojects.com/knowledge/ir/nec.php
bool IRrecv::decodeNEC(decode_results *results, uint16_t nbits, bool strict) {
  if (results->rawlen < 2 * nbits + HEADER + FOOTER &&
      results->rawlen != NEC_RPT_LENGTH)
    return false;  // Can't possibly be a valid NEC message.
  if (strict && nbits != NEC_BITS)
    return false;  // Not strictly an NEC message.

  uint64_t data = 0;
  uint16_t offset = OFFSET_START;

  // Header
  if (!matchMark(results->rawbuf[offset++], NEC_HDR_MARK))
    return false;
  // Check if it is a repeat code.
  if (results->rawlen == NEC_RPT_LENGTH &&
      matchSpace(results->rawbuf[offset], NEC_RPT_SPACE) &&
      matchMark(results->rawbuf[offset + 1], NEC_BIT_MARK)) {
    results->value = REPEAT;
    results->decode_type = NEC;
    results->bits = 0;
    results->address = 0;
    results->command = 0;
    results->repeat = true;
    return true;
  }

  // Header (cont.)
  if (!matchSpace(results->rawbuf[offset++], NEC_HDR_SPACE))
    return false;
  // Data
  for (uint16_t i = 0; i < nbits; i++, offset++) {
    if (!matchMark(results->rawbuf[offset++], NEC_BIT_MARK))
      return false;
    if (matchSpace(results->rawbuf[offset], NEC_ONE_SPACE))
      data = (data << 1) | 1;
    else if (matchSpace(results->rawbuf[offset], NEC_ZERO_SPACE))
      data <<= 1;
    else
      return false;
  }
  // Footer
  if (!matchMark(results->rawbuf[offset], NEC_BIT_MARK))
      return false;

  // Compliance
  // Calculate command and optionally enforce integrity checking.
  uint8_t command = (data & 0xFF00) >>  8;
  // Command is sent twice, once as plain and then inverted .
  if (strict && (command ^ 0xFF) != (data & 0xFF))
    return false;  // Command integrity failed.

  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = NEC;
  results->command = command;
  // Normal NEC protocol has an 8 bit address sent, followed by it inverted.
  uint8_t address = (data & 0xFF000000) >> 24;
  uint8_t address_inverted = (data & 0x00FF0000) >> 16;
  if (address == (address_inverted ^ 0xFF))
    results->address = address;  // Inverted, so it is normal NEC protocol.
  else  // Not inverted, so must be Extended NEC protocol, thus 16 bit address.
    results->address = data >> 16;  // Most significant four bytes.
  return true;
}
#endif
