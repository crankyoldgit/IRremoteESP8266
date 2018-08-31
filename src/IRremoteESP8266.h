 /***************************************************
 * IRremote for ESP8266
 *
 * Based on the IRremote library for Arduino by Ken Shirriff
 * Version 0.11 August, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 * Edited by Mitra to add new controller SANYO
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 *
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 * LG added by Darryl Smith (based on the JVC protocol)
 * Whynter A/C ARC-110WD added by Francesco Meschia
 * Coolix A/C / heatpump added by (send) bakrus & (decode) crankyoldgit
 * Denon: sendDenon, decodeDenon added by Massimiliano Pinto
          (from https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Denon.cpp)
 * Kelvinator A/C and Sherwood added by crankyoldgit
 * Mitsubishi (TV) sending added by crankyoldgit
 * Pronto code sending added by crankyoldgit
 * Mitsubishi & Toshiba A/C added by crankyoldgit
 *     (derived from https://github.com/r45635/HVAC-IR-Control)
 * DISH decode by marcosamarinho
 * Gree Heatpump sending added by Ville Skyttä (scop)
 *     (derived from https://github.com/ToniA/arduino-heatpumpir/blob/master/GreeHeatpumpIR.cpp)
 * Updated by markszabo (https://github.com/markszabo/IRremoteESP8266) for sending IR code on ESP8266
 * Updated by Sebastien Warin (http://sebastien.warin.fr) for receiving IR code on ESP8266
 *
 * Updated by sillyfrog for Daikin, adopted from
 * (https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/)
 * Fujitsu A/C code added by jonnygraham
 * Trotec AC code by stufisher
 * Carrier & Haier AC code by crankyoldgit
 *
 *  GPL license, all text above must be included in any redistribution
 ****************************************************/

#ifndef IRREMOTEESP8266_H_
#define IRREMOTEESP8266_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#ifdef UNIT_TEST
#include <iostream>
#endif

// Library Version
#define _IRREMOTEESP8266_VERSION_ "2.4.3"
// Supported IR protocols
// Each protocol you include costs memory and, during decode, costs time
// Disable (set to false) all the protocols you do not need/want!
// The Air Conditioner protocols are the most expensive memory-wise.
//
#define DECODE_HASH            true  // Semi-unique code for unknown messages

#define SEND_RAW               true

#define DECODE_NEC             true
#define SEND_NEC               true

#define DECODE_SHERWOOD        true  // Doesn't exist. Actually is DECODE_NEC
#define SEND_SHERWOOD          true

#define DECODE_RC5             true
#define SEND_RC5               true

#define DECODE_RC6             true
#define SEND_RC6               true

#define DECODE_RCMM            true
#define SEND_RCMM              true

#define DECODE_SONY            true
#define SEND_SONY              true

#define DECODE_PANASONIC       true
#define SEND_PANASONIC         true

#define DECODE_JVC             true
#define SEND_JVC               true

#define DECODE_SAMSUNG         true
#define SEND_SAMSUNG           true

#define DECODE_WHYNTER         true
#define SEND_WHYNTER           true

#define DECODE_AIWA_RC_T501    true
#define SEND_AIWA_RC_T501      true

#define DECODE_LG              true
#define SEND_LG                true

#define DECODE_SANYO           true
#define SEND_SANYO             true

#define DECODE_MITSUBISHI      true
#define SEND_MITSUBISHI        true

#define DECODE_MITSUBISHI2     true
#define SEND_MITSUBISHI2       true

#define DECODE_DISH            true
#define SEND_DISH              true

#define DECODE_SHARP           true
#define SEND_SHARP             true

#define DECODE_DENON           true
#define SEND_DENON             true

#define DECODE_KELVINATOR      true
#define SEND_KELVINATOR        true

#define DECODE_MITSUBISHI_AC   false  // Not written.
#define SEND_MITSUBISHI_AC     true

#define DECODE_FUJITSU_AC      true
#define SEND_FUJITSU_AC        true

#define DECODE_DAIKIN          true
#define SEND_DAIKIN            true

#define DECODE_COOLIX          true
#define SEND_COOLIX            true

#define DECODE_GLOBALCACHE     false  // Not written.
#define SEND_GLOBALCACHE       true

