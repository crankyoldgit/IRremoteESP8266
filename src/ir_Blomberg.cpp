#include "ir_Blomberg.h"
#include "IRutils.h"

IRBlombergAc::IRBlombergAc(const uint16_t pin, const bool inverted,
                           const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) {
  stateReset();
}

void IRBlombergAc::stateReset() {
  const uint8_t default_state[kBlombergStateLength] = {
      86, 111, 0, 0, 33, 196, 0, 0, 0, 0, 0, 0, 0, 0, 0}; 
  std::memcpy(remote_state, default_state, kBlombergStateLength);
  checksum();
}

void IRBlombergAc::begin() { 
  _irsend.begin(); 
}

void IRBlombergAc::send(const uint16_t repeat) {
  checksum();
  _irsend.sendMirage(remote_state, kBlombergStateLength, repeat);
}

uint8_t* IRBlombergAc::getRaw() {
  checksum();
  return remote_state;
}

void IRBlombergAc::setRaw(const uint8_t state[]) {
  std::memcpy(remote_state, state, kBlombergStateLength);
}

uint8_t IRBlombergAc::calcChecksum(const uint8_t state[], const uint16_t length) {
  uint8_t sum = 0;
  for (uint16_t i = 0; i < length - 1; i++) {
    sum += (state[i] >> 4) & 0x0F;
    sum += state[i] & 0x0F;
  }
  return sum;
}

bool IRBlombergAc::validChecksum(const uint8_t state[], const uint16_t length) {
  return state[length - 1] == calcChecksum(state, length);
}

void IRBlombergAc::checksum() {
  remote_state[kBlombergStateLength - 1] = calcChecksum(remote_state);
}

void IRBlombergAc::setPower(const bool on) {
  if (on) {
    remote_state[5] &= ~0x80;
  } else {
    remote_state[5] |= 0x80;
  }
}

bool IRBlombergAc::getPower() const {
  return !(remote_state[5] & 0x80);
}

void IRBlombergAc::setMode(const uint8_t mode) {
  // Preserve the lower nibble (Fan) and update the upper nibble (Mode)
  remote_state[4] = (mode & 0xF0) | (remote_state[4] & 0x0F);

  // Set the specific Heat/Auto flag on Byte 6
  if (mode == kBlombergModeHeat || mode == kBlombergModeAuto) {
    remote_state[6] = 0x40; // 64
  } else {
    remote_state[6] = 0x00; // 0
  }
}

uint8_t IRBlombergAc::getMode() const {
  return remote_state[4] & 0xF0;
}

void IRBlombergAc::setTemp(const uint8_t temp) {
  uint8_t t = std::max(kBlombergMinTemp, std::min(temp, kBlombergMaxTemp));
  remote_state[1] = t + 92;
}

uint8_t IRBlombergAc::getTemp() const {
  return remote_state[1] - 92;
}

void IRBlombergAc::setFan(const uint8_t fan) {
  // Preserve the upper nibble (Mode) and update the lower nibble (Fan)
  remote_state[4] = (remote_state[4] & 0xF0) | (fan & 0x0F);
}

uint8_t IRBlombergAc::getFan() const {
  return remote_state[4] & 0x0F;
}

void IRBlombergAc::setSwing(const bool on) {
  if (on) {
    remote_state[5] |= 0x02;
  } else {
    remote_state[5] &= ~0x02;
  }
}

bool IRBlombergAc::getSwing() const {
  return remote_state[5] & 0x02;
}

void IRBlombergAc::setLed(const bool on) {
  if (on) {
    remote_state[5] &= ~0x04; 
  } else {
    remote_state[5] |= 0x04; 
  }
}

bool IRBlombergAc::getLed() const {
  return !(remote_state[5] & 0x04);
}

void IRBlombergAc::setTurbo(const bool on) {
  if (on) {
    remote_state[8] |= 0x80;
  } else {
    remote_state[8] &= ~0x80;
  }
}

bool IRBlombergAc::getTurbo() const {
  return remote_state[8] & 0x80;
}
