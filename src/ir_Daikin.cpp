/*
An Arduino sketch to emulate IR Daikin ARC433** remote control unit
Read more at:
http://harizanov.com/2012/02/control-daikin-air-conditioner-over-the-internet/

Copyright 2016 sillyfrog
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
#define DAIKIN_HDR_MARK            3650U  // DAIKIN_ZERO_MARK * 8
#define DAIKIN_HDR_SPACE           1623U  // DAIKIN_ZERO_MARK * 4
#define DAIKIN_ONE_SPACE           1280U
#define DAIKIN_ONE_MARK             428U
#define DAIKIN_ZERO_MARK            428U
#define DAIKIN_ZERO_SPACE           428U
#define DAIKIN_GAP                29000U

#if SEND_DAIKIN
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
    // Header #1
    mark(DAIKIN_HDR_MARK);
    space(DAIKIN_HDR_SPACE);
    // Data #1
    for (uint16_t i = 0; i < 8 && i < nbytes; i++)
      sendData(DAIKIN_ONE_MARK, DAIKIN_ONE_SPACE, DAIKIN_ZERO_MARK,
               DAIKIN_ZERO_SPACE, data[i], 8, false);
    // Footer #1
    mark(DAIKIN_ONE_MARK);
    space(DAIKIN_ZERO_SPACE + DAIKIN_GAP);

    // Header #2
    mark(DAIKIN_HDR_MARK);
    space(DAIKIN_HDR_SPACE);
    // Data #2
    for (uint16_t i = 8; i < nbytes; i++)
      sendData(DAIKIN_ONE_MARK, DAIKIN_ONE_SPACE, DAIKIN_ZERO_MARK,
               DAIKIN_ZERO_SPACE, data[i], 8, false);
    // Footer #2
    mark(DAIKIN_ONE_MARK);
    space(DAIKIN_ZERO_SPACE + DAIKIN_GAP);
  }
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
  daikin[3] = 0xF0;
  daikin[7] = 0x20;
  daikin[8] = 0x11;
  daikin[9] = 0xDA;
  daikin[10] = 0x27;
  daikin[13] = 0x41;
  daikin[14] = 0x1E;
  daikin[16] = 0xB0;
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
  daikin[13] |= 0x01;
  checksum();
}

void IRDaikinESP::off() {
  // state = OFF;
  daikin[13] &= 0xFE;
  checksum();
}

void IRDaikinESP::setPower(bool state) {
  if (state)
    on();
  else
    off();
}

uint8_t IRDaikinESP::getPower() {
  return daikin[13] & 0x01;
}

// DAIKIN_SILENT or DAIKIN_POWERFUL
void IRDaikinESP::setAux(uint8_t aux) {
  daikin[21] = aux;
  checksum();
}

uint8_t IRDaikinESP::getAux() {
  return daikin[21];
}

void IRDaikinESP::setQuiet(bool state) {
  if (state)
    setAux(DAIKIN_SILENT);
  else
    setAux(0x0);
}

bool IRDaikinESP::getQuiet() {
  return (getAux() == DAIKIN_SILENT);
}

void IRDaikinESP::setPowerful(bool state) {
  if (state)
    setAux(DAIKIN_POWERFUL);
  else
    setAux(0x0);
}

bool IRDaikinESP::getPowerful() {
  return (getAux() == DAIKIN_POWERFUL);
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

// Set the speed of the fan, 0-5, 0 is auto, 1-5 is the speed
void IRDaikinESP::setFan(uint8_t fan) {
  // Set the fan speed bits, leave low 4 bits alone
  uint8_t fanset;
  daikin[16] &= 0x0F;
  fan = std::min(fan, DAIKIN_FAN_MAX);
  if (fan == DAIKIN_FAN_AUTO)
    fanset = 0xA0;
  else
    fanset = 0x20 + (0x10 * fan);
  daikin[16] |= fanset;
  checksum();
}

uint8_t IRDaikinESP::getFan() {
  uint8_t fan = daikin[16] >> 4;
  fan -= 2;
  if (fan > DAIKIN_FAN_MAX)
    fan = DAIKIN_FAN_AUTO;
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
  daikin[13] = (mode << 4) | getPower();
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

void IRDaikinESP::setEcono(bool state) {
  if (state)
    daikin[24] |= 0x04;
  else
    daikin[24] &= 0xFB;
  checksum();
}

bool IRDaikinESP::getEcono() {
  return daikin[24] & 0x04;
}

// starttime: Number of minutes after midnight, in 10 minutes increments
void IRDaikinESP::enableOnTimer(uint16_t starttime) {
    daikin[13] |= 0b00000010;
    uint16_t lopbits;
    lopbits = starttime;
    starttime &= 0xFF;
    daikin[18] = lopbits;
    starttime = starttime >> 8;
    // only keep 4 bits
    daikin[19] &= 0xF0;
    daikin[19] |= starttime;
}

void IRDaikinESP::disableOnTimer() {
    daikin[13] &= 0b11111101;
    enableOnTimer(0x600);
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



void IRDaikinESP::printState() {
  uint8_t var;
  Serial.print("Power: ");
  Serial.println(getPower() ? "On" : "Off");

  Serial.print("Temperature: ");
  Serial.println(getTemp());

  Serial.print("Quiet: ");
  Serial.println(getQuiet() ? "On" : "Off");

  Serial.print("Powerful: ");
  Serial.println(getPowerful() ? "On" : "Off");

  Serial.print("Econo: ");
  Serial.println(getEcono() ? "On" : "Off");

  Serial.print("Swing Vertical: ");
  Serial.println(getSwingVertical() ? "On" : "Off");

  Serial.print("Swing Horizontal: ");
  Serial.println(getSwingHorizontal() ? "On" : "Off");

  Serial.print("Fan Speed: ");
  var = getFan();
  if (var == DAIKIN_FAN_AUTO)
    Serial.println("Auto");
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
  Serial.print("countbits: ");
  Serial.println(countbits);
  Serial.println(results->rawlen-1);
  Serial.println(offset < results->rawlen-1);
  Serial.print("  offset: ");
  Serial.println(offset);
  for (uint16_t i = 0; i < countbits && offset < results->rawlen-1; i++, offset++) {
  Serial.print(i);
  Serial.print("  offset: ");
  Serial.println(offset);
  Serial.println(results->rawbuf[offset+1]);
  Serial.println(results->rawbuf[offset+2]);
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
  Serial.println("out......");
  return offset;
}

// TODO(crankyoldgit): NOT WORKING. This needs to be finished.
// Decode the supplied Daikin A/C message. (NOT WORKING - DO NOT USE)
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of bits to expect in the data portion. Typically SAMSUNG_BITS.
//   strict:  Flag to indicate if we strictly adhere to the specification.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: UNFINISHED / Completely not working, not even vaguely.
//
// Ref:
//   https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/tree/master/IRremote
bool IRrecv::decodeDaikin(decode_results *results, uint16_t nbits,
                          bool strict) {
  Serial.print("rawlen: ");
  Serial.println(results->rawlen);
  if (results->rawlen < 2 * nbits + HEADER + FOOTER)
    return false;

  Serial.print("nbits: ");
  Serial.println(nbits);
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

  Serial.println("GOT HERE 1");
  Serial.println(matchMark(results->rawbuf[OFFSET_START+11], DAIKIN_GAP));
  for (uint8_t i = 0; i < 10; i++) {
  Serial.print("Raw buf 1: ");
  Serial.println(results->rawbuf[offset]*RAWTICK);
    if (!matchMark(results->rawbuf[offset++], DAIKIN_ZERO_MARK))
      return false;
  }
  // Daikin GAP
  offset = checkheader(results, offset, daikin_code);
  if (offset == OFFSET_ERR)
      return false;

  Serial.println("GOT HERE 2");
  yield();
  // Data (#1)



  offset = readbits(results, offset, daikin_code, 8*8);
  if (offset == OFFSET_ERR)
      return false;

  // Ignore everything that has just been captured as it is not needed.
  // Some remotes may not send this portion, my remote did, but it's not required.
  for (uint8_t i = 0; i < DAIKIN_COMMAND_LENGTH+2; i++)
    daikin_code[i] = 0;

  Serial.println(data);
  Serial.println("GOT HERE 3");
  yield();

  uint32_t number = data;  // some number...
  uint32_t reversed = reverseBits(number, sizeof(number) * 8)

  DPRINT("Code ");
  DPRINTLN(reversed);

  offset = checkheader(results, offset, daikin_code);
  if (offset == OFFSET_ERR)
      return false;

  // Data (#2)
  offset = readbits(results, offset, daikin_code, 8*8);
  if (offset == OFFSET_ERR)
      return false;

  Serial.println("GOT HERE 4");
  yield();


  offset = checkheader(results, offset, daikin_code);
  if (offset == OFFSET_ERR)
      return false;

  Serial.println("GOT HERE 4.1");
  // Data (#3), read up everything else
  offset = readbits(results, offset, daikin_code, (DAIKIN_COMMAND_LENGTH*8)-(8*8));
  Serial.println(offset);
  if (offset == OFFSET_ERR)
      return false;


  Serial.println("GOT HERE 5");
  yield();




  // Print what we have
  Serial.println("BITS:::");
  for (uint8_t i = 0; i < DAIKIN_COMMAND_LENGTH; i++) {
      String strbits = String(daikin_code[i], BIN);
      while (strbits.length() < 8)
          strbits = String("0") + strbits;
      //Serial.print(i, DEC);
      //Serial.print(": ");
      //Serial.println(strbits);
      Serial.print(strbits);
      Serial.print(" ");
  }
  Serial.println("");
  for (uint8_t i = 0; i < DAIKIN_COMMAND_LENGTH; i++) {
      Serial.print(i);
      Serial.print(":");
      Serial.print(daikin_code[i], HEX);
      Serial.print(" ");
  }
  Serial.println("");





  number = data;  // some number...
  reversed = reverseBits(number, sizeof(number) * 8)

  DPRINT("Code2 ");
  DPRINTLN(reversed);

  // Success
  IRDaikinESP dako = IRDaikinESP(0);
  dako.setRaw(daikin_code);
  dako.printState();

  results->bits = DAIKIN_BITS;
  results->value = reversed;
  results->decode_type = DAIKIN;
  results->address = 0;
  results->command = 0;
  return true;
}
#endif  // DECODE_DAIKIN
