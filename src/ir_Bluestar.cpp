// Copyright 2026 IRremoteESP8266 contributors
/// @file
/// @brief Blue Star IE318YKU A/C support.
/// @see docs/BlueStar.md for capture evidence.

#include "ir_Bluestar.h"
#include <algorithm>
#include <cstring>
#include "IRrecv.h"
#include "IRtext.h"
#include "IRutils.h"

// Timing medians from real receiver captures. The demodulator does not measure
// carrier frequency; 38 kHz is a provisional replay frequency.
const uint16_t kBluestarBitMark = 1000;
const uint16_t kBluestarZeroSpace = 580;
const uint16_t kBluestarOneSpace = 2600;
const uint16_t kBluestarSectionGap = 8950;
const uint16_t kBluestarCommandMark = 660;
const uint16_t kBluestarCommandZeroSpace = 3700;
const uint16_t kBluestarCommandOneSpace = 6500;
const uint16_t kBluestarFreq = 38000;

#if SEND_BLUESTAR
/// Send all 91 wire bits. Status: ALPHA / receiver captures verified.
/// @param[in] data Ten state bytes, then the right-aligned 11-bit command in
/// bytes 10 and 11, most significant command byte first. Checksum is supplied
/// by the caller; IRBluestarAc::getRaw() recomputes it automatically.
/// @param[in] nbytes Storage length; at least kBluestarStateLength.
/// @param[in] repeat Additional complete transmissions.
void IRsend::sendBluestar(const uint8_t data[], const uint16_t nbytes,
                         const uint16_t repeat) {
  if (data == nullptr || nbytes < kBluestarStateLength || data[10] > 7) return;
  const uint16_t command = (static_cast<uint16_t>(data[10]) << 8) | data[11];
  for (uint32_t r = 0; r <= repeat; r++) {
    sendGeneric(0, 0, kBluestarBitMark, kBluestarOneSpace,
                kBluestarBitMark, kBluestarZeroSpace,
                kBluestarBitMark, kBluestarSectionGap,
                data, kBluestarMainStateLength, kBluestarFreq, true, 0,
                kDutyDefault);
    sendGeneric(0, 0, kBluestarCommandMark, kBluestarCommandOneSpace,
                kBluestarCommandMark, kBluestarCommandZeroSpace,
                kBluestarCommandMark, kDefaultMessageGap,
                command, kBluestarCommandBits, kBluestarFreq, true, 0,
                kDutyDefault);
  }
}
#endif  // SEND_BLUESTAR

#if DECODE_BLUESTAR
/// Decode the main state AND secondary command; neither section is optional.
/// @note Strict mode checks the 0x33 signature and the main-state checksum.
bool IRrecv::decodeBluestar(decode_results *results, uint16_t offset,
                           const uint16_t nbits, const bool strict) {
  if (results == nullptr || nbits != kBluestarBits ||
      offset > results->rawlen || results->rawlen - offset <
      2 * kBluestarBits + 3) return false;
  const uint16_t used = matchGeneric(
      results->rawbuf + offset, results->state, results->rawlen - offset,
      kBluestarMainBits, 0, 0,
      kBluestarBitMark, kBluestarOneSpace, kBluestarBitMark, kBluestarZeroSpace,
      kBluestarBitMark, kBluestarSectionGap, false);
  if (!used) return false;
  offset += used;
  uint64_t command = 0;
  if (!matchGeneric(results->rawbuf + offset, &command,
                    results->rawlen - offset, kBluestarCommandBits, 0, 0,
                    kBluestarCommandMark, kBluestarCommandOneSpace,
                    kBluestarCommandMark, kBluestarCommandZeroSpace,
                    kBluestarCommandMark, 0, false)) return false;
  if (strict && (results->state[0] != 0x33 ||
                 !IRBluestarAc::validChecksum(results->state,
                                              kBluestarMainStateLength)))
    return false;
  results->state[10] = command >> 8;
  results->state[11] = command & 0xFF;
  results->decode_type = BLUESTAR;
  results->bits = kBluestarBits;
  results->repeat = false;
  return true;
}
#endif  // DECODE_BLUESTAR

