// Copyright 2009 Ken Shirriff
// Copyright 2015 Mark Szabo
// Copyright 2015 Sebastien Warin
// Copyright 2017 David Conran

#ifndef IRRECV_H_
#define IRRECV_H_

#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include <stddef.h>
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include "IRremoteESP8266.h"

// Constants
#define HEADER         2U  // Usual nr. of header entries.
#define FOOTER         2U  // Usual nr. of footer (stop bits) entries.
#define OFFSET_START   1U  // Usual rawbuf entry to start processing from.
// Marks tend to be 100us too long, and spaces 100us too short
// when received due to sensor lag.
#define MARK_EXCESS  100U
#define RAWBUF       100U  // Length of raw duration buffer
#define REPEAT UINT64_MAX
// receiver states
#define STATE_IDLE     2U
#define STATE_MARK     3U
#define STATE_SPACE    4U
#define STATE_STOP     5U
#define TOLERANCE     25U  // default percent tolerance in measurements
#define USECPERTICK   50U  // microseconds per clock interrupt tick
#define TIMEOUT_MS    15U  // How long before we give up wait for more data.

// Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
#define FNV_PRIME_32 16777619UL
#define FNV_BASIS_32 2166136261UL

// Types
// information for the interrupt handler
typedef struct {
  uint8_t recvpin;              // pin for IR data from detector
  uint8_t rcvstate;             // state machine
  uint16_t timer;               // state timer, counts 50uS ticks.
  uint16_t rawbuf[RAWBUF];      // raw data
  // uint16_t is used for rawlen as it saves 3 bytes of iram in the interrupt
  // handler. Don't ask why, I don't know. It just does.
  uint16_t rawlen;              // counter of entries in rawbuf.
  uint8_t overflow;             // Buffer overflow indicator.
} irparams_t;

// Classes

// Results returned from the decoder
class decode_results {
 public:
  decode_type_t decode_type;  // NEC, SONY, RC5, UNKNOWN
  uint64_t value;  // Decoded value
  uint16_t bits;  // Number of bits in decoded value
  volatile uint16_t *rawbuf;  // Raw intervals in .5 us ticks
  uint16_t rawlen;  // Number of records in rawbuf.
  bool overflow;
  bool repeat;  // Is the result a repeat code?
  uint32_t address;  // Decoded device address.
  uint32_t command;  // Decoded command.
};

// main class for receiving IR
class IRrecv {
 public:
  explicit IRrecv(uint16_t recvpin);
  bool decode(decode_results *results, irparams_t *save = NULL);
  void enableIRIn();
  void disableIRIn();
  void resume();

#ifndef UNIT_TEST

 private:
#endif
  // These are called by decode
  void copyIrParams(irparams_t *dest);
  int16_t compare(uint16_t oldval, uint16_t newval);
  uint32_t ticksLow(uint32_t usecs, uint8_t tolerance = TOLERANCE);
  uint32_t ticksHigh(uint32_t usecs, uint8_t tolerance = TOLERANCE);
  bool match(uint32_t measured_ticks, uint32_t desired_us,
             uint8_t tolerance = TOLERANCE);
  bool matchAtLeast(uint32_t measured_ticks, uint32_t desired_us,
                    uint8_t tolerance = TOLERANCE);
  bool matchMark(uint32_t measured_ticks, uint32_t desired_us,
                 uint8_t tolerance = TOLERANCE, int16_t excess = MARK_EXCESS);
  bool matchSpace(uint32_t measured_ticks, uint32_t desired_us,
                  uint8_t tolerance = TOLERANCE, int16_t excess = MARK_EXCESS);
  bool decodeHash(decode_results *results);
#if (DECODE_NEC || DECODE_SHERWOOD || DECODE_AIWA_RC_T501 || SEND_SANYO)
  bool decodeNEC(decode_results *results, uint16_t nbits = NEC_BITS,
                 bool strict = true);
#endif
#if DECODE_SONY
  bool decodeSony(decode_results *results, uint16_t nbits = SONY_MIN_BITS,
                  bool strict = false);
#endif
#if DECODE_SANYO
  // DISABLED due to poor quality.
  // bool decodeSanyo(decode_results *results,
  //                  uint16_t nbits = SANYO_SA8650B_BITS,
  //                  bool strict = false);
  bool decodeSanyoLC7461(decode_results *results,
                         uint16_t nbits = SANYO_LC7461_BITS,
                         bool strict = true);
#endif
#if DECODE_MITSUBISHI
  bool decodeMitsubishi(decode_results *results,
                        uint16_t nbits = MITSUBISHI_BITS,
                        bool strict = true);
#endif
#if (DECODE_RC5 || DECODE_R6)
  int16_t getRClevel(decode_results *results, uint16_t *offset, uint16_t *used,
                     uint16_t bitTime);
#endif
#if DECODE_RC5
  bool decodeRC5(decode_results *results, uint16_t nbits = RC5X_BITS,
                 bool strict = true);
#endif
#if DECODE_RC6
  bool decodeRC6(decode_results *results, uint16_t nbits = RC6_MODE0_BITS,
                 bool strict = false);
#endif
#if DECODE_RCMM
  bool decodeRCMM(decode_results *results, uint16_t nbits = RCMM_BITS,
                  bool strict = false);
#endif
#if (DECODE_PANASONIC || DECODE_DENON)
  bool decodePanasonic(decode_results *results, uint16_t nbits = PANASONIC_BITS,
                       bool strict = false,
                       uint32_t manufacturer = PANASONIC_MANUFACTURER);
#endif
#if DECODE_LG
  bool decodeLG(decode_results *results, uint16_t nbits = LG_BITS,
                bool strict = false);
#endif
#if DECODE_JVC
  bool decodeJVC(decode_results *results, uint16_t nbits = JVC_BITS,
                 bool strict = true);
#endif
#if DECODE_SAMSUNG
  bool decodeSAMSUNG(decode_results *results, uint16_t nbits = SAMSUNG_BITS,
                     bool strict = true);
#endif
#if DECODE_WHYNTER
  bool decodeWhynter(decode_results *results, uint16_t nbits = WHYNTER_BITS,
                     bool strict = true);
#endif
#if DECODE_COOLIX
  bool decodeCOOLIX(decode_results *results, uint16_t nbits = COOLIX_BITS,
                    bool strict = true);
#endif
#if DECODE_DENON
  bool decodeDenon(decode_results *results, uint16_t nbits = DENON_BITS,
                   bool strict = true);
#endif
#if DECODE_DISH
  bool decodeDISH(decode_results *results, uint16_t nbits = DISH_BITS,
                  bool strict = true);
#endif
#if (DECODE_SHARP || DECODE_DENON)
  bool decodeSharp(decode_results *results, uint16_t nbits = SHARP_BITS,
                   bool strict = true, bool expansion = true);
#endif
#if DECODE_AIWA_RC_T501
  bool decodeAiwaRCT501(decode_results *results,
                        uint16_t nbits = AIWA_RC_T501_BITS, bool strict = true);
#endif
};

#endif  // IRRECV_H_
