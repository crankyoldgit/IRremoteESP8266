// Copyright 2018 Erdem U. Altinyurt
// Copyright 2019 David Conran

#include "ir_Vestel.h"
#include <algorithm>
#ifndef UNIT_TEST
#include <Arduino.h>
#else
#include <string>
#endif
#include "IRrecv.h"
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "IRutils.h"
#include "ir_Haier.h"

//                 VV     VV  EEEEEEE   SSSSS  TTTTTTTT  EEEEEEE  LL
//                 VV     VV  EE       S          TT     EE       LL
//                  VV   VV   EEEEE     SSSS      TT     EEEEE    LL
//                   VV VV    EE            S     TT     EE       LL
//                    VVV     EEEEEEE  SSSSS      TT     EEEEEEE  LLLLLLL

// Vestel added by Erdem U. Altinyurt

// Equipment it seems compatible with:
//  * Vestel AC Model BIOX CXP-9 (9K BTU)
//  * <Add models (A/C & remotes) you've gotten it working with here>

// Ref:
//   None. Totally reverse engineered.

#if SEND_VESTEL_AC
// Send a Vestel message
//
// Args:
//   data:   Contents of the message to be sent.
//   nbits:  Nr. of bits of data to be sent. Typically kVestelBits.
//
// Status: STABLE / Working.
//
void IRsend::sendVestelAC(const uint64_t data, const uint16_t nbits,
                          const uint16_t repeat) {
  if (nbits % 8 != 0) return;  // nbits is required to be a multiple of 8.

  sendGeneric(kVestelACHdrMark, kVestelACHdrSpace,   // Header
              kVestelACBitMark, kVestelACOneSpace,   // Data
              kVestelACBitMark, kVestelACZeroSpace,  // Data
              kVestelACBitMark, 100000,              // Footer + repeat gap
              data, nbits, 38, false, repeat, 50);
}
#endif

// Code to emulate Vestel A/C IR remote control unit.

// Initialise the object.
IRVestelAC::IRVestelAC(uint16_t pin) : _irsend(pin) { stateReset(); }

// Reset the state of the remote to a known good state/sequence.
void IRVestelAC::stateReset() {
  // Power On, Mode Auto, Fan Auto, Temp = 25C/77F
  remote_state = 0x0F00D9001FEF201ULL;
  remote_time_state = 0x201ULL;
  use_time_state = false;
}

// Configure the pin for output.
void IRVestelAC::begin() {
  _irsend.begin();
}

#if SEND_VESTEL_AC
// Send the current desired state to the IR LED.
void IRVestelAC::send() {
  checksum();  // Ensure correct checksum before sending.
  uint64_t code_to_send;
  if (use_time_state)
    code_to_send = remote_time_state;
  else
    code_to_send = remote_state;
  _irsend.sendVestelAC(code_to_send);
}
#endif  // SEND_VESTEL_AC

// Return the internal state date of the remote.
uint64_t IRVestelAC::getRaw() {
  checksum();
  if (use_time_state) return remote_time_state;
  return remote_state;
}

// Override the internal state with the new state.
void IRVestelAC::setRaw(uint8_t* newState) {
  uint64_t upState = 0;
  for (int i = 0; i < 7; i++)
    upState |= static_cast<uint64_t>(newState[i]) << (i * 8);
  remote_state = upState;
  remote_time_state = upState;
}

void IRVestelAC::setRaw(const uint64_t newState) {
  remote_state = newState;
  remote_time_state = newState;
}

// Set the requested power state of the A/C to on.
void IRVestelAC::on() { setPower(true); }

// Set the requested power state of the A/C to off.
void IRVestelAC::off() { setPower(false); }

// Set the requested power state of the A/C.
void IRVestelAC::setPower(const bool state) {
  remote_state &= ~((uint64_t)0xF << kVestelACPowerOffset);
  if (state)
    remote_state |= ((uint64_t)0xF << kVestelACPowerOffset);
  else
    remote_state |= ((uint64_t)0xC << kVestelACPowerOffset);
  use_time_state = false;
}

