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
 * Coolix A/C / heatpump added by bakrus
 * Denon: sendDenon, decodeDenon added by Massimiliano Pinto
          (from https://github.com/z3t0/Arduino-IRremote/blob/master/ir_Denon.cpp)
 * Kelvinator A/C and Sherwood added by crankyoldgit
 * DISH decode by marcosamarinho
 * Updated by markszabo (https://github.com/markszabo/IRremoteESP8266) for sending IR code on ESP8266
 * Updated by Sebastien Warin (http://sebastien.warin.fr) for receiving IR code on ESP8266
 *
 *  Updated by sillyfrog for Daikin, adopted from
 * (https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/)
 *
 *  GPL license, all text above must be included in any redistribution
 ****************************************************/

#ifndef IRremote_h
#define IRremote_h

#include <stdint.h>
#include "IRremoteInt.h"

// The following are compile-time library options.
// If you change them, recompile the library.
// If DEBUG is defined, a lot of debugging output will be printed during decoding.
// TEST must be defined for the IRtest unittests to work.  It will make some
// methods virtual, which will be slightly slower, which is why it is optional.
//#define DEBUG
//#define TEST

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
  RCMM
};

// Results returned from the decoder
class decode_results {
public:
  decode_type_t decode_type; // NEC, SONY, RC5, UNKNOWN
  union { // This is used for decoding Panasonic and Sharp data
    unsigned int panasonicAddress;
    unsigned int sharpAddress;
  };
  unsigned long long value; // Decoded value
  unsigned int bits; // Number of bits in decoded value
  volatile unsigned int *rawbuf; // Raw intervals in .5 us ticks
  unsigned int rawlen; // Number of records in rawbuf.
  bool overflow;
  unsigned long address;  // Decoded device address.
  unsigned long command;  // Decoded command.
};

uint64_t reverseBits(uint64_t input, uint16_t nbits=64);
uint8_t calcLGChecksum(uint16_t data);

// Decoded value for NEC when a repeat code is received
#define REPEAT 0xffffffff

#define SEND_PROTOCOL_NEC      case NEC: sendNEC(data, nbits); break;
#define SEND_PROTOCOL_SONY     case SONY: sendSony(data, nbits); break;
#define SEND_PROTOCOL_RC5      case RC5: sendRC5(data, nbits); break;
#define SEND_PROTOCOL_RC6      case RC6: sendRC6(data, nbits); break;
#define SEND_PROTOCOL_DISH     case DISH: sendDISH(data, nbits); break;
#define SEND_PROTOCOL_JVC      case JVC: sendJVC(data, nbits); break;
#define SEND_PROTOCOL_SAMSUNG  case SAMSUNG: sendSAMSUNG(data, nbits); break;
#define SEND_PROTOCOL_LG       case LG: sendLG(data, nbits); break;
#define SEND_PROTOCOL_WHYNTER  case WHYNTER: sendWhynter(data, nbits); break;
#define SEND_PROTOCOL_COOLIX   case COOLIX: sendCOOLIX(data, nbits); break;
#define SEND_PROTOCOL_DENON    case DENON: sendDenon(data, nbits); break;
#define SEND_PROTOCOL_SHERWOOD case SHERWOOD: sendSherwood(data, nbits); break;
#define SEND_PROTOCOL_RCMM     case RCMM: sendRCMM(data, nbits); break;
#define SEND_PROTOCOL_SHARP    case SHARP: sendSharpRaw(data, nbits); break;

// main class for receiving IR
class IRrecv
{
public:
  IRrecv(int recvpin);
  bool decode(decode_results *results, irparams_t *save=NULL);
  void enableIRIn();
  void disableIRIn();
  void resume();
  private:
  // These are called by decode
  void copyIrParams(irparams_t *dest);
  int getRClevel(decode_results *results, int *offset, int *used, int t1);
  bool decodeNEC(decode_results *results, uint16_t nbits=NEC_BITS,
                 bool strict=false);
  bool decodeSony(decode_results *results);
  bool decodeSanyo(decode_results *results);
  bool decodeMitsubishi(decode_results *results);
  bool decodeRC5(decode_results *results);
  bool decodeRC6(decode_results *results);
  bool decodeRCMM(decode_results *results);
  bool decodePanasonic(decode_results *results, uint16_t nbits=PANASONIC_BITS,
                       bool strict=false);
  bool decodeLG(decode_results *results, uint16_t nbits=LG_BITS,
                bool strict=false);
  bool decodeJVC(decode_results *results, uint16_t nbits=JVC_BITS,
                 bool strict=true);
  bool decodeSAMSUNG(decode_results *results, uint16_t nbits=SAMSUNG_BITS,
                     bool strict=false);
  bool decodeWhynter(decode_results *results, uint16_t nbits=WHYNTER_BITS,
                     bool strict=true);
  bool decodeHash(decode_results *results);
  // COOLIX decode is not implemented yet
  //  bool decodeCOOLIX(decode_results *results);
  bool decodeDaikin(decode_results *results);
  bool decodeDenon(decode_results *results, uint16_t nbits=DENON_BITS,
                   bool strict=true);
  bool decodeDISH(decode_results *results, uint16_t nbits=DISH_BITS,
                  bool strict=true);
  bool decodeSharp(decode_results *results, uint16_t nbits=SHARP_BITS,
                   bool strict=true);
  int compare(unsigned int oldval, unsigned int newval);
  uint32_t ticksLow(uint32_t usecs, uint8_t tolerance=TOLERANCE);
  uint32_t ticksHigh(uint32_t usecs, uint8_t tolerance=TOLERANCE);
  bool match(uint32_t measured_ticks, uint32_t desired_us,
             uint8_t tolerance=TOLERANCE);
  bool matchMark(uint32_t measured_ticks, uint32_t desired_us,
                 uint8_t tolerance=TOLERANCE, int excess=MARK_EXCESS);
  bool matchSpace(uint32_t measured_ticks, uint32_t desired_us,
                  uint8_t tolerance=TOLERANCE, int excess=MARK_EXCESS);
};

// Only used for testing; can remove virtual for shorter code
#ifdef TEST
#define VIRTUAL virtual
#else
#define VIRTUAL
#endif

class IRsend
{
public:
  IRsend(int IRsendPin);
  void begin();
  void send(int type, unsigned long data, int nbits) {
    switch (type) {
        SEND_PROTOCOL_NEC
        SEND_PROTOCOL_SONY
        SEND_PROTOCOL_RC5
        SEND_PROTOCOL_RC6
        SEND_PROTOCOL_DISH
        SEND_PROTOCOL_JVC
        SEND_PROTOCOL_SAMSUNG
        SEND_PROTOCOL_LG
        SEND_PROTOCOL_WHYNTER
        SEND_PROTOCOL_COOLIX
        SEND_PROTOCOL_DENON
        SEND_PROTOCOL_SHERWOOD
        SEND_PROTOCOL_RCMM
        SEND_PROTOCOL_SHARP
      }
  };
  void sendCOOLIX(unsigned long data, int nbits);
  void sendWhynter(unsigned long long data, unsigned int nbits=WHYNTER_BITS,
                   unsigned int repeat=0);
  void sendNEC(unsigned long long data, unsigned int nbits=NEC_BITS,
               unsigned int repeat=0);
  unsigned long encodeNEC(unsigned int address, unsigned int command);
  void sendLG(unsigned long long data, unsigned int nbits=28,
              unsigned int repeat=0);
  unsigned long encodeLG(uint8_t address, uint16_t command);
  // sendSony() should typically be called with repeat=2 as Sony devices
  // expect the code to be sent at least 3 times. (code + 2 repeats = 3 codes)
  // Legacy use of this procedure was to only send a single code so call it with
  // repeat=0 for backward compatiblity. As of v2.0 it defaults to sending
  // a Sony command that will be accepted be a device.
  void sendSony(unsigned long long data, unsigned int nbits=20,
                unsigned int repeat=2);
  unsigned long encodeSony(unsigned int nbits, unsigned int command,
                           unsigned int address, unsigned int extended=0);
  // Neither Sanyo nor Mitsubishi send is implemented yet
  //  void sendSanyo(unsigned long data, int nbits);
  //  void sendMitsubishi(unsigned long data, int nbits);
  void sendRaw(unsigned int buf[], int len, int hz);
  void sendGC(unsigned int buf[], int len);
  void sendRC5(unsigned long data, int nbits);
  void sendRC6(unsigned long long data, unsigned int nbits, unsigned int repeat=0);
  void sendRCMM(uint32_t data, uint8_t nbits=24);
  // sendDISH() should typically be called with repeat=3 as DISH devices
  // expect the code to be sent at least 4 times. (code + 3 repeats = 4 codes)
  // Legacy use of this procedure was only to send a single code
  // so use repeat=0 for backward compatiblity.
  void sendDISH(unsigned long long data, unsigned int nbits=DISH_BITS,
                unsigned int repeat=DISH_MIN_REPEAT);
  unsigned long encodeSharp(unsigned int address, unsigned int command,
                            unsigned int expansion=1, unsigned int check=0,
                            bool MSBfirst=false);
  void sendSharp(unsigned int address, unsigned int command,
                 unsigned int nbits=SHARP_BITS,
                 unsigned int repeat=0);
  void sendSharpRaw(unsigned long long data, unsigned int nbits=SHARP_BITS,
                    unsigned int repeat=0);
  void sendPanasonic64(unsigned long long data,
                       unsigned int nbits=PANASONIC_BITS,
                       unsigned int repeat=0);
  void sendPanasonic(unsigned int address, unsigned long data,
                     unsigned int nbits=PANASONIC_BITS, unsigned int repeat=0);
  unsigned long long encodePanasonic(uint8_t device, uint8_t subdevice,
                                   uint8_t function);
  void sendJVC(unsigned long long data, unsigned int nbits=JVC_BITS,
               unsigned int repeat=0);
  unsigned int encodeJVC(uint8_t address, uint8_t command);
  void sendSAMSUNG(unsigned long long data, unsigned int nbits=32,
                   unsigned int repeat=0);
  unsigned long encodeSAMSUNG(uint8_t customer, uint8_t command);
  void sendDaikin(unsigned char data[]);
  void sendDenon(unsigned long long data, unsigned int nbits=DENON_BITS,
                 unsigned int repeat=0);
  void sendKelvinator(unsigned char data[]);
  void sendSherwood(unsigned long data, int nbits=32, unsigned int repeat=1);
  void sendMitsubishiAC(unsigned char data[]);
  void enableIROut(unsigned long freq, uint8_t duty=50);
  VIRTUAL void mark(unsigned int usec);
  VIRTUAL void space(unsigned long usec);
private:
  uint16_t onTimePeriod;
  uint16_t offTimePeriod;
  int IRpin;
  uint32_t calcUSecPeriod(uint32_t hz);
  void sendMitsubishiACChunk(unsigned char data);
  void sendData(uint16_t onemark, uint32_t onespace,
                uint16_t zeromark, uint32_t zerospace,
                uint64_t data, uint16_t nbits, bool MSBfirst=true);
  void ledOff();
};

class IRtimer {
public:
  IRtimer();
  void reset();
  uint32_t elapsed();
private:
  uint32_t start;
};

#endif
