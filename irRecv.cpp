#include "IRremote.h"
#include "IRremoteInt.h"

#include <string>
using std::string;

 /***************************************************
 * IRremote for ESP8266
 *
 * Based on the IRremote library for Arduino by Ken Shirriff 
 * Version 0.11 August, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 * Modified by Paul Stoffregen <paul@pjrc.com> to support other boards and timers
 * Modified  by Mitra Ardron <mitra@mitra.biz> 
 * Added Sanyo and Mitsubishi controllers
 * Modified Sony to spot the repeat codes that some Sony's send
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 *
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 * LG added by Darryl Smith (based on the JVC protocol)
 * Whynter A/C ARC-110WD added by Francesco Meschia
 * Global Cache IR format sender added by Hisham Khalifa (http://www.hishamkhalifa.com)
 * Coolix A/C / heatpump added by bakrus
 *
 * Updated by markszabo (https://github.com/markszabo/IRremoteESP8266) for sending IR code on ESP8266
 * Updated by Sebastien Warin (http://sebastien.warin.fr) for receiving IR code on ESP8266
 * Nov2016 updated bu marcosamarinho to fix times and Sony and NEC improvements to reduce UNKNOWN 
 * Nov 2016 marcosamarinho split ir types on spared files and merged the changes from IRRemote see 2.0.0 changelog.md
 *  GPL license, all text above must be included in any redistribution
 ****************************************************/


void id2protocol (int id,String& protocol) {
  switch (id) {
  default:
  case UNKNOWN:       protocol="UNKNOWN";     break;
  case RC5:           protocol="RC5";         break;       
  case RC6:           protocol="RC6";         break;    
  case NEC:           protocol="NEC";         break;    
  case SONY:          protocol="SONY";        break;  
  case PANASONIC:     protocol="PANASONIC";   break;  
  case JVC:           protocol="JVC";         break;   
  case SAMSUNG:       protocol="SAMSUNG";     break; 
  case WHYNTER:       protocol="WHYNTER";     break;
  case AIWA_RC_T501:  protocol="AIWA_RC_T501";break; 
  case LG:            protocol="LG";          break;  
  case SANYO:         protocol="SANYO";       break;  
  case MITSUBISHI:    protocol="MITSUBISHI";  break;               
  case DISH:          protocol="DISH";        break;        
  case SHARP:         protocol="SHARP";       break;   
  case DENON:         protocol="DENON";       break;   
  case PRONTO:        protocol="PRONTO";      break;   
  case LEGO_PF:       protocol="LEGO_PF";     break;   
  case COOLIX:        protocol="COOLIX";      break;  
  case DAIKIN:        protocol="DAIKIN";      break;
  case GC:            protocol="GC";          break; 
  }
} 


int  IRrecv::match_mark_nolog(int measured_ticks, int desired) {
  return  measured_ticks >= TICKS_LOW(desired) && measured_ticks <= TICKS_HIGH(desired+MARK_EXCESS);
}

int   IRrecv::match_space_nolog(int measured_ticks, int desired) {
  return   measured_ticks >= TICKS_LOW(desired-MARK_EXCESS) && measured_ticks <= TICKS_HIGH(desired);
}

#ifdef DEBUG

int   IRrecv::MATCH(int measured, int desired) {
  bool   ret = measured >= TICKS_LOW(desired) && measured <= TICKS_HIGH(desired);
  if (!ret) {
    Serial.print("NOT Match ("); 
    Serial.print(TICKS_LOW(desired), DEC);
    Serial.print(" <= ");
    Serial.print(measured, DEC);
    Serial.print(" <= ");
    Serial.print(TICKS_HIGH(desired), DEC);
    Serial.print(")  Expected : ");
    Serial.println(desired, DEC);
  }
  return ret;
}