IRBluestarAc::IRBluestarAc(const uint16_t pin, const bool inverted,
                         const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) {
  stateReset();
}

void IRBluestarAc::begin() { _irsend.begin(); }

/// Reset to a captured Power Off, Cool 24 C, Low fan state. Time bytes remain
/// opaque: this is a known frame, not an invented clock/timer encoding.
void IRBluestarAc::stateReset() {
  const uint8_t reset[kBluestarStateLength] = {
      0x33, 0x88, 0x02, 0x18, 0x12, 0x92, 0x2A, 0xA4, 0x05, 0xB3, 0x05, 0xCD};
  setRaw(reset);
}

#if SEND_BLUESTAR
void IRBluestarAc::send(const uint16_t repeat) {
  _irsend.sendBluestar(getRaw(), kBluestarStateLength, repeat);
}
#endif  // SEND_BLUESTAR

uint8_t IRBluestarAc::calcChecksum(const uint8_t state[],
                                  const uint16_t length) {
  if (state == nullptr || length < kBluestarMainStateLength) return 0;
  return ~sumBytes(state, kBluestarMainStateLength - 1);
}

bool IRBluestarAc::validChecksum(const uint8_t state[], const uint16_t length) {
  return state != nullptr && length >= kBluestarMainStateLength &&
      state[kBluestarMainStateLength - 1] == calcChecksum(state, length);
}

uint8_t *IRBluestarAc::getRaw() {
  _state[9] = calcChecksum(_state);
  return _state;
}

/// Import a complete 12-byte state, or an explicitly sized 10-byte main state.
/// Main-only imports retain the previous command. Unknown/time bits are copied.
bool IRBluestarAc::setRaw(const uint8_t state[], const uint16_t length) {
  if (state == nullptr || length < kBluestarMainStateLength || length == 11 ||
      (length >= kBluestarStateLength && state[10] > 7)) return false;
  std::memcpy(_state, state, std::min(length, kBluestarStateLength));
  _normalFan = getFan() == kBluestarFanTurbo ? kBluestarFanHigh : getFan();
  return true;
}

void IRBluestarAc::setField(const uint8_t byte, const uint8_t mask,
                           const uint8_t value) {
  _state[byte] = (_state[byte] & ~mask) | (value & mask);
}

uint16_t IRBluestarAc::getCommand() const {
  return (static_cast<uint16_t>(_state[10]) << 8) | _state[11];
}

void IRBluestarAc::setCommand(const uint16_t command) {
  if (command >= (1U << kBluestarCommandBits)) return;
  _state[10] = command >> 8;
  _state[11] = command & 0xFF;
}

void IRBluestarAc::on() { setPower(true); }
void IRBluestarAc::off() { setPower(false); }
bool IRBluestarAc::getPower() const { return _state[2] & 0x80; }

/// Mirror the observed Power-button lower bound. setTemp() can subsequently
/// select 16-23 C while On, as the captured ordinary state packets demonstrate.
void IRBluestarAc::setPower(const bool on) {
  setField(2, 0x80, on ? 0x80 : 0);
  if (getTemp() < 24) setField(3, 0x1F, 24);
  setCommand(kBluestarCommandPower);
}

uint8_t IRBluestarAc::getMode() const { return _state[1] & 0x0F; }

void IRBluestarAc::setMode(const uint8_t mode) {
  uint8_t value = mode;
  switch (value) {
    case kBluestarAuto:
    case kBluestarCool:
    case kBluestarDry:
    case kBluestarFanOnly: break;
    default: value = kBluestarCool;
  }
  setField(1, 0x0F, value);
  if (value == kBluestarFanOnly) setField(3, 0x1F, kBluestarFanModeTemp);
  setCommand(kBluestarCommandMode);
}

