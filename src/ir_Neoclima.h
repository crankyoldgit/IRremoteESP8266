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
// state[3]
const uint8_t kNeoclimaHoldMask = 0b00000100;
// state[5]
const uint8_t kNeoclimaButtonPower = 0x00;
const uint8_t kNeoclimaButtonMode = 0x01;
const uint8_t kNeoclimaButtonSwing = 0x04;
const uint8_t kNeoclimaButtonTempUp = 0x02;
const uint8_t kNeoclimaButtonTempDown = 0x03;
const uint8_t kNeoclimaButtonFanSpeed = 0x05;
const uint8_t kNeoclimaButtonAirFlow = 0x07;
const uint8_t kNeoclimaButtonHold = 0x08;
const uint8_t kNeoclimaButtonSleep = 0x09;
const uint8_t kNeoclimaButtonLight = 0x0B;
const uint8_t kNeoclimaButtonEye = 0x0E;
const uint8_t kNeoclimaButtonFollow = 0x13;
const uint8_t kNeoclimaButtonIon = 0x14;
const uint8_t kNeoclimaButton8DegHeat = 0x1D;
const uint8_t kNeoclimaButtonTurbo = 0x6A;
// state[7]
const uint8_t kNeoclimaFanMask =   0b11000000;
const uint8_t kNeoclimaFanAuto =   0b00;
const uint8_t kNeoclimaFanLow =    0b01;
const uint8_t kNeoclimaFanMed =    0b10;
const uint8_t kNeoclimaFanHigh =   0b11;
const uint8_t kNeoclimaPowerMask = 0b00000010;
// state[9]
const uint8_t kNeoclimaModeMask = 0b11100000;
const uint8_t kNeoclimaAuto = 0b000;
const uint8_t kNeoclimaCool = 0b001;
const uint8_t kNeoclimaDry =  0b010;
const uint8_t kNeoclimaFan =  0b011;
const uint8_t kNeoclimaHeat = 0b100;
const uint8_t kNeoclimaMinTemp = 16;   // 16C
const uint8_t kNeoclimaMaxTemp = 32;   // 32C
const uint8_t kNeoclimaTempMask = 0b00011111;

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
  void on(void);
  void off(void);
  void setPower(const bool on);
  bool getPower(void);
  void setMode(const uint8_t mode);
  uint8_t getMode(void);
  void setTemp(const uint8_t temp);
  uint8_t getTemp(void);
  void setFan(const uint8_t speed);
  uint8_t getFan(void);
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
