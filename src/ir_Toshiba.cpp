// Copyright 2017 David Conran

#include "ir_Toshiba.h"
#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRtimer.h"
#include "IRutils.h"

//     TTTTTTT  OOOOO   SSSSS  HH   HH IIIII BBBBB     AAA
//       TTT   OO   OO SS      HH   HH  III  BB   B   AAAAA
//       TTT   OO   OO  SSSSS  HHHHHHH  III  BBBBBB  AA   AA
//       TTT   OO   OO      SS HH   HH  III  BB   BB AAAAAAA
//       TTT    OOOO0   SSSSS  HH   HH IIIII BBBBBB  AA   AA

// Toshiba A/C support added by David Conran

// Constants

// Toshiba A/C
// Ref:
//   https://github.com/r45635/HVAC-IR-Control/blob/master/HVAC_ESP8266/HVAC_ESP8266T.ino#L77
#define TOSHIBA_AC_HDR_MARK    4400U
#define TOSHIBA_AC_HDR_SPACE   4300U
#define TOSHIBA_AC_BIT_MARK     543U
#define TOSHIBA_AC_ONE_SPACE   1623U
#define TOSHIBA_AC_ZERO_SPACE   472U
#define TOSHIBA_AC_RPT_MARK     440U
#define TOSHIBA_AC_RPT_SPACE   7048U

#if SEND_TOSHIBA_AC
// Send a Toshiba A/C message.
//
// Args:
//   data: An array of bytes containing the IR command.
//   nbytes: Nr. of bytes of data in the array. (>=TOSHIBA_AC_STATE_LENGTH)
//   repeat: Nr. of times the message is to be repeated.
//          (Default = TOSHIBA_AC_MIN_REPEAT).
//
// Status: ALPHA / Untested.
//
void IRsend::sendToshibaAC(unsigned char data[], uint16_t nbytes,
                              uint16_t repeat) {
  if (nbytes < TOSHIBA_AC_STATE_LENGTH)
    return;  // Not enough bytes to send a proper message.

  // Set IR carrier frequency
  enableIROut(38);
  // Repeat the message if requested.
  for (uint16_t r = 0; r <= repeat; r++) {
    // Header
    mark(TOSHIBA_AC_HDR_MARK);
    space(TOSHIBA_AC_HDR_SPACE);
    // Data
    for (uint16_t i = 0; i < nbytes; i++)
      sendData(TOSHIBA_AC_BIT_MARK, TOSHIBA_AC_ONE_SPACE,
               TOSHIBA_AC_BIT_MARK, TOSHIBA_AC_ZERO_SPACE,
               data[i], 8, true);
    // Footer
    mark(TOSHIBA_AC_RPT_MARK);
    space(TOSHIBA_AC_RPT_SPACE);
  }
}

// Code to emulate Toshiba A/C IR remote control unit.
// Inspired and derived from the work done at:
//   https://github.com/r45635/HVAC-IR-Control
//
// Warning: Consider this very alpha code. Seems to work, but not validated.
//
// Equipment it seems compatible with:
//  * Toshiba RAS-B13N3KV2 / Akita EVO II
//  * <Add models (A/C & remotes) you've gotten it working with here>
// Initialise the object.
IRToshibaAC::IRToshibaAC(uint16_t pin) : _irsend(pin) {
  stateReset();
}

// Reset the state of the remote to a known good state/sequence.
void IRToshibaAC::stateReset() {
  // The state of the IR remote in IR code form.
  // Known good state obtained from:
  //   https://github.com/r45635/HVAC-IR-Control/blob/master/HVAC_ESP8266/HVAC_ESP8266T.ino#L103
  // Note: Can't use the following because it requires -std=c++11
  // uint8_t remote_state[TOSHIBA_AC_STATE_LENGTH] = {
  //    0xF2, 0x0D, 0x03, 0xFC, 0x01, 0x00, 0x00, 0x00, 0x00 };
  remote_state[0] = 0xF2;
  remote_state[1] = 0x0D;
  remote_state[2] = 0x03;
  remote_state[3] = 0xFC;
  remote_state[4] = 0x01;
  for (uint8_t i = 5; i < TOSHIBA_AC_STATE_LENGTH; i++)
    remote_state[i] = 0;
  mode_state = remote_state[6] & 0b00000011;
  checksum();  // Calculate the checksum
}

