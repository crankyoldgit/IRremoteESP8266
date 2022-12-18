// Author: Dobri Dobrev (Onepamopa)
/// @file
/// @brief Support for IKEDA protocol
// Supports:
//   Brand: IKEDA,  Model: TODO add device and remote


#include "ir_Ikeda.h"
#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRtext.h"
#include "IRutils.h"

using irutils::addBoolToString;
using irutils::addFanToString;
using irutils::addModeToString;
using irutils::addTempToString;
using irutils::bcdToUint8;
using irutils::uint8ToBcd;
using irutils::sumBytes;

/**
 * Decode pulse distance width protocols.
 *
 * We can have the following protocol timings
 * Pulse distance:          Pulses/marks are constant, pause/spaces have different length, like NEC.
 * Pulse width:             Pulses/marks have different length, pause/spaces are constant, like Sony.
 * Pulse distance width:    Pulses/marks and pause/spaces have different length, often the bit length is constant, like MagiQuest.
 * Pulse distance width can be decoded like pulse width decoder, if this decoder does not check the length of pause/spaces.
 *
 * Input is     IrReceiver.decodedIRData.rawDataPtr->rawbuf[]
 * Output is    IrReceiver.decodedIRData.decodedRawData
 *
 * Assume pulse distance if aOneMarkMicros == aZeroMarkMicros
 *
 * @param   aStartOffset        Must point to a mark
 * @param   aOneMarkMicros      Taken as constant BitMarkMicros for pulse distance.
 * @param   aZeroMarkMicros     Not required if DECODE_STRICT_CHECKS is not defined.
 * @param   aOneSpaceMicros     Taken as (constant) BitSpaceMicros for pulse width.
 * @param   aZeroSpaceMicros    Not required if DECODE_STRICT_CHECKS is not defined.
 * @return  true                If decoding was successful
 */


// Original library detects "Pulse distance" and LSB

// See https://github.com/crankyoldgit/IRremoteESP8266/wiki/Adding-support-for-a-new-IR-protocol
// for details of how to include this in the library.
const uint16_t kIKEDAHdrMark 	    = 8912; ///< uSeconds.
const uint16_t kIKEDABitMark 	    = 619;  ///< uSeconds.
const uint16_t kIKEDAHdrSpace 	  = 2042; ///< uSeconds.
const uint16_t kIKEDAOneSpace 	  = 1212; ///< uSeconds.
const uint16_t kIKEDAZeroSpace 	  = 477;  ///< uSeconds.
const uint32_t kTESTSanyoAc88Gap  = 3675;       ///< uSeconds
const uint16_t kIKEDAFreq 		    = 38000;  // Hz. (Guessing the most common frequency.)
const uint16_t kIKEDAOverhead     = 3; // 3 -> 1

#if SEND_IKEDA
// Function should be safe up to 64 bits.
/// Send a IKEDA formatted message.
/// Status: ALPHA / Untested.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
void IRsend::sendIKEDA(uint64_t data, uint16_t nbits, 
                        const uint16_t repeat) {
        sendGeneric(kIKEDAHdrMark, kIKEDAHdrSpace,
                    kIKEDABitMark, kIKEDAOneSpace,
                    kIKEDABitMark, kIKEDAZeroSpace,
                    kIKEDABitMark, kTESTSanyoAc88Gap,
                    data, nbits, kIKEDAFreq, false, repeat, kDutyDefault);
        space(kDefaultMessageGap);  // Make a guess at a post message gap.
}
#endif  // SEND_IKEDA

