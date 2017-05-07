// Copyright 2009 Ken Shirriff
// Copyright 2017 David Conran

#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRtimer.h"

// RRRRRR   CCCCC          555555  XX    XX   RRRRRR   CCCCC            666
// RR   RR CC    C         55       XX  XX    RR   RR CC    C          66
// RRRRRR  CC       _____  555555    XXXX     RRRRRR  CC       _____  666666
// RR  RR  CC    C            5555  XX  XX    RR  RR  CC    C         66   66
// RR   RR  CCCCC          555555  XX    XX   RR   RR  CCCCC           66666

// RC-5 & RC-6 support added from https://github.com/z3t0/Arduino-IRremote
// RC-5X support added by David Conran

// Constants
// RC-5/RC-5X
// Ref:
//   https://en.wikipedia.org/wiki/RC-5
//   http://www.sbprojects.com/knowledge/ir/rc5.php
#define MIN_RC5_SAMPLES            11U
#define MIN_RC6_SAMPLES             1U
#define RC5_T1                    889U
#define RC5_MIN_COMMAND_LENGTH 113778UL
#define RC5_MIN_GAP RC5_MIN_COMMAND_LENGTH - RC5_RAW_BITS * (2 * RC5_T1)
// RC-6
// Ref:
//   https://en.wikipedia.org/wiki/RC-6
//   http://www.pcbheaven.com/userpages/The_Philips_RC6_Protocol/
#define RC6_HDR_MARK             2666U
#define RC6_HDR_SPACE             889U
#define RC6_T1                    444U
#define RC6_RPT_LENGTH          83000UL
// Common (getRClevel())
#define MARK  0U
#define SPACE 1U


#if SEND_RC5
// Send a Philips RC-5/RC-5X packet.
//
// Args:
//   data:    The message you wish to send.
//   nbits:   Bit size of the protocol you want to send.
//   repeat:  Nr. of extra times the data will be sent.
//
// Status: RC-5 (stable), RC-5X (alpha)
//
// Note:
//   Caller needs to take care of flipping the toggle bit (3rd transmitted
//   bit/Most Significant data bit). e.g. data ^= 1ULL<<(nbits-1).).
//   That bit differentiates between key press & key release.
// Ref:
//   http://www.sbprojects.com/knowledge/ir/rc5.php
//   https://en.wikipedia.org/wiki/RC-5
//   https://en.wikipedia.org/wiki/Manchester_code
// TODO(anyone):
//   Testing of the RC-5X components.
void IRsend::sendRC5(uint64_t data, uint16_t nbits, uint16_t repeat) {
  bool skipSpace = true;
  uint16_t field_bit = 1;
  // Set 36kHz IR carrier frequency & a 1/4 (25%) duty cycle.
  enableIROut(36, 25);

  if (nbits == RC5X_BITS) {  // Is this a RC-5X message?
    // For RC-5X, invert the 7th bit and send it as the field/2nd start bit.
    field_bit = (data & (1 << (7-1))) ? 0 : 1;
    // Cut out the 7th bit from data, as we send it as the field/2nd start bit.
    data = ((data >> 7) << 6) | (data & ((1 << 7) - 1));
  }

  IRtimer usecTimer = IRtimer();
  for (uint16_t i = 0; i <= repeat; i++) {
    usecTimer.reset();

    // Header
    // First start bit (0x1). space, then mark.
    if (skipSpace)
      skipSpace = false;  // First time through, we assume the leading space().
    else
      space(RC5_T1);
    mark(RC5_T1);
    // Field/Second start bit.
    if (field_bit) {  // Send a 1. Normal for RC-5.
      space(RC5_T1);
      mark(RC5_T1);
    } else {  // Send a 0. Special case for RC-5X. Means 7th command bit is 1.
      mark(RC5_T1);
      space(RC5_T1);
    }

    // Data
    for (uint64_t mask = 1ULL << (nbits - 1); mask; mask >>= 1)
      if (data & mask) {  // 1
        space(RC5_T1);  // 1 is space, then mark.
        mark(RC5_T1);
      } else {  // 0
        mark(RC5_T1);  // 0 is mark, then space.
        space(RC5_T1);
      }

    // Footer
    space(std::max(RC5_MIN_GAP, RC5_MIN_COMMAND_LENGTH - usecTimer.elapsed()));
  }
}
#endif

