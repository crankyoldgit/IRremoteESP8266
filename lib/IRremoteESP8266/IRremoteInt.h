 /***************************************************
 * IRremote for ESP8266
 *
 * Based on the IRremote library for Arduino by Ken Shirriff
 * Version 0.11 August, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 * Modified by Paul Stoffregen <paul@pjrc.com> to support other boards and timers
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 *
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 * Whynter A/C ARC-110WD added by Francesco Meschia
 * Coolix A/C / heatpump added by (send) bakrus & (decode) crankyoldgit
 * Denon: sendDenon, decodeDenon added by Massimiliano Pinto
          (from https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Denon.cpp)
 * Kelvinator A/C added by crankyoldgit
 * Mitsubishi (TV) sending added by crankyoldgit
 * Mitsubishi A/C added by crankyoldgit
 *     (derived from https://github.com/r45635/HVAC-IR-Control)
 * DISH decode by marcosamarinho
 *
 * 09/23/2015 : Samsung pulse parameters updated by Sebastien Warin to be compatible with EUxxD6200
 *
 *  GPL license, all text above must be included in any redistribution
 ****************************************************/

#ifndef IRREMOTEINT_H_
#define IRREMOTEINT_H_

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

// Pulse parms are *50-100 for the Mark and *50+100 for the space
// First MARK is the one after the long gap
// pulse parameters in usec
#define COOLIX_BIT_MARK   560U  // Approximately 21 cycles at 38kHz
#define COOLIX_ONE_SPACE  COOLIX_BIT_MARK * 3U
#define COOLIX_ZERO_SPACE COOLIX_BIT_MARK * 1U
#define COOLIX_HDR_MARK   COOLIX_BIT_MARK * 8U
#define COOLIX_HDR_SPACE  COOLIX_BIT_MARK * 8U
#define COOLIX_MIN_GAP    COOLIX_HDR_SPACE + COOLIX_ZERO_SPACE

#define WHYNTER_BITS                   32U
#define WHYNTER_HDR_MARK             2850U
#define WHYNTER_HDR_SPACE            2850U
#define WHYNTER_BIT_MARK              750U
#define WHYNTER_ONE_SPACE            2150U
#define WHYNTER_ZERO_SPACE            750U
#define WHYNTER_MIN_COMMAND_LENGTH 108000UL  // Completely made up value.
#define WHYNTER_MIN_GAP WHYNTER_MIN_COMMAND_LENGTH - \
    (2 * (WHYNTER_BIT_MARK + WHYNTER_ZERO_SPACE) + \
     WHYNTER_BITS * (WHYNTER_BIT_MARK + WHYNTER_ONE_SPACE))

#define NEC_HDR_MARK             9000U
#define NEC_HDR_SPACE            4500U
#define NEC_BIT_MARK              560U
#define NEC_ONE_SPACE            1690U
#define NEC_ZERO_SPACE            560U
#define NEC_RPT_SPACE            2250U
#define NEC_RPT_LENGTH              4U
#define NEC_MIN_COMMAND_LENGTH 108000UL
#define NEC_MIN_GAP NEC_MIN_COMMAND_LENGTH - \
    (NEC_HDR_MARK + NEC_HDR_SPACE + NEC_BITS * (NEC_BIT_MARK + NEC_ONE_SPACE) \
     + NEC_BIT_MARK)

#define SHERWOOD_MIN_REPEAT 1U

// Ref:
//   http://www.sbprojects.com/knowledge/ir/sirc.php
#define SONY_HDR_MARK         2400U
#define SONY_SPACE             600U
#define SONY_ONE_MARK   1200U + 50U  // Experiments suggest +50 is better.
#define SONY_ZERO_MARK   600U + 50U  // Experiments suggest +50 is better.
#define SONY_RPT_LENGTH      45000U
#define SONY_MIN_GAP         10000U
#define SONY_12_BITS            12U
#define SONY_15_BITS            15U
#define SONY_20_BITS            20U
#define SONY_MIN_REPEAT          2U

// Sanyo SA 8650B
// Ref:
//   https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Sanyo.cpp
#define SANYO_SA8650B_HDR_MARK          3500U  // seen range 3500
#define SANYO_SA8650B_HDR_SPACE          950U  // seen 950
#define SANYO_SA8650B_ONE_MARK          2400U  // seen 2400
#define SANYO_SA8650B_ZERO_MARK          700U  // seen 700
// usually see 713 - not using ticks as get number wrapround
#define SANYO_SA8650B_DOUBLE_SPACE_USECS 800U
#define SANYO_SA8650B_RPT_LENGTH       45000U