/*

ON+OFF timers at the same time - not possible.

0x010 = 16
0x100 = 256


                  CSUM      TEMP     |Emp   Fan    Flap   FP     FanOnly  MODE |Slp   OnTmr   OffTmr    Pow  Timer| Padding

    0x222311AA              00100010  0     01     0      0      0        11    0     0       0         1    0001  10101010

Power: Off, Mode: 0 (Auto), Temp: 20C, Fan: 0 (Auto), Swing(V): On, Turbo: Off, Sleep: Off  ---> Checksum generation test #1
                    219        32                         16                                1                        170+1+16+32 -> 219 Correct CS
    0xDB201001AA  11011011  00100000  0     00     1      0      0        00    0     0       0         0    0001  10101010

// 19C 
    0xF7192311AA  11110111  00011001  0     01     0      0      0        11    0     0       0         1    0001  10101010

ON; Mode: Fan; Fan: High; Temp: 16, FP: OFF, SLEEP:   OFF, FLAP: OFF; ON-timer enabled with 1h
    0x25162441AA  00100101  00010110  0     01     0      0      1        00    0     1       0         0    0001  10101010

ON; Mode: Fan; Fan: High; Temp: 16, FP: OFF, SLEEP:   OFF, FLAP: OFF; OFF-timer enabled with 12h (max)
    0x2016243CAA  00100000  00010110  0     01     0      0      1        00    0     0       1         1    1100  10101010 // 1100=12 (0xC)

ON; Mode: Fan; Fan: High; Temp: 16, FP: OFF, SLEEP:   OFF, FLAP: OFF; OFF-timer enabled with 5h
    0x19162435AA  00011001  00010110  0     01     0      0      1        00    0     0       1         1    0101  10101010 // 0001=0, 0101=5

ON; Mode: Fan; Fan: High; Temp: 16, FP: OFF, SLEEP:   OFF, FLAP: OFF; OFF-timer enabled with 1h
    0x15162431AA  00010101  00010110  0     01     0      0      1        00    0     0       1         1    0001  10101010


ON; Mode: Fan; Fan: High; Temp: 16, FP: OFF, SLEEP:   OFF, FLAP: OFF
    0xF5162411AA  11110101  00010110  0     01     0      0      1        00    0     0       0         1    0001  10101010

ON; Mode: Fan; Fan: Auto; Temp: 16, FP: OFF, SLEEP:   OFF, FLAP: OFF
    0xD5160411AA  11010101  00010110  0     00     0      0      1        00    0     0       0         1    0001  10101010

ON; Mode: Dry; Fan: Low; Temp: 16, FP: OFF, SLEEP:   OFF, FLAP: OFF
    0x33166211AA  00110011  00010110  0     11     0      0      0        10    0     0       0         1    0001  10101010

ON; Mode: Cool; Fan: Low; Temp: 16, FP: OFF, SLEEP:   OFF, FLAP: OFF
    0x32166111AA  00110010  00010110  0     11     0      0      0        01    0     0       0         1    0001  10101010

ON; Mode: Auto; Fan: Low; Temp: 16, FP: OFF, SLEEP:   OFF, FLAP: OFF
    0x31166011AA  00110001  00010110  0     11     0      0      0        00    0     0       0         1    0001  10101010

ON; Mode: Auto; Fan: Medium; Temp: 16, FP: OFF, SLEEP:   OFF, FLAP: OFF
    0x11164011AA  00010001  00010110  0     10     0      0      0        00    0     0       0         1    0001  10101010

ON; Mode: Auto; Fan: High; Temp: 16, FP: OFF, SLEEP:   OFF, FLAP: OFF
    0xF1162011AA  11110001  00010110  0     01     0      0      0        00    0     0       0         1    0001  10101010

ON; Mode: Auto; Fan: Auto; Temp: 16, FP: OFF, SLEEP:   OFF, FLAP: ON
    0xE1161011AA  11100001  00010110  0     00     1      0      0        00    0     0       0         1    0001  10101010

ON; Mode: Auto; Fan: Auto; Temp: 16, FP: OFF, SLEEP:   OFF, FLAP: OFF
    0xD1160011AA  11010001  00010110  0     00     0      0      0        00    0     0       0         1    0001  10101010

OFF->ON; Mode: Heat; Fan: High; Temp: 16, FP: OFF, SLEEP:   OFF, FLAP: OFF
		0xF4162311AA	11110100  00010110  0     01     0      0      0        11    0     0       0         1    0001  10101010
			
ON->OFF; Mode: Heat; Fan: High; Temp: 16, FP: OFF, SLEEP: OFF, FLAP: OFF
		0xE4162301AA  11100100  00010110  0     01     0      0      0        11    0     0       0         0    0001  10101010

ON;  Mode: Heat; Fan: High; Temp: 16, FP: OFF, SLEEP: ON, FLAP: OFF
    0x74162391AA  01110100  00010110  0     01     0      0      0        11    1     0       0         1    0001  10101010

ON;  Mode: Heat; Fan: High; Temp: 16, FP: ON, SLEEP: OFF, FLAP: OFF
    0xFC162B11AA  11111100  00010110  0     01     0      1      0        11    0     0       0         1    0001  10101010

ON;  Mode: Heat; Fan: High; Temp: 16, FP: ON, SLEEP: ON, FLAP: OFF
    0x7C162B91AA  01111100  00010110  0     01     0      1      0        11    1     0       0         1    0001  10101010

*/

