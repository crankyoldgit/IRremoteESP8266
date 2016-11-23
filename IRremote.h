//******************************************************************************
// IRremote
// Version 2.0.1 June, 2015
// Copyright 2009 Ken Shirriff
// For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
// Edited by Mitra to add new controller SANYO
//
// Interrupt code based on NECIRrcv by Joe Knapp
// http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
// Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
//
// JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
// LG added by Darryl Smith (based on the JVC protocol)
// Whynter A/C ARC-110WD added by Francesco Meschia
//******************************************************************************

#ifndef IRremote_h
#define IRremote_h

//------------------------------------------------------------------------------
// Uncoomment  DEBUG for lots of lovely debug output
// Now it is returning the dump too !!!
//#define DEBUG  1
//------------------------------------------------------------------------------
// The ISR header contains several useful macros the user may wish to use
//
//------------------------------------------------------------------------------
#include "IRremoteInt.h"

//------------------------------------------------------------------------------
// Supported IR protocols
// Each protocol you include costs memory and, during decode, costs time
// Disable (set to 0) all the protocols you do not need/want!
//
#define DECODE_RC5           1
#define SEND_RC5             1

#define DECODE_RC6           1   
#define SEND_RC6             1

#define DECODE_NEC           1
#define SEND_NEC             1

#define DECODE_SONY          1
#define SEND_SONY            1

#define DECODE_PANASONIC     1
#define SEND_PANASONIC       1

#define DECODE_JVC           1
#define SEND_JVC             1

#define DECODE_SAMSUNG       1
#define SEND_SAMSUNG         1

#define DECODE_WHYNTER       1
#define SEND_WHYNTER         1

#define DECODE_AIWA_RC_T501  0  // it can use SANYO instead that uses the same protocol 
#define SEND_AIWA_RC_T501    1  // better remove it as a hardcoded to a specific device 

#define DECODE_LG_32         1  //marcosamarinho
#define DECODE_LG            1  
#define SEND_LG              1

#define DECODE_SANYO         1
#define SEND_SANYO           1 //marcosamarinho

#define DECODE_MITSUBISHI    1
#define SEND_MITSUBISHI      1 // marcosamarinho

#define DECODE_DISH          0 // NOT WRITTEN
#define SEND_DISH            1

#define DECODE_SHARP         1 //marcosamarinho
#define SEND_SHARP           1

#define DECODE_DENON         1  //Sound be the same as SHARP TODO check it 
#define SEND_DENON           1

#define DECODE_PRONTO        0 // This function doe not logically make sense
#define SEND_PRONTO          1

#define DECODE_LEGO_PF       0 // NOT WRITTEN
#define SEND_LEGO_PF         1
// ESP8266
#define DECODE_COOLIX        0 
#define SEND_COOLIX          1

#define DECODE_DAIKIN        1 
#define SEND_DAIKIN          1

#define SEND_GC              1

#define DECODE_HASH          1 // Create a hash if 
 
//------------------------------------------------------------------------------
// When sending a Pronto code we request to send either the "once" code
//                                                   or the "repeat" code
// If the code requested does not exist we can request to fallback on the
// other code (the one we did not explicitly request)
//
// I would suggest that "fallback" will be the standard calling method
// The last paragraph on this page discusses the rationale of this idea:
//   http://www.remotecentral.com/features/irdisp2.htm
//
//#define PRONTO_ONCE        false
//#define PRONTO_REPEAT      true
//#define PRONTO_FALLBACK    true
//#define PRONTO_NOFALLBACK  false

//------------------------------------------------------------------------------
// An enumerated list of all supported formats
// You do NOT need to remove entries from this list when disabling protocols!
//
typedef
	enum {
		UNKNOWN      = -1,
		UNUSED       =  0,
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
		DENON,
		PRONTO,
		LEGO_PF,
		COOLIX ,
		DAIKIN,
		GC
	}
decode_type_t;

// Values for decode_type
#define UNKNOWN      -1
#define RC5           1
#define RC6           2
#define NEC           3
#define SONY          4
#define PANASONIC     5
#define JVC           6
#define SAMSUNG       7
#define WHYNTER       8
#define AIWA_RC_T501  9
#define LG           10
#define SANYO        11
#define MITSUBISHI   12
#define DISH         13
#define SHARP        14
#define DENON        15
#define PRONTO       16
#define LEGO_PF      17 
#define COOLIX       18
#define DAIKIN       19
#define GC           20 



//------------------------------------------------------------------------------
// Debug directives
//
#if DEBUG
#	define DBG_PRINT(...)    Serial.print(__VA_ARGS__)
#	define DBG_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
#	define DBG_PRINT(...)
#	define DBG_PRINTLN(...)
#endif

