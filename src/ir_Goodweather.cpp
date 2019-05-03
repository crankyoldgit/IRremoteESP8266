// Copyright 2019 ribeirodanielf
// Copyright 2019 David Conran
//
// Code to emulate Goodweather protocol compatible HVAC devices.
// Should be compatible with:
// * ZH/JT-03 remote control
//

#include "ir_Goodweather.h"
#include <algorithm>
#ifndef ARDUINO
#include <string>
#endif
#include "IRrecv.h"
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "IRutils.h"

#if SEND_GOODWEATHER
// Send a Goodweather message.
//
// Args:
//   data: The raw message to be sent.
//   nbits: Nr. of bits of data in the message. (Default is kGoodweatherBits)
//   repeat: Nr. of times the message is to be repeated. (Default = 1).
//
// Status: ALPHA / Untested.
//
// Ref:
//   https://github.com/markszabo/IRremoteESP8266/issues/697
void IRsend::sendGoodweather(const uint64_t data, const uint16_t nbits,
                             const uint16_t repeat) {
  if (nbits != kGoodweatherBits)
    return;  // Wrong nr. of bits to send a proper message.
  // Set IR carrier frequency
  enableIROut(38);

  for (uint16_t r = 0; r <= repeat; r++) {
    // Header
    mark(kGoodweatherHdrMark);
    space(kGoodweatherHdrSpace);

    // Data
    for (int16_t i = 0; i < nbits; i += 8) {
      uint16_t chunk = (data >> i) & 0xFF;  // Grab a byte at a time.
      chunk = (~chunk) << 8 | chunk;  // Prepend a inverted copy of the byte.
      sendData(kGoodweatherBitMark, kGoodweatherOneSpace,
               kGoodweatherBitMark, kGoodweatherZeroSpace,
               chunk, 16, false);
    }
    // Footer
    mark(kGoodweatherBitMark);
    space(kGoodweatherHdrSpace);
  }
}
#endif  // SEND_GOODWEATHER

IRGoodweatherAc::IRGoodweatherAc(uint16_t pin) : _irsend(pin) { stateReset(); }

void IRGoodweatherAc::stateReset(void) {
}

void IRGoodweatherAc::begin(void) { _irsend.begin(); }

#if SEND_GOODWEATHER
void IRGoodweatherAc::send(const uint16_t repeat) {
  _irsend.sendGoodweather(remote, kGoodweatherBits, repeat);
}
#endif  // SEND_GOODWEATHER

uint64_t IRGoodweatherAc::getRaw(void) { return remote; }

void IRGoodweatherAc::setRaw(const uint64_t state) { remote = state; }

void IRGoodweatherAc::on(void) { remote |= kGoodweatherBitPower; }

void IRGoodweatherAc::off(void) { remote &= ~kGoodweatherBitPower; }

void IRGoodweatherAc::setPower(const bool on) {
  if (on)
    this->on();
  else
    this->off();
}

bool IRGoodweatherAc::getPower(void) { return remote && kGoodweatherBitPower; }

// Set the temp. in deg C
void IRGoodweatherAc::setTemp(const uint8_t temp) {
  uint8_t new_temp = std::max((uint8_t)kGoodweatherTempMin, temp);
  new_temp = std::min((uint8_t)kGoodweatherTempMax, new_temp);
  remote |= (uint64_t)(new_temp - kGoodweatherTempMin) << kGoodweatherBitTemp;
}

// Return the set temp. in deg C
uint8_t IRGoodweatherAc::getTemp(void) {
  return ((remote & kGoodweatherTempMask) >> kGoodweatherBitTemp) +
      kGoodweatherTempMin;
}

// Set the speed of the fan
void IRGoodweatherAc::setFan(const uint8_t speed) {
  switch (speed) {
    case kGoodweatherFanAuto:
    case kGoodweatherFanLow:
    case kGoodweatherFanMed:
    case kGoodweatherFanHigh:
      remote &= ~kGoodweatherFanMask;
      remote |= ((uint64_t)speed << kGoodweatherBitFan);
      break;
    default:
      this->setFan(kGoodweatherFanAuto);
  }
}

uint8_t IRGoodweatherAc::getFan() {
  return (remote & kGoodweatherFanMask) >> kGoodweatherBitFan;
}

