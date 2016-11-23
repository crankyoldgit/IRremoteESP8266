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
//#include <string>
//using std::string;

 
int  IRsend::protocol2id(String protocol) {
  if (     protocol.equalsIgnoreCase("UNKNOWN"     )) return UNKNOWN;  
  else if (protocol.equalsIgnoreCase("RC5"         )) return  RC5;  
  else if (protocol.equalsIgnoreCase("RC6"         )) return  RC6;  
  else if (protocol.equalsIgnoreCase("NEC"         )) return  NEC;   
  else if (protocol.equalsIgnoreCase("SONY"        )) return  SONY;  
  else if (protocol.equalsIgnoreCase("JVC"         )) return  JVC;   
  else if (protocol.equalsIgnoreCase("PANASONIC"   )) return  PANASONIC;    
  else if (protocol.equalsIgnoreCase("SAMSUNG"     )) return  SAMSUNG;  
  else if (protocol.equalsIgnoreCase("WHYNTER"     )) return  WHYNTER;  
  else if (protocol.equalsIgnoreCase("AIWA_RC_T501")) return  AIWA_RC_T501;  
  else if (protocol.equalsIgnoreCase("LG"          )) return  LG;  
  else if (protocol.equalsIgnoreCase("SANYO"       )) return  SANYO;  
  else if (protocol.equalsIgnoreCase("MITSUBISHI"  )) return  MITSUBISHI;  
  else if (protocol.equalsIgnoreCase("DISH"        )) return  DISH;  
  else if (protocol.equalsIgnoreCase("SHARP"       )) return  SHARP; 
  else if (protocol.equalsIgnoreCase("DENON"       )) return  DENON; 
  else if (protocol.equalsIgnoreCase("PRONTO"      )) return  PRONTO; 
  else if (protocol.equalsIgnoreCase("LEGO_PF"     )) return  LEGO_PF; 
  else if (protocol.equalsIgnoreCase("COOLIX"      )) return  COOLIX; 
  else if (protocol.equalsIgnoreCase("DAIKIN"      )) return  DAIKIN; 
  else if (protocol.equalsIgnoreCase("GC"          )) return  GC;  
  else return UNKNOWN; 
}


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
  // This logic allows go back to right frequence at integer calc like 40Mhz and allow change duty cicle 
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
      delay((timeEnd-micros())/1000); // Allow other process works while waiting,like wifi
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
  } else {
    data <<= 1; 
  } 
} 

void IRsend::sendBitMarkSpaceCoded(bool bit, int timeMark ,int timeSpaceH,int timeSpaceL ) {
  mark(timeMark);   
  if (bit)  space(timeSpaceH);  
  else      space(timeSpaceL);   
}

void IRsend::enableIROut(int khz,int dutycycle) {
  // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
  // dutycycle is optional value 
  // dutycycle=2;  //50% default  value  . 
  // dutycycle=3;  //33%  
  // dutycycle=4;  //25%
  timeHigh      = 1000/(khz*dutycycle);  
  PeriodicTime  = 1000/khz;              
}

// Create to have a more transparent client implementation . 
bool IRsend::send_raw(String protocol, long long rawData, int bits) {  
  return send_raw(protocol2id(protocol), rawData, bits) ; 
}

bool IRsend::send_raw(int id , long long rawData, int bits) {  
  switch (id) {
 // default:
    #if SEND_NEC  
   case NEC          : sendNEC         (rawData, bits);  return true;  
  #endif 
  #if SEND_SONY   
   case SONY         : sendSony        (rawData, bits); return true;    
  #endif 
  #if SEND_LG   
   case LG           : sendLG          (rawData, bits); return true;  
  #endif 
  #if SEND_JVC   
   case JVC          : sendJVC         (rawData, bits); return true;   
  #endif 
  #if SEND_RC5   
   case RC5          : sendRC5         (rawData, bits); return true;  
  #endif 
  #if SEND_RC6   
   case RC6          : sendRC6         (rawData, bits); return true;  
  #endif 
  #if SEND_SAMSUNG   
   case SAMSUNG      : sendSAMSUNG     (rawData, bits); return true; 
  #endif 
  #if SEND_SHARP   
  case SHARP         : sendSharpRaw    (rawData, bits); return true;  
  #endif 
  #if SEND_MITSUBISHI   
   case MITSUBISHI   : sendMitsubishi  (rawData, bits); return true;  
  #endif 
  #if SEND_WHYNTER   
   case WHYNTER      : sendWhynter     (rawData, bits); return true;  
  #endif 
  #if SEND_DENON   
   case DENON        : sendDenon       (rawData, bits); return true;  
  #endif 
  #if SEND_COOLIX   
   case COOLIX       : sendCOOLIX     (rawData, bits); return true;  
  #endif
  #if SEND_AIWA_RC_T501   
  case  AIWA_RC_T501 :  sendAiwaRCT501(rawData    ); return true;  
  #endif
  }
  return false ; 
}
  
     
bool IRsend::send_address(String protocol, int address, int command) {  
  return send_address(protocol2id(protocol), address, command); 
}

bool IRsend::send_address(int id , int address, int command) {  
  switch (id) {
  #if SEND_NEC  
  case PANASONIC : sendPanasonic   (address, command);  return true;  
  #endif 
  #if SEND_SONY   
  case SANYO     : sendSanyo       (address, command);  return true;  
  #endif
  }
  return false ; 
}
  


