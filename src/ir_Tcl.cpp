// Copyright 2019 David Conran

#include "ir_Tcl.h"
#include "IRremoteESP8266.h"
#include "IRutils.h"

// Constants


#if SEND_TCL112AC
void IRsend::sendTcl112Ac(const unsigned char data[], const uint16_t nbytes,
                          const uint16_t repeat) {
  sendGeneric(kTcl112AcHdrMark, kTcl112AcHdrSpace,
              kTcl112AcBitMark, kTcl112AcOneSpace,
              kTcl112AcBitMark, kTcl112AcZeroSpace,
              kTcl112AcBitMark, kTcl112AcGap,
              data, nbytes, 38000, false, repeat, 50);
}
#endif  // SEND_TCL112AC

IRTcl112Ac::IRTcl112Ac(uint16_t pin) : _irsend(pin) { stateReset(); }

void IRTcl112Ac::begin() { _irsend.begin(); }

#if SEND_TCL112AC
void IRTcl112Ac::send(const uint16_t repeat) {
  checksum();
  _irsend.sendTcl112Ac(remote_state, kTcl112AcStateLength, repeat);
}
#endif  // SEND_TCL112AC

void IRTcl112Ac::checksum() {
}

void IRTcl112Ac::stateReset() {
}

uint8_t* IRTcl112Ac::getRaw() {
  checksum();
  return remote_state;
}

void IRTcl112Ac::setRaw(const uint8_t new_code[], const uint16_t length) {
  for (uint8_t i = 0; i < length && i < kTcl112AcStateLength; i++) {
    remote_state[i] = new_code[i];
  }
}

#if DECODE_TCL112AC
// Decode the supplied TCL112AC message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect. Typically kTcl112AcBits.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: BETA / Appears to mostly work.
//
// Ref:
//   https://github.com/markszabo/IRremoteESP8266/issues/619
bool IRrecv::decodeTcl112Ac(decode_results *results, uint16_t nbits,
                            bool strict) {
  if (results->rawlen < 2 * nbits + kHeader + kFooter - 1)
    return false;  // Can't possibly be a valid Samsung A/C message.
  if (strict && nbits != kTcl112AcBits) return false;

  uint16_t offset = kStartOffset;
  uint16_t dataBitsSoFar = 0;
  match_result_t data_result;

  // Message Header
  if (!matchMark(results->rawbuf[offset++], kTcl112AcHdrMark)) return false;
  if (!matchSpace(results->rawbuf[offset++], kTcl112AcHdrSpace)) return false;

  // Data
  // Keep reading bytes until we either run out of section or state to fill.
  for (uint16_t i = 0; offset <= results->rawlen - 16 && i < nbits / 8;
       i++, dataBitsSoFar += 8, offset += data_result.used) {
    data_result = matchData(&(results->rawbuf[offset]), 8, kTcl112AcBitMark,
                            kTcl112AcOneSpace, kTcl112AcBitMark,
                            kTcl112AcZeroSpace, kTolerance, 0, false);
    if (data_result.success == false) {
      DPRINT("DEBUG: offset = ");
      DPRINTLN(offset + data_result.used);
      return false;  // Fail
    }
    results->state[i] = data_result.data;
  }

  // Footer
  if (!matchMark(results->rawbuf[offset++], kTcl112AcBitMark)) return false;
  if (offset <= results->rawlen &&
      !matchAtLeast(results->rawbuf[offset++], kTcl112AcGap)) return false;
  // Compliance
  // Re-check we got the correct size/length due to the way we read the data.
  if (dataBitsSoFar != nbits) return false;
  // Success
  results->decode_type = TCL112AC;
  results->bits = dataBitsSoFar;
  // No need to record the state as we stored it as we decoded it.
  // As we use result->state, we don't record value, address, or command as it
  // is a union data type.
  return true;
}
#endif  // DECODE_TCL112AC