// Sanyo LC7461
// Ref:
//   https://github.com/marcosamarinho/IRremoteESP8266/blob/master/ir_Sanyo.cpp
//   http://slydiman.narod.ru/scr/kb/sanyo.htm
//   http://pdf.datasheetcatalog.com/datasheet/sanyo/LC7461.pdf
#define SANYO_LC7461_ADDRESS_BITS           13U
#define SANYO_LC7461_COMMAND_BITS            8U
#define SANYO_LC7461_BITS ((SANYO_LC7461_ADDRESS_BITS + \
                            SANYO_LC7461_COMMAND_BITS) * 2)
#define SANYO_LC7461_ADDRESS_MASK ((1 << SANYO_LC7461_ADDRESS_BITS) - 1)
#define SANYO_LC7461_COMMAND_MASK ((1 << SANYO_LC7461_COMMAND_BITS) - 1)
#define SANYO_LC7461_HDR_MARK             9000U
#define SANYO_LC7461_HDR_SPACE            4500U
#define SANYO_LC7461_BIT_MARK              560U  // 1T
#define SANYO_LC7461_ONE_SPACE            1690U  // 3T
#define SANYO_LC7461_ZERO_SPACE            560U  // 1T
#define SANYO_LC7461_MIN_COMMAND_LENGTH 108000UL
#define SANYO_LC7461_MIN_GAP SANYO_LC7461_MIN_COMMAND_LENGTH - \
    (SANYO_LC7461_HDR_MARK + SANYO_LC7461_HDR_SPACE + SANYO_LC7461_BITS * \
     (SANYO_LC7461_BIT_MARK + (SANYO_LC7461_ONE_SPACE + \
                               SANYO_LC7461_ZERO_SPACE) / 2) \
     + SANYO_LC7461_BIT_MARK)

// Mitsubishi period time is 1/33000Hz = 30.303 uSeconds (T)
// Ref:
//   GlobalCache's Control Tower's Mitsubishi TV data.
//   https://github.com/marcosamarinho/IRremoteESP8266/blob/master/ir_Mitsubishi.cpp
#define MITSUBISHI_BIT_MARK             303U  // T * 10
#define MITSUBISHI_ONE_SPACE           2121U  // T * 70
#define MITSUBISHI_ZERO_SPACE           909U  // T * 30
#define MITSUBISHI_MIN_COMMAND_LENGTH 54121U  // T * 1786
#define MITSUBISHI_MIN_GAP            28364U  // T * 936
// TODO(anyone): Verify that the repeat is really needed.
#define MITSUBISHI_MIN_REPEAT             1U  // Based on marcosamarinho's code.

// Mitsubishi A/C
// Ref:
//   https://github.com/r45635/HVAC-IR-Control/blob/master/HVAC_ESP8266/HVAC_ESP8266.ino#L84
#define MITSUBISHI_AC_HDR_MARK    3400U
#define MITSUBISHI_AC_HDR_SPACE   1750U
#define MITSUBISHI_AC_BIT_MARK     450U
#define MITSUBISHI_AC_ONE_SPACE   1300U
#define MITSUBISHI_AC_ZERO_SPACE   420U
#define MITSUBISHI_AC_RPT_MARK     440U
#define MITSUBISHI_AC_RPT_SPACE  17100UL

// Ref:
//   https://en.wikipedia.org/wiki/RC-5
//   http://www.sbprojects.com/knowledge/ir/rc5.php
#define RC5_RAW_BITS               14U
#define RC5_T1                    889U
#define RC5_MIN_COMMAND_LENGTH 113778UL
#define RC5_MIN_GAP RC5_MIN_COMMAND_LENGTH - RC5_RAW_BITS * (2 * RC5_T1)

// Ref:
//   https://en.wikipedia.org/wiki/RC-6
//   http://www.pcbheaven.com/userpages/The_Philips_RC6_Protocol/
#define RC6_HDR_MARK             2666U
#define RC6_HDR_SPACE             889U
#define RC6_T1                    444U
#define RC6_RPT_LENGTH          83000UL

