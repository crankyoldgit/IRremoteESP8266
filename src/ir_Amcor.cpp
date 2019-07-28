// Copyright 2019 David Conran

// Supports:
//   Brand: Amcor,  Model: ADR-853H A/C
//   Brand: Amcor,  Model: TAC-495 remote
//   Brand: Amcor,  Model: TAC-444 remote

#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

// Constants
// Ref:
//   https://github.com/crankyoldgit/IRremoteESP8266/issues/385
const uint16_t kAmcorHdrMark = 8200;
const uint16_t kAmcorHdrSpace = 4200;
const uint16_t kAmcorOneMark = 1500;
const uint16_t kAmcorZeroMark = 600;
const uint16_t kAmcorOneSpace = kAmcorZeroMark;
const uint16_t kAmcorZeroSpace = kAmcorOneMark;
const uint16_t kAmcorFooterMark = 1900;
const uint16_t kAmcorGap = 34300;
const uint8_t  kAmcorTolerance = 40;

#if SEND_AMCOR
// Send a Amcor HVAC formatted message.
//
// Args:
//   data:   The message to be sent.
//   nbytes: The byte size of the array being sent. typically kAmcorStateLength.
//   repeat: The number of times the message is to be repeated.
//
// Status: Alpha / Needs testing.
//
void IRsend::sendAmcor(const unsigned char data[], const uint16_t nbytes,
                       const uint16_t repeat) {
  // Check if we have enough bytes to send a proper message.
  if (nbytes < kAmcorStateLength) return;
  sendGeneric(kAmcorHdrMark, kAmcorHdrSpace, kAmcorOneMark, kAmcorOneSpace,
             kAmcorZeroMark, kAmcorZeroSpace, kAmcorFooterMark, kAmcorGap,
             data, nbytes, 38, false, repeat, kDutyDefault);
}
#endif

#if DECODE_AMCOR
// Decode the supplied Amcor HVAC message.
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of bits to expect in the data portion.
//            Typically kAmcorBits.
//   strict:  Flag to indicate if we strictly adhere to the specification.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: BETA / Seems to be working.
//
bool IRrecv::decodeAmcor(decode_results *results, uint16_t nbits,
                         bool strict) {
  if (results->rawlen < 2 * nbits + kHeader - 1)
    return false;  // Can't possibly be a valid Amcor message.
  if (strict && nbits != kAmcorBits)
    return false;  // We expect Amcor to be 64 bits of message.

  uint16_t offset = kStartOffset;

  uint16_t used;
  // Header + Data Block (64 bits) + Footer
  used = matchGeneric(results->rawbuf + offset, results->state,
                      results->rawlen - offset, 64,
                      kAmcorHdrMark, kAmcorHdrSpace,
                      kAmcorOneMark, kAmcorOneSpace,
                      kAmcorZeroMark, kAmcorZeroSpace,
                      kAmcorFooterMark, kAmcorGap, true,
                      kAmcorTolerance, 0, false);
  if (!used) return false;
  offset += used;

  // Success
  results->bits = nbits;
  results->decode_type = AMCOR;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif
