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
 ****************************************************/
//#define DEBUG true 
#include "IRremoteESP8266.h"
#include "IRremoteInt.h"

// These versions of MATCH, MATCH_MARK, and MATCH_SPACE are only for debugging.
// To use them, set DEBUG in IRremoteInt.h
// Normally macros are used for efficiency

bool  MATCH_MARK_NOLOG(int measured_ticks, int desired) {
  return  measured_ticks >= TICKS_LOW(desired) && measured_ticks <= TICKS_HIGH(desired+MARK_EXCESS);
}

bool  MATCH_SPACE_NOLOG(int measured_ticks, int desired) {
  return   measured_ticks >= TICKS_LOW(desired-MARK_EXCESS) && measured_ticks <= TICKS_HIGH(desired);
}

#ifdef DEBUG
// Changes for just log when not matches to have a clean debug 
bool  MATCH(int measured, int desired) {
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


bool  MATCH_MARK(int measured_ticks, int desired) {
  bool  ret = MATCH_MARK_NOLOG(measured_ticks,desired); 
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

bool  MATCH_SPACE(int measured_ticks, int desired) {
  bool   ret = MATCH_SPACE_NOLOG(measured_ticks,desired);
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
  bool MATCH(int measured, int desired) {return measured >= TICKS_LOW(desired) && measured <= TICKS_HIGH(desired);}
  bool MATCH_MARK(int measured_ticks, int desired) {return MATCH_MARK_NOLOG(measured_ticks, desired );}
  bool MATCH_SPACE(int measured_ticks, int desired) {return MATCH_SPACE_NOLOG(measured_ticks, desired );}
#endif

// IRsend -----------------------------------------------------------------------------------

IRsend::IRsend(int IRsendPin)
{
  IRpin = IRsendPin;
}
void IRsend::begin()
{
  pinMode(IRpin, OUTPUT);
}

void IRsend::sendCOOLIX(unsigned long data, int nbits)  {
  enableIROut(38);
  mark(COOLIX_HDR_MARK);
  space(COOLIX_HDR_SPACE);
 // Sending 3 bytes of data. Each byte first beeing sendt straight, then followed by an inverted version.
  unsigned long COOLIXmask;
  bool invert = 0;  // Initializing
  for (int j = 0; j < COOLIX_NBYTES * 2; j++) {
    for (int i = nbits; i > nbits-8; i--) {
      COOLIXmask = (unsigned long) 1 << (i-1);  // Type cast necessary to perform correct for the one byte above 16bit
      if (data & COOLIXmask) {
        mark(COOLIX_BIT_MARK);
        space(COOLIX_ONE_SPACE);
    } else {
        mark(COOLIX_BIT_MARK);
        space(COOLIX_ZERO_SPACE);
      }
    }
    data  ^= 0xFFFFFFFF;     // Inverts all of the data each time we need to send an inverted byte
    invert = !invert;
    nbits -= invert ? 0 : 8;  // Subtract 8 from nbits each time we switch to a new byte.
  }
  mark(COOLIX_BIT_MARK);
  space(COOLIX_ZERO_SPACE);   // Stop bit (0)
  space(COOLIX_HDR_SPACE);    // Pause before repeating
}

void IRsend::sendNEC(unsigned long data, int nbits) {
  enableIROut(38);
  mark(NEC_HDR_MARK);
  space(NEC_HDR_SPACE);
  for (int i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      mark(NEC_BIT_MARK);    //1T
      space(NEC_ONE_SPACE);  //1T 
    }  else {
      mark(NEC_BIT_MARK );   //1T
      space(NEC_ZERO_SPACE); //3T
    }
    data <<= 1;
  }
  mark(NEC_BIT_MARK);  //1T
  space(NEC_RPT_SPACE);
}

void IRsend::sendLG (unsigned long data, int nbits) {
  enableIROut(38);
  mark(LG_HDR_MARK);
  space(LG_HDR_SPACE);
  mark(LG_BIT_MARK);
  for (unsigned long mask = 1UL << (nbits - 1); mask; mask >>= 1) {
    if (data & mask) {
      space(LG_ONE_SPACE);
      mark(LG_BIT_MARK);
    } else {
      space(LG_ZERO_SPACE);
      mark(LG_BIT_MARK);
    }
  }
  space(0);
}

void IRsend::sendWhynter(unsigned long data, int nbits) {
  enableIROut(38);
  mark(WHYNTER_ZERO_MARK);
  space(WHYNTER_ZERO_SPACE);
  mark(WHYNTER_HDR_MARK);
  space(WHYNTER_HDR_SPACE);
  for (int i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
       mark(WHYNTER_ONE_MARK);
       space(WHYNTER_ONE_SPACE);
    } else {
       mark(WHYNTER_ZERO_MARK);
       space(WHYNTER_ZERO_SPACE);
    }
    data <<= 1;
  }
  mark(WHYNTER_ZERO_MARK);
  space(WHYNTER_ZERO_SPACE);
}

void IRsend::sendSony(unsigned long data, int nbits) {
  enableIROut(40);
  data = data << (32 - nbits);
  for (int j = 0; j < 3; j++) { // Need loop at least two times .
    unsigned long data2 = data ; 
    mark(SONY_HDR_MARK);
    space(SONY_HDR_SPACE);
    for (int i = 0; i < nbits; i++) { 
      if (data2 & TOPBIT) {
        mark(SONY_ONE_MARK);
        space(SONY_HDR_SPACE);
      } else {
        mark(SONY_ZERO_MARK);
        space(SONY_HDR_SPACE);
      }
      data2 <<= 1;
    }
    space(SONY_RPT_LENGTH); 
  } 
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

// Global Cache format w/o emitter ID or request ID. Starts from hertz, 
// followed by number of times to emit (count),
// followed by offset for repeats, followed by code as units of periodic time.
void IRsend::sendGC(unsigned int buf[], int len) {
  int khz = buf[0]/1000; // GC data starts with frequency in Hz.
  enableIROut(khz); 
  int periodic_time = 1000/khz;
  int count = buf[1]; // Max 50 as per GC.

  for (int i = 0; i < count; i++) {
    int j = i > 0 ? buf[2] + 2 : 3; // Account for offset if we're repeating, otherwise start at index 3.
    for (; j < len; j++) {
      int microseconds = buf[j] * periodic_time; // Convert periodic units to microseconds. Minimum is 80 for actual GC units.
      if (j & 1) {
        mark(microseconds); // Our codes start at an odd index (not even as with sendRaw).
      } else {
        space(microseconds);
      }
    }
  }
  space(0);
}

// Note: first bit must be a one (start bit)
void IRsend::sendRC5(unsigned long data, int nbits) {
  enableIROut(36);
  data = data << (32 - nbits);
  mark(RC5_T1);  // First start bit
  space(RC5_T1); 
  mark(RC5_T1);  // Second start bit
  for (int i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      space(RC5_T1); // 1 is space, then mark
      mark(RC5_T1);
    } else {
      mark(RC5_T1);  // 0 is mark, then space
      space(RC5_T1);
    }
    data <<= 1;
  }
  space(0); // Turn off at end
}


// Caller needs to take care of flipping the toggle bit
void IRsend::sendRC6(unsigned long data, int nbits) {
  enableIROut(36);
  data = data << (32 - nbits);
  mark(RC6_HDR_MARK);
  space(RC6_HDR_SPACE);
  mark(RC6_T1); // start bit
  space(RC6_T1);
  int t;
  for (int i = 0; i < nbits; i++) {
    if (i == 3) {
      // double-wide trailer bit
      t = 2 * RC6_T1;
    } else {
      t = RC6_T1;
    }
    if (data & TOPBIT) {
      mark(t);   // 1 is mark, then space
      space(t);
    } else {
      space(t);
      mark(t);
    }
    data <<= 1;
  }
  space(0); // Turn off at end
}

void IRsend::sendPanasonic(unsigned int address, unsigned long data) {
    enableIROut(35);
    mark(PANASONIC_HDR_MARK);
    space(PANASONIC_HDR_SPACE);
    for(int i=0;i<16;i++) {
        mark(PANASONIC_BIT_MARK);       // T 
        if (address & 0x8000) {
            space(PANASONIC_ONE_SPACE); // 3T 
        } else {
            space(PANASONIC_ZERO_SPACE); // T
        }
        address <<= 1;        
    }    
    for (int i=0; i < 32; i++) {
        mark(PANASONIC_BIT_MARK);
        if (data & TOPBIT) {
            space(PANASONIC_ONE_SPACE);
        } else {
            space(PANASONIC_ZERO_SPACE);
        }
        data <<= 1;
    }
    mark(PANASONIC_BIT_MARK);
    space(0);
}

void IRsend::sendJVC(unsigned long data, int nbits, int repeat) {
  enableIROut(38);
  data = data << (32 - nbits);
  if (!repeat){
     mark(JVC_HDR_MARK);
     space(JVC_HDR_SPACE);
  }
  for (int i = 0; i < nbits; i++) {
     if (data & TOPBIT) {
        mark(JVC_BIT_MARK);
        space(JVC_ONE_SPACE);
      } else {
         mark(JVC_BIT_MARK);
         space(JVC_ZERO_SPACE);
      }
      data <<= 1;
  }
  mark(JVC_BIT_MARK);
  space(0);
}

void IRsend::sendSAMSUNG(unsigned long data, int nbits) {
  enableIROut(38);
  mark(SAMSUNG_HDR_MARK);
  space(SAMSUNG_HDR_SPACE);
  for (int i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      mark(SAMSUNG_BIT_MARK);
      space(SAMSUNG_ONE_SPACE);
    } else {
      mark(SAMSUNG_BIT_MARK);
      space(SAMSUNG_ZERO_SPACE);
    }
    data <<= 1;
  }
  mark(SAMSUNG_BIT_MARK);
  space(0);
}

