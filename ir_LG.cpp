
#include "IRremote.h"
#include "IRremoteInt.h"
// Nov 2016 marcosamarinho included 32bits 
//==============================================================================
//                               L       GGGG
//                               L      G
//                               L      G  GG
//                               L      G   G
//                               LLLLL   GGG
//==============================================================================
//LIRC LGE_6711A20015N
#define LG_BITS                   28
#define LG_HDR_MARK             8000
#define LG_HDR_SPACE            4000
#define LG_BIT_MARK              600
#define LG_ONE_SPACE            1600
#define LG_ZERO_SPACE            550
#define LG_RPT_LENGTH          60000
//included marcosamarinho Nov 2016
// see LIRC LGE_6711A20015N 
#define LG_BITS_32                32
#define LG_HDR_MARK_32          4500
#define LG_HDR_SPACE_32         4500
#define LG_BIT_MARK_32           600
#define LG_ONE_SPACE_32         1600
#define LG_ZERO_SPACE_32         550
#define LG_RPT_LENGTH_32       44000

//+=============================================================================

#if DECODE_LG_32
bool IRrecv::decodeLG_32(decode_results *results) {
  // Check we have the right amount of data
  if (irparams.rawlen < 2 * LG_BITS_32 + OFFSET_START ) return false;
  unsigned long data = 0;
  int offset = OFFSET_START; // Skip first space
  
  // Header 
  if (!MATCH_MARK( results->rawbuf[offset++], LG_HDR_MARK_32 ))  return false;
  if (!MATCH_SPACE(results->rawbuf[offset++], LG_HDR_SPACE_32))  return false;
 
  // Data 
  for (int i = 0; i < LG_BITS_32; i++) {
    if (!MATCH_MARK( results->rawbuf[offset++], LG_BIT_MARK_32 ))  return false;
    if (!space_decode(data,results->rawbuf[offset++],LG_ONE_SPACE_32,LG_ZERO_SPACE_32)) return false;
   }
  // Forced decode repetition to avoid Samsung that have same times 
  if (!MATCH_MARK( results->rawbuf[offset++], LG_BIT_MARK_32  )) return false;
  if (!MATCH_SPACE(results->rawbuf[offset++], LG_RPT_LENGTH_32)) return false;
  if (!MATCH_MARK( results->rawbuf[offset++], LG_HDR_MARK_32  )) return false;
  // Success
  results->bits        = LG_BITS_32;
  results->address     = data >> 12; 
  results->command     = data & 0xFF; 
  results->value       = data;
  results->decode_type = LG;
  return true;
}
#endif 

#if DECODE_LG
bool IRrecv::decodeLG(decode_results *results) {
  // Check we have the right amount of data
   if (irparams.rawlen < 2 * LG_BITS + OFFSET_START ) return false;
  unsigned long data = 0;
  int offset = OFFSET_START; // Skip first space

  // Header 
  if (!MATCH_MARK( results->rawbuf[offset++], LG_HDR_MARK )) return false;
  if (!MATCH_SPACE(results->rawbuf[offset++], LG_HDR_SPACE)) return false;
 
  // Data 
  for (int i = 0; i < LG_BITS; i++) {
   if (!MATCH_MARK( results->rawbuf[offset++], LG_BIT_MARK )) return false;
   if (!space_decode(data,results->rawbuf[offset++],LG_ONE_SPACE,LG_ZERO_SPACE)) return false;
  }
   // Forced decode repetition to avoid Sanyo that have same times  
  if (!MATCH_MARK( results->rawbuf[offset++], LG_BIT_MARK  )) return false;
  if (!MATCH_SPACE(results->rawbuf[offset++], LG_RPT_LENGTH)) return false;
  if (!MATCH_MARK( results->rawbuf[offset++], LG_HDR_MARK  )) return false;

  // Success
  results->bits        = LG_BITS;
  results->value       = data;
  results->decode_type = LG;
  return true;
}

#endif


//+=============================================================================
#if SEND_LG
void IRsend::sendLG (unsigned long data, int nbits) {
  // Set IR carrier frequency
  enableIROut(38);
  // Header
  if ( nbits==32 ) {
    mark( LG_HDR_MARK_32 );
    space(LG_HDR_SPACE_32);
 
    // Data bits 
    for (unsigned long mask = 1UL << (nbits - 1); mask; mask >>= 1) {
      mark( LG_BIT_MARK_32 );
      space_encode(data & mask,LG_ONE_SPACE_32,LG_ZERO_SPACE_32);
    }
    // forced decode footer  for not take Samsung that have the same times 
    mark( LG_BIT_MARK_32  );
    space(LG_RPT_LENGTH_32); 
    mark( LG_HDR_MARK_32  ); 
    space(LG_HDR_SPACE_32 ); //force low 
 
  } else { 
    //28 bits 
    mark( LG_HDR_MARK );
    space(LG_HDR_SPACE);
    // Data bits 
    for (unsigned long mask = 1UL << (nbits - 1); mask; mask >>= 1) {
      mark( LG_BIT_MARK );
     space_encode(data & mask,LG_ONE_SPACE,LG_ZERO_SPACE);
    }
    mark( LG_BIT_MARK  );
    space(LG_RPT_LENGTH);  
    mark( LG_HDR_MARK  );
    space(LG_HDR_SPACE ); 
  }
}

#endif
