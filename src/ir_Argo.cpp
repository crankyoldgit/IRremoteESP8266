/*
Node MCU/ESP8266 Sketch to emulate Argo Ulisse 13 DCI remote
Controls Argo Ulisse 13 DCI A/C
Copyright 2017 Schmolders
Copyright 2019 crankyoldgit
*/

#include "ir_Argo.h"
#include <algorithm>
#include "IRremoteESP8266.h"
#include "IRutils.h"

// Constants
// using SPACE modulation. MARK is always const 400u
const uint16_t kArgoHdrMark = 6400;
const uint16_t kArgoHdrSpace = 3300;
const uint16_t kArgoBitMark = 400;
const uint16_t kArgoOneSpace = 2200;
const uint16_t kArgoZeroSpace = 900;

#if SEND_ARGO
// Send an Argo A/C message.
//
// Args:
//   data: An array of kArgoStateLength bytes containing the IR command.
//
// Status: ALPHA / Untested.

void IRsend::sendArgo(const unsigned char data[], const uint16_t nbytes,
                      const uint16_t repeat) {
  // Check if we have enough bytes to send a proper message.
  if (nbytes < kArgoStateLength) return;
  // TODO(kaschmo): validate
  sendGeneric(kArgoHdrMark, kArgoHdrSpace, kArgoBitMark, kArgoOneSpace,
              kArgoBitMark, kArgoZeroSpace, 0, 0,  // No Footer.
              data, nbytes, 38, false, repeat, kDutyDefault);
}
#endif  // SEND_ARGO

IRArgoAC::IRArgoAC(const uint16_t pin) : _irsend(pin) { this->stateReset(); }

void IRArgoAC::begin(void) { _irsend.begin(); }

#if SEND_ARGO
void IRArgoAC::send(const uint16_t repeat) {
  this->checksum();  // Create valid checksum before sending
  _irsend.sendArgo(argo, kArgoStateLength, repeat);
}
#endif  // SEND_ARGO

void IRArgoAC::checksum(void) {
  uint8_t sum = 2;  // Corresponds to byte 11 being constant 0b01

  // Only add up bytes to 9. byte 10 is 0b01 constant anyway.
  // Assume that argo array is MSB first (left)
  for (uint8_t i = 0; i < 10; i++) sum += argo[i];

  sum = sum % 256;  // modulo 256
  // Append sum to end of array
  // Set const part of checksum bit 10
  argo[10] = 0b00000010;
  argo[10] += sum << 2;  // Shift up 2 bits and append to byte 10
  argo[11] = sum >> 6;   // Shift down 6 bits and add in two LSBs of bit 11
}

void IRArgoAC::stateReset(void) {
  for (uint8_t i = 0; i < kArgoStateLength; i++) argo[i] = 0x0;

  // Argo Message. Store MSB left.
  // Default message:
  argo[0] = 0b10101100;  // LSB first (as sent) 0b00110101; //const preamble
  argo[1] = 0b11110101;  // LSB first: 0b10101111; //const preamble
  // Keep payload 2-9 at zero
  argo[10] = 0b00000010;  // Const 01, checksum 6bit
  argo[11] = 0b00000000;  // Checksum 2bit

  this->off();
  this->setTemp(20);
  this->setRoomTemp(25);
  this->setMode(kArgoAuto);
  this->setFan(kArgoFanAuto);
}

uint8_t* IRArgoAC::getRaw(void) {
  this->checksum();  // Ensure correct bit array before returning
  return argo;
}

void IRArgoAC::setRaw(const char state[]) {
  for (uint8_t i = 0; i < kArgoStateLength; i++) argo[i] = state[i];
}

void IRArgoAC::on(void) {
  // Bit 5 of byte 9 is on/off
  // in MSB first
  argo[9]|= kArgoPowerBit;  // Set ON/OFF bit to 1
}

void IRArgoAC::off(void) {
  // in MSB first
  // bit 5 of byte 9 to off
  argo[9] &= ~kArgoPowerBit;  // Set on/off bit to 0
}

void IRArgoAC::setPower(const bool on) {
  if (on)
    this->on();
  else
    this->off();
}

bool IRArgoAC::getPower(void) { return argo[9] & kArgoPowerBit; }

void IRArgoAC::setMax(const bool on) {
  if (on)
    argo[9] |= kArgoMaxBit;
  else
    argo[9] &= ~kArgoMaxBit;
}

bool IRArgoAC::getMax(void) { return argo[9] & kArgoMaxBit; }

// Set the temp in deg C
// Sending 0 equals +4
void IRArgoAC::setTemp(const uint8_t degrees) {
  uint8_t temp = std::max(kArgoMinTemp, degrees);
  temp = std::min(kArgoMaxTemp, temp);

  // offset 4 degrees. "If I want 12 degrees, I need to send 8"
  temp -= kArgoTempOffset;
  // Settemp = Bit 6,7 of byte 2, and bit 0-2 of byte 3
  // mask out bits
  // argo[13] & 0x00000100;  // mask out ON/OFF Bit
  argo[2] &= ~kArgoTempLowMask;
  argo[3] &= ~kArgoTempHighMask;

  // append to bit 6,7
  argo[2] += temp << 6;
  // remove lowest to bits and append in 0-2
  argo[3] += ((temp >> 2) & kArgoTempHighMask);
}

uint8_t IRArgoAC::getTemp(void) {
  return (((argo[3] & kArgoTempHighMask) << 2 ) | (argo[2] >> 6)) +
      kArgoTempOffset;
}

