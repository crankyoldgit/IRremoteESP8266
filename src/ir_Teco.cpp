// Copyright 2019 Fabien Valthier
/*
Node MCU/ESP8266 Sketch to emulate Teco
*/

#include "ir_Teco.h"
#include <algorithm>
#include "IRremoteESP8266.h"
#include "IRutils.h"
#ifndef ARDUINO
#include <string>
#endif

// Constants
// using SPACE modulation.
const uint16_t kTecoHdrMark = 9000;
const uint16_t kTecoHdrSpace = 4440;
const uint16_t kTecoBitMark = 620;
const uint16_t kTecoOneSpace = 1650;
const uint16_t kTecoZeroSpace = 580;

#if SEND_TECO
// Send a Teco A/C message.
//
//
// Args:
//   data:   Contents of the message to be sent.
//   nbits:  Nr. of bits of data to be sent. Typically kTecoBits.
//   repeat: Nr. of additional times the message is to be sent.

void IRsend::sendTeco(uint64_t data, uint16_t nbits, uint16_t repeat) {
  // if (nbits % 35 != 0) return;  // Not enough bits to send a proper message.
  sendGeneric(kTecoHdrMark, kTecoHdrSpace, kTecoBitMark, kTecoOneSpace,
              kTecoBitMark, kTecoZeroSpace, kTecoBitMark, 0,
              data, nbits, 38, false, repeat, kDutyDefault);
}
#endif  // SEND_TECO

IRTecoAC::IRTecoAC(uint16_t pin) : _irsend(pin) { stateReset(); }

void IRTecoAC::begin() { _irsend.begin(); }

#if SEND_TECO
void IRTecoAC::send(const uint16_t repeat) {
  _irsend.sendTeco(remote_state, kTecoBits, repeat);
}
#endif  // SEND_TECO

void IRTecoAC::stateReset() {
  // automatically sets: Mode:auto, Off, fan:auto, temp:16
  remote_state = kTecoReset;
  this->setSwing(true);
}

uint64_t IRTecoAC::getRaw() { return remote_state; }

void IRTecoAC::setRaw(const uint64_t new_code) { remote_state = new_code; }

void IRTecoAC::on() {
  remote_state |= kTecoPower;
}

void IRTecoAC::off() {
  remote_state &= ~kTecoPower;
}

void IRTecoAC::setPower(bool state) {
  if (state)
    on();
  else
    off();
}

bool IRTecoAC::getPower() {
  return (remote_state & kTecoPower) == kTecoPower; }

void IRTecoAC::setTemp(uint8_t temp) {
  uint8_t newtemp = temp;
  newtemp = std::min(newtemp, kTecoMaxTemp);
  newtemp = std::max(newtemp, kTecoMinTemp);
  newtemp -= 16;  // 16=0b000

  remote_state &= ~kTecoTempMask;  // reinit temp
  remote_state |= (newtemp << 8);
}

uint8_t IRTecoAC::getTemp() {
  uint32_t temp;
  temp = (remote_state & kTecoTempMask) >> 8;
  return temp+16;
}

// Set the speed of the fan
void IRTecoAC::setFan(uint8_t speed) {
  uint8_t newspeed = speed;
  switch (speed) {
    case kTecoFanAuto:
    case kTecoFan3:
    case kTecoFan2:
    case kTecoFan1:
      break;
    default:
      newspeed = kTecoFanAuto;
  }
  remote_state &= ~kTecoFanMask;  // reinit fan
  remote_state |= (newspeed << 4);
}

uint8_t IRTecoAC::getFan() {
  return (remote_state & kTecoFanMask) >> 4;
}

void IRTecoAC::setMode(uint8_t mode) {
  uint8_t newmode = mode;
  switch (mode) {
    case kTecoAuto:
    case kTecoCool:
    case kTecoDry:
    case kTecoFan:
    case kTecoHeat:
      break;
    default:
      newmode = kTecoAuto;
  }
  remote_state &= ~kTecoModeMask;  // reinit mode
  remote_state |= newmode;
}

