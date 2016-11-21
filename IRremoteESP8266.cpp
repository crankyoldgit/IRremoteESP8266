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
 *  GPL license, all text above must be included in any redistribution
 * Nov 2016 marcosamarinho split ir types on spared files and merged the changes from IRRemote 
 *  Changes clean up debug and code clean up , improvements and fixes  on Sony and NEC , decode SHARP ,SANYO, LG 32 bits  
 * Send Mitsubish 
 *  Note : Some Functions on IRRemote changed signature , and 
 *  include "IRremoteESP8266.h" removed and now  use IRRemote.h 
 ****************************************************/

#include "IRremote.h"
#include "IRremoteInt.h"
#include <string>
using std::string;

// These versions of MATCH, MATCH_MARK, and MATCH_SPACE are only for debugging.
// To use them, set DEBUG in IRremoteInt.h


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

void ir_type (int type,String& ret) {
  switch (type) {
  default:
  case UNKNOWN:       ret="UNKNOWN";     break;
  case RC5:           ret="RC5";         break;       
  case RC6:           ret="RC6";         break;    
  case NEC:           ret="NEC";         break;    
  case SONY:          ret="SONY";        break;  
  case PANASONIC:     ret="PANASONIC";   break;  
  case JVC:           ret="JVC";         break;   
  case SAMSUNG:       ret="SAMSUNG";     break; 
  case WHYNTER:       ret="WHYNTER";     break;
  case AIWA_RC_T501:  ret="AIWA_RC_T501";break; 
  case LG:            ret="LG";          break;  
  case SANYO:         ret="SANYO";       break;  
  case MITSUBISHI:    ret="MITSUBISHI";  break;               
  case DISH:          ret="DISH";        break;        
  case SHARP:         ret="SHARP";       break;   
  case DENON:         ret="DENON";       break;   
  case PRONTO:        ret="PRONTO";      break;   
  case LEGO_PF:       ret="LEGO_PF";     break;   
  case COOLIX:        ret="COOLIX";      break;  
  case DAIKIN:        ret="DAIKIN";      break;
  case GC:            ret="GC";          break; 
  }
} 

void  printDecoded(decode_results *results) {
  Serial.print("IR DECODED Type:") ; 
  Serial.print(results->decode_type);
  Serial.print(" Name:") ; 
  String sir; 
  ir_type(results->decode_type, sir) ; 
  Serial.print(sir); 
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
  Serial.print(" Bits:"); 
  Serial.print(results->bits);  Serial.print(" "); 
  if (results->address >-1) { 
      Serial.print(" Address:"); 
      Serial.print(results->address ,HEX); 
  }
  if (results->command >-1) {
     Serial.print(" Command:");
     Serial.print(results->command,HEX) ;
  } 
  Serial.println(); 
} 

