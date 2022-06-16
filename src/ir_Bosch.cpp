// Copyright 2022 David Conran
/// @file
/// @brief Support for the Bosch A/C / heatpump protocol
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1787

// Supports:
//   Brand: Bosch,  Model: CL3000i-Set 26 E A/C
//   Brand: Bosch,  Model: RG10A(G2S)BGEF remote

#include <algorithm>
#ifndef ARDUINO
#include <string>
#endif
#include "IRrecv.h"
#include "IRsend.h"
#include "IRtext.h"
#include "IRutils.h"

// Constants
const uint16_t kBoschHdrMark = 4366;
const uint16_t kBoschBitMark = 502;
const uint16_t kBoschHdrSpace = 4415;
const uint16_t kBoschOneSpace = 1645;
const uint16_t kBoschZeroSpace = 571;
const uint16_t kBoschFooterSpace = 5235;
const uint16_t kBoschFreq = 38000;  // Hz. (Guessing the most common frequency.)
const uint16_t kBosch144NrOfSections = 3;

#if SEND_BOSCH144
/// Send a Bosch 144-bit / 18-byte message
/// Status: STABLE / Confirmed Working.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void IRsend::sendBosch144(const unsigned char data[], const uint16_t nbytes,
                          const uint16_t repeat) {
  // nbytes is required to be a multiple of kBosch144NrOfSections.
  if (nbytes % kBosch144NrOfSections != 0) return;

  // Set IR carrier frequency
  enableIROut(kBoschFreq);

  for (uint16_t r = 0; r <= repeat; r++) {
    const uint16_t kSectionByteSize = nbytes / kBosch144NrOfSections;
    for (uint16_t offset = 0; offset < nbytes; offset += kSectionByteSize)
      // Section Header + Data + Footer
      sendGeneric(kBoschHdrMark, kBoschHdrSpace,
                  kBoschBitMark, kBoschOneSpace,
                  kBoschBitMark, kBoschZeroSpace,
                  kBoschBitMark, kBoschFooterSpace,
                  data + offset, kSectionByteSize,
                  kBoschFreq, true, 0, kDutyDefault);
    space(kDefaultMessageGap);  // Complete guess
  }
}

#endif  // SEND_BOSCH144

#if DECODE_BOSCH144
/// Decode the supplied Bosch 144-bit / 18-byte A/C message.
/// Status: STABLE / Confirmed Working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool IRrecv::decodeBosch144(decode_results *results, uint16_t offset,
                            const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * nbits +
                        kBosch144NrOfSections * (kHeader + kFooter) -
                        1 + offset)
    return false;  // Can't possibly be a valid BOSCH144 message.
  if (strict && nbits != kBosch144Bits)
    return false;      // Not strictly a BOSCH144 message.
  if (nbits % 8 != 0)  // nbits has to be a multiple of nr. of bits in a byte.
    return false;
  if (nbits % kBosch144NrOfSections != 0)
    return false;  // nbits has to be a multiple of kBosch144NrOfSections.
  const uint16_t kSectionBits = nbits / kBosch144NrOfSections;
  const uint16_t kSectionBytes = kSectionBits / 8;
  const uint16_t kNBytes = kSectionBytes * kBosch144NrOfSections;
  // Capture each section individually
  for (uint16_t pos = 0, section = 0;
       pos < kNBytes;
       pos += kSectionBytes, section++) {
    uint16_t used = 0;
    // Section Header + Section Data + Section Footer
    used = matchGeneric(results->rawbuf + offset, results->state + pos,
                        results->rawlen - offset, kSectionBits,
                        kBoschHdrMark, kBoschHdrSpace,
                        kBoschBitMark, kBoschOneSpace,
                        kBoschBitMark, kBoschZeroSpace,
                        kBoschBitMark, kBoschFooterSpace,
                        section >= kBosch144NrOfSections - 1,
                        _tolerance, kMarkExcess, true);
    if (!used) return false;  // Didn't match.
    offset += used;
  }

  // Compliance

  // Success
  results->decode_type = decode_type_t::BOSCH144;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // DECODE_BOSCH144