uint8_t IRTecoAC::getMode() {
  return remote_state & kTecoModeMask;
}

void IRTecoAC::setSwing(bool state) {
  if (state) remote_state |= kTecoSwing;
  else
    remote_state &= ~kTecoSwing;
}

bool IRTecoAC::getSwing() {
  return (remote_state & kTecoSwing) == kTecoSwing;
}

void IRTecoAC::setSleep(bool state) {
  if (state) remote_state |= kTecoSleep;
  else
    remote_state &= ~kTecoSleep;
}

bool IRTecoAC::getSleep() {
  return (remote_state & kTecoSleep) == kTecoSleep;
}

// Convert the internal state into a human readable string.
#ifdef ARDUINO
String IRTecoAC::toString() {
  String result = "";
#else
std::string IRTecoAC::toString() {
  std::string result = "";
#endif  // ARDUINO
  result += "Power: ";
  if (getPower()) {
    result += "On";
  } else {
    result += "Off";
    return result;  // If it's off, there is no other info.
  }
  result += ", Fan: " + uint64ToString(getFan());
  switch (getFan()) {
    case kTecoFanAuto:
      result += " (AUTO)";
      break;
    case kTecoFan3:
      result += " (MAX)";
      break;
    case kTecoFan1:
      result += " (MIN)";
      break;
    case kTecoFan2:
      result += " (MED)";
      break;
    default:
      result += " (UNKNOWN)";
  }
  // Special modes.
  if (getSwing()) {
    result += ", Swing: Toggle";
    return result;
  }
  if (getSleep()) {
    result += ", Sleep: Toggle";
    return result;
  }
  result += ", Mode: " + uint64ToString(getMode());
  switch (getMode()) {
    case kTecoAuto:
      result += " (AUTO)";
      break;
    case kTecoCool:
      result += " (COOL)";
      break;
    case kTecoHeat:
      result += " (HEAT)";
      break;
    case kTecoDry:
      result += " (DRY)";
      break;
    case kTecoFan:
      result += " (FAN)";
      break;
    default:
      result += " (UNKNOWN)";
  }
  if (getMode() != kTecoFan)  // Fan mode doesn't have a temperature.
    result += ", Temp: " + uint64ToString(getTemp()) + "C";
  return result;
}

#if DECODE_TECO
// Decode the supplied Gree message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect. Typically kTecoBits.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: STABLE / Tested.
bool IRrecv::decodeTeco(decode_results* results, uint16_t nbits, bool strict) {
  // Check if can possibly be a valid Teco message.
  if (results->rawlen < kHeader + 2 * nbits) return false;
  if (strict && nbits != kTecoBits) return false;  // Not what is expected

  uint64_t data = 0;
  uint16_t offset = kStartOffset;
  match_result_t data_result;

  // Header
  if (!matchMark(results->rawbuf[offset++], kTecoHdrMark)) return false;
  if (!matchSpace(results->rawbuf[offset++], kTecoHdrSpace)) return false;
  // Data (35 bits)
  data_result =
      matchData(&(results->rawbuf[offset]), 35, kTecoBitMark, kTecoOneSpace,
                kTecoBitMark, kTecoZeroSpace, kTolerance, kMarkExcess, false);
  if (data_result.success == false) return false;
  data = data_result.data;
  offset += data_result.used;
  uint16_t actualBits = data_result.used / 2;

  // Footer.
  if (!matchMark(results->rawbuf[offset++], kTecoBitMark)) return false;

  // Compliance
  if (actualBits < nbits) return false;
  if (strict && actualBits != nbits) return false;  // Not as we expected.

  // Success
  results->decode_type = TECO;
  results->bits = actualBits;
  results->value = data;
  results->address = 0;
  results->command = 0;
  return true;
}
#endif  // DECODE_TECO