/// Class constructor
/// @param[in] pin GPIO to be used when sending.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRIkedaAc::IRIkedaAc(const uint16_t pin, const bool inverted,
                         const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) { stateReset(); }

/// Reset the state of the remote to a known good state/sequence.
/// @see https://docs.google.com/spreadsheets/d/1IOxfKN2PPuIayC1UFZqkHrk7LAqH0AwUjgUn6dIrnzg
void IRIkedaAc::stateReset(void) {
  // 0xEF161011AA == ON,  Mode: Auto; Fan: Auto, Temp: 16 C, Flap: ON
  // 0xDF161001AA == OFF, Mode: Auto; Fan: Auto, Temp: 16 C, Flap: ON <-
  _.remote_state = 0x00161001AA; // checksum 0x00   
}

/// Set up hardware to be able to send a message.
void IRIkedaAc::begin(void) { _irsend.begin(); }

#if SEND_IKEDA
/// Send the current internal state as IR messages.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRIkedaAc::send(const uint16_t repeat) {
  _irsend.sendIKEDA(getRaw(), kIKEDABits, repeat);
}
#endif  // SEND_IKEDA

/// Calculate and set the checksum values for the internal state.
void IRIkedaAc::checksum(void) {
  uint8_t cs = sumBytes(_.remote_state, 4, 0, true);
  _.Checksum = cs;
  //DPRINTLN((String)"checksum(): " + _.Checksum);
}


uint8_t IRIkedaAc::calcChecksum(const uint64_t data) {
  uint8_t sum = sumBytes(data, 4, 0, true);
  //DPRINTLN((String)"calcChecksum(): " + sum);
  return sum;
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool IRIkedaAc::validChecksum(const uint64_t state) {
  // Validate the checksum of the given state.
  uint8_t bits = GETBITS64(state, 32, 8);
  //DPRINTLN((String)"validChecksum(): " + bits);
  return (bits == calcChecksum(state));
}

/// Get a copy of the internal state/code for this protocol.
/// @return The code for this protocol based on the current internal state.
uint64_t IRIkedaAc::getRaw(void) {
  checksum();  // Ensure correct checksum before sending.
  return _.remote_state;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] newState A valid code for this protocol.
void IRIkedaAc::setRaw(const uint64_t newState) { _.remote_state = newState; }


/// Set the requested power state of the A/C to on.
void IRIkedaAc::on(void) { setPower(true); }

/// Set the requested power state of the A/C to off.
void IRIkedaAc::off(void) { setPower(false); }

/// Change the power setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRIkedaAc::setPower(const bool on) {   _.Power = on; }

/// Get the value of the current power setting.
/// @return true, the setting is on. false, the setting is off.
bool IRIkedaAc::getPower(void) const { return _.Power; }

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t IRIkedaAc::getMode(void) const { return _.Mode; }

/// Set the operating mode of the A/C.
/// @param[in] mode The desired operating mode.
/// @note If we get an unexpected mode, default to AUTO.
void IRIkedaAc::setMode(const uint8_t mode) {
  switch (mode) {
    case ikedaAuto:
    case ikedaCool:
    case ikedaDry:
    case ikedaHeat:
      _.FanOnly = 0;
      _.Mode = mode;
      break;
    default:
      _.Mode = ikedaAuto;
      // Turn on FanOnty here?
  }
}

/// Convert a stdAc::opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRIkedaAc::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kCool: return ikedaCool;
    case stdAc::opmode_t::kHeat: return ikedaHeat;
    case stdAc::opmode_t::kDry:  return ikedaDry;
    default:                     return ikedaAuto;
  }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::opmode_t IRIkedaAc::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case ikedaCool:
      return stdAc::opmode_t::kCool;
    case ikedaHeat:
      return stdAc::opmode_t::kHeat;
    case ikedaDry:
      return stdAc::opmode_t::kDry;
    default:
      return stdAc::opmode_t::kAuto; // Fan (Quiet)
  }
}

