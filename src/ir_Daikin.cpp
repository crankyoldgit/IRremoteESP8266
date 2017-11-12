/*
An Arduino sketch to emulate IR Daikin ARC433** remote control unit
Read more at:
http://harizanov.com/2012/02/control-daikin-air-conditioner-over-the-internet/

Copyright 2017 sillyfrog
*/

#include "ir_Daikin.h"
#include <algorithm>
#include "IRremoteESP8266.h"
#include "IRutils.h"
#include "IRrecv.h"

//                DDDDD     AAA   IIIII KK  KK IIIII NN   NN
//                DD  DD   AAAAA   III  KK KK   III  NNN  NN
//                DD   DD AA   AA  III  KKKK    III  NN N NN
//                DD   DD AAAAAAA  III  KK KK   III  NN  NNN
//                DDDDDD  AA   AA IIIII KK  KK IIIII NN   NN

// Constants
// Ref:
//   https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/tree/master/IRremote
//   http://rdlab.cdmt.vn/project-2013/daikin-ir-protocol





#define DAIKIN_HDR_MARK            3650U  // DAIKIN_ZERO_MARK * 8
#define DAIKIN_HDR_SPACE           1623U  // DAIKIN_ZERO_MARK * 4
#define DAIKIN_ONE_SPACE           1280U
#define DAIKIN_ONE_MARK             428U
#define DAIKIN_ZERO_MARK            428U
#define DAIKIN_ZERO_SPACE           428U
#define DAIKIN_GAP                29000U

#if SEND_DAIKIN

// Note bits in each octet swapped so can be sent as a single value
#define FIRST_HEADER64 0b1101011100000000000000001100010100000000001001111101101000010001
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
    sendData(DAIKIN_ONE_MARK, DAIKIN_ONE_SPACE, DAIKIN_ZERO_MARK,
             DAIKIN_ZERO_SPACE, 0, 5, false);
    sendDaikinGapHeader();
    // Leading header
    // Do this as a constant to save RAM and keep in flash memory
    sendData(DAIKIN_ONE_MARK, DAIKIN_ONE_SPACE, DAIKIN_ZERO_MARK,
             DAIKIN_ZERO_SPACE, FIRST_HEADER64, 64, false);
    sendDaikinGapHeader();
    // Data #1
    for (uint16_t i = 0; i < 8 && i < nbytes; i++)
      sendData(DAIKIN_ONE_MARK, DAIKIN_ONE_SPACE, DAIKIN_ZERO_MARK,
               DAIKIN_ZERO_SPACE, data[i], 8, false);
    sendDaikinGapHeader();
    // Data #2
    for (uint16_t i = 8; i < nbytes; i++)
      sendData(DAIKIN_ONE_MARK, DAIKIN_ONE_SPACE, DAIKIN_ZERO_MARK,
               DAIKIN_ZERO_SPACE, data[i], 8, false);
    // Footer #2
    mark(DAIKIN_ONE_MARK);
    space(DAIKIN_ZERO_SPACE + DAIKIN_GAP);
  }
}

void IRsend::sendDaikinGapHeader() {
  mark(DAIKIN_ONE_MARK);
  space(DAIKIN_ZERO_SPACE + DAIKIN_GAP);
  mark(DAIKIN_HDR_MARK);
  space(DAIKIN_HDR_SPACE);
}

IRDaikinESP::IRDaikinESP(uint16_t pin) : _irsend(pin) {
  stateReset();
}

void IRDaikinESP::begin() {
  _irsend.begin();
}

void IRDaikinESP::send() {
  _irsend.sendDaikin(daikin);
}

void IRDaikinESP::checksum() {
  uint8_t sum = 0;
  uint8_t i;

  for (i = 0; i <= 6; i++)
    sum += daikin[i];

  daikin[7] = sum & 0xFF;
  sum = 0;
  for (i = 8; i <= 25; i++)
    sum += daikin[i];
  daikin[26] = sum & 0xFF;
}


