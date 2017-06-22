// Copyright 2015 Kristian Lauszus
// Copyright 2017 David Conran

#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRtimer.h"

//       PPPP    AAA   N   N   AAA    SSSS   OOO   N   N  IIIII   CCCC
//       P   P  A   A  NN  N  A   A  S      O   O  NN  N    I    C
//       PPPP   AAAAA  N N N  AAAAA   SSS   O   O  N N N    I    C
//       P      A   A  N  NN  A   A      S  O   O  N  NN    I    C
//       P      A   A  N   N  A   A  SSSS    OOO   N   N  IIIII   CCCC

// Panasonic protocol originally added by Kristian Lauszus from:
//   https://github.com/z3t0/Arduino-IRremote
// (Thanks to zenwheel and other people at the original blog post)

// Constants
// Ref:
//   http://www.remotecentral.com/cgi-bin/mboard/rc-pronto/thread.cgi?26152
#define PANASONIC_HDR_MARK             3456U
#define PANASONIC_HDR_SPACE            1728U
#define PANASONIC_BIT_MARK              432U
#define PANASONIC_ONE_SPACE            1296U
#define PANASONIC_ZERO_SPACE            432U
#define PANASONIC_MIN_COMMAND_LENGTH 130000UL
#define PANASONIC_END_GAP              5000U  // See issue #245
#define PANASONIC_MIN_GAP ((uint32_t)(PANASONIC_MIN_COMMAND_LENGTH - \
    (PANASONIC_HDR_MARK + PANASONIC_HDR_SPACE + \
     PANASONIC_BITS * (PANASONIC_BIT_MARK + PANASONIC_ONE_SPACE) + \
     PANASONIC_BIT_MARK)))

#if (SEND_PANASONIC || SEND_DENON)
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
void IRsend::sendPanasonic64(uint64_t data, uint16_t nbits, uint16_t repeat) {
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
// Status: STABLE.
//
// Note:
//   This protocol is a modified version of Kaseikyo.
void IRsend::sendPanasonic(uint16_t address, uint32_t data, uint16_t nbits,
                           uint16_t repeat) {
  sendPanasonic64(((uint64_t) address << 32) | (uint64_t) data, nbits, repeat);
}

// Calculate the raw Panasonic data based on device, subdevice, & function.
//
// Args:
//   manufacturer: A 16-bit manufacturer code. e.g. 0x4004 is Panasonic.
//   device:       An 8-bit code.
//   subdevice:    An 8-bit code.
//   function:     An 8-bit code.
// Returns:
//   A raw uint64_t Panasonic message.
//
// Status: BETA / Should be working..
//
// Note:
//   Panasonic 48-bit protocol is a modified version of Kaseikyo.
// Ref:
//   http://www.remotecentral.com/cgi-bin/mboard/rc-pronto/thread.cgi?2615
uint64_t IRsend::encodePanasonic(uint16_t manufacturer,
                                 uint8_t device,
                                 uint8_t subdevice,
                                 uint8_t function) {
  uint8_t checksum = device ^ subdevice ^ function;
  return (((uint64_t) manufacturer << 32) |
          ((uint64_t) device << 24) |
          ((uint64_t) subdevice << 16) |
          ((uint64_t) function << 8) |
          checksum);
}
#endif  // (SEND_PANASONIC || SEND_DENON)

#if (DECODE_PANASONIC || DECODE_DENON)
// Decode the supplied Panasonic message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of data bits to expect.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: BETA / Should be working.
// Note:
//   Panasonic 48-bit protocol is a modified version of Kaseikyo.
// Ref:
//   http://www.remotecentral.com/cgi-bin/mboard/rc-pronto/thread.cgi?26152
//   http://www.hifi-remote.com/wiki/index.php?title=Panasonic
bool IRrecv::decodePanasonic(decode_results *results, uint16_t nbits,
                             bool strict, uint32_t manufacturer) {
  if (results->rawlen < 2 * nbits + HEADER + FOOTER - 1)
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
  if (!match(results->rawbuf[offset++], PANASONIC_BIT_MARK))
    return false;
  if (offset < results->rawlen &&
      !matchAtLeast(results->rawbuf[offset], PANASONIC_END_GAP))
    return false;

  // Compliance
  uint32_t address = data >> 32;
  uint32_t command = data & 0xFFFFFFFF;
  if (strict) {
    if (address != manufacturer)  // Verify the Manufacturer code.
      return false;
    // Verify the checksum.
    uint8_t checksumOrig = data & 0xFF;
    uint8_t checksumCalc = ((data >> 24) ^ (data >> 16) ^ (data >> 8)) & 0xFF;
    if (checksumOrig != checksumCalc)
      return false;
  }

  // Success
  results->value = data;
  results->address = address;
  results->command = command;
  results->decode_type = PANASONIC;
  results->bits = nbits;
  return true;
}
#endif  // (DECODE_PANASONIC || DECODE_DENON)