// Return the requested power state of the A/C.
bool IRVestelAC::getPower() {
  return (remote_state >> kVestelACPowerOffset == 0xF);
}

// Set the temperature in Celsius degrees.
void IRVestelAC::setTemp(const uint8_t temp) {
  uint8_t new_temp = temp;
  new_temp = std::max(kVestelACMinTempC, new_temp);
  // new_temp = std::max(kVestelACMinTempH, new_temp); Check MODE
  new_temp = std::min(kVestelACMaxTemp, new_temp);
  remote_state &= ~((uint64_t)0xF << kVestelACTempOffset);
  remote_state |= (uint64_t)(new_temp - 16) << kVestelACTempOffset;
  use_time_state = false;
}

// Return the set temperature.
uint8_t IRVestelAC::getTemp(void) {
  return ((remote_state >> kVestelACTempOffset) & 0xF) + 16;
}

// Set the speed of the fan,
// 1-3 set the fan speed, 0 or anything else set it to auto.
void IRVestelAC::setFan(const uint8_t fan) {
  switch (fan) {
    case kVestelACFanLow:
    case kVestelACFanMed:
    case kVestelACFanHigh:
    case kVestelACFanAutoCool:
    case kVestelACFanAutoHot:
    case kVestelACFanAuto:
      remote_state &= ~((uint64_t)0xF << kVestelACFanOffset);
      remote_state |= (uint64_t)fan << kVestelACFanOffset;
      break;
    default:
      setFan(kVestelACFanAuto);
  }
  use_time_state = false;
}

// Return the requested state of the unit's fan.
uint8_t IRVestelAC::getFan() {
  return (remote_state >> kVestelACFanOffset) & 0xF;
}

// Get the requested climate operation mode of the a/c unit.
// Returns:
//   A uint8_t containing the A/C mode.
uint8_t IRVestelAC::getMode() {
  return (remote_state >> kVestelACModeOffset) & 0xF;
}

// Set the requested climate operation mode of the a/c unit.
void IRVestelAC::setMode(const uint8_t mode) {
  // If we get an unexpected mode, default to AUTO.
  switch (mode) {
    case kVestelACAuto:
    case kVestelACCool:
    case kVestelACHeat:
    case kVestelACDry:
    case kVestelACFan:
      remote_state &= ~((uint64_t)0xF << kVestelACModeOffset);
      remote_state |= (uint64_t)mode << kVestelACModeOffset;
      break;
    default:
      setMode(kVestelACAuto);
  }
  use_time_state = false;
}

// Set Auto mode of AC.
void IRVestelAC::setAuto(const int8_t autoLevel) {
  if (autoLevel < -2 || autoLevel > 2) return;
  setMode(kVestelACAuto);
  setFan((autoLevel < 0 ? kVestelACFanAutoCool : kVestelACFanAutoHot));
  if (autoLevel == 2)
    setTemp(30);
  else if (autoLevel == 1)
    setTemp(31);
  else if (autoLevel == 0)
    setTemp(25);
  else if (autoLevel == -1)
    setTemp(16);
  else if (autoLevel == -2)
    setTemp(17);
}

void IRVestelAC::setTimerActive(const bool on) {
  if (on)  // activation
    remote_time_state |= ((uint64_t)1 << kVestelACTimerFlagOffset);
  else  // deactivate
    remote_time_state &= ~((uint64_t)1 << kVestelACTimerFlagOffset);
  use_time_state = true;
}

bool IRVestelAC::isTimerActive(void) {
  return (remote_time_state >> kVestelACTimerFlagOffset) & 1;
}

