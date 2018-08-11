// Copyright 2015 Darryl Smith
// Copyright 2015 cheaplin
// Copyright 2017 David Conran

#include "ir_LG.h"
#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

//                               L       GGGG
//                               L      G
//                               L      G  GG
//                               L      G   G
//                               LLLLL   GGG

// LG decode originally added by Darryl Smith (based on the JVC protocol)
// LG send originally added by https://github.com/chaeplin

// Constants
const uint16_t kLGTick = 50;
const uint16_t kLGHdrMarkTicks = 170;
const uint16_t kLGHdrMark = kLGHdrMarkTicks * kLGTick;  // 8500
const uint16_t kLGHdrSpaceTicks = 85;
const uint16_t kLGHdrSpace = kLGHdrSpaceTicks * kLGTick;  // 4250
const uint16_t kLGBitMarkTicks = 11;
const uint16_t kLGBitMark = kLGBitMarkTicks * kLGTick;
const uint16_t kLGOneSpaceTicks = 32;
const uint16_t kLGOneSpace = kLGOneSpaceTicks * kLGTick;
const uint16_t kLGZeroSpaceTicks = 11;
const uint16_t kLGZeroSpace = kLGZeroSpaceTicks * kLGTick;
const uint16_t kLGRptSpaceTicks = 45;
const uint16_t kLGRptSpace = kLGRptSpaceTicks * kLGTick;
const uint16_t kLGMinGapTicks = 795;
const uint16_t kLGMinGap = kLGMinGapTicks * kLGTick;
const uint16_t kLGMinMessageLengthTicks = 2161;
const uint32_t kLGMinMessageLength = kLGMinMessageLengthTicks * kLGTick;

const uint16_t kLG32HdrMarkTicks = 90;
const uint16_t kLG32HdrMark = kLG32HdrMarkTicks * kLGTick;
const uint16_t kLG32HdrSpaceTicks = 89;
const uint16_t kLG32HdrSpace = kLG32HdrSpaceTicks * kLGTick;
const uint16_t kLG32RptHdrMarkTicks = 179;
const uint16_t kLG32RptHdrMark = kLG32RptHdrMarkTicks * kLGTick;

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
//           Typically kLGBits or kLG32Bits.
//   repeat: The number of times you want the message to be repeated.
//
// Status: Beta / Should be working.
//
// Notes:
//   LG has a separate message to indicate a repeat, like NEC does.
void IRsend::sendLG(uint64_t data, uint16_t nbits, uint16_t repeat) {
  uint16_t repeatHeaderMark = 0;

  if (nbits >= kLG32Bits) {
    // LG 32bit protocol is near identical to Samsung except for repeats.
    sendSAMSUNG(data, nbits, 0);  // Send it as a single Samsung message.
    repeatHeaderMark = kLG32RptHdrMark;
    repeat++;
  } else {
    // LG (28-bit) protocol.
    repeatHeaderMark = kLGHdrMark;
    sendGeneric(kLGHdrMark, kLGHdrSpace,
                kLGBitMark, kLGOneSpace,
                kLGBitMark, kLGZeroSpace,
                kLGBitMark,
                kLGMinGap, kLGMinMessageLength,
                data, nbits, 38, true, 0,  // Repeats are handled later.
                50);
  }

  // Repeat
  // Protocol has a mandatory repeat-specific code sent after every command.
  if (repeat)
    sendGeneric(repeatHeaderMark, kLGRptSpace,
                0, 0, 0, 0,  // No data is sent.
                kLGBitMark, kLGMinGap, kLGMinMessageLength,
                0, 0,  // No data.
                38, true, repeat - 1, 50);
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
//            Typically kLGBits or kLG32Bits.
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
  if (nbits >= kLG32Bits) {
    if (results->rawlen < 2 * nbits + 2 * (kHeader + kFooter) - 1)
      return false;  // Can't possibly be a valid LG32 message.
  } else {
    if (results->rawlen < 2 * nbits + kHeader + kFooter - 1)
      return false;  // Can't possibly be a valid LG message.
  }
  if (strict && nbits != kLGBits && nbits != kLG32Bits)
    return false;  // Doesn't comply with expected LG protocol.

  uint64_t data = 0;
  uint16_t offset = kStartOffset;

  // Header
  if (!matchMark(results->rawbuf[offset], kLGHdrMark) &&
      !matchMark(results->rawbuf[offset], kLG32HdrMark)) return false;
  uint32_t m_tick;
  if (matchMark(results->rawbuf[offset], kLGHdrMark))
    m_tick = results->rawbuf[offset++] * kRawTick / kLGHdrMarkTicks;
  else
    m_tick = results->rawbuf[offset++] * kRawTick / kLG32HdrMarkTicks;
  if (!matchSpace(results->rawbuf[offset], kLGHdrSpace) &&
      !matchSpace(results->rawbuf[offset], kLG32HdrSpace)) return false;
  uint32_t s_tick;
  if (matchSpace(results->rawbuf[offset], kLGHdrSpace))
    s_tick = results->rawbuf[offset++] * kRawTick / kLGHdrSpaceTicks;
  else
    s_tick = results->rawbuf[offset++] * kRawTick / kLG32HdrSpaceTicks;

  // Data
  match_result_t data_result = matchData(&(results->rawbuf[offset]), nbits,
                                         kLGBitMarkTicks * m_tick,
                                         kLGOneSpaceTicks * s_tick,
                                         kLGBitMarkTicks * m_tick,
                                         kLGZeroSpaceTicks * s_tick);
  if (data_result.success == false) return false;
  data = data_result.data;
  offset += data_result.used;

  // Footer
  if (!matchMark(results->rawbuf[offset++], kLGBitMarkTicks * m_tick))
    return false;
  if (offset < results->rawlen &&
      !matchAtLeast(results->rawbuf[offset], kLGMinGapTicks * s_tick))
    return false;

  // Repeat
  if (nbits >= kLG32Bits) {
    // If we are expecting the LG 32-bit protocol, there is always
    // a repeat message. So, check for it.
    offset++;
    if (!matchMark(results->rawbuf[offset++], kLG32RptHdrMarkTicks * m_tick))
      return false;
    if (!matchSpace(results->rawbuf[offset++], kLGRptSpaceTicks * s_tick))
      return false;
    if (!matchMark(results->rawbuf[offset++], kLGBitMarkTicks * m_tick))
      return false;
    if (offset < results->rawlen &&
        !matchAtLeast(results->rawbuf[offset], kLGMinGapTicks * s_tick))
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