void IRDaikinESP::stateReset() {
  for (uint8_t i = 4; i < DAIKIN_COMMAND_LENGTH; i++)
    daikin[i] = 0x0;

  daikin[0] = 0x11;
  daikin[1] = 0xDA;
  daikin[2] = 0x27;
  daikin[4] = 0x42;
  daikin[8] = 0x11;
  daikin[9] = 0xDA;
  daikin[10] = 0x27;
  daikin[13] = 0x49;
  daikin[14] = 0x1E;
  daikin[16] = 0xB0;
  daikin[19] = 0x06;
  daikin[20] = 0x60;
  daikin[23] = 0xC0;
  daikin[26] = 0xE3;
  checksum();
}

uint8_t* IRDaikinESP::getRaw() {
  checksum();   // Ensure correct settings before sending.
  return daikin;
}

void IRDaikinESP::setRaw(uint8_t new_code[]) {
  for (uint8_t i = 0; i < DAIKIN_COMMAND_LENGTH; i++) {
    daikin[i] = new_code[i];
  }
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
  checksum();
}

uint8_t IRDaikinESP::getTemp() {
  return daikin[14] / 2;
}

// Set the speed of the fan, 1-5 or DAIKIN_FAN_AUTO or DAIKIN_FAN_QUITE
void IRDaikinESP::setFan(uint8_t fan) {
  // Set the fan speed bits, leave low 4 bits alone
  uint8_t fanset;
  if (fan == DAIKIN_FAN_QUITE || fan == DAIKIN_FAN_AUTO)
    fanset = fan;
  else if (fan < DAIKIN_FAN_MIN || fan > DAIKIN_FAN_MAX)
    fanset = DAIKIN_FAN_AUTO;
  else
    fanset = 2 + fan;
  fanset = fanset << 4;
  daikin[16] &= 0x0F;
  daikin[16] |= fanset;
  checksum();
}

uint8_t IRDaikinESP::getFan() {
  uint8_t fan = daikin[16] >> 4;
  if (fan == DAIKIN_FAN_QUITE || fan == DAIKIN_FAN_AUTO)
    {} // pass
  else
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
  mode = mode << 4;
  daikin[13] &= 0b10001111;
  daikin[13] |= mode;
  checksum();
}

void IRDaikinESP::setSwingVertical(bool state) {
  if (state)
    daikin[16] |= 0x0F;
  else
    daikin[16] &= 0xF0;
  checksum();
}

bool IRDaikinESP::getSwingVertical() {
  return daikin[16] & 0x01;
}

void IRDaikinESP::setSwingHorizontal(bool state) {
  if (state)
    daikin[17] |= 0x0F;
  else
    daikin[17] &= 0xF0;
  checksum();
}

bool IRDaikinESP::getSwingHorizontal() {
  return daikin[17] & 0x01;
}


void IRDaikinESP::setQuiet(bool state) {
  if (state)
    setBit(DAIKIN_BYTE_SILENT, DAIKIN_BIT_SILENT);
  else
    clearBit(DAIKIN_BYTE_SILENT, DAIKIN_BIT_SILENT);
}

bool IRDaikinESP::getQuiet() {
  return (getBit(DAIKIN_BYTE_SILENT, DAIKIN_BIT_SILENT) > 0);
}


void IRDaikinESP::setPowerful(bool state) {
  if (state)
    setBit(DAIKIN_BYTE_POWERFUL, DAIKIN_BIT_POWERFUL);
  else
    clearBit(DAIKIN_BYTE_POWERFUL, DAIKIN_BIT_POWERFUL);
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
  if (state)
    setBit(DAIKIN_BYTE_ECONO, DAIKIN_BIT_ECONO);
  else
    clearBit(DAIKIN_BYTE_ECONO, DAIKIN_BIT_ECONO);
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
  checksum();
}


void IRDaikinESP::clearBit(uint8_t byte, uint8_t bitmask) {
  bitmask = ~bitmask;
  daikin[byte] &= bitmask;
  checksum();
}

uint8_t IRDaikinESP::getBit(uint8_t byte, uint8_t bitmask) {
  uint8_t ret;
  ret = daikin[byte] & bitmask;
  return ret;
}