void IRGoodweatherAc::setMode(const uint8_t mode) {
  switch (mode) {
    case kGoodweatherAuto:
    case kGoodweatherDry:
    case kGoodweatherCool:
    case kGoodweatherFan:
    case kGoodweatherHeat:
      remote &= ~kGoodweatherModeMask;
      remote |= (uint64_t)mode << kGoodweatherBitMode;
      break;
    default:
      // If we get an unexpected mode, default to AUTO.
      this->setMode(kGoodweatherAuto);
  }
}

uint8_t IRGoodweatherAc::getMode() {
  return (remote & kGoodweatherModeMask) >> kGoodweatherBitMode;
}

void IRGoodweatherAc::setLight(const bool toggle) {
  if (toggle)
    remote |= kGoodweatherLightMask;
  else
    remote &= ~kGoodweatherLightMask;
}

bool IRGoodweatherAc::getLight() { return remote & kGoodweatherLightMask; }

void IRGoodweatherAc::setSleep(const bool toggle) {
  if (toggle)
    remote |= kGoodweatherSleepMask;
  else
    remote &= ~kGoodweatherSleepMask;
}

bool IRGoodweatherAc::getSleep() { return remote & kGoodweatherSleepMask; }

void IRGoodweatherAc::setTurbo(const bool toggle) {
  if (toggle)
    remote |= kGoodweatherTurboMask;
  else
    remote &= ~kGoodweatherTurboMask;
}

bool IRGoodweatherAc::getTurbo() { return remote & kGoodweatherTurboMask; }

void IRGoodweatherAc::setSwing(const uint8_t speed) {
  switch (speed) {
    case kGoodweatherSwingOff:
    case kGoodweatherSwingSlow:
    case kGoodweatherSwingFast:
      remote &= ~kGoodweatherSwingMask;
      remote |= ((uint64_t)speed << kGoodweatherBitSwing);
      break;
    default:
      this->setSwing(kGoodweatherSwingOff);
  }
}

uint8_t IRGoodweatherAc::getSwing() {
  return (remote & kGoodweatherSwingMask) >> kGoodweatherBitSwing;
}

// Convert a standard A/C mode into its native mode.
uint8_t IRGoodweatherAc::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kCool:
      return kGoodweatherCool;
    case stdAc::opmode_t::kHeat:
      return kGoodweatherHeat;
    case stdAc::opmode_t::kDry:
      return kGoodweatherDry;
    case stdAc::opmode_t::kFan:
      return kGoodweatherFan;
    default:
      return kGoodweatherAuto;
  }
}

// Convert a standard A/C Fan speed into its native fan speed.
uint8_t IRGoodweatherAc::convertFan(const stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin:
    case stdAc::fanspeed_t::kLow:
      return kGoodweatherFanLow;
    case stdAc::fanspeed_t::kMedium:
      return kGoodweatherFanMed;
    case stdAc::fanspeed_t::kHigh:
    case stdAc::fanspeed_t::kMax:
      return kGoodweatherFanHigh;
    default:
      return kGoodweatherFanAuto;
  }
}

// Convert a standard A/C Vertical Swing into its native version.
uint8_t IRGoodweatherAc::convertSwingV(const stdAc::swingv_t swingv) {
  switch (swingv) {
    case stdAc::swingv_t::kHighest:
    case stdAc::swingv_t::kHigh:
    case stdAc::swingv_t::kMiddle:
      return kGoodweatherSwingFast;
    case stdAc::swingv_t::kLow:
    case stdAc::swingv_t::kLowest:
    case stdAc::swingv_t::kAuto:
      return kGoodweatherSwingSlow;
    default:
      return kGoodweatherSwingOff;
  }
}

