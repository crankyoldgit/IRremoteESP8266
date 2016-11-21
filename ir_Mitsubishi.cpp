#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//    MMMMM  IIIII TTTTT   SSSS  U   U  BBBB   IIIII   SSSS  H   H  IIIII
//    M M M    I     T    S      U   U  B   B    I    S      H   H    I
//    M M M    I     T     SSS   U   U  BBBB     I     SSS   HHHHH    I
//    M   M    I     T        S  U   U  B   B    I        S  H   H    I
//    M   M  IIIII   T    SSSS    UUU   BBBBB  IIIII  SSSS   H   H  IIIII
//==============================================================================

// marcosamarino nov 2016 changed to space encoding following LIRC file  Mitsubishi RM 75501, not tested with real device 
#define BITS              16
#define BIT_MARK         283 
#define ONE_SPACE       2157
#define ZERO_SPACE       937 
#define GAP            53681
#define MIN_REPEAT         2
//+=============================================================================
#if DECODE_MITSUBISHI
bool  IRrecv::decodeMitsubishi (decode_results *results)
{
  unsigned long data = 0;
  if (irparams.rawlen < 2 * BITS + 1 + OFFSET_START)  return false ;
  int offset = OFFSET_START; // Skip first space
  for (int i=0;i<BITS;i++ ) {
     if (!MATCH_MARK(results->rawbuf[offset++], BIT_MARK))  return false ;
     if (!space_decode(data,results->rawbuf[offset++],ONE_SPACE, ZERO_SPACE))  return false;
  }
  if (!MATCH_MARK(results->rawbuf[offset++], BIT_MARK))  return false ;
  if (!MATCH_SPACE(results->rawbuf[offset++], GAP))  return false ;
  // Success
  results->bits       = BITS; 
  results->value       = data;
  results->decode_type = MITSUBISHI;
  return true;
}
#endif
#if SEND_MITSUBISHI
void IRsend::sendMitsubishi (unsigned long data, int nbits) {
  // Set IR carrier frequency
  enableIROut(38);
  // Header
  // Data
  for ( int i=0 ; i< MIN_REPEAT ; i++) {
   for (unsigned long  mask = 1UL << (BITS - 1);  mask;  mask >>= 1) {
      mark( BIT_MARK); 
      space_encode(data & mask,ONE_SPACE,ZERO_SPACE);
    }
    // Footer
    mark( BIT_MARK);
    space(GAP);
  }
}
#endif
