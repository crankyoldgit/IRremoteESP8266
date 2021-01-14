// Copyright 2021 Victor Mukayev (vitos1k)
/// @file
/// @brief Support for the MilesTag2 IR protocol for LaserTag gaming
/// @see http://hosting.cmalton.me.uk/chrism/lasertag/MT2Proto.pdf

// Supports:
//   Brand: Milestag2,  Model: Various

// TODO(vitos1k): This implementation would support only
// short SHOT packets(14bits) and MSGs = 24bits. Support
// for long MSGs > 24bits is TODO

#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

// Constants
const uint16_t kMilesShotMask = 1 << (kMilesTag2ShotBits - 1); // Shot packets
                                                               // have this bit
                                                               // as `0`
const uint32_t kMilesMsgMask = 1 << (kMilesTag2MsgBits - 1);  // Msg packets
                                                              // have this bit
                                                              // as `1`
const uint8_t  kMilesMsgTerminator = 0xE8;
const uint16_t kMilesHdrMark = 2400;  // Header
const uint16_t kMilesSpace = 600;  // Space
const uint16_t kMilesOneMark = 1200;  //One Mark
const uint16_t kMilesZeroMark = 600;  //Zero Mark
const uint16_t kMilesRptLength = 32000;
const uint16_t kMilesMinGap = 32000;
const uint16_t kMilesStdFreq = 38000;  // kHz
const uint16_t kMilesStdDuty = 25;

#if SEND_MILESTAG2
/// Send a MilesTag2 formatted Shot packet.
/// Status: NEEDS TESTING
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void IRsend::sendMilestag2(const uint64_t data, const uint16_t nbits,
                      const uint16_t repeat) {
  sendGeneric(
    kMilesHdrMark, kMilesSpace,  // Header
    kMilesOneMark, kMilesSpace,  // 1 bit
    kMilesZeroMark, kMilesSpace,  // 0 bit
    0,  // No footer mark
    kMilesRptLength, data, nbits, kMilesStdFreq, true,  // MSB First
    repeat, kMilesStdDuty);
}
#endif  // SEND_MILESTAG2

#if DECODE_MILESTAG2
/// Decode the supplied MilesTag2 message.
/// Status: NEEDS TESTING
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return True if it can decode it, false if it can't.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/706
bool IRrecv::decodeMiles(decode_results *results, uint16_t offset,
                        const uint16_t nbits, const bool strict) {
  // Compliance
  if (strict) {
    switch (nbits) {  // Check we've been called with a correct bit size.
      case kMilesTag2ShotBits:
      case kMilesTag2MsgBits:
        break;
      default:
        DPRINT("incorrect nbits:");
        DPRINTLN(nbits);
        return false;  // The request doesn't strictly match the protocol defn.
    }
  }
  uint64_t data = 0;
  if (!matchGeneric(results->rawbuf + offset, &data,
                  results->rawlen - offset, nbits,
                  kMilesHdrMark, kMilesSpace,
                  kMilesOneMark, kMilesSpace,
                  kMilesZeroMark, kMilesSpace,
                  0, kMilesRptLength, true)) return false;
  results->command = 0;
  results->address = 0;

  // Compliance
  if (strict) {
    switch (nbits) {
      case kMilesTag2ShotBits:
        // Is it a valid shot packet?
        if (data & kMilesShotMask) return false;
        results->command = data & 0x3F;  // Team & Damage
        results->address = data >> 6;  // Player ID.
        break;
      case kMilesTag2MsgBits:
        // Is it a valid msg packet? i.e. Msg bit set & Terminator present.
        if (!(data & kMilesMsgMask) || ((data & 0xFF) != kMilesMsgTerminator))
          return false;
        results->command = (data >> 8) & 0xFF;  // Message data
        results->address = (data >> 16) & 0x7F;  // Message ID
        break;
    }
  }
  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = decode_type_t::MILESTAG2;
  return true;
}
#endif  // DECODE_MILESTAG2
