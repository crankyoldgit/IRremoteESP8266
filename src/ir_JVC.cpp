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
const uint16_t kJVCTick = 75;
const uint16_t kJVCHdrMarkTicks = 112;
const uint16_t kJVCHdrMark = kJVCHdrMarkTicks * kJVCTick;
const uint16_t kJVCHdrSpaceTicks = 56;
const uint16_t kJVCHdrSpace = kJVCHdrSpaceTicks * kJVCTick;
const uint16_t kJVCBitMarkTicks = 7;
const uint16_t kJVCBitMark = kJVCBitMarkTicks * kJVCTick;
const uint16_t kJVCOneSpaceTicks = 23;
const uint16_t kJVCOneSpace = kJVCOneSpaceTicks * kJVCTick;
const uint16_t kJVCZeroSpaceTicks = 7;
const uint16_t kJVCZeroSpace = kJVCZeroSpaceTicks * kJVCTick;
const uint16_t kJVCRptLengthTicks = 800;
const uint16_t kJVCRptLength = kJVCRptLengthTicks * kJVCTick;
const uint16_t kJVCMinGapTicks = kJVCRptLengthTicks -
    (kJVCHdrMarkTicks + kJVCHdrSpaceTicks +
     kJVCBits * (kJVCBitMarkTicks + kJVCOneSpaceTicks) +
     kJVCBitMarkTicks);
const uint16_t kJVCMinGap = kJVCMinGapTicks * kJVCTick;

#if SEND_JVC
// Send a JVC message.
//
// Args:
//   data:   The contents of the command you want to send.
//   nbits:  The bit size of the command being sent. (kJVCBits)
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
  mark(kJVCHdrMark);
  space(kJVCHdrSpace);

  // We always send the data & footer at least once, hence '<= repeat'.
  for (uint16_t i = 0; i <= repeat; i++) {
    sendGeneric(0, 0,  // No Header
                kJVCBitMark, kJVCOneSpace,
                kJVCBitMark, kJVCZeroSpace,
                kJVCBitMark, kJVCMinGap,
                data, nbits, 38, true, 0,  // Repeats are handles elsewhere.
                33);
    // Wait till the end of the repeat time window before we send another code.
    uint32_t elapsed = usecs.elapsed();
    // Avoid potential unsigned integer underflow.
    // e.g. when elapsed > kJVCRptLength.
    if (elapsed < kJVCRptLength)
      space(kJVCRptLength - elapsed);
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
//   nbits:   Nr. of bits of data to expect. Typically kJVCBits.
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
  if (strict && nbits != kJVCBits)
    return false;  // Must be called with the correct nr. of bits.
  if (results->rawlen < 2 * nbits + kFooter - 1)
    return false;  // Can't possibly be a valid JVC message.

  uint64_t data = 0;
  uint16_t offset = kStartOffset;
  bool isRepeat = true;

  uint32_t m_tick;
  uint32_t s_tick;
  // Header
  // (Optional as repeat codes don't have the header)
  if (matchMark(results->rawbuf[offset], kJVCHdrMark)) {
    isRepeat = false;
    m_tick = results->rawbuf[offset++] * kRawTick / kJVCHdrMarkTicks;
    if (results->rawlen < 2 * nbits + 4)
      return false;  // Can't possibly be a valid JVC message with a header.
    if (!matchSpace(results->rawbuf[offset], kJVCHdrSpace))
      return false;
    s_tick = results->rawbuf[offset++] * kRawTick / kJVCHdrSpaceTicks;
  } else {
    // We can't easily auto-calibrate as there is no header, so assume
    // the default tick time.
    m_tick = kJVCTick;
    s_tick = kJVCTick;
  }

  // Data
  match_result_t data_result = matchData(&(results->rawbuf[offset]), nbits,
                                         kJVCBitMarkTicks * m_tick,
                                         kJVCOneSpaceTicks * s_tick,
                                         kJVCBitMarkTicks * m_tick,
                                         kJVCZeroSpaceTicks * s_tick);
  if (data_result.success == false) return false;
  data = data_result.data;
  offset += data_result.used;

  // Footer
  if (!matchMark(results->rawbuf[offset++], kJVCBitMarkTicks * m_tick))
    return false;
  if (offset < results->rawlen &&
      !matchAtLeast(results->rawbuf[offset], kJVCMinGapTicks * s_tick))
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