// starttime: Number of minutes after midnight, in 10 minutes increments
void IRDaikinESP::enableOnTimer(uint16_t starttime) {
  daikin[13] |= 0b00000010;
  uint16_t lopbits;
  lopbits = starttime;
  lopbits &= 0x00FF;
  daikin[18] = lopbits;
  starttime = starttime >> 8;
  // only keep 4 bits
  daikin[19] &= 0xF0;
  daikin[19] |= starttime;
  checksum();
}

void IRDaikinESP::disableOnTimer() {
  enableOnTimer(0x600);
  daikin[13] &= 0b11111101;
  checksum();
}

uint16_t IRDaikinESP::getOnTime() {
  uint16_t ret;
  ret = daikin[19] & 0x0F;
  ret = ret << 8;
  ret += daikin[18];
  return ret;
}
bool IRDaikinESP::getOnTimerEnabled() {
    return daikin[13] & 0b00000010;
}

// endtime: Number of minutes after midnight, in 10 minutes increments
void IRDaikinESP::enableOffTimer(uint16_t endtime) {
  daikin[13] |= 0b00000100;
  uint16_t lopbits;
  lopbits = endtime;
  lopbits &= 0x0FF0;
  lopbits = lopbits >> 4;
  daikin[20] = lopbits;
  endtime &= 0x000F;
  endtime = endtime << 4;
  daikin[19] &= 0x0F;
  daikin[19] |= endtime;
  checksum();
}

void IRDaikinESP::disableOffTimer() {
  enableOffTimer(0x600);
  daikin[13] &= 0b11111011;
  checksum();
}

uint16_t IRDaikinESP::getOffTime() {
  uint16_t ret, tmp;
  ret = daikin[20];
  ret = ret << 4;
  tmp = daikin[19] & 0xF0;
  tmp = tmp >> 4;
  ret += tmp;
  return ret;
}

bool IRDaikinESP::getOffTimerEnabled() {
  return daikin[13] & 0b00000100;
}

void IRDaikinESP::setCurrentTime(uint16_t time) {
  uint16_t lopbits;
  lopbits = time;
  lopbits &= 0x00FF;
  daikin[5] = lopbits;
  time = time >> 8;
  // only keep 4 bits
  daikin[6] &= 0xF0;
  daikin[6] |= time;
  checksum();
}

uint16_t IRDaikinESP::getCurrentTime() {
  uint16_t ret;
  ret = daikin[6] & 0x0F;
  ret = ret << 8;
  ret += daikin[5];
  return ret;
}

String IRDaikinESP::renderTime(uint16_t timemins) {
  uint16_t hours, mins;
  hours = timemins / 60;
  mins = timemins - (hours * 60);
  String ret = String(mins);
  if (ret.length() == 1)
    ret = "0" + ret;
  ret = ":" + ret;
  ret = String(hours) + ret;
  return ret;
}


#if DAIKIN_DEBUG