// Set Timer option of AC.
// Valid time arguments are 0, 0.5, 1, 2, 3 and 5 hours (in min). 0 disables the
// timer.
void IRVestelAC::setTimer(const uint16_t minutes) {
  // Clear both On & Off timers.
  remote_time_state &= ~((uint64_t)0xFFFF << kVestelACOffTimeOffset);
  // Set the "Off" time with the nr of minutes before we turn off.
  remote_time_state |= (uint64_t)(((minutes / 60) << 3) + (minutes % 60) / 10)
                       << kVestelACOffTimeOffset;
  setOffTimerActive(false);
  // Yes. On Timer instead of Off timer active.
  setOnTimerActive(minutes != 0);
  setTimerActive(minutes != 0);
  use_time_state = true;
}

uint16_t IRVestelAC::getTimer(void) { return getOffTimer(); }

// Set the AC's internal clock
void IRVestelAC::setTime(const uint16_t minutes) {
  remote_time_state &= ~((uint64_t)0x1F << kVestelACHourOffset);
  remote_time_state |= (uint64_t)((minutes / 60) & 0x1F)
                       << kVestelACHourOffset;
  remote_time_state &= ~((uint64_t)0xFF << kVestelACMinuteOffset);
  remote_time_state |= (uint64_t)((minutes % 60) & 0xFF)
                       << kVestelACMinuteOffset;
  use_time_state = true;
}

uint16_t IRVestelAC::getTime(void) {
  return ((remote_time_state >> kVestelACHourOffset) & 0x1F) * 60 +
         ((remote_time_state >> kVestelACMinuteOffset) & 0xFF);
}

void IRVestelAC::setOnTimerActive(const bool on) {
  if (on)  // activation
    remote_time_state |= ((uint64_t)1 << kVestelACOnTimerFlagOffset);
  else  // deactivate
    remote_time_state &= ~((uint64_t)1 << kVestelACOnTimerFlagOffset);
  use_time_state = true;
}

bool IRVestelAC::isOnTimerActive(void) {
  return (remote_time_state >> kVestelACOnTimerFlagOffset) & 1;
}

// Set AC's wake up time. Takes time in minute.
void IRVestelAC::setOnTimer(const uint16_t minutes) {
  remote_time_state &= ~((uint64_t)0xFF << kVestelACOnTimeOffset);
  remote_time_state |= (uint64_t)(((minutes / 60) << 3) + (minutes % 60) / 10)
                       << kVestelACOnTimeOffset;
  setOnTimerActive(minutes != 0);
  setTimerActive(false);
  use_time_state = true;
}

uint16_t IRVestelAC::getOnTimer(void) {
  uint8_t ontime = (remote_time_state >> kVestelACOnTimeOffset) & 0xFF;
  return (ontime >> 3) * 60 + (ontime & 0x7) * 10;
}

void IRVestelAC::setOffTimerActive(const bool on) {
  if (on)  // activation
    remote_time_state |= ((uint64_t)1 << kVestelACOffTimerFlagOffset);
  else  // deactivate
    remote_time_state &= ~((uint64_t)1 << kVestelACOffTimerFlagOffset);
  use_time_state = true;
}

bool IRVestelAC::isOffTimerActive(void) {
  return (remote_time_state >> kVestelACOffTimerFlagOffset) & 1;
}

// Set AC's turn off time. Takes time in minute.
void IRVestelAC::setOffTimer(const uint16_t minutes) {
  remote_time_state &= ~((uint64_t)0xFF << kVestelACOffTimeOffset);
  remote_time_state |=
      (uint64_t)((((minutes / 60) << 3) + (minutes % 60) / 10) & 0xFF)
      << kVestelACOffTimeOffset;
  setOffTimerActive(minutes != 0);
  setTimerActive(false);
  use_time_state = true;
}

uint16_t IRVestelAC::getOffTimer(void) {
  uint8_t offtime = (remote_time_state >> kVestelACOffTimeOffset) & 0xFF;
  return (offtime >> 3) * 60 + (offtime & 0x7) * 10;
}