// Set the speed of the fan
void IRArgoAC::setFan(const uint8_t fan) {
  // Mask out bits
  argo[3] &= ~kArgoFanMask;
  // Set fan mode at bit positions
  argo[3] |= (std::min(fan, kArgoFan3) << 3);
}

uint8_t IRArgoAC::getFan(void) { return (argo[3] & kArgoFanMask) >> 3; }

void IRArgoAC::setFlap(const uint8_t flap) {
  flap_mode = flap;
  // TODO(kaschmo): set correct bits for flap mode
}

uint8_t IRArgoAC::getFlap(void) { return flap_mode; }

uint8_t IRArgoAC::getMode(void) {
  return (argo[2] & kArgoModeMask) >> 3;
}

void IRArgoAC::setMode(const uint8_t mode) {
  switch (mode) {
    case kArgoCool:
    case kArgoDry:
    case kArgoAuto:
    case kArgoOff:
    case kArgoHeat:
    case kArgoHeatAuto:
      // Mask out bits
      argo[2] &= ~kArgoModeMask;
      // Set the mode at bit positions
      argo[2] |= ((mode << 3) & kArgoModeMask);
      return;
    default:
      this->setMode(kArgoAuto);
  }
}

void IRArgoAC::setNight(const bool on) {
  if (on)
    // Set bit at night position: bit 2
    argo[9] |= kArgoNightBit;
  else
    argo[9] &= ~kArgoNightBit;
}

bool IRArgoAC::getNight(void) { return argo[9] & kArgoNightBit; }

void IRArgoAC::setiFeel(const bool on) {
  if (on)
    // Set bit at iFeel position: bit 7
    argo[9] |= kArgoIFeelBit;
  else
    argo[9] &= ~kArgoIFeelBit;
}

bool IRArgoAC::getiFeel(void) { return argo[9] & kArgoIFeelBit; }

void IRArgoAC::setTime(void) {
  // TODO(kaschmo): use function call from checksum to set time first
}

void IRArgoAC::setRoomTemp(const uint8_t degrees) {
  uint8_t temp = degrees - kArgoTempOffset;
  // Mask out bits
  argo[3] &= ~kArgoRoomTempLowMask;
  argo[4] &= ~kArgoRoomTempHighMask;

  argo[3] += temp << 5;  // Append to bit 5,6,7
  argo[4] += temp >> 3;  // Remove lowest 3 bits and append in 0,1
}

uint8_t IRArgoAC::getRoomTemp(void) {
  return ((argo[4] & kArgoRoomTempHighMask) << 3 | (argo[3] >> 5)) +
      kArgoTempOffset;
}

// Convert a standard A/C mode into its native mode.
uint8_t IRArgoAC::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kCool:
      return kArgoCool;
    case stdAc::opmode_t::kHeat:
      return kArgoHeat;
    case stdAc::opmode_t::kDry:
      return kArgoDry;
    case stdAc::opmode_t::kOff:
      return kArgoOff;
    // No fan mode.
    default:
      return kArgoAuto;
  }
}

// Convert a standard A/C Fan speed into its native fan speed.
uint8_t IRArgoAC::convertFan(const stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin:
    case stdAc::fanspeed_t::kLow:
      return kArgoFan1;
    case stdAc::fanspeed_t::kMedium:
      return kArgoFan2;
    case stdAc::fanspeed_t::kHigh:
    case stdAc::fanspeed_t::kMax:
      return kArgoFan3;
    default:
      return kArgoFanAuto;
  }
}

// Convert a standard A/C Fan speed into its native fan speed.
uint8_t IRArgoAC::convertSwingV(const stdAc::swingv_t position) {
  switch (position) {
    case stdAc::swingv_t::kHighest:
      return kArgoFlapFull;
    case stdAc::swingv_t::kHigh:
      return kArgoFlap5;
    case stdAc::swingv_t::kMiddle:
      return kArgoFlap4;
    case stdAc::swingv_t::kLow:
      return kArgoFlap3;
    case stdAc::swingv_t::kLowest:
      return kArgoFlap1;
    default:
      return kArgoFlapAuto;
  }
}

// Convert a native mode to it's common equivalent.
stdAc::opmode_t IRArgoAC::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kArgoCool: return stdAc::opmode_t::kCool;
    case kArgoHeat: return stdAc::opmode_t::kHeat;
    case kArgoDry: return stdAc::opmode_t::kDry;
    // No fan mode.
    default: return stdAc::opmode_t::kAuto;
  }
}

// Convert a native fan speed to it's common equivalent.
stdAc::fanspeed_t IRArgoAC::toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kArgoFan3: return stdAc::fanspeed_t::kMax;
    case kArgoFan2: return stdAc::fanspeed_t::kMedium;
    case kArgoFan1: return stdAc::fanspeed_t::kMin;
    default: return stdAc::fanspeed_t::kAuto;
  }
}

// Convert the A/C state to it's common equivalent.
stdAc::state_t IRArgoAC::toCommon(void) {
  stdAc::state_t result;
  result.protocol = decode_type_t::ARGO;
  result.power = this->getPower();
  result.mode = this->toCommonMode(this->getMode());
  result.celsius = true;
  result.degrees = this->getTemp();
  result.fanspeed = this->toCommonFanSpeed(this->getFan());
  result.turbo = this->getMax();
  result.sleep = this->getNight() ? 0 : -1;
  // Not supported.
  result.model = -1;  // Not supported.
  result.swingv = stdAc::swingv_t::kOff;
  result.swingh = stdAc::swingh_t::kOff;
  result.light = false;
  result.filter = false;
  result.econo = false;
  result.quiet = false;
  result.clean = false;
  result.beep = false;
  result.clock = -1;
  return result;
}