// Convert the internal state into a human readable string.
#ifdef ARDUINO
String IRGoodweatherAc::toString() {
  String result = "";
#else
std::string IRGoodweatherAc::toString() {
  std::string result = "";
#endif  // ARDUINO
  result.reserve(150);  // Reserve some heap for the string to reduce fragging.
  result += F("Power: ");
  if (this->getPower())
    result += F("On");
  else
    return result += F("Off");  // If it's off, there is no other info.
  result += F(", Mode: ");
  result += uint64ToString(this->getMode());
  switch (this->getMode()) {
    case kGoodweatherAuto:
      result += F(" (AUTO)");
      break;
    case kGoodweatherCool:
      result += F(" (COOL)");
      break;
    case kGoodweatherHeat:
      result += F(" (HEAT)");
      break;
    case kGoodweatherDry:
      result += F(" (DRY)");
      break;
    case kGoodweatherFan:
      result += F(" (FAN)");
      break;
    default:
      result += F(" (UNKNOWN)");
  }
  result += F(", Temp: ");
  result += uint64ToString(this->getTemp());
  result += F("C, Fan: ");
  result += uint64ToString(this->getFan());
  switch (this->getFan()) {
    case kGoodweatherFanAuto:
      result += F(" (AUTO)");
      break;
    case kGoodweatherFanHigh:
      result += F(" (HIGH)");
      break;
    case kGoodweatherFanMed:
      result += F(" (MED)");
      break;
    case kGoodweatherFanLow:
      result += F(" (LOW)");
      break;
  }
  result += F(", Turbo: ");
  if (this->getTurbo())
    result += F("Toggle");
  else
    result += F("-");
  result += F(", Light: ");
  if (this->getLight())
    result += F("Toggle");
  else
    result += F("-");
  result += F(", Sleep: ");
  if (this->getSleep())
    result += F("Toggle");
  else
    result += F("-");
  result += F(", Swing: ");
  result += uint64ToString(this->getSwing());
  switch (this->getSwing()) {
    case kGoodweatherSwingFast:
      result += F(" (Fast)");
      break;
    case kGoodweatherSwingSlow:
      result += F(" (Slow)");
      break;
    case kGoodweatherSwingOff:
      result += F(" (Off)");
      break;
    default:
      result += F(" (UNKNOWN)");
  }
  return result;
}

#if DECODE_GOODWEATHER
// Decode the supplied Goodweather message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect. Typically kGoodweatherBits.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: ALPHA / Untested.
bool IRrecv::decodeGoodweather(decode_results* results,
                               const uint16_t nbits,
                               const bool strict) {
  if (results->rawlen < 2 * (2 * nbits + kHeader + kFooter) - 1)
    return false;  // Can't possibly be a valid Goodweather message.
  if (strict && nbits != kGoodweatherBits)
    return false;  // Not strictly a Goodweather message.

  uint64_t dataSoFar = 0;
  uint16_t dataBitsSoFar = 0;
  uint16_t offset = kStartOffset;
  match_result_t data_result;

  // Header
  if (!matchMark(results->rawbuf[offset++], kGoodweatherHdrMark)) return false;
  if (!matchSpace(results->rawbuf[offset++], kGoodweatherHdrSpace))
    return false;

  // Data
  for (; offset <= results->rawlen - 32 && dataBitsSoFar < nbits;
       dataBitsSoFar += 8) {
    DPRINT("DEBUG: Attempting Byte #");
    DPRINTLN(dataBitsSoFar / 8);
    // Read in a byte at a time.
    // Normal first.
    data_result = matchData(&(results->rawbuf[offset]), 8,
                            kGoodweatherBitMark, kGoodweatherOneSpace,
                            kGoodweatherBitMark, kGoodweatherZeroSpace,
                            kTolerance, kMarkExcess, false);
    if (data_result.success == false) return false;
    DPRINTLN("DEBUG: Normal byte read okay.");
    offset += data_result.used;
    uint8_t data = (uint8_t)data_result.data;
    // Then inverted.
    data_result = matchData(&(results->rawbuf[offset]), 8,
                            kGoodweatherBitMark, kGoodweatherOneSpace,
                            kGoodweatherBitMark, kGoodweatherZeroSpace,
                            kTolerance, kMarkExcess, false);
    if (data_result.success == false) return false;
    DPRINTLN("DEBUG: Inverted byte read okay.");
    offset += data_result.used;
    uint8_t inverted = (uint8_t)data_result.data;
    DPRINT("DEBUG: data = ");
    DPRINTLN((uint16_t)data);
    DPRINT("DEBUG: inverted = ");
    DPRINTLN((uint16_t)inverted);
    if (data != (inverted ^ 0xFF)) return false;  // Data integrity failed.
    dataSoFar |= (uint64_t)data << dataBitsSoFar;
  }

  // Footer.
  if (!matchMark(results->rawbuf[offset++], kGoodweatherBitMark)) return false;
  if (offset <= results->rawlen &&
      !matchAtLeast(results->rawbuf[offset], kGoodweatherHdrSpace))
    return false;

  // Compliance
  if (strict && (dataBitsSoFar != kGoodweatherBits)) return false;

  // Success
  results->decode_type = decode_type_t::GOODWEATHER;
  results->bits = dataBitsSoFar;
  results->value = dataSoFar;
  results->address = 0;
  results->command = 0;
  return true;
}
#endif  // DECODE_GOODWEATHER
