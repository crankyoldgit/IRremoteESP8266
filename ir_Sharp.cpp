
#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//                       SSSS  H   H   AAA   RRRR   PPPP
//                      S      H   H  A   A  R   R  P   P
//                       SSS   HHHHH  AAAAA  RRRR   PPPP
//                          S  H   H  A   A  R  R   P
//                      SSSS   H   H  A   A  R   R  P
//==============================================================================

// Sharp and DISH support by Todd Treece: http://unionbridge.org/design/ircommand
//
// The send function has the necessary repeat built in because of the need to
// invert the signal.
//
// Sharp protocol documentation:
//   http://www.sbprojects.com/knowledge/ir/sharp.htm
//
// Here is the LIRC file I found that seems to match the remote codes from the
// oscilloscope:
//   Sharp LCD TV:
//   http://lirc.sourceforge.net/remotes/sharp/GA538WJSA
/*
#define SHARP_BITS             15
#define SHARP_BIT_MARK        245
#define SHARP_ONE_SPACE      1805
#define SHARP_ZERO_SPACE      795
#define SHARP_GAP          600000
#define SHARP_RPT_SPACE      3000
*/
#define SHARP_BITS              15
#define SHARP_BIT_MARK         320
#define SHARP_ONE_SPACE       1650
#define SHARP_ZERO_SPACE       650
#define SHARP_RPT_SPACE      46000




#define SHARP_TOGGLE_MASK  0x3FF

//+=============================================================================
#if SEND_SHARP
void  IRsend::sendSharpRaw (unsigned long data,  int nbits) { 
  enableIROut(38);
  // Sending codes in bursts of 3 (normal, inverted, normal) makes transmission
  // much more reliable. That's the exact behaviour of CD-S6470 remote control.
  for (int n = 0; n < 3; n++) {
    for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
      mark(SHARP_BIT_MARK); 
      space_encode(data & mask,SHARP_ONE_SPACE,SHARP_ZERO_SPACE); 
    }
    // Footer 
    mark(SHARP_BIT_MARK);
    space(SHARP_RPT_SPACE);
   
    data = data ^ SHARP_TOGGLE_MASK;  // Invert data
  }
}


#endif

#if SEND_SHARP
void  IRsend::sendSharp (unsigned int address,  unsigned int command)
{
   sendSharpRaw((address << 10) | (command << 2) | 2, SHARP_BITS);
}
#endif

bool IRrecv::decodeSharp(decode_results *results) {
  // Check we have the right amount of data
  if (irparams.rawlen < 2 * SHARP_BITS + 3 + OFFSET_START) return false; 
  unsigned long data = 0;
  int offset = OFFSET_START; // Skip first space
  
  // NO Header 
  // Data bits 
  for (int i = 0; i < SHARP_BITS; i++) {
    if (!MATCH_MARK(results->rawbuf[offset++],SHARP_BIT_MARK)) return false;
    if (!space_decode(data,results->rawbuf[offset++],SHARP_ONE_SPACE,SHARP_ZERO_SPACE)) return false;
  }
   // validate REPEAT 
  if (!MATCH_MARK(results->rawbuf[offset++],SHARP_BIT_MARK)) return false;
  if (!results->rawbuf[offset++]>.7*SHARP_RPT_SPACE ) return false;
  if (!MATCH_MARK(results->rawbuf[offset++],SHARP_BIT_MARK)) return false;
  // TODO check the inverted value post repeat to check integrity 
  // Success
  results->bits        = SHARP_BITS; 
  //results->address     = data >> 12; 
  //results->command     = data & 0xFF; 
  results->value       = data;
  results->decode_type = SHARP;
  return true;
}

