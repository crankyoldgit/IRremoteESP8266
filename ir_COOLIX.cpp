#include "IRremote.h"
#include "IRremoteInt.h"

#define COOLIX_NBYTES             3
#define COOLIX_BIT_MARK         560       // Approximately 21 cycles at 38kHz
#define COOLIX_ONE_SPACE	COOLIX_BIT_MARK * 3
#define COOLIX_ZERO_SPACE	COOLIX_BIT_MARK * 1
#define COOLIX_HDR_MARK	        COOLIX_BIT_MARK * 8
#define COOLIX_HDR_SPACE	COOLIX_BIT_MARK * 8

#if SEND_COOLIX
void IRsend::sendCOOLIX(unsigned long data, int nbits)  {
  enableIROut(38);
  mark(COOLIX_HDR_MARK);
  space(COOLIX_HDR_SPACE);
  // Sending 3 bytes of data. Each byte first beeing sendt straight, then followed by an inverted version.
  unsigned long COOLIXmask;
  bool invert = 0;  // Initializing
  for (int j = 0; j < COOLIX_NBYTES * 2; j++) {
    for (int i = nbits; i > nbits-8; i--) {
      COOLIXmask = (unsigned long) 1 << (i-1);  // Type cast necessary to perform correct for the one byte above 16bit
      mark(COOLIX_BIT_MARK);
      space_encode(data & COOLIXmask, COOLIX_ONE_SPACE, COOLIX_ZERO_SPACE);
    }
    data  ^= 0xFFFFFFFF;     // Inverts all of the data each time we need to send an inverted byte
    invert = !invert;
    nbits -= invert ? 0 : 8;  // Subtract 8 from nbits each time we switch to a new byte.
  }
  mark(COOLIX_BIT_MARK);
  space(COOLIX_ZERO_SPACE);   // Stop bit (0)
  space(COOLIX_HDR_SPACE);    // Pause before repeating
}
#endif
