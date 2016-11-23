
#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//                           SSSS   OOO   N   N  Y   Y
//                          S      O   O  NN  N   Y Y
//                           SSS   O   O  N N N    Y
//                              S  O   O  N  NN    Y
//                          SSSS    OOO   N   N    Y
//==============================================================================

#define SONY_BITS                   12
#define SONY_HDR_MARK             2400  //4T
#define SONY_SPACE                 600  //T
#define SONY_ONE_MARK             1200
#define SONY_ZERO_MARK             600
#define SONY_RPT_LENGTH          25000
//#define SONY_DOUBLE_SPACE_USECS    500  // usually ssee 713 - not using ticks as get number wrapround
#define MIN_REPEAT                   2

//+=============================================================================
#if SEND_SONY
void  IRsend::sendSony (unsigned long data,  int nbits) {
 // Set IR carrier frequency
  enableIROut(40);
  
  for (int j = 0; j < MIN_REPEAT; j++) { 
    
    // Header
    mark(SONY_HDR_MARK);

    // Data
    for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
      space(SONY_SPACE);
      mark_encode(data & mask, SONY_ONE_MARK,SONY_ZERO_MARK ) ;
    }
    space(SONY_RPT_LENGTH); 
  } 
}
#endif

//+=============================================================================
#if DECODE_SONY


int getBit(unsigned long value , int bit ) {
return  value >>bit & 1; 
}

bool IRrecv::decodeSony(decode_results *results) {
  //SONY protocol, SIRC (Serial Infra-Red Control) can be  12,15,20 bits long 
  unsigned long data  = 0;
  int offset = OFFSET_START; 
  int nbits  = 0; 
  //header 
  if (!MATCH_MARK(results->rawbuf[offset++], SONY_HDR_MARK))    return false; //START MARK 4T
  // Data bits   
  while (offset+1 < irparams.rawlen) {
    if (results->rawbuf[offset]>SONY_RPT_LENGTH*.7)             break ; //found repeat space 
    if (!MATCH_SPACE(results->rawbuf[offset++], SONY_SPACE))                         return false; //SPACE  1T
    if (!mark_decode(data,results->rawbuf[offset++],SONY_ONE_MARK,SONY_ZERO_MARK))   return false;
    nbits++; 
  }
  if (!((12 == nbits )||(15 == nbits )||(20 == nbits ))) {
    return false;
  } 
  // spare command and address from data 
  unsigned int  command  = 0;
  unsigned int  address   = 0;
  // reverse order 
  for (int i =0; i <nbits; i++) {
     // 12-bit version, 7 command bits, 5 address bits.
     if ( nbits == 12 )  { 
       if (i >= 5 && i <=11)  {     
         addBit(command,getBit(data,i));
       } else if (i >=0 &&  i <= 4) {    
         addBit(address,getBit(data,i)); 
       }
    // 15-bit version, 7 command bits, 8 address bits.
    } else if ( nbits == 15 )  { 
       if (i >= 8 && i <=14)  {    
         addBit(command,getBit(data,i));
       } else if (i >=0 && i <= 7) {   
         addBit(address,getBit(data,i)); 
       }
    // 20-bit version, 7 command bits, 5 address bits, 8 extended bits. 
    } else if ( nbits == 20)  { 
       if ((i >= 0 && i <=7) ||(i >= 13 && i <=19)) {    
         addBit(command,getBit(data,i));
       } else if (i >=8 && i <= 12) {   
         addBit(address,getBit(data,i)); 
       }
    } 
  } 
  // Success
  results->bits        = nbits ; 
  results->value       = data;
  results->command     = command; 
  results->address     = address; 
  results->decode_type = SONY;
  return true;
}
#endif
