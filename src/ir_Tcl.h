// Copyright 2019 David Conran

#ifndef IR_TCL_H_
#define IR_TCL_H_

#include "IRremoteESP8266.h"
#include "IRsend.h"

// Constants
const uint16_t kTcl112AcHdrMark = 3000;
const uint16_t kTcl112AcHdrSpace = 1650;
const uint16_t kTcl112AcBitMark = 500;
const uint16_t kTcl112AcOneSpace = 1050;
const uint16_t kTcl112AcZeroSpace = 325;
const uint32_t kTcl112AcGap = 100000;  // Just a guess.

class IRTcl112Ac {
 public:
  explicit IRTcl112Ac(uint16_t pin);

#if SEND_TCL112AC
  void send(const uint16_t repeat = kTcl112AcDefaultRepeat);
#endif  // SEND_TCL
  void begin(void);
  uint8_t* getRaw(void);
  void setRaw(const uint8_t new_code[],
              const uint16_t length = kTcl112AcStateLength);
  void setTemp(const float temp);  // Celsius in 0.5 increments
  float getTemp(void);
#ifdef ARDUINO
  String toString();
#else
  std::string toString();
#endif

 private:
  uint8_t remote_state[kTcl112AcStateLength];
  void stateReset();
  void checksum();
  IRsend _irsend;
};

#endif  // IR_TCL_H_
