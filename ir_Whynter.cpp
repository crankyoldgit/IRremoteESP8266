#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//               W   W  H   H  Y   Y N   N TTTTT EEEEE  RRRRR
//               W   W  H   H   Y Y  NN  N   T   E      R   R
//               W W W  HHHHH    Y   N N N   T   EEE    RRRR
//               W W W  H   H    Y   N  NN   T   E      R  R
//                WWW   H   H    Y   N   N   T   EEEEE  R   R
//==============================================================================

#define WHYNTER_BITS          32
#define WHYNTER_HDR_MARK    2850
#define WHYNTER_HDR_SPACE   2850
#define WHYNTER_BIT_MARK     750
#define WHYNTER_ONE_SPACE   2150
#define WHYNTER_ZERO_SPACE   750


//+=============================================================================
#if SEND_WHYNTER
void IRsend::sendWhynter(unsigned long data, int nbits) {
  // Set IR carrier frequency
  enableIROut(38);
  // Start
  mark(WHYNTER_BIT_MARK);
  space(WHYNTER_ZERO_SPACE);
  // Header
  mark(WHYNTER_HDR_MARK);
  space(WHYNTER_HDR_SPACE);
  
  // Data
  for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
    mark(WHYNTER_BIT_MARK);
    space_encode(data & mask, WHYNTER_ONE_SPACE, WHYNTER_ZERO_SPACE);
  }
  // Footer
  mark(WHYNTER_BIT_MARK);
  space(WHYNTER_ZERO_SPACE);
}
#endif

//+=============================================================================
#if DECODE_WHYNTER
bool IRrecv::decodeWhynter(decode_results *results) {
 
  unsigned long data = 0;
  int         offset = OFFSET_START; // Skip first space
  
  // Check we have the right amount of data
  if (irparams.rawlen < 2 * WHYNTER_BITS + 5 + OFFSET_START) return false;
  
  // Start
  if (!MATCH_MARK(results->rawbuf[offset++],  WHYNTER_BIT_MARK ))  return false;
  if (!MATCH_SPACE(results->rawbuf[offset++], WHYNTER_ZERO_SPACE)) return false;
  // Header 
  if (!MATCH_MARK(results->rawbuf[offset++],  WHYNTER_HDR_MARK ))  return false;
  if (!MATCH_SPACE(results->rawbuf[offset++], WHYNTER_HDR_SPACE )) return false;
  // Data 
  for (int i = 0; i < WHYNTER_BITS; i++) {
    if (!MATCH_MARK(results->rawbuf[offset++], WHYNTER_BIT_MARK)) return false;
    if (!space_decode(data,results->rawbuf[offset++],WHYNTER_ONE_SPACE, WHYNTER_ZERO_SPACE)) return false;
  }
  // Trailing mark
  if (!MATCH_MARK(results->rawbuf[offset], WHYNTER_BIT_MARK))     return false;
  // Success
  results->bits        = WHYNTER_BITS;
  results->value       = data;
  results->decode_type = WHYNTER;
  return true;
}
#endif