int   IRrecv::MATCH_MARK(int measured_ticks, int desired) {
  bool  ret = match_mark_nolog(measured_ticks,desired); 
  if (!ret) { 
    Serial.print("MARK NOT Match (");
    Serial.print(TICKS_LOW(desired ), DEC);
    Serial.print(" <= ");
    Serial.print(measured_ticks, DEC);
    Serial.print(" <= ");
    Serial.print(TICKS_HIGH(desired), DEC);
    Serial.print(") Expected : ");
    Serial.println(desired, DEC);
  }
  return ret;
}

int   IRrecv::MATCH_SPACE(int measured_ticks, int desired) {
  bool   ret = match_space_nolog(measured_ticks,desired);
  if (!ret) { 
    Serial.print("SPACE NOT Match (");
    Serial.print(TICKS_LOW(desired ), DEC);
    Serial.print(" <= ");
    Serial.print(measured_ticks, DEC);
    Serial.print(" <= ");
    Serial.print(TICKS_HIGH(desired ), DEC);
    Serial.print(") Expected : ");
    Serial.println(desired, DEC);
  }
  return ret;
}
#else
  int  IRrecv::MATCH(int measured, int desired) {return measured >= TICKS_LOW(desired) && measured <= TICKS_HIGH(desired);}
  int  IRrecv::MATCH_MARK( int measured_ticks, int desired) {return match_mark_nolog( measured_ticks, desired );}
  int  IRrecv::MATCH_SPACE(int measured_ticks, int desired) {return match_space_nolog(measured_ticks, desired );}
#endif

void  printDecoded(decode_results *results) {
  Serial.print("IR DECODED Type:") ; 
  Serial.print(results->decode_type);
  Serial.print(" Name:") ; 
  String sir; 
  Serial.print(results->protocol); 
  Serial.print(" RawData:") ;
  //Print long long 
  int start =false ; 
  for ( int i=15;  i>-1; i--) { 
    int val = results->value>>(4*i) & 0xF; 
    if ( val == 0 ) {
      if (start)  Serial.print("0"); 
    } else {
      start =true ; 
      Serial.print(val,HEX) ; 
    }  
  } 
  if (results->address >-1) { 
    Serial.print(" Address:"); 
    Serial.print(results->address ,HEX); 
  }
  if (results->command >-1) {
    Serial.print(" Command:");
    Serial.print(results->command,HEX) ;
  } 
  Serial.print(" Bits:"); 
  Serial.print(results->bits);  Serial.print(" "); 
  Serial.println(); 
} 

#ifdef DEBUG
void  dump(decode_results *results) {
  Serial.println("IR Received dump.");
  Serial.print("Results:");
  Serial.print(results->rawlen-OFFSET_START,  DEC);
  Serial.println(" Timing in us ");
  for (int i = OFFSET_START;  i < results->rawlen;  i++) {
    unsigned long  x = results->rawbuf[i] * USECPERTICK;
    if (x < 10000)  Serial.print(" ") ;
    if (x < 1000)   Serial.print(" ") ;
    if (x < 100)    Serial.print(" ") ;
    Serial.print("  ");
    if (!(i & 1)) {  // even
      Serial.print("-");
      Serial.print(x, DEC);
    } else {  // odd
      Serial.print("  +");
      Serial.print(x, DEC);
    }
    if (!(i % 8))  Serial.println("");
  }
  Serial.println("");                   
}

#endif

void IRrecv::addBit(unsigned long long  &data,bool  bit) {
  if (bit) {
    data = (unsigned  long long ) (data << 1) | 1;
    //Serial.print("1"); 
  } else {
    data <<= 1; 
    //Serial.print("0"); 
  } 
} 

void IRrecv::addBit(unsigned  long  &data,bool  bit) {
  if (bit) {
    data = (unsigned  long) (data << 1) | 1;
    //Serial.print("1"); 
  } else {
    data <<= 1; 
    //Serial.print("0"); 
  } 
} 

void IRrecv::addBit(unsigned int &data,bool bit) {
  if (bit) {
    data = (data << 1) | 1;
  } else {
    data <<= 1; 
  } 
} 

