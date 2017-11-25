/*
An Arduino sketch to emulate IR Daikin ARC433** remote control unit
Read more at:
http://harizanov.com/2012/02/control-daikin-air-conditioner-over-the-internet/

Copyright 2016 sillyfrog
Copyright 2017 sillyfrog, crankyoldgit
*/

#include "ir_Daikin.h"
#include <algorithm>
#ifndef ARDUINO
#include <string>
#endif
#include "IRremoteESP8266.h"
#include "IRutils.h"
#include "IRrecv.h"
#include "IRsend.h"

//                DDDDD     AAA   IIIII KK  KK IIIII NN   NN
//                DD  DD   AAAAA   III  KK KK   III  NNN  NN
//                DD   DD AA   AA  III  KKKK    III  NN N NN
//                DD   DD AAAAAAA  III  KK KK   III  NN  NNN
//                DDDDDD  AA   AA IIIII KK  KK IIIII NN   NN

// Constants
// Ref:
//   https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/tree/master/IRremote
//   http://rdlab.cdmt.vn/project-2013/daikin-ir-protocol

#if SEND_DAIKIN
// Original header
// static uint8_t header1[DAIKIN_HEADER1_LENGTH];
// header1[0] = 0b00010001;
// header1[1] = 0b11011010;
// header1[2] = 0b00100111;
// header1[3] = 0b00000000;
// header1[4] = 0b11000101;
// header1[5] = 0b00000000;
// header1[6] = 0b00000000;
// header1[7] = 0b11010111;

// Send a Daikin A/C message.
//
// Args:
//   data: An array of DAIKIN_COMMAND_LENGTH bytes containing the IR command.
//
// Status: STABLE
//
// Ref:
//   IRDaikinESP.cpp
//   https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/tree/master/IRremote
void IRsend::sendDaikin(unsigned char data[], uint16_t nbytes,
                        uint16_t repeat) {
  if (nbytes < DAIKIN_COMMAND_LENGTH)
    return;  // Not enough bytes to send a proper message.
  // Set IR carrier frequency
  enableIROut(38);
  for (uint16_t r = 0; r <= repeat; r++) {
    // Send the header, 0b00000
    sendData(DAIKIN_BIT_MARK, DAIKIN_ONE_SPACE, DAIKIN_BIT_MARK,
             DAIKIN_ZERO_SPACE, 0, 5, false);
    sendDaikinGapHeader();
    // Leading header
    // Do this as a constant to save RAM and keep in flash memory
    sendData(DAIKIN_BIT_MARK, DAIKIN_ONE_SPACE, DAIKIN_BIT_MARK,
             DAIKIN_ZERO_SPACE, DAIKIN_FIRST_HEADER64, 64, false);
    sendDaikinGapHeader();
    // Data #1
    for (uint16_t i = 0; i < 8 && i < nbytes; i++)
      sendData(DAIKIN_BIT_MARK, DAIKIN_ONE_SPACE, DAIKIN_BIT_MARK,
               DAIKIN_ZERO_SPACE, data[i], 8, false);
    sendDaikinGapHeader();
    // Data #2
    for (uint16_t i = 8; i < nbytes; i++)
      sendData(DAIKIN_BIT_MARK, DAIKIN_ONE_SPACE, DAIKIN_BIT_MARK,
               DAIKIN_ZERO_SPACE, data[i], 8, false);
    // Footer #2
    mark(DAIKIN_BIT_MARK);
    space(DAIKIN_ZERO_SPACE + DAIKIN_GAP);
  }
}

void IRsend::sendDaikinGapHeader() {
  mark(DAIKIN_BIT_MARK);
  space(DAIKIN_ZERO_SPACE + DAIKIN_GAP);
  mark(DAIKIN_HDR_MARK);
  space(DAIKIN_HDR_SPACE);
}
#endif  // SEND_DAIKIN