// Ref:
//   http://www.sbprojects.com/knowledge/ir/rcmm.php
#define RCMM_HDR_MARK      416U
#define RCMM_HDR_SPACE     277U
#define RCMM_BIT_MARK      166U
#define RCMM_BIT_SPACE_0   277U
#define RCMM_BIT_SPACE_1   444U
#define RCMM_BIT_SPACE_2   611U
#define RCMM_BIT_SPACE_3   777U
#define RCMM_RPT_LENGTH  27778U
#define RCMM_MIN_GAP      3360U

// Use a tolerance of +/-10% when matching some data spaces.
#define RCMM_TOLERANCE      10U
#define RCMM_EXCESS         50U

// Sharp period time = 1/38000Hz = 26.316 microseconds.
// Ref:
//   GlobalCache's IR Control Tower data.
//   http://www.sbprojects.com/knowledge/ir/sharp.php
#define SHARP_ADDRESS_BITS     5U
#define SHARP_COMMAND_BITS     8U
#define SHARP_BIT_MARK       316U  // 12 * T
#define SHARP_ONE_SPACE     1684U  // 64 * T
#define SHARP_ZERO_SPACE     684U  // 26 * T
#define SHARP_GAP          43606U  // 1657 * T
// Address(5) + Command(8) + Expansion(1) + Check(1)
#define SHARP_BITS         SHARP_ADDRESS_BITS + SHARP_COMMAND_BITS + 2
#define SHARP_TOGGLE_MASK  ((1 << (SHARP_BITS - SHARP_ADDRESS_BITS)) - 1)
#define SHARP_ADDRESS_MASK ((1 << SHARP_ADDRESS_BITS) - 1)
#define SHARP_COMMAND_MASK ((1 << SHARP_COMMAND_BITS) - 1)

// Ref:
//   https://github.com/marcosamarinho/IRremoteESP8266/blob/master/ir_Dish.cpp
//   http://www.hifi-remote.com/wiki/index.php?title=Dish
#define DISH_HDR_MARK    400U
#define DISH_HDR_SPACE  6100U
#define DISH_BIT_MARK    400U
#define DISH_ONE_SPACE  1700U
#define DISH_ZERO_SPACE 2800U
#define DISH_RPT_SPACE  6200U
#define DISH_MIN_REPEAT    3U

// Ref:
//   http://www.remotecentral.com/cgi-bin/mboard/rc-pronto/thread.cgi?26152
#define PANASONIC_HDR_MARK             3456U
#define PANASONIC_HDR_SPACE            1728U
#define PANASONIC_BIT_MARK              432U
#define PANASONIC_ONE_SPACE            1296U
#define PANASONIC_ZERO_SPACE            432U
#define PANASONIC_MIN_COMMAND_LENGTH 130000UL
// 130000 - 3456 - 1728 - 48*(432+1296) - 432 = 41440
#define PANASONIC_MIN_GAP             41440U
#define PANASONIC_MANUFACTURER       0x2002ULL


// Ref:
//   http://www.sbprojects.com/knowledge/ir/jvc.php
#define JVC_HDR_MARK    8400U
#define JVC_HDR_SPACE   4200U
#define JVC_BIT_MARK     525U
#define JVC_ONE_SPACE   1725U
#define JVC_ZERO_SPACE   525U
#define JVC_RPT_LENGTH 60000U
#define JVC_MIN_GAP    11400U  // 60000 - 16 * (1725 + 525) - 8400 - 4200

#define LG_HDR_MARK             8000U
#define LG_HDR_SPACE            4000U
#define LG_BIT_MARK              600U
#define LG_ONE_SPACE            1600U
#define LG_ZERO_SPACE            550U
#define LG_RPT_SPACE            2250U
#define LG_MIN_GAP             20000U  // Completely made up figure.
#define LG_MIN_MESSAGE_LENGTH 108000UL
#define LG32_HDR_MARK           4500U

// Ref:
//   http://elektrolab.wz.cz/katalog/samsung_protocol.pdf
#define SAMSUNG_HDR_MARK             4500U
#define SAMSUNG_HDR_SPACE            4500U
#define SAMSUNG_BIT_MARK              560U
#define SAMSUNG_ONE_SPACE            1690U
#define SAMSUNG_ZERO_SPACE            560U
#define SAMSUNG_RPT_SPACE            2250U
#define SAMSUNG_MIN_GAP             20000U  // Completely made up figure.
#define SAMSUNG_MIN_MESSAGE_LENGTH 108000UL