uint8_t IRBluestarAc::getTemp() const { return _state[3] & 0x1F; }
uint8_t IRBluestarAc::getEffectiveTemp() const {
  return getTurbo() ? kBluestarMinTemp : getTemp();
}

void IRBluestarAc::setTemp(const uint8_t celsius) {
  // No temperature control on the physical remote in Fan-only mode.
  if (getMode() == kBluestarFanOnly) return;
  setField(3, 0x1F, std::max(kBluestarMinTemp,
                            std::min(kBluestarMaxTemp, celsius)));
  setCommand(kBluestarCommandTemp);
}

uint8_t IRBluestarAc::getFan() const { return (_state[1] >> 5) & 7; }
uint8_t IRBluestarAc::getEffectiveFan() const {
  return getTurbo() || getFan() == kBluestarFanTurbo ?
      kBluestarFanHigh : getFan();
}

void IRBluestarAc::setFan(const uint8_t speed) {
  uint8_t value = speed;
  switch (value) {
    case kBluestarFanLow:
    case kBluestarFanMed:
    case kBluestarFanHigh:
    case kBluestarFanAuto: break;
    default: value = kBluestarFanAuto;
  }
  _normalFan = value;
  setField(1, 0xE0, value << 5);
  setCommand(kBluestarCommandFan);
}

bool IRBluestarAc::getSwingH() const { return !(_state[2] & 0x01); }
bool IRBluestarAc::getSwingV() const { return !(_state[2] & 0x02); }
bool IRBluestarAc::getLight() const { return !(_state[2] & 0x08); }
bool IRBluestarAc::getSleep() const { return _state[2] & 0x40; }
bool IRBluestarAc::getTurbo() const { return _state[2] & 0x10; }
bool IRBluestarAc::getEcono() const { return _state[3] & 0x20; }

void IRBluestarAc::setSwingH(const bool on) {
  setField(2, 0x01, on ? 0 : 0x01);
  setCommand(kBluestarCommandSwing);
}
void IRBluestarAc::setSwingV(const bool on) {
  setField(2, 0x02, on ? 0 : 0x02);
  setCommand(kBluestarCommandSwing);
}
void IRBluestarAc::setLight(const bool on) {
  setField(2, 0x08, on ? 0 : 0x08);
  setCommand(kBluestarCommandLight);
}
void IRBluestarAc::setSleep(const bool on) {
  setField(2, 0x40, on ? 0x40 : 0);
  setCommand(kBluestarCommandSleep);
}
void IRBluestarAc::setTurbo(const bool on) {
  setField(2, 0x10, on ? 0x10 : 0);
  setCommand(kBluestarCommandTurbo);
}
void IRBluestarAc::setEcono(const bool on) {
  setField(3, 0x20, on ? 0x20 : 0);
  if (on) {
    setField(3, 0x1F, 24);
    setFan(kBluestarFanLow);
  }
  setCommand(kBluestarCommandEcono);
}

uint8_t IRBluestarAc::getCapacity() const { return _state[8] & 7; }

/// Select capacity and the observed fan code. Default restores a fan selected
/// locally before the override. Raw imports only reveal the currently encoded
/// fan; earlier remote fan history cannot be recovered from an active capture.
void IRBluestarAc::setCapacity(const uint8_t capacity) {
  const uint8_t value = capacity >= kBluestarCapacityTurbo &&
      capacity <= kBluestarCapacityDefault ?
      capacity : kBluestarCapacityDefault;
  if (getCapacity() == kBluestarCapacityDefault) _normalFan = getFan();
  setField(8, 7, value);
  uint8_t fan = _normalFan;
  switch (value) {
    case kBluestarCapacityTurbo: fan = kBluestarFanTurbo; break;
    case kBluestarCapacity80: fan = kBluestarFanHigh; break;
    case kBluestarCapacity60: fan = kBluestarFanMed; break;
    case kBluestarCapacity40: fan = kBluestarFanLow; break;
    default: break;
  }
  setField(1, 0xE0, fan << 5);
  setCommand(kBluestarCommandCapacity);
}

