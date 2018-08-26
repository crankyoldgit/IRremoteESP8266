// Copyright 2009 Ken Shirriff
// Copyright 2017 David Conran

#include "ir_Mitsubishi.h"
#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

//    MMMMM  IIIII TTTTT   SSSS  U   U  BBBB   IIIII   SSSS  H   H  IIIII
//    M M M    I     T    S      U   U  B   B    I    S      H   H    I
//    M M M    I     T     SSS   U   U  BBBB     I     SSS   HHHHH    I
//    M   M    I     T        S  U   U  B   B    I        S  H   H    I
//    M   M  IIIII   T    SSSS    UUU   BBBBB  IIIII  SSSS   H   H  IIIII

// Mitsubishi (TV) decoding added from https://github.com/z3t0/Arduino-IRremote
// Mitsubishi (TV) sending & Mitsubishi A/C support added by David Conran

// Constants
// Mitsubishi TV
// period time is 1/33000Hz = 30.303 uSeconds (T)
// Ref:
//   GlobalCache's Control Tower's Mitsubishi TV data.
//   https://github.com/marcosamarinho/IRremoteESP8266/blob/master/ir_Mitsubishi.cpp
const uint16_t kMitsubishiTick = 30;
const uint16_t kMitsubishiBitMarkTicks = 10;
const uint16_t kMitsubishiBitMark = kMitsubishiBitMarkTicks * kMitsubishiTick;
const uint16_t kMitsubishiOneSpaceTicks = 70;
const uint16_t kMitsubishiOneSpace = kMitsubishiOneSpaceTicks * kMitsubishiTick;
const uint16_t kMitsubishiZeroSpaceTicks = 30;
const uint16_t kMitsubishiZeroSpace =
    kMitsubishiZeroSpaceTicks * kMitsubishiTick;
const uint16_t kMitsubishiMinCommandLengthTicks = 1786;
const uint16_t kMitsubishiMinCommandLength =
    kMitsubishiMinCommandLengthTicks * kMitsubishiTick;
const uint16_t kMitsubishiMinGapTicks = 936;
const uint16_t kMitsubishiMinGap = kMitsubishiMinGapTicks * kMitsubishiTick;

// Mitsubishi Projector (HC3000)
// Ref:
//   https://github.com/markszabo/IRremoteESP8266/issues/441

const uint16_t kMitsubishi2HdrMark = 8400;
const uint16_t kMitsubishi2HdrSpace = kMitsubishi2HdrMark / 2;
const uint16_t kMitsubishi2BitMark = 560;
const uint16_t kMitsubishi2ZeroSpace = 520;
const uint16_t kMitsubishi2OneSpace = kMitsubishi2ZeroSpace * 3;
const uint16_t kMitsubishi2MinGap = 28500;

// Mitsubishi A/C
// Ref:
//   https://github.com/r45635/HVAC-IR-Control/blob/master/HVAC_ESP8266/HVAC_ESP8266.ino#L84

const uint16_t kMitsubishiAcHdrMark = 3400;
const uint16_t kMitsubishiAcHdrSpace = 1750;
const uint16_t kMitsubishiAcBitMark = 450;
const uint16_t kMitsubishiAcOneSpace = 1300;
const uint16_t kMitsubishiAcZeroSpace = 420;
const uint16_t kMitsubishiAcRptMark = 440;
const uint16_t kMitsubishiAcRptSpace = 17100;

#if SEND_MITSUBISHI
// Send a Mitsubishi message
//
// Args:
//   data:   Contents of the message to be sent.
//   nbits:  Nr. of bits of data to be sent. Typically kMitsubishiBits.
//   repeat: Nr. of additional times the message is to be sent.
//
// Status: ALPHA / untested.
//
// Notes:
//   This protocol appears to have no header.
// Ref:
//   https://github.com/marcosamarinho/IRremoteESP8266/blob/master/ir_Mitsubishi.cpp
//   GlobalCache's Control Tower's Mitsubishi TV data.
void IRsend::sendMitsubishi(uint64_t data, uint16_t nbits, uint16_t repeat) {
  sendGeneric(0, 0,  // No Header
              kMitsubishiBitMark, kMitsubishiOneSpace,
              kMitsubishiBitMark, kMitsubishiZeroSpace,
              kMitsubishiBitMark, kMitsubishiMinGap,
              kMitsubishiMinCommandLength,
              data, nbits, 33, true, repeat, 50);
}
#endif   // SEND_MITSUBISHI