// No log if not matches to clen up debuggind 
bool IRrecv::mark_decode(unsigned long &data,int val, int timeOne, int timeZero) { 
  if (match_mark_nolog(val, timeOne)) {
    addBit(data,1) ;
  } else if (match_mark_nolog(val , timeZero)) {
    addBit(data,0) ;
  } else {
    #ifdef DEBUG
    Serial.print("Wrong MARK_DECODE :") ;Serial.print(val) ; 
    Serial.print(" TimeOne :"  ) ;Serial.print(timeOne) ; 
    Serial.print(" TimeZero :" ) ;Serial.println(timeZero) ; 
    #endif
    return false;  
 }
 return true; 
} 

bool IRrecv::space_decode(unsigned long &data,int val, int timeOne, int timeZero) { 
  if (match_space_nolog(val, timeOne)) {
    addBit(data,1) ;
  } else if (match_space_nolog(val, timeZero)) {
    addBit(data,0) ;
  } else {
    #ifdef DEBUG
    Serial.print("Wrong SPACE_DECODE :") ;Serial.print(val) ; 
    Serial.print(" TimeOne :"   ) ;Serial.print(timeOne) ; 
    Serial.print(" TimeZero :"  ) ;Serial.println(timeZero) ; 
    #endif
    return false; 
  }
  return true; 
} 

bool IRrecv::mark_decode(unsigned long long &data,int val, int timeOne, int timeZero) { 
  if (match_mark_nolog(val, timeOne)) {
    addBit(data,1) ;
  } else if (match_mark_nolog(val , timeZero)) {
    addBit(data,0) ;
  } else {
    #ifdef DEBUG
    Serial.print("Wrong MARK_DECODE :") ;Serial.print(val) ; 
    Serial.print(" TimeOne :"  ) ;Serial.print(timeOne) ; 
    Serial.print(" TimeZero :" ) ;Serial.println(timeZero) ; 
    #endif
    return false;  
 }
 return true; 
} 

bool IRrecv::space_decode(unsigned long long &data,int val, int timeOne, int timeZero) { 
  if (match_space_nolog(val, timeOne)) {
    addBit(data,1) ;
  } else if (match_space_nolog(val, timeZero)) {
    addBit(data,0) ;
  } else {
    #ifdef DEBUG
    Serial.print("Wrong SPACE_DECODE :") ;Serial.print(val) ; 
    Serial.print(" TimeOne :"   ) ;Serial.print(timeOne) ; 
    Serial.print(" TimeZero :"  ) ;Serial.println(timeZero) ; 
    #endif
    return false; 
  }
  return true; 
} 


