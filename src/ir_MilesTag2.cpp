// Copyright 2020 Victor Mukayev (vitos1k)
/// @file
/// @brief Support for the MilesTag2 IR protocol for LaserTag gaming
/// @see http://hosting.cmalton.me.uk/chrism/lasertag/MT2Proto.pdf

// Supports:
//   Brand: Theoretically MilesTag2 supported hardware

//TODO: This implementation would support only short SHOT packets(14bits) and MSGs = 24bits. Support for long MSGs > 24bits is TODO

#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

// Constants
// Constants are similar to Sony SIRC protocol, bassically they are very similar, just nbits are varying

const uint16_t kMilesTick = 200;
const uint16_t kMilesHdrMarkTicks = 12;
const uint16_t kMilesHdrMark = kMilesHdrMarkTicks * kMilesTick;
const uint16_t kMilesSpaceTicks = 3;
const uint16_t kMilesSpace = kMilesSpaceTicks * kMilesTick;
const uint16_t kMilesOneMarkTicks = 6;
const uint16_t kMilesOneMark = kMilesOneMarkTicks * kMilesTick;
const uint16_t kMilesZeroMarkTicks = 3;
const uint16_t kMilesZeroMark = kMilesZeroMarkTicks * kMilesTick;
const uint16_t kMilesRptLengthTicks = 225;
const uint16_t kMilesRptLength = kMilesRptLengthTicks * kMilesTick;
const uint16_t kMilesMinGapTicks = 50;
const uint16_t kMilesMinGap = kMilesMinGapTicks * kMilesTick;
const uint16_t kMilesStdFreq = 38000;  // kHz



#if SEND_MILESTAG2
/// Send a MilesTag2 formatted message.
/// Status: NEEDS TESTING
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void IRsend::sendMilesShot(const uint64_t data, const uint16_t nbits,
                      const uint16_t repeat)
{
    _sendMiles(data, nbits,repeat);
}

void IRsend::sendMilesMsg(const uint64_t data, const uint16_t nbits,
                      const uint16_t repeat)
{
    _sendMiles(data, nbits,repeat);
}

void IRsend::_sendMiles(const uint64_t data, const uint16_t nbits,
                      const uint16_t repeat) {
  sendGeneric(kMilesHdrMark, kMilesSpace,
              kMilesOneMark, kMilesSpace,
              kMilesZeroMark, kMilesSpace,
              0, kMilesMinGap,
              data, nbits, kMilesStdFreq, true, repeat, kDutyDefault);
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
      case 14:
      case 24:      
        break;
      default:
        return false;  // The request doesn't strictly match the protocol defn.
    }
  }
  uint64_t data = 0;

  // Match Header + Data
  if (!matchGeneric(results->rawbuf + offset, &data,
                    results->rawlen - offset, nbits,
                    kMilesHdrMark, kMilesSpace,
                    kMilesOneMark, kMilesSpace,
                    kMilesZeroMark, kMilesSpace,
                    0, 0, true)) return false;
  // Success

  results->bits = nbits;
  results->value = data;
  switch (nbits)
  {
    case 14:
      results->decode_type = decode_type_t::MILESTAG2SHOT;
      break;
    case 24:
      results->decode_type = decode_type_t::MILESTAG2MSG;
      break;
    default:
      return false;
  }  
  results->command = 0;
  results->address = 0;
  return true;
}
#endif  // DECODE_MILESTAG2
