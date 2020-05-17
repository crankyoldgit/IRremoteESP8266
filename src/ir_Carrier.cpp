// Copyright 2018 David Conran

// Supports:
//   Brand: Carrier/Surrey,  Model: 42QG5A55970 remote
//   Brand: Carrier/Surrey,  Model: 619EGX0090E0 A/C
//   Brand: Carrier/Surrey,  Model: 619EGX0120E0 A/C
//   Brand: Carrier/Surrey,  Model: 619EGX0180E0 A/C
//   Brand: Carrier/Surrey,  Model: 619EGX0220E0 A/C
//   Brand: Carrier/Surrey,  Model: 53NGK009/012 Inverter

#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

// Constants
// Ref:
//   https://github.com/crankyoldgit/IRremoteESP8266/issues/385
const uint16_t kCarrierAcHdrMark = 8532;
const uint16_t kCarrierAcHdrSpace = 4228;
const uint16_t kCarrierAcBitMark = 628;
const uint16_t kCarrierAcOneSpace = 1320;
const uint16_t kCarrierAcZeroSpace = 532;
const uint16_t kCarrierAcGap = 20000;



const uint16_t kTestCarrierHdrMark = 8968;
const uint16_t kTestCarrierBitMark = 503;
const uint16_t kTestCarrierHdrSpace = 4526;
const uint16_t kTestCarrierOneSpace = 1736;
const uint16_t kTestCarrierZeroSpace = 615;
const uint16_t kTestCarrierFreq = 38000;  // Hz. (Guessing the most common frequency.)
const uint16_t kTestCarrierOverhead = 3;

#if SEND_CARRIER_AC
// Send a Carrier HVAC formatted message.
//
// Args:
//   data:   The message to be sent.
//   nbits:  The bit size of the message being sent. typically kCarrierAcBits.
//   repeat: The number of times the message is to be repeated.
//
// Status: STABLE / Work on real devices.
//
void IRsend::sendCarrierAC(uint64_t data, uint16_t nbits, uint16_t repeat) {
  for (uint16_t r = 0; r <= repeat; r++) {
    uint64_t temp_data = data;
    // Carrier sends the data block three times. normal + inverted + normal.
    for (uint16_t i = 0; i < 3; i++) {
      sendGeneric(kCarrierAcHdrMark, kCarrierAcHdrSpace, kCarrierAcBitMark,
                  kCarrierAcOneSpace, kCarrierAcBitMark, kCarrierAcZeroSpace,
                  kCarrierAcBitMark, kCarrierAcGap, temp_data, nbits, 38, true,
                  0, kDutyDefault);
      temp_data = invertBits(temp_data, nbits);
    }
  }
}
#endif

#if DECODE_CARRIER_AC
// Decode the supplied Carrier HVAC message.
// Carrier HVAC messages contain only 32 bits, but it is sent three(3) times.
// i.e. normal + inverted + normal
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   offset:  The starting index to use when attempting to decode the raw data.
//            Typically/Defaults to kStartOffset.
//   nbits:   Nr. of bits to expect in the data portion.
//            Typically kCarrierAcBits.
//   strict:  Flag to indicate if we strictly adhere to the specification.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: BETA / Probably works.
//
bool IRrecv::decodeCarrierAC(decode_results *results, uint16_t offset,
                             const uint16_t nbits, const bool strict) {
  if (results->rawlen < ((2 * nbits + kHeader + kFooter) * 3) - 1 + offset)
    return false;  // Can't possibly be a valid Carrier message.
  if (strict && nbits != kCarrierAcBits)
    return false;  // We expect Carrier to be 32 bits of message.

  uint64_t data = 0;
  uint64_t prev_data = 0;

  for (uint8_t i = 0; i < 3; i++) {
    prev_data = data;
    // Match Header + Data + Footer
    uint16_t used;
    used = matchGeneric(results->rawbuf + offset, &data,
                        results->rawlen - offset, nbits,
                        kCarrierAcHdrMark, kCarrierAcHdrSpace,
                        kCarrierAcBitMark, kCarrierAcOneSpace,
                        kCarrierAcBitMark, kCarrierAcZeroSpace,
                        kCarrierAcBitMark, kCarrierAcGap, true);
    if (!used) return false;
    offset += used;
    // Compliance.
    if (strict) {
      // Check if the data is an inverted copy of the previous data.
      if (i > 0 && prev_data != invertBits(data, nbits)) return false;
    }
  }

  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = CARRIER_AC;
  results->address = data >> 16;
  results->command = data & 0xFFFF;
  return true;
}
#endif



#if SEND_TESTCARRIER
// Function should be safe up to 64 bits.
void IRsend::sendTestCarrier(const uint64_t data, const uint16_t nbits, const uint16_t repeat) {
  enableIROut(kTestCarrierFreq);
  for (uint16_t r = 0; r <= repeat; r++) {
    uint64_t send_data = data;
    // Header
    mark(kTestCarrierHdrMark);
    space(kTestCarrierHdrSpace);
    // Data Section #1
    // e.g. data = 0x21AA46C008000202, nbits = 64
    sendData(kTestCarrierBitMark, kTestCarrierOneSpace, kTestCarrierBitMark, kTestCarrierZeroSpace, send_data, 64, true);
    send_data >>= 64;
    // Footer
    mark(kTestCarrierBitMark);
    space(kDefaultMessageGap);  // A 100% made up guess of the gap between messages.
  }
}
#endif  // SEND_TESTCARRIER

#if DECODE_TESTCARRIER
// Function should be safe up to 64 bits.
bool IRrecv::decodeTestCarrier(decode_results *results, const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * nbits + kTestCarrierOverhead)
    return false;  // Too short a message to match.
  if (strict && nbits != kTestCarrierBits)
    return false;

  uint16_t offset = kStartOffset;
  uint64_t data = 0;
  match_result_t data_result;

  // Header
  if (!matchMark(results->rawbuf[offset++], kTestCarrierHdrMark))
    return false;
  if (!matchSpace(results->rawbuf[offset++], kTestCarrierHdrSpace))
    return false;

  // Data Section #1
  // e.g. data_result.data = 0x21AA46C008000202, nbits = 64
  data_result = matchData(&(results->rawbuf[offset]), 64,
                          kTestCarrierBitMark, kTestCarrierOneSpace,
                          kTestCarrierBitMark, kTestCarrierZeroSpace);
  offset += data_result.used;
  if (data_result.success == false) return false;  // Fail
  data <<= 64;  // Make room for the new bits of data.
  data |= data_result.data;

  // Footer
  if (!matchMark(results->rawbuf[offset++], kTestCarrierBitMark))
    return false;

  // Success
  results->decode_type = decode_type_t::TESTCARRIER;
  results->bits = nbits;
  results->value = data;
  results->command = 0;
  results->address = 0;
  return true;
}
#endif  // DECODE_TESTCARRIER