#if DECODE_MITSUBISHI
// Decode the supplied Mitsubishi message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of data bits to expect.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: BETA / previously working.
//
// Notes:
//   This protocol appears to have no header.
//
// Ref:
//   GlobalCache's Control Tower's Mitsubishi TV data.
bool IRrecv::decodeMitsubishi(decode_results *results, uint16_t nbits,
                              bool strict) {
  if (results->rawlen < 2 * nbits + kFooter - 1)
    return false;  // Shorter than shortest possibly expected.
  if (strict && nbits != kMitsubishiBits)
    return false;  // Request is out of spec.

  uint16_t offset = kStartOffset;
  uint64_t data = 0;

  // No Header
  // But try to auto-calibrate off the initial mark signal.
  if (!matchMark(results->rawbuf[offset], kMitsubishiBitMark, 30))
    return false;
  // Calculate how long the common tick time is based on the initial mark.
  uint32_t tick = results->rawbuf[offset] *
      kRawTick / kMitsubishiBitMarkTicks;

  // Data
  match_result_t data_result = matchData(&(results->rawbuf[offset]), nbits,
                                         kMitsubishiBitMarkTicks * tick,
                                         kMitsubishiOneSpaceTicks * tick,
                                         kMitsubishiBitMarkTicks * tick,
                                         kMitsubishiZeroSpaceTicks * tick);
  if (data_result.success == false) return false;
  data = data_result.data;
  offset += data_result.used;
  uint16_t actualBits = data_result.used / 2;

  // Footer
  if (!matchMark(results->rawbuf[offset++], kMitsubishiBitMarkTicks * tick,
                 30)) return false;
  if (offset < results->rawlen &&
      !matchAtLeast(results->rawbuf[offset], kMitsubishiMinGapTicks * tick))
    return false;

  // Compliance
  if (actualBits < nbits)
    return false;
  if (strict && actualBits != nbits)
    return false;  // Not as we expected.

  // Success
  results->decode_type = MITSUBISHI;
  results->bits = actualBits;
  results->value = data;
  results->address = 0;
  results->command = 0;
  return true;
}
#endif  // DECODE_MITSUBISHI

#if SEND_MITSUBISHI2
// Send a Mitsubishi2 message
//
// Args:
//   data:   Contents of the message to be sent.
//   nbits:  Nr. of bits of data to be sent. Typically kMitsubishiBits.
//   repeat: Nr. of additional times the message is to be sent.
//
// Status: ALPHA / untested.
//
// Notes:
//   Based on a Mitsubishi HC3000 projector's remote.
//   This protocol appears to have a manditory in-protocol repeat.
//   That is in *addition* to the entire message needing to be sent twice
//   for the device to accept the command. That is separate from the repeat.
//   i.e. Allegedly, the real remote requires the "OFF" button pressed twice.
//        You will need to add a suitable gap yourself.
// Ref:
//   https://github.com/markszabo/IRremoteESP8266/issues/441
void IRsend::sendMitsubishi2(uint64_t data, uint16_t nbits, uint16_t repeat) {
  for (uint16_t i = 0; i <= repeat; i++) {
    // First half of the data.
    sendGeneric(kMitsubishi2HdrMark, kMitsubishi2HdrSpace,
      kMitsubishi2BitMark, kMitsubishi2OneSpace,
      kMitsubishi2BitMark, kMitsubishi2ZeroSpace,
      kMitsubishi2BitMark, kMitsubishi2HdrSpace,
      data >> (nbits / 2), nbits / 2, 33, true, 0, 50);
    // Second half of the data.
    sendGeneric(0, 0,  // No header for the second data block
      kMitsubishi2BitMark, kMitsubishi2OneSpace,
      kMitsubishi2BitMark, kMitsubishi2ZeroSpace,
      kMitsubishi2BitMark, kMitsubishi2MinGap,
      data & ((1 << (nbits / 2)) - 1), nbits / 2, 33, true, 0, 50);
  }
}
#endif  // SEND_MITSUBISHI2