#if SEND_RC6
// Send a Philips RC-6 packet.
// Note: Caller needs to take care of flipping the toggle bit (The 4th Most
//   Significant Bit). That bit differentiates between key press & key release.
//
// Args:
//   data:    The message you wish to send.
//   nbits:   Bit size of the protocol you want to send.
//   repeat:  Nr. of extra times the data will be sent.
//
// Status: Stable.
//
// Ref:
//   http://www.sbprojects.com/knowledge/ir/rc6.php
//   http://www.righto.com/2010/12/64-bit-rc6-codes-arduino-and-xbox.html
//   https://en.wikipedia.org/wiki/Manchester_code
void IRsend::sendRC6(uint64_t data, uint16_t nbits, uint16_t repeat) {
  // Check we can send the number of bits requested.
  if (nbits > sizeof(data) * 8)
    return;
  // Set 36kHz IR carrier frequency & a 1/3 (33%) duty cycle.
  enableIROut(36, 33);
  for (uint16_t r = 0; r <= repeat; r++) {
    // Header
    mark(RC6_HDR_MARK);
    space(RC6_HDR_SPACE);
    // Start bit.
    mark(RC6_T1);  // mark, then space == 0x1.
    space(RC6_T1);
    // Data
    uint16_t bitTime;
    for (uint64_t i = 1, mask = 1ULL << (nbits - 1); mask; i++, mask >>= 1) {
      if (i == 4)  // The fourth bit we send is a "double width trailer bit".
        bitTime = 2 * RC6_T1;  // double-wide trailer bit
      else
        bitTime = RC6_T1;  // Normal bit
      if (data & mask) {  // 1
        mark(bitTime);
        space(bitTime);
      } else {  // 0
        space(bitTime);
        mark(bitTime);
      }
    }
    // Footer
    space(RC6_RPT_LENGTH);
  }
}
#endif

#if (DECODE_RC5 || DECODE_RC6)
// Gets one undecoded level at a time from the raw buffer.
// The RC5/6 decoding is easier if the data is broken into time intervals.
// E.g. if the buffer has MARK for 2 time intervals and SPACE for 1,
// successive calls to getRClevel will return MARK, MARK, SPACE.
// offset and used are updated to keep track of the current position.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   offset:  Ptr to the currect offset to the rawbuf.
//   used:    Ptr to the current used counter.
//   bitTime: Time interval of single bit in microseconds.
// Returns:
//   int: MARK, SPACE, or -1 for error (The measured time interval is not a
//                                      multiple of t1.)
// Ref:
//   https://en.wikipedia.org/wiki/Manchester_code
int16_t IRrecv::getRClevel(decode_results *results,  uint16_t *offset,
                           uint16_t *used, uint16_t bitTime) {
  if (*offset >= results->rawlen)
    return SPACE;  // After end of recorded buffer, assume SPACE.
  uint16_t width = results->rawbuf[*offset];
  //  If the value of offset is odd, it's a MARK. Even, it's a SPACE.
  uint16_t val = ((*offset) % 2) ? MARK : SPACE;
  int16_t correction = (val == MARK) ? MARK_EXCESS : - MARK_EXCESS;

  // Calculate the look-ahead for our current position in the buffer.
  uint16_t avail;
  if (match(width, bitTime + correction))
    avail = 1;
  else if (match(width, 2 * bitTime + correction))
    avail = 2;
  else if (match(width, 3 * bitTime + correction))
    avail = 3;
  else
    return -1;  // The width is not what we expected.

  (*used)++;  // Count another one of the avail slots as used.
  if (*used >= avail) {  // Are we out of look-ahead/avail slots?
    // Yes, so reset the used counter, and move the offset ahead.
    *used = 0;
    (*offset)++;
  }

#ifdef DEBUG
  if (val == MARK)
    Serial.println("MARK");
  else
    Serial.println("SPACE");
#endif
  return val;
}
#endif