#define DECODE_GREE            true
#define SEND_GREE              true

#define DECODE_PRONTO          false  // Not written.
#define SEND_PRONTO            true

#define DECODE_ARGO            false  // Not written.
#define SEND_ARGO              true

#define DECODE_TROTEC          false  // Not implemented.
#define SEND_TROTEC            true

#define DECODE_NIKAI           true
#define SEND_NIKAI             true

#define DECODE_TOSHIBA_AC      true
#define SEND_TOSHIBA_AC        true

#define DECODE_MAGIQUEST       true
#define SEND_MAGIQUEST         true

#define DECODE_MIDEA           true
#define SEND_MIDEA             true

#define DECODE_LASERTAG        true
#define SEND_LASERTAG          true

#define DECODE_CARRIER_AC      true
#define SEND_CARRIER_AC        true

#define DECODE_HAIER_AC        true
#define SEND_HAIER_AC          true

#define DECODE_HITACHI_AC      true
#define SEND_HITACHI_AC        true

#define DECODE_HITACHI_AC1     true
#define SEND_HITACHI_AC1       true

#define DECODE_HITACHI_AC2     true
#define SEND_HITACHI_AC2       true

#define DECODE_GICABLE         true
#define SEND_GICABLE           true

#define DECODE_HAIER_AC_YRW02  true
#define SEND_HAIER_AC_YRW02    true

#if (DECODE_ARGO || DECODE_DAIKIN || DECODE_FUJITSU_AC || DECODE_GREE || \
     DECODE_KELVINATOR || DECODE_MITSUBISHI_AC || DECODE_TOSHIBA_AC || \
     DECODE_TROTEC || DECODE_HAIER_AC || DECODE_HITACHI_AC || \
     DECODE_HITACHI_AC1 || DECODE_HITACHI_AC2 || DECODE_HAIER_AC_YRW02)
#define DECODE_AC true  // We need some common infrastructure for decoding A/Cs.
#else
#define DECODE_AC false   // We don't need that infrastructure.
#endif

// Use millisecond 'delay()' calls where we can to avoid tripping the WDT.
// Note: If you plan to send IR messages in the callbacks of the AsyncWebserver
//       library, you need to set ALLOW_DELAY_CALLS to false.
//       Ref: https://github.com/markszabo/IRremoteESP8266/issues/430
#define ALLOW_DELAY_CALLS true

/*
 * Always add to the end of the list and should never remove entries
 * or change order. Projects may save the type number for later usage
 * so numbering should always stay the same.
 */
enum decode_type_t {
  UNKNOWN = -1,
  UNUSED = 0,
  RC5,
  RC6,
  NEC,
  SONY,
  PANASONIC,
  JVC,
  SAMSUNG,
  WHYNTER,
  AIWA_RC_T501,
  LG,
  SANYO,
  MITSUBISHI,
  DISH,
  SHARP,
  COOLIX,
  DAIKIN,
  DENON,
  KELVINATOR,
  SHERWOOD,
  MITSUBISHI_AC,
  RCMM,
  SANYO_LC7461,
  RC5X,
  GREE,
  PRONTO,  // Technically not a protocol, but an encoding.
  NEC_LIKE,
  ARGO,
  TROTEC,
  NIKAI,
  RAW,  // Technically not a protocol, but an encoding.
  GLOBALCACHE,  // Technically not a protocol, but an encoding.
  TOSHIBA_AC,
  FUJITSU_AC,
  MIDEA,
  MAGIQUEST,
  LASERTAG,
  CARRIER_AC,
  HAIER_AC,
  MITSUBISHI2,
  HITACHI_AC,
  HITACHI_AC1,
  HITACHI_AC2,
  GICABLE,
  HAIER_AC_YRW02
};

