 /***************************************************
 * IRremote for ESP8266
 *
 * Based on the IRremote library for Arduino by Ken Shirriff
 * Version 0.11 August, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see
 *   http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 * Modified by Paul Stoffregen <paul@pjrc.com> to support other boards and
 *   timers
 * Modified by Mitra Ardron <mitra@mitra.biz>
 * Added Sanyo and Mitsubishi controllers
 * Modified Sony to spot the repeat codes that some Sony's send
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by
 *   http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 *
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and
 *   other people at the original blog post)
 * LG added by Darryl Smith (based on the JVC protocol)
 * Whynter A/C ARC-110WD added by Francesco Meschia
 * Global Cache IR format sender added by Hisham Khalifa
 *   (http://www.hishamkhalifa.com)
 * Coolix A/C / heatpump added by bakrus
 * Denon: sendDenon, decodeDenon added by Massimiliano Pinto
 *   (from https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Denon.cpp)
 * Kelvinator A/C and Sherwood added by crankyoldgit
 * Mitsubishi A/C added by crankyoldgit
 *     (derived from https://github.com/r45635/HVAC-IR-Control)
 *
 * Updated by markszabo (https://github.com/markszabo/IRremoteESP8266) for
 *   sending IR code on ESP8266
 * Updated by Sebastien Warin (http://sebastien.warin.fr) for receiving IR code
 *   on ESP8266
 *
 *  GPL license, all text above must be included in any redistribution
 ****************************************************/

#include "IRremoteESP8266.h"
#include "IRremoteInt.h"
#include "IRDaikinESP.h"
#include "IRKelvinator.h"
#include "IRMitsubishiAC.h"

// IRtimer ---------------------------------------------------------------------
// This class performs a simple time in useconds since instantiated.
// Handles when the system timer wraps around (once).

IRtimer::IRtimer() {
  reset();
}

void ICACHE_FLASH_ATTR IRtimer::reset() {
  start = micros();
}

uint32_t ICACHE_FLASH_ATTR IRtimer::elapsed() {
  uint32_t now = micros();
  if (start <= now)  // Check if the system timer has wrapped.
    return (now - start);  // No wrap.
  else
    return (0xFFFFFFFF - start + now);  // Has wrapped.
}

// IRsend ----------------------------------------------------------------------

IRsend::IRsend(int IRsendPin) {
	IRpin = IRsendPin;
}

void ICACHE_FLASH_ATTR IRsend::begin() {
	pinMode(IRpin, OUTPUT);
  ledOff();  // Turn off the IR LED just to be safe.
}

// Generic method for sending data that is common to most protocols.
// Default to transmitting the Most Significant Bit (MSB) first.
void ICACHE_FLASH_ATTR IRsend::sendData(uint16_t onemark, uint32_t onespace,
                                        uint16_t zeromark, uint32_t zerospace,
                                        uint32_t data, uint8_t nbits,
                                        bool MSBfirst) {
  if (MSBfirst)  // Send the MSB first.
    for (uint32_t mask = 1UL << (nbits - 1);  mask;  mask >>= 1)
      if (data & mask) {  // 1
        mark(onemark);
        space(onespace);
      } else {  // 0
        mark(zeromark);
        space(zerospace);
      }
  else {  // Send the Least Significant Bit (LSB) first / MSB last.
    for (uint8_t bit = 0; bit < nbits; bit++, data >>= 1)
      if (data & 1) {  // 1
        mark(onemark);
        space(onespace);
      } else {  // 0
        mark(zeromark);
        space(zerospace);
      }
  }
}

void ICACHE_FLASH_ATTR IRsend::sendCOOLIX(unsigned long data, int nbits) {
  // Set IR carrier frequency
  enableIROut(38);
  // Header
  mark(COOLIX_HDR_MARK);
  space(COOLIX_HDR_SPACE);
  // Data
  // Sending 3 bytes of data. Each byte first being sent straight, then followed
  // by an inverted version.
  unsigned long COOLIXmask;
  bool invert = 0;  // Initializing
  for (int j = 0; j < COOLIX_NBYTES * 2; j++) {
    for (int i = nbits; i > nbits-8; i--) {
      // Type cast necessary to perform correct for the one byte above 16bit
      COOLIXmask = (unsigned long) 1 << (i-1);
      if (data & COOLIXmask) {  // 1
        mark(COOLIX_BIT_MARK);
        space(COOLIX_ONE_SPACE);
      } else {  // 0
        mark(COOLIX_BIT_MARK);
        space(COOLIX_ZERO_SPACE);
      }
    }
    // Inverts all of the data each time we need to send an inverted byte
    data ^= 0xFFFFFFFF;
    invert = !invert;
    // Subtract 8 from nbits each time we switch to a new byte.
    nbits -= invert ? 0 : 8;
  }
  // Footer
  mark(COOLIX_BIT_MARK);
  space(COOLIX_ZERO_SPACE);   // Stop bit (0)
  space(COOLIX_HDR_SPACE);    // Pause before repeating
}

void ICACHE_FLASH_ATTR IRsend::sendNEC (unsigned long data, int nbits,
                                        unsigned int repeat) {
  // Details about timings can be found at:
  //   http://www.sbprojects.com/knowledge/ir/nec.php

  // Set IR carrier frequency
  enableIROut(38);
  IRtimer usecs = IRtimer();
  // Header
  mark(NEC_HDR_MARK);
  space(NEC_HDR_SPACE);
  // Data
  sendData(NEC_BIT_MARK, NEC_ONE_SPACE, NEC_BIT_MARK, NEC_ZERO_SPACE,
           data, nbits, true);
  // Footer
  mark(NEC_BIT_MARK);
  // Gap to next command.
  space(max(0, NEC_MIN_COMMAND_LENGTH - usecs.elapsed()));

  // Optional command repeat sequence.
  for (unsigned int i = 0; i < repeat; i++) {
    usecs.reset();
    mark(NEC_HDR_MARK);
    space(NEC_RPT_SPACE);
    mark(NEC_BIT_MARK);
    // Gap till next command.
    space(max(0, NEC_MIN_COMMAND_LENGTH - usecs.elapsed()));
  }
}

void ICACHE_FLASH_ATTR IRsend::sendLG (unsigned long data, int nbits,
                                       unsigned int repeat) {
  // Args:
  //   data:   The contents of the command you want to send.
  //   nbits:  The bit size of the command being sent.
  //   repeat: The number of times you want the command to be repeated.

  // Set IR carrier frequency
  enableIROut(38);
  // We always send a command, even for repeat=0, hence '<= repeat'.
  for (unsigned int i = 0; i <= repeat; i++) {
    // Header
    mark(LG_HDR_MARK);
    space(LG_HDR_SPACE);
    // Data
    sendData(LG_BIT_MARK, LG_ONE_SPACE, LG_BIT_MARK, LG_ZERO_SPACE,
             data, nbits, true);
    // Footer
    mark(LG_BIT_MARK);
    space(LG_RPT_LENGTH);
  }
}

void ICACHE_FLASH_ATTR IRsend::sendWhynter(unsigned long data, int nbits) {
  // Set IR carrier frequency
  enableIROut(38);
  // Header
  mark(WHYNTER_ZERO_MARK);
  space(WHYNTER_ZERO_SPACE);
  mark(WHYNTER_HDR_MARK);
  space(WHYNTER_HDR_SPACE);
  // Data
  sendData(WHYNTER_ONE_MARK, WHYNTER_ONE_SPACE, WHYNTER_ZERO_MARK,
           WHYNTER_ZERO_SPACE, data, nbits, true);
  // Footer
  mark(WHYNTER_ZERO_MARK);
  space(WHYNTER_ZERO_SPACE);
}

