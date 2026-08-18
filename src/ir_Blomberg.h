#ifndef IR_BLOMBERG_H_
#define IR_BLOMBERG_H_

#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "IRrecv.h"

// Constants
const uint16_t kBlombergStateLength = 15;
const uint8_t kBlombergMinTemp = 19;
const uint8_t kBlombergMaxTemp = 30;

// Operation Modes (Upper nibble of Byte 4)
const uint8_t kBlombergModeHeat = 0x10; // 16
const uint8_t kBlombergModeCool = 0x20; // 32
const uint8_t kBlombergModeDry  = 0x30; // 48
const uint8_t kBlombergModeAuto = 0x40; // 64
const uint8_t kBlombergModeFan  = 0x50; // 80

// Fan Speeds (Lower nibble of Byte 4)
const uint8_t kBlombergFanAuto = 0x00; // 0
const uint8_t kBlombergFanHigh = 0x01; // 1
const uint8_t kBlombergFanLow  = 0x02; // 2
const uint8_t kBlombergFanMed  = 0x03; // 3

class IRBlombergAc {
 public:
  explicit IRBlombergAc(const uint16_t pin, const bool inverted = false,
                        const bool use_modulation = true);
  void stateReset();
  void begin();
  void send(const uint16_t repeat = 0);

  uint8_t* getRaw();
  void setRaw(const uint8_t state[]);
  static uint8_t calcChecksum(const uint8_t state[], 
                              const uint16_t length = kBlombergStateLength);
  static bool validChecksum(const uint8_t state[], 
                            const uint16_t length = kBlombergStateLength);
  void checksum();

  void setPower(const bool on);
  bool getPower() const;

  void setMode(const uint8_t mode);
  uint8_t getMode() const;

  void setTemp(const uint8_t temp);
  uint8_t getTemp() const;

  void setFan(const uint8_t fan);
  uint8_t getFan() const;

  void setSwing(const bool on);
  bool getSwing() const;

  void setLed(const bool on);
  bool getLed() const;

  void setTurbo(const bool on);
  bool getTurbo() const;

 private:
  IRsend _irsend;
  uint8_t remote_state[kBlombergStateLength];
};

#endif  // IR_BLOMBERG_H_