// Message lengths & required repeat values
const uint16_t kNoRepeat = 0;
const uint16_t kSingleRepeat = 1;
const uint16_t kAiwaRcT501Bits = 15;
#define AIWA_RC_T501_BITS           kAiwaRcT501Bits
const uint16_t kAiwaRcT501MinRepeats = kSingleRepeat;
const uint16_t kCoolixBits = 24;
#define COOLIX_BITS                 kCoolixBits
const uint16_t kCarrierACBits = 32;
#define CARRIER_AC_BITS             kCarrierACBits
const uint16_t kCarrierACMinRepeat = kNoRepeat;
// Daikin has a lot of static stuff that is discarded
const uint16_t kDaikinRawBits = 583;
const uint16_t kDaikinStateLength = 27;
#define DAIKIN_COMMAND_LENGTH       kDaikinStateLength
const uint16_t kDaikinBits = kDaikinStateLength * 8;
#define DENON_BITS                  kSharpBits
#define DENON_48_BITS               kPanasonicBits
const uint16_t kDenonLegacyBits = 14;
#define DENON_LEGACY_BITS           kDenonLegacyBits
const uint16_t kDishBits = 16;
#define DISH_BITS                   kDishBits
const uint16_t kDishMinRepeat = 3;
const uint16_t kGICableBits = 16;
#define GICABLE_BITS                kGICableBits
const uint16_t kGICableMinRepeat = kSingleRepeat;
const uint16_t kGreeStateLength = 8;
#define GREE_STATE_LENGTH            kGreeStateLength
const uint16_t kGreeBits = kGreeStateLength * 8;
const uint16_t kHaierACStateLength = 9;
#define HAIER_AC_STATE_LENGTH        kHaierACStateLength
const uint16_t kHaierACBits = kHaierACStateLength * 8;
const uint16_t kHaierACYRW02StateLength = 14;
#define HAIER_AC_YRW02_STATE_LENGTH kHaierACYRW02StateLength
const uint16_t kHaierACYRW02Bits = kHaierACYRW02StateLength * 8;
const uint16_t kHitachiACStateLength = 28;
#define HITACHI_AC_STATE_LENGTH     kHitachiACStateLength
const uint16_t kHitachiACBits = kHitachiACStateLength * 8;
#define HITACHI_AC_BITS             kHitachiACBits
const uint16_t kHitachiAC1StateLength = 13;
#define HITACHI_AC1_STATE_LENGTH    kHitachiAC1StateLength
const uint16_t kHitachiAC1Bits = kHitachiAC1StateLength * 8;
#define HITACHI_AC1_BITS            kHitachiAC1Bits
const uint16_t kHitachiAC2StateLength = 53;
#define HITACHI_AC2_STATE_LENGTH    kHitachiAC2StateLength
const uint16_t kHitachiAC2Bits = kHitachiAC2StateLength * 8;
#define HITACHI_AC2_BITS            kHitachiAC2Bits
const uint16_t kJVCBits = 16;
#define JVC_BITS                    kJVCBits
const uint16_t kKelvinatorStateLength = 16;
#define KELVINATOR_STATE_LENGTH     kKelvinatorStateLength
const uint16_t kKelvinatorBits = kKelvinatorStateLength * 8;
const uint16_t kLGBits = 28;
#define LG_BITS                     kLGBits
const uint16_t kLG32Bits = 32;
#define LG32_BITS                   kLG32Bits
const uint16_t kMitsubishiBits = 16;
#define MITSUBISHI_BITS             kMitsubishiBits
// TODO(anyone): Verify that the Mitsubishi repeat is really needed.
//               Based on marcosamarinho's code.
const uint16_t kMitsubishiMinRepeat = kSingleRepeat;
const uint16_t kMitsubishiACStateLength = 18;
#define MITSUBISHI_AC_STATE_LENGTH  kMitsubishiACStateLength
const uint16_t kMitsubishiACMinRepeat = kSingleRepeat;
const uint16_t kFujitsuACMinRepeat = kNoRepeat;
#define FUJITSU_AC_MIN_REPEAT        kFujitsuACMinRepeat
const uint16_t kFujitsuACStateLength = 16;
#define FUJITSU_AC_STATE_LENGTH     kFujitsuACStateLength
const uint16_t kFujitsuACStateLengthShort = 7;
#define FUJITSU_AC_STATE_LENGTH_SHORT kFujitsuACStateLengthShort
const uint16_t kFujitsuACBits = kFujitsuACStateLength * 8;
#define FUJITSU_AC_BITS             kFujitsuACBits
const uint16_t kFujitsuACMinBits = (kFujitsuACStateLengthShort - 1) * 8;
#define FUJITSU_AC_MIN_BITS         kFujitsuACMinBits
const uint16_t kNECBits = 32;
#define NEC_BITS                    kNECBits
const uint16_t kPanasonicBits = 48;
#define PANASONIC_BITS              kPanasonicBits
const uint32_t kPanasonicManufacturer = 0x4004;
const uint16_t kProntoMinLength = 6;
const uint16_t kRC5RawBits = 14;
const uint16_t kRC5Bits = kRC5RawBits - 2;
#define RC5_BITS      kRC5Bits
const uint16_t kRC5XBits = kRC5RawBits - 1;
#define RC5X_BITS     kRC5XBits
const uint16_t kRC6Mode0Bits = 20;  // Excludes the 'start' bit.
#define RC6_MODE0_BITS              kRC6Mode0Bits
const uint16_t kRC6_36Bits = 36;  // Excludes the 'start' bit.
#define RC6_36_BITS                 kRC6_36Bits
const uint16_t kRCMMBits = 24;
#define RCMM_BITS                   kRCMMBits
const uint16_t kSamsungBits = 32;
#define SAMSUNG_BITS                kSamsungBits
const uint16_t kSanyoSA8650BBits = 12;
#define SANYO_SA8650B_BITS          kSanyoSA8650BBits
const uint16_t kSanyoLC7461AddressBits = 13;
const uint16_t kSanyoLC7461CommandBits = 8;
const uint16_t kSanyoLC7461Bits = (kSanyoLC7461AddressBits +
                                   kSanyoLC7461CommandBits) * 2;
