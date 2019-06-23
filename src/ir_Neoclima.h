// Neoclima A/C
//
// Copyright 2019 David Conran

#ifndef IR_NEOCLIMA_H_
#define IR_NEOCLIMA_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include "IRremoteESP8266.h"
#include "IRsend.h"
#ifdef UNIT_TEST
#include "IRsend_test.h"
#endif

// Supports:
//   Brand: Neoclima,  Model: NS-09AHTI A/C
//   Brand: Neoclima,  Model: ZH/TY-01 remote

// Ref:
//  https://github.com/markszabo/IRremoteESP8266/issues/764

// Constants
const uint8_t kNeoclimaMinTemp = 16;   // 16C
const uint8_t kNeoclimaMaxTemp = 32;   // 32C
const uint8_t kNeoclimaTempMask = 0x3F;        // 0b00111111

// Classes
class IRNeoclimaAc {
 public:
  explicit IRNeoclimaAc(const uint16_t pin);

  void stateReset(void);
#if SEND_NEOCLIMA
  void send(const uint16_t repeat = kNeoclimaMinRepeat);
  uint8_t calibrate(void) { return _irsend.calibrate(); }
#endif  // SEND_NEOCLIMA
  void begin(void);
  void setTemp(const uint8_t temp);
  uint8_t getTemp(void);
  uint8_t* getRaw(void);
  void setRaw(const uint8_t new_code[],
              const uint16_t length = kNeoclimaStateLength);
  static bool validChecksum(const uint8_t state[],
                            const uint16_t length = kNeoclimaStateLength);
  static uint8_t calcChecksum(const uint8_t state[],
                              const uint16_t length = kNeoclimaStateLength);
  String toString(void);
#ifndef UNIT_TEST

 private:
  IRsend _irsend;
#else
  IRsendTest _irsend;
#endif
  // The state of the IR remote in IR code form.
  uint8_t remote_state[kNeoclimaStateLength];
  void checksum(const uint16_t length = kNeoclimaStateLength);
};

#endif  // IR_NEOCLIMA_H_
