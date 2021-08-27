#include "ir_Bose.h"

#include "IRrecv.h"
#include "IRsend.h"

const uint16_t kBoseHdrMark = 1100;
const uint16_t kBoseHdrSpace = 1350;
const uint16_t kBoseBitMark = 555;
const uint16_t kBoseOneSpace = 1435;
const uint16_t kBoseZeroSpace = 500;
const uint32_t kBoseGap = kDefaultMessageGap;
const uint16_t kBoseFreq = 38;

#if SEND_BOSE

void IRsend::sendBose(const uint64_t data, const uint16_t nbits,
                      const uint16_t repeat) {
  sendGeneric(kBoseHdrMark, kBoseHdrSpace,
              kBoseBitMark, kBoseOneSpace,
              kBoseBitMark, kBoseZeroSpace,
              kBoseBitMark, kBoseGap,
              data, nbits, kBoseFreq, false,
              repeat, 50);
}

#endif // SEND_BOSE

#if DECODE_KELON

bool IRrecv::decodeBose(decode_results *results, uint16_t offset,
                        const uint16_t nbits, const bool strict) {
  if (strict && nbits != kBoseBits) {
    return false;
  }

  if (!matchGeneric(results->rawbuf + offset, results->state,
                    results->rawlen - offset, nbits,
                    kBoseHdrMark, kBoseHdrSpace,
                    kBoseBitMark, kBoseOneSpace,
                    kBoseBitMark, kBoseZeroSpace,
                    kBoseBitMark, 0, false,
                    _tolerance, 0, false)) {
    return false;
  }

  results->decode_type = decode_type_t::BOSE;
  results->bits = nbits;
  return true;
}
        
#endif // DECODE_BOSE
