// Copyright 2017 David Conran

#include "IRutils.h"
#include <Arduino.h>
#include <stdint.h>
#include <algorithm>

// Reverse the order of the requested least significant nr. of bits.
// Args:
//   input: Bit pattern/integer to reverse.
//   nbits: Nr. of bits to reverse.
// Returns:
//   The reversed bit pattern.
uint64_t reverseBits(uint64_t input, uint16_t nbits) {
  uint64_t output = input;
  for (uint16_t i = 1;
       i < std::min(nbits, (uint16_t) (sizeof(input) * 8));
       i++) {
    output <<= 1;
    input  >>= 1;
    output |= (input & 1);
  }
  return output;
}
