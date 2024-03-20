// Copyright 2016 sillyfrog
// Copyright 2017 sillyfrog, crankyoldgit
// Copyright 2018-2022 crankyoldgit
// Copyright 2019 pasna (IRDaikin160 class / Daikin176 class)

/// @file
/// @brief Support for 312-bit Daikin A/C protocols.
/// @see Daikin http://harizanov.com/2012/02/control-daikin-air-conditioner-over-the-internet/
/// @see Daikin https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/tree/master/IRremote
/// @see Daikin http://rdlab.cdmt.vn/project-2013/daikin-ir-protocol
/// @see Daikin https://github.com/blafois/Daikin-IR-Reverse
/// @see Daikin128 https://github.com/crankyoldgit/IRremoteESP8266/issues/827
/// @see Daikin152 https://github.com/crankyoldgit/IRremoteESP8266/issues/873
/// @see Daikin152 https://github.com/ToniA/arduino-heatpumpir/blob/master/DaikinHeatpumpARC480A14IR.cpp
/// @see Daikin152 https://github.com/ToniA/arduino-heatpumpir/blob/master/DaikinHeatpumpARC480A14IR.h
/// @see Daikin160 https://github.com/crankyoldgit/IRremoteESP8266/issues/731
/// @see Daikin2 https://docs.google.com/spreadsheets/d/1f8EGfIbBUo2B-CzUFdrgKQprWakoYNKM80IKZN4KXQE/edit#gid=236366525&range=B25:D32
/// @see Daikin2 https://github.com/crankyoldgit/IRremoteESP8266/issues/582
/// @see Daikin2 https://github.com/crankyoldgit/IRremoteESP8266/issues/1535
/// @see Daikin2 https://www.daikin.co.nz/sites/default/files/daikin-split-system-US7-FTXZ25-50NV1B.pdf
/// @see Daikin216 https://github.com/crankyoldgit/IRremoteESP8266/issues/689
/// @see Daikin216 https://github.com/danny-source/Arduino_DY_IRDaikin
/// @see Daikin64 https://github.com/crankyoldgit/IRremoteESP8266/issues/1064
/// @see Daikin200 https://github.com/crankyoldgit/IRremoteESP8266/issues/1802

// Supports:
//   Brand: Daikin,  Model: FTXM20R5V1B A/C (DAIKIN312)
//   Brand: Daikin,  Model: ARC466A67 remote (DAIKIN312)

#include "ir_Daikin.h"
#include <algorithm>
#include <cstring>
#ifndef ARDUINO
#include <string>
#endif
#include "IRrecv.h"
#include "IRremoteESP8266.h"
#include "IRsend.h"
#ifdef UNIT_TEST
#include "IRsend_test.h"
#endif
#include "IRtext.h"
#include "IRutils.h"

using irutils::addBoolToString;
using irutils::addDayToString;
using irutils::addIntToString;
using irutils::addLabeledString;
using irutils::addModeToString;
using irutils::addSwingHToString;
using irutils::addSwingVToString;
using irutils::addTempToString;
using irutils::addFanToString;
using irutils::bcdToUint8;
using irutils::minsToString;
using irutils::setBit;
using irutils::setBits;
using irutils::sumNibbles;
using irutils::uint8ToBcd;

#if SEND_DAIKIN312
/// Send a Daikin312 (312-bit / 39 byte) A/C formatted message.
/// Status: BETA / Untested on a real device.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1829
void IRsend::sendDaikin312(const unsigned char data[], const uint16_t nbytes,
                           const uint16_t repeat) {
  if (nbytes < kDaikin312Section1Length)
    return;  // Not enough bytes to send a partial message.

  for (uint16_t r = 0; r <= repeat; r++) {
    // Send the header, 0b00000
    sendGeneric(0, 0,  // No header for the header
                kDaikin312BitMark, kDaikin312OneSpace,
                kDaikin312BitMark, kDaikin312ZeroSpace,
                kDaikin312BitMark, kDaikin312HdrGap,
                (uint64_t)0b00000, kDaikinHeaderLength,
                kDaikin2Freq, false, 0, kDutyDefault);
    // Section #1
    sendGeneric(kDaikin312HdrMark, kDaikin312HdrSpace, kDaikin312BitMark,
                kDaikin312OneSpace, kDaikin312BitMark, kDaikin312ZeroSpace,
                kDaikin312BitMark, kDaikin312SectionGap, data,
                kDaikin312Section1Length,
                kDaikin2Freq, false, 0, kDutyDefault);
    // Section #2
    sendGeneric(kDaikin312HdrMark, kDaikin312HdrSpace, kDaikin312BitMark,
                kDaikin312OneSpace, kDaikin312BitMark, kDaikin312ZeroSpace,
                kDaikin312BitMark, kDaikin312SectionGap,
                data + kDaikin312Section1Length,
                nbytes - kDaikin312Section1Length,
                kDaikin2Freq, false, 0, kDutyDefault);
  }
}
#endif  // SEND_DAIKIN312

#if DECODE_DAIKIN312
/// Decode the supplied Daikin 312-bit / 39-byte message. (DAIKIN312)
/// Status: STABLE / Confirmed working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1829
bool IRrecv::decodeDaikin312(decode_results *results, uint16_t offset,
                             const uint16_t nbits, const bool strict) {
  // Is there enough data to match successfully?
  if (results->rawlen < 2 * (nbits + kDaikinHeaderLength + kHeader + kFooter) +
                        kFooter - 1 + offset)
    return false;

  // Compliance
  if (strict && nbits != kDaikin312Bits) return false;

  const uint8_t ksectionSize[kDaikin312Sections] = {kDaikin312Section1Length,
                                                    kDaikin312Section2Length};
  // Header/Leader Section
  uint64_t leaderdata = 0;
  uint16_t used = matchGeneric(results->rawbuf + offset, &leaderdata,
                      results->rawlen - offset, kDaikinHeaderLength,
                      0, 0,  // No Header Mark or Space for the "header"
                      kDaikin312BitMark, kDaikin312OneSpace,
                      kDaikin312BitMark, kDaikin312ZeroSpace,
                      kDaikin312BitMark, kDaikin312HdrGap,
                      false, kDaikinTolerance, 0, false);
  if (!used) return false;  // Failed to match.
  if (leaderdata) return false;  // The header bits should all be zero.

  offset += used;

  // Data Sections
  uint16_t pos = 0;
  for (uint8_t section = 0; section < kDaikin312Sections; section++) {
    // Section Header + Section Data + Section Footer
    used = matchGeneric(results->rawbuf + offset, results->state + pos,
                        results->rawlen - offset, ksectionSize[section] * 8,
                        kDaikin312HdrMark, kDaikin312HdrSpace,
                        kDaikin312BitMark, kDaikin312OneSpace,
                        kDaikin312BitMark, kDaikin312ZeroSpace,
                        kDaikin312BitMark, kDaikin312SectionGap,
                        section >= kDaikin312Sections - 1,
                        kDaikinTolerance, 0, false);
    if (used == 0) return false;
    offset += used;
    pos += ksectionSize[section];
  }
  // Compliance
  if (strict) {
    if (pos * 8 != kDaikin312Bits) return false;
  }

  // Success
  results->decode_type = decode_type_t::DAIKIN312;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // DECODE_DAIKIN312