#if DECODE_RC5
// Decode the supplied RC-5/RC5X message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: RC-5 (stable), RC-5X (alpha)
//
// Note:
//   The 'toggle' bit is included as the 6th (MSB) address bit, the MSB of data,
//   & in the count of bits decoded.
// Ref:
//   http://www.sbprojects.com/knowledge/ir/rc5.php
//   https://en.wikipedia.org/wiki/RC-5
//   https://en.wikipedia.org/wiki/Manchester_code
// TODO(anyone):
//   Serious testing of the RC-5X and strict aspects needs to be done.
bool IRrecv::decodeRC5(decode_results *results, uint16_t nbits, bool strict) {
  if (results->rawlen < MIN_RC5_SAMPLES + HEADER) return false;

  // Compliance
  if (strict && nbits != RC5_BITS && nbits != RC5X_BITS)
    return false;  // It's neither RC-5 or RC-5X.

  uint16_t offset = OFFSET_START;
  uint16_t used = 0;
  uint16_t field_bit;  // a.k.a. second start bit.
  bool is_rc5x = false;
  if (nbits == RC5X_BITS) is_rc5x = true;

  // Header
  // Get start bit #1.
  if (getRClevel(results, &offset, &used, RC5_T1) != MARK) return false;
  // Get field/start bit #2 (inverted bit-7 of the command if RC-5X protocol)
  int16_t levelA = getRClevel(results, &offset, &used, RC5_T1);
  int16_t levelB = getRClevel(results, &offset, &used, RC5_T1);
  if (levelA == SPACE && levelB == MARK) {
    field_bit = 1;  // Matched a 1.
  } else if (levelA == MARK && levelB == SPACE) {
    field_bit = 0;  // Matched a 0.
    is_rc5x = true;
    if (strict && nbits == RC5_BITS)
      return false;  // Can't strictly be RC-5X with only RC5_BITS.
  } else {
    return false;  // Not what we expected.
  }

  uint16_t actual_bits = 0;
  if (is_rc5x) actual_bits = 1;  // We've already have our first (field) bit.

  // Data
  uint64_t data = 0;
  for (; offset < results->rawlen; actual_bits++) {
    int16_t levelA = getRClevel(results, &offset, &used, RC5_T1);
    int16_t levelB = getRClevel(results, &offset, &used, RC5_T1);
    if (levelA == SPACE && levelB == MARK)
      data = (data << 1) | 1;  // 1
    else if (levelA == MARK && levelB == SPACE)
      data <<= 1;  // 0
    else
      return false;
  }
  // Footer (None)

  // Compliance
  if (strict) {
    if (actual_bits != RC5_BITS && actual_bits != RC5X_BITS) return false;
    if (actual_bits != nbits) return false;
  }

  // Success
  results->bits = actual_bits;
  if (is_rc5x) {
    results->decode_type = RC5X;
    // Bit juggling. Insert a new 7th command bit.
    data = ((data >> 6) << 7) | (data & ((1 << 7) - 1));
    // Set the 7th bit to the value of the inverted field/2nd start bit.
    data ^= (uint64_t) field_bit << (7 - 1);
    results->address = data >> 7;
    results->command = data & ((1 << 7) - 1);
  } else {
    results->decode_type = RC5;
    results->address = data >> 6;
    results->command = data & ((1 << 6) - 1);
  }
  results->value = data;
  return true;
}
#endif

#if DECODE_RC6
// Decode the supplied RC6 message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: Stable.
//
// Ref:
//   http://www.sbprojects.com/knowledge/ir/rc6.php
//   https://en.wikipedia.org/wiki/Manchester_code
// TODO(anyone):
//   Testing of the strict compliance aspects.
bool IRrecv::decodeRC6(decode_results *results, uint16_t nbits, bool strict) {
  if (results->rawlen < HEADER + 2)  // Header + start bit.
    return false;  // Smaller than absolute smallest possible RC6 message.

  if (strict) {  // Compliance
    // Unlike typical protocols, the ability to have mark+space, and space+mark
    // as data bits means it is possible to only have nbits of entries for the
    // data portion, rather than the typically required 2 * nbits.
    // Also due to potential melding with the start bit, we can only count
    // the start bit as 1, instead of a more typical 2 value. The header still
    // remains as normal.
    if (results->rawlen < nbits + HEADER + 1)
      return false;  // Don't have enough entries/samples to be valid.
    if (nbits != RC6_MODE0_BITS && nbits != RC6_36_BITS)
      return false;  // Asking for the wrong number of bits.
  }

  uint16_t offset = OFFSET_START;

  // Header
  if (!matchMark(results->rawbuf[offset++], RC6_HDR_MARK)) return false;
  if (!matchSpace(results->rawbuf[offset++], RC6_HDR_SPACE)) return false;

  uint16_t used = 0;

  // Get the start bit. e.g. 1.
  if (getRClevel(results, &offset, &used, RC6_T1) != MARK) return false;
  if (getRClevel(results, &offset, &used, RC6_T1) != SPACE) return false;

  uint16_t actual_bits;
  uint64_t data = 0;

  // Data (Warning: Here be dragons^Wpointers!!)
  for (actual_bits = 0; offset < results->rawlen; actual_bits++) {
    int16_t levelA, levelB;  // Next two levels
    levelA = getRClevel(results, &offset, &used, RC6_T1);
    // T bit is double wide; make sure second half matches
    if (actual_bits == 3 &&
        levelA != getRClevel(results, &offset, &used, RC6_T1))
      return false;
    levelB = getRClevel(results, &offset, &used, RC6_T1);
    // T bit is double wide; make sure second half matches
    if (actual_bits == 3 &&
        levelB != getRClevel(results, &offset, &used, RC6_T1))
      return false;
    if (levelA == MARK && levelB == SPACE)  // reversed compared to RC5
      data = (data << 1) | 1;  // 1
    else if (levelA == SPACE && levelB == MARK)
      data <<= 1;  // 0
    else
      return false;
  }

  // More compliance
  if (strict && actual_bits != nbits)
    return false;  // Actual nr. of bits didn't match expected.

  // Success
  results->decode_type = RC6;
  results->bits = actual_bits;
  results->value = data;
  results->address = data >> (actual_bits - 4);  // Top four bits are the mode.
  results->command = data & ((1ULL << (actual_bits - 4)) - 1);  // The rest.
  return true;
}
#endif
