
#include "IRremote.h"
#include "IRremoteInt.h"

// Global Cache format w/o emitter ID or request ID. Starts from hertz, 
// followed by number of times to emit (count),
// followed by offset for repeats, followed by code as units of periodic time.
#if SEND_GC
void IRsend::sendGC(unsigned int buf[], int len) {
  int khz = buf[0]/1000; // GC data starts with frequency in Hz.
  enableIROut(khz); 
  int periodic_time = 1000/khz;
  int count = buf[1]; // Max 50 as per GC.
  for (int i = 0; i < count; i++) {
    int j = i > 0 ? buf[2] + 2 : 3; // Account for offset if we're repeating, otherwise start at index 3.
    for (; j < len; j++) {
      int microseconds = buf[j] * periodic_time; // Convert periodic units to microseconds. Minimum is 80 for actual GC units.
      if (j & 1) {
        mark(microseconds); // Our codes start at an odd index (not even as with sendRaw).
      } else {
        space(microseconds);
      }
    }
  }
  space(0);
}
#endif