void ICACHE_FLASH_ATTR IRsend::sendSony(unsigned long data, int nbits,
                                        unsigned int repeat) {
  // Send an IR command to a compatible Sony device.
  //
  // Args:
  //   data: IR command to be sent.
  //   nbits: Nr. of bits of the IR command to be sent.
  //   repeat: Nr. of additional times the IR command is to be sent.
  //
  // sendSony() should typically be called with repeat=2 as Sony devices
  // expect the code to be sent at least 3 times.
  //
  // Timings and details are taken from:
  //   http://www.sbprojects.com/knowledge/ir/sirc.php

  enableIROut(40);  // Sony devices use a 40kHz IR carrier frequency.
  IRtimer usecs = IRtimer();

  for (uint16_t i = 0; i <= repeat; i++) {  // Typically loop 3 or more times.
    usecs.reset();
    // Header
    mark(SONY_HDR_MARK);
    space(SONY_HDR_SPACE);
    // Data
    sendData(SONY_ONE_MARK, SONY_HDR_SPACE, SONY_ZERO_MARK, SONY_HDR_SPACE,
             data, nbits, true);
    // Footer
    // The Sony protocol requires us to wait 45ms from start of a code to the
    // start of the next one. A 10ms minimum gap is also required.
    space(max(10000, 45000 - usecs.elapsed()));
  }
  // A space() is always performed last, so no need to turn off the LED.
}

void ICACHE_FLASH_ATTR IRsend::sendRaw(unsigned int buf[], int len, int hz) {
  // Set IR carrier frequency
  enableIROut(hz);
  for (int i = 0; i < len; i++) {
    if (i & 1) { // Odd bit.
      space(buf[i]);
    } else {  // Even bit.
      mark(buf[i]);
    }
  }
  ledOff();
}

// Global Cache format w/o emitter ID or request ID. Starts from hertz,
// followed by number of times to emit (count),
// followed by offset for repeats, followed by code as units of periodic time.
void ICACHE_FLASH_ATTR IRsend::sendGC(unsigned int buf[], int len) {
  int khz = buf[0]/1000; // GC data starts with frequency in Hz.
  enableIROut(khz);
  int periodic_time = 1000/khz;
  int count = buf[1]; // Max 50 as per GC.
  // Data
  for (int i = 0; i < count; i++) {
    // Account for offset if we're repeating, otherwise start at index 3.
    int j = i > 0 ? buf[2] + 2 : 3;
    for (; j < len; j++) {
      // Convert periodic units to microseconds. Minimum is 80 for actual GC
      // units.
      int microseconds = buf[j] * periodic_time;
      if (j & 1) {  // Odd bit.
        // Our codes start at an odd index (not even as with sendRaw).
        mark(microseconds);
      } else {  // Even bit.
        space(microseconds);
      }
    }
  }
  // Footer
  ledOff();
}

// Note: first bit must be a one (start bit)
void ICACHE_FLASH_ATTR IRsend::sendRC5(unsigned long data, int nbits) {
  // Set IR carrier frequency
  enableIROut(36);
  // Header
  mark(RC5_T1);  // First start bit
  space(RC5_T1);  // Second start bit
  mark(RC5_T1);  // Second start bit
  // Data
  for (unsigned long mask = 1UL << (nbits - 1); mask; mask >>= 1) {
    if (data & mask) {  // 1
      space(RC5_T1);  // 1 is space, then mark
      mark(RC5_T1);
    } else {  // 0
      mark(RC5_T1);
      space(RC5_T1);
    }
  }
  // Footer
  ledOff();
}

// Caller needs to take care of flipping the toggle bit
void ICACHE_FLASH_ATTR IRsend::sendRC6(unsigned long data, int nbits) {
  // Set IR carrier frequency
  enableIROut(36);
  // Header
  mark(RC6_HDR_MARK);
  space(RC6_HDR_SPACE);
  mark(RC6_T1);  // Start bit
  space(RC6_T1);
  int t;
  // Data
  for (unsigned long i = 0, mask = 1UL << (nbits - 1); mask; i++, mask >>= 1) {
    // The fourth bit we send is a "double width trailer bit".
    if (i == 3) {
      // double-wide trailer bit
      t = 2 * RC6_T1;
    } else {
      t = RC6_T1;
    }
    if (data & mask) {  // 1
      mark(t);
      space(t);
    } else {  // 0
      space(t);
      mark(t);
    }
  }
  // Footer
  ledOff();
}

// Send a Philips RC-MM packet.
// Based on http://www.sbprojects.com/knowledge/ir/rcmm.php
// Args:
//   data: The data we want to send. MSB first.
//   nbits: The number of bits of data to send. (Typically 12, 24, or 32[Nokia])
// Status:  ALPHA (untested and unconfirmed.)
void ICACHE_FLASH_ATTR IRsend::sendRCMM(uint32_t data, uint8_t nbits) {
  // Set IR carrier frequency
  enableIROut(36);
  IRtimer usecs = IRtimer();

  // Header
  mark(RCMM_HDR_MARK);
  space(RCMM_HDR_SPACE);
  // Data
  uint32_t mask = B11 << (nbits - 2);
  // RC-MM sends data 2 bits at a time.
  for (uint8_t i = nbits; i > 0; i -= 2) {
    mark(RCMM_BIT_MARK);
    // Grab the next Most Significant Bits to send.
    switch ((data & mask) >> (i - 2)) {
      case B00: space(RCMM_BIT_SPACE_0); break;
      case B01: space(RCMM_BIT_SPACE_1); break;
      case B10: space(RCMM_BIT_SPACE_2); break;
      case B11: space(RCMM_BIT_SPACE_3); break;
    }
    mask >>= 2;
  }
  // Footer
  mark(RCMM_BIT_MARK);
  // Protocol requires us to wait at least RCMM_RPT_LENGTH usecs from the start
  // or RCMM_MIN_GAP usecs.
  space(max(RCMM_RPT_LENGTH - usecs.elapsed(), RCMM_MIN_GAP));
}

void ICACHE_FLASH_ATTR IRsend::sendPanasonic(unsigned int address,
                                             unsigned long data) {
  // Set IR carrier frequency
  enableIROut(37);
  // Header
  mark(PANASONIC_HDR_MARK);
  space(PANASONIC_HDR_SPACE);
  // Address (16 bits)
  sendData(PANASONIC_BIT_MARK, PANASONIC_ONE_SPACE,
           PANASONIC_BIT_MARK, PANASONIC_ZERO_SPACE,
           address, 16, true);
  // Data (32 bits)
  sendData(PANASONIC_BIT_MARK, PANASONIC_ONE_SPACE,
           PANASONIC_BIT_MARK, PANASONIC_ZERO_SPACE,
           data, 32, true);
  // Footer
  mark(PANASONIC_BIT_MARK);
  ledOff();
}

void ICACHE_FLASH_ATTR IRsend::sendJVC(unsigned long data, int nbits,
                                       unsigned int repeat) {
  // Args:
  //   data:   The contents of the command you want to send.
  //   nbits:  The bit size of the command being sent.
  //   repeat: The number of times you want the command to be repeated.
  //
  // Based on information at: http://www.sbprojects.com/knowledge/ir/jvc.php

  // Set IR carrier frequency
  enableIROut(38);

  IRtimer usecs = IRtimer();
  // Header
  mark(JVC_HDR_MARK);
  space(JVC_HDR_SPACE);

  // We always send the data & footer at least once, hence '<= repeat'.
  for (unsigned int i = 0; i <= repeat; i++) {
    // Data
    sendData(JVC_BIT_MARK, JVC_ONE_SPACE, JVC_BIT_MARK, JVC_ZERO_SPACE,
             data, nbits, true);
    // Footer
    mark(JVC_BIT_MARK);
    // Wait till the end of the repeat time window before we send another code.
    space(max(0, JVC_RPT_LENGTH - usecs.elapsed()));
    usecs.reset();
  }
  // No need to turn off the LED as we will always end with a space().
}