//------------------------------------------------------------------------------
// Mark & Space matching functions
//

//---------------------------------------------------------------------------
// Results returned from the decoder
//

// Results returned from the decoder
class decode_results {
public:
  int decode_type;                 // number of  UNKNOWN, NEC, SONY, RC5, ...
  String protocol;                 // UNKNOWN, NEC, SONY, RC5, ..
  unsigned long long value;        // Decoded value
  int address;                     // Decoded Device Address
  int command;                     // Decoded command    
  int bits;                        // Number of bits in decoded value
  volatile unsigned int *rawbuf;  // Raw intervals in .5 us ticks
  int rawlen;                      // Number of records in rawbuf.
 // int overflow;                    // true iff IR raw code too long TODO implement 
};

//------------------------------------------------------------------------------
// Decoded value for NEC when a repeat code is received
//
#define REPEAT 0xFFFFFFFF

//------------------------------------------------------------------------------
// Main class for receiving IR
//

class IRrecv
{
	public:
		IRrecv (int recvpin) ;
		//IRrecv (int recvpin, int blinkpin); // TODO implement 
		//void  blink13  (int blinkflag) ;
		bool  isIdle   () ;
		int   decode     (decode_results *results) ;
		int   decodeESP8266(decode_results *results);
		void  enableIRIn () ;
                void  disableIRIn();
		void  resume     () ;

	private:
		bool decodeHash       (decode_results *results) ;
		int  compare          (unsigned int oldval, unsigned int newval);
		int  MATCH            (int measured, int desired) ;
		int  MATCH_MARK       (int measured, int desired) ;
		int  MATCH_SPACE      (int measured, int desired) ;
		int  match_mark_nolog (int measured, int desired) ; 
		int  match_space_nolog(int measured, int desired); 
		bool mark_decode      (unsigned long &data,int val, int timeOne, int timeZero) ;  
		bool space_decode     (unsigned long &data,int val, int timeOne, int timeZero) ;
		bool mark_decode      (unsigned long long &data,int val, int timeOne, int timeZero) ;  
		bool space_decode     (unsigned long long &data,int val, int timeOne, int timeZero) ; 
		void addBit           (unsigned int       &data,bool bit) ;
		void addBit           (unsigned long      &data,bool bit) ;
                void addBit           (unsigned long long &data,bool bit) ;
		//......................................................................
#		if (DECODE_RC5 || DECODE_RC6)
			// This helper function is shared by RC5 and RC6
			int  getRClevel (decode_results *results,  int *offset,  int *used,  int t1) ;
#		endif
#		if DECODE_RC5
			bool  decodeRC5        (decode_results *results) ;
#		endif
#		if DECODE_RC6
			bool  decodeRC6        (decode_results *results) ;
#		endif
		//......................................................................
#		if DECODE_NEC
			bool  decodeNEC        (decode_results *results) ;
#		endif
		//......................................................................
#		if DECODE_SONY
			bool  decodeSony       (decode_results *results) ;
#		endif
		//......................................................................
#		if DECODE_PANASONIC
			bool  decodePanasonic  (decode_results *results) ;
#		endif
		//......................................................................
#		if DECODE_JVC
			bool  decodeJVC        (decode_results *results) ;
#		endif
		//......................................................................
#		if DECODE_SAMSUNG
			bool  decodeSAMSUNG    (decode_results *results) ;
#		endif
		//......................................................................
#		if DECODE_WHYNTER
			bool  decodeWhynter    (decode_results *results) ;
#		endif
		//......................................................................
#		if DECODE_AIWA_RC_T501
			bool  decodeAiwaRCT501 (decode_results *results) ;
#		endif
		//......................................................................
#		if DECODE_LG
			bool  decodeLG         (decode_results *results) ;
			bool  decodeLG_32      (decode_results *results) ; 
#		endif
		//......................................................................
#		if DECODE_SANYO
			bool  decodeSanyo      (decode_results *results) ;
#		endif
		//......................................................................
#		if DECODE_MITSUBISHI
			bool  decodeMitsubishi (decode_results *results) ;
#		endif
		//......................................................................
#		if DECODE_DISH
			bool  decodeDish       (decode_results *results) ; // NOT WRITTEN
#		endif
		//......................................................................
#		if DECODE_SHARP
			bool  decodeSharp      (decode_results *results) ; // marcosamarinho included 

#		endif
		//......................................................................
#		if DECODE_DENON
			bool  decodeDenon      (decode_results *results) ;
#		endif
		//......................................................................
#		if DECODE_LEGO_PF
			bool  decodeLegoPowerFunctions (decode_results *results) ;
#		endif
		//......................................................................
#		if DECODE_DAIKIN
			 bool decodeDaikin    (decode_results *results);
#		endif

} ;


