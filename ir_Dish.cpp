#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//                       DDDD   IIIII   SSSS  H   H
//                        D  D    I    S      H   H
//                        D  D    I     SSS   HHHHH
//                        D  D    I        S  H   H
//                       DDDD   IIIII  SSSS   H   H
//==============================================================================

// Sharp and DISH support by Todd Treece ( http://unionbridge.org/design/ircommand )
//
// The sned function needs to be repeated 4 times
//
// Only send the last for characters of the hex.
// I.E.  Use 0x1C10 instead of 0x0000000000001C10 as listed in the LIRC file.
//
// Here is the LIRC file I found that seems to match the remote codes from the
// oscilloscope:
//   DISH NETWORK (echostar 301):
//   http://lirc.sourceforge.net/remotes/echostar/301_501_3100_5100_58xx_59xx

#define DISH_BITS          16
#define DISH_HDR_MARK     400
#define DISH_HDR_SPACE   6100
#define DISH_BIT_MARK     400
#define DISH_ONE_SPACE   1700
#define DISH_ZERO_SPACE  2800
#define DISH_RPT_SPACE   6200
#define MIN_REPEAT          4

//+=============================================================================
#if SEND_DISH
void  IRsend::sendDISH (unsigned long data,  int nbits)
{
 // Set IR carrier frequency
  enableIROut(56);
  for (int i=0 ; i<MIN_REPEAT; i++) {
    // Header
    mark(DISH_HDR_MARK);
    space(DISH_HDR_SPACE);
    // Data
    for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
       mark(DISH_BIT_MARK);
       space_encode(data & mask,DISH_ONE_SPACE,DISH_ZERO_SPACE);
    }
    // Footer
    mark( DISH_HDR_MARK); //added 26th March 2016, by AnalysIR ( https://www.AnalysIR.com )
    space(DISH_RPT_SPACE);  
  }
       
}
#endif
#if DECODE_DISH
bool IRrecv::decodeDISH(decode_results *results) {
  if (irparams.rawlen < 2 * DISH_BITS + 1 + OFFSET_START) return false;
 
  unsigned   long data = 0;
  int offset = OFFSET_START;  // Skip first space
  
  // Header   
  if (!MATCH_MARK( results->rawbuf[offset++], DISH_HDR_MARK))  return false;
  if (!MATCH_SPACE(results->rawbuf[offset++], DISH_HDR_SPACE)) return false;
  
  // Data bits 
  for (int i = 0; i < DISH_BITS ; i++) {
    if (!MATCH_MARK(results->rawbuf[offset++], DISH_BIT_MARK)) return false;
    if (!space_decode(data,results->rawbuf[offset++],DISH_ONE_SPACE,DISH_ZERO_SPACE))  return false;
  }
  // Footer
  if (!MATCH_MARK( results->rawbuf[offset++], DISH_HDR_MARK )) return false;
  if (!MATCH_SPACE(results->rawbuf[offset++], DISH_RPT_SPACE)) return false;
  
  // Success
  results->bits        = DISH_BITS ;
  results->value       = data;
  results->decode_type = DISH;
  return true;
}
#endif