void ICACHE_FLASH_ATTR IRsend::sendSAMSUNG(unsigned long data, int nbits) {
  // Set IR carrier frequency
  enableIROut(38);
  // Header
  mark(SAMSUNG_HDR_MARK);
  space(SAMSUNG_HDR_SPACE);
  // Data
  sendData(SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_BIT_MARK,
           SAMSUNG_ZERO_SPACE, data, nbits, true);
  // Footer
  mark(SAMSUNG_BIT_MARK);
  ledOff();
}

// Denon, from https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Denon.cpp
void ICACHE_FLASH_ATTR IRsend::sendDenon (unsigned long data,  int nbits) {
  // Set IR carrier frequency
  enableIROut(38);
  // Header
  mark(DENON_HDR_MARK);
  space(DENON_HDR_SPACE);
  // Data
  sendData(DENON_BIT_MARK, DENON_ONE_SPACE, DENON_BIT_MARK, DENON_ZERO_SPACE,
           data, nbits, true);
  // Footer
  mark(DENON_BIT_MARK);
  ledOff();
}

void ICACHE_FLASH_ATTR IRsend::mark(unsigned int usec) {
  // Sends an IR mark for the specified number of microseconds.
  // The mark output is modulated at the PWM frequency.
  IRtimer usecTimer = IRtimer();
  while (usecTimer.elapsed() < usec) {
    digitalWrite(IRpin, HIGH);
    delayMicroseconds(halfPeriodicTime);
    digitalWrite(IRpin, LOW);
    // e.g. 38 kHz -> T = 26.31 microsec (periodic time), half of it is 13
    delayMicroseconds(halfPeriodicTime);
  }
}

void ICACHE_FLASH_ATTR IRsend::ledOff() {
  digitalWrite(IRpin, LOW);
}

/* Leave pin off for time (given in microseconds) */
void ICACHE_FLASH_ATTR IRsend::space(unsigned long time) {
  // Sends an IR space for the specified number of microseconds.
  // A space is no output, so the PWM output is disabled.
  ledOff();
  if (time == 0) return;
  if (time <= 16383)  // delayMicroseconds is only accurate to 16383us.
    delayMicroseconds(time);
  else {
    // Invoke a delay(), where possible, to avoid triggering the WDT.
    delay(time / 1000UL);  // Delay for as many whole ms as we can.
    delayMicroseconds((int) time % 1000UL);  // Delay the remaining sub-msecond.
  }
}

void ICACHE_FLASH_ATTR IRsend::enableIROut(int khz) {
  // Enables IR output.
  // The khz value controls the modulation frequency in kilohertz.

  // T = 1/f but we need T/2 in microsecond and f is in kHz
  halfPeriodicTime = 500/khz;
}


/* Sharp and DISH support by Todd Treece
( http://unionbridge.org/design/ircommand )

The Dish send function needs to be repeated 4 times, and the Sharp function
has the necessary repeat built in because of the need to invert the signal.

Sharp protocol documentation:
http://www.sbprojects.com/knowledge/ir/sharp.htm

Here are the LIRC files that I found that seem to match the remote codes
from the oscilloscope:

Sharp LCD TV:
http://lirc.sourceforge.net/remotes/sharp/GA538WJSA

DISH NETWORK (echostar 301):
http://lirc.sourceforge.net/remotes/echostar/301_501_3100_5100_58xx_59xx

For the DISH codes, only send the last for characters of the hex.
i.e. use 0x1C10 instead of 0x0000000000001C10 which is listed in the
linked LIRC file.
*/

void ICACHE_FLASH_ATTR IRsend::sendSharpRaw(unsigned long data, int nbits) {
  // Set IR carrier frequency
  enableIROut(38);
  // Sending codes in bursts of 3 (normal, inverted, normal) makes transmission
  // much more reliable. That's the exact behaviour of CD-S6470 remote control.
  for (int n = 0; n < 3; n++) {
    // Data
    sendData(SHARP_BIT_MARK, SHARP_ONE_SPACE, SHARP_BIT_MARK, SHARP_ZERO_SPACE,
             data, nbits, true);
    // Footer
    mark(SHARP_BIT_MARK);
    space(SHARP_ZERO_SPACE + 40000);

    data = data ^ SHARP_TOGGLE_MASK;
  }
}

// Sharp send compatible with data obtained through decodeSharp
void ICACHE_FLASH_ATTR IRsend::sendSharp(unsigned int address,
                                         unsigned int command) {
  sendSharpRaw((address << 10) | (command << 2) | 2, 15);
}

// Send an IR command to a DISH device.
// Note: Typically a DISH device needs to get a command a total of at least 4
//       times to accept it.
// Args:
//   data:   The contents of the command you want to send.
//   nbits:  The bit size of the command being sent.
//   repeat: The number of times you want the command to be repeated.
void ICACHE_FLASH_ATTR IRsend::sendDISH(unsigned long data, int nbits,
                                        unsigned int repeat) {
  // Set IR carrier frequency
  enableIROut(56);
  // We always send a command, even for repeat=0, hence '<= repeat'.
  for (unsigned int i = 0; i <= repeat; i++) {
    // Header
    mark(DISH_HDR_MARK);
    space(DISH_HDR_SPACE);
    // Data
    sendData(DISH_BIT_MARK, DISH_ONE_SPACE, DISH_BIT_MARK, DISH_ZERO_SPACE,
             data, nbits, true);
    // Footer
    space(DISH_RPT_SPACE);
  }
}

// From https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/tree/master/IRremote
void ICACHE_FLASH_ATTR IRsend::sendDaikin(unsigned char data[]) {
  // Args:
  //   data: An array of DAIKIN_COMMAND_LENGTH bytes containing the IR command.

  // Set IR carrier frequency
  enableIROut(38);
  // Header #1
  mark(DAIKIN_HDR_MARK);
  space(DAIKIN_HDR_SPACE);
  // Data #1
  for (uint8_t i = 0; i < 8; i++)
    sendData(DAIKIN_ONE_MARK, DAIKIN_ONE_SPACE, DAIKIN_ZERO_MARK,
             DAIKIN_ZERO_SPACE, data[i], 8, false);
  // Footer #1
  mark(DAIKIN_ONE_MARK);
  space(DAIKIN_ZERO_SPACE + 29000);

  // Header #2
  mark(DAIKIN_HDR_MARK);
  space(DAIKIN_HDR_SPACE);
  // Data #2
  for (uint8_t i = 8; i < DAIKIN_COMMAND_LENGTH; i++)
    sendData(DAIKIN_ONE_MARK, DAIKIN_ONE_SPACE, DAIKIN_ZERO_MARK,
             DAIKIN_ZERO_SPACE, data[i], 8, false);
  // Footer #2
  mark(DAIKIN_ONE_MARK);
  space(DAIKIN_ZERO_SPACE);
}

