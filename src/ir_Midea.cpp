// Copyright 2017 bwze, crankyoldgit

#include "IRrecv.h"
#include "IRsend.h"
#include "IRtimer.h"
#include "IRutils.h"

//                  MM    MM IIIII DDDDD   EEEEEEE   AAA
//                  MMM  MMM  III  DD  DD  EE       AAAAA
//                  MM MM MM  III  DD   DD EEEEE   AA   AA
//                  MM    MM  III  DD   DD EE      AAAAAAA
//                  MM    MM IIIII DDDDDD  EEEEEEE AA   AA

// Midea A/C added by (send) bwze/crankyoldgit & (decode) crankyoldgit
//
// Equipment it seems compatible with:
//  * Pioneer System Model RYBO12GMFILCAD (12K BTU)
//  * Pioneer System Model RUBO18GMFILCAD (18K BTU)
//  * <Add models (A/C & remotes) you've gotten it working with here>

// Constants
#define MIDEA_TICK             100U
#define MIDEA_BIT_MARK_TICKS     5U
#define MIDEA_BIT_MARK         (MIDEA_BIT_MARK_TICKS * MIDEA_TICK)
#define MIDEA_ONE_SPACE_TICKS   17U
#define MIDEA_ONE_SPACE        (MIDEA_ONE_SPACE_TICKS * MIDEA_TICK)
#define MIDEA_ZERO_SPACE_TICKS   5U
#define MIDEA_ZERO_SPACE       (MIDEA_ZERO_SPACE_TICKS * MIDEA_TICK)
#define MIDEA_HDR_MARK_TICKS    43U
#define MIDEA_HDR_MARK         (MIDEA_HDR_MARK_TICKS * MIDEA_TICK)
#define MIDEA_HDR_SPACE_TICKS   43U
#define MIDEA_HDR_SPACE        (MIDEA_HDR_SPACE_TICKS * MIDEA_TICK)
#define MIDEA_MIN_GAP_TICKS    (MIDEA_HDR_MARK_TICKS + MIDEA_ZERO_SPACE_TICKS \
                                + MIDEA_BIT_MARK_TICKS)
#define MIDEA_MIN_GAP          (MIDEA_MIN_GAP_TICKS * MIDEA_TICK)
#define MIDEA_TOLERANCE         50U  // Percent

#if SEND_MIDEA
// Send a Midea message
//
// Args:
//   data:   Contents of the message to be sent.
//   nbits:  Nr. of bits of data to be sent. Typically MIDEA_BITS.
//   repeat: Nr. of additional times the message is to be sent.
//
// Status: Alpha / Needs testing against a real device.
//
void IRsend::sendMidea(uint64_t data, uint16_t nbits, uint16_t repeat) {
  if (nbits % 8 != 0)
    return;  // nbits is required to be a multiple of 8.

  // Set IR carrier frequency
  enableIROut(38);

  for (uint16_t r = 0; r <= repeat; r++) {
    // The protcol sends the message, then follows up with an entirely
    // inverted payload.
    for (size_t inner_loop = 0; inner_loop < 2; inner_loop++) {
      // Header
      mark(MIDEA_HDR_MARK);
      space(MIDEA_HDR_SPACE);
      // Data
      //   Break data into byte segments, starting at the Most Significant
      //   Byte. Each byte then being sent normal, then followed inverted.
      for (uint16_t i = 8; i <= nbits; i += 8) {
        // Grab a bytes worth of data.
        uint8_t segment = (data >> (nbits - i)) & 0xFF;
        sendData(MIDEA_BIT_MARK, MIDEA_ONE_SPACE,
                 MIDEA_BIT_MARK, MIDEA_ZERO_SPACE,
                 segment, 8, true);
      }
      // Footer
      mark(MIDEA_BIT_MARK);
      space(MIDEA_MIN_GAP);  // Pause before repeating

      // Invert the data for the 2nd phase of the message.
      // As we get called twice in the inner loop, we will always revert
      // to the original 'data' state.
      data = ~data;
    }
  }
}
#endif

#if DECODE_MIDEA
// Decode the supplied Midea message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect. Typically MIDEA_BITS.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: Alpha / Needs testing against a real device.
//
bool IRrecv::decodeMidea(decode_results *results, uint16_t nbits,
                         bool strict) {
  if (nbits % 8 != 0)  // nbits has to be a multiple of nr. of bits in a byte.
    return false;

  uint8_t min_nr_of_messages = 1;
  if (strict) {
    if (nbits != MIDEA_BITS)
      return false;  // Not strictly an MIDEA message.
    min_nr_of_messages = 2;
  }

  // The protocol sends the data normal + inverted, alternating on
  // each byte. Hence twice the number of expected data bits.
  if (results->rawlen < min_nr_of_messages * (2 * nbits + HEADER + FOOTER) - 1)
    return false;  // Can't possibly be a valid MIDEA message.

  uint64_t data = 0;
  uint64_t inverted = 0;
  uint16_t offset = OFFSET_START;

  if (nbits > sizeof(data) * 8)
    return false;  // We can't possibly capture a Midea packet that big.

  for (uint8_t i = 0; i < min_nr_of_messages; i++) {
    // Header
    if (!matchMark(results->rawbuf[offset], MIDEA_HDR_MARK)) return false;
    // Calculate how long the common tick time is based on the header mark.
    uint32_t m_tick = results->rawbuf[offset++] * RAWTICK /
        MIDEA_HDR_MARK_TICKS;
    if (!matchSpace(results->rawbuf[offset], MIDEA_HDR_SPACE)) return false;
    // Calculate how long the common tick time is based on the header space.
    uint32_t s_tick = results->rawbuf[offset++] * RAWTICK /
        MIDEA_HDR_SPACE_TICKS;

    // Data (Normal)
    match_result_t data_result = matchData(&(results->rawbuf[offset]), nbits,
                                           MIDEA_BIT_MARK_TICKS * m_tick,
                                           MIDEA_ONE_SPACE_TICKS * s_tick,
                                           MIDEA_BIT_MARK_TICKS * m_tick,
                                           MIDEA_ZERO_SPACE_TICKS * s_tick,
                                           MIDEA_TOLERANCE);
    if (data_result.success == false) return false;
    offset += data_result.used;
    if (i % 2 == 0)
      data = data_result.data;
    else
      inverted = data_result.data;

    // Footer
    if (!matchMark(results->rawbuf[offset++], MIDEA_BIT_MARK_TICKS * m_tick,
                   MIDEA_TOLERANCE))
      return false;
    if (offset < results->rawlen &&
        !matchAtLeast(results->rawbuf[offset++], MIDEA_MIN_GAP_TICKS * s_tick,
                      MIDEA_TOLERANCE))
      return false;
  }


  // Compliance
  if (strict) {
    // Protocol requires a second message with all the data bits inverted.
    // We should have checked we got a second message in the previous loop.
    // Just need to check it's value is an inverted copy of the first message.
    uint64_t mask = (1ULL << MIDEA_BITS) - 1;
    if ((data & mask) != ((inverted ^ mask) & mask))  return false;
    // TODO(crankyoldgit): Add checksum verification.
  }

  // Success
  results->decode_type = MIDEA;
  results->bits = nbits;
  results->value = data;
  results->address = 0;
  results->command = 0;
  return true;
}
#endif