void IRsend::mark(int time) {
  // Sends an IR MARK for the specified number of microseconds.
  // The MARK output is modulated at the PWM frequency.
  int  timeLow = PeriodicTime-timeHigh; 
  // this logic allows go back to right frequence at integer calc like 40Mhz and allow change duty cicle 
  long timeEnd = micros()+time; 
  while(micros() < timeEnd){
    digitalWrite(IRpin, HIGH);
    delayMicroseconds(timeHigh );
    digitalWrite(IRpin, LOW);
    delayMicroseconds(timeLow); 
   }
}


/* Leave pin off for time (given in microseconds) */
void IRsend::space(int time) {
  // Sends an IR space for the specified number of microseconds.
  // A space is no output, so the PWM output is disabled.
  digitalWrite(IRpin, LOW);
  if (time > 0 ) { 
    if   (time < 1000) delayMicroseconds(time);
    else {
      long timeEnd = micros()+time; 
      delay((timeEnd-micros())/1000); // allow others process works while waiting like wifi
      long finalTime=timeEnd-micros(); 
      if  (finalTime >0) delayMicroseconds(finalTime);
    }
  }
}

void IRsend::enableIROut(int khz) {
  // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
  int dutycycle; 
  dutycycle=2;    //50% sounds the best on tests , Lircs have it as parameter  better latter to be generic  . 
  //dutycycle=3;  //33%
  //dutycycle=4;  //25%
  timeHigh      = 1000/(khz*dutycycle);  
  PeriodicTime  = 1000/khz;              
}