void ICACHE_FLASH_ATTR IRsend::sendKelvinator(unsigned char data[]) {
  uint8_t i = 0;
  // Set IR carrier frequency
  enableIROut(38);
  // Header #1
  mark(KELVINATOR_HDR_MARK);
  space(KELVINATOR_HDR_SPACE);
  // Data (command)
  // Send the first command data (4 bytes)
  for (; i < 4; i++)
    sendData(KELVINATOR_BIT_MARK, KELVINATOR_ONE_SPACE, KELVINATOR_BIT_MARK,
             KELVINATOR_ZERO_SPACE, data[i], 8, false);
  // Send Footer for the command data (3 bits (B010))
  sendData(KELVINATOR_BIT_MARK, KELVINATOR_ONE_SPACE, KELVINATOR_BIT_MARK,
           KELVINATOR_ZERO_SPACE, KELVINATOR_CMD_FOOTER, 3, false);
  // Send an interdata gap.
  mark(KELVINATOR_BIT_MARK);
  space(KELVINATOR_GAP_SPACE);
  // Data (options)
  // Send the 1st option chunk of data (4 bytes).
  for (; i < 8; i++)
    sendData(KELVINATOR_BIT_MARK, KELVINATOR_ONE_SPACE, KELVINATOR_BIT_MARK,
             KELVINATOR_ZERO_SPACE, data[i], 8, false);
  // Send a double data gap to signify we are starting a new command sequence.
  mark(KELVINATOR_BIT_MARK);
  space(KELVINATOR_GAP_SPACE * 2);
  // Header #2
  mark(KELVINATOR_HDR_MARK);
  space(KELVINATOR_HDR_SPACE);
  // Data (command)
  // Send the 2nd command data (4 bytes).
  // Basically an almost identical repeat of the earlier command data.
  for (; i < 12; i++)
    sendData(KELVINATOR_BIT_MARK, KELVINATOR_ONE_SPACE, KELVINATOR_BIT_MARK,
             KELVINATOR_ZERO_SPACE, data[i], 8, false);
  // Send Footer for the command data (3 bits (B010))
  sendData(KELVINATOR_BIT_MARK, KELVINATOR_ONE_SPACE, KELVINATOR_BIT_MARK,
           KELVINATOR_ZERO_SPACE, KELVINATOR_CMD_FOOTER, 3, false);
  // Send an interdata gap.
  mark(KELVINATOR_BIT_MARK);
  space(KELVINATOR_GAP_SPACE);
  // Data (options)
  // Send the 2nd option chunk of data (4 bytes).
  // Unlike the commands, definately not a repeat of the earlier option data.
  for (; i < KELVINATOR_STATE_LENGTH; i++)
    sendData(KELVINATOR_BIT_MARK, KELVINATOR_ONE_SPACE, KELVINATOR_BIT_MARK,
             KELVINATOR_ZERO_SPACE, data[i], 8, false);
  // Footer
  mark(KELVINATOR_BIT_MARK);
  ledOff();
}

void ICACHE_FLASH_ATTR IRsend::sendSherwood(unsigned long data, int nbits,
                                            unsigned int repeat) {
  // Sherwood remote codes appear to be NEC codes with a manditory repeat code.
  // i.e. repeat should be >= 1.
  sendNEC(data, nbits, max(1, repeat));
}

void ICACHE_FLASH_ATTR IRsend::sendMitsubishiAC(unsigned char data[]) {
  // Set IR carrier frequency
  enableIROut(38);
  // Mitsubishi AC remote sends the packet twice.
  for (uint8_t count = 0; count < 2; count++) {
    // Header
    mark(MITSUBISHI_AC_HDR_MARK);
    space(MITSUBISHI_AC_HDR_SPACE);
    // Data
    for (uint8_t i = 0; i < MITSUBISHI_AC_STATE_LENGTH; i++)
      sendData(MITSUBISHI_AC_BIT_MARK, MITSUBISHI_AC_ONE_SPACE,
               MITSUBISHI_AC_BIT_MARK, MITSUBISHI_AC_ZERO_SPACE,
               data[i], 8, false);
    // Footer
    mark(MITSUBISHI_AC_RPT_MARK);
    space(MITSUBISHI_AC_RPT_SPACE);
  }
  // A space() is always performed last, so no need to turn off the LED.
}
// ---------------------------------------------------------------


//IRRecv------------------------------------------------------

extern "C" {
	#include "user_interface.h"
	#include "gpio.h"
}

static ETSTimer timer;
volatile irparams_t irparams;

static void ICACHE_RAM_ATTR read_timeout(void *arg __attribute__((unused))) {
  os_intr_lock();
  if (irparams.rawlen)
    irparams.rcvstate = STATE_STOP;
  os_intr_unlock();
}

static void ICACHE_RAM_ATTR gpio_intr() {
  uint32_t now = system_get_time();
  uint32_t gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
  static uint32_t start = 0;

  os_timer_disarm(&timer);
  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);

  // Grab a local copy of rawlen to reduce instructions used in IRAM.
  // This is an ugly premature optimisation code-wise, but we do everything we
  // can to save IRAM.
  // It seems referencing the value via the structure uses more instructions.
  // Less instructions means faster and less IRAM used.
  // N.B. It saves about 13 bytes of IRAM.
  uint16_t rawlen = irparams.rawlen;

  if (rawlen >= RAWBUF) {
    irparams.overflow = true;
    irparams.rcvstate = STATE_STOP;
  }

  if (irparams.rcvstate == STATE_STOP)
    return;

  if (irparams.rcvstate == STATE_IDLE) {
    irparams.rcvstate = STATE_MARK;
    irparams.rawbuf[rawlen] = 1;
  } else {
    if (now < start)
      irparams.rawbuf[rawlen] = (0xFFFFFFFF - start + now) / USECPERTICK + 1;
    else
      irparams.rawbuf[rawlen] = (now - start) / USECPERTICK + 1;
  }
  irparams.rawlen++;

  start = now;
  #define ONCE 0
  os_timer_arm(&timer, 15, ONCE);
}

IRrecv::IRrecv(int recvpin) {
  irparams.recvpin = recvpin;
}

// initialization
void ICACHE_FLASH_ATTR IRrecv::enableIRIn() {
  // initialize state machine variables
  resume();

  // Initialize timer
  os_timer_disarm(&timer);
  os_timer_setfn(&timer, (os_timer_func_t *)read_timeout, NULL);

  // Attach Interrupt
  attachInterrupt(irparams.recvpin, gpio_intr, CHANGE);
}

void ICACHE_FLASH_ATTR IRrecv::disableIRIn() {
  os_timer_disarm(&timer);
  detachInterrupt(irparams.recvpin);
}

void ICACHE_FLASH_ATTR IRrecv::resume() {
  irparams.rcvstate = STATE_IDLE;
  irparams.rawlen = 0;
  irparams.overflow = false;
}

// Make a copy of the interrupt state/data.
// Needed because irparams is marked as volatile, thus memcpy() isn't allowed.
// Only call this when you know the interrupt handlers won't modify anything.
// i.e. In STATE_STOP.
//
// Args:
//   dest: Pointer to an irparams_t structure to copy to.
void ICACHE_FLASH_ATTR IRrecv::copyIrParams(irparams_t *dest) {
  // Typecast src and dest addresses to (char *)
  char *csrc = (char *)&irparams;
  char *cdest = (char *)dest;

  // Copy contents of src[] to dest[]
  for (int i=0; i<sizeof(irparams_t); i++)
    cdest[i] = csrc[i];
}