void IRDaikinESP::printState() {
  // Print what we have
  Serial.println("Raw Bits:");
  for (uint8_t i = 0; i < DAIKIN_COMMAND_LENGTH; i++) {
    String strbits = String(daikin[i], BIN);
    while (strbits.length() < 8)
      strbits = String("0") + strbits;
    Serial.print(strbits);
    Serial.print(" ");
  }
  Serial.println("");

  // Human readable
  uint8_t var;
  Serial.print("Power: ");
  Serial.println(getPower() ? "On" : "Off");

  Serial.print("Temperature: ");
  Serial.println(getTemp());

  Serial.print("Quiet: ");
  Serial.println(getQuiet() ? "On" : "Off");

  Serial.print("Sensor: ");
  Serial.println(getSensor() ? "On" : "Off");

  Serial.print("Powerful: ");
  Serial.println(getPowerful() ? "On" : "Off");

  Serial.print("Econo: ");
  Serial.println(getEcono() ? "On" : "Off");

  Serial.print("Eye: ");
  Serial.println(getEye() ? "On" : "Off");

  Serial.print("Mold: ");
  Serial.println(getMold() ? "On" : "Off");


  Serial.print("Swing Vertical: ");
  Serial.println(getSwingVertical() ? "On" : "Off");

  Serial.print("Swing Horizontal: ");
  Serial.println(getSwingHorizontal() ? "On" : "Off");

  Serial.print("Fan Speed: ");
  var = getFan();
  if (var == DAIKIN_FAN_AUTO)
    Serial.println("Auto");
  else if (var == DAIKIN_FAN_QUITE)
    Serial.println("Quite");
  else
    Serial.println(var);

  Serial.print("Mode: ");
  switch (getMode()) {
    case DAIKIN_COOL:
        Serial.println("Cool");
        break;
    case DAIKIN_HEAT:
        Serial.println("Heat");
        break;
    case DAIKIN_FAN:
        Serial.println("Fan");
        break;
    case DAIKIN_AUTO:
        Serial.println("Auto");
        break;
    case DAIKIN_DRY:
        Serial.println("Dry");
        break;
  }

  var = getOnTimerEnabled();
  Serial.print("On Timer: ");
  Serial.println(var ? "Enabled" : "Off");
  if (var) {
    Serial.print("On Time: ");
    Serial.println(renderTime(getOnTime()));
  }

  var = getOffTimerEnabled();
  Serial.print("Off Timer: ");
  Serial.println(var ? "Enabled" : "Off");
  if (var) {
    Serial.print("Off Time: ");
    Serial.println(renderTime(getOffTime()));
  }

  Serial.print("Current Time: ");
  Serial.println(renderTime(getCurrentTime()));

}

#endif // DAIKIN_DEBUG

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
  tmp = tmp << 4;
  ret |= tmp;

  tmp = getTemp();
  tmp = tmp << 8;
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
  uint32_t time = getCurrentTime();
  time = time << 20;
  ret |= time;
  return ret;
}

void IRDaikinESP::setCommand(uint32_t value) {
  uint32_t tmp = 0;
  if (value & 0b00000000000000000000000000000001)
    setPower(true);
  tmp = value & 0b00000000000000000000000000001110;
  tmp = tmp >> 1;
  setMode(tmp);

  tmp = value & 0b00000000000000000000000011110000;
  tmp = tmp >> 4;
  setFan(tmp);

  tmp = value & 0b00000000000000000111111100000000;
  tmp = tmp >> 8;
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

  value = value >> 20;
  setCurrentTime(value);
}

#endif  // SEND_DAIKIN

#if DECODE_DAIKIN

#define DAIKIN_CURBIT DAIKIN_COMMAND_LENGTH
#define DAIKIN_CURINDEX DAIKIN_COMMAND_LENGTH+1
#define OFFSET_ERR 65432

#define DAIKIN_TOLERANCE 35
#define DAIKIN_MARK_EXCESS MARK_EXCESS

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

uint16_t checkheader(decode_results *results, uint16_t offset, unsigned char daikin_code[]) {
  if (!IRrecv::matchMark(results->rawbuf[offset++], DAIKIN_ZERO_MARK,
              DAIKIN_TOLERANCE, DAIKIN_MARK_EXCESS))
    return OFFSET_ERR;
  if (!IRrecv::matchSpace(results->rawbuf[offset++], DAIKIN_ZERO_SPACE + DAIKIN_GAP,
              DAIKIN_TOLERANCE, DAIKIN_MARK_EXCESS))
    return OFFSET_ERR;
  if (!IRrecv::matchMark(results->rawbuf[offset++], DAIKIN_HDR_MARK,
              DAIKIN_TOLERANCE, DAIKIN_MARK_EXCESS))
    return OFFSET_ERR;
  if (!IRrecv::matchSpace(results->rawbuf[offset++], DAIKIN_HDR_SPACE,
              DAIKIN_TOLERANCE, DAIKIN_MARK_EXCESS))
    return OFFSET_ERR;
  /*
  if (daikin_code[DAIKIN_CURBIT] != 0) {
    daikin_code[DAIKIN_CURBIT] = 0;
    daikin_code[DAIKIN_CURINDEX]++;
  }
  */
  return offset;
}

