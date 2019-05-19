// Copyright 2017 stufisher

#include "ir_Trotec.h"
#include <algorithm>
#include "IRremoteESP8266.h"
#include "IRutils.h"

// Constants
const uint16_t kTrotecHdrMark = 5952;
const uint16_t kTrotecHdrSpace = 7364;
const uint16_t kTrotecOneMark = 592;
const uint16_t kTrotecOneSpace = 1560;
const uint16_t kTrotecZeroMark = 592;
const uint16_t kTrotecZeroSpace = 592;
const uint16_t kTrotecGap = 6184;
const uint16_t kTrotecGapEnd = 1500;  // made up value

#if SEND_TROTEC

void IRsend::sendTrotec(const unsigned char data[], const uint16_t nbytes,
                        const uint16_t repeat) {
  if (nbytes < kTrotecStateLength) return;

  for (uint16_t r = 0; r <= repeat; r++) {
    sendGeneric(kTrotecHdrMark, kTrotecHdrSpace, kTrotecOneMark,
                kTrotecOneSpace, kTrotecZeroMark, kTrotecZeroSpace,
                kTrotecOneMark, kTrotecGap, data, nbytes, 36, false,
                0,  // Repeats handled elsewhere
                50);
    // More footer
    enableIROut(36);
    mark(kTrotecOneMark);
    space(kTrotecGapEnd);
  }
}
#endif  // SEND_TROTEC

IRTrotecESP::IRTrotecESP(const uint16_t pin) : _irsend(pin) {
  this->stateReset();
}

void IRTrotecESP::begin(void) { _irsend.begin(); }

#if SEND_TROTEC
void IRTrotecESP::send(const uint16_t repeat) {
  this->checksum();
  _irsend.sendTrotec(remote_state, kTrotecStateLength, repeat);
}
#endif  // SEND_TROTEC

void IRTrotecESP::checksum(void) {
  uint8_t sum = 0;

  for (uint8_t i = 2; i < 8; i++) sum += remote_state[i];
  remote_state[8] = sum & 0xFF;
}

void IRTrotecESP::stateReset(void) {
  for (uint8_t i = 2; i < kTrotecStateLength; i++) remote_state[i] = 0x0;

  remote_state[0] = kTrotecIntro1;
  remote_state[1] = kTrotecIntro2;

  this->setPower(false);
  this->setTemp(kTrotecDefTemp);
  this->setSpeed(kTrotecFanMed);
  this->setMode(kTrotecAuto);
}

uint8_t* IRTrotecESP::getRaw(void) {
  this->checksum();
  return remote_state;
}

void IRTrotecESP::setPower(const bool on) {
  if (on)
    remote_state[2] |= kTrotecPowerBit;
  else
    remote_state[2] &= ~kTrotecPowerBit;
}

bool IRTrotecESP::getPower(void) { return remote_state[2] & kTrotecPowerBit; }

void IRTrotecESP::setSpeed(const uint8_t fan) {
  uint8_t speed = std::min(fan, kTrotecFanHigh);
  remote_state[2] = (remote_state[2] & 0b11001111) | (speed << 4);
}

uint8_t IRTrotecESP::getSpeed(void) {
  return (remote_state[2] & 0b00110000) >> 4;
}

void IRTrotecESP::setMode(const uint8_t mode) {
  switch (mode) {
    case kTrotecAuto:
    case kTrotecCool:
    case kTrotecDry:
    case kTrotecFan:
      remote_state[2] = (remote_state[2] & 0b11111100) | mode;
      return;
    default:
      this->setMode(kTrotecAuto);
  }
}

uint8_t IRTrotecESP::getMode(void) { return remote_state[2] & 0b00000011; }

void IRTrotecESP::setTemp(const uint8_t celsius) {
  uint8_t temp = std::max(celsius, kTrotecMinTemp);
  temp = std::min(temp, kTrotecMaxTemp);
  remote_state[3] = (remote_state[3] & 0x80) | (temp - kTrotecMinTemp);
}

uint8_t IRTrotecESP::getTemp(void) {
  return (remote_state[3] & 0b01111111) + kTrotecMinTemp;
}

void IRTrotecESP::setSleep(const bool on) {
  if (on)
    remote_state[3] |= kTrotecSleepBit;
  else
    remote_state[3] &= ~kTrotecSleepBit;
}

bool IRTrotecESP::getSleep(void) { return remote_state[3] & kTrotecSleepBit; }

void IRTrotecESP::setTimer(const uint8_t timer) {
  if (timer)
    remote_state[5] |= kTrotecTimerBit;
  else
    remote_state[5] &= ~kTrotecTimerBit;
  remote_state[6] = (timer > kTrotecMaxTimer) ? kTrotecMaxTimer : timer;
}

uint8_t IRTrotecESP::getTimer(void) { return remote_state[6]; }

// Convert a standard A/C mode into its native mode.
uint8_t IRTrotecESP::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kCool:
      return kTrotecCool;
    case stdAc::opmode_t::kDry:
      return kTrotecDry;
    case stdAc::opmode_t::kFan:
      return kTrotecFan;
    // Note: No Heat mode.
    default:
      return kTrotecAuto;
  }
}

// Convert a standard A/C Fan speed into its native fan speed.
uint8_t IRTrotecESP::convertFan(const stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin:
    case stdAc::fanspeed_t::kLow:
      return kTrotecFanLow;
    case stdAc::fanspeed_t::kMedium:
      return kTrotecFanMed;
    case stdAc::fanspeed_t::kHigh:
    case stdAc::fanspeed_t::kMax:
      return kTrotecFanHigh;
    default:
      return kTrotecFanMed;
  }
}


// Convert a native mode to it's common equivalent.
stdAc::opmode_t IRTrotecESP::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kTrotecCool: return stdAc::opmode_t::kCool;
    case kTrotecDry: return stdAc::opmode_t::kDry;
    case kTrotecFan: return stdAc::opmode_t::kFan;
    default: return stdAc::opmode_t::kAuto;
  }
}

// Convert a native fan speed to it's common equivalent.
stdAc::fanspeed_t IRTrotecESP::toCommonFanSpeed(const uint8_t spd) {
  switch (spd) {
    case kTrotecFanHigh: return stdAc::fanspeed_t::kMax;
    case kTrotecFanMed: return stdAc::fanspeed_t::kMedium;
    case kTrotecFanLow: return stdAc::fanspeed_t::kMin;
    default: return stdAc::fanspeed_t::kAuto;
  }
}

// Convert the A/C state to it's common equivalent.
stdAc::state_t IRTrotecESP::toCommon(void) {
  stdAc::state_t result;
  result.protocol = decode_type_t::TROTEC;
  result.power = this->getPower();
  result.mode = this->toCommonMode(this->getMode());
  result.celsius = true;
  result.degrees = this->getTemp();
  result.fanspeed = this->toCommonFanSpeed(this->getSpeed());
  result.sleep = this->getSleep() ? 0 : -1;
  // Not supported.
  result.model = -1;  // Not supported.
  result.swingv = stdAc::swingv_t::kOff;
  result.swingh = stdAc::swingh_t::kOff;
  result.turbo = false;
  result.light = false;
  result.filter = false;
  result.econo = false;
  result.quiet = false;
  result.clean = false;
  result.beep = false;
  result.clock = -1;
  return result;
}
