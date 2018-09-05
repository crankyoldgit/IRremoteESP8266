// Copyright 2009 Ken Shirriff
// Copyright 2017 David Conran

#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

//              SSSS   AAA    MMM    SSSS  U   U  N   N   GGGG
//             S      A   A  M M M  S      U   U  NN  N  G
//              SSS   AAAAA  M M M   SSS   U   U  N N N  G  GG
//                 S  A   A  M   M      S  U   U  N  NN  G   G
//             SSSS   A   A  M   M  SSSS    UUU   N   N   GGG

// Samsung originally added from https://github.com/shirriff/Arduino-IRremote/

// Constants
// Ref:
//   http://elektrolab.wz.cz/katalog/samsung_protocol.pdf
const uint16_t kSamsungTick = 560;
const uint16_t kSamsungHdrMarkTicks = 8;
const uint16_t kSamsungHdrMark = kSamsungHdrMarkTicks * kSamsungTick;
const uint16_t kSamsungHdrSpaceTicks = 8;
const uint16_t kSamsungHdrSpace = kSamsungHdrSpaceTicks * kSamsungTick;
const uint16_t kSamsungBitMarkTicks = 1;
const uint16_t kSamsungBitMark = kSamsungBitMarkTicks * kSamsungTick;
const uint16_t kSamsungOneSpaceTicks = 3;
const uint16_t kSamsungOneSpace = kSamsungOneSpaceTicks * kSamsungTick;
const uint16_t kSamsungZeroSpaceTicks = 1;
const uint16_t kSamsungZeroSpace = kSamsungZeroSpaceTicks * kSamsungTick;
const uint16_t kSamsungRptSpaceTicks = 4;
const uint16_t kSamsungRptSpace = kSamsungRptSpaceTicks * kSamsungTick;
const uint16_t kSamsungMinMessageLengthTicks = 193;
const uint32_t kSamsungMinMessageLength =
    kSamsungMinMessageLengthTicks * kSamsungTick;
const uint16_t kSamsungMinGapTicks = kSamsungMinMessageLengthTicks -
    (kSamsungHdrMarkTicks + kSamsungHdrSpaceTicks +
     kSamsungBits * (kSamsungBitMarkTicks + kSamsungOneSpaceTicks) +
     kSamsungBitMarkTicks);
const uint32_t kSamsungMinGap = kSamsungMinGapTicks * kSamsungTick;

const uint16_t kSamsungAcHdrMark = 690;
const uint16_t kSamsungAcHdrSpace = 17844;
const uint8_t  kSamsungAcSections = 2;
const uint16_t kSamsungAcSectionMark = 3086;
const uint16_t kSamsungAcSectionSpace = 8864;
const uint16_t kSamsungAcSectionGap = 2886;
const uint16_t kSamsungAcBitMark = 586;
const uint16_t kSamsungAcOneSpace = 1432;
const uint16_t kSamsungAcZeroSpace = 436;

