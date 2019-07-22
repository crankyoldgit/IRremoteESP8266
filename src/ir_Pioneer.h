// Copyright 2009 Ken Shirriff
// Copyright 2017, 2018 David Conran

// NEC originally added from https://github.com/shirriff/Arduino-IRremote/

#ifndef IR_PIONEER_H_
#define IR_PIONEER_H_

#include <stdint.h>
#include "IRremoteESP8266.h"

// Constants
// Ref:
//  http://www.adrian-kingston.com/IRFormatPioneer.htm
const uint16_t kPioneerTick = 534;
const uint16_t kPioneerHdrMarkTicks = 16;
const uint16_t kPioneerHdrMark = kPioneerHdrMarkTicks * kPioneerTick;
const uint16_t kPioneerHdrSpaceTicks = 8;
const uint16_t kPioneerHdrSpace = kPioneerHdrSpaceTicks * kPioneerTick;
const uint16_t kPioneerBitMarkTicks = 1;
const uint16_t kPioneerBitMark = kPioneerBitMarkTicks * kPioneerTick;
const uint16_t kPioneerOneSpaceTicks = 3;
const uint16_t kPioneerOneSpace = kPioneerOneSpaceTicks * kPioneerTick;
const uint16_t kPioneerZeroSpaceTicks = 1;
const uint16_t kPioneerZeroSpace = kPioneerZeroSpaceTicks * kPioneerTick;
const uint16_t kPioneerMinCommandLengthTicks = 159;
const uint32_t kPioneerMinCommandLength = kPioneerMinCommandLengthTicks * kPioneerTick;
const uint16_t kPioneerMinGapTicks = 47;
const uint32_t kPioneerMinGap = kPioneerMinGapTicks * kPioneerTick;

#endif  // IR_PIONEER_H_