//+=============================================================================
// Decodes the received IR message
// Returns 0 if no data ready, 1 if data ready.
// Results of decoding are stored in results
//
int IRrecv::decode_protocols(decode_results *results) {
 #if DECODE_NEC
  DBG_PRINTLN("Attempting NEC decode");
  if (decodeNEC(results))  return true;
#endif
#if DECODE_SONY
  DBG_PRINTLN("Attempting Sony decode");
  if (decodeSony(results)) return true;
#endif
#if DECODE_SANYO
  DBG_PRINTLN("Attempting Sanyo decode");
  if (decodeSanyo(results))  return true;  
#endif
#if DECODE_MITSUBISHI
  DBG_PRINTLN("Attempting Mitsubishi decode");
  if (decodeMitsubishi(results)) return true;
  #endif
#if DECODE_RC5
  DBG_PRINTLN("Attempting RC5 decode");
  if (decodeRC5(results))  return true;
#endif
#if DECODE_RC6
  DBG_PRINTLN("Attempting RC6 decode");
  if (decodeRC6(results))  return true;
#endif
#if DECODE_PANASONIC
  DBG_PRINTLN("Attempting Panasonic decode");
  if (decodePanasonic(results)) return true;
#endif
#if DECODE_LG_32
  DBG_PRINTLN("Attempting LG 32 bits decode");
  if (decodeLG_32(results)) return true;
#endif
#if DECODE_LG 
  DBG_PRINTLN("Attempting LG 28 bits decode");
  if (decodeLG(results)) return true;
#endif
#if DECODE_JVC
  DBG_PRINTLN("Attempting JVC decode");
  if (decodeJVC(results))  return true;
#endif
#if DECODE_SAMSUNG
  DBG_PRINTLN("Attempting SAMSUNG decode");
  if (decodeSAMSUNG(results)) return true;
#endif
#if DECODE_SHARP
  DBG_PRINTLN("Attempting SHARP decode");
  if (decodeSharp(results)) return true;
#endif
#if DECODE_WHYNTER
   DBG_PRINTLN("Attempting Whynter decode");
   if (decodeWhynter(results))  return true;
#endif
#if DECODE_AIWA_RC_T501
  DBG_PRINTLN("Attempting Aiwa RC-T501 decode");
  if (decodeAiwaRCT501(results))  return true ;
#endif
#if DECODE_DENON
  DBG_PRINTLN("Attempting Denon decode");
  if (decodeDenon(results))  return true ;
#endif
#if DECODE_LEGO_PF
  DBG_PRINTLN("Attempting Lego Power Functions");
  if (decodeLegoPowerFunctions(results))  return true ;
#endif
#if DECODE_DISH
  DBG_PRINTLN("Attempting DISH decode");
  if (decodeDISH(results))  return true ;
#endif

  // decodeHash returns a hash on any input.
  // Thus, it needs to be last in the list.
  // If you add any decodes, add them before this.
  #if DECODE_HASH
  DBG_PRINTLN("Attempting HASH decode");
  if (decodeHash(results))  return true ;
  #endif
  return false;
}  
 
// Decodes the received IR message
// Returns 0 if no data ready, 1 if data ready.
// Results of decoding are stored in results
int IRrecv::decode(decode_results *results) {
  if (irparams.rcvstate != STATE_STOP) return false;
  if (irparams.rawlen <15) {
    resume(); 
    return false; 
  } 
  results->rawbuf      = irparams.rawbuf;
  results->rawlen       = irparams.rawlen;
  // results->overflow    = irparams.overflow; 
  results->command     = -1;  
  results->address     = -1; 
  #ifdef DEBUG
  dump(results) ; 
  #endif 
  if ( decode_protocols(results) ) { 
    id2protocol(results->decode_type, results->protocol) ; 
    #ifdef DEBUG
    printDecoded(results) ; 
    #endif  
    return true ; 
  } 
  // Throw away and start over
  resume();
  return false;
}

extern "C" {
	#include "user_interface.h"
	#include "gpio.h"
}

static ETSTimer timer;
volatile irparams_t irparams;

static void ICACHE_FLASH_ATTR read_timeout(void *arg) {
  os_intr_lock();
  if (irparams.rawlen) {
    irparams.rcvstate = STATE_STOP;
  }
  os_intr_unlock();
}



static void ICACHE_FLASH_ATTR gpio_intr() {
  uint32_t gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);
  if (irparams.rcvstate == STATE_STOP) {
    return;
  }
	
  static uint32_t start = 0;
  uint32_t now = system_get_time();
  if  (irparams.rcvstate == STATE_IDLE) {
    irparams.rcvstate = STATE_MARK;
    // if OFFSET_START==1 Include an fake initial value to keep compatible with IR-Remote . 
    if ( OFFSET_START==1)    irparams.rawbuf[irparams.rawlen++] = 20 ; 
  } else { 
    if (irparams.rcvstate == STATE_MARK) irparams.rcvstate = STATE_SPACE;
    else                                  irparams.rcvstate = STATE_MARK;

    if (irparams.rawlen < RAWBUF) {
      irparams.rawbuf[irparams.rawlen++] = (now-start)/USECPERTICK ;
    } else { 
      irparams.overflow = true; 
    }  
  }
  start = now;
  os_timer_disarm(&timer);
  // This time need to be larger tham the larger repetition time, to allow full REPEAT decode . 
  os_timer_arm(&timer, 80, 0);  
}


