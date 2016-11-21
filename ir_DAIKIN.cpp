#include "IRremote.h"
#include "IRremoteInt.h"


// Dakin, from https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/tree/master/IRremote

#define DAIKIN_BITS               99
#define DAIKIN_HDR_MARK	        3650 //DAIKIN_ZERO_MARK*8
#define DAIKIN_HDR_SPACE        1623 //DAIKIN_ZERO_MARK*4
#define DAIKIN_ONE_SPACE        1280 
#define DAIKIN_ONE_MARK	         428
#define DAIKIN_ZERO_MARK         428
#define DAIKIN_ZERO_SPACE        428

// From https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/tree/master/IRremote
void IRsend::sendDaikin(unsigned char daikin[]) {
  sendDaikinChunk(daikin, 8,0);
  delay(29);
  sendDaikinChunk(daikin, 19,8);
}

void IRsend::sendDaikinChunk(unsigned char buf[], int len, int start) {
  int data2;
  enableIROut(38);
  mark(DAIKIN_HDR_MARK);
  space(DAIKIN_HDR_SPACE);
  for (int i = start; i < start+len; i++) {
    data2=buf[i];
    for (int j = 0; j < 8; j++) {
      mark(DAIKIN_ONE_MARK);
      space_encode((1 << j & data2),DAIKIN_ONE_SPACE , DAIKIN_ZERO_SPACE) ; 
    }
  }
  mark(DAIKIN_ONE_MARK);
  space(DAIKIN_ZERO_SPACE);
}

// From https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/tree/master/IRremote
// decoding not actually tested
bool IRrecv::decodeDaikin(decode_results *results) {
  unsigned long data  = 0;
  int offset = OFFSET_START; // Skip first space
  if (irparams.rawlen < 2 * DAIKIN_BITS + 3 + OFFSET_START)  return false;
  // Header 
  if (!MATCH_MARK(results->rawbuf[offset++] , DAIKIN_HDR_MARK))   return false;
  if (!MATCH_SPACE(results->rawbuf[offset++], DAIKIN_HDR_SPACE))  return false;
  //Data bits 
  for (int i = 0; i < 32; i++) {
    if (!MATCH_MARK(results->rawbuf[offset++], DAIKIN_ONE_MARK))  return false;
    if (!space_decode(data,results->rawbuf[offset++],DAIKIN_ONE_SPACE,DAIKIN_ZERO_SPACE))  return false;
  }
  unsigned long number = data ; // some number...
  int bits = 32 ; // nr of bits in some number
  unsigned long reversed = 0;
  for ( int b=0 ; b < bits ; b++ ) reversed = ( reversed << 1 ) | ( 0x0001 & ( number >> b ) );
  //Serial.print ("Code ");Serial.println (reversed,  HEX);
  //Data bits 
  for (int i = 0; i < 32; i++) {
    if (!MATCH_MARK(results->rawbuf[offset++], DAIKIN_ONE_MARK))   return false;
    if (!space_decode(data,results->rawbuf[offset++],DAIKIN_ONE_SPACE,DAIKIN_ZERO_SPACE))  return false;
  }
  number   = data ; // some number...
  bits     =  32  ;   // nr of bits in some number
  reversed =   0  ;
  for ( int b=0 ; b < bits ; b++ ) reversed = ( reversed << 1 ) | ( 0x0001 & ( number >> b ) );
  //Serial.print ("Code2 ");Serial.println (reversed,  HEX);
  if (!MATCH_SPACE(results->rawbuf[offset++], 29000)) {
    //Serial.println ("no gap");
    return false;
  }
  // Success
  results->bits        = DAIKIN_BITS;
  results->value       = reversed;
  results->decode_type = DAIKIN;
  return true;
}