#if DECODE_MITSUBISHI2
// Decode the supplied Mitsubishi2 message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   Nr. of data bits to expect.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: BETA / Works with simulated data.
//
// Notes:
//   Hardware supported:
//     * Mitsubishi HC3000 projector's remote.
//
// Ref:
//   https://github.com/markszabo/IRremoteESP8266/issues/441
bool IRrecv::decodeMitsubishi2(decode_results *results, uint16_t nbits,
                               bool strict) {
  if (results->rawlen < 2 * nbits + kHeader + (kFooter * 2) - 1)
    return false;  // Shorter than shortest possibly expected.
  if (strict && nbits != kMitsubishiBits)
    return false;  // Request is out of spec.

  uint16_t offset = kStartOffset;
  uint64_t data = 0;
  uint16_t actualBits = 0;

  // Header
  if (!matchMark(results->rawbuf[offset++], kMitsubishi2HdrMark))
    return false;
  if (!matchSpace(results->rawbuf[offset++], kMitsubishi2HdrSpace))
    return false;
  for (uint8_t i = 1; i <= 2; i++) {
    // Data
    match_result_t data_result = matchData(&(results->rawbuf[offset]),
                                           nbits / 2,
                                           kMitsubishi2BitMark,
                                           kMitsubishi2OneSpace,
                                           kMitsubishi2BitMark,
                                           kMitsubishi2ZeroSpace);
    if (data_result.success == false) return false;
    data <<= nbits / 2;
    data += data_result.data;
    offset += data_result.used;
    actualBits += data_result.used / 2;

    // Footer
    if (!matchMark(results->rawbuf[offset++], kMitsubishi2BitMark))
      return false;
    if (i % 2) {  // Every odd data block, we expect a HDR space.
      if (!matchSpace(results->rawbuf[offset++], kMitsubishi2HdrSpace))
        return false;
    } else {  // Every even data block, we expect Min Gap or end of the message.
      if (offset < results->rawlen &&
          !matchAtLeast(results->rawbuf[offset++], kMitsubishi2MinGap))
        return false;
    }
  }


  // Compliance
  if (actualBits < nbits)
    return false;
  if (strict && actualBits != nbits)
    return false;  // Not as we expected.

  // Success
  results->decode_type = MITSUBISHI2;
  results->bits = actualBits;
  results->value = data;
  results->address = data >> actualBits / 2;
  results->command = data & ((1 << (actualBits / 2)) - 1);
  return true;
}
#endif  // DECODE_MITSUBISHI2

#if SEND_MITSUBISHI_AC
// Send a Mitsubishi A/C message.
//
// Args:
//   data: An array of bytes containing the IR command.
//   nbytes: Nr. of bytes of data in the array. (>=kMitsubishiACStateLength)
//   repeat: Nr. of times the message is to be repeated.
//          (Default = kMitsubishiACMinRepeat).
//
// Status: BETA / Appears to be working.
//
void IRsend::sendMitsubishiAC(unsigned char data[], uint16_t nbytes,
                              uint16_t repeat) {
  if (nbytes < kMitsubishiACStateLength)
    return;  // Not enough bytes to send a proper message.

  sendGeneric(kMitsubishiAcHdrMark, kMitsubishiAcHdrSpace,
              kMitsubishiAcBitMark, kMitsubishiAcOneSpace,
              kMitsubishiAcBitMark, kMitsubishiAcZeroSpace,
              kMitsubishiAcRptMark, kMitsubishiAcRptSpace,
              data, nbytes, 38, false, repeat, 50);
}
#endif  // SEND_MITSUBISHI_AC

// Code to emulate Mitsubishi A/C IR remote control unit.
// Inspired and derived from the work done at:
//   https://github.com/r45635/HVAC-IR-Control
//
// Warning: Consider this very alpha code. Seems to work, but not validated.
//
// Equipment it seems compatible with:
//  * <Add models (A/C & remotes) you've gotten it working with here>
// Initialise the object.
IRMitsubishiAC::IRMitsubishiAC(uint16_t pin) : _irsend(pin) {
  stateReset();
}

// Reset the state of the remote to a known good state/sequence.
void IRMitsubishiAC::stateReset() {
  // The state of the IR remote in IR code form.
  // Known good state obtained from:
  //   https://github.com/r45635/HVAC-IR-Control/blob/master/HVAC_ESP8266/HVAC_ESP8266.ino#L108
  // Note: Can't use the following because it requires -std=c++11
  // uint8_t known_good_state[kMitsubishiACStateLength] = {
  //    0x23, 0xCB, 0x26, 0x01, 0x00, 0x20, 0x08, 0x06, 0x30, 0x45, 0x67, 0x00,
  //    0x00, 0x00, 0x00, 0x00, 0x00, 0x1F};
  remote_state[0] = 0x23;
  remote_state[1] = 0xCB;
  remote_state[2] = 0x26;
  remote_state[3] = 0x01;
  remote_state[4] = 0x00;
  remote_state[5] = 0x20;
  remote_state[6] = 0x08;
  remote_state[7] = 0x06;
  remote_state[8] = 0x30;
  remote_state[9] = 0x45;
  remote_state[10] = 0x67;
  for (uint8_t i = 11; i < kMitsubishiACStateLength - 1; i++)
    remote_state[i] = 0;
  remote_state[kMitsubishiACStateLength - 1] = 0x1F;
  checksum();  // Calculate the checksum
}