// Decodes the received IR message.
// If the interrupt state is saved, we will immediately resume waiting
// for the next IR message to avoid missing messages.
// Note: There is a trade-off here. Saving the state means less time lost until
// we can receiving the next message vs. using more RAM. Choose appropriately.
//
// Args:
//   results:  A pointer to where the decoded IR message will be stored.
//   save:  A pointer to an irparams_t instance in which to save
//          the interrupt's memory/state. NULL means don't save it.
// Returns:
//   A boolean indicating if an IR message is ready or not.
bool ICACHE_FLASH_ATTR IRrecv::decode(decode_results *results,
                                      irparams_t *save) {
  // Proceed only if an IR message been received.
  if (irparams.rcvstate != STATE_STOP) {
    return false;
  }

  bool resumed = false;  // Flag indicating if we have resumed.

  if (save == NULL) {
    // We haven't been asked to copy it so use the existing memory.
    results->rawbuf = irparams.rawbuf;
    results->rawlen = irparams.rawlen;
    results->overflow = irparams.overflow;
  } else {
    copyIrParams(save);  // Duplicate the interrupt's memory.
    resume();  // It's now safe to rearm. The IR message won't be overridden.
    resumed = true;
    // Point the results at the saved copy.
    results->rawbuf = save->rawbuf;
    results->rawlen = save->rawlen;
    results->overflow = save->overflow;
  }

#ifdef DEBUG
  Serial.println("Attempting NEC decode");
#endif
  if (decodeNEC(results)) {
    return true;
  }

#ifdef DEBUG
  Serial.println("Attempting Sony decode");
#endif
  if (decodeSony(results)) {
    return true;
  }
  /*
#ifdef DEBUG
  Serial.println("Attempting Sanyo decode");
#endif
  if (decodeSanyo(results)) {
    return true;
  }*/
#ifdef DEBUG
  Serial.println("Attempting Mitsubishi decode");
#endif
  if (decodeMitsubishi(results)) {
    return true;
  }
#ifdef DEBUG
  Serial.println("Attempting RC5 decode");
#endif
  if (decodeRC5(results)) {
    return true;
  }
#ifdef DEBUG
  Serial.println("Attempting RC6 decode");
#endif
  if (decodeRC6(results)) {
    return true;
  }
#ifdef DEBUG
  Serial.println("Attempting RC-MM decode");
#endif
  if (decodeRCMM(results)) {
    return true;
  }
#ifdef DEBUG
  Serial.println("Attempting Panasonic decode");
#endif
  if (decodePanasonic(results)) {
    return true;
  }
#ifdef DEBUG
  Serial.println("Attempting LG decode");
#endif
  if (decodeLG(results)) {
      return true;
  }
#ifdef DEBUG
  Serial.println("Attempting JVC decode");
#endif
  if (decodeJVC(results)) {
    return true;
  }
#ifdef DEBUG
  Serial.println("Attempting SAMSUNG decode");
#endif
  if (decodeSAMSUNG(results)) {
    return true;
  }
#ifdef DEBUG
  Serial.println("Attempting Whynter decode");
#endif
  if (decodeWhynter(results)) {
    return true;
  }
#ifdef DEBUG
  Serial.println("Attempting Denon decode");
#endif
  if (decodeDenon(results)) {
    return true;
  }
  // decodeHash returns a hash on any input.
  // Thus, it needs to be last in the list.
  // If you add any decodes, add them before this.
  if (decodeHash(results)) {
    return true;
  }
  // Throw away and start over
  if (!resumed)  // Check if we have already resumed.
    resume();
  return false;
}

// Calculate the lower bound of the nr. of ticks.
//
// Args:
//   usecs:  Nr. of uSeconds.
//   tolerance:  Percent as an integer. e.g. 10 is 10%
// Returns:
//   Nr. of ticks.
uint32_t IRrecv::ticksLow(uint32_t usecs, uint8_t tolerance) {
  // max() used to ensure the result can't drop below 0 before the cast.
  return((uint32_t) max(usecs * (1.0 - tolerance/100.)/USECPERTICK, 0));
}

// Calculate the upper bound of the nr. of ticks.
//
// Args:
//   usecs:  Nr. of uSeconds.
//   tolerance:  Percent as an integer. e.g. 10 is 10%
// Returns:
//   Nr. of ticks.
uint32_t IRrecv::ticksHigh(uint32_t usecs, uint8_t tolerance) {
  return((uint32_t) usecs * (1.0 + tolerance/100.)/USECPERTICK + 1);
}

// Check if we match a pulse(measured_ticks) with the desired_us within
// +/-tolerance percent.
//
// Args:
//   measured_ticks:  The recorded period of the signal pulse.
//   desired_us:  The expected period (in useconds) we are matching against.
//   tolerance:  A percentage expressed as an integer. e.g. 10 is 10%.
//
// Returns:
//   Boolean: true if it matches, false if it doesn't.
bool ICACHE_FLASH_ATTR IRrecv::match(uint32_t measured_ticks,
                                     uint32_t desired_us,
                                     uint8_t tolerance) {
  #ifdef DEBUG
    Serial.print("Matching: ");
    Serial.print(ticksLow(desired_us, tolerance), DEC);
    Serial.print(" <= ");
    Serial.print(measured_ticks, DEC);
    Serial.print(" <= ");
    Serial.println(ticksHigh(desired_us, tolerance), DEC);
  #endif
  return (measured_ticks >= ticksLow(desired_us, tolerance) &&
          measured_ticks <= ticksHigh(desired_us, tolerance));
}

// Check if we match a mark signal(measured_ticks) with the desired_us within
// +/-tolerance percent, after an expected is excess is added.
//
// Args:
//   measured_ticks:  The recorded period of the signal pulse.
//   desired_us:  The expected period (in useconds) we are matching against.
//   tolerance:  A percentage expressed as an integer. e.g. 10 is 10%.
//   excess:  Nr. of useconds.
//
// Returns:
//   Boolean: true if it matches, false if it doesn't.
bool ICACHE_FLASH_ATTR IRrecv::matchMark(uint32_t measured_ticks,
                                         uint32_t desired_us,
                                         uint8_t tolerance, int excess) {
  #ifdef DEBUG
    Serial.print("Matching MARK ");
    Serial.print(measured_ticks * USECPERTICK, DEC);
    Serial.print(" vs ");
    Serial.print(desired_us, DEC);
    Serial.print(". ");
  #endif
  return match(measured_ticks, desired_us + excess, tolerance);
}
// Check if we match a space signal(measured_ticks) with the desired_us within
// +/-tolerance percent, after an expected is excess is removed.
//
// Args:
//   measured_ticks:  The recorded period of the signal pulse.
//   desired_us:  The expected period (in useconds) we are matching against.
//   tolerance:  A percentage expressed as an integer. e.g. 10 is 10%.
//   excess:  Nr. of useconds.
//
// Returns:
//   Boolean: true if it matches, false if it doesn't.
bool ICACHE_FLASH_ATTR IRrecv::matchSpace(uint32_t measured_ticks,
                                          uint32_t desired_us,
                                          uint8_t tolerance, int excess) {
  #ifdef DEBUG
    Serial.print("Matching SPACE ");
    Serial.print(measured_ticks * USECPERTICK, DEC);
    Serial.print(" vs ");
    Serial.print(desired_us, DEC);
    Serial.print(". ");
  #endif
  return match(measured_ticks, desired_us - excess, tolerance);
}

// NECs have a repeat only 4 items long
bool ICACHE_FLASH_ATTR IRrecv::decodeNEC(decode_results *results) {
  long data = 0;
  int offset = 1; // Skip initial space
  // Initial mark
  if (!matchMark(results->rawbuf[offset], NEC_HDR_MARK)) {
    return false;
  }
  offset++;
  // Check for repeat
  if (results->rawlen == 4 &&
    matchSpace(results->rawbuf[offset], NEC_RPT_SPACE) &&
    matchMark(results->rawbuf[offset+1], NEC_BIT_MARK)) {
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = NEC;
    return true;
  }
  if (results->rawlen < 2 * NEC_BITS + 4) {
    return false;
  }
  // Initial space
  if (!matchSpace(results->rawbuf[offset], NEC_HDR_SPACE)) {
    return false;
  }
  offset++;
  for (int i = 0; i < NEC_BITS; i++) {
    if (!matchMark(results->rawbuf[offset], NEC_BIT_MARK)) {
      return false;
    }
    offset++;
    if (matchSpace(results->rawbuf[offset], NEC_ONE_SPACE)) {
      data = (data << 1) | 1;
    } else if (matchSpace(results->rawbuf[offset], NEC_ZERO_SPACE)) {
      data <<= 1;
    } else {
      return false;
    }
    offset++;
  }
  // Success
  results->bits = NEC_BITS;
  results->value = data;
  results->decode_type = NEC;
  return true;
}

bool ICACHE_FLASH_ATTR IRrecv::decodeSony(decode_results *results) {
  long data = 0;
  if (results->rawlen < 2 * SONY_BITS + 2) {
    return false;
  }
  int offset = 0; // Dont skip first space, check its size

  /*
  // Some Sony's deliver repeats fast after first
  // unfortunately can't spot difference from of repeat from two fast clicks
  if (results->rawbuf[offset] < SONY_DOUBLE_SPACE_USECS) {
    // Serial.print("IR Gap found: ");
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = SANYO;
    return true;
  }*/
  offset++;

  // Initial mark
  if (!matchMark(results->rawbuf[offset], SONY_HDR_MARK)) {
    return false;
  }
  offset++;

  while (offset + 1 < results->rawlen) {
    if (!matchSpace(results->rawbuf[offset], SONY_HDR_SPACE)) {
      break;
    }
    offset++;
    if (matchMark(results->rawbuf[offset], SONY_ONE_MARK)) {
      data = (data << 1) | 1;
    } else if (matchMark(results->rawbuf[offset], SONY_ZERO_MARK)) {
      data <<= 1;
    } else {
      return false;
    }
    offset++;
  }

  // Success
  results->bits = (offset - 1) / 2;
  if (results->bits < 12) {
    results->bits = 0;
    return false;
  }
  results->value = data;
  results->decode_type = SONY;
  return true;
}

