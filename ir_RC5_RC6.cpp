#include "IRremote.h"
#include "IRremoteInt.h"
//#define VERBOSE   1
//+=============================================================================
// Gets one undecoded level at a time from the raw buffer.
// The RC5/6 decoding is easier if the data is broken into time intervals.
// E.g. if the buffer has MARK for 2 time intervals and SPACE for 1,
// successive calls to getRClevel will return MARK, MARK, SPACE.
// offset and used are updated to keep track of the current position.
// t1 is the time interval for a single bit in microseconds.
// Returns -1 for error (measured time interval is not a multiple of t1).
//
#if (DECODE_RC5 || DECODE_RC6)
int  IRrecv::getRClevel (decode_results *results,  int *offset,  int *used,  int t1)
{
	int  width;
	int  val;
	int  correction;
	int  avail;

	if (*offset >= results->rawlen)  return SPACE ;  // After end of recorded buffer, assume SPACE.
	width      = results->rawbuf[*offset];
	val        = ((*offset) % 2) ? MARK : SPACE;
        if (val ==MARK) {
	  if      (match_mark_nolog(width,   t1 ))  avail = 1 ;
	  else if (match_mark_nolog(width, 2*t1 ))  avail = 2 ;
	  else if (match_mark_nolog(width, 3*t1 ))  avail = 3 ;
	  else {
             #if DEBUG 
            Serial.print("MARK RC5 or RC6 expected T,2T,3T T=" ) ; 
            Serial.print(t1) ; 
            Serial.print(" Measured : " ) ; 
            Serial.println(width) ; 
            #endif
            return -1 ;
          }
	} else {
	  if      (match_space_nolog(width,   t1 ))  avail = 1 ;
	  else if (match_space_nolog(width, 2*t1 ))  avail = 2 ;
	  else if (match_space_nolog(width, 3*t1 ))  avail = 3 ;
	  else {
            #if DEBUG 
            Serial.print("SPACE RC5 or RC6 expected T,2T,3T T=" ) ; 
            Serial.print(t1) ; 
            Serial.print(" Measured : " ) ; 
            Serial.println(width) ; 
            #endif
            return -1 ;
          }
        }
	(*used)++;
	if (*used >= avail) {
	  *used = 0;
	  (*offset)++;
          #if DEBUG 
          #if VERBOSE
  	  DBG_PRINT( "USED" );
 	  DBG_PRINT( (val == MARK) ? " MARK " : " SPACE " );
          DBG_PRINT( avail );
          DBG_PRINTLN( "T" );
          #endif
          #endif
	} else {
          #if DEBUG 
          #if VERBOSE
          DBG_PRINT( "REMAINING" );
          DBG_PRINTLN( (val == MARK) ? " MARK " : " SPACE " );
          #endif
          #endif
        }
	return val;
}
#endif

//==============================================================================
// RRRR    CCCC  55555
// R   R  C      5
// RRRR   C      5555
// R  R   C          5
// R   R   CCCC  5555
//
// NB: First bit must be a one (start bit)
//
#define MIN_RC5_SAMPLES     11
#define RC5_T1             889
#define RC5_RPT_LENGTH   46000

//+=============================================================================
#if SEND_RC5
void  IRsend::sendRC5 (unsigned long data,  int nbits)
{
	// Set IR carrier frequency
	enableIROut(36);

	// Start
	mark( RC5_T1);
	space(RC5_T1);
	mark( RC5_T1);

	// Data
	for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
		if (data & mask) {
			space(RC5_T1); // 1 is space, then mark
			mark( RC5_T1);
		} else {
			mark( RC5_T1);
			space(RC5_T1);
		}
	}

	space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