#ifdef DEBUG
void  dump(decode_results *results)
{
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
// IRsend -----------------------------------------------------------------------------------

IRsend::IRsend(int IRsendPin)
{
  IRpin = IRsendPin;
}
void IRsend::begin()
{
  pinMode(IRpin, OUTPUT);
}

void IRsend::sendRaw(unsigned int buf[], int len, int hz) {
  enableIROut(hz);
  for (int i = 0; i < len; i++) {
    if (i & 1) {
      space(buf[i]);
    } else {
      mark(buf[i]);
    }
  }
  space(0); // Just to be sure
}

void IRsend::mark(unsigned int usec) {
  // Sends an IR MARK for the specified number of microseconds.
  // The MARK output is modulated at the PWM frequency.
  int  timeLow = PeriodicTime-timeHigh; 
  // this logic allows go back to right frequence at integer calc like 40Mhz and allow change duty cicle 
  long timeEnd = micros()+usec; 
  while(micros() < timeEnd){
    digitalWrite(IRpin, HIGH);
    delayMicroseconds(timeHigh );
    digitalWrite(IRpin, LOW);
    delayMicroseconds(timeLow); 
   }
}


/* Leave pin off for time (given in microseconds) */
void IRsend::space(unsigned int usec) {
  // Sends an IR space for the specified number of microseconds.
  // A space is no output, so the PWM output is disabled.
  digitalWrite(IRpin, LOW);
  if (usec > 0 ) { 
    if   (usec < 1000) delayMicroseconds(usec);
    else {
      long timeEnd = micros()+usec; 
      delay((timeEnd-micros())/1000); // allow other process works while waiting,like wifi
      long finalTime=timeEnd-micros(); 
      if  (finalTime >0) delayMicroseconds(finalTime);
    }
  }
}

void IRsend::space_encode (bool bit,int timeH,int timeL ) {
  if (bit) {
     space(timeH); 
  } else { 
     space(timeL); 
  }
}

void IRsend::mark_encode (bool bit,int timeH,int timeL ) {
  if (bit) {
     mark(timeH); 
  } else { 
     mark(timeL); 
  }
}

void IRsend::addBit(unsigned long long  &data,bool  bit) {
  if (bit) {
    data = (unsigned  long long ) (data << 1) | 1;
    //Serial.print("1"); 
  } else {
    data <<= 1; 
    //Serial.print("0"); 
  } 
} 

void IRsend::sendBitMarkSpaceCoded(bool bit, int timeMark ,int timeSpaceH,int timeSpaceL ) {
  mark(timeMark);   
  if (bit)  space(timeSpaceH);  
  else      space(timeSpaceL);   
}

void IRsend::enableIROut(int khz,int dutycycle) {
  // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
  //dutycycle=2;  //50% default   . 
  //dutycycle=3;  //33%  
  //dutycycle=4;  //25%
  timeHigh      = 1000/(khz*dutycycle);  
  PeriodicTime  = 1000/khz;              
}


//IRRecv------------------------------------------------------

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
    // Include an fake initial space to keep compatible with IR-Remote 
    if ( OFFSET_START==1)    irparams.rawbuf[irparams.rawlen++] = 20 ; 
  } else if (irparams.rawlen < RAWBUF) {
    irparams.rawbuf[irparams.rawlen++] = (now-start)/USECPERTICK ;
  }
  start = now;
  os_timer_disarm(&timer);
  // Wait for new data 12ms !, this  avoid repetitions at the receiver ( Sony we can see 14.5 ms in RM-AAU023  )
  os_timer_arm(&timer, 80, 0);  
}
IRrecv::IRrecv(int recvpin) {
  irparams.recvpin = recvpin;
}

// initialization
void IRrecv::enableIRIn() {
 // initialize state machine variables
  irparams.rcvstate = STATE_IDLE;
  irparams.rawlen   = 0;
  // Initialize timer
  os_timer_disarm(&timer);
  os_timer_setfn(&timer, (os_timer_func_t *)read_timeout, &timer);
  // Attach Interrupt
  attachInterrupt(irparams.recvpin, gpio_intr, CHANGE);
}

void IRrecv::disableIRIn() {
  //irReadTimer.stop();
  //os_timer_disarm(&irReadTimer);   
   ETS_INTR_LOCK();
   ETS_GPIO_INTR_DISABLE();
}

void IRrecv::resume() {
  irparams.rcvstate = STATE_IDLE;
  irparams.rawlen = 0;
}