bool ICACHE_FLASH_ATTR IRrecv::decodeWhynter(decode_results *results) {
  long data = 0;

  if (results->rawlen < 2 * WHYNTER_BITS + 6) {
     return false;
  }

  int offset = 1; // Skip first space


  // sequence begins with a bit mark and a zero space
  if (!matchMark(results->rawbuf[offset], WHYNTER_BIT_MARK)) {
    return false;
  }
  offset++;
  if (!matchSpace(results->rawbuf[offset], WHYNTER_ZERO_SPACE)) {
    return false;
  }
  offset++;

  // header mark and space
  if (!matchMark(results->rawbuf[offset], WHYNTER_HDR_MARK)) {
    return false;
  }
  offset++;
  if (!matchSpace(results->rawbuf[offset], WHYNTER_HDR_SPACE)) {
    return false;
  }
  offset++;

  // data bits
  for (int i = 0; i < WHYNTER_BITS; i++) {
    if (!matchMark(results->rawbuf[offset], WHYNTER_BIT_MARK)) {
      return false;
    }
    offset++;
    if (matchSpace(results->rawbuf[offset], WHYNTER_ONE_SPACE)) {
      data = (data << 1) | 1;
    } else if (matchSpace(results->rawbuf[offset],WHYNTER_ZERO_SPACE)) {
      data <<= 1;
    } else {
      return false;
    }
    offset++;
  }

  // trailing mark
  if (!matchMark(results->rawbuf[offset], WHYNTER_BIT_MARK)) {
    return false;
  }
  // Success
  results->bits = WHYNTER_BITS;
  results->value = data;
  results->decode_type = WHYNTER;
  return true;
}

// I think this is a Sanyo decoder - serial = SA 8650B
// Looks like Sony except for timings, 48 chars of data and time/space different
bool ICACHE_FLASH_ATTR IRrecv::decodeSanyo(decode_results *results) {
  long data = 0;
  if (results->rawlen < 2 * SANYO_BITS + 2) {
    return false;
  }
  int offset = 1; // Skip first space


  // Initial space
  /* Put this back in for debugging - note can't use #DEBUG as if Debug on we don't see the repeat cos of the delay
  Serial.print("IR Gap: ");
  Serial.println( results->rawbuf[offset]);
  Serial.println( "test against:");
  Serial.println(results->rawbuf[offset]);
  */

  if (results->rawbuf[offset] < SANYO_DOUBLE_SPACE_USECS) {
    // Serial.print("IR Gap found: ");
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = SANYO;
    return true;
  }
  offset++;

  // Initial mark
  if (!matchMark(results->rawbuf[offset], SANYO_HDR_MARK)) {
    return false;
  }
  offset++;

  // Skip Second Mark
  if (!matchMark(results->rawbuf[offset], SANYO_HDR_MARK)) {
    return false;
  }
  offset++;

  while (offset + 1 < results->rawlen) {
    if (!matchSpace(results->rawbuf[offset], SANYO_HDR_SPACE)) {
      break;
    }
    offset++;
    if (matchMark(results->rawbuf[offset], SANYO_ONE_MARK)) {
      data = (data << 1) | 1;
    } else if (matchMark(results->rawbuf[offset], SANYO_ZERO_MARK)) {
      data <<= 1;
    } else {
      return false;
    }
    offset++;
  }

  // Success
  results->bits = (offset - 1) / 2;
  if (results->bits < 12) {
    results->bits = 0;
    return false;
  }
  results->value = data;
  results->decode_type = SANYO;
  return true;
}

// Looks like Sony except for timings, 48 chars of data and time/space different
bool ICACHE_FLASH_ATTR IRrecv::decodeMitsubishi(decode_results *results) {
  // Serial.print("?!? decoding Mitsubishi:");Serial.print(results->rawlen);
  // Serial.print(" want "); Serial.println( 2 * MITSUBISHI_BITS + 2);
  long data = 0;
  if (results->rawlen < 2 * MITSUBISHI_BITS + 2) {
    return false;
  }
  int offset = 1; // Skip first space
  // Initial space
  /* Put this back in for debugging - note can't use #DEBUG as if Debug on we
     don't see the repeat cos of the delay
  Serial.print("IR Gap: ");
  Serial.println( results->rawbuf[offset]);
  Serial.println( "test against:");
  Serial.println(results->rawbuf[offset]);
  */
  /* Not seeing double keys from Mitsubishi
  if (results->rawbuf[offset] < MITSUBISHI_DOUBLE_SPACE_USECS) {
    // Serial.print("IR Gap found: ");
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = MITSUBISHI;
    return true;
  }
  */

  offset++;

  // Typical
  // 14200 7 41 7 42 7 42 7 17 7 17 7 18 7 41 7 18 7 17 7 17 7 18 7 41 8 17 7 17 7 18 7 17 7

  // Initial Space
  if (!matchMark(results->rawbuf[offset], MITSUBISHI_HDR_SPACE)) {
    return false;
  }
  offset++;
  while (offset + 1 < results->rawlen) {
    if (matchMark(results->rawbuf[offset], MITSUBISHI_ONE_MARK)) {
      data = (data << 1) | 1;
    } else if (matchMark(results->rawbuf[offset], MITSUBISHI_ZERO_MARK)) {
      data <<= 1;
    } else {
      // Serial.println("A"); Serial.println(offset); Serial.println(results->rawbuf[offset]);
      return false;
    }
    offset++;
    if (!matchSpace(results->rawbuf[offset], MITSUBISHI_HDR_SPACE)) {
      // Serial.println("B"); Serial.println(offset); Serial.println(results->rawbuf[offset]);
      break;
    }
    offset++;
  }

  // Success
  results->bits = (offset - 1) / 2;
  if (results->bits < MITSUBISHI_BITS) {
    results->bits = 0;
    return false;
  }
  results->value = data;
  results->decode_type = MITSUBISHI;
  return true;
}

// Gets one undecoded level at a time from the raw buffer.
// The RC5/6 decoding is easier if the data is broken into time intervals.
// E.g. if the buffer has MARK for 2 time intervals and SPACE for 1,
// successive calls to getRClevel will return MARK, MARK, SPACE.
// offset and used are updated to keep track of the current position.
// t1 is the time interval for a single bit in microseconds.
// Returns -1 for error (measured time interval is not a multiple of t1).
int ICACHE_FLASH_ATTR IRrecv::getRClevel(decode_results *results, int *offset,
                                         int *used, int t1) {
  if (*offset >= results->rawlen) {
    // After end of recorded buffer, assume SPACE.
    return SPACE;
  }
  int width = results->rawbuf[*offset];
  int val = ((*offset) % 2) ? MARK : SPACE;
  int correction = (val == MARK) ? MARK_EXCESS : - MARK_EXCESS;

  int avail;
  if (match(width, t1 + correction)) {
    avail = 1;
  } else if (match(width, 2*t1 + correction)) {
    avail = 2;
  } else if (match(width, 3*t1 + correction)) {
    avail = 3;
  } else {
    return -1;
  }

  (*used)++;
  if (*used >= avail) {
    *used = 0;
    (*offset)++;
  }
#ifdef DEBUG
  if (val == MARK) {
    Serial.println("MARK");
  } else {
    Serial.println("SPACE");
  }
#endif
  return val;
}