uint16_t readbits(decode_results *results, uint16_t offset, unsigned char daikin_code[], uint16_t countbits) {
  for (uint16_t i = 0; i < countbits && offset < results->rawlen-1; i++, offset++) {
    if (!IRrecv::matchMark(results->rawbuf[offset++], DAIKIN_ONE_MARK,
              DAIKIN_TOLERANCE, DAIKIN_MARK_EXCESS))
      return OFFSET_ERR;
    if (IRrecv::matchSpace(results->rawbuf[offset], DAIKIN_ONE_SPACE,
              DAIKIN_TOLERANCE, DAIKIN_MARK_EXCESS))
      addbit(1, daikin_code);
    else if (IRrecv::matchSpace(results->rawbuf[offset], DAIKIN_ZERO_SPACE,
              DAIKIN_TOLERANCE, DAIKIN_MARK_EXCESS))
      addbit(0, daikin_code);
    else
      return OFFSET_ERR;
  }
  return offset;
}


// Decode the supplied Daikin A/C message.
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of bits to expect in the data portion. Typically SAMSUNG_BITS.
//   strict:  Flag to indicate if we strictly adhere to the specification.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: Working, return command is incomplete as there is too much data,
//         if DAIKIN_DEBUG enabled, will print all the set options and values.
//
// Ref:
//   https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/tree/master/IRremote
bool IRrecv::decodeDaikin(decode_results *results, uint16_t nbits,
                          bool strict) {
  if (results->rawlen < DAIKIN_BITS)
    return false;

  // Compliance
  if (strict && nbits != DAIKIN_BITS)
    return false;

  uint32_t data = 0;
  uint16_t offset = OFFSET_START;
  unsigned char daikin_code[DAIKIN_COMMAND_LENGTH + 2];
  for (uint8_t i = 0; i < DAIKIN_COMMAND_LENGTH+2; i++)
    daikin_code[i] = 0;
  uint8_t code_bit = 0;
  uint8_t code_index = 0;

  for (uint8_t i = 0; i < 10; i++) {
    if (!matchMark(results->rawbuf[offset++], DAIKIN_ZERO_MARK))
      return false;
  }
  // Daikin GAP
  offset = checkheader(results, offset, daikin_code);
  if (offset == OFFSET_ERR)
      return false;

  // Data (#1)
  offset = readbits(results, offset, daikin_code, 8*8);
  if (offset == OFFSET_ERR)
      return false;

  // Ignore everything that has just been captured as it is not needed.
  // Some remotes may not send this portion, my remote did, but it's not required.
  for (uint8_t i = 0; i < DAIKIN_COMMAND_LENGTH+2; i++)
    daikin_code[i] = 0;

  offset = checkheader(results, offset, daikin_code);
  if (offset == OFFSET_ERR)
      return false;

  // Data (#2)
  offset = readbits(results, offset, daikin_code, 8*8);
  if (offset == OFFSET_ERR)
      return false;

  offset = checkheader(results, offset, daikin_code);
  if (offset == OFFSET_ERR)
      return false;

  // Data (#3), read up everything else
  offset = readbits(results, offset, daikin_code, (DAIKIN_COMMAND_LENGTH*8)-(8*8));
  if (offset == OFFSET_ERR)
      return false;

  yield();

  // Success
  IRDaikinESP dako = IRDaikinESP(0);
  dako.setRaw(daikin_code);
#if DAIKIN_DEBUG
  dako.printState();
#endif

  // Copy across the bits to state
  for (uint8_t i = 0; i < DAIKIN_COMMAND_LENGTH; i++)
    results->state[i] = daikin_code[i];
  results->bits = DAIKIN_COMMAND_LENGTH * 8;
  results->decode_type = DAIKIN;
  //results->command = dako.getCommand(); // include the common options
  return true;
}
#endif  // DECODE_DAIKIN
