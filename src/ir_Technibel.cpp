// Technibel based protocol.

#include "ir_Technibel.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "IRtext.h"
#include "IRutils.h"
#include <algorithm>

using irutils::addBoolToString;
using irutils::addModeToString;
using irutils::addFanToString;
using irutils::addLabeledString;
using irutils::addTempToString;
using irutils::minsToString;
using irutils::setBit;
using irutils::setBits;

const uint16_t kTechnibelAcHdrMark = 8836;;
const uint16_t kTechnibelAcHdrSpace = 4380;;
const uint16_t kTechnibelAcBitMark = 523;;
const uint16_t kTechnibelAcOneSpace = 1696;;
const uint16_t kTechnibelAcZeroSpace = 564;;
const uint32_t kTechnibelAcGap = kDefaultMessageGap;
const uint16_t kTechnibelAcFreq = 38000;
const uint16_t kTechnibelAcOverhead = 3;


#if SEND_TECHNIBEL_AC
// Send an Technibel AC formatted message.
//
// Args:
//   data:   The message to be sent.
//   nbits:  The number of bits of the message to be sent.
//           Typically kTechnibelAcBits.
//   repeat: The number of times the command is to be repeated.
//
// Status: STABLE / Reported as working on a real device.
void IRsend::sendTechnibelAc(const uint64_t data, const uint16_t nbits,
                            const uint16_t repeat) {
  sendGeneric(kTechnibelAcHdrMark, kTechnibelAcHdrSpace,
              kTechnibelAcBitMark, kTechnibelAcOneSpace,
              kTechnibelAcBitMark, kTechnibelAcZeroSpace,
              kTechnibelAcBitMark, kTechnibelAcGap,
              data, nbits, kTechnibelAcFreq, false,  // LSB First.
              repeat, kDutyDefault);
}
#endif  // SEND_TECHNIBEL_AC

#if DECODE_TECHNIBEL_AC
// Decode the supplied TECHNIBEL_AC message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   offset:  The starting index to use when attempting to decode the raw data.
//            Typically/Defaults to kStartOffset.
//   nbits:   The number of data bits to expect. Typically kTechnibelAcBits.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: STABLE / Expected to be working.
bool IRrecv::decodeTechnibelAc(decode_results *results, uint16_t offset,
                              const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * nbits + kTechnibelAcOverhead - offset)
  {
    return false;  // Too short a message to match.
  }
  if (strict && nbits != kTechnibelAcBits)
  {
    return false;
  }

  uint64_t data = 0;

  // Header + Data + Footer
  if (!matchGeneric(results->rawbuf + offset, &data,
                    results->rawlen - offset, nbits,
                    kTechnibelAcHdrMark, kTechnibelAcHdrSpace,
                    kTechnibelAcBitMark, kTechnibelAcOneSpace,
                    kTechnibelAcBitMark, kTechnibelAcZeroSpace,
                    kTechnibelAcBitMark, kTechnibelAcGap, true,
                    _tolerance, kMarkExcess, false)) return false;

  // Success
  results->decode_type = decode_type_t::TECHNIBEL_AC;
  results->bits = nbits;
  results->value = data;
  results->command = 0;
  results->address = 0;
  return true;
}
#endif  // DECODE_TECHNIBEL_AC


// Class for controlling the settings of a Technibel A/C

IRTechnibelAc::IRTechnibelAc(const uint16_t pin, const bool inverted,
                           const bool use_modulation)
      : _irsend(pin, inverted, use_modulation) { this->stateReset(); }

void IRTechnibelAc::begin(void) { _irsend.begin(); }

#if SEND_TECHNIBEL_AC
void IRTechnibelAc::send(const uint16_t repeat) {
  _irsend.sendTechnibelAc(getRaw(), kTechnibelAcBits, repeat);
}
#endif  // SEND_TECHNIBEL_AC

uint8_t IRTechnibelAc::calcChecksum(const uint64_t state) {
  uint8_t sum = 0;
  // Add up all the 8 bit data chunks.
  for (uint8_t offset = kTechnibelAcPowerBit; offset < kTechnibelAcFooterOffset; offset += 8) {
    sum += reverseBits(GETBITS64(state, offset, 8),8);
  }
  sum -= 1;
  sum = invertBits(sum,8);
  sum = reverseBits(sum,8);

  return sum;
}

bool IRTechnibelAc::validChecksum(const uint64_t state) {
  return (GETBITS64(state, kTechnibelAcChecksumOffset,
                    kTechnibelAcChecksumSize) ==
      IRTechnibelAc::calcChecksum(state));
}

void IRTechnibelAc::checksum(void) {
  setBits(&remote_state, kTechnibelAcChecksumOffset, kTechnibelAcChecksumSize,
          calcChecksum(remote_state));
}

