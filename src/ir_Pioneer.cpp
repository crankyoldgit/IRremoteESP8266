// Copyright 2009 Ken Shirriff
// Copyright 2017 David Conran
// Copyright 2018 Kamil Palczewski

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"
#include <Arduino.h>

//                        PPPP  III  OOO  N   N EEEE EEEE RRRR
//                        P   P  I  O   O NN  N E    E    R   R
//                        PPPP   I  O   O N N N EEE  EEE  RRRR
//                        P      I  O   O N  NN E    E    R R
//                        P     III  OOO  N   N EEEE EEEE R  RR

// Constants
// Ref:
//  http://adrian-kingston.com/IRFormatPioneer.htm

const uint16_t kNecTick = 560;
const uint16_t kNecHdrMarkTicks = 16;
const uint16_t kNecHdrMark = kNecHdrMarkTicks * kNecTick;
const uint16_t kNecHdrSpaceTicks = 8;
const uint16_t kNecHdrSpace = kNecHdrSpaceTicks * kNecTick;
const uint16_t kNecBitMarkTicks = 1;
const uint16_t kNecBitMark = kNecBitMarkTicks * kNecTick;
const uint16_t kNecOneSpaceTicks = 3;
const uint16_t kNecOneSpace = kNecOneSpaceTicks * kNecTick;
const uint16_t kNecZeroSpaceTicks = 1;
const uint16_t kNecZeroSpace = kNecZeroSpaceTicks * kNecTick;
const uint16_t kPioneerCodeSeparatorTicks = 989;
const uint16_t kNecRptSpaceTicks = 4;
const uint16_t kNecRptSpace = kNecRptSpaceTicks * kNecTick;
const uint16_t kNecRptLength = 4;
const uint16_t kNecMinCommandLengthTicks = 193;
const uint32_t kNecMinCommandLength = kNecMinCommandLengthTicks * kNecTick;
const uint32_t kNecMinGap = kNecMinCommandLength -
    (kNecHdrMark + kNecHdrSpace + kNECBits * (kNecBitMark + kNecOneSpace) +
     kNecBitMark);
const uint16_t kNecMinGapTicks = kNecMinCommandLengthTicks -
    (kNecHdrMarkTicks + kNecHdrSpaceTicks +
     kNECBits * (kNecBitMarkTicks + kNecOneSpaceTicks) +
     kNecBitMarkTicks);

#if SEND_PIONEER
// Send a raw Pioneer formatted message.
//
// Args:
//   data:   The message to be sent.
//   nbits:  The number of bits of the message to be sent. Typically kPioneerBits.
//   repeat: The number of times the command is to be repeated.
//
// Status: STABLE / Known working.
//
// Ref:
//  http://adrian-kingston.com/IRFormatPioneer.htm
void IRsend::sendPioneer(uint64_t data, uint16_t nbits, uint16_t repeat) {
  // prepare codes
  uint64_t PioneerCode1 = data;
  uint64_t PioneerCode2;
  PioneerCode1 >>= nbits/2;  // 1st code
  PioneerCode2 = data & 0xffffffffUL;  // 2nd code

  // send 1st part of the code
  sendGeneric(kNecHdrMark, kNecHdrSpace,
              kNecBitMark, kNecOneSpace,
              kNecBitMark, kNecZeroSpace,
              kNecBitMark, kNecMinGap, kNecMinCommandLength,
              PioneerCode1, nbits/2, 38, true, 0,  // Repeats are handled later.
              33);

  // send space between the codes
  sendGeneric(kNecBitMark, kPioneerCodeSeparatorTicks,
              0, 0, 0, 0,  // No actual data sent.
              0, 0, 0,
              0, 0, 38, true, 0,  // Repeats are handled later.
              33);

  // send 2nd part of the code
  sendNEC(uint64_t PioneerCode2, uint16_t nbits/2, uint16_t repeat)
}
#endif
