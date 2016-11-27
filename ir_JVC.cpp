#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//                             JJJJJ  V   V   CCCC
//                               J    V   V  C
//                               J     V V   C
//                             J J     V V   C
//                              J       V     CCCC
//==============================================================================
//16 bits
//http://lirc.sourceforge.net/remotes/jvc/6711R1P037A
//http://lirc.sourceforge.net/remotes/jvc/RM-RX050
//Nov 2016 marcosamarinho implemented 32 bits at the same protocol 
#define JVC_BITS           16
#define JVC_HDR_MARK     8000
#define JVC_HDR_SPACE    4000
#define JVC_BIT_MARK      600
#define JVC_ONE_SPACE    1600
#define JVC_ZERO_SPACE    550
#define JVC_RPT_SPACE   22000
#define MIN_REPEAT          2




#if SEND_JVC

// DEPRECATED repeat please use just two parameters and send just a time  
void  IRsend::sendJVC (unsigned long data,  int nbits ){
  enableIROut(38);
  // Header 
  //+=============================================================================
  // JVC does NOT repeat by sending a separate code (like NEC does).
  // The JVC protocol repeats by skipping the header.
  mark( JVC_HDR_MARK );
  space(JVC_HDR_SPACE);
  for  (int j = 0; j < MIN_REPEAT; j++) { 
    // Data
    for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
      mark( JVC_BIT_MARK );
      space_encode(data & mask,JVC_ONE_SPACE,JVC_ZERO_SPACE);
    }
    // Footer
    mark( JVC_BIT_MARK);
    space(JVC_RPT_SPACE); 
  }
}
#endif


//+=============================================================================
#if DECODE_JVC

bool IRrecv::decodeJVC(decode_results *results) {
  unsigned long data = 0;
  int offset = OFFSET_START; // Skip first space
  if (irparams.rawlen < 2 * JVC_BITS + OFFSET_START)  return false;
  
  // Header  
  if (!MATCH_MARK( results->rawbuf[offset++], JVC_HDR_MARK))  return false;
  if (!MATCH_SPACE(results->rawbuf[offset++], JVC_HDR_SPACE)) return false;
  
  // Data
  for (int i = 0; i < JVC_BITS; i++) {  
    if (!MATCH_MARK(results->rawbuf[offset++], JVC_BIT_MARK)) return false;
    if (!space_decode(data,results->rawbuf[offset++],JVC_ONE_SPACE,JVC_ZERO_SPACE) )  return false;
  }
  if (!MATCH_MARK(results->rawbuf[offset++], JVC_BIT_MARK)) return false;
  if (!(results->rawbuf[offset] > JVC_RPT_SPACE*.7))         return false;  // found JVC_RPT_SPACE
  // Success
  results->bits        = JVC_BITS;
  results->command     = data & 0xFF;
  results->address     = data  >>8; 
  results->value       = data;
  results->decode_type = JVC;
  return true;
}
#endif