// Configure the pin for output.
void IRToshibaAC::begin() {
    _irsend.begin();
}

// Send the current desired state to the IR LED.
void IRToshibaAC::send() {
  checksum();   // Ensure correct checksum before sending.
  _irsend.sendToshibaAC(remote_state);
}

// Return a pointer to the internal state date of the remote.
uint8_t* IRToshibaAC::getRaw() {
  checksum();
  return remote_state;
}

// Calculate the checksum for the current internal state of the remote.
void IRToshibaAC::checksum() {
  uint8_t checksum = 0;
  // Checksum is simple XOR of all previous bytes.
  // Stored as an 8 bit value in the last byte.
  for (uint8_t i = 0; i < TOSHIBA_AC_STATE_LENGTH - 1; i++)
    checksum ^= remote_state[i];
  remote_state[TOSHIBA_AC_STATE_LENGTH - 1] = checksum;
}

// Set the requested power state of the A/C to off.
void IRToshibaAC::on() {
  // state = ON;
  remote_state[6] &= ~TOSHIBA_AC_POWER;
  setMode(mode_state);
}

// Set the requested power state of the A/C to off.
void IRToshibaAC::off() {
  // state = OFF;
  remote_state[6] |= (TOSHIBA_AC_POWER | 0b00000011);
}

// Set the requested power state of the A/C.
void IRToshibaAC::setPower(bool state) {
  if (state)
    on();
  else
    off();
}

// Return the requested power state of the A/C.
bool IRToshibaAC::getPower() {
  return((remote_state[6] & TOSHIBA_AC_POWER) == 0);
}

// Set the temp. in deg C
void IRToshibaAC::setTemp(uint8_t temp) {
  temp = std::max((uint8_t) TOSHIBA_AC_MIN_TEMP, temp);
  temp = std::min((uint8_t) TOSHIBA_AC_MAX_TEMP, temp);
  remote_state[5] = (temp - TOSHIBA_AC_MIN_TEMP) << 4;
}

// Return the set temp. in deg C
uint8_t IRToshibaAC::getTemp() {
  return((remote_state[5] >> 4) + TOSHIBA_AC_MIN_TEMP);
}

// Set the speed of the fan, 0-5.
// 0 is auto, 1-5 is the speed, 5 is Max.
void IRToshibaAC::setFan(uint8_t fan) {
  // Bounds check
  if (fan > TOSHIBA_AC_FAN_MAX)
    fan = TOSHIBA_AC_FAN_MAX;  // Set the fan to maximum if out of range.
  if (fan > TOSHIBA_AC_FAN_AUTO) fan++;
  remote_state[6] &= 0b00011111;  // Clear the previous fan state
  remote_state[6] |= (fan << 5);
}

// Return the requested state of the unit's fan.
uint8_t IRToshibaAC::getFan() {
  uint8_t fan = remote_state[6] >> 5;
  if (fan == TOSHIBA_AC_FAN_AUTO) return TOSHIBA_AC_FAN_AUTO;
  return --fan;
}

// Return the requested climate operation mode of the a/c unit.
uint8_t IRToshibaAC::getMode() {
  return(mode_state);
}

// Set the requested climate operation mode of the a/c unit.
void IRToshibaAC::setMode(uint8_t mode) {
  // If we get an unexpected mode, default to AUTO.
  switch (mode) {
    case TOSHIBA_AC_AUTO: break;
    case TOSHIBA_AC_COOL: break;
    case TOSHIBA_AC_DRY: break;
    case TOSHIBA_AC_HEAT: break;
    default: mode = TOSHIBA_AC_AUTO;
  }
  mode_state = mode;
  // Only adjust the remote_state if we have power set to on.
  if (getPower()) {
    remote_state[6] &= 0b11111100;  // Clear the previous mode.
    remote_state[6] |= mode_state;
  }
}
#endif  // SEND_TOSHIBA_AC