#if DECODE_RC5
bool  IRrecv::decodeRC5 (decode_results *results)
{
	int   nbits;
	long  data   = 0;
	int   used   = 0;
	int   offset = OFFSET_START;  // Skip gap space

	if (irparams.rawlen < MIN_RC5_SAMPLES + 1 + OFFSET_START)  return false ;

	// Get start bits
	if (getRClevel(results, &offset, &used, RC5_T1) != MARK)   return false ;
	if (getRClevel(results, &offset, &used, RC5_T1) != SPACE)  return false ;
	if (getRClevel(results, &offset, &used, RC5_T1) != MARK)   return false ;

	for (nbits = 0;  offset < irparams.rawlen;  nbits++) {
		int  levelA = getRClevel(results, &offset, &used, RC5_T1);
		int  levelB = getRClevel(results, &offset, &used, RC5_T1);

		if      ((levelA == SPACE) && (levelB == MARK ))  data = (data << 1) | 1 ;
		else if ((levelA == MARK ) && (levelB == SPACE))  data = (data << 1) | 0 ;
		else                                              return false ;
	}

	// Success
	results->bits        = nbits;
	results->value       = data;
	results->decode_type = RC5;
	return true;
}
#endif

//+=============================================================================
// RRRR    CCCC   6666
// R   R  C      6
// RRRR   C      6666
// R  R   C      6   6
// R   R   CCCC   666
//
// NB : Caller needs to take care of flipping the toggle bit
//
#define MIN_RC6_SAMPLES      1
#define RC6_HDR_MARK      2666
#define RC6_HDR_SPACE      889
#define RC6_T1             444
#define RC6_RPT_LENGTH   46000

#if SEND_RC6
void  IRsend::sendRC6 (unsigned long data,  int nbits)
{
	// Set IR carrier frequency
	enableIROut(36);

	// Header
	mark(RC6_HDR_MARK);
	space(RC6_HDR_SPACE);

	// Start bit
	mark(RC6_T1);
	space(RC6_T1);

	// Data
	for (unsigned long  i = 1, mask = 1UL << (nbits - 1);  mask;  i++, mask >>= 1) {
		// The fourth bit we send is a "double width trailer bit"
		int  t = (i == 4) ? (RC6_T1 * 2) : (RC6_T1) ;
		if (data & mask) {
			mark(t);
			space(t);
		} else {
			space(t);
			mark(t);
		}
	}

	space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
#if DECODE_RC6
bool  IRrecv::decodeRC6 (decode_results *results)
{
	int   nbits;
	long  data   = 0;
	int   used   = 0;
	int   offset = OFFSET_START;  // Skip first space

	if (results->rawlen < MIN_RC6_SAMPLES)  return false ;

	// Initial mark
	if (!MATCH_MARK(results->rawbuf[offset++],  RC6_HDR_MARK))   return false ;
	if (!MATCH_SPACE(results->rawbuf[offset++], RC6_HDR_SPACE))  return false ;

	// Get start bit (1)
	if (getRClevel(results, &offset, &used, RC6_T1) != MARK)   return false ;
	if (getRClevel(results, &offset, &used, RC6_T1) != SPACE)  return false ;

	for (nbits = 0;  offset < results->rawlen;  nbits++) {
		int  levelA, levelB;  // Next two levels

		levelA = getRClevel(results, &offset, &used, RC6_T1);
		if (nbits == 3) {
			// T bit is double wide; make sure second half matches
			if (levelA != getRClevel(results, &offset, &used, RC6_T1)) return false;
		}

		levelB = getRClevel(results, &offset, &used, RC6_T1);
		if (nbits == 3) {
			// T bit is double wide; make sure second half matches
			if (levelB != getRClevel(results, &offset, &used, RC6_T1)) return false;
		}

		if      ((levelA == MARK ) && (levelB == SPACE))  data = (data << 1) | 1 ;  // inverted compared to RC5
		else if ((levelA == SPACE) && (levelB == MARK ))  data = (data << 1) | 0 ;  // ...
		else                                              return false ;            // Error
	}

	// Success
	results->bits        = nbits;
	results->value       = data;
	results->decode_type = RC6;
	return true;
}
#endif
