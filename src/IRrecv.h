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
#define MS_TO_USEC(x)  (x * 1000U)  // Convert milli-Seconds to micro-Seconds.
// Marks tend to be 100us too long, and spaces 100us too short
// when received due to sensor lag.
#define MARK_EXCESS   50U
#define RAWBUF       100U  // Default length of raw capture buffer
#define REPEAT UINT64_MAX
#define UNKNOWN_THRESHOLD 6U  // Default min size of reported UNKNOWN messages.
// receiver states
#define STATE_IDLE     2U
#define STATE_MARK     3U
#define STATE_SPACE    4U
#define STATE_STOP     5U
#define TOLERANCE     25U  // default percent tolerance in measurements
#define RAWTICK        2U  // Capture tick to uSec factor.
// How long (ms) before we give up wait for more data?
// Don't exceed MAX_TIMEOUT_MS without a good reason.
// That is the capture buffers maximum value size. (UINT16_MAX / RAWTICK)
// Typically messages/protocols tend to repeat around the 100ms timeframe,
// thus we should timeout before that to give us some time to try to decode
// before we need to start capturing a possible new message.
// Typically 15ms suits most applications. However, some protocols demand a
// higher value. e.g. 90ms for XMP-1 and some aircon units.
#define TIMEOUT_MS    15U  // In MilliSeconds.
#define MAX_TIMEOUT_MS (RAWTICK * UINT16_MAX / MS_TO_USEC(1))

// Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
#define FNV_PRIME_32 16777619UL
#define FNV_BASIS_32 2166136261UL

#define MAX2(a, b) ((a > b)?(a):(b))
#define MAX4(a, b, c, d) MAX2(MAX2(a, b), MAX2(c, d))
#define STATE_SIZE_MAX MAX2(MAX4(ARGO_COMMAND_LENGTH, \
                                 TROTEC_COMMAND_LENGTH, \
                                 MITSUBISHI_AC_STATE_LENGTH, \
                                 KELVINATOR_STATE_LENGTH), \
                            MAX4(GREE_STATE_LENGTH, \
                                 DAIKIN_COMMAND_LENGTH, \
                                 TOSHIBA_AC_STATE_LENGTH, \
                                 FUJITSU_AC_STATE_LENGTH))

// Types
// information for the interrupt handler
typedef struct {
  uint8_t recvpin;              // pin for IR data from detector
  uint8_t rcvstate;             // state machine
  uint16_t timer;               // state timer, counts 50uS ticks.
  uint16_t bufsize;             // max. nr. of entries in the capture buffer.
  uint16_t *rawbuf;             // raw data
  // uint16_t is used for rawlen as it saves 3 bytes of iram in the interrupt
  // handler. Don't ask why, I don't know. It just does.
  uint16_t rawlen;              // counter of entries in rawbuf.
  uint8_t overflow;             // Buffer overflow indicator.
  uint8_t timeout;              // Nr. of milliSeconds before we give up.
} irparams_t;

// results from a data match
typedef struct {
  bool success;  // Was the match successful?
  uint64_t data;  // The data found.
  uint16_t used;  // How many buffer positions were used.
} match_result_t;

// Classes

// Results returned from the decoder
class decode_results {
 public:
  decode_type_t decode_type;  // NEC, SONY, RC5, UNKNOWN
  // value, address, & command are all mutually exclusive with state.
  // i.e. They MUST NOT be used at the same time as state, so we can use a union
  // structure to save us a handful of valuable bytes of memory.
  union {
    struct {
      uint64_t value;  // Decoded value
      uint32_t address;  // Decoded device address.
      uint32_t command;  // Decoded command.
    };
#if DECODE_AC  // Only include state if we must. It's big.
    uint8_t state[STATE_SIZE_MAX];  // Complex multi-byte A/C result.
#endif
  };
  uint16_t bits;  // Number of bits in decoded value
  volatile uint16_t *rawbuf;  // Raw intervals in .5 us ticks
  uint16_t rawlen;  // Number of records in rawbuf.
  bool overflow;
  bool repeat;  // Is the result a repeat code?
};