/* Sharp support

The Dish send function needs to be repeated 4 times, and the Sharp function
has the necessary repeat built in because of the need to invert the signal.

Sharp protocol documentation:
http://www.sbprojects.com/knowledge/ir/sharp.htm

Here are the LIRC files that I found that seem to match the remote codes
from the oscilloscope:

Sharp LCD TV:
http://lirc.sourceforge.net/remotes/sharp/GA538WJSA

DISH NETWORK (echostar 301):
http://lirc.sourceforge.net/remotes/echostar/301_501_3100_5100_58xx_59xx

For the DISH codes, only send the last for characters of the hex.
i.e. use 0x1C10 instead of 0x0000000000001C10 which is listed in the
linked LIRC file.
*/

void IRsend::sendSharpRaw(unsigned long data, int nbits) {
  enableIROut(38);
  // Sending codes in bursts of 3 (normal, inverted, normal) makes transmission
  // much more reliable. That's the exact behaviour of CD-S6470 remote control.
  for (int n = 0; n < 3; n++) {
    for (int i = 1 << (nbits-1); i > 0; i>>=1) {
      if (data & i) {
        mark(SHARP_BIT_MARK);
        space(SHARP_ONE_SPACE);
      }
      else {
        mark(SHARP_BIT_MARK);
        space(SHARP_ZERO_SPACE);
      }
    }
    mark(SHARP_BIT_MARK);
    space(SHARP_ZERO_SPACE);
    delay(40);
    data = data ^ SHARP_TOGGLE_MASK;  // invert data
  }
}

// Sharp send compatible with data obtained through decodeSharp
void IRsend::sendSharp(unsigned int address, unsigned int command) {
  sendSharpRaw((address << 10) | (command << 2) | 2, 15);
}

void IRsend::sendDISH(unsigned long data, int nbits) {
  enableIROut(56);
  mark(DISH_HDR_MARK);
  space(DISH_HDR_SPACE);
  for (int i = 0; i < nbits; i++) {
    if (data & DISH_TOP_BIT) {
      mark(DISH_BIT_MARK);
      space(DISH_ONE_SPACE);
    } else {
      mark(DISH_BIT_MARK);
      space(DISH_ZERO_SPACE);
    }
    data <<= 1;
  }
} 

// From https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/tree/master/IRremote
void IRsend::sendDaikin(unsigned char daikin[]) {
  sendDaikinChunk(daikin, 8,0);
  delay(29);
  sendDaikinChunk(daikin, 19,8);
}

void IRsend::sendDaikinChunk(unsigned char buf[], int len, int start) {
  int data2;
  enableIROut(38);
  mark(DAIKIN_HDR_MARK);
  space(DAIKIN_HDR_SPACE);
  for (int i = start; i < start+len; i++) {
    data2=buf[i];
    for (int j = 0; j < 8; j++) {
      if ((1 << j & data2)) {
        mark(DAIKIN_ONE_MARK);
        space(DAIKIN_ONE_SPACE);
      }
      else {
        mark(DAIKIN_ZERO_MARK);
        space(DAIKIN_ZERO_SPACE);
      }
    }
  }
  mark(DAIKIN_ONE_MARK);
  space(DAIKIN_ZERO_SPACE);
}

// ---------------------------------------------------------------
  

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
    irparams.rawbuf[irparams.rawlen++] = 20 ; //include an initial space to keep compatible with IR-Remote
  } else if (irparams.rawlen < RAWBUF) {
    irparams.rawbuf[irparams.rawlen++] = (now-start)/ USECPERTICK ;
  }
  start = now;
  os_timer_disarm(&timer);
  os_timer_arm(&timer, 12, 0);  
  // Changed to wait for new data just 12ms !, this  avoid repetitions at the receiver .
  // at sony we can see 14.5 ms in RM-AAU023  
}

IRrecv::IRrecv(int recvpin) {
  irparams.recvpin = recvpin;
}

// initialization
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

void dump(decode_results *results) {
  Serial.print("Dump: #results : ") ; 
  Serial.println(results->rawlen) ; 
  for (int i = 1; i < results->rawlen; i++) {
     Serial.print( (i==0 ||!(i % 2)) ? "+" : "-");  

     Serial.print(results->rawbuf[i]); 
     Serial.print(" ") ; 
   } 
   Serial.println("") ; 
}