const uint8_t kIkedaAcTempDelta = 6;   ///< Celsius to Native Temp difference.

/// Set the desired temperature.
/// @param[in] degrees The temperature in degrees celsius.
void IRIkedaAc::setTemp(const uint8_t degrees) {
  uint8_t temp = std::max((uint8_t)ikedaMinTemp, degrees);
  temp = std::min((uint8_t)ikedaMaxTemp, temp);
  _.Temperature = uint8ToBcd(temp); // Binary coded decimal
}

/// Get the current desired temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t IRIkedaAc::getTemp(void) const {
  return bcdToUint8(_.Temperature); // Binary coded decimal
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
void IRIkedaAc::setFan(const uint8_t speed) { _.Fan = speed; }

/// Get the current fan speed setting.
/// @return The current fan speed/mode.
uint8_t IRIkedaAc::getFan(void) const { return _.Fan; }

/// Convert a stdAc::fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRIkedaAc::convertFan(const stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin:
    case stdAc::fanspeed_t::kLow:    return ikedaFanLow;
    case stdAc::fanspeed_t::kMedium: return ikedaFanMed;
    case stdAc::fanspeed_t::kHigh:
    case stdAc::fanspeed_t::kMax:    return ikedaFanHigh;
    default:                         return ikedaFanAuto;
  }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] spd The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::fanspeed_t IRIkedaAc::toCommonFanSpeed(const uint8_t spd) {
  switch (spd) {
    case ikedaFanHigh:    return stdAc::fanspeed_t::kHigh;
    case ikedaFanMed:     return stdAc::fanspeed_t::kMedium;
    case ikedaFanLow:     return stdAc::fanspeed_t::kLow;
    default:              return stdAc::fanspeed_t::kAuto;
  }
}

/// Change the SwingV setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRIkedaAc::setSwingV(const bool on) { _.Flap = on; }

/// Get the value of the current SwingV setting.
/// @return true, the setting is on. false, the setting is off.
bool IRIkedaAc::getSwingV(void) const { return _.Flap; }

// Ikeda only has on/off for swing
/// Convert a stdAc::swingv_t enum into it's native setting.
/// @param[in] position The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRIkedaAc::convertSwingV(const stdAc::swingv_t position) {
  switch (position) {
    case stdAc::swingv_t::kHighest:
    case stdAc::swingv_t::kHigh:
    case stdAc::swingv_t::kMiddle:
    case stdAc::swingv_t::kLow:
    case stdAc::swingv_t::kLowest:
    case stdAc::swingv_t::kAuto:    return 1;
    // Native "Auto" doesn't have a good match for this in stdAc. (Case 2)
    // So we repurpose stdAc's "Off" (and anything else) to be Native Auto.
    default:                        return 0;
  }
}

/// Convert a native vertical swing postion to it's common equivalent.
/// @param[in] pos A native position to convert.
/// @return The common vertical swing position.
stdAc::swingv_t IRIkedaAc::toCommonSwingV(const uint8_t pos) {
  switch (pos) {
    case 1:                         return stdAc::swingv_t::kAuto;
    default:                        return stdAc::swingv_t::kOff;
  }
}

/// Change the Turbo setting. (Force run)
/// @param[in] on true, the setting is on. false, the setting is off.
void IRIkedaAc::setTurbo(const bool on) { _.Fp = on; }

/// Get the value of the current Turbo setting.
/// @return true, the setting is on. false, the setting is off.
bool IRIkedaAc::getTurbo(void) const { return _.Fp; }

/// Change the Sleep setting.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRIkedaAc::setSleep(const bool on) { _.Sleep = on; }

/// Get the value of the current Sleep setting.
/// @return true, the setting is on. false, the setting is off.
bool IRIkedaAc::getSleep(void) const { return _.Sleep; }

