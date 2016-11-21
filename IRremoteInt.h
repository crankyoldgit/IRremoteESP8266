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
// Length of raw duration buffer
#define RAWBUF 140     

// information for the interrupt handler
typedef struct {
  uint8_t recvpin;              // pin for IR data from detector
  uint8_t rcvstate;             // state machine
  unsigned int timer;          // state timer, counts ticks.
  unsigned int rawbuf[RAWBUF];  // raw data
  uint8_t rawlen;               // counter of entries in rawbuf
} 
irparams_t;

// Defined in IRremote.cpp
extern volatile irparams_t irparams;
//-------------------------------------------------------

// Useful constants
// microseconds per clock interrupt tick ESP8266 use 1 to receive the right number.
#define USECPERTICK 1  

 //  Relative match LIRC default 30%
#define TOLERANCE 30   
#define LTOL (1.0 - TOLERANCE/100.) 
#define UTOL (1.0 + TOLERANCE/100.) 
// pulse parameters in usec
// Absolute match 
// Marks tend to be 100us too long, and spaces 100us too short
// when received due to sensor lag.
#define MARK_EXCESS 100

#define TICKS_LOW(us)  (int) (((us)*LTOL/USECPERTICK))
#define TICKS_HIGH(us) (int) (((us)*UTOL/USECPERTICK))

// receiver states
#define STATE_IDLE     2
#define STATE_MARK     3
#define STATE_SPACE    4
#define STATE_STOP     5

// IR detector output is active low 
#define MARK  0
#define SPACE 1

//This was created to avoid include an initial fake value at the rawbuf[]
//This was created to keep compatibility with IRRemote 
// TODO fix RC5/RC6 and review examples , and document to allow move it to 0 
#define OFFSET_START 1 

#endif