void  printDecoded(decode_results *results) {
  Serial.print("DECODED Type=") ; 
  Serial.print(results->decode_type);
  Serial.print(" Data= ") ;
  Serial.print(results->value);
  Serial.print(" Bits= "); 
  Serial.print(results->bits);
  if (results->device  >-1)  Serial.print(" Device= " ); Serial.print(results->device ,HEX); 
  if (results->command >-1)  Serial.print(" Command= "); Serial.print(results->command,HEX) ; 
  Serial.println(); 
} 

void addBit(unsigned  long  &data,bool  bit) {
  if (bit) {
    data = (data << 1) | 1;
  } else {
    data <<= 1; 
  } 
} 

void addBit(unsigned int &data,bool bit) {
  if (bit) {
    data = (data << 1) | 1;
  } else {
    data <<= 1; 
  } 
} 

// No log if not matches to clen up debuggind when decoding data 
int addBitIfMatch(unsigned long &data,int val,int test, int timeH, int timeL) { 
 if ( test == MARK) { 
    if (MATCH_MARK_NOLOG(val, timeH)) {
      addBit(data,1) ;
    } else if (MATCH_MARK_NOLOG(val , timeL)) {
      addBit(data,0) ;
    } else {
      #ifdef DEBUG
      Serial.print("Wrong MARK :") ;Serial.println(val) ; 
      #endif
      return false;  
   }
  } else  if ( test == SPACE ) { 
    if (MATCH_SPACE_NOLOG(val, timeH)) {
      addBit(data,1) ;
    } else if (MATCH_SPACE_NOLOG(val, timeL)) {
      addBit(data,0) ;
    } else {
      #ifdef DEBUG
      Serial.print("Wrong SPACE :") ; Serial.println(val) ; 
      #endif
      return false; 
    }
  }
  return true; 
} 

int IRrecv::decodeESP8266(decode_results *results) {
  
#ifdef DEBUG
  Serial.println("Attempting NEC decode");
#endif
  if (decodeNEC(results))  return DECODED;
#ifdef DEBUG
  Serial.println("Attempting Sony decode");
#endif
  if (decodeSony(results)) return DECODED;
  /*
#ifdef DEBUG
  Serial.println("Attempting Sanyo decode");
#endif
  if (decodeSanyo(results))  return DECODED;
*/  
#ifdef DEBUG
  Serial.println("Attempting Mitsubishi decode");
#endif
  if (decodeMitsubishi(results)) return DECODED;
#ifdef DEBUG
  Serial.println("Attempting RC5 decode");
#endif  
  if (decodeRC5(results))  return DECODED;
#ifdef DEBUG
  Serial.println("Attempting RC6 decode");
#endif 
  if (decodeRC6(results))  return DECODED;
#ifdef DEBUG
  Serial.println("Attempting Panasonic decode");
#endif 
  if (decodePanasonic(results)) return DECODED;
 #ifdef DEBUG
  Serial.println("Attempting LG decode");
#endif 
  if (decodeLG(results)) return DECODED;
#ifdef DEBUG
  Serial.println("Attempting JVC decode");
#endif 
  if (decodeJVC(results))  return DECODED;
#ifdef DEBUG
  Serial.println("Attempting SAMSUNG decode");
#endif
  if (decodeSAMSUNG(results)) return DECODED;
#ifdef DEBUG
  Serial.println("Attempting Whynter decode");
#endif
  if (decodeWhynter(results))  return DECODED;
  // decodeHash returns a hash on any input.
  // Thus, it needs to be last in the list.
  // If you add any decodes, add them before this.
  if (decodeHash(results))  return DECODED;
  return ERR;  
}  
 
// Decodes the received IR message
// Returns 0 if no data ready, 1 if data ready.
// Results of decoding are stored in results
int IRrecv::decode(decode_results *results) {
  if (irparams.rcvstate != STATE_STOP) {
      return ERR;
  }
  results->rawbuf = irparams.rawbuf;
  results->rawlen = irparams.rawlen;
  results->command     = -1;  
  results->device      = -1; 
  #ifdef DEBUG
  dump(results) ; 
  #endif 
  //  DECODED AND NOT REPEAT
  if ( decodeESP8266(results) && (results->bits!=0)) { 
    #ifdef DEBUG
    printDecoded(results) ; 
    #endif  
    return DECODED ; 
  } 
  resume();
  return ERR;
}


// Calculate data based on  device and commnd .  
unsigned long hashNEC(unsigned int device ,unsigned  int command ) {
return ( device << 24) + ((device ^ 0xFF) << 16) + ( command <<  8) + (command ^ 0xFF); 
}


