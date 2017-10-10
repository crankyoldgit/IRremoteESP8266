#ifndef IRUTILS_H_
#define IRUTILS_H_

// Copyright 2017 David Conran

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <string>

uint64_t reverseBits(uint64_t input, uint16_t nbits);
std::string uint64ToString(uint64_t input, uint8_t base = 10);
void serialPrintUint64(uint64_t input, uint8_t base = 10);

#endif  // IRUTILS_H_