void IRTechnibelAc::stateReset(void) {
  remote_state = 0x00000000000018;
  _saved_temp = 20;  // DegC  (Random reasonable default value)
  _saved_temp_units = 0;  // Celsius

  this->off();
  this->setTemp(_saved_temp);
  this->setTempUnit(_saved_temp_units);
  this->setMode(kTechnibelAcCool);
  this->setFan(kTechnibelAcFanLow);
}

uint64_t IRTechnibelAc::getRaw(void) {
  this->checksum();
  return remote_state;
}

void IRTechnibelAc::setRaw(const uint64_t state) {
  remote_state = state;
}

void IRTechnibelAc::on(void) {
  setPower(true);
}

void IRTechnibelAc::off(void) {
  setPower(false);
}

void IRTechnibelAc::setPower(const bool on) {
  setBit(&remote_state, kTechnibelAcPowerBit, on);
}

bool IRTechnibelAc::getPower(void) {
  return GETBIT64(remote_state, kTechnibelAcPowerBit);
}

void IRTechnibelAc::setTempUnit(const bool fahrenheit) {
  setBit(&remote_state, kTechnibelAcTempUnitBit, fahrenheit);
}

bool IRTechnibelAc::getTempUnit(void) {
  return GETBIT64(remote_state, kTechnibelAcTempUnitBit);
}

// Set the temp
void IRTechnibelAc::setTemp(const uint8_t degrees, const bool fahrenheit) {
  uint8_t temp;
  uint8_t temp_min = kTechnibelAcTempMinC;
  uint8_t temp_max = kTechnibelAcTempMaxC;
  setTempUnit(fahrenheit);
  if (fahrenheit) {
    temp_min = kTechnibelAcTempMinF;
    temp_max = kTechnibelAcTempMaxF;
  }
  temp = std::max(temp_min, degrees);
  temp = std::min(temp_max, temp);
  _saved_temp = temp;
  _saved_temp_units = fahrenheit;

  uint8_t temp_reversed = reverseBits(temp,kTechnibelAcTempSize);
  setBits(&remote_state, kTechnibelAcTempOffset, kTechnibelAcTempSize,
          temp_reversed);
}

uint8_t IRTechnibelAc::getTemp(void) {
  uint8_t temp = GETBITS64(remote_state, kTechnibelAcTempOffset, kTechnibelAcTempSize);
  return reverseBits(temp, kTechnibelAcTempSize);
}

// Set the speed of the fan
void IRTechnibelAc::setFan(const uint8_t speed) {
  // Mode fan speed rules.
  if(getMode() == kTechnibelAcDry && speed != kTechnibelAcFanLow) {
    setFan(kTechnibelAcFanLow);
    return;
  }
  // Bounds check enforcement
  if (speed > kTechnibelAcFanHigh)
    setFan(kTechnibelAcFanHigh);
  else if (speed < kTechnibelAcFanLow)
    setFan(kTechnibelAcFanLow);
  else {
    uint8_t speed_reversed = reverseBits(speed, kTechnibelAcFanSize);
    setBits(&remote_state, kTechnibelAcFanOffset, kTechnibelAcFanSize, speed_reversed);
  }
}

uint8_t IRTechnibelAc::getFan(void) {
  uint8_t speed = GETBITS64(remote_state, kTechnibelAcFanOffset, kTechnibelAcFanSize);
  return reverseBits(speed, kTechnibelAcFanSize);
}

// Convert a standard A/C Fan speed into its native fan speed.
uint8_t IRTechnibelAc::convertFan(const stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin:
    case stdAc::fanspeed_t::kLow:
      return kTechnibelAcFanLow;
    case stdAc::fanspeed_t::kMedium:
      return kTechnibelAcFanMedium;
    case stdAc::fanspeed_t::kHigh:
    case stdAc::fanspeed_t::kMax:
      return kTechnibelAcFanHigh;
    default:
      return kTechnibelAcFanLow;
  }
}

// Convert a native fan speed to it's common equivalent.
stdAc::fanspeed_t IRTechnibelAc::toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kTechnibelAcFanHigh:   return stdAc::fanspeed_t::kHigh;
    case kTechnibelAcFanMedium: return stdAc::fanspeed_t::kMedium;
    case kTechnibelAcFanLow:    return stdAc::fanspeed_t::kLow;
    default:                    return stdAc::fanspeed_t::kLow;
  }
}

uint8_t IRTechnibelAc::getMode(void) {
  uint8_t mode = GETBITS64(remote_state, kTechnibelAcModeOffset, kTechnibelAcModeSize);
  return reverseBits(mode, kTechnibelAcModeSize);
}

void IRTechnibelAc::setMode(const uint8_t mode) {
  switch (mode) {
    case kTechnibelAcHeat:
    case kTechnibelAcFan:
    case kTechnibelAcDry:
    case kTechnibelAcCool:
      break;
    default:
      this->setMode(kTechnibelAcCool);
      return;
  }
  uint8_t mode_reversed = reverseBits(mode, kTechnibelAcModeSize);
  setBits(&remote_state, kTechnibelAcModeOffset, kTechnibelAcModeSize, mode_reversed);
  setFan(getFan());  // Re-force any fan speed constraints.
  // Restore previous temp settings for cool mode.
  setTemp(_saved_temp, _saved_temp_units);
}