// NECs have a repeat only 4 items long ,
// so the repeat is not comming anymore as time waiting was changed to 12 ms 
long IRrecv::decodeNEC(decode_results *results) {
  if (irparams.rawlen < 2 * NEC_BITS + 2) return ERR; // 66
  unsigned   long data = 0;
  int offset = 1; 
  if (!MATCH_MARK(results->rawbuf[offset++], NEC_HDR_MARK))   return ERR; //9 ms
  if (!MATCH_SPACE(results->rawbuf[offset++], NEC_HDR_SPACE)) return ERR; //4.5ms  repeat  code      8800  2200
   // repeat  code    40.5ms   8800us  2200us While key is pressed 
  unsigned  int device  =0; 
  unsigned  int command =0; 
  for (int i = 0; i < NEC_BITS; i++) {
    if (!MATCH_MARK(results->rawbuf[offset++], NEC_BIT_MARK))                               return ERR;
    if (!addBitIfMatch(data,results->rawbuf[offset++],SPACE,NEC_ONE_SPACE,NEC_ZERO_SPACE))  return ERR;
   }
  device  = (data & 0xFF000000) >> 24;  // Most significant  two bytes 
  command = (data & 0x0000FF00) >>  8;  // byte 4 and 3 
  // integrity check command ,  device and command is sent twice , one as plain and other inverted .  
  if ( !( device ^ 0xFF == ((data & 0x00FF0000) >> 16) ) ||!( command ^ 0xFF == (data & 0x000000FF) ))  {
    #ifdef DEBUG
    Serial.println("NEC Integrity fails it is not a true NEC  ") ; 
    #endif 
    return ERR;
  } 
  // Success
  results->bits    = NEC_BITS;
  results->value   = data;
  results->command = command ; 
  results->device  = device ; 
  results->decode_type = NEC;
  #ifdef DEBUG
  // testing from device,command to value to use latter 
  Serial.println(String(hashNEC(device,command),HEX)); 
  #endif  
  return DECODED;
}

int getBit(unsigned long value , int bit ) {
return  value >>bit & 1; 
}

long IRrecv::decodeSony(decode_results *results) {
  //SONY protocol, SIRC (Serial Infra-Red Control) can be  12,15,20 bits long 
  unsigned long data  = 0;
  int offset = 1; //skip first space
  int nbits  = 0; 
  if (!MATCH_MARK(results->rawbuf[offset++], SONY_HDR_MARK))    return ERR; //START 4T
  while (offset+1 < irparams.rawlen) {
    if (!MATCH_MARK(results->rawbuf[offset++], SONY_HDR_SPACE)) return ERR; // 1T
    if (!addBitIfMatch(data,results->rawbuf[offset++],SPACE,NEC_ONE_SPACE,NEC_ZERO_SPACE))   return ERR;
    nbits++; 
  }
  if (!((12 == nbits )||(15 == nbits )||(20 == nbits ))) {
    return ERR;
  } 
  // spare command and address device from data 
  unsigned int  command  = 0;
  unsigned int  device   = 0;
  // reverse order 
  for (int i =0; i <nbits; i++) {
     // 12-bit version, 7 command bits, 5 device bits.
     if ( nbits == 12 )  { 
       if (i >= 5 && i <=11)  {     
          addBit(command,getBit(data,i));
       } else if (i >=0 &&  i <= 4) {    
        addBit(device,getBit(data,i)); 
       }
    // 15-bit version, 7 command bits, 8 device bits.
    } else if ( nbits == 15 )  { 
       if (i >= 8 && i <=14)  {    
       addBit(command,getBit(data,i));
       } else if (i >=0 && i <= 7) {   
          addBit(device,getBit(data,i)); 
       }
    // 20-bit version, 7 command bits, 5 device bits, 8 extended bits. 
    } else if ( nbits == 20)  { 
       if ((i >= 0 && i <=7) ||(i >= 13 && i <=19)) {    
         addBit(command,getBit(data,i));
       } else if (i >=8 && i <= 12) {   
          addBit(device,getBit(data,i)); 
       }
    } 
  } 
  // Success
  results->bits        = nbits ; 
  results->value       = data;
  results->command     = command; 
  results->device      = device; 
  results->decode_type = SONY;
  return DECODED;
}

long IRrecv::decodeWhynter(decode_results *results) {
  if (irparams.rawlen < 2 * WHYNTER_BITS + 6) return ERR;
  unsigned   long data = 0;
  int offset = 1; // Skip first space
  // sequence begins with a bit markand a zero space
  if (!MATCH_MARK(results->rawbuf[offset++], WHYNTER_BIT_MARK ))   return ERR;
  if (!MATCH_SPACE(results->rawbuf[offset++], WHYNTER_ZERO_SPACE)) return ERR;
  // header markand space
  if (!MATCH_MARK(results->rawbuf[offset++], WHYNTER_HDR_MARK ))   return ERR;
  if (!MATCH_SPACE(results->rawbuf[offset++], WHYNTER_HDR_SPACE )) return ERR;
  // data bits
  for (int i = 0; i < WHYNTER_BITS; i++) {
    if (!MATCH_MARK(results->rawbuf[offset++], WHYNTER_BIT_MARK)) return ERR;
    if (!addBitIfMatch(data,results->rawbuf[offset++],SPACE,WHYNTER_ONE_SPACE, WHYNTER_ZERO_SPACE)) return ERR;
  }
  // trailing mark
  if (!MATCH_MARK(results->rawbuf[offset], WHYNTER_BIT_MARK)) {
    return ERR;
  }
  // Success
  results->bits        = WHYNTER_BITS;
  results->value       = data;
  results->decode_type = WHYNTER;
  return DECODED;
}

