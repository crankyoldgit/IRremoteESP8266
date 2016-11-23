#include "IRremote.h"
#include "IRremoteInt.h"


//==============================================================================
//                      SSSS   AAA   N   N  Y   Y   OOO
//                     S      A   A  NN  N   Y Y   O   O
//                      SSS   AAAAA  N N N    Y    O   O
//                         S  A   A  N  NN    Y    O   O
//                     SSSS   A   A  N   N    Y     OOO
//==============================================================================

  /*
  * Nov2016 marcosamarinho as the old implementation was specifc and had errors  
  *
  *  changed based on  http://slydiman.narod.ru/scr/kb/sanyo.htm 
  * This protocol uses the NEC protocol timings. However, data is formatted as:
  *	13 bits Custom Code
  *	13 bits NOT(Custom Code)
  *	 8 bits Key data
  *	 8 bits NOT(Key data)
  *
  * According with LIRC, this protocol is used on Sanyo, Aiwa and Chinon
  *  http://lirc.sourceforge.net/remotes/aiwa/RC-5VP05
  *  pre_data       0x3F8003F                // address(first 13 bits ) --> 0x1FC0
  *  KEY_POWER      0x00FF  #  Was: POWER    // command(first  8 bits ) --> 0x00
  * Information for this protocol is available at the Sanyo LC7461 datasheet. 
  */

#define SANYO_BITS                42  // (13+!13,8+!8) 
#define SANYO_HDR_MARK	        9000  // same Header as NEC times , so with 42 bits lenght 
#define SANYO_HDR_SPACE	        4500
#define SANYO_BIT_MARK	         560
#define SANYO_ONE_SPACE         1690  //3T 
#define SANYO_ZERO_SPACE         560  //1T
#define SANYO_RPT_SPACE         2360  //107897 on RC-5VP05

//+=============================================================================
#if DECODE_SANYO


bool IRrecv::decodeSanyo(decode_results *results) {
 if (irparams.rawlen < 2 * SANYO_BITS + 1+OFFSET_START) return false;
  unsigned long long data = 0; // 48 bits need long long
  int offset = OFFSET_START;   // Skip first space 
   
  // Header
  if (!MATCH_MARK( results->rawbuf[offset++], SANYO_HDR_MARK))   return false; 
  if (!MATCH_SPACE(results->rawbuf[offset++], SANYO_HDR_SPACE))  return false;
  int bits=0;
  // Data 
  while (offset + 1 < irparams.rawlen) {
    if (!MATCH_MARK(results->rawbuf[offset++], SANYO_BIT_MARK)) break;
    if (!space_decode(data,results->rawbuf[offset++],SANYO_ONE_SPACE, SANYO_ZERO_SPACE) ) return false;
    bits++; 
  }
  if (bits< SANYO_BITS) return false;
  int command ; 
  int address ; 
  int not_command ; 
  int not_address ; 
  not_command = data & 0xFFLL; 
  command     = data>>(     8) & 0xFFLL; 
  address     = data>>(13+8+8);
  not_address = data>>(   8+8) & 0x1FFF ; 
  // Checksum
  if (!(command ^ 0xFF == not_command) || !(address ^ 0x1FFF == not_address)) {
    Serial.println(" *** SANYO checksum error:") ; 
    return false ; 
  }
  // Success
  results->command     = command; 
  results->address     = address; 
  results->bits        = SANYO_BITS; 
  results->value       = data;
  results->decode_type = SANYO;
  return true;
}
#endif
#if SEND_SANYO

void IRsend::sendSanyo(unsigned int address , unsigned int  command) {
  enableIROut(38,3); //dutycycle 33% ; 
  // Header 
  mark(SANYO_HDR_MARK);
  space(SANYO_HDR_SPACE);
  unsigned long long rawData=0; 
  // Address 
  for (unsigned long  mask = 1UL << (13 - 1); mask;  mask >>= 1)  addBit(rawData,address & mask  );  
   // NOT Address 
  for (unsigned long  mask = 1UL << (13 - 1); mask;  mask >>= 1) addBit(rawData,!(address & mask));
  // Command 
  for (unsigned long  mask = 1UL << (8 - 1);  mask;  mask >>= 1) addBit(rawData,address & mask   );
  // NOT Command 
  for (unsigned long  mask = 1UL << (8 - 1);  mask;  mask >>= 1) addBit(rawData,!(address & mask)); 
  // Send rawData
  for (unsigned long long mask = 1ULL << (SANYO_BITS - 1); mask;  mask >>= 1)  {
    mark(SANYO_BIT_MARK); 
    space_encode(rawData,SANYO_ONE_SPACE,SANYO_ZERO_SPACE); 
  }
  // Footer 
  mark(SANYO_BIT_MARK);        
  space(SANYO_RPT_SPACE);
}
#endif