#define DISH_BITS 16U

// Ref:
//   https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/tree/master/IRremote
#define DAIKIN_HDR_MARK    3650U  // DAIKIN_ZERO_MARK * 8
#define DAIKIN_HDR_SPACE   1623U  // DAIKIN_ZERO_MARK * 4
#define DAIKIN_ONE_SPACE   1280U
#define DAIKIN_ONE_MARK     428U
#define DAIKIN_ZERO_MARK    428U
#define DAIKIN_ZERO_SPACE   428U
#define DAIKIN_GAP        29000U

// Ref:
//   https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Denon.cpp
#define DENON_BITS                   14U  // The number of bits in the command
#define DENON_HDR_MARK              263U  // The length of the Header:Mark
#define DENON_HDR_SPACE             789U  // The lenght of the Header:Space
#define DENON_BIT_MARK              263U  // The length of a Bit:Mark
#define DENON_ONE_SPACE            1842U  // The length of a Bit:Space for 1's
#define DENON_ZERO_SPACE            789U  // The length of a Bit:Space for 0's
#define DENON_MIN_COMMAND_LENGTH 134052UL
#define DENON_MIN_GAP DENON_MIN_COMMAND_LENGTH - \
    (DENON_HDR_MARK + DENON_HDR_SPACE + DENON_BITS * \
     (DENON_BIT_MARK + DENON_ONE_SPACE) + DENON_BIT_MARK)

#define KELVINATOR_HDR_MARK    8990U
#define KELVINATOR_HDR_SPACE   4490U
#define KELVINATOR_BIT_MARK     675U
#define KELVINATOR_ONE_SPACE   1560U
#define KELVINATOR_ZERO_SPACE   520U
#define KELVINATOR_GAP_SPACE  19950U
#define KELVINATOR_CMD_FOOTER     2U

#define GLOBALCACHE_MAX_REPEAT       50U
#define GLOBALCACHE_MIN_USEC         80U
#define GLOBALCACHE_FREQ_INDEX        0U
#define GLOBALCACHE_RPT_INDEX        GLOBALCACHE_FREQ_INDEX + 1U
#define GLOBALCACHE_RPT_START_INDEX  GLOBALCACHE_RPT_INDEX + 1U
#define GLOBALCACHE_START_INDEX      GLOBALCACHE_RPT_START_INDEX + 1U

// Some useful constants
#define USECPERTICK  50U  // microseconds per clock interrupt tick
#define RAWBUF      100U  // Length of raw duration buffer
#define HEADER        2U  // Usual nr. of header entries.
#define FOOTER        2U  // Usual nr. of footer (stop bits) entries.
#define OFFSET_START  1U  // Usual rawbuf entry to start processing from.
// Marks tend to be 100us too long, and spaces 100us too short
// when received due to sensor lag.
#define MARK_EXCESS 100U
#define TOLERANCE    25U  // default percent tolerance in measurements
// receiver states
#define STATE_IDLE    2U
#define STATE_MARK    3U
#define STATE_SPACE   4U
#define STATE_STOP    5U

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

// Defined in IRremote.cpp
extern volatile irparams_t irparams;

// IR detector output is active low
#define MARK  0U
#define SPACE 1U

#define NEC_BITS           32U
#define SHERWOOD_BITS      NEC_BITS
#define SONY_MIN_BITS      SONY_12_BITS
#define SANYO_SA8650B_BITS 12U
#define MITSUBISHI_BITS    16U
#define MIN_RC5_SAMPLES    11U
#define MIN_RC6_SAMPLES     1U
#define RC5_BITS           RC5_RAW_BITS - 2U
#define RC5X_BITS          RC5_RAW_BITS - 1U
#define RC6_MODE0_BITS     20U  // Excludes the 'start' bit.
#define RC6_36_BITS        36U  // Excludes the 'start' bit.
#define PANASONIC_BITS     48U
#define JVC_BITS           16U
#define LG_BITS            28U
#define SAMSUNG_BITS       32U
#define COOLIX_BITS        24U
#define DAIKIN_BITS        99U
#define RCMM_BITS          24U

#endif  // IRREMOTEINT_H_
