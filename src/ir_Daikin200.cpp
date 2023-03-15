// Copyright 2016 sillyfrog
// Copyright 2017 sillyfrog, crankyoldgit
// Copyright 2018-2022 crankyoldgit
// Copyright 2019 pasna (IRDaikin160 class / Daikin176 class)

/// @file
/// @brief Support for Daikin A/C protocols.
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

#if SEND_DAIKIN200
/// Send a Daikin200 (200-bit) A/C formatted message.
/// Status: BETA / Untested on a real device.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1802
void IRsend::sendDaikin200(const unsigned char data[], const uint16_t nbytes,
                           const uint16_t repeat) {
  if (nbytes < kDaikin200Section1Length)
    return;  // Not enough bytes to send a partial message.

  for (uint16_t r = 0; r <= repeat; r++) {
    // Section #1
    sendGeneric(kDaikin200HdrMark, kDaikin200HdrSpace, kDaikin200BitMark,
                kDaikin200OneSpace, kDaikin200BitMark, kDaikin200ZeroSpace,
                kDaikin200BitMark, kDaikin200Gap, data,
                kDaikin200Section1Length,
                kDaikin200Freq, false, 0, kDutyDefault);
    // Section #2
    sendGeneric(kDaikin200HdrMark, kDaikin200HdrSpace, kDaikin200BitMark,
                kDaikin200OneSpace, kDaikin200BitMark, kDaikin200ZeroSpace,
                kDaikin200BitMark, kDaikin200Gap,
                data + kDaikin200Section1Length,
                nbytes - kDaikin200Section1Length,
                kDaikin200Freq, false, 0, kDutyDefault);
  }
}
#endif  // SEND_DAIKIN200

#if DECODE_DAIKIN200
/// Decode the supplied Daikin 200-bit message. (DAIKIN200)
/// Status: STABLE / Known to be working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1802
bool IRrecv::decodeDaikin200(decode_results *results, uint16_t offset,
                             const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * (nbits + kHeader + kFooter) - 1 + offset)
    return false;

  // Compliance
  if (strict && nbits != kDaikin200Bits) return false;

  const uint8_t ksectionSize[kDaikin200Sections] = {kDaikin200Section1Length,
                                                    kDaikin200Section2Length};
  // Sections
  uint16_t pos = 0;
  for (uint8_t section = 0; section < kDaikin200Sections; section++) {
    uint16_t used;
    // Section Header + Section Data + Section Footer
    used = matchGeneric(results->rawbuf + offset, results->state + pos,
                        results->rawlen - offset, ksectionSize[section] * 8,
                        kDaikin200HdrMark, kDaikin200HdrSpace,
                        kDaikin200BitMark, kDaikin200OneSpace,
                        kDaikin200BitMark, kDaikin200ZeroSpace,
                        kDaikin200BitMark, kDaikin200Gap,
                        section >= kDaikin200Sections - 1,
                        kDaikinTolerance, 0, false);
    if (used == 0) return false;
    offset += used;
    pos += ksectionSize[section];
  }
  // Compliance
  if (strict) {
    if (pos * 8 != kDaikin200Bits) return false;
    // Validate the checksum.
    if (!IRDaikin176::validChecksum(results->state, pos)) return false;
  }

  // Success
  results->decode_type = decode_type_t::DAIKIN200;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // DECODE_DAIKIN200
