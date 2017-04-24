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
 * Coolix A/C / heatpump added by bakrus
 * Denon: sendDenon, decodeDenon added by Massimiliano Pinto
          (from https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Denon.cpp)
 * Kelvinator A/C added by crankyoldgit
 * Mitsubishi A/C added by crankyoldgit
 *     (based on https://github.com/r45635/HVAC-IR-Control)
 * DISH decode by marcosamarinho
 *
 * 09/23/2015 : Samsung pulse parameters updated by Sebastien Warin to be compatible with EUxxD6200
 *
 *  GPL license, all text above must be included in any redistribution
 ****************************************************/

#ifndef IRremoteint_h
#define IRremoteint_h

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

// Pulse parms are *50-100 for the Mark and *50+100 for the space
// First MARK is the one after the long gap
// pulse parameters in usec
#define COOLIX_BIT_MARK	560       // Approximately 21 cycles at 38kHz
#define COOLIX_ONE_SPACE	COOLIX_BIT_MARK * 3
#define COOLIX_ZERO_SPACE	COOLIX_BIT_MARK * 1
#define COOLIX_HDR_MARK	COOLIX_BIT_MARK * 8
#define COOLIX_HDR_SPACE	COOLIX_BIT_MARK * 8

#define WHYNTER_HDR_MARK	2850
#define WHYNTER_HDR_SPACE	2850
#define WHYNTER_BIT_MARK	750
#define WHYNTER_ONE_MARK	750
#define WHYNTER_ONE_SPACE	2150
#define WHYNTER_ZERO_MARK	750
#define WHYNTER_ZERO_SPACE	750

#define NEC_HDR_MARK             9000
#define NEC_HDR_SPACE            4500
#define NEC_BIT_MARK              560
#define NEC_ONE_SPACE            1690
#define NEC_ZERO_SPACE            560
#define NEC_RPT_SPACE            2250
#define NEC_RPT_LENGTH              4
#define NEC_MIN_COMMAND_LENGTH 108000UL
#define NEC_MIN_GAP NEC_MIN_COMMAND_LENGTH - NEC_HDR_MARK - NEC_HDR_SPACE - NEC_BITS * (NEC_BIT_MARK + NEC_ONE_SPACE) + NEC_BIT_MARK

// Timings based on http://www.sbprojects.com/knowledge/ir/sirc.php
#define SONY_HDR_MARK	  2400
#define SONY_SPACE      600
#define SONY_ONE_MARK	  1200 + 50  // Experiments suggest +50 to spec is better.
#define SONY_ZERO_MARK	600 + 50  // Experiments suggest +50 to spec is better.
#define SONY_RPT_LENGTH 45000
#define SONY_MIN_GAP    10000

// Sanyo SA 8650B
// Ref: https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Sanyo.cpp
#define SANYO_SA8650B_HDR_MARK          3500U  // seen range 3500
#define SANYO_SA8650B_HDR_SPACE          950U  // seen 950
#define SANYO_SA8650B_ONE_MARK          2400U  // seen 2400
#define SANYO_SA8650B_ZERO_MARK          700U  // seen 700
#define SANYO_SA8650B_DOUBLE_SPACE_USECS 800U  // usually see 713 - not using ticks as get number wrapround
#define SANYO_SA8650B_RPT_LENGTH       45000U

// Sanyo LC7461
// Ref:
//   https://github.com/marcosamarinho/IRremoteESP8266/blob/master/ir_Sanyo.cpp
//   http://slydiman.narod.ru/scr/kb/sanyo.htm
//   http://pdf.datasheetcatalog.com/datasheet/sanyo/LC7461.pdf
#define SANYO_LC7461_ADDRESS_BITS           13U
#define SANYO_LC7461_COMMAND_BITS            8U
#define SANYO_LC7461_BITS ((SANYO_LC7461_ADDRESS_BITS + SANYO_LC7461_COMMAND_BITS) * 2)
#define SANYO_LC7461_ADDRESS_MASK ((1 << SANYO_LC7461_ADDRESS_BITS) - 1)
#define SANYO_LC7461_COMMAND_MASK ((1 << SANYO_LC7461_COMMAND_BITS) - 1)
#define SANYO_LC7461_HDR_MARK             9000U
#define SANYO_LC7461_HDR_SPACE            4500U
#define SANYO_LC7461_BIT_MARK              560U  // 1T
#define SANYO_LC7461_ONE_SPACE            1690U  // 3T
#define SANYO_LC7461_ZERO_SPACE            560U  // 1T
#define SANYO_LC7461_MIN_COMMAND_LENGTH 108000UL
#define SANYO_LC7461_MIN_GAP SANYO_LC7461_MIN_COMMAND_LENGTH - (SANYO_LC7461_HDR_MARK + SANYO_LC7461_HDR_SPACE + SANYO_LC7461_BITS * (SANYO_LC7461_BIT_MARK + (SANYO_LC7461_ONE_SPACE + SANYO_LC7461_ZERO_SPACE) / 2) + SANYO_LC7461_BIT_MARK)

// Mitsubishi RM 75501
// 14200 7 41 7 42 7 42 7 17 7 17 7 18 7 41 7 18 7 17 7 17 7 18 7 41 8 17 7 17 7 18 7 17 7

// #define MITSUBISHI_HDR_MARK	250  // seen range 3500
#define MITSUBISHI_HDR_SPACE	350 //  7*50+100
#define MITSUBISHI_ONE_MARK	1950 // 41*50-100
#define MITSUBISHI_ZERO_MARK  750 // 17*50-100
// #define MITSUBISHI_DOUBLE_SPACE_USECS  800  // usually ssee 713 - not using ticks as get number wrapround
// #define MITSUBISHI_RPT_LENGTH 45000

// Mitsubishi A/C
// Values were initially obtained from:
//   https://github.com/r45635/HVAC-IR-Control/blob/master/HVAC_ESP8266/HVAC_ESP8266.ino#L84
#define MITSUBISHI_AC_HDR_MARK    3400
#define MITSUBISHI_AC_HDR_SPACE   1750
#define MITSUBISHI_AC_BIT_MARK    450
#define MITSUBISHI_AC_ONE_SPACE   1300
#define MITSUBISHI_AC_ZERO_SPACE  420
#define MITSUBISHI_AC_RPT_MARK    440
#define MITSUBISHI_AC_RPT_SPACE   17100L


#define RC5_T1		889
#define RC5_RPT_LENGTH	46000

#define RC6_HDR_MARK	2666
#define RC6_HDR_SPACE	889
#define RC6_T1		444
#define RC6_RPT_LENGTH	46000

// http://www.sbprojects.com/knowledge/ir/rcmm.php
#define RCMM_HDR_MARK 416
#define RCMM_HDR_SPACE 277
#define RCMM_BIT_MARK 166
#define RCMM_BIT_SPACE_0 277
#define RCMM_BIT_SPACE_1 444
#define RCMM_BIT_SPACE_2 611
#define RCMM_BIT_SPACE_3 777
#define RCMM_RPT_LENGTH 27778
#define RCMM_MIN_GAP 3360
// Use a tolerance of +/-10% when matching some data spaces.
#define RCMM_TOLERANCE 10
#define RCMM_EXCESS 50

#define SHARP_BIT_MARK 245
#define SHARP_ONE_SPACE 1805
#define SHARP_ZERO_SPACE 795
#define SHARP_GAP 600000
#define SHARP_TOGGLE_MASK 0x3FF
#define SHARP_RPT_SPACE 3000

// Ref:
//   https://github.com/marcosamarinho/IRremoteESP8266/blob/master/ir_Dish.cpp
//   http://www.hifi-remote.com/wiki/index.php?title=Dish
#define DISH_HDR_MARK    400
#define DISH_HDR_SPACE  6100
#define DISH_BIT_MARK    400
#define DISH_ONE_SPACE  1700
#define DISH_ZERO_SPACE 2800
#define DISH_RPT_SPACE  6200
#define DISH_MIN_REPEAT    3

// Ref: http://www.remotecentral.com/cgi-bin/mboard/rc-pronto/thread.cgi?26152
#define PANASONIC_HDR_MARK             3456
#define PANASONIC_HDR_SPACE            1728
#define PANASONIC_BIT_MARK              432
#define PANASONIC_ONE_SPACE            1296
#define PANASONIC_ZERO_SPACE            432
#define PANASONIC_MIN_COMMAND_LENGTH 130000UL
// 130000 - 3456 - 1728 - 48*(432+1296) - 432 = 41440
#define PANASONIC_MIN_GAP             41440
#define PANASONIC_MANUFACTURER       0x2002ULL


// Ref: http://www.sbprojects.com/knowledge/ir/jvc.php
#define JVC_HDR_MARK    8400
#define JVC_HDR_SPACE   4200
#define JVC_BIT_MARK     525
#define JVC_ONE_SPACE   1725
#define JVC_ZERO_SPACE   525
#define JVC_RPT_LENGTH 60000
#define JVC_MIN_GAP    11400  // 60000 - 16 * (1725 + 525) - 8400 - 4200

#define LG_HDR_MARK    8000
#define LG_HDR_SPACE   4000
#define LG_BIT_MARK     600
#define LG_ONE_SPACE   1600
#define LG_ZERO_SPACE   550
#define LG_RPT_SPACE   2250
#define LG_MIN_GAP    20000  // Completely made up figure.
#define LG_MIN_MESSAGE_LENGTH 108000UL
#define LG32_HDR_MARK  4500

// Ref: http://elektrolab.wz.cz/katalog/samsung_protocol.pdf
#define SAMSUNG_HDR_MARK   4500
#define SAMSUNG_HDR_SPACE  4500
#define SAMSUNG_BIT_MARK    560
#define SAMSUNG_ONE_SPACE  1690
#define SAMSUNG_ZERO_SPACE  560
#define SAMSUNG_RPT_SPACE  2250
#define SAMSUNG_MIN_GAP   20000  // Completely made up figure.
#define SAMSUNG_MIN_MESSAGE_LENGTH 108000UL

#define SHARP_BITS 15
#define DISH_BITS 16

// Daikin, from https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/tree/master/IRremote
#define DAIKIN_HDR_MARK	    3650 //DAIKIN_ZERO_MARK*8
#define DAIKIN_HDR_SPACE	1623 //DAIKIN_ZERO_MARK*4
#define DAIKIN_ONE_SPACE	1280
#define DAIKIN_ONE_MARK	    428
#define DAIKIN_ZERO_MARK	428
#define DAIKIN_ZERO_SPACE 428

//Denon, from https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Denon.cpp
#define DENON_BITS                   14  // The number of bits in the command
#define DENON_HDR_MARK              263  // The length of the Header:Mark
#define DENON_HDR_SPACE             789  // The lenght of the Header:Space
#define DENON_BIT_MARK              263  // The length of a Bit:Mark
#define DENON_ONE_SPACE            1842  // The length of a Bit:Space for 1's
#define DENON_ZERO_SPACE            789  // The length of a Bit:Space for 0's
#define DENON_MIN_COMMAND_LENGTH 134052UL
#define DENON_MIN_GAP DENON_MIN_COMMAND_LENGTH - DENON_HDR_MARK - DENON_HDR_SPACE - DENON_BITS * (DENON_BIT_MARK + DENON_ONE_SPACE) - DENON_BIT_MARK

#define KELVINATOR_HDR_MARK	  8990U
#define KELVINATOR_HDR_SPACE	4490U
#define KELVINATOR_BIT_MARK	  675U
#define KELVINATOR_ONE_SPACE	1560U
#define KELVINATOR_ZERO_SPACE	520U
#define KELVINATOR_GAP_SPACE	19950U
#define KELVINATOR_CMD_FOOTER	2U

// Some useful constants
#define USECPERTICK 50U  // microseconds per clock interrupt tick
#define RAWBUF 100U // Length of raw duration buffer
#define HEADER 2U  // Usual nr. of header entries.
#define FOOTER 2U  // Usual nr. of footer (stop bits) entries.
#define OFFSET_START 1U  // Usual rawbuf entry to start processing from.

// Marks tend to be 100us too long, and spaces 100us too short
// when received due to sensor lag.
#define MARK_EXCESS 100

#define _GAP 5000 // Minimum map between transmissions
#define GAP_TICKS (_GAP/USECPERTICK)

#define TOLERANCE 25  // default percent tolerance in measurements

// receiver states
#define STATE_IDLE     2
#define STATE_MARK     3
#define STATE_SPACE    4
#define STATE_STOP     5

// information for the interrupt handler
typedef struct {
  uint8_t recvpin;              // pin for IR data from detector
  uint8_t rcvstate;             // state machine
  unsigned int timer;           // state timer, counts 50uS ticks.
  unsigned int rawbuf[RAWBUF];  // raw data
  // uint16_t is used for rawlen as it saves 3 bytes of iram in the interrupt
  // handler. Don't ask why, I don't know. It just does.
  uint16_t rawlen;              // counter of entries in rawbuf.
  uint8_t overflow;             // Buffer overflow indicator.
}
irparams_t;

// Defined in IRremote.cpp
extern volatile irparams_t irparams;

// IR detector output is active low
#define MARK  0
#define SPACE 1

#define TOPBIT 0x80000000

#define NEC_BITS 32
#define SONY_MIN_BITS 12
#define SANYO_SA8650B_BITS 12U
#define MITSUBISHI_BITS 16
#define MIN_RC5_SAMPLES 11
#define MIN_RC6_SAMPLES 1
#define PANASONIC_BITS 48
#define JVC_BITS 16
#define LG_BITS 28
#define SAMSUNG_BITS 32
#define WHYNTER_BITS 32
#define COOLIX_NBYTES 3
#define DAIKIN_BITS 99

#endif