/// Change the Quiet mode (Mode: FAN ???).
/// @param[in] on true, the setting is on. false, the setting is off.
void IRIkedaAc::setQuiet(const bool on) { _.FanOnly = on; setMode(ikedaAuto); }

/// Get the value of the current Sleep setting.
/// @return true, the setting is on. false, the setting is off.
bool IRIkedaAc::getQuiet(void) const { return _.FanOnly; }

/// Convert the current internal state into its stdAc::state_t equivalent.
/// @param[in] prev A Ptr to the previous state.
/// @return The stdAc equivalent of the native settings.
stdAc::state_t IRIkedaAc::toCommon(const stdAc::state_t *prev) {
  stdAc::state_t result{};
  if (prev != NULL) {
    result = *prev;
  } else {
    result.protocol = decode_type_t::IKEDA;
    result.model = -1;  // Not supported.
    result.power = getPower();
    result.mode = toCommonMode(_.Mode);
    result.celsius = true;
    result.degrees = getTemp();
    result.fanspeed = toCommonFanSpeed(_.Fan);
    result.swingv = _.Flap ? stdAc::swingv_t::kAuto : stdAc::swingv_t::kOff;
    result.turbo = _.Fp;
    result.sleep = _.Sleep ? 0 : -1;
    result.clock = false;
    // Not supported.
    result.filter = false;
    result.swingh = stdAc::swingh_t::kOff;
    result.econo = false;
    result.light = false;
    result.quiet = false;
    result.beep = false;
    result.clean = false;
  }
  return result;
}

/// Convert the current internal state into a human readable string.
/// @return A human readable string.
String IRIkedaAc::toString(void) const {
  String result = "";
  result.reserve(115);
  result += addBoolToString(getPower(), kPowerStr, false);
  result += addModeToString(_.Mode, ikedaAuto, ikedaCool,
                            ikedaHeat, ikedaDry, ikedaFan);
  result += addTempToString(getTemp());
  result += addFanToString(_.Fan, ikedaFanHigh, ikedaFanLow,
                           ikedaFanAuto, ikedaFanAuto,
                           ikedaFanMed);
  result += addBoolToString(_.Flap, kSwingVStr);
  result += addBoolToString(_.Fp, kTurboStr);
  result += addBoolToString(_.Sleep, kSleepStr);
  return result;
}



#if DECODE_IKEDA
// Function should be safe up to 64 bits.
/// Decode the supplied IKEDA message.
/// Status: ALPHA / Untested.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.

// Returns 40 bits, MSB first, Arduino-IRremote (original library) returns LSB first
bool IRrecv::decodeIKEDA(decode_results *results, uint16_t offset,
                          const uint16_t nbits, const bool strict) {
  if (results->rawlen < 2 * nbits + kIKEDAOverhead - offset)
    return false;  // Too short a message to match.
  if (strict && nbits != kIKEDABits)
    return false;

  uint64_t data = 0;
  match_result_t data_result;

  // Header
  if (!matchMark(results->rawbuf[offset++], kIKEDAHdrMark))
    return false;
  if (!matchSpace(results->rawbuf[offset++], kIKEDAHdrSpace))
    return false;

  data_result = matchData(&(results->rawbuf[offset]), kIKEDABits,
              // onemark		onespace
              kIKEDABitMark, kIKEDAOneSpace,
              // zeromark       zerisoace
              kIKEDABitMark, kIKEDAZeroSpace,
              // tolerance, excess, msbfirst, expectlastspace
              kUseDefTol, kMarkExcess, false, true); // MSB disabled
  
  offset += data_result.used;

  if (data_result.success == false) return false;  // Fail
  data <<= IKEDA_BITS;  // Make room for the new bits of data.
  data |= data_result.data;

  // Footer
  if (!matchMark(results->rawbuf[offset++], kIKEDABitMark))
    return false;

  // Compliance
  if (strict && !IRIkedaAc::validChecksum(data)) return false;

  // Success
  results->decode_type = decode_type_t::IKEDA;
  results->bits = nbits;
  results->value = data;

  return true;
}
#endif  // DECODE_IKEDA