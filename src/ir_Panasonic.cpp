// Copyright 2015 Kristian Lauszus
// Copyright 2017 David Conran

#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

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

const uint16_t kPanasonicTick = 432;
const uint16_t kPanasonicHdrMarkTicks = 8;
const uint16_t kPanasonicHdrMark = kPanasonicHdrMarkTicks * kPanasonicTick;
const uint16_t kPanasonicHdrSpaceTicks = 4;
const uint16_t kPanasonicHdrSpace = kPanasonicHdrSpaceTicks * kPanasonicTick;
const uint16_t kPanasonicBitMarkTicks = 1;
const uint16_t kPanasonicBitMark = kPanasonicBitMarkTicks * kPanasonicTick;
const uint16_t kPanasonicOneSpaceTicks = 3;
const uint16_t kPanasonicOneSpace = kPanasonicOneSpaceTicks * kPanasonicTick;
const uint16_t kPanasonicZeroSpaceTicks = 1;
const uint16_t kPanasonicZeroSpace = kPanasonicZeroSpaceTicks * kPanasonicTick;
const uint16_t kPanasonicMinCommandLengthTicks = 378;
const uint32_t kPanasonicMinCommandLength =
    kPanasonicMinCommandLengthTicks * kPanasonicTick;
const uint16_t kPanasonicEndGap = 5000;  // See issue #245
const uint16_t kPanasonicMinGapTicks = kPanasonicMinCommandLengthTicks -
    (kPanasonicHdrMarkTicks + kPanasonicHdrSpaceTicks +
     kPanasonicBits * (kPanasonicBitMarkTicks + kPanasonicOneSpaceTicks) +
     kPanasonicBitMarkTicks);
const uint32_t kPanasonicMinGap = kPanasonicMinGapTicks * kPanasonicTick;

#if (SEND_PANASONIC || SEND_DENON)
// Send a Panasonic formatted message.
//
// Args:
//   data:   The message to be sent.
//   nbits:  The number of bits of the message to be sent. (kPanasonicBits).
//   repeat: The number of times the command is to be repeated.
//
// Status: BETA / Should be working.
//
// Note:
//   This protocol is a modified version of Kaseikyo.
void IRsend::sendPanasonic64(uint64_t data, uint16_t nbits, uint16_t repeat) {
  sendGeneric(kPanasonicHdrMark, kPanasonicHdrSpace,
              kPanasonicBitMark, kPanasonicOneSpace,
              kPanasonicBitMark, kPanasonicZeroSpace,
              kPanasonicBitMark,
              kPanasonicMinGap, kPanasonicMinCommandLength,
              data, nbits, 36700U, true, repeat, 50);
}

// Send a Panasonic formatted message.
//
// Args:
//   address: The manufacturer code.
//   data:    The data portion to be sent.
//   nbits:   The number of bits of the message to be sent. (kPanasonicBits).
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
  if (results->rawlen < 2 * nbits + kHeader + kFooter - 1)
    return false;  // Not enough entries to be a Panasonic message.
  if (strict && nbits != kPanasonicBits)
    return false;  // Request is out of spec.

  uint64_t data = 0;
  uint16_t offset = kStartOffset;

  // Header
  if (!matchMark(results->rawbuf[offset], kPanasonicHdrMark)) return false;
  // Calculate how long the common tick time is based on the header mark.
  uint32_t m_tick = results->rawbuf[offset++] * kRawTick /
      kPanasonicHdrMarkTicks;
  if (!matchSpace(results->rawbuf[offset], kPanasonicHdrSpace)) return false;
  // Calculate how long the common tick time is based on the header space.
  uint32_t s_tick = results->rawbuf[offset++] * kRawTick /
      kPanasonicHdrSpaceTicks;

  // Data
  match_result_t data_result = matchData(&(results->rawbuf[offset]), nbits,
                                         kPanasonicBitMarkTicks * m_tick,
                                         kPanasonicOneSpaceTicks * s_tick,
                                         kPanasonicBitMarkTicks * m_tick,
                                         kPanasonicZeroSpaceTicks * s_tick);
  if (data_result.success == false) return false;
  data = data_result.data;
  offset += data_result.used;

  // Footer
  if (!match(results->rawbuf[offset++], kPanasonicBitMarkTicks * m_tick))
    return false;
  if (offset < results->rawlen &&
      !matchAtLeast(results->rawbuf[offset], kPanasonicEndGap))
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
