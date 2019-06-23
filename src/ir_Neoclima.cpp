// Copyright 2019 David Conran

// Neoclima A/C support

// Supports:
//   Brand: Neoclima,  Model: NS-09AHTI A/C
//   Brand: Neoclima,  Model: ZH/TY-01 remote

#include "ir_Neoclima.h"
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

IRNeoclimaAc::IRNeoclimaAc(const uint16_t pin) : _irsend(pin) {
  this->stateReset();
}

void IRNeoclimaAc::stateReset(void) {
  for (uint8_t i = 0; i < kNeoclimaStateLength; i++)
    remote_state[i] = 0x0;
  remote_state[7] = 0x6A;
  remote_state[8] = 0x00;
  remote_state[9] = 0x2A;
  remote_state[10] = 0xA5;
  // [11] is the checksum.
}

void IRNeoclimaAc::begin(void) { _irsend.begin(); }

uint8_t IRNeoclimaAc::calcChecksum(const uint8_t state[],
                                   const uint16_t length) {
  if (length == 0) return state[0];
  return sumBytes(state, length - 1);
}

bool IRNeoclimaAc::validChecksum(const uint8_t state[], const uint16_t length) {
  if (length < 2)
    return true;  // No checksum to compare with. Assume okay.
  return (state[length - 1] == calcChecksum(state, length));
}

// Update the checksum for the internal state.
void IRNeoclimaAc::checksum(uint16_t length) {
  if (length < 2) return;
  remote_state[length - 1] = calcChecksum(remote_state, length);
}

#if SEND_NEOCLIMA
void IRNeoclimaAc::send(const uint16_t repeat) {
  this->checksum();
  _irsend.sendNeoclima(remote_state, kNeoclimaStateLength, repeat);
}
#endif  // SEND_NEOCLIMA

uint8_t *IRNeoclimaAc::getRaw(void) {
  this->checksum();
  return remote_state;
}

void IRNeoclimaAc::setRaw(const uint8_t new_code[], const uint16_t length) {
  for (uint8_t i = 0; i < length && i < kNeoclimaStateLength; i++)
    remote_state[i] = new_code[i];
}

// Set the temp. in deg C
void IRNeoclimaAc::setTemp(const uint8_t temp) {
  uint8_t newtemp = std::max(kNeoclimaMinTemp, temp);
  newtemp = std::min(kNeoclimaMaxTemp, newtemp);
  remote_state[9] = (remote_state[9] & ~kNeoclimaTempMask) |
    (newtemp + kNeoclimaMinTemp);
}

// Return the set temp. in deg C
uint8_t IRNeoclimaAc::getTemp(void) {
  return (remote_state[9] & kNeoclimaTempMask) - kNeoclimaMinTemp;
}

// Convert the internal state into a human readable string.
String IRNeoclimaAc::toString(void) {
  String result = "";
  result.reserve(100);  // Reserve some heap for the string to reduce fragging.
  result += F("Temp: ");
  result += uint64ToString(getTemp());
  result += F("C");
  return result;
}

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
// Status: BETA / Known working
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

  // Compliance
  if (strict) {
    // Check we got a valid checksum.
    if (!IRNeoclimaAc::validChecksum(results->state, nbits / 8)) return false;
  }

  // Success
  results->decode_type = decode_type_t::NEOCLIMA;
  results->bits = nbits;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // DECODE_NEOCLIMA