// I think this is a Sanyo decoder - serial = SA 8650B
// Looks like Sony except for timings, 48 chars of data and time/space different
long IRrecv::decodeSanyo(decode_results *results) {
  if (irparams.rawlen < 2 * SANYO_BITS + 2) return ERR;
  unsigned   long data = 0;
  int offset = 1; // Skip first space 
  // Initial space  
  // Put this back in for debugging - note can't use #DEBUG as if Debug on we don't see the repeat cos of the delay
  Serial.print("IR Gap: ");
  Serial.println( results->rawbuf[offset]);
  Serial.println( "test against:");
  Serial.println(results->rawbuf[offset]);
 
  if (results->rawbuf[offset] < SANYO_DOUBLE_SPACE_USECS) {
    // Serial.print("IR Gap found: ");
    results->bits        = 0;
    results->value       = REPEAT;
    results->decode_type = SANYO;
    return DECODED;
  } 
  // Initial mark
  if (!MATCH_MARK(results->rawbuf[offset++], SANYO_HDR_MARK))  return ERR;
  offset++;  //Note included skip space TODO check length to fix 
  // Skip Second Mark
  if (!MATCH_MARK(results->rawbuf[offset++], SANYO_HDR_MARK))  return ERR;
  int bits=0;
  while (offset + 1 < irparams.rawlen) {
    if (!MATCH_SPACE(results->rawbuf[offset++], SANYO_HDR_SPACE)) break;
    if (!addBitIfMatch(data,results->rawbuf[offset++],MARK,SANYO_ONE_MARK, SANYO_ZERO_MARK) ) return ERR;
    bits++; 
  }
  if (bits< 12) return ERR;
  // Success
  results->bits        = bits; 
  results->value       = data;
  results->decode_type = SANYO;
  return DECODED;
}

// Looks like Sony except for timings, 48 chars of data and time/space different
long IRrecv::decodeMitsubishi(decode_results *results) {
  // Serial.print("?!? decoding Mitsubishi:");Serial.print(irparams.rawlen); Serial.print(" want "); Serial.println( 2 * MITSUBISHI_BITS + 2);
  unsigned   long data = 0;
  if (irparams.rawlen < 2 * MITSUBISHI_BITS + 2) {
    return ERR;
  }
 int offset = 1; // Skip first space
  // Initial space  
  /* Put this back in for debugging - note can't use #DEBUG as if Debug on we don't see the repeat cos of the delay
  Serial.print("IR Gap: ");
  Serial.println( results->rawbuf[offset]);
  Serial.println( "test against:");
  Serial.println(results->rawbuf[offset]);
  */
  /* Not seeing double keys from Mitsubishi
  if (results->rawbuf[offset] < MITSUBISHI_DOUBLE_SPACE_USECS) {
    // Serial.print("IR Gap found: ");
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = MITSUBISHI;
    return DECODED;
  }
  */
  offset++;  //  TODO check it 
  // Typical
  // 14200 7 41 7 42 7 42 7 17 7 17 7 18 7 41 7 18 7 17 7 17 7 18 7 41 8 17 7 17 7 18 7 17 7 
  // Initial Space
  if (!MATCH_MARK(results->rawbuf[offset++], MITSUBISHI_HDR_SPACE)) return ERR;
  int bits=0;
  while (offset + 1 < irparams.rawlen) {
    if (!addBitIfMatch(data,results->rawbuf[offset++],MARK,MITSUBISHI_ONE_MARK, MITSUBISHI_ZERO_MARK))  return ERR;
    if (!MATCH_SPACE(results->rawbuf[offset++], MITSUBISHI_HDR_SPACE)) return ERR;
    bits++; 
  }
  if (bits < MITSUBISHI_BITS) return ERR;
  // Success
  results->value       = bits; 
  results->value       = data;
  results->decode_type = MITSUBISHI;
  return DECODED;
}

// Gets one undecoded level at a time from the raw buffer.
// The RC5/6 decoding is easier if the data is broken into time intervals.
// E.g. if the buffer has markfor 2 time intervals and SPACE for 1,
// successive calls to getRClevel will return MARK, MARK, SPACE.
// offset and used are updated to keep track of the current position.
// t1 is the time interval for a single bit in microseconds.
// Returns -1 for error (measured time interval is not a multiple of t1).
int IRrecv::getRClevel(decode_results *results, int *offset, int *used, int t1) {
  if (*offset >= results->rawlen) {
    // After end of recorded buffer, assume SPACE.
    return SPACE;
  }
  int width = results->rawbuf[*offset];
  int val = ((*offset) % 2) ? MARK : SPACE;
  int correction = (val == MARK) ? MARK_EXCESS : - MARK_EXCESS;
  int avail;
  if (MATCH(width, t1 + correction)) {
    avail = 1;
  } else if (MATCH(width, 2*t1 + correction)) {
    avail = 2;
  } else if (MATCH(width, 3*t1 + correction)) {
    avail = 3;
  } else {
    return -1;
  }
  (*used)++;
  if (*used >= avail) {
    *used = 0;
    (*offset)++;
  }
#ifdef DEBUG
  if (val == MARK) {
    Serial.println("MARK");
  } else {
    Serial.println("SPACE");
  }
#endif
  return val;   
}