bool ICACHE_FLASH_ATTR IRrecv::decodeRC5(decode_results *results) {
  if (results->rawlen < MIN_RC5_SAMPLES + 2) {
    return false;
  }
  int offset = 1; // Skip gap space
  long data = 0;
  int used = 0;
  // Get start bits
  if (getRClevel(results, &offset, &used, RC5_T1) != MARK) return false;
  if (getRClevel(results, &offset, &used, RC5_T1) != SPACE) return false;
  if (getRClevel(results, &offset, &used, RC5_T1) != MARK) return false;
  int nbits;
  for (nbits = 0; offset < results->rawlen; nbits++) {
    int levelA = getRClevel(results, &offset, &used, RC5_T1);
    int levelB = getRClevel(results, &offset, &used, RC5_T1);
    if (levelA == SPACE && levelB == MARK) {
      // 1 bit
      data = (data << 1) | 1;
    } else if (levelA == MARK && levelB == SPACE) {
      // zero bit
      data <<= 1;
    } else {
      return false;
    }
  }

  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = RC5;
  return true;
}

bool ICACHE_FLASH_ATTR IRrecv::decodeRC6(decode_results *results) {
  if (results->rawlen < MIN_RC6_SAMPLES) {
    return false;
  }
  int offset = 1; // Skip first space
  // Initial mark
  if (!matchMark(results->rawbuf[offset], RC6_HDR_MARK)) {
    return false;
  }
  offset++;
  if (!matchSpace(results->rawbuf[offset], RC6_HDR_SPACE)) {
    return false;
  }
  offset++;
  long data = 0;
  int used = 0;
  // Get start bit (1)
  if (getRClevel(results, &offset, &used, RC6_T1) != MARK) return false;
  if (getRClevel(results, &offset, &used, RC6_T1) != SPACE) return false;
  int nbits;
  for (nbits = 0; offset < results->rawlen; nbits++) {
    int levelA, levelB; // Next two levels
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
    if (levelA == MARK && levelB == SPACE) { // reversed compared to RC5
      // 1 bit
      data = (data << 1) | 1;
    } else if (levelA == SPACE && levelB == MARK) {
      // zero bit
      data <<= 1;
    } else {
      return false; // Error
    }
  }
  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = RC6;
  return true;
}

// Decode a Philips RC-MM packet (between 12 & 32 bits) if possible.
// Places successful decode information in the results pointer.
// Returns:
//   The decode success status.
// Based on http://www.sbprojects.com/knowledge/ir/rcmm.php
// Status:  ALPHA (untested and unconfirmed.)
bool ICACHE_FLASH_ATTR IRrecv::decodeRCMM(decode_results *results) {
  uint32_t data = 0;
  unsigned int offset = 1;  // Skip the leading space.

  int bitSize = results->rawlen - 4;
  if (bitSize < 12 || bitSize > 32)
    return false;
  // Header decode
  if (!matchMark(results->rawbuf[offset++], RCMM_HDR_MARK))
    return false;
  if (!matchSpace(results->rawbuf[offset++], RCMM_HDR_SPACE))
    return false;
  // Data decode
  // RC-MM has two bits of data per mark/space pair.
  for (int i = 0; i < bitSize; i += 2) {
    data <<= 2;
    // Use non-default tolerance & excess for matching some of the spaces as the
    // defaults are too generous and causes mis-matches in some cases.
    if (!matchMark(results->rawbuf[offset++], RCMM_BIT_MARK, RCMM_TOLERANCE))
      return false;
    if (matchSpace(results->rawbuf[offset],
                   RCMM_BIT_SPACE_0, TOLERANCE, RCMM_EXCESS))
      data += 0;
    else if (matchSpace(results->rawbuf[offset],
                        RCMM_BIT_SPACE_1, TOLERANCE, RCMM_EXCESS))
      data += 1;
    else if (matchSpace(results->rawbuf[offset],
                        RCMM_BIT_SPACE_2, RCMM_TOLERANCE, RCMM_EXCESS))
      data += 2;
    else if (matchSpace(results->rawbuf[offset],
                        RCMM_BIT_SPACE_3, RCMM_TOLERANCE, RCMM_EXCESS))
      data += 3;
    else
      return false;
    offset++;
  }
  // Footer decode
  if (!matchMark(results->rawbuf[offset], RCMM_BIT_MARK))
    return false;

  // Success
  results->value = (unsigned long) data;
  results->decode_type = RCMM;
  results->bits = bitSize;
  return true;
}

bool ICACHE_FLASH_ATTR IRrecv::decodePanasonic(decode_results *results) {
  unsigned long long data = 0;
	int offset = 1;  // Dont skip first space
  if (!matchMark(results->rawbuf[offset], PANASONIC_HDR_MARK)) {
    return false;
  }
  offset++;
  if (!matchMark(results->rawbuf[offset], PANASONIC_HDR_SPACE)) {
    return false;
  }
  offset++;
  // decode address
  for (int i = 0; i < PANASONIC_BITS; i++) {
    if (!match(results->rawbuf[offset++], PANASONIC_BIT_MARK)) {
      return false;
    }
    if (match(results->rawbuf[offset],PANASONIC_ONE_SPACE)) {
      data = (data << 1) | 1;
    } else if (match(results->rawbuf[offset],PANASONIC_ZERO_SPACE)) {
      data <<= 1;
    } else {
      return false;
    }
    offset++;
  }
  results->value = (unsigned long)data;
  results->panasonicAddress = (unsigned int)(data >> 32);
  results->decode_type = PANASONIC;
  results->bits = PANASONIC_BITS;
  return true;
}

bool ICACHE_FLASH_ATTR IRrecv::decodeLG(decode_results *results) {
  long data = 0;
	int offset = 1; // Skip first space

  // Initial mark
  if (!matchMark(results->rawbuf[offset], LG_HDR_MARK)) {
    return false;
  }
  offset++;
  if (results->rawlen < 2 * LG_BITS + 1 ) {
    return false;
  }
  // Initial space
  if (!matchSpace(results->rawbuf[offset], LG_HDR_SPACE)) {
    return false;
  }
  offset++;
  for (int i = 0; i < LG_BITS; i++) {
    if (!matchMark(results->rawbuf[offset], LG_BIT_MARK)) {
      return false;
    }
    offset++;
    if (matchSpace(results->rawbuf[offset], LG_ONE_SPACE)) {
      data = (data << 1) | 1;
    } else if (matchSpace(results->rawbuf[offset], LG_ZERO_SPACE)) {
      data <<= 1;
    } else {
      return false;
    }
    offset++;
  }
  //Stop bit
  if (!matchMark(results->rawbuf[offset], LG_BIT_MARK)){
    return false;
  }
  // Success
  results->bits = LG_BITS;
  results->value = data;
  results->decode_type = LG;
  return true;
}

bool ICACHE_FLASH_ATTR IRrecv::decodeJVC(decode_results *results) {
  long data = 0;
	int offset = 1; // Skip first space
  // Check for repeat
  if (results->rawlen - 1 == 33 &&
      matchMark(results->rawbuf[offset], JVC_BIT_MARK) &&
      matchMark(results->rawbuf[irparams.rawlen-1], JVC_BIT_MARK)) {
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = JVC;
    return true;
  }
  // Initial mark
  if (!matchMark(results->rawbuf[offset], JVC_HDR_MARK)) {
    return false;
  }
  offset++;
  if (results->rawlen < 2 * JVC_BITS + 1 ) {
    return false;
  }
  // Initial space
  if (!matchSpace(results->rawbuf[offset], JVC_HDR_SPACE)) {
    return false;
  }
  offset++;
  for (int i = 0; i < JVC_BITS; i++) {
    if (!matchMark(results->rawbuf[offset], JVC_BIT_MARK)) {
      return false;
    }
    offset++;
    if (matchSpace(results->rawbuf[offset], JVC_ONE_SPACE)) {
      data = (data << 1) | 1;
    } else if (matchSpace(results->rawbuf[offset], JVC_ZERO_SPACE)) {
      data <<= 1;
    } else {
      return false;
    }
    offset++;
  }
  //Stop bit
  if (!matchMark(results->rawbuf[offset], JVC_BIT_MARK)) {
    return false;
  }
  // Success
  results->bits = JVC_BITS;
  results->value = data;
  results->decode_type = JVC;
  return true;
}