#if (SEND_DAIKIN || DECODE_DAIKIN)
IRDaikinESP::IRDaikinESP(uint16_t pin) : _irsend(pin) {
  stateReset();
}

void IRDaikinESP::begin() {
  _irsend.begin();
}

void IRDaikinESP::send() {
  checksum();
  _irsend.sendDaikin(daikin);
}

// Calculate the checksum for a given data block.
// Args:
//   block:  Ptr to the start of the data block.
//   length: Nr. of bytes to checksum.
// Returns:
//   A byte containing the calculated checksum.
uint8_t IRDaikinESP::calcBlockChecksum(const uint8_t *block,
                                       const uint16_t length) {
  uint8_t sum = 0;
  // Daikin checksum is just the addition of all the data bytes
  // in the block but capped to 8 bits.
  for (uint16_t i = 0; i < length; i++, block++)
    sum += *block;
  return sum & 0xFFU;
}

// Verify the checksum is valid for a given state.
// Args:
//   state:  The array to verify the checksum of.
//   length: The size of the state.
// Returns:
//   A boolean.
bool IRDaikinESP::validChecksum(const uint8_t state[],
                                const uint16_t length) {
  if (length < 8 || state[7] != calcBlockChecksum(state, 7))  return false;
  if (length < 10 ||
      state[length - 1] != calcBlockChecksum(state + 8, length - 9))
    return false;
  return true;
}

// Calculate and set the checksum values for the internal state.
void IRDaikinESP::checksum() {
  daikin[7] = calcBlockChecksum(daikin, 7);
  daikin[26] = calcBlockChecksum(daikin + 8, 17);
}

void IRDaikinESP::stateReset() {
  for (uint8_t i = 0; i < DAIKIN_COMMAND_LENGTH; i++)
    daikin[i] = 0x0;

  daikin[0] = 0x11;
  daikin[1] = 0xDA;
  daikin[2] = 0x27;
  daikin[4] = 0x42;
  // daikin[7] is a checksum byte, it will be set by checksum().
  daikin[8] = 0x11;
  daikin[9] = 0xDA;
  daikin[10] = 0x27;
  daikin[13] = 0x49;
  daikin[14] = 0x1E;
  daikin[16] = 0xB0;
  daikin[19] = 0x06;
  daikin[20] = 0x60;
  daikin[23] = 0xC0;
  // daikin[26] is a checksum byte, it will be set by checksum().
  checksum();
}

uint8_t* IRDaikinESP::getRaw() {
  checksum();   // Ensure correct settings before sending.
  return daikin;
}

void IRDaikinESP::setRaw(uint8_t new_code[]) {
  for (uint8_t i = 0; i < DAIKIN_COMMAND_LENGTH; i++)
    daikin[i] = new_code[i];
}

void IRDaikinESP::on() {
  // state = ON;
  setBit(DAIKIN_BYTE_POWER, DAIKIN_BIT_POWER);
}

void IRDaikinESP::off() {
  // state = OFF;
  clearBit(DAIKIN_BYTE_POWER, DAIKIN_BIT_POWER);
}

void IRDaikinESP::setPower(bool state) {
  if (state)
    on();
  else
    off();
}

bool IRDaikinESP::getPower() {
  return (getBit(DAIKIN_BYTE_POWER, DAIKIN_BIT_POWER) > 0);
}

// Set the temp in deg C
void IRDaikinESP::setTemp(uint8_t temp) {
  if (temp < DAIKIN_MIN_TEMP)
    temp = DAIKIN_MIN_TEMP;
  else if (temp > DAIKIN_MAX_TEMP)
    temp = DAIKIN_MAX_TEMP;
  daikin[14] = temp * 2;
}

uint8_t IRDaikinESP::getTemp() {
  return daikin[14] / 2;
}

