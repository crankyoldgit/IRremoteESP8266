// Copyright 2018 David Conran

#ifndef IR_PANASONIC_H_
#define IR_PANASONIC_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#ifdef ARDUINO
#include <Arduino.h>
#else
#include <string>
#endif
#include "IRremoteESP8266.h"
#include "IRsend.h"

//       PPPP    AAA   N   N   AAA    SSSS   OOO   N   N  IIIII   CCCC
//       P   P  A   A  NN  N  A   A  S      O   O  NN  N    I    C
//       PPPP   AAAAA  N N N  AAAAA   SSS   O   O  N N N    I    C
//       P      A   A  N  NN  A   A      S  O   O  N  NN    I    C
//       P      A   A  N   N  A   A  SSSS    OOO   N   N  IIIII   CCCC

// Panasonic A/C support heavily influenced by:
//   https://github.com/ToniA/ESPEasy/blob/HeatpumpIR/lib/HeatpumpIR/PanasonicHeatpumpIR.cpp

// Constants
const uint16_t kPanasonicFreq = 36700;

const uint8_t kPanasonicAcAuto = 0;  // 0b0000
const uint8_t kPanasonicAcDry = 2;   // 0b0010
const uint8_t kPanasonicAcCool = 3;  // 0b0011
const uint8_t kPanasonicAcHeat = 4;  // 0b0010
const uint8_t kPanasonicAcFan = 6;   // 0b0110
const uint8_t kPanasonicAcFanMin = 0;
const uint8_t kPanasonicAcFanMax = 4;
const uint8_t kPanasonicAcFanAuto = 7;
const uint8_t kPanasonicAcFanOffset = 3;
const uint8_t kPanasonicAcPower = 1;  // 0b1
const uint8_t kPanasonicAcMinTemp = 16;  // Celsius
const uint8_t kPanasonicAcMaxTemp = 30;  // Celsius
const uint8_t kPanasonicAcQuiet = 1;  // 0b1
const uint8_t kPanasonicAcPowerful = 0x20;  // 0b100000
const uint8_t kPanasonicAcSwingVAuto = 0xF;
const uint8_t kPanasonicAcSwingVUp = 0x1;
const uint8_t kPanasonicAcSwingVDown = 0x5;
const uint8_t kPanasonicAcSwingHAuto = 0xD;
const uint8_t kPanasonicAcSwingHMiddle = 0x6;
const uint8_t kPanasonicAcSwingHFullLeft = 0x9;
const uint8_t kPanasonicAcSwingHLeft = 0xA;
const uint8_t kPanasonicAcSwingHRight = 0xB;
const uint8_t kPanasonicAcSwingHFullRight = 0xC;
const uint8_t kPanasonicAcChecksumInit = 0xF4;
const uint8_t kPanasonicKnownGoodState[kPanasonicAcStateLength] = {
  0x02, 0x20, 0xE0, 0x04, 0x00, 0x00, 0x00, 0x06,
  0x02, 0x20, 0xE0, 0x04, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
  0x00, 0x0E, 0xE0, 0x00, 0x00, 0x81, 0x00, 0x00, 0x00};

enum panasonic_ac_remote_model_t {
  kPanasonicUnknown = 0,
  kPanasonicLke = 1,
  kPanasonicNke = 2,
  kPanasonicDke = 3,
  kPanasonicJke = 4,
};


class IRPanasonicAc {
 public:
  explicit IRPanasonicAc(uint16_t pin);

  void stateReset();
#if SEND_PANASONIC
  void send();
#endif  // SEND_PANASONIC
  void begin();
  void on();
  void off();
  void setPower(const bool state);
  bool getPower();
  void setTemp(const uint8_t temp);
  uint8_t getTemp();
  void setFan(const uint8_t fan);
  uint8_t getFan();
  void setMode(const uint8_t mode);
  uint8_t getMode();
  void setRaw(const uint8_t state[]);
  uint8_t* getRaw();
  static bool validChecksum(uint8_t *state,
                            const uint16_t length = kPanasonicAcStateLength);
  static uint8_t calcChecksum(uint8_t *state,
                               const uint16_t length = kPanasonicAcStateLength);
  void setQuiet(const bool state);
  bool getQuiet();
  void setPowerful(const bool state);
  bool getPowerful();
  void setModel(const panasonic_ac_remote_model_t model);
  panasonic_ac_remote_model_t getModel();
  void setSwingV(const uint8_t elevation);
  uint8_t getSwingVertical();
  void setSwingH(const uint8_t direction);
  uint8_t getSwingHorizontal();

#ifdef ARDUINO
  String toString();
#else
  std::string toString();
#endif
#ifndef UNIT_TEST

 private:
#endif
  uint8_t remote_state[kPanasonicAcStateLength];
  uint8_t _swingh = kPanasonicAcSwingHMiddle;
  void fixChecksum(const uint16_t length = kPanasonicAcStateLength);
  static uint8_t calcChecksum(const uint8_t *state,
                              const uint16_t length = kPanasonicAcStateLength);
  IRsend _irsend;
};

#endif  // IR_PANASONIC_H_
