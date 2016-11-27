#include "IRremote.h"
#include "IRremoteInt.h"

//+=============================================================================
void  IRsend::sendRaw (const unsigned int buf[],  unsigned int len,  unsigned int hz)
{
	// Set IR carrier frequency
	enableIROut(hz);

	for (unsigned int i = 0;  i < len;  i++) {
		if (i & 1)  space(buf[i]) ;
		else        mark (buf[i]) ;
	}

	space(0);  // Always end with the LED off
}

//+=============================================================================
// Sends an IR mark for the specified number of microseconds.
// The mark output is modulated at the PWM frequency.
//
void  IRsend::mark (unsigned int time) {
  // Sends an IR MARK for the specified number of microseconds.
  // The MARK output is modulated at the PWM frequency.
  int  timeLow = PeriodicTime-timeHigh; 
  // This logic allows go back to right frequence at integer calc like 40Mhz and allow change duty cicle 
  long timeEnd = micros()+time; 
  while(micros() < timeEnd){
    digitalWrite(IRpin, HIGH);
    delayMicroseconds(timeHigh );
    digitalWrite(IRpin, LOW);
    delayMicroseconds(timeLow); 
  }
}

//+=============================================================================
// Leave pin off for time (given in microseconds)
// Sends an IR space for the specified number of microseconds.
// A space is no output, so the PWM output is disabled.
//
void  IRsend::space(unsigned int time) {
  // Sends an IR space for the specified number of microseconds.
  // A space is no output, so the PWM output is disabled.
  digitalWrite(IRpin, LOW);
  if (time > 0 ) { 
    if   (time < 1000) delayMicroseconds(time);
    else {
      long timeEnd = micros()+time; 
      delay((timeEnd-micros())/1000); // Allow other process works while waiting,like wifi
      long finalTime=timeEnd-micros(); 
      if  (finalTime >0) delayMicroseconds(finalTime);
    }
  }
}
//+=============================================================================
// Enables IR output.  The khz value controls the modulation frequency in kilohertz.
// The IR output will be on pin defined .

void IRsend::enableIROut(int khz,int dutycycle) {
  // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
  // dutycycle is optional value 
  // dutycycle=2;  //50% default  value  . 
  // dutycycle=3;  //33%  
  // dutycycle=4;  //25%
  timeHigh      = 1000/(khz*dutycycle);  
  PeriodicTime  = 1000/khz;              
}



IRsend::IRsend(int IRsendPin)
{
  IRpin = IRsendPin;
}

void IRsend::begin()
{
  pinMode(IRpin, OUTPUT);
}




void IRsend::space_encode (bool bit,int timeH, int timeL ) {
  if (bit) {
     space(timeH); 
  } else { 
     space(timeL); 
  }
}

void IRsend::mark_encode (bool bit,int timeH, int timeL ) {
  if (bit) {
     mark(timeH); 
  } else { 
     mark(timeL); 
  }
}

void IRsend::addBit(unsigned long long &data,bool  bit) {
  if (bit) {
    data = (unsigned  long long ) (data << 1) | 1;
  } else {
    data <<= 1; 
  } 
}

//Convert hex string to long long 64bits  
long long s16toll(String hex_value) {
  long long ll = 0;
  for (int i = hex_value.length() - 1; i >= 0; i--) {
    char ch = hex_value.charAt(i);
    int val = int(ch);
    if (ch >= '0' && ch <= '9') {
       val -= 48;  // '0' -> 48
    } else if (ch >= 'A' && ch <= 'F') {
       val -= 55;  // 'A' 65   65-10 = 55 
    } else if (ch >= 'a' && ch <= 'f') {
       val -= 87;  // 'A' 97   97-10 = 87 
    }
    ll = ll << 4 | val ;
  }
  return ll;
}

// Created to simplify client implementation and allow long long conversion 
bool IRsend::send_raw(String protocol, String hexRawData, int bits) {  
  return send_raw(protocol,(long long ) s16toll(hexRawData),bits) ; 
}  

// Created to simplify client implementation . 
bool IRsend::send_raw(String protocol, long long rawData, int bits) {  
  return send_raw(protocol2id(protocol), rawData, bits) ; 
}
  
bool IRsend::send_raw(int id , long long rawData, int bits) {  
  switch (id) {
    // default:
    #if SEND_NEC  
    case NEC          : sendNEC         (rawData, bits); return true;  
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
    case  AIWA_RC_T501 : sendAiwaRCT501 (rawData      ); return true;  
    #endif
  }
  return false ; 
}

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

bool IRsend::send_address(String protocol, int address, int command, int bits) {  
  return send_address(protocol2id(protocol), address, command,bits); 
}

// Note: forced bits to allow generic protocols with different bits 
bool IRsend::send_address(int id , int address, int command, int bits ) {  
  // larger then 32 bits  
  switch (id) {
  #if SEND_PANASONIC  
  case PANASONIC : sendPanasonic    (address, command      ); return true;  
  #endif 
  #if SEND_SANYO   
  case SANYO     : sendSanyo        (address, command      );  return true;  
  #endif
  // Others   
  #if SEND_NEC   
  case NEC       : send_addressNEC  (address, command, bits); return true;  
  #endif
  }
  return false ; 
}
  