// Set the Sleep state of the A/C.
void IRVestelAC::setSleep(const bool state) {
  remote_state &= ~((uint64_t)0xF << kVestelACTurboSleepOffset);
  remote_state |= (uint64_t)(state ? kVestelACSleep : kVestelACNormal)
                  << kVestelACTurboSleepOffset;
  use_time_state = false;
}

// Return the Sleep state of the A/C.
bool IRVestelAC::getSleep() {
  return ((remote_state >> kVestelACTurboSleepOffset) & 0xF) == kVestelACSleep;
}

// Set the Turbo state of the A/C.
void IRVestelAC::setTurbo(const bool state) {
  remote_state &= ~((uint64_t)0xF << kVestelACTurboSleepOffset);
  remote_state |= (uint64_t)(state ? kVestelACTurbo : kVestelACNormal)
                  << kVestelACTurboSleepOffset;
  use_time_state = false;
}

// Return the Turbo state of the A/C.
bool IRVestelAC::getTurbo() {
  return ((remote_state >> kVestelACTurboSleepOffset) & 0xF) == kVestelACTurbo;
}

// Set the Ion state of the A/C.
void IRVestelAC::setIon(const bool state) {
  remote_state &= ~((uint64_t)0x1 << kVestelACIonOffset);

  remote_state |= (uint64_t)(state ? 1 : 0) << kVestelACIonOffset;
  use_time_state = false;
}

// Return the Ion state of the A/C.
bool IRVestelAC::getIon() { return (remote_state >> kVestelACIonOffset) & 1; }

// Set the Swing Roaming state of the A/C.
void IRVestelAC::setSwing(const bool state) {
  remote_state &= ~((uint64_t)0xF << kVestelACSwingOffset);

  remote_state |= (uint64_t)(state ? kVestelACSwing : 0xF)
                  << kVestelACSwingOffset;
  use_time_state = false;
}

// Return the Swing Roaming state of the A/C.
bool IRVestelAC::getSwing() {
  return ((remote_state >> kVestelACSwingOffset) & 0xF) == kVestelACSwing;
}

// Calculate the checksum for a given array.
// Args:
//   state:  The state to calculate the checksum over.
// Returns:
//   The 8 bit checksum value.
uint8_t IRVestelAC::calcChecksum(const uint64_t state) {
  // Just counts the set bits +1 on stream and take inverse after mask
  uint8_t sum = 0;
  uint64_t temp_state = state & kVestelACCRCMask;
  for (; temp_state; temp_state >>= 1)
    if (temp_state & 1) sum++;
  sum += 2;
  sum = 0xff - sum;
  return sum;
}

// Verify the checksum is valid for a given state.
// Args:
//   state:  The state to verify the checksum of.
// Returns:
//   A boolean.
bool IRVestelAC::validChecksum(const uint64_t state) {
  return (((state >> kVestelACChecksumOffset) & 0xFF) == calcChecksum(state));
}

// Calculate & set the checksum for the current internal state of the remote.
void IRVestelAC::checksum() {
  // Stored the checksum value in the last byte.
  remote_state &= ~((uint64_t)0xFF << kVestelACChecksumOffset);
  remote_state |= (uint64_t)calcChecksum(remote_state)
                  << kVestelACChecksumOffset;

  remote_time_state &= ~((uint64_t)0xFF << kVestelACChecksumOffset);
  remote_time_state |= (uint64_t)calcChecksum(remote_time_state)
                       << kVestelACChecksumOffset;
}

bool IRVestelAC::isTimeCommand() {
  return (remote_state >> kVestelACPowerOffset == 0x00 || use_time_state);
}

