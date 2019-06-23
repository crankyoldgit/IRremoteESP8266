// Copyright 2019 David Conran

// Neoclima A/C support

// Supports:
//   Brand: Neoclima,  Model: NS-09AHTI A/C
//   Brand: Neoclima,  Model: ZH/TY-01 remote

#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

// Constants

const uint16_t kNeoclimaHdrMark = 6112;
const uint16_t kNeoclimaHdrSpace = 7391;
const uint16_t kNeoclimaBitMark = 537;
const uint16_t kNeoclimaOneSpace = 1651;
const uint16_t kNeoclimaZeroSpace = 571;
const uint32_t kNeoclimaMinGap = kDefaultMessageGap;

#if SEND_NEOCLIMA
// Send a Neoclima message.
//
// Args:
//   data: message to be sent.
//   nbytes: Nr. of bytes of the message to be sent.
//   repeat: Nr. of additional times the message is to be sent.
//
// Status: Beta / Known to be working.
//
// Ref:
//   https://github.com/markszabo/IRremoteESP8266/issues/764
void IRsend::sendNeoclima(const unsigned char data[], const uint16_t nbytes,
                          const uint16_t repeat) {
  // Set IR carrier frequency
  enableIROut(38);

  for (uint16_t i = 0; i <= repeat; i++) {
    sendGeneric(kNeoclimaHdrMark, kNeoclimaHdrSpace,
                kNeoclimaBitMark, kNeoclimaOneSpace,
                kNeoclimaBitMark, kNeoclimaZeroSpace,
                kNeoclimaBitMark, kNeoclimaHdrSpace,
                data, nbytes, 38000, false, 0,  // Repeats are already handled.
                50);
     // Extra footer.
     mark(kNeoclimaBitMark);
     space(kNeoclimaMinGap);
  }
}
#endif  // SEND_NEOCLIMA

#if DECODE_NEOCLIMA
// Decode the supplied Neoclima message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of data bits to expect. Typically kNeoclimaBits.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: BETA / Probably works
//
// Ref:
//   https://github.com/markszabo/IRremoteESP8266/issues/764
bool IRrecv::decodeNeoclima(decode_results *results, const uint16_t nbits,
                            const bool strict) {
  // Compliance
  if (strict && nbits != kNeoclimaBits)
    return false;  // Incorrect nr. of bits per spec.

  uint16_t offset = kStartOffset;
  // Match Main Header + Data + Footer
  uint16_t used;
  used = matchGeneric(results->rawbuf + offset, results->state,
                      results->rawlen - offset, nbits,
                      kNeoclimaHdrMark, kNeoclimaHdrSpace,
                      kNeoclimaBitMark, kNeoclimaOneSpace,
                      kNeoclimaBitMark, kNeoclimaZeroSpace,
                      kNeoclimaBitMark, kNeoclimaHdrSpace, false,
                      kTolerance, 0, false);
  if (!used) return false;
  offset += used;
  // Extra footer.
  uint64_t unused;
  if (!matchGeneric(results->rawbuf + offset, &unused,
                    results->rawlen - offset, 0, 0, 0, 0, 0, 0, 0,
                    kNeoclimaBitMark, kNeoclimaHdrSpace, true)) return false;
  // Success
  results->decode_type = decode_type_t::NEOCLIMA;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // DECODE_NEOCLIMA