// SAMSUNGs have a repeat only 4 items long
bool ICACHE_FLASH_ATTR IRrecv::decodeSAMSUNG(decode_results *results) {
  long data = 0;
  int offset = 1;  // Dont skip first space
  // Initial mark
  if (!matchMark(results->rawbuf[offset], SAMSUNG_HDR_MARK)) {
    return false;
  }
  offset++;
  // Check for repeat
  if (results->rawlen == 4 &&
      matchSpace(results->rawbuf[offset], SAMSUNG_RPT_SPACE) &&
      matchMark(results->rawbuf[offset+1], SAMSUNG_BIT_MARK)) {
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = SAMSUNG;
    return true;
  }
  if (results->rawlen < 2 * SAMSUNG_BITS + 2) {
    return false;
  }
  // Initial space
  if (!matchSpace(results->rawbuf[offset], SAMSUNG_HDR_SPACE)) {
    return false;
  }
  offset++;
  for (int i = 0; i < SAMSUNG_BITS; i++) {
    if (!matchMark(results->rawbuf[offset], SAMSUNG_BIT_MARK)) {
      return false;
    }
    offset++;
    if (matchSpace(results->rawbuf[offset], SAMSUNG_ONE_SPACE)) {
      data = (data << 1) | 1;
    } else if (matchSpace(results->rawbuf[offset], SAMSUNG_ZERO_SPACE)) {
      data <<= 1;
    } else {
      return false;
    }
    offset++;
  }
  // Success
  results->bits = SAMSUNG_BITS;
  results->value = data;
  results->decode_type = SAMSUNG;
  return true;
}

// From https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/tree/master/IRremote
// decoding not actually tested
bool ICACHE_FLASH_ATTR IRrecv::decodeDaikin(decode_results *results) {
  long data = 0;
  int offset = 1; // Skip first space

  if (results->rawlen < 2 * DAIKIN_BITS + 4) {
    //return false;
  }

  // Initial mark
  if (!matchMark(results->rawbuf[offset], DAIKIN_HDR_MARK)) {
      return false;
  }
  offset++;

  if (!matchSpace(results->rawbuf[offset], DAIKIN_HDR_SPACE)) {
      return false;
  }
  offset++;

  for (int i = 0; i < 32; i++) {
    if (!matchMark(results->rawbuf[offset], DAIKIN_ONE_MARK)) {
      return false;
    }
    offset++;
    if (matchSpace(results->rawbuf[offset], DAIKIN_ONE_SPACE)) {
      data = (data << 1) | 1;
    } else if (matchSpace(results->rawbuf[offset], DAIKIN_ZERO_SPACE)) {
      data <<= 1;
    } else {
      return false;
    }
    offset++;
  }

  unsigned long number = data ; // some number...
  int bits = 32 ; // nr of bits in some number
  unsigned long reversed = 0;
  for ( int b=0 ; b < bits ; b++ ) {
    reversed = ( reversed << 1 ) | ( 0x0001 & ( number >> b ) );
  }

  Serial.print ("Code ");
  Serial.println (reversed,  HEX);

  //==========

  for (int i = 0; i < 32; i++) {
    if (!matchMark(results->rawbuf[offset], DAIKIN_ONE_MARK)) {
      return false;
    }
    offset++;
    if (matchSpace(results->rawbuf[offset], DAIKIN_ONE_SPACE)) {
      data = (data << 1) | 1;
    } else if (matchSpace(results->rawbuf[offset], DAIKIN_ZERO_SPACE)) {
      data <<= 1;
    } else {
      return false;
    }
    offset++;
  }

  number = data ; // some number...
  bits = 32 ; // nr of bits in some number
  reversed = 0;
  for ( int b=0 ; b < bits ; b++ ) {
    reversed = ( reversed << 1 ) | ( 0x0001 & ( number >> b ) );
  }

  //Serial.print ("Code2 ");
  //Serial.println (reversed,  HEX);

  //===========
  if (!matchSpace(results->rawbuf[offset], 29000)) {
    //Serial.println ("no gap");
	  return false;
  }
  offset++;

  // Success
  results->bits = DAIKIN_BITS;
  results->value = reversed;
  results->decode_type = DAIKIN;
  return true;
}

// Denon, from https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Denon.cpp
bool ICACHE_FLASH_ATTR IRrecv::decodeDenon (decode_results *results) {
	unsigned long data   = 0;  // Somewhere to build our code
	int offset = 1;  // Skip the Gap reading

	// Check we have the right amount of data
	if (results->rawlen != 1 + 2 + (2 * DENON_BITS) + 1) {
	  return false;
	}

	// Check initial Mark+Space match
	if (!matchMark (results->rawbuf[offset++], DENON_HDR_MARK )) {
	  return false;
	}
	if (!matchSpace(results->rawbuf[offset++], DENON_HDR_SPACE)) {
	  return false;
	}

	// Read the bits in
	for (int i = 0;  i < DENON_BITS;  i++) {
		// Each bit looks like: DENON_MARK + DENON_SPACE_1 -> 1
		//                 or : DENON_MARK + DENON_SPACE_0 -> 0
		if (!matchMark(results->rawbuf[offset++], DENON_BIT_MARK)) {
		  return false;
		}

		// IR data is big-endian, so we shuffle it in from the right:
		if (matchSpace(results->rawbuf[offset], DENON_ONE_SPACE)) {
		  data = (data << 1) | 1;
		} else if (matchSpace(results->rawbuf[offset], DENON_ZERO_SPACE)) {
		  data = (data << 1) | 0;
		} else {
		  return false;
		}
		offset++;
	}

	// Success
	results->bits = DENON_BITS;
	results->value = data;
	results->decode_type = DENON;
	return true;
}


/* -----------------------------------------------------------------------
 * hashdecode - decode an arbitrary IR code.
 * Instead of decoding using a standard encoding scheme
 * (e.g. Sony, NEC, RC5), the code is hashed to a 32-bit value.
 *
 * The algorithm: look at the sequence of MARK signals, and see if each one
 * is shorter (0), the same length (1), or longer (2) than the previous.
 * Do the same with the SPACE signals.  Hszh the resulting sequence of 0's,
 * 1's, and 2's to a 32-bit value.  This will give a unique value for each
 * different code (probably), for most code systems.
 *
 * http://arcfn.com/2010/01/using-arbitrary-remotes-with-arduino.html
 */

// Compare two tick values, returning 0 if newval is shorter,
// 1 if newval is equal, and 2 if newval is longer
// Use a tolerance of 20%
int ICACHE_FLASH_ATTR IRrecv::compare(unsigned int oldval,
                                      unsigned int newval) {
  if (newval < oldval * .8) {
    return 0;
  } else if (oldval < newval * .8) {
    return 2;
  } else {
    return 1;
  }
}

// Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
#define FNV_PRIME_32 16777619
#define FNV_BASIS_32 2166136261

/* Converts the raw code values into a 32-bit hash code.
 * Hopefully this code is unique for each button.
 * This isn't a "real" decoding, just an arbitrary value.
 */
bool ICACHE_FLASH_ATTR IRrecv::decodeHash(decode_results *results) {
  // Require at least 6 samples to prevent triggering on noise
  if (results->rawlen < 6) {
    return false;
  }
  long hash = FNV_BASIS_32;
  for (int i = 1; i+2 < results->rawlen; i++) {
    int value =  compare(results->rawbuf[i], results->rawbuf[i+2]);
    // Add value into the hash
    hash = (hash * FNV_PRIME_32) ^ value;
  }
  results->value = hash;
  results->bits = 32;
  results->decode_type = UNKNOWN;
  return true;
}

// ---------------------------------------------------------------
