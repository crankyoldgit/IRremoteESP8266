/*
Node MCU/ESP8266 Sketch to emulate Argo Ulisse 13 DCI remote
Copyright 2017 Schmolders
*/

#include "ir_Argo.h"
#include <algorithm>
#include "IRremoteESP8266.h"
#include "IRutils.h"
#include <Arduino.h>


//  ARgo

// Constants
//using SPACE modulation. MARK is always const 400u
#define ARGO_PREAMBLE_1           6400U //Mark
#define ARGO_PREAMBLE_2           3300U //Space
#define ARGO_MARK                 400U
#define ARGO_ONE_SPACE             2200U
#define ARGO_ZERO_SPACE            900U

#if SEND_ARGO
// Send a Argo A/C message.
//
// Args:
//   data: An array of ARGO_COMMAND_LENGTH bytes containing the IR command.
//
// Status: STABLE
//
//overloading the IRSend Function

void IRsend::sendArgo(unsigned char data[], uint16_t nbytes,
                        uint16_t repeat) {
  if (nbytes < ARGO_COMMAND_LENGTH)
    return;  // Not enough bytes to send a proper message.
  // Set IR carrier frequency
  enableIROut(38);
  for (uint16_t r = 0; r <= repeat; r++) {
    // Header TODO validate
    mark(ARGO_PREAMBLE_1);
    space(ARGO_PREAMBLE_2);
    //space(ARGO_);
    //send data, defined in IRSend.cpp
    for (uint16_t i = 0; i < nbytes; i++)
      sendData(ARGO_MARK, ARGO_ONE_SPACE, ARGO_MARK,
               ARGO_ZERO_SPACE, data[i], 8, false);
               //send LSB first reverses the bit order in array for sending.

  }
}

IRArgoESP::IRArgoESP(uint16_t pin) : _irsend(pin) {
  stateReset();
}

void IRArgoESP::begin() {
  _irsend.begin();
}

void IRArgoESP::send() {
  Serial.println("Sending IR code");
  _irsend.sendArgo(argo);
}

void IRArgoESP::checksum() {
  uint8_t sum = 2; //corresponts to byte 11 being constant 0b01
  uint8_t i;

  //only add up bytes to 9. byte 10 is 0b01 constant anyway.
  //assume that argo array is MSB first (left)
  for (i = 0; i < 10; i++)
    sum += argo[i];

  sum=sum%256; //modulo 256
  //append sum to end of array
  //set const part of checksum bit 10
  argo[10] = 0b00000010;
  argo[10] += sum << 2; //shift up 2 bits and append to byte 10
  argo[11] = sum >> 6; //shift down 6 bits and add in two LSBs of bit 11

}


void IRArgoESP::stateReset() {
  for (uint8_t i = 0; i < ARGO_COMMAND_LENGTH; i++)
    argo[i] = 0x0;

  //Argo Message. Store MSB left.
  //default message:
  argo[0] = 0b10101100; //LSB first (as sent) 0b00110101; //const preamble
  argo[1] = 0b11110101; //LSB first: 0b10101111; //const preamble
  //keep payload 2-9 at zero
  argo[10] = 0b00000010; //const 01, checksum 6bit
  argo[11] = 0b00000000; //checksum 2bit

  this->off();
  this->setTemp(20);
  this->setRoomTemp(25);
  this->setCoolMode(ARGO_COOL_AUTO);
  this->setFan(ARGO_FAN_AUTO);

  checksum();
}

uint8_t* IRArgoESP::getRaw() {
  checksum();   // Ensure correct settings before sending.
  return argo;
}

void IRArgoESP::on() {
  // state = ON;
  ac_state=1;
  //bit 5 of byte 9 is on/off
  //in MSB first
  argo[9] = argo[9] | 0b00100000; //set ON/OFF bit to 1

  checksum();
}

void IRArgoESP::off() {
  // state = OFF;
  ac_state=0;
  //in MSB first
  //bit 5 of byte 9 to off
  argo[9] = argo[9] & 0b11011111; //set on/off bit to 0
  checksum();
}

