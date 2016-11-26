
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
#define NEC_ONE_SPACE   1690 //3T
#define NEC_ZERO_SPACE   560 //1T
#define NEC_RPT_SPACE  40000 


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

// Calculate data based on  address and commnd .  
unsigned long IRsend::encodeNEC(unsigned int address ,unsigned  int command ) {
   return ( address << 24) + ((address ^ 0xFF) << 16) + ( command <<  8) + (command ^ 0xFF); 
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
  // Full Stop 
  if (!MATCH_MARK( results->rawbuf[offset++], NEC_BIT_MARK  ))  return false;
  if (!MATCH_SPACE(results->rawbuf[offset++], NEC_RPT_SPACE ))  return false;
  // Repeat optional 
  //if (!MATCH_MARK( results->rawbuf[offset++], NEC_HDR_MARK  ))  
  //if (!MATCH_SPACE(results->rawbuf[offset++], NEC_ZERO_SPACE))  
  unsigned  int address =0; 
  unsigned  int command =0; 
  address = (data & 0xFF000000) >> 24;  // Most significant  two bytes 
  command = (data & 0xFF00)     >>  8;  // byte 4 and 3 
  // integrity check command ,  address and command is sent twice , one as plain and other inverted .  
  if ( !( address ^ 0xFF == ((data & 0xFF0000) >> 16) ) ||!( command ^ 0xFF == (data & 0xFF) ))  {
    #ifdef DEBUG
    Serial.println("NEC Integrity fails, it is not a true NEC  ") ; 
    #endif 
    return false;
  } 
  if (!MATCH_MARK(results->rawbuf[offset++], NEC_BIT_MARK))  
  // Success
  results->bits        = NEC_BITS;
  results->value       = data;
  results->command     = command ; 
  results->address     = address ; 
  results->decode_type = NEC;
  
  //Decode address,command to use latter as send input 
  //Serial.println(String(rawNEC(address,command),HEX)); 
 
  return true;
}
#endif