long IRrecv::decodeRC5(decode_results *results) {
  if (irparams.rawlen < MIN_RC5_SAMPLES + 2) {
    return ERR;
  }
  int offset = 1; // Skip gap space
  unsigned long data = 0;
  int used  = 0;
  // Get start bits
  if (getRClevel(results, &offset, &used, RC5_T1) != MARK ) return ERR;
  if (getRClevel(results, &offset, &used, RC5_T1) != SPACE) return ERR;
  if (getRClevel(results, &offset, &used, RC5_T1) != MARK ) return ERR;
  int nbits;
  for (nbits = 0; offset < irparams.rawlen; nbits++) {
    int levelA = getRClevel(results, &offset, &used, RC5_T1); 
    int levelB = getRClevel(results, &offset, &used, RC5_T1);
    if (levelA == SPACE && levelB == MARK) {
      data = (data << 1) | 1;
    } else if (levelA == MARK && levelB == SPACE) {
      data <<= 1;
    } else {
      return ERR;
    } 
  }
  // Success
  results->bits        = nbits;
  results->value       = data;
  results->decode_type = RC5;
  return DECODED;
}

long IRrecv::decodeRC6(decode_results *results) {
  if (results->rawlen < MIN_RC6_SAMPLES) {
    return ERR;
  }
  int offset = 1; // Skip first space
  // Initial mark
  if (!MATCH_MARK(results->rawbuf[offset++], RC6_HDR_MARK))   return ERR;
  if (!MATCH_SPACE(results->rawbuf[offset++], RC6_HDR_SPACE)) return ERR;
  long data = 0;
  int used  = 0;
  // Get start bit (1)
  if (getRClevel(results, &offset, &used, RC6_T1) != MARK ) return ERR;
  if (getRClevel(results, &offset, &used, RC6_T1) != SPACE) return ERR;
  int nbits;
  for (nbits = 0; offset < results->rawlen; nbits++) {
    int levelA, levelB; // Next two levels
    levelA = getRClevel(results, &offset, &used, RC6_T1); 
    if (nbits == 3) {
      // T bit is double wide; make sure second half matches
      if (levelA != getRClevel(results, &offset, &used, RC6_T1)) return ERR;
    } 
    levelB = getRClevel(results, &offset, &used, RC6_T1);
    if (nbits == 3) {
      // T bit is double wide; make sure second half matches
      if (levelB != getRClevel(results, &offset, &used, RC6_T1)) return ERR;
    } 
    if (levelA == MARK && levelB == SPACE) { // reversed compared to RC5
      // 1 bit
      data = (data << 1) | 1;
    } else if (levelA == SPACE && levelB == MARK) {
      // zero bit
      data <<= 1;
    } else {
      return ERR; // Error
    } 
  }
  // Success
  results->bits        = nbits;
  results->value       = data;
  results->decode_type = RC6;
  return DECODED;
}

long IRrecv::decodePanasonic(decode_results *results) {
  unsigned long data = 0;
  int offset = 1;    
  if (!MATCH_MARK(results->rawbuf[offset++], PANASONIC_HDR_MARK))  return ERR;
  if (!MATCH_MARK(results->rawbuf[offset++], PANASONIC_HDR_SPACE)) return ERR;
  // decode address
  for (int i = 0; i < PANASONIC_BITS; i++) {
    if (!MATCH(results->rawbuf[offset++], PANASONIC_BIT_MARK))  return ERR;
    if (!addBitIfMatch(data,results->rawbuf[offset++],SPACE,PANASONIC_ONE_SPACE,PANASONIC_ZERO_SPACE)) return ERR;
  }
  // Success
  results->bits             = PANASONIC_BITS;
  results->value            = (unsigned long)data;
  results->panasonicAddress = (unsigned int)(data >> 32); 
  results->decode_type      = PANASONIC;
  return DECODED;
}

long IRrecv::decodeLG(decode_results *results) {
    if (irparams.rawlen < 2 * LG_BITS + 1 ) return ERR;
    unsigned   long data = 0;
    int offset = 0; 
    if (!MATCH_MARK(results->rawbuf[offset++], LG_HDR_MARK))   return ERR;
    if (!MATCH_SPACE(results->rawbuf[offset++], LG_HDR_SPACE)) return ERR;
    for (int i = 0; i < LG_BITS; i++) {
      if (!MATCH_MARK(results->rawbuf[offset++], LG_BIT_MARK)) return ERR;
      if (!addBitIfMatch(data,results->rawbuf[offset++],SPACE,LG_ONE_SPACE,LG_ZERO_SPACE))   return ERR;
    }
    if (!MATCH_MARK(results->rawbuf[offset], LG_BIT_MARK)) return ERR;
    // Success
    results->bits        = LG_BITS;
    results->value       = data;
    results->decode_type = LG;
    return DECODED;
}

