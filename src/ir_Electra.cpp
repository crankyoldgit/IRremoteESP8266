// Copyright 2018, 2019 David Conran

#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

// Electra A/C added by crankyoldgit
//
// Equipment it seems compatible with:
//  * <Add models (A/C & remotes) you've gotten it working with here>

// Ref:
//   https://github.com/markszabo/IRremoteESP8266/issues/527
//   https://github.com/markszabo/IRremoteESP8266/issues/642

// Constants
const uint16_t kElectraAcHdrMark = 9166;
const uint16_t kElectraAcBitMark = 646;
const uint16_t kElectraAcHdrSpace = 4470;
const uint16_t kElectraAcOneSpace = 1647;
const uint16_t kElectraAcZeroSpace = 547;
const uint32_t kElectraAcMessageGap = kDefaultMessageGap;  // Just a guess.

#if SEND_ELECTRA_AC
// Send a Electra message
//
// Args:
//   data:   Contents of the message to be sent. (Guessing MSBF order)
//   nbits:  Nr. of bits of data to be sent. Typically kElectraAcBits.
//   repeat: Nr. of additional times the message is to be sent.
//
// Status: Alpha / Needs testing against a real device.
//
void IRsend::sendElectraAC(const uint8_t data[], const uint16_t nbytes,
                           const uint16_t repeat) {
  for (uint16_t r = 0; r <= repeat; r++)
    sendGeneric(kElectraAcHdrMark, kElectraAcHdrSpace, kElectraAcBitMark,
                kElectraAcOneSpace, kElectraAcBitMark, kElectraAcZeroSpace,
                kElectraAcBitMark, kElectraAcMessageGap, data, nbytes,
                38000,  // Complete guess of the modulation frequency.
                false,  // Send data in LSB order per byte
                0, 50);
}
#endif

#if DECODE_ELECTRA_AC
// Decode the supplied Electra A/C message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect. Typically kElectraAcBits.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: Beta / Probably works.
//
bool IRrecv::decodeElectraAC(decode_results *results, uint16_t nbits,
                             bool strict) {
  if (nbits % 8 != 0)  // nbits has to be a multiple of nr. of bits in a byte.
    return false;

  if (strict) {
    if (nbits != kElectraAcBits)
      return false;  // Not strictly a ELECTRA_AC message.
  }

  uint16_t offset = kStartOffset;
  // Match Header + Data + Footer
  if (!matchGenericBytes(results->rawbuf + offset, results->state,
                         results->rawlen - offset, nbits / 8,
                         kElectraAcHdrMark, kElectraAcHdrSpace,
                         kElectraAcBitMark, kElectraAcOneSpace,
                         kElectraAcBitMark, kElectraAcZeroSpace,
                         kElectraAcBitMark, kElectraAcMessageGap, true,
                         kTolerance, 0, false)) return false;

  // Compliance
  if (strict) {
    // Verify the checksum.
    if (sumBytes(results->state, (nbits / 8) - 1) !=
        results->state[(nbits / 8) - 1]) return false;
  }

  // Success
  results->decode_type = ELECTRA_AC;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // DECODE_ELECTRA_AC