void IRrecv::disableIRIn() {
  //irReadTimer.stop();
  //os_timer_disarm(&irReadTimer);   
   ETS_INTR_LOCK();
   ETS_GPIO_INTR_DISABLE();
}

//+=============================================================================

IRrecv::IRrecv(int recvpin) {
  irparams.recvpin = recvpin;
  irparams.blinkflag = 0;
}

IRrecv::IRrecv (int recvpin, int blinkpin){
	irparams.recvpin = recvpin;
	irparams.blinkpin = blinkpin;
	pinMode(blinkpin, OUTPUT);
	irparams.blinkflag = 0;
}


//+=============================================================================
// initialization
//
void IRrecv::enableIRIn() {

  // initialize state machine variables
  irparams.rcvstate = STATE_IDLE;
  irparams.rawlen = 0;
  
  // Initialize timer
  os_timer_disarm(&timer);
  os_timer_setfn(&timer, (os_timer_func_t *)read_timeout, &timer);
  // Attach Interrupt
  attachInterrupt(irparams.recvpin, gpio_intr, CHANGE);
}

/*
//+=============================================================================
// Enable/disable blinking of pin 13 on IR processing
//
void  IRrecv::blink13 (int blinkflag)
{
	irparams.blinkflag = blinkflag;
	if (blinkflag)  pinMode(BLINKLED, OUTPUT) ;
}
*/

//+=============================================================================
// Return if receiving new IR signals
//
bool  IRrecv::isIdle ( )
{
 return (irparams.rcvstate == STATE_IDLE || irparams.rcvstate == STATE_STOP) ? true : false;
}

//+=============================================================================
// Restart the ISR state machine
//
void  IRrecv::resume ( )
{
	irparams.rcvstate = STATE_IDLE;
	irparams.rawlen = 0;
}

//+=============================================================================
// hashdecode - decode an arbitrary IR code.
// Instead of decoding using a standard encoding scheme
// (e.g. Sony, NEC, RC5), the code is hashed to a 32-bit value.
//
// The algorithm: look at the sequence of MARK signals, and see if each one
// is shorter (0), the same length (1), or longer (2) than the previous.
// Do the same with the SPACE signals.  Hash the resulting sequence of 0's,
// 1's, and 2's to a 32-bit value.  This will give a unique value for each
// different code (probably), for most code systems.
//
// http://arcfn.com/2010/01/using-arbitrary-remotes-with-arduino.html
//
// Compare two tick values, returning 0 if newval is shorter,
// 1 if newval is equal, and 2 if newval is longer
// Use a tolerance of 20%
//
int  IRrecv::compare (unsigned int oldval,  unsigned int newval)
{
	if      (newval < oldval * .8)  return 0 ;
	else if (oldval < newval * .8)  return 2 ;
	else                            return 1 ;
}

//+=============================================================================
// Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
// Converts the raw code values into a 32-bit hash code.
// Hopefully this code is unique for each button.
// This isn't a "real" decoding, just an arbitrary value.
//
#define FNV_PRIME_32 16777619
#define FNV_BASIS_32 2166136261

 bool IRrecv::decodeHash(decode_results *results) {
  // take the dirt out when it is a noise 
  if (results->rawlen < 20+ OFFSET_START)  return false;
  long hash = FNV_BASIS_32;
  for (int i = OFFSET_START; i+2 < results->rawlen; i++) {
    int value =  compare(results->rawbuf[i], results->rawbuf[i+2]);
    // Add value into the hash
    hash = (hash * FNV_PRIME_32) ^ value;
  }
  results->value       = hash;
  results->bits        = 32;
  results->decode_type = UNKNOWN;
  return true;
}