// Convert a standard A/C mode into its native mode.
uint8_t IRTechnibelAc::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kCool:
      return kTechnibelAcCool;
    case stdAc::opmode_t::kHeat:
      return kTechnibelAcHeat;
    case stdAc::opmode_t::kDry:
      return kTechnibelAcDry;
    case stdAc::opmode_t::kFan:
      return kTechnibelAcFan;
    default:
      return kTechnibelAcCool;
  }
}

// Convert a native mode to it's common equivalent.
stdAc::opmode_t IRTechnibelAc::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kTechnibelAcCool:  return stdAc::opmode_t::kCool;
    case kTechnibelAcHeat:  return stdAc::opmode_t::kHeat;
    case kTechnibelAcDry:   return stdAc::opmode_t::kDry;
    case kTechnibelAcFan:   return stdAc::opmode_t::kFan;
    default:                return stdAc::opmode_t::kAuto;
  }
}

void IRTechnibelAc::setSwing(const bool on) {
  setBit(&remote_state, kTechnibelAcSwingBit, on);
}

bool IRTechnibelAc::getSwing(void) {
  return GETBIT64(remote_state, kTechnibelAcSwingBit);
}

// Convert a standard A/C swing into its native swing.
bool IRTechnibelAc::convertSwing(const stdAc::swingv_t swing) {
  switch (swing) {
    case stdAc::swingv_t::kOff:
      return false;
    case stdAc::swingv_t::kAuto:
      return true;
    default:
      return true;
  }
}

// Convert a native swing to it's common equivalent.
stdAc::swingv_t IRTechnibelAc::toCommonSwing(const bool swing) {
  if(swing)
    return stdAc::swingv_t::kAuto;
  else
    return stdAc::swingv_t::kOff;
}

void IRTechnibelAc::setSleep(const bool on) {
  setBit(&remote_state, kTechnibelAcSleepBit, on);
}

bool IRTechnibelAc::getSleep(void) {
  return GETBIT64(remote_state, kTechnibelAcSleepBit);
}

void IRTechnibelAc::setTimerEnabled(const bool on) {
  setBit(&remote_state, kTechnibelAcTimerEnableBit, on);
}

bool IRTechnibelAc::getTimerEnabled(void) {
  return GETBIT64(remote_state, kTechnibelAcTimerEnableBit);
}

// Set the timer to shutdown the device in nr of minutes.
// Args:
//   nr_of_hours: Total nr of hours to wait before shutting down the device.
//               (Max 24 hrs)
void IRTechnibelAc::setTimer(const uint8_t nr_of_hours) {
  uint8_t value = std::min(kTechnibelAcTimerMax, nr_of_hours);
  uint8_t value_reversed = reverseBits(value, kTechnibelAcHoursSize);
  setBits(&remote_state, kTechnibelAcTimerHoursOffset, kTechnibelAcHoursSize, value_reversed); 
  // Enable or not?
  setTimerEnabled(value > 0);
}

uint8_t IRTechnibelAc::getTimer(void) {
  uint8_t timer = GETBITS64(remote_state, kTechnibelAcTimerHoursOffset, kTechnibelAcHoursSize);
  return reverseBits(timer, kTechnibelAcHoursSize);
}

// Convert the A/C state to it's common equivalent.
stdAc::state_t IRTechnibelAc::toCommon(void) {
  stdAc::state_t result;
  result.protocol = decode_type_t::TECHNIBEL_AC;
  result.power = getPower();
  result.mode = toCommonMode(getMode());
  result.celsius = getTempUnit();
  result.degrees = getTemp();
  result.fanspeed = toCommonFanSpeed(getFan());
  result.sleep = getSleep() ? 0 : -1;
  result.swingv = toCommonSwing(getSwing());
  // Not supported.
  result.model = -1;
  result.turbo = false;
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

// Convert the internal state into a human readable string.
String IRTechnibelAc::toString(void) {
  String result = "";
  result.reserve(100);  // Reserve some heap for the string to reduce fragging. //80
  result += addBoolToString(getPower(), kPowerStr, false);
  result += addModeToString(getMode(), kTechnibelAcCool, kTechnibelAcCool,
                            kTechnibelAcHeat, kTechnibelAcDry, kTechnibelAcFan);
  result += addFanToString(getFan(), kTechnibelAcFanHigh, kTechnibelAcFanLow,
                           kTechnibelAcFanLow, kTechnibelAcFanLow,
                           kTechnibelAcFanMedium);
  result += addTempToString(getTemp(), !getTempUnit());
  result += addBoolToString(getSleep(), kSleepStr);
  result += addBoolToString(getSwing(), kSwingVStr);
  uint8_t hours = getTimer();
  result += addLabeledString((hours && getTimerEnabled()) ? minsToString(hours * 60) : kOffStr, kTimerStr);
  return result;
}
