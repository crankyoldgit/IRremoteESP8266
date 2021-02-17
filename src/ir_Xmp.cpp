// Copyright 2021 David Conran

/// @file
/// @brief Support for XMP protocols.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1414
/// @see http://www.hifi-remote.com/wiki/index.php/XMP

// Supports:
//   Brand: Xfinity,  Model: XR2 remote
//   Brand: Xfinity,  Model: XR11 remote


#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

// Constants
const uint16_t kXmpMark =          210;  ///< uSeconds.
const uint16_t kXmpBaseSpace =     760;  ///< uSeconds
const uint16_t kXmpSpaceStep =     135;  ///< uSeconds
const uint16_t kXmpFooterSpace = 13000;  ///< uSeconds.
const uint32_t kXmpMessageGap =  80400;  ///< uSeconds.
const uint8_t  kXmpWordSize = kNibbleSize;  ///< nr. of Bits in a word.
const uint8_t  kXmpMaxWordValue = (1 << kXmpWordSize) - 1;  // Max word value.
const uint8_t  kXmpSections = 2;  ///< Nr. of Data sections


#if SEND_XMP
/// Send a XMP packet.
/// Status:  ALPHA / Untested.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void IRsend::sendXmp(uint64_t data, uint16_t nbits, uint16_t repeat) {
  enableIROut(38000);
  if (nbits < 2 * kXmpWordSize) return;  // Too small to send, abort!
  for (uint16_t r = 0; r <= repeat; r++) {
    uint16_t bits_so_far = kXmpWordSize;
    for (uint64_t mask = ((uint64_t)kXmpMaxWordValue) << (nbits - kXmpWordSize);
         mask;
         mask >>= kXmpWordSize) {
      uint8_t word = (data & mask) >> (nbits - bits_so_far);
      mark(kXmpMark);
      space(kXmpBaseSpace + word * kXmpSpaceStep);
      bits_so_far += kXmpWordSize;
      // Are we at a data section boundary?
      if ((bits_so_far - kXmpWordSize) % (nbits / kXmpSections) == 0) {  // Yes.
        mark(kXmpMark);
        space(kXmpFooterSpace);
      }
    }
    space(kXmpMessageGap - kXmpFooterSpace);
  }
}
#endif  // SEND_XMP

#if DECODE_XMP
/// Decode the supplied XMP packet/message.
/// Status:  BETA / Probably works.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return True if it can decode it, false if it can't.
bool IRrecv::decodeXmp(decode_results *results, uint16_t offset,
                       const uint16_t nbits, const bool strict) {
  uint64_t data = 0;

  if (results->rawlen < 2 * (nbits / kXmpWordSize) + (kXmpSections * kFooter) +
      offset - 1)
    return false;  // Not enough entries to ever be XMP.

  // Compliance
  if (strict && nbits != kXmpBits) return false;

  // Data
  // Sections
  for (uint8_t section = 1; section <= kXmpSections; section++) {
    for (uint16_t bits_so_far = 0; bits_so_far < nbits / kXmpSections;
         bits_so_far += kXmpWordSize) {
      if (!matchMarkRange(results->rawbuf[offset++], kXmpMark)) return 0;
      uint8_t value = 0;
      bool found = false;
      for (; value <= kXmpMaxWordValue; value++) {
        if (matchSpaceRange(results->rawbuf[offset],
                            kXmpBaseSpace + value * kXmpSpaceStep,
                            kXmpSpaceStep / 2, 0)) {
          found = true;
          break;
        }
      }
      if (!found) return 0;  // Failure.
      data <<= kXmpWordSize;
      data += value;
      offset++;
    }
    // Section Footer
    if (!matchMarkRange(results->rawbuf[offset++], kXmpMark)) return 0;
    if (section < kXmpSections) {
      if (!matchSpace(results->rawbuf[offset++], kXmpFooterSpace)) return 0;
    } else {  // Last section
      if (offset < results->rawlen &&
       !matchAtLeast(results->rawbuf[offset++], kXmpFooterSpace)) return 0;
    }
  }

  // Compliance
  if (strict) {
    // Validate checksums.
    uint64_t checksum_data = data;
    const uint16_t section_size = nbits / kXmpSections;
    // Each section has a checksum.
    for (uint16_t section = 0; section < kXmpSections; section++) {
      // The checksum is the 2nd most significant nibble of a section.
      const uint8_t checksum = GETBITS64(checksum_data,
                                         section_size - (2 * kNibbleSize),
                                         kNibbleSize);
      if (checksum != (0xF & ~(irutils::sumNibbles(checksum_data,
                                                   section_size / kNibbleSize,
                                                   0xF, false) - checksum)))
        return 0;
      checksum_data >>= section_size;
    }
  }

  // Success
  results->value = data;
  results->decode_type = decode_type_t::XMP;
  results->bits = nbits;
  results->address = 0;
  results->command = 0;
  return true;
}
#endif  // DECODE_XMP