// Convert the internal state into a human readable string.
#ifdef ARDUINO
String IRVestelAC::toString() {
  String result = "";
#else
std::string IRVestelAC::toString() {
  std::string result = "";
#endif  // ARDUINO
  if (isTimeCommand()) {
    result += "Time: " + IRHaierAC::timeToString(getTime());

    result += ", Timer: ";
    result += isTimerActive() ? IRHaierAC::timeToString(getTimer()) : "Off";

    result += ", On Timer: ";
    result += (isOnTimerActive() && !isTimerActive())
                  ? IRHaierAC::timeToString(getOnTimer())
                  : "Off";

    result += ", Off Timer: ";
    result +=
        isOffTimerActive() ? IRHaierAC::timeToString(getOffTimer()) : "Off";
    return result;
  }
  // Not a time command, it's a normal command.
  result += "Power: ";
  result += (getPower() ? "On" : "Off");
  result += ", Mode: " + uint64ToString(getMode());
  switch (getMode()) {
    case kVestelACAuto:
      result += " (AUTO)";
      break;
    case kVestelACCool:
      result += " (COOL)";
      break;
    case kVestelACHeat:
      result += " (HEAT)";
      break;
    case kVestelACDry:
      result += " (DRY)";
      break;
    case kVestelACFan:
      result += " (FAN)";
      break;
    default:
      result += " (UNKNOWN)";
  }
  result += ", Temp: " + uint64ToString(getTemp()) + "C";
  result += ", Fan: " + uint64ToString(getFan());
  switch (getFan()) {
    case kVestelACFanAuto:
      result += " (AUTO)";
      break;
    case kVestelACFanLow:
      result += " (LOW)";
      break;
    case kVestelACFanMed:
      result += " (MEDIUM)";
      break;
    case kVestelACFanHigh:
      result += " (HIGH)";
      break;
    case kVestelACFanAutoCool:
      result += " (AUTO COOL)";
      break;
    case kVestelACFanAutoHot:
      result += " (AUTO HOT)";
      break;
    default:
      result += " (UNKNOWN)";
  }
  result += ", Sleep: ";
  result += (getSleep() ? "On" : "Off");
  result += ", Turbo: ";
  result += (getTurbo() ? "On" : "Off");
  result += ", Ion: ";
  result += (getIon() ? "On" : "Off");
  result += ", Swing: ";
  result += (getSwing() ? "On" : "Off");
  return result;
}

#if DECODE_VESTEL_AC
// Decode the supplied Vestel message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect. Typically kVestelBits.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: Alpha / Needs testing against a real device.
//
bool IRrecv::decodeVestelAC(decode_results* results, uint16_t nbits,
                            bool strict) {
  if (nbits % 8 != 0)  // nbits has to be a multiple of nr. of bits in a byte.
    return false;

  if (strict)
    if (nbits != kVestelACBits)
      return false;  // Not strictly a Vestel AC message.

  uint64_t data = 0;
  uint16_t offset = kStartOffset;

  if (nbits > sizeof(data) * 8)
    return false;  // We can't possibly capture a Vestel packet that big.

  // Header
  if (!matchMark(results->rawbuf[offset++], kVestelACHdrMark)) return false;
  if (!matchSpace(results->rawbuf[offset++], kVestelACHdrSpace)) return false;

  // Data (Normal)
  match_result_t data_result =
      matchData(&(results->rawbuf[offset]), nbits, kVestelACBitMark,
                kVestelACOneSpace, kVestelACBitMark, kVestelACZeroSpace,
                kVestelACTolerance, kMarkExcess, false);

  if (data_result.success == false) return false;
  offset += data_result.used;
  data = data_result.data;

  // Footer
  if (!matchMark(results->rawbuf[offset++], kVestelACBitMark)) return false;

  // Compliance
  if (strict)
    if (!IRVestelAC::validChecksum(data_result.data)) return false;

  // Success
  results->decode_type = VESTEL_AC;
  results->bits = nbits;
  results->value = data;
  results->address = 0;
  results->command = 0;

  return true;
}
#endif  // DECODE_VESTEL_AC
