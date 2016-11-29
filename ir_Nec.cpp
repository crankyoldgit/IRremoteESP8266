
#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//                           N   N  EEEEE   CCCC
//                           NN  N  E      C
//                           N N N  EEE    C
//                           N  NN  E      C
//                           N   N  EEEEE   CCCC
//==============================================================================

#define NEC_BITS          32
#define NEC_HDR_MARK    9000
#define NEC_HDR_SPACE   4500
#define NEC_BIT_MARK     560
#define NEC_ONE_SPACE   1690 // 3T
#define NEC_ZERO_SPACE   560 // 1T
#define NEC_RPT_SPACE  40000 

// [Data NEC] 	   8 address bits + 8 inverted address bits + 8 command bits + 8 inverted command bits
// [Extended NEC] 16 address bits + 8 command bits + 8 inverted command bits
// NEC, Pioneer, JVC, Toshiba, NoName etc. 

//+=============================================================================
#if SEND_NEC

void IRsend::sendNEC(unsigned long data, int nbits) { 

  // Set IR carrier frequency
  enableIROut(38);
  // Header
  mark( NEC_HDR_MARK );
  space(NEC_HDR_SPACE);
  // Data
  for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
    mark(NEC_BIT_MARK); 
    space_encode(data & mask, NEC_ONE_SPACE, NEC_ZERO_SPACE); 
  }
  // Footer
  mark( NEC_BIT_MARK  );  
  // Repeat
  space(NEC_RPT_SPACE );     
  mark( NEC_HDR_MARK  );
  space(NEC_ZERO_SPACE);
}

// Calculate Raw data based on  address and commnd .  
unsigned long IRsend::encodeNEC(unsigned int address ,unsigned  int command ) {
   if ( address>0xFF ) {
      // Extended NEC 
      return (address << 16) + (command <<  8) + (command ^ 0xFF); 
   }
   return    (address << 24) + ((address ^ 0xFF) << 16) + (command <<  8) + (command ^ 0xFF); 
}

void IRsend::send_addressNEC(unsigned int address ,unsigned  int command,int nbits) { 
  return sendNEC(encodeNEC( address, command ), nbits) ; 
} 

#endif
//+=============================================================================
// NECs have a repeat only 4 items long
//
#if DECODE_NEC
bool IRrecv::decodeNEC(decode_results *results) {
  if (irparams.rawlen < 2 * NEC_BITS + 1 + OFFSET_START) return false; 
  unsigned   long data = 0;
  int offset = OFFSET_START; 
  
  // Header 
  if (!MATCH_MARK(results->rawbuf[offset++],  NEC_HDR_MARK )) return false; 
  if (!MATCH_SPACE(results->rawbuf[offset++], NEC_HDR_SPACE)) return false;  
  // Data 
  for (int i = 0; i < NEC_BITS; i++) {
    if (!MATCH_MARK(results->rawbuf[offset++], NEC_BIT_MARK))                        return false;
    if (!space_decode(data,results->rawbuf[offset++],NEC_ONE_SPACE,NEC_ZERO_SPACE))  return false;
  }
  // Footer 
  if (!MATCH_MARK( results->rawbuf[offset++], NEC_BIT_MARK  ))  return false;
  if (!MATCH_SPACE(results->rawbuf[offset++], NEC_RPT_SPACE ))  return false;
  // Repeat check optional sometimes 
  //if (!MATCH_MARK( results->rawbuf[offset++], NEC_HDR_MARK  ))  
  //if (!MATCH_SPACE(results->rawbuf[offset++], NEC_ZERO_SPACE))  
  unsigned  int address = (data & 0xFF000000) >> 24;  // Most significant  two bytes if [ Data NEC ]
  unsigned  int command = (data & 0xFF00)     >>  8;  
  
  // Integrity check command , command is sent twice , one as plain and other inverted .  
  if (!(((unsigned  int ) command ^ 0xFF ) == ((unsigned  int )(data & 0xFF )))) {
    DBG_PRINTLN("NEC Integrity fails, it is not NEC protocol ") ; 
    return false;
  } 
  if ( !(((unsigned  int ) address ^ 0xFF ) == ((unsigned  int )((data & 0xFF0000) >> 16))))  {
    // [Extended NEC] 
    address = (data & 0xFFFF0000) >> 16;  // Most significant four bytes 
    DBG_PRINTLN("NEC Extended"); 
  } 
 
  // Success
  results->bits        = NEC_BITS;
  results->value       = data;
  results->command     = command ; 
  results->address     = address ; 
  results->decode_type = NEC;
  return true;
}
#endif