// main class for receiving IR
class IRrecv {
 public:
  explicit IRrecv(uint16_t recvpin, uint16_t bufsize = RAWBUF,
                  uint8_t timeout = TIMEOUT_MS,
                  bool save_buffer = false);  // Constructor
  ~IRrecv();  // Destructor
  bool decode(decode_results *results, irparams_t *save = NULL);
  void enableIRIn();
  void disableIRIn();
  void resume();
  uint16_t getBufSize();
#if DECODE_HASH
  void setUnknownThreshold(uint16_t length);
#endif
  static bool match(uint32_t measured, uint32_t desired,
             uint8_t tolerance = TOLERANCE, uint16_t delta = 0);
  static bool matchMark(uint32_t measured, uint32_t desired,
                 uint8_t tolerance = TOLERANCE, int16_t excess = MARK_EXCESS);
  static bool matchSpace(uint32_t measured, uint32_t desired,
                  uint8_t tolerance = TOLERANCE, int16_t excess = MARK_EXCESS);
#ifndef UNIT_TEST

 private:
#endif
  irparams_t *irparams_save;
#if DECODE_HASH
  uint16_t unknown_threshold;
#endif
  // These are called by decode
  void copyIrParams(volatile irparams_t *src, irparams_t *dst);
  int16_t compare(uint16_t oldval, uint16_t newval);
  static uint32_t ticksLow(uint32_t usecs, uint8_t tolerance = TOLERANCE,
                           uint16_t delta = 0);
  static uint32_t ticksHigh(uint32_t usecs, uint8_t tolerance = TOLERANCE,
                           uint16_t delta = 0);
  bool matchAtLeast(uint32_t measured, uint32_t desired,
                    uint8_t tolerance = TOLERANCE, uint16_t delta = 0);
  match_result_t matchData(volatile uint16_t *data_ptr, const uint16_t nbits,
                           const uint16_t onemark, const uint32_t onespace,
                           const uint16_t zeromark, const uint32_t zerospace,
                           const uint8_t tolerance = TOLERANCE);
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
#if (DECODE_RC5 || DECODE_R6 || DECODE_LASERTAG)
  int16_t getRClevel(decode_results *results, uint16_t *offset, uint16_t *used,
                     uint16_t bitTime, uint8_t tolerance = TOLERANCE,
                     int16_t excess = MARK_EXCESS, uint16_t delta = 0);
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
#if DECODE_NIKAI
  bool decodeNikai(decode_results *results, uint16_t nbits = NIKAI_BITS,
                   bool strict = true);
#endif
#if DECODE_MAGIQUEST
  bool decodeMagiQuest(decode_results *results, uint16_t nbits = MAGIQUEST_BITS,
                       bool strict = true);
#endif
#if DECODE_KELVINATOR
  bool decodeKelvinator(decode_results *results,
                        uint16_t nbits = KELVINATOR_BITS,
                        bool strict = true);
#endif
#if DECODE_DAIKIN
  bool decodeDaikin(decode_results *results, uint16_t nbits = DAIKIN_RAW_BITS,
                    bool strict = true);
#endif
#if DECODE_TOSHIBA_AC
  bool decodeToshibaAC(decode_results *results,
                       uint16_t nbytes = TOSHIBA_AC_BITS,
                       bool strict = true);
#endif
#if DECODE_MIDEA
  bool decodeMidea(decode_results *results, uint16_t nbits = MIDEA_BITS,
                   bool strict = true);
#endif
#if DECODE_FUJITSU_AC
  bool decodeFujitsuAC(decode_results *results,
                       uint16_t nbits = FUJITSU_AC_BITS,
                       bool strict = false);
#endif
#if DECODE_LASERTAG
  bool decodeLasertag(decode_results *results, uint16_t nbits = LASERTAG_BITS,
                      bool strict = true);
#endif
};

#endif  // IRRECV_H_