// Set the speed of the fan, 1-5 or DAIKIN_FAN_AUTO or DAIKIN_FAN_QUIET
void IRDaikinESP::setFan(uint8_t fan) {
  // Set the fan speed bits, leave low 4 bits alone
  uint8_t fanset;
  if (fan == DAIKIN_FAN_QUIET || fan == DAIKIN_FAN_AUTO)
    fanset = fan;
  else if (fan < DAIKIN_FAN_MIN || fan > DAIKIN_FAN_MAX)
    fanset = DAIKIN_FAN_AUTO;
  else
    fanset = 2 + fan;
  daikin[16] &= 0x0F;
  daikin[16] |= (fanset << 4);
}

uint8_t IRDaikinESP::getFan() {
  uint8_t fan = daikin[16] >> 4;
  if (fan != DAIKIN_FAN_QUIET && fan != DAIKIN_FAN_AUTO)
    fan -= 2;
  return fan;
}

uint8_t IRDaikinESP::getMode() {
  /*
  DAIKIN_COOL
  DAIKIN_HEAT
  DAIKIN_FAN
  DAIKIN_AUTO
  DAIKIN_DRY
  */
  return daikin[13] >> 4;
}

void IRDaikinESP::setMode(uint8_t mode) {
  switch (mode) {
    case DAIKIN_COOL:
    case DAIKIN_HEAT:
    case DAIKIN_FAN:
    case DAIKIN_DRY:
      break;
    default:
      mode = DAIKIN_AUTO;
  }
  mode <<= 4;
  daikin[13] &= 0b10001111;
  daikin[13] |= mode;
}

void IRDaikinESP::setSwingVertical(bool state) {
  if (state)
    daikin[16] |= 0x0F;
  else
    daikin[16] &= 0xF0;
}

bool IRDaikinESP::getSwingVertical() {
  return daikin[16] & 0x01;
}

void IRDaikinESP::setSwingHorizontal(bool state) {
  if (state)
    daikin[17] |= 0x0F;
  else
    daikin[17] &= 0xF0;
}

bool IRDaikinESP::getSwingHorizontal() {
  return daikin[17] & 0x01;
}

void IRDaikinESP::setQuiet(bool state) {
  if (state) {
    setBit(DAIKIN_BYTE_SILENT, DAIKIN_BIT_SILENT);
    // Powerful & Quiet mode being on are mutually exclusive.
    setPowerful(false);
  } else {
    clearBit(DAIKIN_BYTE_SILENT, DAIKIN_BIT_SILENT);
  }
}

bool IRDaikinESP::getQuiet() {
  return (getBit(DAIKIN_BYTE_SILENT, DAIKIN_BIT_SILENT) > 0);
}

void IRDaikinESP::setPowerful(bool state) {
  if (state) {
    setBit(DAIKIN_BYTE_POWERFUL, DAIKIN_BIT_POWERFUL);
    // Powerful, Quiet, & Econo mode being on are mutually exclusive.
    setQuiet(false);
    setEcono(false);
  } else {
    clearBit(DAIKIN_BYTE_POWERFUL, DAIKIN_BIT_POWERFUL);
  }
}

bool IRDaikinESP::getPowerful() {
  return (getBit(DAIKIN_BYTE_POWERFUL, DAIKIN_BIT_POWERFUL) > 0);
}

void IRDaikinESP::setSensor(bool state) {
  if (state)
    setBit(DAIKIN_BYTE_SENSOR, DAIKIN_BIT_SENSOR);
  else
    clearBit(DAIKIN_BYTE_SENSOR, DAIKIN_BIT_SENSOR);
}

bool IRDaikinESP::getSensor() {
  return (getBit(DAIKIN_BYTE_SENSOR, DAIKIN_BIT_SENSOR) > 0);
}

void IRDaikinESP::setEcono(bool state) {
  if (state) {
    setBit(DAIKIN_BYTE_ECONO, DAIKIN_BIT_ECONO);
    // Powerful & Econo mode being on are mutually exclusive.
    setPowerful(false);
  } else {
    clearBit(DAIKIN_BYTE_ECONO, DAIKIN_BIT_ECONO);
  }
}

