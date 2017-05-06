// Copyright 2009 Ken Shirriff
// Copyright 2015 Mark Szabo
// Copyright 2017 David Conran
#ifndef IRSEND_H_
#define IRSEND_H_

#include <stdint.h>
#include "IRremoteESP8266.h"

// Originally from https://github.com/shirriff/Arduino-IRremote/
// Updated by markszabo (https://github.com/markszabo/IRremoteESP8266) for
// sending IR code on ESP8266

#ifdef TEST
#define VIRTUAL virtual
#else
#define VIRTUAL
#endif

// Constants
// Offset (in microseconds) to use in Period time calculations to account for
// code excution time in producing the software PWM signal.
// Value determined in https://github.com/markszabo/IRremoteESP8266/issues/62
#define PERIOD_OFFSET -3
#define DUTY_DEFAULT 50

// Classes
class IRsend {
 public:
  explicit IRsend(uint16_t IRsendPin);
  void begin();
  void enableIROut(uint32_t freq, uint8_t duty = DUTY_DEFAULT);
  VIRTUAL uint16_t mark(uint16_t usec);
  VIRTUAL void space(uint32_t usec);
  void calibrate(uint16_t hz = 38000U);
  void sendRaw(uint16_t buf[], uint16_t len, uint16_t hz);
  void sendData(uint16_t onemark, uint32_t onespace, uint16_t zeromark,
                uint32_t zerospace, uint64_t data, uint16_t nbits,
                bool MSBfirst = true);
  void send(uint16_t type, uint64_t data, uint16_t nbits);
#if (SEND_NEC || SEND_SHERWOOD)
  void sendNEC(uint64_t data, uint16_t nbits = NEC_BITS, uint16_t repeat = 0);
  uint32_t encodeNEC(uint16_t address, uint16_t command);
#endif
#if SEND_SONY
  // sendSony() should typically be called with repeat=2 as Sony devices
  // expect the code to be sent at least 3 times. (code + 2 repeats = 3 codes)
  // Legacy use of this procedure was to only send a single code so call it with
  // repeat=0 for backward compatiblity. As of v2.0 it defaults to sending
  // a Sony command that will be accepted be a device.
  void sendSony(uint64_t data, uint16_t nbits = SONY_20_BITS,
                uint16_t repeat = SONY_MIN_REPEAT);
  uint32_t encodeSony(uint16_t nbits, uint16_t command, uint16_t address,
                      uint16_t extended = 0);
#endif
#if SEND_SHERWOOD
  void sendSherwood(uint64_t data, uint16_t nbits = SHERWOOD_BITS,
                    uint16_t repeat = SHERWOOD_MIN_REPEAT);
#endif
#if SEND_SAMSUNG
  void sendSAMSUNG(uint64_t data, uint16_t nbits = SAMSUNG_BITS,
                   uint16_t repeat = 0);
  uint32_t encodeSAMSUNG(uint8_t customer, uint8_t command);
#endif
#if SEND_LG
  void sendLG(uint64_t data, uint16_t nbits = LG_BITS, uint16_t repeat = 0);
  uint32_t encodeLG(uint8_t address, uint16_t command);
#endif
#if SEND_SHARP
  uint32_t encodeSharp(uint16_t address, uint16_t command,
                       uint16_t expansion = 1, uint16_t check = 0,
                       bool MSBfirst = false);
  void sendSharp(uint16_t address, uint16_t command,
                 uint16_t nbits = SHARP_BITS, uint16_t repeat = 0);
  void sendSharpRaw(uint64_t data, uint16_t nbits = SHARP_BITS,
                    uint16_t repeat = 0);
#endif
#if SEND_JVC
  void sendJVC(uint64_t data, uint16_t nbits = JVC_BITS, uint16_t repeat = 0);
  uint16_t encodeJVC(uint8_t address, uint8_t command);
#endif
#if SEND_DENON
  void sendDenon(uint64_t data, uint16_t nbits = DENON_BITS,
                 uint16_t repeat = 0);
#endif
#if SEND_SANYO
  uint64_t encodeSanyoLC7461(uint16_t address, uint16_t command);
  void sendSanyoLC7461(uint64_t data, uint16_t nbits = SANYO_LC7461_BITS,
                       uint16_t repeat = 0);
#endif
#if SEND_DISH
  // sendDISH() should typically be called with repeat=3 as DISH devices
  // expect the code to be sent at least 4 times. (code + 3 repeats = 4 codes)
  // Legacy use of this procedure was only to send a single code
  // so use repeat=0 for backward compatiblity.
  void sendDISH(uint64_t data, uint16_t nbits = DISH_BITS,
                uint16_t repeat = DISH_MIN_REPEAT);
#endif
#if SEND_PANASONIC
  void sendPanasonic64(uint64_t data, uint16_t nbits = PANASONIC_BITS,
                       uint16_t repeat = 0);
  void sendPanasonic(uint16_t address, uint32_t data,
                     uint16_t nbits = PANASONIC_BITS, uint16_t repeat = 0);
  uint64_t encodePanasonic(uint8_t device, uint8_t subdevice, uint8_t function);
#endif
#if SEND_RC5
  void sendRC5(uint64_t data, uint16_t nbits = RC5X_BITS, uint16_t repeat = 0);
#endif
#if SEND_RC6
  void sendRC6(uint64_t data, uint16_t nbits = RC6_MODE0_BITS,
               uint16_t repeat = 0);
#endif
#if SEND_RCMM
  void sendRCMM(uint32_t data, uint8_t nbits = RCMM_BITS);
#endif
#if SEND_COOLIX
  void sendCOOLIX(uint64_t data, uint16_t nbits = COOLIX_BITS,
                  uint16_t repeat = 0);
#endif
#if SEND_WHYNTER
  void sendWhynter(uint64_t data, uint16_t nbits = WHYNTER_BITS,
                   uint16_t repeat = 0);
#endif
#if SEND_MITSUBISHI
  void sendMitsubishi(uint64_t data, uint16_t nbits = MITSUBISHI_BITS,
                      uint16_t repeat = MITSUBISHI_MIN_REPEAT);
#endif
#if SEND_MITSUBISHI_AC
  void sendMitsubishiAC(unsigned char data[]);
#endif
#if SEND_GLOBALCACHE
  void sendGC(uint16_t buf[], uint16_t len);
#endif
#if SEND_KELVINATOR
  void sendKelvinator(unsigned char data[]);
#endif
#if SEND_DAIKIN
  void sendDaikin(unsigned char data[]);
#endif

 private:
  uint16_t onTimePeriod;
  uint16_t offTimePeriod;
  uint16_t IRpin;
  int8_t periodOffset;
  void ledOff();
  uint32_t calcUSecPeriod(uint32_t hz);
};

#endif  // IRSEND_H_
