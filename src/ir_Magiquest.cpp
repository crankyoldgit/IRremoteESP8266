#include "ir_Magiquest.h"
#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRtimer.h"
#include "IRutils.h"

// #define DEBUG

#define IS_ZERO(m, s) (((m) * 100 / ((m) + (s))) <= MAGIQUEST_ZERO_RATIO)
#define IS_ONE(m, s)  (((m) * 100 / ((m) + (s))) >= MAGIQUEST_ONE_RATIO)

// Strips taken from:
// https://github.com/kitlaan/Arduino-IRremote/blob/master/ir_Magiquest.cpp
// and
// https://github.com/mpflaga/Arduino-IRremote

// Source: https://github.com/mpflaga/Arduino-IRremote

#if SEND_MAGIQUEST
void IRsend::sendMagiQuest(uint32_t wand_id, uint16_t magnitude)
{
  magiquest data;
  //data.cmd.scrap = 0x00;
  data.cmd.padding = 0x00;
  data.cmd.magnitude = magnitude;
  data.cmd.wand_id = wand_id;
  
  enableIROut(38);
  for (uint16_t i = 0; i < 56; i++) {
    data.llword <<= 1;
    if (((int8_t) data.byte[6]) < 0) { // if negative then MSBit is one.
      mark(MAGIQUEST_MARK_ONE);
      space(MAGIQUEST_SPACE_ONE);
    }
    else {
      mark(MAGIQUEST_MARK_ZERO);
      space(MAGIQUEST_SPACE_ZERO);
    }
  }
}
#endif

// Source: https://github.com/kitlaan/Arduino-IRremote/blob/master/ir_Magiquest.cpp

#if DECODE_MAGIQUEST
bool  IRrecv::decodeMagiQuest (decode_results *results)
{
	int bits = 0;
	uint64_t data = 0;
	int offset = 1;  // Skip first SPACE

	if (results->rawlen < (2 * MAGIQUEST_BITS))  {
#ifdef DEBUG
    Serial.printf ("Not enough bits to be Magiquest - Rawlen: %i  Expected: %i\n", results->rawlen, (2 * MAGIQUEST_BITS));
#endif
    return false ;
  }
	// Of six wands as datapoints, so far they all start with 8 ZEROs.
	// For example, here is the data from two wands
	// 00000000 00100011 01001100 00100110 00000010 00000010 00010111
	// 00000000 00100000 10001000 00110001 00000010 00000010 10110100

	// Decode the (MARK + SPACE) bits
	while (offset + 1 < results->rawlen) {
		int mark  = results->rawbuf[offset+0];
		int space = results->rawbuf[offset+1];

		if (!matchMark(mark + space, MAGIQUEST_TOTAL_USEC)) {
#ifdef DEBUG
    Serial.printf ("Not enough time to be Magiquest - Mark: %i  Space: %i  Total: %i  Expected: %i\n", mark, space, mark+space, MAGIQUEST_TOTAL_USEC);
#endif
      return false ;
    }

		if      (IS_ZERO(mark, space))  data = (data << 1) | 0;
		else if (IS_ONE( mark, space))  data = (data << 1) | 1;
		else                            return false ;

		bits++;
		offset += 2;
	}

	// Grab the last MARK bit, assuming a good SPACE after it
	if (offset < results->rawlen) {
		int mark  = results->rawbuf[offset+0];
		int space = (MAGIQUEST_TOTAL_USEC / RAWTICK) - mark;

		if      (IS_ZERO(mark, space))  data = (data << 1) | 0;
		else if (IS_ONE( mark, space))  data = (data << 1) | 1;
		else                            return false ;

		bits++;
	}

	if (bits != MAGIQUEST_BITS)  return false ;

	results->decode_type = MAGIQUEST;
	results->bits = 32;
	results->value = data >> 24;
	results->magiquestMagnitude = data & 0xFFFFFF;

	return true;
}
#endif