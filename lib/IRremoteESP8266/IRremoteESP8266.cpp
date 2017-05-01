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
 * Coolix A/C / heatpump added by (send) bakrus & (decode) crankyoldgit
 * Denon: sendDenon, decodeDenon added by Massimiliano Pinto
 *   (from https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Denon.cpp)
 * Kelvinator A/C and Sherwood added by crankyoldgit
 * Mitsubishi (TV) sending added by crankyoldgit
 * Mitsubishi A/C added by crankyoldgit
 *     (derived from https://github.com/r45635/HVAC-IR-Control)
 * DISH decode by marcosamarinho
 *
 * Updated by markszabo (https://github.com/markszabo/IRremoteESP8266) for
 *   sending IR code on ESP8266
 * Updated by Sebastien Warin (http://sebastien.warin.fr) for receiving IR code
 *   on ESP8266
 *
 *  GPL license, all text above must be included in any redistribution
 ****************************************************/

#include <IRremoteESP8266.h>
extern "C" {
  #include <gpio.h>
  #include <user_interface.h>
}
#include <algorithm>
#include <IRremoteInt.h>
#include <IRDaikinESP.h>
#include <IRKelvinator.h>
#include <IRMitsubishiAC.h>

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

IRsend::IRsend(uint16_t IRsendPin) {
  IRpin = IRsendPin;
}

void ICACHE_FLASH_ATTR IRsend::begin() {
  pinMode(IRpin, OUTPUT);
}

// Generic method for sending data that is common to most protocols.
// Will send leading or trailing 0's if the nbits is larger than the number
// of bits in data.
//
// Args:
//   onemark:    Nr. of usecs for the led to be pulsed for a '1' bit.
//   onespace:   Nr. of usecs for the led to be fully off for a '1' bit.
//   zeromark:   Nr. of usecs for the led to be pulsed for a '0' bit.
//   zerospace:  Nr. of usecs for the led to be fully off for a '0' bit.
//   data:       The data to be transmitted.
//   nbits:      Nr. of bits of data to be sent.
//   MSBfirst:   Flag for bit transmission order. Defaults to MSB->LSB order.
void ICACHE_FLASH_ATTR IRsend::sendData(uint16_t onemark, uint32_t onespace,
                                        uint16_t zeromark, uint32_t zerospace,
                                        uint64_t data, uint16_t nbits,
                                        bool MSBfirst) {
  if (MSBfirst) {  // Send the MSB first.
    // Send 0's until we get down to a bit size we can actually manage.
    while (nbits > sizeof(data) * 8) {
      mark(zeromark);
      space(zerospace);
      nbits--;
    }
    // Send the supplied data.
    for (uint64_t mask = 1ULL << (nbits - 1);  mask;  mask >>= 1)
      if (data & mask) {  // Send a 1
        mark(onemark);
        space(onespace);
      } else {  // Send a 0
        mark(zeromark);
        space(zerospace);
      }
  } else {  // Send the Least Significant Bit (LSB) first / MSB last.
    for (uint16_t bit = 0; bit < nbits; bit++, data >>= 1)
      if (data & 1) {  // Send a 1
        mark(onemark);
        space(onespace);
      } else {  // Send a 0
        mark(zeromark);
        space(zerospace);
      }
  }
}

// Send a Coolix message
//
// Args:
//   data:   Contents of the message to be sent.
//   nbits:  Nr. of bits of data to be sent. Typically COOLIX_BITS.
//   repeat: Nr. of additional times the message is to be sent.
//
// Status: BETA / Probably works.
//
// Ref:
//   https://github.com/z3t0/Arduino-IRremote/blob/master/ir_COOLIX.cpp
// TODO(anyone): Verify repeat functionality against a real unit.
void ICACHE_FLASH_ATTR IRsend::sendCOOLIX(uint64_t data, uint16_t nbits,
                                          uint16_t repeat) {
  if (nbits % 8 != 0)
    return;  // nbits is required to be a multiple of 8.

  // Set IR carrier frequency
  enableIROut(38);

  for (uint16_t r = 0; r <= repeat; r++) {
    // Header
    mark(COOLIX_HDR_MARK);
    space(COOLIX_HDR_SPACE);

    // Data
    //   Break data into byte segments, starting at the Most Significant
    //   Byte. Each byte then being sent normal, then followed inverted.
    for (uint16_t i = 8; i <= nbits; i += 8) {
      // Grab a bytes worth of data.
      uint8_t segment = (data >> (nbits - i)) & 0xFF;
      // Normal
      sendData(COOLIX_BIT_MARK, COOLIX_ONE_SPACE,
               COOLIX_BIT_MARK, COOLIX_ZERO_SPACE,
               segment, 8, true);
      // Inverted.
      sendData(COOLIX_BIT_MARK, COOLIX_ONE_SPACE,
               COOLIX_BIT_MARK, COOLIX_ZERO_SPACE,
               segment ^ 0xFF, 8, true);
    }

    // Footer
    mark(COOLIX_BIT_MARK);
    space(COOLIX_MIN_GAP);  // Pause before repeating
  }
}

// Send a raw NEC(Renesas) formatted message.
//
// Args:
//   data:   The message to be sent.
//   nbits:  The number of bits of the message to be sent. Typically NEC_BITS.
//   repeat: The number of times the command is to be repeated.
//
// Status: STABLE / Known working.
//
// Ref:
//  http://www.sbprojects.com/knowledge/ir/nec.php
void ICACHE_FLASH_ATTR IRsend::sendNEC(uint64_t data, uint16_t nbits,
                                       uint16_t repeat) {
  // Set 38kHz IR carrier frequency & a 1/3 (33%) duty cycle.
  enableIROut(38, 33);
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
  space(std::max(NEC_MIN_GAP, NEC_MIN_COMMAND_LENGTH - usecs.elapsed()));

  // Optional command repeat sequence.
  for (uint16_t i = 0; i < repeat; i++) {
    usecs.reset();
    mark(NEC_HDR_MARK);
    space(NEC_RPT_SPACE);
    mark(NEC_BIT_MARK);
    // Gap till next command.
    space(std::max(NEC_MIN_GAP, NEC_MIN_COMMAND_LENGTH - usecs.elapsed()));
  }
}

// Calculate the raw NEC data based on address and command.
// Args:
//   address: An address value.
//   command: An 8-bit command value.
// Returns:
//   A raw 32-bit NEC message.
//
// Status: ALPHA / Untested.
//
// Ref:
//  http://www.sbprojects.com/knowledge/ir/nec.php
uint32_t ICACHE_FLASH_ATTR IRsend::encodeNEC(uint16_t address,
                                             uint16_t command) {
  command &= 0xFF;  // We only want the least significant byte of command.
  command = (command <<  8) + (command ^ 0xFF);  // Calculate the new command.
  if (address > 0xFF)  // Is it Extended NEC?
    return (address << 16) + command;  // Extended.
  else
    return (address << 24) + ((address ^ 0xFF) << 16) + command;  // Normal.
}

// Send an LG formatted message.
//
// Args:
//   data:   The contents of the message you want to send.
//   nbits:  The bit size of the message being sent. Typically LG_BITS.
//   repeat: The number of times you want the message to be repeated.
//
// Status: STABLE (< 32 bits), ALPHA (>= 32 bits)
//
// Notes:
//   LG has a separate message to indicate a repeat, like NEC does.
void ICACHE_FLASH_ATTR IRsend::sendLG(uint64_t data,
                                      uint16_t nbits,
                                      uint16_t repeat) {
  // Set IR carrier frequency
  enableIROut(38);

  uint16_t repeatHeaderMark = 0;
  IRtimer usecTimer = IRtimer();

  if (nbits >= 32) {
    // LG 32bit protocol is near identical to Samsung except for repeats.
    sendSAMSUNG(data, nbits, 0);  // Send it as a single Samsung message.
    repeatHeaderMark = LG32_HDR_MARK;
  } else {
    // LG (28-bit) protocol.
    repeatHeaderMark = LG_HDR_MARK;
    // Header
    usecTimer.reset();
    mark(LG_HDR_MARK);
    space(LG_HDR_SPACE);
    // Data
    sendData(LG_BIT_MARK, LG_ONE_SPACE, LG_BIT_MARK, LG_ZERO_SPACE,
             data, nbits, true);
    // Footer
    mark(LG_BIT_MARK);
    space(std::max((uint32_t) (LG_MIN_MESSAGE_LENGTH - usecTimer.elapsed()),
                   (uint32_t) LG_MIN_GAP));
  }
  // Repeat
  for (uint16_t i = 0; i < repeat; i++) {
    usecTimer.reset();
    mark(repeatHeaderMark);
    space(LG_RPT_SPACE);
    mark(LG_BIT_MARK);
    space(std::max((uint32_t) LG_MIN_MESSAGE_LENGTH - usecTimer.elapsed(),
                   (uint32_t) LG_MIN_GAP));
  }
}

// Construct a raw 28-bit LG message from the supplied address & command.
//
// Args:
//   address: The address code.
//   command: The command code.
// Returns:
//   A raw 28-bit LG message suitable for sendLG().
//
// Status: BETA / Should work.
//
// Notes:
//   e.g. Sequence of bits = address + command + checksum.
uint32_t ICACHE_FLASH_ATTR IRsend::encodeLG(uint8_t address, uint16_t command) {
  return ((address << 20) | (command << 4) | calcLGChecksum(command));
}

// Send a Whynter message.
//
// Args:
//   data: message to be sent.
//   nbits: Nr. of bits of the message to be sent.
//   repeat: Nr. of additional times the message is to be sent.
//
// Status: STABLE
//
// Ref:
//   https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Whynter.cpp
void ICACHE_FLASH_ATTR IRsend::sendWhynter(uint64_t data, uint16_t nbits,
                                           uint16_t repeat) {
  // Set IR carrier frequency
  enableIROut(38);
  IRtimer usecTimer = IRtimer();

  for (uint16_t i = 0; i <= repeat; i++) {
    usecTimer.reset();
    // Header
    mark(WHYNTER_BIT_MARK);
    space(WHYNTER_ZERO_SPACE);
    mark(WHYNTER_HDR_MARK);
    space(WHYNTER_HDR_SPACE);
    // Data
    sendData(WHYNTER_BIT_MARK, WHYNTER_ONE_SPACE, WHYNTER_BIT_MARK,
             WHYNTER_ZERO_SPACE, data, nbits, true);
    // Footer
    mark(WHYNTER_BIT_MARK);
    space(std::max(WHYNTER_MIN_COMMAND_LENGTH - usecTimer.elapsed(),
                   WHYNTER_MIN_GAP));
  }
}

// Send a Sony/SIRC(Serial Infra-Red Control) message.
//
// Args:
//   data: message to be sent.
//   nbits: Nr. of bits of the message to be sent.
//   repeat: Nr. of additional times the message is to be sent. (Default: 2)
//
// Status: BETA / Should work fine.
//
// Notes:
//   sendSony() should typically be called with repeat=2 as Sony devices
//   expect the message to be sent at least 3 times.
//
// Ref:
//   http://www.sbprojects.com/knowledge/ir/sirc.php
void ICACHE_FLASH_ATTR IRsend::sendSony(uint64_t data, uint16_t nbits,
                                        uint16_t repeat) {
  // Sony devices use a 40kHz IR carrier frequency & a 1/3 (33%) duty cycle.
  enableIROut(40, 33);
  IRtimer usecs = IRtimer();

  for (uint16_t i = 0; i <= repeat; i++) {  // Typically loop 3 or more times.
    usecs.reset();
    // Header
    mark(SONY_HDR_MARK);
    space(SONY_SPACE);
    // Data
    sendData(SONY_ONE_MARK, SONY_SPACE, SONY_ZERO_MARK, SONY_SPACE,
             data, nbits, true);
    // Footer
    // The Sony protocol requires us to wait 45ms from start of a code to the
    // start of the next one. A 10ms minimum gap is also required.
    space(std::max(SONY_MIN_GAP, SONY_RPT_LENGTH - usecs.elapsed()));
  }
  // A space() is always performed last, so no need to turn off the LED.
}

// Convert Sony/SIRC command, address, & extended bits into sendSony format.
// Args:
//   nbits:    Sony protocol bit size.
//   command:  Sony command bits.
//   address:  Sony address bits.
//   extended: Sony extended bits.
// Returns:
//   A sendSony compatible data message.
//
// Status: ALPHA / Untested.
uint32_t ICACHE_FLASH_ATTR IRsend::encodeSony(uint16_t nbits, uint16_t command,
                                              uint16_t address,
                                              uint16_t extended) {
  uint32_t result = 0;
  switch (nbits) {
    case 12:  // 5 address bits.
      result = address & 0x1F;
      break;
    case 15:  // 8 address bits.
      result = address & 0xFF;
      break;
    case 20:  // 5 address bits, 8 extended bits.
      result = address & 0x1F;
      result |= (extended & 0xFF) << 5;
      break;
    default:
      return 0;  // This is not an expected Sony bit size/protocol.
  }
  result = (result << 7) | (command & 0x7F);  // All sizes have 7 command bits.
  return reverseBits(result, nbits);  // sendSony uses reverse ordered bits.
}

// Send a raw IRremote message.
//
// Args:
//   buf: An array of uint16_t's that has microseconds elements.
//   len: Nr. of elements in the buf[] array.
//   hz:  Frequency to send the message at. (kHz < 1000; Hz >= 1000)
//
// Status: STABLE / Known working.
//
// Notes:
//   Even elements are Mark times (On), Odd elements are Space times (Off).
//
// Ref:
//   examples/IRrecvDumpV2/IRrecvDumpV2.ino
void ICACHE_FLASH_ATTR IRsend::sendRaw(uint16_t buf[], uint16_t len,
                                       uint16_t hz) {
  // Set IR carrier frequency
  enableIROut(hz);
  for (uint16_t i = 0; i < len; i++) {
    if (i & 1) {  // Odd bit.
      space(buf[i]);
    } else {  // Even bit.
      mark(buf[i]);
    }
  }
  ledOff();  // We potentially have ended with a mark(), so turn of the LED.
}

// Send a shortened GlobalCache (GC) IRdb/control tower formatted message.
//
// Args:
//   buf: An array of uint16_t containing the shortened GlobalCache data.
//   len: Nr. of entries in the buf[] array.
//
// Status: STABLE / Known working.
//
// Note:
//   Global Cache format without the emitter ID or request ID.
//   Starts at the frequency (Hertz), followed by nr. of times to emit (count),
//   then the offset for repeats (where a repeat will start from),
//   then the rest of entries are the actual IR message as units of periodic
//   time.
//   e.g. sendir,1:1,1,38000,1,1,9,70,9,30,9,... -> 38000,1,1,9,70,9,30,9,...
// Ref:
//   https://irdb.globalcache.com/Home/Database
void ICACHE_FLASH_ATTR IRsend::sendGC(uint16_t buf[], uint16_t len) {
  uint16_t hz = buf[GLOBALCACHE_FREQ_INDEX];  // GC frequency is in Hz.
  enableIROut(hz);
  uint32_t periodic_time = calcUSecPeriod(hz);
  uint8_t emits = std::min(buf[GLOBALCACHE_RPT_INDEX],
                           (uint16_t) GLOBALCACHE_MAX_REPEAT);
  // Repeat
  for (uint8_t repeat = 0; repeat < emits; repeat++) {
    // First time through, start at the beginning (GLOBALCACHE_START_INDEX),
    // otherwise for repeats, we start a specified offset from that.
    uint16_t offset = GLOBALCACHE_START_INDEX;
    if (repeat)
      offset += buf[GLOBALCACHE_RPT_START_INDEX] - 1;
    // Data
    for (; offset < len; offset++) {
      // Convert periodic units to microseconds.
      // Minimum is GLOBALCACHE_MIN_USEC for actual GC units.
      uint32_t microseconds = std::max(buf[offset] * periodic_time,
                                       GLOBALCACHE_MIN_USEC);
      // These codes start at an odd index (not even as with sendRaw).
      if (offset & 1)  // Odd bit.
        mark(microseconds);
      else  // Even bit.
        space(microseconds);
    }
  }
  // It's possible that we've ended on a mark(), thus ensure the LED is off.
  ledOff();
}

// Note: first bit must be a one (start bit)
void ICACHE_FLASH_ATTR IRsend::sendRC5(uint32_t data, uint16_t nbits) {
  // Set 36kHz IR carrier frequency & a 1/3 (33%) duty cycle.
  enableIROut(36, 33);
  // Header
  mark(RC5_T1);  // First start bit
  space(RC5_T1);  // Second start bit
  mark(RC5_T1);  // Second start bit
  // Data
  for (uint32_t mask = 1UL << (nbits - 1); mask; mask >>= 1) {
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

// Send a Philips RC-6 packet.
// Note: Caller needs to take care of flipping the toggle bit (The 4th Most
//   Significant Bit). That bit differentiates between key press & key release.
// Based on http://www.sbprojects.com/knowledge/ir/rc6.php
// Args:
//   data:    The code you wish to send.
//   nbits:   Bit size of the protocol you want to send.
//   repeat:  Nr. of extra times the data will be sent.
void ICACHE_FLASH_ATTR IRsend::sendRC6(uint64_t data, uint16_t nbits,
                                       uint16_t repeat) {
  // Check we can send the number of bits requested.
  if (nbits > sizeof(data) * 8)
    return;
  // Set 36kHz IR carrier frequency & a 1/3 (33%) duty cycle.
  enableIROut(36, 33);
  for (uint16_t r = 0; r <= repeat; r++) {
    // Header
    mark(RC6_HDR_MARK);
    space(RC6_HDR_SPACE);
    mark(RC6_T1);  // Start bit
    space(RC6_T1);
    uint16_t t;
    // Data
    for (uint64_t i = 0, mask = 1ULL << (nbits - 1); mask; i++, mask >>= 1) {
      // The fourth bit we send is a "double width trailer bit".
      if (i == 3)
        t = 2 * RC6_T1;  // double-wide trailer bit
      else
        t = RC6_T1;
      if (data & mask) {  // 1
        mark(t);
        space(t);
      } else {  // 0
        space(t);
        mark(t);
      }
    }
    // Footer
    space(RC6_RPT_LENGTH);
  }
}

// Send a Philips RC-MM packet.
// Based on http://www.sbprojects.com/knowledge/ir/rcmm.php
// Args:
//   data: The data we want to send. MSB first.
//   nbits: The number of bits of data to send. (Typically 12, 24, or 32[Nokia])
//
// Status:  ALPHA (untested and unconfirmed.)
void ICACHE_FLASH_ATTR IRsend::sendRCMM(uint32_t data, uint16_t nbits) {
  // Set 36kHz IR carrier frequency & a 1/3 (33%) duty cycle.
  enableIROut(36, 33);
  IRtimer usecs = IRtimer();

  // Header
  mark(RCMM_HDR_MARK);
  space(RCMM_HDR_SPACE);
  // Data
  uint32_t mask = 0b11 << (nbits - 2);
  // RC-MM sends data 2 bits at a time.
  for (uint8_t i = nbits; i > 0; i -= 2) {
    mark(RCMM_BIT_MARK);
    // Grab the next Most Significant Bits to send.
    switch ((data & mask) >> (i - 2)) {
      case 0b00: space(RCMM_BIT_SPACE_0); break;
      case 0b01: space(RCMM_BIT_SPACE_1); break;
      case 0b10: space(RCMM_BIT_SPACE_2); break;
      case 0b11: space(RCMM_BIT_SPACE_3); break;
    }
    mask >>= 2;
  }
  // Footer
  mark(RCMM_BIT_MARK);
  // Protocol requires us to wait at least RCMM_RPT_LENGTH usecs from the start
  // or RCMM_MIN_GAP usecs.
  space(std::max(RCMM_RPT_LENGTH - usecs.elapsed(), RCMM_MIN_GAP));
}

// Send a Panasonic formatted message.
//
// Args:
//   data:   The message to be sent.
//   nbits:  The number of bits of the message to be sent. (PANASONIC_BITS).
//   repeat: The number of times the command is to be repeated.
//
// Status: BETA / Should be working.
//
// Note:
//   This protocol is a modified version of Kaseikyo.
void ICACHE_FLASH_ATTR IRsend::sendPanasonic64(uint64_t data, uint16_t nbits,
                                               uint16_t repeat) {
  enableIROut(36700U);  // Set IR carrier frequency of 36.7kHz.
  IRtimer usecTimer = IRtimer();

  for (uint16_t i = 0; i <= repeat; i++) {
    usecTimer.reset();
    // Header
    mark(PANASONIC_HDR_MARK);
    space(PANASONIC_HDR_SPACE);
    // Data
    sendData(PANASONIC_BIT_MARK, PANASONIC_ONE_SPACE,
             PANASONIC_BIT_MARK, PANASONIC_ZERO_SPACE,
             data, nbits, true);
    // Footer
    mark(PANASONIC_BIT_MARK);
    space(std::max((uint32_t) PANASONIC_MIN_COMMAND_LENGTH -
                       usecTimer.elapsed(),
                   PANASONIC_MIN_GAP));
  }
}

// Send a Panasonic formatted message.
//
// Args:
//   address: The manufacturer code.
//   data:    The data portion to be sent.
//   nbits:   The number of bits of the message to be sent. (PANASONIC_BITS).
//   repeat:  The number of times the command is to be repeated.
//
// Status: BETA / Should be working.
//
// Note:
//   This protocol is a modified version of Kaseikyo.
void ICACHE_FLASH_ATTR IRsend::sendPanasonic(uint16_t address, uint32_t data,
                                             uint16_t nbits, uint16_t repeat) {
  sendPanasonic64(((uint64_t) address << 32) | (uint64_t) data, nbits, repeat);
}

// Calculate the raw Panasonic data based on device, subdevice, & function.
//
// Args:
//   device:    An 8-bit code.
//   subdevice: An 8-bit code.
//   function:  An 8-bit code.
// Returns:
//   A raw uint64_t Panasonic message.
//
// Status: ALPHA / Untested.
//
// Note:
//   Panasonic 48-bit protocol is a modified version of Kaseikyo.
// Ref:
//   http://www.remotecentral.com/cgi-bin/mboard/rc-pronto/thread.cgi?2615
uint64_t ICACHE_FLASH_ATTR IRsend::encodePanasonic(uint8_t device,
                                                   uint8_t subdevice,
                                                   uint8_t function) {
  uint8_t checksum = device ^ subdevice ^ function;
  return (((uint64_t) PANASONIC_MANUFACTURER << 32) |
          ((uint64_t) device << 24) |
          ((uint64_t) subdevice << 16) |
          ((uint64_t) function << 8) |
          checksum);
}

// Send a JVC message.
//
// Args:
//   data:   The contents of the command you want to send.
//   nbits:  The bit size of the command being sent. (JVC_BITS)
//   repeat: The number of times you want the command to be repeated.
//
// Status: STABLE.
//
// Ref:
//   http://www.sbprojects.com/knowledge/ir/jvc.php
void ICACHE_FLASH_ATTR IRsend::sendJVC(uint64_t data, uint16_t nbits,
                                       uint16_t repeat) {
  // Set 38kHz IR carrier frequency & a 1/3 (33%) duty cycle.
  enableIROut(38, 33);

  IRtimer usecs = IRtimer();
  // Header
  // Only sent for the first message.
  mark(JVC_HDR_MARK);
  space(JVC_HDR_SPACE);

  // We always send the data & footer at least once, hence '<= repeat'.
  for (uint16_t i = 0; i <= repeat; i++) {
    // Data
    sendData(JVC_BIT_MARK, JVC_ONE_SPACE, JVC_BIT_MARK, JVC_ZERO_SPACE,
             data, nbits, true);
    // Footer
    mark(JVC_BIT_MARK);
    // Wait till the end of the repeat time window before we send another code.
    space(std::max(JVC_MIN_GAP, JVC_RPT_LENGTH - usecs.elapsed()));
    usecs.reset();
  }
}

// Calculate the raw JVC data based on address and command.
//
// Args:
//   address: An 8-bit address value.
//   command: An 8-bit command value.
// Returns:
//   A raw JVC message.
//
// Status: ALPHA / Untested.
//
// Ref:
//   http://www.sbprojects.com/knowledge/ir/jvc.php
uint16_t ICACHE_FLASH_ATTR IRsend::encodeJVC(uint8_t address, uint8_t command) {
  return reverseBits((command << 8) | address, 16);
}

// Send a Samsung formatted message.
// Samsung has a separate message to indicate a repeat, like NEC does.
// TODO(crankyoldgit): Confirm that is actually how Samsung sends a repeat.
//                     The refdoc doesn't indicate it is true.
//
// Args:
//   data:   The message to be sent.
//   nbits:  The bit size of the message being sent. typically SAMSUNG_BITS.
//   repeat: The number of times the message is to be repeated.
//
// Status: BETA / Should be working.
//
// Ref: http://elektrolab.wz.cz/katalog/samsung_protocol.pdf
void ICACHE_FLASH_ATTR IRsend::sendSAMSUNG(uint64_t data, uint16_t nbits,
                                           uint16_t repeat) {
  // Set 38kHz IR carrier frequency & a 1/3 (33%) duty cycle.
  enableIROut(38, 33);
  IRtimer usecTimer = IRtimer();
  // We always send a message, even for repeat=0, hence '<= repeat'.
  for (uint16_t i=0; i <= repeat; i++) {
    usecTimer.reset();
    // Header
    mark(SAMSUNG_HDR_MARK);
    space(SAMSUNG_HDR_SPACE);
    // Data
    sendData(SAMSUNG_BIT_MARK, SAMSUNG_ONE_SPACE, SAMSUNG_BIT_MARK,
             SAMSUNG_ZERO_SPACE, data, nbits, true);
    // Footer
    mark(SAMSUNG_BIT_MARK);
    space(std::max((uint32_t) SAMSUNG_MIN_GAP,
                   (uint32_t) (SAMSUNG_MIN_MESSAGE_LENGTH -
                               usecTimer.elapsed())));
  }
}

// Construct a raw Samsung message from the supplied customer(address) &
// command.
//
// Args:
//   customer: The customer code. (aka. Address)
//   command:  The command code.
// Returns:
//   A raw 32-bit Samsung message suitable for sendSAMSUNG().
//
// Status: BETA / Should be working.
uint32_t ICACHE_FLASH_ATTR IRsend::encodeSAMSUNG(uint8_t customer,
                                                 uint8_t command) {
  customer = reverseBits(customer, sizeof(customer));
  command = reverseBits(command, sizeof(command));
  return(((command ^ 0xFF) << 24) | (command << 16) |
         (customer << 8) | customer);
}

// Send a Denon message
//
// Args:
//   data:   Contents of the message to be sent.
//   nbits:  Nr. of bits of data to be sent. Typically DENON_BITS.
//   repeat: Nr. of additional times the message is to be sent.
//
// Status: BETA / Should be working.
//
// Notes:
//   Some Denon devices use a Kaseikyo/Panasonic 48-bit format.
// Ref:
//   https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Denon.cpp
//   http://assets.denon.com/documentmaster/us/denon%20master%20ir%20hex.xls
void ICACHE_FLASH_ATTR IRsend::sendDenon(uint64_t data, uint16_t nbits,
                                         uint16_t repeat) {
  enableIROut(38);  // Set IR carrier frequency
  IRtimer usecTimer = IRtimer();

  for (uint16_t i = 0; i <= repeat; i++) {
    usecTimer.reset();
    // Header
    mark(DENON_HDR_MARK);
    space(DENON_HDR_SPACE);
    // Data
    sendData(DENON_BIT_MARK, DENON_ONE_SPACE, DENON_BIT_MARK, DENON_ZERO_SPACE,
             data, nbits, true);
    // Footer
    mark(DENON_BIT_MARK);
    space(std::max(DENON_MIN_COMMAND_LENGTH - usecTimer.elapsed(),
                   DENON_MIN_GAP));
  }
}

// Modulate the IR LED for the given period (usec) and at the duty cycle set.
//
// Args:
//   usec: The period of time to modulate the IR LED for, in microseconds.
//
// Note:
//   The ESP8266 has no good way to do hardware PWM, so we have to do it all
//   in software. There is a horrible kludge/brilliant hack to use the second
//   serial TX line to do fairly accurate hardware PWM, but it is only
//   available on a single specific GPIO and only available on some modules.
//   e.g. It's not available on the ESP-01 module.
//   Hence, for greater compatiblity & choice, we don't use that method.
// Ref:
//   https://www.analysir.com/blog/2017/01/29/updated-esp8266-nodemcu-backdoor-upwm-hack-for-ir-signals/
void ICACHE_FLASH_ATTR IRsend::mark(uint16_t usec) {
  IRtimer usecTimer = IRtimer();
  // Cache the time taken so far. This saves us calling time, and we can be
  // assured that we can't have odd math problems. i.e. unsigned under/overflow.
  uint32_t elapsed = usecTimer.elapsed();

  while (elapsed < usec) {  // Loop until we've met/exceeded our required time.
    digitalWrite(IRpin, HIGH);  // Turn the LED on.
    // Calculate how long we should pulse on for.
    // e.g. Are we to close to the end of our requested mark time (usec)?
    delayMicroseconds(std::min((uint32_t) onTimePeriod, usec - elapsed));
    digitalWrite(IRpin, LOW);  // Turn the LED off.
    if (elapsed + onTimePeriod >= usec)
      return;  // LED is now off & we've passed our allotted time. Safe to stop.
    // Wait for the lesser of the rest of the duty cycle, or the time remaining.
    delayMicroseconds(std::min(usec - elapsed - onTimePeriod,
                               (uint32_t) offTimePeriod));
    elapsed = usecTimer.elapsed();  // Update & recache the actual elapsed time.
  }
}

void ICACHE_FLASH_ATTR IRsend::ledOff() {
  digitalWrite(IRpin, LOW);
}

// Turn the pin (LED) off for a given time.
// Sends an IR space for the specified number of microseconds.
// A space is no output, so the PWM output is disabled.
//
// Args:
//   time: Time in microseconds (us).
void ICACHE_FLASH_ATTR IRsend::space(uint32_t time) {
  ledOff();
  if (time == 0) return;
  // delayMicroseconds is only accurate to 16383us.
  // Ref: https://www.arduino.cc/en/Reference/delayMicroseconds
  if (time <= 16383) {
    delayMicroseconds(time);
  } else {
    // Invoke a delay(), where possible, to avoid triggering the WDT.
    delay(time / 1000UL);  // Delay for as many whole milliseconds as we can.
    // Delay the remaining sub-millisecond.
    delayMicroseconds(static_cast<uint16_t>(time % 1000UL));
  }
}

// Calculate the period for a given frequency. (T = 1/f)
//
// Args:
//   freq: Frequency in Hz.
// Returns:
//   nr. of uSeconds.
uint32_t ICACHE_FLASH_ATTR IRsend::calcUSecPeriod(uint32_t hz) {
  return (1000000UL + hz/2) / hz;  // round(1000000/hz).
}

// Set the output frequency modulation and duty cycle.
//
// Args:
//   freq: The freq we want to modulate at. Assumes < 1000 means kHz else Hz.
//   duty: Percentage duty cycle of the LED. e.g. 25 = 25% = 1/4 on, 3/4 off.
//
// Note:
//   Integer timing functions & math mean we can't do fractions of
//   microseconds timing. Thus minor changes to the freq & duty values may have
//   limited effect. You've been warned.
void ICACHE_FLASH_ATTR IRsend::enableIROut(uint32_t freq, uint8_t duty) {
  duty = std::min(duty,
                  (uint8_t) 100);  // Can't have more than 100% duty cycle.
  if (freq < 1000)  // Were we given kHz? Supports the old call usage.
    freq *= 1000;
  uint32_t period = calcUSecPeriod(freq);
  // Nr. of uSeconds the LED will be on per pulse.
  onTimePeriod = (period * duty) / 100;
  // Nr. of uSeconds the LED will be off per pulse.
  offTimePeriod = period - onTimePeriod;
}

// Send a (raw) Sharp message
//
// Args:
//   data:   Contents of the message to be sent.
//   nbits:  Nr. of bits of data to be sent. Typically SHARP_BITS.
//   repeat: Nr. of additional times the message is to be sent.
//
// Status: BETA / Previously working fine.
//
// Notes:
//   This procedure handles the inversion of bits required per protocol.
//   The protocol spec says to send the LSB first, but legacy code & usage
//   has us sending the MSB first. Grrrr. Normal invocation of encodeSharp()
//   handles this for you, assuming you are using the correct/standard values.
//   e.g. sendSharpRaw(encodeSharp(address, command));
//
// Ref:
//   http://www.sbprojects.com/knowledge/ir/sharp.htm
//   http://lirc.sourceforge.net/remotes/sharp/GA538WJSA
//   http://www.mwftr.com/ucF08/LEC14%20PIC%20IR.pdf
//   http://www.hifi-remote.com/johnsfine/DecodeIR.html#Sharp
void ICACHE_FLASH_ATTR IRsend::sendSharpRaw(uint64_t data, uint16_t nbits,
                                            uint16_t repeat) {
  // Set 38kHz IR carrier frequency & a 1/3 (33%) duty cycle.
  enableIROut(38, 33);

  for (uint16_t i = 0; i <= repeat; i++) {
    // Protocol demands that the data be sent twice; once normally,
    // then with all but the address bits inverted.
    // Note: Previously this used to be performed 3 times (normal, inverted,
    //       normal), however all data points to that being incorrect.
    for (uint8_t n = 0; n < 2; n++) {
      // No Header

      // Data
      sendData(SHARP_BIT_MARK, SHARP_ONE_SPACE,
               SHARP_BIT_MARK, SHARP_ZERO_SPACE,
               data, nbits, true);
      // Footer
      mark(SHARP_BIT_MARK);
      space(SHARP_GAP);

      // Invert the data per protocol. This is always called twice, so it's
      // retured to original upon exiting the inner loop.
      data ^= SHARP_TOGGLE_MASK;
    }
  }
}

// Encode a (raw) Sharp message from it's components.
//
// Args:
//   address:   The value of the address to be sent.
//   command:   The value of the address to be sent. (8 bits)
//   expansion: The value of the expansion bit to use. (0 or 1, typically 1)
//   check:     The value of the check bit to use. (0 or 1, typically 0)
//   MSBfirst:  Flag indicating MSB first or LSB first order. (Default: false)
// Returns:
//   An uint32_t containing the raw Sharp message for sendSharpRaw().
//
// Status: ALPHA / Untested
//
// Notes:
//   Assumes the standard Sharp bit sizes.
//   Historically sendSharp() sends address & command in
//     MSB first order. This is actually incorrect. It should be sent in LSB
//     order. The behaviour of sendSharp() hasn't been changed to maintain
//     backward compatiblity.
//
// Ref:
//   http://www.sbprojects.com/knowledge/ir/sharp.htm
//   http://lirc.sourceforge.net/remotes/sharp/GA538WJSA
//   http://www.mwftr.com/ucF08/LEC14%20PIC%20IR.pdf
uint32_t ICACHE_FLASH_ATTR IRsend::encodeSharp(uint16_t address,
                                               uint16_t command,
                                               uint16_t expansion,
                                               uint16_t check,
                                               bool MSBfirst) {
  // Mask any unexpected bits.
  address &= ((1 << SHARP_ADDRESS_BITS) - 1);
  command &= ((1 << SHARP_COMMAND_BITS) - 1);
  expansion &= 1;
  check &= 1;

  if (!MSBfirst) {  // Correct bit order if needed.
    address = reverseBits(address, SHARP_ADDRESS_BITS);
    command = reverseBits(command, SHARP_COMMAND_BITS);
  }
  // Concatinate all the bits.
  return (address << (SHARP_COMMAND_BITS + 2)) | (command << 2) |
         (expansion << 1) | check;
}

// Send a Sharp message
//
// Args:
//   address:  Address value to be sent.
//   command:  Command value to be sent.
//   nbits:    Nr. of bits of data to be sent. Typically SHARP_BITS.
//   repeat:   Nr. of additional times the message is to be sent.
//
// Status:  DEPRICATED / Previously working fine.
//
// Notes:
//   This procedure has a non-standard invocation style compared to similar
//     sendProtocol() routines. This is due to legacy, compatiblity, & historic
//     reasons. Normally the calling syntax version is like sendSharpRaw().
//   This procedure transmits the address & command in MSB first order, which is
//     incorrect. This behaviour is left as-is to maintain backward
//     compatiblity with legacy code.
//   In short, you should use sendSharpRaw(), encodeSharp(), and the correct
//     values of address & command instead of using this, & the wrong values.
//
// Ref:
//   http://www.sbprojects.com/knowledge/ir/sharp.htm
//   http://lirc.sourceforge.net/remotes/sharp/GA538WJSA
//   http://www.mwftr.com/ucF08/LEC14%20PIC%20IR.pdf
void ICACHE_FLASH_ATTR IRsend::sendSharp(uint16_t address, uint16_t command,
                                         uint16_t nbits, uint16_t repeat) {
  sendSharpRaw(encodeSharp(address, command, 1, 0, true), nbits, repeat);
}

// Send an IR command to a DISH NETWORK device.
//
// Args:
//   data:   The contents of the command you want to send.
//   nbits:  The bit size of the command being sent.
//   repeat: The number of times you want the command to be repeated.
//
// Status: BETA / Previously working.
//
// Note:
//   Dishplayer is a different protocol.
//   Typically a DISH device needs to get a command a total of at least 4
//   times to accept it. e.g. repeat=3
//   DISH support by Todd Treece (http://unionbridge.org/design/ircommand)
//
//   Here is the LIRC file I found that seems to match the remote codes from the
//   oscilloscope:
//     DISH NETWORK (echostar 301):
//     http://lirc.sourceforge.net/remotes/echostar/301_501_3100_5100_58xx_59xx
//
// Ref:
//   http://www.hifi-remote.com/wiki/index.php?title=Dish
void ICACHE_FLASH_ATTR IRsend::sendDISH(uint64_t data, uint16_t nbits,
                                        uint16_t repeat) {
  // Set 57.6kHz IR carrier frequency, duty cycle is unknown.
  enableIROut(57600);
  // We always send a command, even for repeat=0, hence '<= repeat'.
  for (uint16_t i = 0; i <= repeat; i++) {
    // Header
    mark(DISH_HDR_MARK);
    space(DISH_HDR_SPACE);
    // Data
    sendData(DISH_BIT_MARK, DISH_ONE_SPACE, DISH_BIT_MARK, DISH_ZERO_SPACE,
             data, nbits, true);
    // Footer
    mark(DISH_BIT_MARK);
    space(DISH_RPT_SPACE);
  }
}

// Construct a Sanyo LC7461 message.
//
// Args:
//   address: The 13 bit value of the address(Custom) portion of the protocol.
//   command: The 8 bit value of the command(Key) portion of the protocol.
// Returns:
//   An uint64_t with the encoded raw 42 bit Sanyo LC7461 data value.
//
// Notes:
//   This protocol uses the NEC protocol timings. However, data is
//   formatted as : address(13 bits), !address, command(8 bits), !command.
//   According with LIRC, this protocol is used on Sanyo, Aiwa and Chinon
uint64_t ICACHE_FLASH_ATTR IRsend::encodeSanyoLC7461(uint16_t address,
                                                     uint16_t command) {
  // Mask our input values to ensure the correct bit sizes.
  address &= SANYO_LC7461_ADDRESS_MASK;
  command &= SANYO_LC7461_COMMAND_MASK;

  uint64_t data = address;
  address ^= SANYO_LC7461_ADDRESS_MASK;  // Invert the 13 LSBs.
  // Append the now inverted address.
  data = (data << SANYO_LC7461_ADDRESS_BITS) | address;
  // Append the command.
  data = (data << SANYO_LC7461_COMMAND_BITS) | command;
  command ^= SANYO_LC7461_COMMAND_MASK;  // Invert the command.
  // Append the now inverted command.
  data = (data << SANYO_LC7461_COMMAND_BITS) | command;

  return data;
}

// Send a Sanyo LC7461 message.
//
// Args:
//   data:   The contents of the command you want to send.
//   nbits:  The bit size of the command being sent.
//   repeat: The number of times you want the command to be repeated.
//
// Status: ALPHA / untested.
//
// Notes:
//   Based on @marcosamarinho's work.
//   This protocol uses the NEC protocol timings. However, data is
//   formatted as : address(13 bits), !address, command (8 bits), !command.
//   According with LIRC, this protocol is used on Sanyo, Aiwa and Chinon
//   Information for this protocol is available at the Sanyo LC7461 datasheet.
//   Repeats are performed similar to the NEC method of sending a special
//   repeat message, rather than duplicating the entire message.
// Ref:
//   https://github.com/marcosamarinho/IRremoteESP8266/blob/master/ir_Sanyo.cpp
//   http://pdf.datasheetcatalog.com/datasheet/sanyo/LC7461.pdf
void IRsend::sendSanyoLC7461(uint64_t data, uint16_t nbits, uint16_t repeat) {
  // Set 38kHz IR carrier frequency & a 1/3 (33%) duty cycle.
  enableIROut(38, 33);
  IRtimer usecTimer = IRtimer();

  // Header
  mark(SANYO_LC7461_HDR_MARK);
  space(SANYO_LC7461_HDR_SPACE);
  // Data
  sendData(SANYO_LC7461_BIT_MARK, SANYO_LC7461_ONE_SPACE,
           SANYO_LC7461_BIT_MARK, SANYO_LC7461_ZERO_SPACE, data, nbits, true);
  // Footer
  mark(SANYO_LC7461_BIT_MARK);
  space(std::max(SANYO_LC7461_MIN_COMMAND_LENGTH - usecTimer.elapsed(),
                 SANYO_LC7461_MIN_GAP));

  // Repeat
  // Similar to the NEC protocol, sending a special repeat message to indicate
  // a repeat, rather than duplicating the entire message.
  for (uint16_t i = 0; i < repeat; i++) {
    usecTimer.reset();
    // Header
    mark(SANYO_LC7461_HDR_MARK);
    space(SANYO_LC7461_HDR_SPACE);
    // Footer
    mark(SANYO_LC7461_BIT_MARK);
    space(std::max(SANYO_LC7461_MIN_COMMAND_LENGTH - usecTimer.elapsed(),
                   SANYO_LC7461_MIN_GAP));
  }
}

// Send a Daikin A/C message.
//
// Args:
//   data: An array of DAIKIN_COMMAND_LENGTH bytes containing the IR command.
//
// Status: STABLE
//
// Ref:
//   IRDaikinESP.cpp
//   https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/tree/master/IRremote
void ICACHE_FLASH_ATTR IRsend::sendDaikin(unsigned char data[]) {
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

// Send a Kelvinator A/C message.
//
// Args:
//   data: An array of KELVINATOR_STATE_LENGTH bytes containing the IR command.
//
// Status: STABLE / Known working.
//
// Ref:
//   IRKelvinator.cpp
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

// Send an IR command to a Sherwood device.
//
// Args:
//   data:   The contents of the command you want to send.
//   nbits:  The bit size of the command being sent. (SHERWOOD_BITS)
//   repeat: The nr. of times you want the command to be repeated. (Default: 1)
//
// Status: STABLE / Known working.
//
// Note:
//   Sherwood remote codes appear to be NEC codes with a manditory repeat code.
//   i.e. repeat should be >= SHERWOOD_MIN_REPEAT (1).
void ICACHE_FLASH_ATTR IRsend::sendSherwood(uint64_t data, uint16_t nbits,
                                            uint16_t repeat) {
  sendNEC(data, nbits, std::max((uint16_t) SHERWOOD_MIN_REPEAT, repeat));
}

// Send a Mitsubishi message
//
// Args:
//   data:   Contents of the message to be sent.
//   nbits:  Nr. of bits of data to be sent. Typically MITSUBISHI_BITS.
//   repeat: Nr. of additional times the message is to be sent.
//
// Status: ALPHA / untested.
//
// Notes:
//   This protocol appears to have no header.
// Ref:
//   https://github.com/marcosamarinho/IRremoteESP8266/blob/master/ir_Mitsubishi.cpp
//   GlobalCache's Control Tower's Mitsubishi TV data.
void ICACHE_FLASH_ATTR IRsend::sendMitsubishi(uint64_t data, uint16_t nbits,
                                              uint16_t repeat) {
  enableIROut(33);  // Set IR carrier frequency
  IRtimer usecTimer = IRtimer();

  for (uint16_t i = 0; i <= repeat; i++) {
    usecTimer.reset();
    // No header

    // Data
    sendData(MITSUBISHI_BIT_MARK, MITSUBISHI_ONE_SPACE,
             MITSUBISHI_BIT_MARK, MITSUBISHI_ZERO_SPACE,
             data, nbits, true);
    // Footer
    mark(MITSUBISHI_BIT_MARK);
    space(std::max(MITSUBISHI_MIN_COMMAND_LENGTH - usecTimer.elapsed(),
                   MITSUBISHI_MIN_GAP));
  }
}

// Send a Mitsubishi A/C message.
//
// Args:
//   data: Array of MITSUBISHI_AC_STATE_LENGTH bytes containing the IR command.
//
// Status: BETA / Appears to be working.
//
// Ref:
//   IRMitsubishiAC.cpp
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
}
// ---------------------------------------------------------------


// IRRecv------------------------------------------------------

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

// Reverse the order of the requested least significant nr. of bits.
// Args:
//   input: Bit pattern/integer to reverse.
//   nbits: Nr. of bits to reverse.
// Returns:
//   The reversed bit pattern.
uint64_t reverseBits(uint64_t input, uint16_t nbits) {
  uint64_t output = input;
  for (uint16_t i = 1;
       i < std::min(nbits, (uint16_t) (sizeof(input) * 8));
       i++) {
    output <<= 1;
    input  >>= 1;
    output |= (input & 1);
  }
  return output;
}

// Calculate the rolling 4-bit wide checksum over all of the data.
//  Args:
//    data: The value to be checksum'ed.
//  Returns:
//    A 4-bit checksum.
uint8_t ICACHE_FLASH_ATTR calcLGChecksum(uint16_t data) {
  return(((data >> 12) + ((data >> 8) & 0xF) + ((data >> 4) & 0xF) +
         (data & 0xF)) & 0xF);
}

IRrecv::IRrecv(uint16_t recvpin) {
  irparams.recvpin = recvpin;
}

// initialization
void ICACHE_FLASH_ATTR IRrecv::enableIRIn() {
  // initialize state machine variables
  resume();

  // Initialize timer
  os_timer_disarm(&timer);
  os_timer_setfn(&timer, reinterpret_cast<os_timer_func_t *>(read_timeout),
                 NULL);

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
  char *csrc = (char *) (&irparams);  // NOLINT(readability/casting)
  char *cdest = (char *) dest;  // NOLINT(readability/casting)

  // Copy contents of src[] to dest[]
  for (uint16_t i=0; i < sizeof(irparams_t); i++)
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
  if (irparams.rcvstate != STATE_STOP)
    return false;

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
  Serial.println("Attempting Sanyo LC7461 decode");
#endif
  // Try decodeSanyoLC7461() before decodeNEC() because the protocols are
  // similar in timings & structure, but the Sanyo one is much longer than the
  // NEC protocol (42 vs 32 bits) so this one should be tried first to try to
  // reduce false detection as a NEC packet.
  if (decodeSanyoLC7461(results))
    return true;
#ifdef DEBUG
  Serial.println("Attempting NEC decode");
#endif
  if (decodeNEC(results))
    return true;
#ifdef DEBUG
  Serial.println("Attempting Sony decode");
#endif
  if (decodeSony(results))
    return true;
#ifdef DEBUG
  Serial.println("Attempting Mitsubishi decode");
#endif
  if (decodeMitsubishi(results))
    return true;
#ifdef DEBUG
  Serial.println("Attempting RC5 decode");
#endif
  if (decodeRC5(results))
    return true;
#ifdef DEBUG
  Serial.println("Attempting RC6 decode");
#endif
  if (decodeRC6(results))
    return true;
#ifdef DEBUG
  Serial.println("Attempting RC-MM decode");
#endif
  if (decodeRCMM(results))
    return true;
#ifdef DEBUG
  Serial.println("Attempting Panasonic decode");
#endif
  if (decodePanasonic(results))
    return true;
#ifdef DEBUG
  Serial.println("Attempting LG decode");
#endif
  if (decodeLG(results))
      return true;
#ifdef DEBUG
  Serial.println("Attempting JVC decode");
#endif
  if (decodeJVC(results))
    return true;
#ifdef DEBUG
  Serial.println("Attempting SAMSUNG decode");
#endif
  if (decodeSAMSUNG(results))
    return true;
#ifdef DEBUG
  Serial.println("Attempting Whynter decode");
#endif
  if (decodeWhynter(results))
    return true;
#ifdef DEBUG
  Serial.println("Attempting Denon decode");
#endif
  if (decodeDenon(results))
    return true;
#ifdef DEBUG
  Serial.println("Attempting DISH decode");
#endif
  if (decodeDISH(results))
    return true;
#ifdef DEBUG
  Serial.println("Attempting Sharp decode");
#endif
  if (decodeSharp(results))
    return true;
#ifdef DEBUG
  Serial.println("Attempting Coolix decode");
#endif
  if (decodeCOOLIX(results))
    return true;
  // The Sanyo S866500B decoder is very poor quality & depricated.
  // *IF* you are going to enable it, do it near last to avoid false positive
  // matches.
  /*********** DISABLED & DEPRICATED **********
#ifdef DEBUG
  Serial.println("Attempting Sanyo SA8650B decode");
#endif
  if (decodeSanyo(results))
    return true;
  ********** DISABLED & DEPRICATED ***********/
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
  return((uint32_t) std::max((int32_t) (
      usecs * (1.0 - tolerance/100.0) / USECPERTICK), 0));
}

// Calculate the upper bound of the nr. of ticks.
//
// Args:
//   usecs:  Nr. of uSeconds.
//   tolerance:  Percent as an integer. e.g. 10 is 10%
// Returns:
//   Nr. of ticks.
uint32_t IRrecv::ticksHigh(uint32_t usecs, uint8_t tolerance) {
  return((uint32_t) usecs * (1.0 + tolerance/100.0) / USECPERTICK + 1);
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
                                         uint8_t tolerance, int16_t excess) {
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
                                          uint8_t tolerance, int16_t excess) {
  #ifdef DEBUG
    Serial.print("Matching SPACE ");
    Serial.print(measured_ticks * USECPERTICK, DEC);
    Serial.print(" vs ");
    Serial.print(desired_us, DEC);
    Serial.print(". ");
  #endif
  return match(measured_ticks, desired_us - excess, tolerance);
}

// Decode the supplied NEC message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect. Typically NEC_BITS.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: STABLE / Known good.
//
// Notes:
//   NEC protocol has three varients/forms.
//     Normal:   a 8 bit address & a 8 bit command in 32 bit data form.
//               i.e. address + inverted(address) + command + inverted(command)
//     Extended: a 16 bit address & a 8 bit command in 32 bit data form.
//               i.e. address + command + inverted(command)
//     Repeat:   a 0-bit code. i.e. No data bits. Just the header + footer.
//
// Ref:
//   http://www.sbprojects.com/knowledge/ir/nec.php
bool ICACHE_FLASH_ATTR IRrecv::decodeNEC(decode_results *results,
                                         uint16_t nbits, bool strict) {
  if (results->rawlen < 2 * nbits + HEADER + FOOTER &&
      results->rawlen != NEC_RPT_LENGTH)
    return false;  // Can't possibly be a valid NEC message.
  if (strict && nbits != NEC_BITS)
    return false;  // Not strictly an NEC message.

  uint64_t data = 0;
  uint16_t offset = OFFSET_START;

  // Header
  if (!matchMark(results->rawbuf[offset++], NEC_HDR_MARK))
    return false;
  // Check if it is a repeat code.
  if (results->rawlen == NEC_RPT_LENGTH &&
      matchSpace(results->rawbuf[offset], NEC_RPT_SPACE) &&
      matchMark(results->rawbuf[offset + 1], NEC_BIT_MARK)) {
    results->value = REPEAT;
    results->decode_type = NEC;
    results->bits = 0;
    results->address = 0;
    results->command = 0;
    return true;
  }

  // Header (cont.)
  if (!matchSpace(results->rawbuf[offset++], NEC_HDR_SPACE))
    return false;
  // Data
  for (uint16_t i = 0; i < nbits; i++, offset++) {
    if (!matchMark(results->rawbuf[offset++], NEC_BIT_MARK))
      return false;
    if (matchSpace(results->rawbuf[offset], NEC_ONE_SPACE))
      data = (data << 1) | 1;
    else if (matchSpace(results->rawbuf[offset], NEC_ZERO_SPACE))
      data <<= 1;
    else
      return false;
  }
  // Footer
  if (!matchMark(results->rawbuf[offset], NEC_BIT_MARK))
      return false;

  // Compliance
  // Calculate command and optionally enforce integrity checking.
  uint8_t command = (data & 0xFF00) >>  8;
  // Command is sent twice, once as plain and then inverted .
  if (strict && (command ^ 0xFF) != (data & 0xFF))
    return false;  // Command integrity failed.

  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = NEC;
  results->command = command;
  // Normal NEC protocol has an 8 bit address sent, followed by it inverted.
  uint8_t address = (data & 0xFF000000) >> 24;
  uint8_t address_inverted = (data & 0x00FF0000) >> 16;
  if (address == (address_inverted ^ 0xFF))
    results->address = address;  // Inverted, so it is normal NEC protocol.
  else  // Not inverted, so must be Extended NEC protocol, thus 16 bit address.
    results->address = data >> 16;  // Most significant four bytes.
  return true;
}

// Decode the supplied Coolix message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect. Typically COOLIX_BITS.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: ALPHA / untested.
bool ICACHE_FLASH_ATTR IRrecv::decodeCOOLIX(decode_results *results,
                                            uint16_t nbits,
                                            bool strict) {
  // The protocol sends the data normal + inverted, alternating on
  // each byte. Hence twice the number of expected data bits.
  if (results->rawlen < 2 * 2 * nbits + HEADER + FOOTER)
    return false;  // Can't possibly be a valid COOLIX message.
  if (strict && nbits != COOLIX_BITS)
    return false;  // Not strictly an COOLIX message.
  if (nbits % 8 != 0)  // nbits has to be a multiple of nr. of bits in a byte.
    return false;

  uint64_t data = 0;
  uint16_t offset = OFFSET_START;

  if (nbits * 2 > sizeof(data) * 8)
    return false;  // We can't possibly capture a Coolix packet that big.

  // Header
  if (!matchMark(results->rawbuf[offset++], COOLIX_HDR_MARK))
    return false;
  if (!matchSpace(results->rawbuf[offset++], COOLIX_HDR_SPACE))
    return false;

  // Data
  // Twice as many bits as there are normal plus inverted bits.
  for (uint16_t i = 0; i < nbits * 2; i++, offset++) {
    if (!matchMark(results->rawbuf[offset++], COOLIX_BIT_MARK))
      return false;
    if (matchSpace(results->rawbuf[offset], COOLIX_ONE_SPACE))
      data = (data << 1) | 1;  // 1
    else if (matchSpace(results->rawbuf[offset], COOLIX_ZERO_SPACE))
      data <<= 1;  // 0
    else
      return false;
  }

  // Footer
  if (!matchMark(results->rawbuf[offset], COOLIX_BIT_MARK))
      return false;

  // Data should now be (MSB to LSB) byte1,!byte1,byte2,!byte2,byte3,!byte3
  // Decode, and verify if needed.
  uint64_t result = 0;  // Build a new result for we destroy the existing data.
  for (uint16_t i = 0; i < nbits; i += 8) {
    uint8_t inverted = (data & 0xFF) ^ 0xFF;  // Un-invert the byte.
    data >>= 8;
    uint8_t normal = data & 0xFF;
    data >>= 8;
    if (strict && data != inverted)  // Compliance
      return false;  // The message doesn't verify.
    result |= (normal << i);  // Add the byte in front of the previous result.
  }

  // Success
  results->decode_type = COOLIX;
  results->bits = nbits;
  results->value = result;
  results->address = 0;
  results->command = 0;
  return true;
}

// Decode the supplied Sony/SIRC message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: BETA / Should be working. strict mode is ALPHA / Untested.
//
// Notes:
//   SONY protocol, SIRC (Serial Infra-Red Control) can be 12,15,20 bits long.
// Ref:
// http://www.sbprojects.com/knowledge/ir/sirc.php
bool ICACHE_FLASH_ATTR IRrecv::decodeSony(decode_results *results,
                                          uint16_t nbits, bool strict) {
  if (results->rawlen < 2 * nbits + HEADER)
    return false;  // Message is smaller than we expected.

  // Compliance
  if (strict) {
    switch (nbits) {  // Check we've been called with a correct bit size.
      case 12:
      case 15:
      case 20:
        break;
      default:
        return false;  // The request doesn't strictly match the protocol defn.
    }
  }

  uint32_t data = 0;
  uint16_t offset = OFFSET_START;
  uint16_t actualBits;
  uint32_t timeSoFar = 0;  // Time in uSecs of the message length.

  // Header
  timeSoFar += results->rawbuf[offset] * USECPERTICK;
  if (!matchMark(results->rawbuf[offset++], SONY_HDR_MARK))
    return false;
  // Data
  for (actualBits = 0; offset < results->rawlen - 1; actualBits++, offset++) {
    // The gap after a Sony packet for a repeat should be SONY_MIN_GAP or
    //   (SONY_RPT_LENGTH - timeSoFar) according to the spec.
    if (matchSpace(results->rawbuf[offset], SONY_MIN_GAP) ||
        matchSpace(results->rawbuf[offset], SONY_RPT_LENGTH - timeSoFar))
      break;  // Found a repeat space.
    timeSoFar += results->rawbuf[offset] * USECPERTICK;
    if (!matchSpace(results->rawbuf[offset++], SONY_SPACE))
      return false;
    timeSoFar += results->rawbuf[offset] * USECPERTICK;
    if (matchMark(results->rawbuf[offset], SONY_ONE_MARK))
      data = (data << 1) | 1;
    else if (matchMark(results->rawbuf[offset], SONY_ZERO_MARK))
      data <<= 1;
    else
      return false;
  }
  // No Footer for Sony.

  // Compliance
  if (strict && actualBits != nbits)
    return false;  // We got the wrong number of bits.

  // Success
  results->bits = actualBits;
  results->value = data;
  results->decode_type = SONY;
  // Message comes in LSB first. Convert ot MSB first.
  data = reverseBits(data, actualBits);
  // Decode the address & command from raw decode value.
  switch (actualBits) {
    case 12:  // 7 command bits, 5 address bits.
    case 15:  // 7 command bits, 8 address bits.
      results->command = data & 0x7F;  // Bits 0-6
      results->address = data >> 7;  // Bits 7-14
      break;
    case 20:  // 7 command bits, 5 address bits, 8 extended (command) bits.
      results->command = (data & 0x7F) + ((data >> 12) << 7);  // Bits 0-6,12-19
      results->address = (data >> 7) & 0x1F;  // Bits 7-11
      break;
    default:  // Shouldn't happen, but just in case.
      results->address = 0;
      results->command = 0;
  }
  return true;
}

// Decode the supplied Whynter message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of data bits to expect.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: BETA  Strict mode is ALPHA.
//
// Ref:
//   https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Whynter.cpp
bool ICACHE_FLASH_ATTR IRrecv::decodeWhynter(decode_results *results,
                                             uint16_t nbits, bool strict) {
  if (results->rawlen < 2 * nbits + 2 * HEADER + FOOTER)
     return false;  // We don't have enough entries to possibly match.

  // Compliance
  if (strict && nbits != WHYNTER_BITS)
    return false;  // Incorrect nr. of bits per spec.

  uint16_t offset = OFFSET_START;

  // Header
  // Sequence begins with a bit mark and a zero space
  if (!matchMark(results->rawbuf[offset++], WHYNTER_BIT_MARK))
    return false;
  if (!matchSpace(results->rawbuf[offset++], WHYNTER_ZERO_SPACE))
    return false;
  // header mark and space
  if (!matchMark(results->rawbuf[offset++], WHYNTER_HDR_MARK))
    return false;
  if (!matchSpace(results->rawbuf[offset++], WHYNTER_HDR_SPACE))
    return false;

  // Data
  uint64_t data = 0;
  for (uint16_t i = 0; i < nbits; i++, offset++) {
    if (!matchMark(results->rawbuf[offset++], WHYNTER_BIT_MARK))
      return false;
    if (matchSpace(results->rawbuf[offset], WHYNTER_ONE_SPACE))
      data = (data << 1) | 1;  // 1
    else if (matchSpace(results->rawbuf[offset], WHYNTER_ZERO_SPACE))
      data <<= 1;  // 0
    else
      return false;
  }

  // Footer
  if (!matchMark(results->rawbuf[offset], WHYNTER_BIT_MARK))
    return false;

  // Success
  results->decode_type = WHYNTER;
  results->bits = nbits;
  results->value = data;
  results->address = 0;
  results->command = 0;
  return true;
}

// Decode the supplied SANYO LC7461 message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of data bits to expect.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: ALPHA / untested.
//
// Notes:
//   Based on @marcosamarinho's work.
//   This protocol uses the NEC protocol timings. However, data is
//   formatted as : address(13 bits), !address, command (8 bits), !command.
//   According with LIRC, this protocol is used on Sanyo, Aiwa and Chinon
//   Information for this protocol is available at the Sanyo LC7461 datasheet.
// Ref:
//   http://slydiman.narod.ru/scr/kb/sanyo.htm
//   https://github.com/marcosamarinho/IRremoteESP8266/blob/master/ir_Sanyo.cpp
//   http://pdf.datasheetcatalog.com/datasheet/sanyo/LC7461.pdf
bool ICACHE_FLASH_ATTR IRrecv::decodeSanyoLC7461(decode_results *results,
                                                 uint16_t nbits, bool strict) {
  if (results->rawlen < 2 * nbits + HEADER + FOOTER)
    return false;  // Shorter than the shortest expected.
  if (strict && nbits != SANYO_LC7461_BITS)
    return false;  // Not strictly in spec.

  uint16_t offset = OFFSET_START;

  // Header
  if (!matchMark(results->rawbuf[offset++], SANYO_LC7461_HDR_MARK))
    return false;
  if (!matchSpace(results->rawbuf[offset++], SANYO_LC7461_HDR_SPACE))
    return false;

  // Data
  uint64_t data = 0;
  for (uint16_t i = 0; i < nbits; i++, offset++) {
    if (!matchMark(results->rawbuf[offset++], SANYO_LC7461_BIT_MARK))
      return false;
    if (matchSpace(results->rawbuf[offset], SANYO_LC7461_ONE_SPACE))
      data = (data << 1) | 1;  // 1
    else if (matchSpace(results->rawbuf[offset], SANYO_LC7461_ZERO_SPACE))
      data <<= 1;  // 0
    else
      return false;
  }

  // Footer
  if (!matchMark(results->rawbuf[offset], SANYO_LC7461_BIT_MARK))
    return false;

  // Bits 30 to 42+.
  uint16_t address = data >> (SANYO_LC7461_BITS - SANYO_LC7461_ADDRESS_BITS);
  // Bits 9 to 16.
  uint8_t command = (data >> SANYO_LC7461_COMMAND_BITS) &
                    SANYO_LC7461_COMMAND_MASK;

  if (strict) {  // Compliance
    uint16_t inverted_address = (data >> (SANYO_LC7461_COMMAND_BITS * 2)) &
                                SANYO_LC7461_ADDRESS_MASK;  // Bits 17 to 29.
    uint8_t inverted_command = data & SANYO_LC7461_COMMAND_MASK;  // Bits 1-8.
    if ((results->address ^ SANYO_LC7461_ADDRESS_MASK) != inverted_address)
      return false;  // Address integrity check failed.
    if ((results->command ^ SANYO_LC7461_COMMAND_MASK) != inverted_command)
      return false;  // Command integrity check failed.
  }

  // Success
  results->decode_type = SANYO_LC7461;
  results->bits = nbits;
  results->value = data;
  results->address = address;
  results->command = command;
  return true;
}

// Decode the supplied Sanyo SA 8650B message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of data bits to expect.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: Depricated.
//
// NOTE: This decoder looks like rubbish. Only keeping it for compatiblity
//       with the Arduino IRremote library. Seriously, don't trust it.
//       If someone has a device that this is supposed to be for, please log an
//       Issue on github with a rawData dump please. We should probably remove
//       it. We think this is a Sanyo decoder - serial = SA 8650B
// Ref:
//   https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Sanyo.cpp
bool ICACHE_FLASH_ATTR IRrecv::decodeSanyo(decode_results *results,
                                           uint16_t nbits, bool strict) {
  if (results->rawlen < 2 * nbits + HEADER)
    return false;  // Shorter than shortest possible.
  if (strict && nbits != SANYO_SA8650B_BITS)
    return false;  // Doesn't match the spec.

  uint16_t offset = 0;

  // TODO(crankyoldgit): This repeat code looks like garbage, it should never
  //   match or if it does, it won't be reliable. We should probably just
  //   remove it.
  if (results->rawbuf[offset++] < SANYO_SA8650B_DOUBLE_SPACE_USECS) {
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = SANYO;
    results->address = 0;
    results->command = 0;
    return true;
  }

  // Header
  if (!matchMark(results->rawbuf[offset++], SANYO_SA8650B_HDR_MARK))
    return false;
  // NOTE: These next two lines look very wrong. Treat as suspect.
  if (!matchMark(results->rawbuf[offset++], SANYO_SA8650B_HDR_MARK))
    return false;
  // Data
  uint64_t data = 0;
  while (offset + 1 < results->rawlen) {
    if (!matchSpace(results->rawbuf[offset], SANYO_SA8650B_HDR_SPACE))
      break;
    offset++;
    if (matchMark(results->rawbuf[offset], SANYO_SA8650B_ONE_MARK))
      data = (data << 1) | 1;  // 1
    else if (matchMark(results->rawbuf[offset], SANYO_SA8650B_ZERO_MARK))
      data <<= 1;  // 0
    else
      return false;
    offset++;
  }

  if (strict && SANYO_SA8650B_BITS > (offset - 1U) / 2U)
    return false;

  // Success
  results->bits = (offset - 1) / 2;
  results->decode_type = SANYO;
  results->value = data;
  results->address = 0;
  results->command = 0;
  return true;
}

// Decode the supplied Mitsubishi message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of data bits to expect.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: BETA / previously working.
//
// Notes:
//   This protocol appears to have no header.
//
// Ref:
//   GlobalCache's Control Tower's Mitsubishi TV data.
bool ICACHE_FLASH_ATTR IRrecv::decodeMitsubishi(decode_results *results,
                                                uint16_t nbits, bool strict) {
  if (results->rawlen < 2 * nbits + FOOTER)
    return false;  // Shorter than shortest possibly expected.
  if (strict && nbits != MITSUBISHI_BITS)
    return false;  // Request is out of spec.

  uint16_t offset = OFFSET_START;
  uint64_t data = 0;

  // No Header

  // Data
  uint16_t actualBits;
  for (actualBits = 0; offset < results->rawlen - 1; actualBits++, offset++) {
    if (!matchMark(results->rawbuf[offset++], MITSUBISHI_BIT_MARK))
      return false;
    if (matchSpace(results->rawbuf[offset], MITSUBISHI_ONE_SPACE))
      data = (data << 1) | 1;  // 1
    else if (matchSpace(results->rawbuf[offset], MITSUBISHI_ZERO_SPACE))
      data <<= 1;  // 0
    else
      break;
  }

  // Footer is matched by the last iteration of the data loop.

  // Compliance
  if (strict && actualBits != nbits)
    return false;  // Not as we expected.

  // Success
  results->decode_type = MITSUBISHI;
  results->bits = actualBits;
  results->value = data;
  results->address = 0;
  results->command = 0;
  return true;
}

// Gets one undecoded level at a time from the raw buffer.
// The RC5/6 decoding is easier if the data is broken into time intervals.
// E.g. if the buffer has MARK for 2 time intervals and SPACE for 1,
// successive calls to getRClevel will return MARK, MARK, SPACE.
// offset and used are updated to keep track of the current position.
// t1 is the time interval for a single bit in microseconds.
// Returns -1 for error (measured time interval is not a multiple of t1).
int16_t ICACHE_FLASH_ATTR IRrecv::getRClevel(decode_results *results,
                                             int *offset, int *used,
                                             uint16_t t1) {
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
  int offset = 1;  // Skip gap space
  uint32_t data = 0;
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
  int offset = 1;  // Skip first space
  // Initial mark
  if (!matchMark(results->rawbuf[offset], RC6_HDR_MARK)) {
    return false;
  }
  offset++;
  if (!matchSpace(results->rawbuf[offset], RC6_HDR_SPACE)) {
    return false;
  }
  offset++;
  int32_t data = 0;
  int used = 0;
  // Get start bit (1)
  if (getRClevel(results, &offset, &used, RC6_T1) != MARK) return false;
  if (getRClevel(results, &offset, &used, RC6_T1) != SPACE) return false;
  int nbits;
  for (nbits = 0; offset < results->rawlen; nbits++) {
    int levelA, levelB;  // Next two levels
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
    if (levelA == MARK && levelB == SPACE) {  // reversed compared to RC5
      // 1 bit
      data = (data << 1) | 1;
    } else if (levelA == SPACE && levelB == MARK) {
      // zero bit
      data <<= 1;
    } else {
      return false;  // Error
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
//
// Status:  ALPHA (untested and unconfirmed.)
// Ref:
//   http://www.sbprojects.com/knowledge/ir/rcmm.php
bool ICACHE_FLASH_ATTR IRrecv::decodeRCMM(decode_results *results) {
  uint32_t data = 0;
  uint16_t offset = OFFSET_START;

  int16_t bitSize = results->rawlen - 4;
  if (bitSize < 12 || bitSize > 32)
    return false;
  // Header decode
  if (!matchMark(results->rawbuf[offset++], RCMM_HDR_MARK))
    return false;
  if (!matchSpace(results->rawbuf[offset++], RCMM_HDR_SPACE))
    return false;
  // Data decode
  // RC-MM has two bits of data per mark/space pair.
  for (uint16_t i = 0; i < bitSize; i += 2, offset++) {
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
  }
  // Footer decode
  if (!matchMark(results->rawbuf[offset], RCMM_BIT_MARK))
    return false;

  // Success
  results->value = data;
  results->decode_type = RCMM;
  results->bits = bitSize;
  results->address = 0;
  results->command = 0;
  return true;
}

// Decode the supplied Sharp message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of data bits to expect. Typically SHARP_BITS.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: ALPHA / Untested.
//
// Note:
//   This procedure returns a value suitable for use in sendSharpRaw().
// TODO(crankyoldgit): Need to ensure capture of the inverted message as it can
//   be missed due to the interrupt timeout used to detect an end of message.
//   Several compliance checks are disabled until that is resolved.
// Ref:
//   http://www.sbprojects.com/knowledge/ir/sharp.php
//   http://www.mwftr.com/ucF08/LEC14%20PIC%20IR.pdf
//   http://www.hifi-remote.com/johnsfine/DecodeIR.html#Sharp
bool ICACHE_FLASH_ATTR IRrecv::decodeSharp(decode_results *results,
                                           uint16_t nbits,
                                           bool strict) {
  if (results->rawlen < 2 * nbits + FOOTER)
    return false;  // Not enough entries to be a Sharp message.
  // Compliance
  if (strict) {
    if (nbits != SHARP_BITS)
      return false;  // Request is out of spec.
    /* DISABLED - See TODO
    // An in spec message has the data sent normally, then inverted. So we
    // expect twice as many entries than to just get the results.
    if (results->rawlen < 2 * (2 * nbits + FOOTER))
      return false;
    */
  }

  uint64_t data = 0;
  uint16_t offset = OFFSET_START;

  // No header

  // Data
  for (uint16_t i = 0; i < nbits; i++, offset++) {
    if (!matchMark(results->rawbuf[offset++], SHARP_BIT_MARK))
      return false;
    if (matchSpace(results->rawbuf[offset], SHARP_ONE_SPACE))
      data = (data << 1) | 1;  // 1
    else if (matchSpace(results->rawbuf[offset], SHARP_ZERO_SPACE))
      data <<= 1;  // 0
    else
      return false;
  }

  // Footer
  if (!match(results->rawbuf[offset++], SHARP_BIT_MARK))
    return false;


  // Compliance
  if (strict) {
    // We expect the expansion bit to be set, and the check bit cleared
    // in a normal message.
    if ((data & 0b11) != 0b10)
      return false;
    /* DISABLED - See TODO
    // Grab the second copy of the data (i.e. inverted)
    // Header
    // i.e. The inter-data/command repeat gap.
    if (!matchSpace(results->rawbuf[offset++], SHARP_GAP))
      return false;

    // Data
    uint64_t second_data = 0;
    for (uint16_t i = 0; i < nbits; i++, offset++) {
      if (!matchMark(results->rawbuf[offset++], SHARP_BIT_MARK))
        return false;
      if (matchSpace(results->rawbuf[offset], SHARP_ONE_SPACE))
        second_data = (second_data << 1) | 1;  // 1
      else if (matchSpace(results->rawbuf[offset], SHARP_ZERO_SPACE))
        second_data <<= 1;  // 0
      else
        return false;
    }
    // Footer
    if (!match(results->rawbuf[offset++], SHARP_BIT_MARK))
      return false;

    // Check that second_data has been inverted correctly.
    if (data != (second_data ^ SHARP_TOGGLE_MASK))
      return false;
    */
  }

  // Success
  results->decode_type = SHARP;
  results->bits = nbits;
  results->value = data;
  // Address & command are actually transmitted in LSB first order.
  results->address = reverseBits(data, nbits) & SHARP_ADDRESS_MASK;
  results->sharpAddress = results->address;
  results->command = reverseBits((data >> 2) & SHARP_COMMAND_MASK,
                                 SHARP_COMMAND_BITS);
  return true;
}

// Decode the supplied Panasonic message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of data bits to expect.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: BETA  Strict mode is ALPHA / Untested.
// Note:
//   Panasonic 48-bit protocol is a modified version of Kaseikyo.
// Ref:
//   http://www.remotecentral.com/cgi-bin/mboard/rc-pronto/thread.cgi?26152
//   http://www.hifi-remote.com/wiki/index.php?title=Panasonic
bool ICACHE_FLASH_ATTR IRrecv::decodePanasonic(decode_results *results,
                                               uint16_t nbits,
                                               bool strict) {
  if (results->rawlen < 2 * nbits + HEADER + FOOTER)
    return false;  // Not enough entries to be a Panasonic message.
  if (strict && nbits != PANASONIC_BITS)
    return false;  // Request is out of spec.

  uint64_t data = 0;
  uint16_t offset = OFFSET_START;

  // Header
  if (!matchMark(results->rawbuf[offset++], PANASONIC_HDR_MARK))
    return false;
  if (!matchMark(results->rawbuf[offset++], PANASONIC_HDR_SPACE))
    return false;

  // Data
  for (uint16_t i = 0; i < nbits; i++, offset++) {
    if (!match(results->rawbuf[offset++], PANASONIC_BIT_MARK))
      return false;
    if (match(results->rawbuf[offset], PANASONIC_ONE_SPACE))
      data = (data << 1) | 1;  // 1
    else if (match(results->rawbuf[offset], PANASONIC_ZERO_SPACE))
      data <<= 1;  // 0
    else
      return false;
  }
  // Footer
  if (!match(results->rawbuf[offset], PANASONIC_BIT_MARK))
    return false;

  // Compliance
  uint32_t address = data >> 32;
  uint32_t command = data & 0xFFFFFFFF;
  if (strict) {
    if (address != PANASONIC_MANUFACTURER)  // Verify the Manufacturer code.
      return false;
    // Verify the checksum.
    uint8_t checksumOrig = data & 0xFF;
    uint8_t checksumCalc = (data >> 24) ^ ((data >> 16) & 0xFF) ^
                           ((data >> 8) & 0xFF);
    if (checksumOrig != checksumCalc)
      return false;
  }

  // Success
  results->value = data;
  results->panasonicAddress = (uint16_t) address;
  results->address = address;
  results->command = command;
  results->decode_type = PANASONIC;
  results->bits = nbits;
  return true;
}

// Decode the supplied LG message.
// LG protocol has a repeat code which is 4 items long.
// Even though the protocol has 28 bits of data, only 16 bits are distinct.
// In transmission order, the 28 bits are constructed as follows:
//   8 bits of address + 16 bits of command + 4 bits of checksum.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of bits to expect in the data portion. Typically LG_BITS.
//   strict:  Flag to indicate if we strictly adhere to the specification.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: BETA  Strict mode is ALPHA / Untested.
//
// Note:
//   LG 32bit protocol appears near identical to the Samsung protocol.
//   They differ on their compliance criteria and how they repeat.
// Ref:
//   https://funembedded.wordpress.com/2014/11/08/ir-remote-control-for-lg-conditioner-using-stm32f302-mcu-on-mbed-platform/
bool ICACHE_FLASH_ATTR IRrecv::decodeLG(decode_results *results,
                                        uint16_t nbits, bool strict) {
  if (results->rawlen < 2 * nbits + 4 && results->rawlen != 4)
    return false;  // Can't possibly be a valid LG message.
  if (strict && nbits != LG_BITS)
    return false;  // Doesn't comply with expected LG protocol.

  uint64_t data = 0;
  uint16_t offset = OFFSET_START;

  // Header
  if (!matchMark(results->rawbuf[offset++], LG_HDR_MARK))
    return false;
  if (!matchSpace(results->rawbuf[offset++], LG_HDR_SPACE))
    return false;
  // Data
  for (uint16_t i = 0; i < nbits; i++, offset++) {
    if (!matchMark(results->rawbuf[offset++], LG_BIT_MARK))
      return false;
    if (matchSpace(results->rawbuf[offset], LG_ONE_SPACE))
      data = (data << 1) | 1;  // 1
    else if (matchSpace(results->rawbuf[offset], LG_ZERO_SPACE))
      data <<= 1;  // 0
    else
      return false;
  }
  // Footer
  if (!matchMark(results->rawbuf[offset], LG_BIT_MARK))
    return false;

  // Compliance

  uint8_t address = (data >> 16) & 0xFF;  // Address is the first 8 bits sent.
  uint16_t command = (data >> 4) & 0xFFFF;  // Command is the next 16 bits sent.
  // The last 4 bits sent are the expected checksum.
  if (strict && (data & 0xF) != calcLGChecksum(command))
    return false;

  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = LG;
  results->address = address;
  results->command = command;
  return true;
}

// Decode the supplied JVC message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of bits of data to expect. Typically JVC_BITS.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: STABLE
//
// Note:
//   JVC repeat codes don't have a header.
// Ref:
//   http://www.sbprojects.com/knowledge/ir/jvc.php
// TODO(crankyoldgit):
//   Report that it is a repeat code and still record the value.
bool ICACHE_FLASH_ATTR IRrecv::decodeJVC(decode_results *results,
                                         uint16_t nbits,
                                         bool strict) {
  if (strict && nbits != JVC_BITS)
    return false;  // Must be called with the correct nr. of bits.
  if (results->rawlen < 2 * nbits + 2)
    return false;  // Can't possibly be a valid JVC message.

  uint64_t data = 0;
  uint16_t offset = OFFSET_START;
  bool isRepeat = true;

  // Header
  // (Optional as repeat codes don't have the header)
  if (matchMark(results->rawbuf[offset], JVC_HDR_MARK)) {
    isRepeat = false;
    offset++;
    if (results->rawlen < 2 * nbits + 4)
      return false;  // Can't possibly be a valid JVC message with a header.
    if (!matchSpace(results->rawbuf[offset++], JVC_HDR_SPACE))
      return false;
  }

  // Data
  for (uint16_t i = 0; i < nbits; i++) {
    if (!matchMark(results->rawbuf[offset++], JVC_BIT_MARK))
      return false;
    if (matchSpace(results->rawbuf[offset], JVC_ONE_SPACE))
      data = (data << 1) | 1;  // 1
    else if (matchSpace(results->rawbuf[offset], JVC_ZERO_SPACE))
      data <<= 1;  // 0
    else
      return false;
    offset++;
  }

  // Footer
  if (!matchMark(results->rawbuf[offset], JVC_BIT_MARK))
    return false;

  // Success
  results->decode_type = JVC;
  results->bits = nbits;
  // command & address are transmitted LSB first, so we need to reverse them.
  results->command = reverseBits(data >> 8, 8);  // The first 8 bits sent.
  results->address = reverseBits(data & 0xFF, 8);  // The last 8 bits sent.
  if (isRepeat)
    results->value = REPEAT;
  else
    results->value = data;
  return true;
}

// Decode the supplied Samsung message.
// Samsung messages whilst 32 bits in size, only contain 16 bits of distinct
// data. e.g. In transmition order:
//   customer_byte + customer_byte(same) + address_byte + invert(address_byte)
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of bits to expect in the data portion. Typically SAMSUNG_BITS.
//   strict:  Flag to indicate if we strictly adhere to the specification.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: STABLE
//
// Note:
//   LG 32bit protocol appears near identical to the Samsung protocol.
//   They differ on their compliance criteria and how they repeat.
// Ref:
//  http://elektrolab.wz.cz/katalog/samsung_protocol.pdf
bool ICACHE_FLASH_ATTR IRrecv::decodeSAMSUNG(decode_results *results,
                                             uint16_t nbits,
                                             bool strict) {
  if (results->rawlen < 2 * nbits + 4)
    return false;  // Can't possibly be a valid Samsung message.
  if (strict && nbits != SAMSUNG_BITS)
    return false;  // We expect Samsung to be 32 bits of message.

  uint64_t data = 0;
  uint16_t offset = OFFSET_START;

  // Header
  if (!matchMark(results->rawbuf[offset++], SAMSUNG_HDR_MARK))
    return false;
  if (!matchSpace(results->rawbuf[offset++], SAMSUNG_HDR_SPACE))
    return false;
  // Data
  for (uint16_t i = 0; i < nbits; i++, offset++) {
    if (!matchMark(results->rawbuf[offset++], SAMSUNG_BIT_MARK))
      return false;
    if (matchSpace(results->rawbuf[offset], SAMSUNG_ONE_SPACE))
      data = (data << 1) | 1;  // 1
    else if (matchSpace(results->rawbuf[offset], SAMSUNG_ZERO_SPACE))
      data <<= 1;  // 0
    else
      return false;
  }
  // Footer
  if (!matchMark(results->rawbuf[offset++], SAMSUNG_BIT_MARK))
    return false;

  // Compliance

  // According to the spec, the customer (address) code is the first 8
  // transmitted bits. It's then repeated. Check for that.
  uint8_t address = data >> 24;
  if (strict && address != ((data >> 16) & 0xFF))
    return false;
  // Spec says the command code is the 3rd block of transmitted 8-bits,
  // followed by the inverted command code.
  uint8_t command = (data & 0xFF00) >> 8;
  if (strict && command != ((data & 0xFF) ^ 0xFF))
    return false;

  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = SAMSUNG;
  // command & address need to be reversed as they are transmitted LSB first,
  results->command = reverseBits(command, sizeof(command));
  results->address = reverseBits(address, sizeof(address));
  return true;
}

/* TODO: NOT WORKING. This needs to be finished. Commented out till then.
// Decode the supplied Daikin A/C message. (NOT WORKING - DO NOT USE)
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of bits to expect in the data portion. Typically SAMSUNG_BITS.
//   strict:  Flag to indicate if we strictly adhere to the specification.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: UNFINISHED / Completely not working, not even vaguely.
//
// Ref:
//   https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/tree/master/IRremote
bool ICACHE_FLASH_ATTR IRrecv::decodeDaikin(decode_results *results,
                                            uint16_t nbits,
                                            bool strict) {
  if (results->rawlen < 2 * nbits + HEADER + FOOTER)
    return false;

  // Compliance
  if (strict && nbits != DAIKIN_BITS)
    return false;

  uint32_t data = 0;
  uint16_t offset = OFFSET_START;

  // Header
  if (!matchMark(results->rawbuf[offset++], DAIKIN_HDR_MARK))
      return false;
  if (!matchSpace(results->rawbuf[offset++], DAIKIN_HDR_SPACE))
      return false;

  // Data (#1)
  for (uint8_t i = 0; i < sizeof(data) * 8; i++, offset++) {
    if (!matchMark(results->rawbuf[offset++], DAIKIN_ONE_MARK))
      return false;
    if (matchSpace(results->rawbuf[offset], DAIKIN_ONE_SPACE))
      data = (data << 1) | 1;  // 1
    else if (matchSpace(results->rawbuf[offset], DAIKIN_ZERO_SPACE))
      data <<= 1;  // 0
    else
      return false;
  }

  uint32_t number = data; // some number...
  uint32_t reversed = reverseBits(number, sizeof(number) * 8)

#ifdef DEBUG
  Serial.print ("Code ");
  Serial.println (reversed,  HEX);
#endif

  // Data (#2)
  for (uint8_t i = 0; i < sizeof(data) * 8; i++, offset++) {
    if (!matchMark(results->rawbuf[offset++], DAIKIN_ONE_MARK))
      return false;
    if (matchSpace(results->rawbuf[offset], DAIKIN_ONE_SPACE))
      data = (data << 1) | 1;  // 1
    else if (matchSpace(results->rawbuf[offset], DAIKIN_ZERO_SPACE))
      data <<= 1;  // 0
    else
      return false;
  }

  number = data ; // some number...
  reversed = reverseBits(number, sizeof(number) * 8)

#ifdef DEBUG
  //Serial.print ("Code2 ");
  //Serial.println (reversed,  HEX);
#endif

  if (!matchSpace(results->rawbuf[offset++], DAIKIN_GAP))
#ifdef DEBUG
    //Serial.println ("no gap");
#endif
	  return false;

  // Success
  results->bits = DAIKIN_BITS;
  results->value = reversed;
  results->decode_type = DAIKIN;
  results->address = 0;
  results->command = 0;
  return true;
}
*/

// Decode a Denon message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Expected nr. of data bits. (Typically DENON_BITS)
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: STABLE
//
//
// Ref:
//   https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Denon.cpp
bool ICACHE_FLASH_ATTR IRrecv::decodeDenon(decode_results *results,
                                           uint16_t nbits,
                                           bool strict) {
  // Check we have enough data
  if (results->rawlen < 2 * nbits + HEADER + FOOTER)
    return false;
  if (strict && nbits != DENON_BITS)
    return false;

  uint64_t data = 0;
  uint16_t offset = OFFSET_START;

  // Header
  if (!matchMark(results->rawbuf[offset++], DENON_HDR_MARK))
    return false;
  if (!matchSpace(results->rawbuf[offset++], DENON_HDR_SPACE))
    return false;
  // Data
  for (uint16_t i = 0; i < nbits; i++, offset++) {
    if (!matchMark(results->rawbuf[offset++], DENON_BIT_MARK))
      return false;
    if (matchSpace(results->rawbuf[offset], DENON_ONE_SPACE))
      data = (data << 1) | 1;  // 1
    else if (matchSpace(results->rawbuf[offset], DENON_ZERO_SPACE))
      data = (data << 1);  // 0
    else
      return false;
  }
  // Footer
  if (!matchMark(results->rawbuf[offset++], DENON_BIT_MARK))
    return false;

  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = DENON;
  results->address = 0;
  results->command = 0;
  return true;
}

// Decode the supplied DISH NETWORK message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of bits to expect in the data portion. Typically DISH_BITS.
//   strict:  Flag to indicate if we strictly adhere to the specification.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status:  ALPHA (untested and unconfirmed.)
//
// Note:
//   Dishplayer is a different protocol.
//   Typically a DISH device needs to get a command a total of at least 4
//   times to accept it.
// Ref:
//   http://www.hifi-remote.com/wiki/index.php?title=Dish
//   http://lirc.sourceforge.net/remotes/echostar/301_501_3100_5100_58xx_59xx
//   https://github.com/marcosamarinho/IRremoteESP8266/blob/master/ir_Dish.cpp
bool IRrecv::decodeDISH(decode_results *results, uint16_t nbits, bool strict) {
  if (results->rawlen < 2 * nbits + HEADER + FOOTER)
    return false;  // Not enough entries to be valid.
  if (strict && nbits != DISH_BITS)
    return false;  // Not strictly compliant.

  uint64_t data = 0;
  uint16_t offset = OFFSET_START;

  // Header
  if (!matchMark(results->rawbuf[offset++], DISH_HDR_MARK))
    return false;
  if (!matchSpace(results->rawbuf[offset++], DISH_HDR_SPACE))
    return false;

  // Data
  for (uint16_t i = 0; i < nbits; i++, offset++) {
    if (!matchMark(results->rawbuf[offset++], DISH_BIT_MARK))
      return false;
    if (matchSpace(results->rawbuf[offset], DISH_ONE_SPACE))
      data = (data << 1) | 1;  // 1
    else if (matchSpace(results->rawbuf[offset], DISH_ZERO_SPACE))
      data = data << 1;  // 0
    else
      return false;
  }

  // Footer
  if (!matchMark(results->rawbuf[offset++], DISH_HDR_MARK))
    return false;
  // The DISH protocol calls for a repeated message, so strictly speaking
  // there should be a code following this. Only require it if we are set to
  // strict matching.
  if (strict && !matchSpace(results->rawbuf[offset], DISH_RPT_SPACE))
    return false;

  // Success
  results->decode_type = DISH;
  results->bits = nbits;
  results->value = data;
  results->address = 0;
  results->command = 0;
  return true;
}

/* -----------------------------------------------------------------------
 * hashdecode - decode an arbitrary IR code.
 * Instead of decoding using a standard encoding scheme
 * (e.g. Sony, NEC, RC5), the code is hashed to a 32-bit value.
 *
 * The algorithm: look at the sequence of MARK signals, and see if each one
 * is shorter (0), the same length (1), or longer (2) than the previous.
 * Do the same with the SPACE signals.  Hash the resulting sequence of 0's,
 * 1's, and 2's to a 32-bit value.  This will give a unique value for each
 * different code (probably), for most code systems.
 *
 * http://arcfn.com/2010/01/using-arbitrary-remotes-with-arduino.html
 */

// Compare two tick values, returning 0 if newval is shorter,
// 1 if newval is equal, and 2 if newval is longer
// Use a tolerance of 20%
int16_t ICACHE_FLASH_ATTR IRrecv::compare(uint16_t oldval, uint16_t newval) {
  if (newval < oldval * 0.8)
    return 0;
  else if (oldval < newval * 0.8)
    return 2;
  else
    return 1;
}

// Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
#define FNV_PRIME_32 16777619UL
#define FNV_BASIS_32 2166136261UL

/* Converts the raw code values into a 32-bit hash code.
 * Hopefully this code is unique for each button.
 * This isn't a "real" decoding, just an arbitrary value.
 */
bool ICACHE_FLASH_ATTR IRrecv::decodeHash(decode_results *results) {
  // Require at least 6 samples to prevent triggering on noise
  if (results->rawlen < 6)
    return false;
  int32_t hash = FNV_BASIS_32;
  // 'rawlen - 2' to avoid the look ahead from going out of bounds.
  // Should probably be -3 to avoid comparing the trailing space entry,
  // however it is left this way for compatiblity with previously captured
  // values.
  for (uint16_t i = 1; i < results->rawlen - 2; i++) {
    int16_t value = compare(results->rawbuf[i], results->rawbuf[i + 2]);
    // Add value into the hash
    hash = (hash * FNV_PRIME_32) ^ value;
  }
  results->value = hash & 0xFFFFFFFF;
  results->bits = results->rawlen / 2;
  results->address = 0;
  results->command = 0;
  results->decode_type = UNKNOWN;
  return true;
}

// ---------------------------------------------------------------