int IRrecv::decodeESP8266(decode_results *results) {
#if DECODE_NEC
  #ifdef DEBUG
    Serial.println("Attempting NEC decode");
  #endif
  if (decodeNEC(results))  return true;
#endif
#if DECODE_SONY
  #ifdef DEBUG
   Serial.println("Attempting Sony decode");
  #endif
  if (decodeSony(results)) return true;
#endif
#if DECODE_SANYO
  #ifdef DEBUG
    Serial.println("Attempting Sanyo decode");
  #endif
  if (decodeSanyo(results))  return true;  
#endif
#if DECODE_MITSUBISHI
  #ifdef DEBUG
    Serial.println("Attempting Mitsubishi decode");
  #endif
    if (decodeMitsubishi(results)) return true;
  #endif
#if DECODE_RC5
  #ifdef DEBUG
    Serial.println("Attempting RC5 decode");
  #endif  
  if (decodeRC5(results))  return true;
#endif
#if DECODE_RC6
  #ifdef DEBUG
    Serial.println("Attempting RC6 decode");
  #endif 
  if (decodeRC6(results))  return true;
#endif
#if DECODE_PANASONIC
  #ifdef DEBUG
    Serial.println("Attempting Panasonic decode");
  #endif 
  if (decodePanasonic(results)) return true;
#endif
#if DECODE_LG_32
  #ifdef DEBUG
    Serial.println("Attempting LG 32 bits decode");
  #endif 
  if (decodeLG_32(results)) return true;
#endif
#if DECODE_LG 
  #ifdef DEBUG
    Serial.println("Attempting LG 28 bits decode");
  #endif 
  if (decodeLG(results)) return true;
#endif
#if DECODE_JVC
  #ifdef DEBUG
    Serial.println("Attempting JVC decode");
  #endif 
  if (decodeJVC(results))  return true;
#endif
#if DECODE_SAMSUNG
  #ifdef DEBUG
    Serial.println("Attempting SAMSUNG decode");
  #endif
  if (decodeSAMSUNG(results)) return true;
#endif
#if DECODE_SHARP
  #ifdef DEBUG
    Serial.println("Attempting SHARP decode");
  #endif
  if (decodeSharp(results)) return true;
#endif
#if DECODE_WHYNTER
  #ifdef DEBUG
    Serial.println("Attempting Whynter decode");
  #endif
  if (decodeWhynter(results))  return true;
#endif
#if DECODE_AIWA_RC_T501
  #ifdef DEBUG
    Serial.println("Attempting Aiwa RC-T501 decode");
  #endif
  if (decodeAiwaRCT501(results))  return true ;
#endif
#if DECODE_DENON
  #ifdef DEBUG
    Serial.println("Attempting Denon decode");
  #endif
  if (decodeDenon(results))  return true ;
#endif
#if DECODE_LEGO_PF
  #ifdef DEBUG
    Serial.println("Attempting Lego Power Functions");
  #endif
  if (decodeLegoPowerFunctions(results))  return true ;
#endif
  // decodeHash returns a hash on any input.
  // Thus, it needs to be last in the list.
  // If you add any decodes, add them before this.
#if DECODE_HASH
  if (decodeHash(results))  return true;
#endif 
return false;
}  
 
// Decodes the received IR message
// Returns 0 if no data ready, 1 if data ready.
// Results of decoding are stored in results
int IRrecv::decode(decode_results *results) {
  if (irparams.rcvstate != STATE_STOP) {
      return false;
  }
  results->rawbuf      = irparams.rawbuf;
  results->rawlen      = irparams.rawlen;
  results->command     = -1;  
  results->address     = -1; 
  #ifdef DEBUG
  dump(results) ; 
  #endif 
  //  DECODED AND it is Not is a REPEAT  
  if ( decodeESP8266(results) && (results->bits!=0)) { 
    #ifdef DEBUG
    printDecoded(results) ; 
    #endif  
    return true ; 
  } 
  resume();
  return false;
}


/* -----------------------------------------------------------------------
 * hashdecode - decode an arbitrary IR code.
 * Instead of decoding using a standard encoding scheme
 * (e.g. Sony, NEC, RC5), the code is hashed to a 32-bit value.
 *
 * The algorithm: look at the sequence of MARK signals, and see if each one
 * is shorter (0), the same length (1), or longer (2) than the previous.
 * Do the same with the SPACE signals.  Hszh the resulting sequence of 0's,
 * 1's, and 2's to a 32-bit value.  This will give a unique value for each
 * different code (probably), for most code systems.
 *
 * http://arcfn.com/2010/01/using-arbitrary-remotes-with-arduino.html
 */

// Compare two tick values, returning 0 if newval is shorter,
// 1 if newval is equal, and 2 if newval is longer
// Use a tolerance of 20%
int IRrecv::compare(unsigned int oldval, unsigned int newval) {
  if (newval < oldval * .8) {
    return 0;
  } else if ( oldval < newval * .8) {
    return 2;
  } else {
    return 1;
  }
}

// Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
#define FNV_PRIME_32 16777619
#define FNV_BASIS_32 2166136261

/* Converts the raw code values into a 32-bit hash code.
 * Hopefully this code is unique for each button.
 * This isn't a "real" decoding, just an arbitrary value.
 */

bool IRrecv::decodeHash(decode_results *results) {
  // take the dirt out when it is a noise or repetition
   if (results->rawlen < 5+ OFFSET_START)       return false;
  #ifdef DEBUG
  dump(results);
  #endif 
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