uint8_t IRBluestarAc::getClockMinute() const { return _state[6]; }

uint8_t IRBluestarAc::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kAuto: return kBluestarAuto;
    case stdAc::opmode_t::kDry: return kBluestarDry;
    case stdAc::opmode_t::kFan: return kBluestarFanOnly;
    default: return kBluestarCool;
  }
}
uint8_t IRBluestarAc::convertFan(const stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin:
    case stdAc::fanspeed_t::kLow: return kBluestarFanLow;
    case stdAc::fanspeed_t::kMedium: return kBluestarFanMed;
    case stdAc::fanspeed_t::kMediumHigh:
    case stdAc::fanspeed_t::kHigh:
    case stdAc::fanspeed_t::kMax: return kBluestarFanHigh;
    default: return kBluestarFanAuto;
  }
}
stdAc::opmode_t IRBluestarAc::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kBluestarAuto: return stdAc::opmode_t::kAuto;
    case kBluestarDry: return stdAc::opmode_t::kDry;
    case kBluestarFanOnly: return stdAc::opmode_t::kFan;
    default: return stdAc::opmode_t::kCool;
  }
}
stdAc::fanspeed_t IRBluestarAc::toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kBluestarFanLow: return stdAc::fanspeed_t::kLow;
    case kBluestarFanMed: return stdAc::fanspeed_t::kMedium;
    case kBluestarFanTurbo:
    case kBluestarFanHigh: return stdAc::fanspeed_t::kHigh;
    default: return stdAc::fanspeed_t::kAuto;
  }
}

stdAc::state_t IRBluestarAc::toCommon() const {
  stdAc::state_t result{};
  result.protocol = BLUESTAR;
  result.power = getPower();
  result.mode = toCommonMode(getMode());
  result.celsius = true;
  result.degrees = getEffectiveTemp();
  result.fanspeed = toCommonFanSpeed(getEffectiveFan());
  result.swingh = getSwingH() ? stdAc::swingh_t::kAuto : stdAc::swingh_t::kOff;
  result.swingv = getSwingV() ? stdAc::swingv_t::kAuto : stdAc::swingv_t::kOff;
  // Common API has one Turbo bit; the native class retains the distinction.
  result.turbo = getTurbo() || getCapacity() == kBluestarCapacityTurbo;
  result.econo = getEcono();
  result.light = getLight();
  result.sleep = getSleep() ? 0 : -1;
  result.clock = -1;  // Full clock/hour encoding is unresolved.
  return result;
}

String IRBluestarAc::toString() const {
  using irutils::addBoolToString;
  using irutils::addIntToString;
  String result;
  result.reserve(220);
  result += addBoolToString(getPower(), kPowerStr, false);
  result += irutils::addModeToString(getMode(), kBluestarAuto, kBluestarCool,
                                    0, kBluestarDry, kBluestarFanOnly);
  result += irutils::addTempToString(getTemp());
  result += irutils::addFanToString(getFan(),
                                   getFan() == kBluestarFanTurbo ?
                                       kBluestarFanTurbo : kBluestarFanHigh,
                                   kBluestarFanLow,
                                   kBluestarFanAuto, kBluestarFanHigh,
                                   kBluestarFanMed);
  result += addBoolToString(getSwingH(), kSwingHStr);
  result += addBoolToString(getSwingV(), kSwingVStr);
  result += addBoolToString(getLight(), kLightStr);
  result += addBoolToString(getTurbo(), kTurboStr);
  result += addBoolToString(getEcono(), kEconoStr);
  result += addBoolToString(getSleep(), kSleepStr);
  const uint8_t percentages[] = {0, 110, 80, 60, 40, 100};
  const uint8_t capacity = getCapacity();
  result += addIntToString(capacity <= kBluestarCapacityDefault ?
                           percentages[capacity] : 0, "Capacity") + "%";
  result += addIntToString(getClockMinute(), "Clock Minute");
  result += addIntToString(getCommand(), kCommandStr);
  return result;
}