void IRArgoESP::setPower(bool state) {
  if (state)
    on();
  else
    off();
}

uint8_t IRArgoESP::getPower() {
  //return
  return ac_state;
}

void IRArgoESP::setMax(bool state) {
  max_mode=state;
  if (max_mode)
    argo[9] |= 0b00001000;
  else
    argo[9] &= 0b11110111;
  checksum();
}

bool IRArgoESP::getMax() {
  return max_mode;
}

// Set the temp in deg C
//sending 0 equals +4
void IRArgoESP::setTemp(uint8_t temp) {
  if (temp < ARGO_MIN_TEMP)
    temp = ARGO_MIN_TEMP;
  else if (temp > ARGO_MAX_TEMP)
    temp = ARGO_MAX_TEMP;

  //store in attributes
  set_temp=temp;
  //offset 4 degrees. "If I want 12 degrees, I need to send 8"
  temp -=4;
  //Settemp = Bit 6,7 of byte 2, and bit 0-2 of byte 3
  //mask out bits
  //argo[13] & 0x00000100; //mask out ON/OFF Bit
  argo[2] &= 0b00111111;
  argo[3] &= 0b11111000;

  argo[2] += temp << 6; //append to bit 6,7
  argo[3] += temp >> 2; //remove lowest to bits and append in 0-2
  checksum();
}

uint8_t IRArgoESP::getTemp() {
  return set_temp;
}

// Set the speed of the fan
void IRArgoESP::setFan(uint8_t fan) {
  // Set the fan speed bits, leave low 4 bits alone
  fan_mode = fan;
  //mask out bits
  argo[3] &= 0b11100111;
  //set fan mode at bit positions
  argo[3] += fan << 3;
  checksum();
}

uint8_t IRArgoESP::getFan() {
  return fan_mode;
}

void IRArgoESP::setFlap(uint8_t flap) {
  flap_mode = flap;
  //TODO
  checksum();
}
uint8_t IRArgoESP::getFlap() {
  return flap_mode;
}


uint8_t IRArgoESP::getMode() {
  //return cooling 0, heating 1
  return ac_mode;
}
void IRArgoESP::setCoolMode(uint8_t mode) {
  ac_mode=0; //set ac mode to cooling
  cool_mode=mode;
  //mask out bits, also leave bit 5 on 0 for cooling
  argo[2] &= 0b11000111;

  //set cool mode at bit positions
  argo[2] += mode << 3;
  checksum();

}
uint8_t IRArgoESP::getCoolMode() {
  return cool_mode;
}

void IRArgoESP::setHeatMode(uint8_t mode) {
  ac_mode=1; //set ac mode to heating
  heat_mode=mode;
  //mask out bits
  argo[2] &= 0b11000111;
  //set heating bit
  argo[2] |= 0b00100000;
  //set cool mode at bit positions
  argo[2] += mode << 3;
  checksum();
}
uint8_t IRArgoESP::getHeatMode() {
  return heat_mode;
}

void IRArgoESP::setNight(bool state) {
  night_mode=state;
  if (night_mode)
    //set bit at night position: bit 2
    argo[9] |= 0b00000100;
  else
    argo[9] &= 0b11111011;
  checksum();
}
bool IRArgoESP::getNight() {
  return night_mode;
}

void IRArgoESP::setiFeel(bool state) {
  ifeel_mode=state;
  if (ifeel_mode)
    //set bit at iFeel position: bit 7
    argo[9] |= 0b10000000;
  else
    argo[9] &= 0b01111111;
  checksum();
}
bool IRArgoESP::getiFeel() {
  return ifeel_mode;
}

void IRArgoESP::setTime() {
  //TODO
  //use function call from checksum to set time first
}
void IRArgoESP::setRoomTemp(uint8_t temp) {
  //use function call from checksum to set room temp
  temp-=4;
  //mask out bits
  argo[3] &= 0b00011111;
  argo[4] &= 0b11111100;

  argo[3] += temp << 5; //append to bit 5,6,7
  argo[4] += temp >> 3; //remove lowest 3 bits and append in 0,1
  checksum();
}

#endif  // SEND_ARGO