#define SANYO_LC7461_BITS           kSanyoLC7461Bits
const uint8_t kSharpAddressBits = 5;
const uint8_t kSharpCommandBits = 8;
const uint16_t kSharpBits = kSharpAddressBits + kSharpCommandBits + 2;  // 15
#define SHARP_BITS                  kSharpBits
const uint8_t kSherwoodBits = kNECBits;
#define SHERWOOD_BITS          kSherwoodBits
const uint8_t kSherwoodMinRepeat = kSingleRepeat;
const uint16_t kSony12Bits = 12;
#define SONY_12_BITS                kSony12Bits
const uint16_t kSony15Bits = 15;
#define SONY_15_BITS                kSony15Bits
const uint16_t kSony20Bits = 20;
#define SONY_20_BITS                kSony20Bits
const uint16_t kSonyMinBits = 12;
const uint16_t kSonyMinRepeat = 2;
const uint16_t kToshibaACStateLength = 9;
#define TOSHIBA_AC_STATE_LENGTH      kToshibaACStateLength
const uint16_t kToshibaACBits = kToshibaACStateLength * 8;
const uint16_t kToshibaACMinRepeat = kSingleRepeat;
const uint16_t kTrotecStateLength = 9;
#define TROTEC_COMMAND_LENGTH        kTrotecStateLength
const uint16_t kWhynterBits = 32;
#define WHYNTER_BITS                kWhynterBits
const uint16_t kArgoStateLength = 12;
#define ARGO_COMMAND_LENGTH         kArgoStateLength
const uint16_t kNikaiBits = 24;
#define NIKAI_BITS                  kNikaiBits
const uint16_t kMagiquestBits = 56;
#define MAGIQUEST_BITS              kMagiquestBits
const uint16_t kMideaBits = 48;
#define MIDEA_BITS                  kMideaBits
const uint16_t kMideaMinRepeat = kNoRepeat;
const uint16_t kLasertagBits = 13;
#define LASERTAG_BITS               kLasertagBits
const uint16_t kLasertagMinRepeat = kNoRepeat;

// Turn on Debugging information by uncommenting the following line.
// #define DEBUG 1

#ifdef DEBUG
#ifdef UNIT_TEST
#define DPRINT(x) do { std::cout << x; } while (0)
#define DPRINTLN(x) do { std::cout << x << std::endl; } while (0)
#endif  // UNIT_TEST
#ifdef ARDUINO
#define DPRINT(x) do { Serial.print(x); } while (0)
#define DPRINTLN(x) do { Serial.println(x); } while (0)
#endif  // ARDUINO
#else  // DEBUG
#define DPRINT(x)
#define DPRINTLN(x)
#endif  // DEBUG

#endif  // IRREMOTEESP8266_H_