// Configure the pin for output.
void IRMitsubishiAC::begin() {
    _irsend.begin();
}

#if SEND_MITSUBISHI_AC
// Send the current desired state to the IR LED.
void IRMitsubishiAC::send() {
  checksum();   // Ensure correct checksum before sending.
  _irsend.sendMitsubishiAC(remote_state);
}
#endif  // SEND_MITSUBISHI_AC

// Return a pointer to the internal state date of the remote.
uint8_t* IRMitsubishiAC::getRaw() {
  checksum();
  return remote_state;
}

// Calculate the checksum for the current internal state of the remote.
void IRMitsubishiAC::checksum() {
  uint8_t sum = 0;
  // Checksum is simple addition of all previous bytes stored
  // as an 8 bit value.
  for (uint8_t i = 0; i < 17; i++)
    sum += remote_state[i];
  remote_state[17] = sum & 0xFFU;
}

// Set the requested power state of the A/C to off.
void IRMitsubishiAC::on() {
  // state = ON;
  remote_state[5] |= kMitsubishiAcPower;
}

// Set the requested power state of the A/C to off.
void IRMitsubishiAC::off() {
  // state = OFF;
  remote_state[5] &= ~kMitsubishiAcPower;
}

// Set the requested power state of the A/C.
void IRMitsubishiAC::setPower(bool state) {
  if (state)
    on();
  else
    off();
}

// Return the requested power state of the A/C.
bool IRMitsubishiAC::getPower() {
  return((remote_state[5] & kMitsubishiAcPower) != 0);
}

// Set the temp. in deg C
void IRMitsubishiAC::setTemp(uint8_t temp) {
  temp = std::max((uint8_t) kMitsubishiAcMinTemp, temp);
  temp = std::min((uint8_t) kMitsubishiAcMaxTemp, temp);
  remote_state[7] = temp - kMitsubishiAcMinTemp;
}

// Return the set temp. in deg C
uint8_t IRMitsubishiAC::getTemp() {
  return(remote_state[7] + kMitsubishiAcMinTemp);
}

// Set the speed of the fan, 0-6.
// 0 is auto, 1-5 is the speed, 6 is silent.
void IRMitsubishiAC::setFan(uint8_t fan) {
  // Bounds check
  if (fan > kMitsubishiAcFanSilent)
    fan = kMitsubishiAcFanMax;  // Set the fan to maximum if out of range.
  if (fan == kMitsubishiAcFanAuto) {   // Automatic is a special case.
    remote_state[9] = 0b10000000 | (remote_state[9] & 0b01111000);
    return;
  } else if (fan >= kMitsubishiAcFanMax) {
    fan--;  // There is no spoon^H^H^Heed 5 (max), pretend it doesn't exist.
  }
  remote_state[9] &= 0b01111000;  // Clear the previous state
  remote_state[9] |= fan;
}

// Return the requested state of the unit's fan.
uint8_t IRMitsubishiAC::getFan() {
  uint8_t fan = remote_state[9] & 0b111;
  if (fan == kMitsubishiAcFanMax)
    return kMitsubishiAcFanSilent;
  return fan;
}

// Return the requested climate operation mode of the a/c unit.
uint8_t IRMitsubishiAC::getMode() {
  return(remote_state[6]);
}

// Set the requested climate operation mode of the a/c unit.
void IRMitsubishiAC::setMode(uint8_t mode) {
  // If we get an unexpected mode, default to AUTO.
  switch (mode) {
    case kMitsubishiAcAuto: break;
    case kMitsubishiAcCool: break;
    case kMitsubishiAcDry: break;
    case kMitsubishiAcHeat: break;
    default: mode = kMitsubishiAcAuto;
  }
  remote_state[6] = mode;
}

// Set the requested vane operation mode of the a/c unit.
void IRMitsubishiAC::setVane(uint8_t mode) {
  mode = std::min(mode, (uint8_t) 0b111);  // bounds check
  mode |= 0b1000;
  mode <<= 3;
  remote_state[9] &= 0b11000111;  // Clear the previous setting.
  remote_state[9] |= mode;
}

// Return the requested vane operation mode of the a/c unit.
uint8_t IRMitsubishiAC::getVane() {
  return ((remote_state[9] & 0b00111000) >> 3);
}