bool IRDaikinESP::getEcono() {
  return (getBit(DAIKIN_BYTE_ECONO, DAIKIN_BIT_ECONO) > 0);
}

void IRDaikinESP::setEye(bool state) {
  if (state)
    setBit(DAIKIN_BYTE_EYE, DAIKIN_BIT_EYE);
  else
    clearBit(DAIKIN_BYTE_EYE, DAIKIN_BIT_EYE);
}

bool IRDaikinESP::getEye() {
  return (getBit(DAIKIN_BYTE_EYE, DAIKIN_BIT_EYE) > 0);
}

void IRDaikinESP::setMold(bool state) {
  if (state)
    setBit(DAIKIN_BYTE_MOLD, DAIKIN_BIT_MOLD);
  else
    clearBit(DAIKIN_BYTE_MOLD, DAIKIN_BIT_MOLD);
}

bool IRDaikinESP::getMold() {
  return (getBit(DAIKIN_BYTE_MOLD, DAIKIN_BIT_MOLD) > 0);
}

void IRDaikinESP::setBit(uint8_t byte, uint8_t bitmask) {
  daikin[byte] |= bitmask;
}

void IRDaikinESP::clearBit(uint8_t byte, uint8_t bitmask) {
  bitmask = ~bitmask;
  daikin[byte] &= bitmask;
}

uint8_t IRDaikinESP::getBit(uint8_t byte, uint8_t bitmask) {
  return daikin[byte] & bitmask;
}

// starttime: Number of minutes after midnight, in 10 minutes increments
void IRDaikinESP::enableOnTimer(uint16_t starttime) {
  setBit(DAIKIN_BYTE_ON_TIMER, DAIKIN_BIT_ON_TIMER);
  daikin[18] = (uint8_t) (starttime & 0x00FF);
  // only keep 4 bits
  daikin[19] &= 0xF0;
  daikin[19] |= (uint8_t) ((starttime >> 8) & 0x0F);
}

void IRDaikinESP::disableOnTimer() {
  enableOnTimer(0x600);
  clearBit(DAIKIN_BYTE_ON_TIMER, DAIKIN_BIT_ON_TIMER);
}

uint16_t IRDaikinESP::getOnTime() {
  uint16_t ret;
  ret = daikin[19] & 0x0F;
  ret = ret << 8;
  ret += daikin[18];
  return ret;
}

bool IRDaikinESP::getOnTimerEnabled() {
  return getBit(DAIKIN_BYTE_ON_TIMER, DAIKIN_BIT_ON_TIMER);
}

// endtime: Number of minutes after midnight, in 10 minutes increments
void IRDaikinESP::enableOffTimer(uint16_t endtime) {
  setBit(DAIKIN_BYTE_OFF_TIMER, DAIKIN_BIT_OFF_TIMER);
  daikin[20] = (uint8_t)((endtime >> 4) & 0xFF);
  daikin[19] &= 0x0F;
  daikin[19] |= (uint8_t) ((endtime & 0x000F) << 4);
}

void IRDaikinESP::disableOffTimer() {
  enableOffTimer(0x600);
  clearBit(DAIKIN_BYTE_OFF_TIMER, DAIKIN_BIT_OFF_TIMER);
}

uint16_t IRDaikinESP::getOffTime() {
  uint16_t ret, tmp;
  ret = daikin[20];
  ret <<= 4;
  tmp = daikin[19] & 0xF0;
  tmp >>= 4;
  ret += tmp;
  return ret;
}

bool IRDaikinESP::getOffTimerEnabled() {
  return getBit(DAIKIN_BYTE_OFF_TIMER, DAIKIN_BIT_OFF_TIMER);
}

void IRDaikinESP::setCurrentTime(uint16_t numMins) {
  if (numMins > 24 * 60) numMins = 0;  // If > 23:59, set to 00:00
  daikin[5] = (uint8_t) (numMins & 0x00FF);
  // only keep 4 bits
  daikin[6] &= 0xF0;
  daikin[6] |= (uint8_t) ((numMins >> 8) & 0x0F);
}