//------------------------------------------------------------------------------
// Main class for sending IR
//

/*
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
      }
  };
*/
class IRsend
{
	private:
		int timeHigh;
		int PeriodicTime;
int IRpin;
	public:
		IRsend(int IRsendPin);
                void  begin();
		void  custom_delay_usec        (unsigned long uSecs);
		void  enableIROut 		(int khz,int dutycycle=2 ) ;  // 2 meaning 50%  3 to 33%  4 to 25%
		void  mark        		(unsigned int usec) ;
		void  space       		(unsigned int usec) ;
		void  sendBitMarkSpaceCoded     (bool bit, int timeMark ,int timeSpaceH,int timeSpaceL ) ; 
		void  space_encode              (bool bit,int timeH,int timeL ) ; 
		void  mark_encode               (bool bit,int timeH,int timeL ) ; 
		void  addBit                    (unsigned long long  &data,bool  bit) ; 
  		void  sendRaw     		 (const unsigned int buf[],  unsigned int len,  unsigned int hz) ;
		void  sendRaw                    (unsigned int buf[], int len, int hz);
                // new generic ones to be used at client to allow new protocols without change code nt void IRsend::protocol2id (String protocol)      
      
		int protocol2id  (String protocol); 
                bool send_raw    (String protocol, long long rawData, int IrBits) ; 
		bool send_raw    (int id         , long long rawData, int bits); 
		bool send_address(String protocol, int address       , int command); 
		bool send_address(int id         , int address       , int command);  
		//......................................................................
#		if SEND_RC5
			void  sendRC5        (unsigned long data,  int nbits) ;
#		endif
#		if SEND_RC6
			void  sendRC6        (unsigned long data,  int nbits) ;
#		endif
		//......................................................................
#		if SEND_NEC
			unsigned long rawNEC(unsigned int address ,unsigned  int command ); 
			void  sendNEC        (unsigned long data,  int nbits) ;
#		endif
		//......................................................................
#		if SEND_SONY
			void  sendSony       (unsigned long data,  int nbits) ;
#		endif
		//......................................................................
#		if SEND_JVC
                        void  sendJVC        (unsigned long data,  int nbits) ;
#		endif
		//......................................................................
#		if SEND_SAMSUNG
			void  sendSAMSUNG    (unsigned long data,  int nbits) ;
#		endif
		//......................................................................
#		if SEND_WHYNTER
			void  sendWhynter    (unsigned long data,  int nbits) ;
#		endif
		//......................................................................
#		if SEND_LG
			void  sendLG         (unsigned long data,  int nbits) ;
#		endif
		//......................................................................
#		if SEND_MITSUBISHI
			void sendMitsubishi (unsigned long data, int nbits) ; 
#		endif
		//......................................................................
#		if SEND_DISH
			void  sendDISH       (unsigned long data,  int nbits) ;
#		endif
		//......................................................................
#		if SEND_DENON
			void  sendDenon      (unsigned long data,  int nbits) ;
#		endif
		//......................................................................
#		if SEND_SHARP
			void  sendSharpRaw   (unsigned long data,  int nbits) ;     
			void  sendSharp      (unsigned int address,  unsigned int command) ;  
#		endif

#		if SEND_AIWA_RC_T501
			void  sendAiwaRCT501 (int code) ; // this sonds be te same as SAYO better take it out as it is an specific hardcoded control 
#		endif
		//......................................................................
#		if SEND_PANASONIC
			void  sendPanasonic  (unsigned int address,  unsigned int command) ;
#		endif
		//......................................................................
#		if SEND_SANYO
			void  sendSanyo      (unsigned int address,  unsigned int command) ; 
#		endif
		//......................................................................
#		if SEND_PRONTO
			void  sendPronto     (char* code,  bool repeat,  bool fallback) ;
#		endif
//......................................................................
#		if SEND_LEGO_PF
			void  sendLegoPowerFunctions (uint16_t data, bool repeat = true) ;
#		endif
//......................................................................
#		if SEND_DAIKIN
			void sendDaikin(unsigned char daikin[]);
 			void sendDaikinChunk(unsigned char buf[], int len, int start);
#		endif
#		if SEND_COOLIX
			void sendCOOLIX(unsigned long data, int nbits)  ;
#		endif
#		if SEND_GC
			void sendGC(unsigned int buf[], int len);
#		endif
              

} ;

#endif
