#include "IRremote.h"
#include "IRremoteInt.h"


//==============================================================================
//              SSSS   AAA    MMM    SSSS  U   U  N   N   GGGG
//             S      A   A  M M M  S      U   U  NN  N  G
//              SSS   AAAAA  M M M   SSS   U   U  N N N  G  GG
//                 S  A   A  M   M      S  U   U  N  NN  G   G
//             SSSS   A   A  M   M  SSSS    UUU   N   N   GGG
//==============================================================================

/*
#define SAMSUNG_HDR_MARK    5000
#define SAMSUNG_HDR_SPACE   5000
#define SAMSUNG_BIT_MARK     560
#define SAMSUNG_ONE_SPACE   1600
#define SAMSUNG_ZERO_SPACE   560
#define SAMSUNG_RPT_SPACE   2250
*/

// Update by Sebastien Warin for my EU46D6200
#define SAMSUNG_BITS          32
#define SAMSUNG_HDR_MARK    4500
#define SAMSUNG_HDR_SPACE   4500
#define SAMSUNG_BIT_MARK     590
#define SAMSUNG_ONE_SPACE   1690
#define SAMSUNG_ZERO_SPACE   590
#define SAMSUNG_RPT_SPACE   2250

//+=============================================================================
#if SEND_SAMSUNG

void IRsend::sendSAMSUNG(unsigned long data, int nbits) {
  // Set IR carrier frequency
  enableIROut(38);

  // Header
  mark( SAMSUNG_HDR_MARK);
  space(SAMSUNG_HDR_SPACE);
  
  // Data
  for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
      mark(SAMSUNG_BIT_MARK);
      space_encode( data & mask, SAMSUNG_ONE_SPACE,SAMSUNG_ZERO_SPACE);
  }
 
  // Footer
  mark( SAMSUNG_BIT_MARK);
  space(SAMSUNG_RPT_SPACE);
}


#endif

//+=============================================================================
// SAMSUNGs have a repeat only 4 items long
//
#if DECODE_SAMSUNG
bool IRrecv::decodeSAMSUNG(decode_results *results) {
  if (irparams.rawlen < 2 * SAMSUNG_BITS + 1 + OFFSET_START) return false;
 
  unsigned   long data = 0;
  int offset = OFFSET_START;  // Skip first space
  
  // Header   
  if (!MATCH_MARK(results->rawbuf[offset++],  SAMSUNG_HDR_MARK))  return false;
  if (!MATCH_SPACE(results->rawbuf[offset++], SAMSUNG_HDR_SPACE)) return false;
  
  // Data bits 
  for (int i = 0; i < SAMSUNG_BITS; i++) {
    if (!MATCH_MARK(results->rawbuf[offset++], SAMSUNG_BIT_MARK)) return false;
    if (!space_decode(data,results->rawbuf[offset++],SAMSUNG_ONE_SPACE,SAMSUNG_ZERO_SPACE))  return false;
  }
  // Success
  results->bits        = SAMSUNG_BITS;
  results->value       = data;
  results->decode_type = SAMSUNG;
  return true;
}
#endif