uint16_t IRDaikinESP::getCurrentTime() {
  uint16_t ret;
  ret = daikin[6] & 0x0F;
  ret <<= 8;
  ret += daikin[5];
  return ret;
}

#ifdef ARDUINO
String IRDaikinESP::renderTime(uint16_t timemins) {
  String ret;
#else  // ARDUINO
std::string IRDaikinESP::renderTime(uint16_t timemins) {
  std::string ret;
#endif  // ARDUINO
  uint16_t hours, mins;
  hours = timemins / 60;
  ret = uint64ToString(hours) + ":";
  mins = timemins - (hours * 60);
  if (mins < 10)
    ret += "0";
  ret += uint64ToString(mins);
  return ret;
}

// Convert the internal state into a human readable string.
#ifdef ARDUINO
String IRDaikinESP::toString() {
  String result = "";
#else  // ARDUINO
std::string IRDaikinESP::toString() {
  std::string result = "";
#endif  // ARDUINO
  result += "Power: ";
  if (getPower())
    result += "On";
  else
    result += "Off";
  result += ", Mode: " + uint64ToString(getMode());
  switch (getMode()) {
    case DAIKIN_AUTO:
      result += " (AUTO)";
      break;
    case DAIKIN_COOL:
      result += " (COOL)";
      break;
    case DAIKIN_HEAT:
      result += " (HEAT)";
      break;
    case DAIKIN_DRY:
      result += " (DRY)";
      break;
    case DAIKIN_FAN:
      result += " (FAN)";
      break;
    default:
      result += " (UNKNOWN)";
  }
  result += ", Temp: " + uint64ToString(getTemp()) + "C";
  result += ", Fan: " + uint64ToString(getFan());
  switch (getFan()) {
    case DAIKIN_FAN_AUTO:
      result += " (AUTO)";
      break;
    case DAIKIN_FAN_QUIET:
      result += " (QUIET)";
      break;
    case DAIKIN_FAN_MIN:
      result += " (MIN)";
      break;
    case DAIKIN_FAN_MAX:
      result += " (MAX)";
      break;
  }
  result += ", Powerful: ";
  if (getPowerful())
    result += "On";
  else
    result += "Off";
  result += ", Quiet: ";
  if (getQuiet())
    result += "On";
  else
    result += "Off";
  result += ", Sensor: ";
  if (getSensor())
    result += "On";
  else
    result += "Off";
  result += ", Eye: ";
  if (getEye())
    result += "On";
  else
    result += "Off";
  result += ", Mold: ";
  if (getMold())
    result += "On";
  else
    result += "Off";
  result += ", Swing (Horizontal): ";
  if (getSwingHorizontal())
    result += "On";
  else
    result += "Off";
  result += ", Swing (Vertical): ";
  if (getSwingVertical())
    result += "On";
  else
    result += "Off";
  result += ", Current Time: " + renderTime(getCurrentTime());
  result += ", On Time: ";
  if (getOnTimerEnabled())
    result += renderTime(getOnTime());
  else
    result += "Off";
  result += ", Off Time: ";
  if (getOffTimerEnabled())
    result += renderTime(getOffTime());
  else
    result += "Off";

  return result;
}

#if DAIKIN_DEBUG
// Print what we have
void IRDaikinESP::printState() {
#ifdef ARDUINO
  String strbits;
#else  // ARDUINO
  std::string strbits;
#endif  // ARDUINO
  DPRINTLN("Raw Bits:");
  for (uint8_t i = 0; i < DAIKIN_COMMAND_LENGTH; i++) {
    strbits = uint64ToString(daikin[i], BIN);
    while (strbits.length() < 8)
      strbits = "0" + strbits;
    DPRINT(strbits);
    DPRINT(" ");
  }
  DPRINTLN("");
  DPRINTLN(toString());
}
#endif  // DAIKIN_DEBUG

/*
 * Return most important bits to allow replay
 * layout is:
 *      0:      Power
 *      1-3:    Mode
 *      4-7:    Fan speed/mode
 *      8-14:   Target Temperature
 *      15:     Econo
 *      16:     Powerful
 *      17:     Quiet
 *      18:     Sensor
 *      19:     Swing Vertical
 *      20-31:  Current time (mins since midnight)
 * */
uint32_t IRDaikinESP::getCommand() {
  uint32_t ret = 0;
  uint32_t tmp = 0;
  if (getPower())
    ret |= 0b00000000000000000000000000000001;
  tmp = getMode();
  tmp = tmp << 1;
  ret |= tmp;

  tmp = getFan();
  tmp <<= 4;
  ret |= tmp;

  tmp = getTemp();
  tmp <<= 8;
  ret |= tmp;

  if (getEcono())
    ret |= 0b00000000000000001000000000000000;
  if (getPowerful())
    ret |= 0b00000000000000010000000000000000;
  if (getQuiet())
    ret |= 0b00000000000000100000000000000000;
  if (getSensor())
    ret |= 0b00000000000001000000000000000000;
  if (getSwingVertical())
    ret |= 0b00000000000010000000000000000000;
  ret |= (getCurrentTime() << 20);
  return ret;
}

void IRDaikinESP::setCommand(uint32_t value) {
  uint32_t tmp = 0;
  if (value & 0b00000000000000000000000000000001)
    setPower(true);
  tmp = value & 0b00000000000000000000000000001110;
  tmp >>= 1;
  setMode(tmp);

  tmp = value & 0b00000000000000000000000011110000;
  tmp >>= 4;
  setFan(tmp);

  tmp = value & 0b00000000000000000111111100000000;
  tmp >>= 8;
  setTemp(tmp);

  if (value & 0b00000000000000001000000000000000)
    setEcono(true);
  if (value & 0b00000000000000010000000000000000)
    setPowerful(true);
  if (value & 0b00000000000000100000000000000000)
    setQuiet(true);
  if (value & 0b00000000000001000000000000000000)
    setSensor(true);
  if (value & 0b00000000000010000000000000000000)
    setSwingVertical(true);

  value >>= 20;
  setCurrentTime(value);
}
#endif  // (SEND_DAIKIN || DECODE_DAIKIN)

#if DECODE_DAIKIN

void addbit(bool val, unsigned char data[]) {
  uint8_t curbit = data[DAIKIN_CURBIT];
  uint8_t curindex = data[DAIKIN_CURINDEX];
  if (val) {
    unsigned char bit = 1;
    bit = bit << curbit;
    data[curindex] |= bit;
  }
  curbit++;
  if (curbit == 8) {
    curbit = 0;
    curindex++;
  }
  data[DAIKIN_CURBIT] = curbit;
  data[DAIKIN_CURINDEX] = curindex;
}

bool checkheader(decode_results *results, uint16_t* offset) {
  if (!IRrecv::matchMark(results->rawbuf[(*offset)++], DAIKIN_BIT_MARK,
                         DAIKIN_TOLERANCE, DAIKIN_MARK_EXCESS))
    return false;
  if (!IRrecv::matchSpace(results->rawbuf[(*offset)++],
                          DAIKIN_ZERO_SPACE + DAIKIN_GAP,
                          DAIKIN_TOLERANCE, DAIKIN_MARK_EXCESS))
    return false;
  if (!IRrecv::matchMark(results->rawbuf[(*offset)++], DAIKIN_HDR_MARK,
                         DAIKIN_TOLERANCE, DAIKIN_MARK_EXCESS))
    return false;
  if (!IRrecv::matchSpace(results->rawbuf[(*offset)++], DAIKIN_HDR_SPACE,
                          DAIKIN_TOLERANCE, DAIKIN_MARK_EXCESS))
    return false;

  return true;
}

bool readbits(decode_results *results, uint16_t *offset,
              unsigned char daikin_code[], uint16_t countbits) {
  for (uint16_t i = 0; i < countbits && *offset < results->rawlen - 1;
       i++, (*offset)++) {
    if (!IRrecv::matchMark(results->rawbuf[(*offset)++], DAIKIN_BIT_MARK,
              DAIKIN_TOLERANCE, DAIKIN_MARK_EXCESS))
      return false;
    if (IRrecv::matchSpace(results->rawbuf[*offset], DAIKIN_ONE_SPACE,
              DAIKIN_TOLERANCE, DAIKIN_MARK_EXCESS))
      addbit(1, daikin_code);
    else if (IRrecv::matchSpace(results->rawbuf[*offset], DAIKIN_ZERO_SPACE,
              DAIKIN_TOLERANCE, DAIKIN_MARK_EXCESS))
      addbit(0, daikin_code);
    else
      return false;
  }
  return true;
}

// Decode the supplied Daikin A/C message.
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of bits to expect in the data portion. (DAIKIN_RAW_BITS)
//   strict:  Flag to indicate if we strictly adhere to the specification.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: BETA / Should be working.
//
// Notes:
//   If DAIKIN_DEBUG enabled, will print all the set options and values.
//
// Ref:
//   https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/tree/master/IRremote
bool IRrecv::decodeDaikin(decode_results *results, uint16_t nbits,
                          bool strict) {
  if (results->rawlen < DAIKIN_RAW_BITS)
    return false;

  // Compliance
  if (strict && nbits != DAIKIN_RAW_BITS)
    return false;

  uint16_t offset = OFFSET_START;
  unsigned char daikin_code[DAIKIN_COMMAND_LENGTH + 2];
  for (uint8_t i = 0; i < DAIKIN_COMMAND_LENGTH+2; i++)
    daikin_code[i] = 0;

  // Header (#1)
  for (uint8_t i = 0; i < 10; i++) {
    if (!matchMark(results->rawbuf[offset++], DAIKIN_BIT_MARK))
      return false;
  }
  if (!checkheader(results, &offset)) return false;

  // Data (#1)
  if (!readbits(results, &offset, daikin_code, 8 * 8)) return false;

  // Ignore everything that has just been captured as it is not needed.
  // Some remotes may not send this portion, my remote did, but it's not
  // required.
  for (uint8_t i = 0; i < DAIKIN_COMMAND_LENGTH + 2; i++)
    daikin_code[i] = 0;

  // Header (#2)
  if (!checkheader(results, &offset)) return false;

  // Data (#2)
  if (!readbits(results, &offset, daikin_code, 8 * 8)) return false;

  // Header (#3)
  if (!checkheader(results, &offset)) return false;

  // Data (#3), read up everything else
  if (!readbits(results, &offset, daikin_code, DAIKIN_BITS - (8 * 8)))
    return false;

  // Footer
  if (!matchMark(results->rawbuf[offset++], DAIKIN_BIT_MARK))
    return false;
  if (offset <= results->rawlen && !matchAtLeast(results->rawbuf[offset],
                                                 DAIKIN_GAP))
    return false;

  // Compliance
  if (strict) {
    if (!IRDaikinESP::validChecksum(daikin_code)) return false;
  }

  // Success
#if DAIKIN_DEBUG
  IRDaikinESP dako = IRDaikinESP(0);
  dako.setRaw(daikin_code);
#ifdef ARDUINO
  yield();
#endif  // ARDUINO
  dako.printState();
#endif  // DAIKIN_DEBUG

  // Copy across the bits to state
  for (uint8_t i = 0; i < DAIKIN_COMMAND_LENGTH; i++)
    results->state[i] = daikin_code[i];
  results->bits = DAIKIN_COMMAND_LENGTH * 8;
  results->decode_type = DAIKIN;
  return true;
}
#endif  // DECODE_DAIKIN