long IRrecv::decodeJVC(decode_results *results) {
  if (irparams.rawlen < 2 * JVC_BITS + 1 )  return ERR;
  unsigned long data = 0;
  int offset = 0;
  /*
   offset = 1; // Skip first 
  // Check for repeat
  if (irparams.rawlen - 1 == 33 &&
       MATCH_MARK(results->rawbuf[offset], JVC_BIT_MARK) &&
       MATCH_MARK(results->rawbuf[irparams.rawlen-1], JVC_BIT_MARK)) {
      results->bits = 0;
      results->value = REPEAT;
      results->decode_type = JVC;
      return DECODED;
  } 
  */
  // Initial mark  
  if (!MATCH_MARK(results->rawbuf[offset++],  JVC_HDR_MARK))  return ERR;
  if (!MATCH_SPACE(results->rawbuf[offset++], JVC_HDR_SPACE)) return ERR;
  for (int i = 0; i < JVC_BITS; i++) {
    if (!MATCH_MARK(results->rawbuf[offset++], JVC_BIT_MARK))  return ERR;
    if (!addBitIfMatch(data,results->rawbuf[offset++],SPACE,JVC_ONE_SPACE,JVC_ZERO_SPACE) )  return ERR;
  }
  if (!MATCH_MARK(results->rawbuf[offset], JVC_BIT_MARK))  return ERR;
  // Success
  results->bits        = JVC_BITS;
  results->value       = data;
  results->decode_type = JVC;
  return DECODED;
}

// SAMSUNGs have a repeat only 4 items long
long IRrecv::decodeSAMSUNG(decode_results *results) {
  if (irparams.rawlen < 2 * SAMSUNG_BITS + 2) return ERR;
  unsigned   long data = 0;
  int offset = 1;  
  /*
   // Check for repeat commented as blocked earlier 
  if (irparams.rawlen == 4 &&
    MATCH_SPACE(results->rawbuf[offset], SAMSUNG_RPT_SPACE) &&
    MATCH_MARK(results->rawbuf[offset+1], SAMSUNG_BIT_MARK)) {
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = SAMSUNG;
    return DECODED;
  }
  */
  // Initial space  
  if (!MATCH_MARK(results->rawbuf[offset++], SAMSUNG_HDR_MARK))   return ERR;
  if (!MATCH_SPACE(results->rawbuf[offset++], SAMSUNG_HDR_SPACE)) return ERR;
  bool  bit = 0; 
  for (int i = 0; i < SAMSUNG_BITS; i++) {
    if (!MATCH_MARK(results->rawbuf[offset++], SAMSUNG_BIT_MARK)) return ERR;
    if (!addBitIfMatch(data,results->rawbuf[offset++],SPACE,SAMSUNG_ONE_SPACE,SAMSUNG_ZERO_SPACE))  return ERR;
  }
  // Success
  results->bits        = SAMSUNG_BITS;
  results->value       = data;
  results->decode_type = SAMSUNG;
  return DECODED;
}

// From https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/tree/master/IRremote
// decoding not actually tested
long IRrecv::decodeDaikin(decode_results *results) {
  unsigned long data  = 0;
  int offset = 1; // Skip first space
  if (irparams.rawlen < 2 * DAIKIN_BITS + 4) {
    //return ERR;
  }
  // Initial mark
  if (!MATCH_MARK(results->rawbuf[offset++] , DAIKIN_HDR_MARK))   return ERR;
  if (!MATCH_SPACE(results->rawbuf[offset++], DAIKIN_HDR_SPACE))  return ERR;
  for (int i = 0; i < 32; i++) {
    if (!MATCH_MARK(results->rawbuf[offset++], DAIKIN_ONE_MARK))  return ERR;
    if (!addBitIfMatch(data,results->rawbuf[offset++],SPACE,DAIKIN_ONE_SPACE,DAIKIN_ZERO_SPACE))  return ERR;
  }
  unsigned long number = data ; // some number...
  int bits = 32 ; // nr of bits in some number
  unsigned long reversed = 0;
  for ( int b=0 ; b < bits ; b++ ) reversed = ( reversed << 1 ) | ( 0x0001 & ( number >> b ) );
  Serial.print ("Code ");
  Serial.println (reversed,  HEX);
  //==========
  for (int i = 0; i < 32; i++) {
    if (!MATCH_MARK(results->rawbuf[offset++], DAIKIN_ONE_MARK))   return ERR;
    if (!addBitIfMatch(data,results->rawbuf[offset++],SPACE,DAIKIN_ONE_SPACE,DAIKIN_ZERO_SPACE))  return ERR;
  }
  number = data ; // some number...
  bits = 32 ; // nr of bits in some number
  reversed = 0;
  for ( int b=0 ; b < bits ; b++ ) reversed = ( reversed << 1 ) | ( 0x0001 & ( number >> b ) );
  //Serial.print ("Code2 ");
  //Serial.println (reversed,  HEX);
  //===========
  if (!MATCH_SPACE(results->rawbuf[offset++], 29000)) {
    //Serial.println ("no gap");
    return ERR;
  }
  // Success
  results->bits        = DAIKIN_BITS;
  results->value       = reversed;
  results->decode_type = DAIKIN;
  return DECODED;
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


long IRrecv::decodeHash(decode_results *results) {
  if (results->rawbuf[0]<1000) { // take the dirt out when noise or repetition  
    return ERR;
  }
  if (results->rawlen < 6) {
    return ERR;
  }
  dump(results); 
  long hash = FNV_BASIS_32;
  for (int i = 1; i+2 < results->rawlen; i++) {
    int value =  compare(results->rawbuf[i], results->rawbuf[i+2]);
    // Add value into the hash
    hash = (hash * FNV_PRIME_32) ^ value;
  }
  results->value       = hash;
  results->bits        = 32;
  results->decode_type = UNKNOWN;
  return DECODED;
}

// ---------------------------------------------------------------