#if SEND_SAMSUNG
// Send a Samsung formatted message.
// Samsung has a separate message to indicate a repeat, like NEC does.
// TODO(crankyoldgit): Confirm that is actually how Samsung sends a repeat.
//                     The refdoc doesn't indicate it is true.
//
// Args:
//   data:   The message to be sent.
//   nbits:  The bit size of the message being sent. typically kSamsungBits.
//   repeat: The number of times the message is to be repeated.
//
// Status: BETA / Should be working.
//
// Ref: http://elektrolab.wz.cz/katalog/samsung_protocol.pdf
void IRsend::sendSAMSUNG(uint64_t data, uint16_t nbits, uint16_t repeat) {
  sendGeneric(kSamsungHdrMark, kSamsungHdrSpace,
              kSamsungBitMark, kSamsungOneSpace,
              kSamsungBitMark, kSamsungZeroSpace,
              kSamsungBitMark,
              kSamsungMinGap, kSamsungMinMessageLength,
              data, nbits, 38, true, repeat, 33);
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
uint32_t IRsend::encodeSAMSUNG(uint8_t customer, uint8_t command) {
  customer = reverseBits(customer, sizeof(customer) * 8);
  command = reverseBits(command, sizeof(command) * 8);
  return((command ^ 0xFF) | (command << 8) |
         (customer << 16) | (customer << 24));
}
#endif

#if DECODE_SAMSUNG
// Decode the supplied Samsung message.
// Samsung messages whilst 32 bits in size, only contain 16 bits of distinct
// data. e.g. In transmition order:
//   customer_byte + customer_byte(same) + address_byte + invert(address_byte)
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of bits to expect in the data portion. Typically kSamsungBits.
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
bool IRrecv::decodeSAMSUNG(decode_results *results, uint16_t nbits,
                           bool strict) {
  if (results->rawlen < 2 * nbits + kHeader + kFooter - 1)
    return false;  // Can't possibly be a valid Samsung message.
  if (strict && nbits != kSamsungBits)
    return false;  // We expect Samsung to be 32 bits of message.

  uint64_t data = 0;
  uint16_t offset = kStartOffset;

  // Header
  if (!matchMark(results->rawbuf[offset], kSamsungHdrMark)) return false;
  // Calculate how long the common tick time is based on the header mark.
  uint32_t m_tick = results->rawbuf[offset++] * kRawTick /
      kSamsungHdrMarkTicks;
  if (!matchSpace(results->rawbuf[offset], kSamsungHdrSpace)) return false;
  // Calculate how long the common tick time is based on the header space.
  uint32_t s_tick = results->rawbuf[offset++] * kRawTick /
      kSamsungHdrSpaceTicks;
  // Data
  match_result_t data_result = matchData(&(results->rawbuf[offset]), nbits,
                                         kSamsungBitMarkTicks * m_tick,
                                         kSamsungOneSpaceTicks * s_tick,
                                         kSamsungBitMarkTicks * m_tick,
                                         kSamsungZeroSpaceTicks * s_tick);
  if (data_result.success == false) return false;
  data = data_result.data;
  offset += data_result.used;
  // Footer
  if (!matchMark(results->rawbuf[offset++], kSamsungBitMarkTicks * m_tick))
    return false;
  if (offset < results->rawlen &&
      !matchAtLeast(results->rawbuf[offset], kSamsungMinGapTicks * s_tick))
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
  results->command = reverseBits(command, sizeof(command) * 8);
  results->address = reverseBits(address, sizeof(address) * 8);
  return true;
}
#endif

#if SEND_SAMSUNG_AC
// Send a Samsung A/C message.
//
// Args:
//   data: An array of bytes containing the IR command.
//   nbytes: Nr. of bytes of data in the array. (>=kSamsungAcStateLength)
//   repeat: Nr. of times the message is to be repeated. (Default = 0).
//
// Status: ALPHA / Untested.
//
// Ref:
//   https://github.com/markszabo/IRremoteESP8266/issues/505
void IRsend::sendSamsungAC(uint8_t data[], uint16_t nbytes, uint16_t repeat) {
  if (nbytes < kSamsungAcStateLength)
  return;  // Not enough bytes to send a proper message.

  enableIROut(38);
  for (uint16_t r = 0; r <= repeat; r++) {
    // Header
    mark(kSamsungAcHdrMark);
    space(kSamsungAcHdrSpace);
    // Section 1
    sendGeneric(kSamsungAcSectionMark, kSamsungAcSectionSpace,
                kSamsungAcBitMark, kSamsungAcOneSpace,
                kSamsungAcBitMark, kSamsungAcZeroSpace,
                kSamsungAcBitMark, kSamsungAcSectionGap,
                data, 7,  // 7 bytes == 56 bits
                38000,
                false, 0, 50);  // Send in LSBF order
    // Section 2
    sendGeneric(kSamsungAcSectionMark, kSamsungAcSectionSpace,
                kSamsungAcBitMark, kSamsungAcOneSpace,
                kSamsungAcBitMark, kSamsungAcZeroSpace,
                kSamsungAcBitMark,
                100000,  // Complete made up guess at inter-message gap.
                data + 7, 7,  // 7 bytes == 56 bits
                38000,
                false, 0, 50);  // Send in LSBF order
    }
}
#endif  // SEND_SAMSUNG_AC

#if DECODE_SAMSUNG_AC
// Decode the supplied Samsung A/C message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect. Typically kSamsungAcBits
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: ALPHA / Untested.
//
//
// Ref:
//   https://github.com/markszabo/IRremoteESP8266/issues/505
bool IRrecv::decodeSamsungAC(decode_results *results, uint16_t nbits,
                             bool strict) {
  if (results->rawlen < 2 * nbits + kHeader * 3 + kFooter * 2 - 1)
    return false;  // Can't possibly be a valid Samsung A/C message.
  if (strict) {
    if (nbits != kSamsungAcBits)
      return false;
  }

  uint16_t offset = kStartOffset;
  uint16_t dataBitsSoFar = 0;
  match_result_t data_result;

  // Message Header
  if (!matchMark(results->rawbuf[offset++], kSamsungAcBitMark))
    return false;
  if (!matchSpace(results->rawbuf[offset++], kSamsungAcHdrSpace))
    return false;
  // Section(s)
  for (uint8_t section = 0, pos = 7, i = 0;
       section < kSamsungAcSections;
       section++, pos += 7) {
    // Section Header
    if (!matchMark(results->rawbuf[offset++], kSamsungAcSectionMark))
      return false;
    if (!matchSpace(results->rawbuf[offset++], kSamsungAcSectionSpace))
      return false;
    // Section Data
    // Keep reading bytes until we either run out of section or state to fill.
    for (; offset <= results->rawlen - 16 && i < pos;
         i++, dataBitsSoFar += 8, offset += data_result.used) {
      data_result = matchData(&(results->rawbuf[offset]), 8,
                              kSamsungAcBitMark,
                              kSamsungAcOneSpace,
                              kSamsungAcBitMark,
                              kSamsungAcZeroSpace);
      if (data_result.success == false)  break;  // Fail
      // Data is in LSB order. We need to reverse it.
      results->state[i] = (uint8_t) reverseBits(data_result.data & 0xFF, 8);
    }
    // Section Footer
    if (!matchMark(results->rawbuf[offset++], kSamsungAcBitMark))
      return false;
    if (section < kSamsungAcSections - 1) {  // Inter-section gap.
      if (!matchSpace(results->rawbuf[offset++], kSamsungAcSectionGap))
        return false;
    } else {  // Last section / End of message gap.
      if (offset <= results->rawlen &&
        !matchAtLeast(results->rawbuf[offset++], kSamsungAcSectionGap))
        return false;
    }
  }
  // Compliance
  if (strict) {
    // Re-check we got the correct size/length due to the way we read the data.
    if (dataBitsSoFar != kSamsungAcBits)  return false;
  }
  // Success
  results->decode_type = SAMSUNG_AC;
  results->bits = dataBitsSoFar;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // DECODE_SAMSUNG_AC
