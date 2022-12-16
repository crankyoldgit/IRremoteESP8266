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

ON:
21:21:56.033 -> Protocol  : IKEDA (Repeat)
21:21:56.033 -> Code      : 0x2242311AA (40 Bits)
21:21:56.033 -> No description
21:21:56.033 -> uint16_t rawData[83] = {8960, 2016,  620, 474,  618, 1206,  644, 478,  618, 1204,  644, 478,  618, 1204,  644, 474,  618, 1208,  644, 1178,  618, 474,  622, 474,  644, 474,  618, 1208,  644, 474,  618, 448,  648, 476,  618, 1204,  650, 1204,  618, 474,  618, 452,  644, 474,  618, 1210,  644, 474,  618, 474,  622, 474,  618, 474,  644, 1178,  648, 448,  618, 500,  622, 1204,  618, 474,  618, 452,  644, 474,  618, 1208,  644, 474,  618, 478,  618, 476,  618, 474,  648, 448,  644, 448,  618};  // IKEDA (Repeat) 2242311AA
21:21:56.080 -> uint64_t data = 0x2242311AA; 242311AA [0x2]

OFF:
21:21:59.337 -> Protocol  : IKEDA (Repeat)
21:21:59.337 -> Code      : 0xF2242301AA (40 Bits)
21:21:59.337 -> No description
21:21:59.337 -> uint16_t rawData[83] = {8964, 2012,  618, 474,  618, 1210,  644, 476,  618, 1204,  648, 474,  618, 1204,  644, 478,  618, 1204,  644, 1178,  622, 474,  618, 474,  644, 478,  618, 474,  618, 474,  622, 474,  644, 448,  618, 1234,  618, 1204,  618, 474,  622, 474,  644, 448,  644, 1210,  644, 448,  618, 474,  622, 500,  618, 474,  624, 1204,  618, 474,  618, 474,  622, 1204,  618, 474,  618, 474,  648, 448,  618, 1230,  618, 478,  618, 500,  618, 1204,  622, 1204,  644, 1178,  618, 1206,  622};  // IKEDA (Repeat) F2242301AA
21:21:59.383 -> uint64_t data = 0xF2242301AA; 242301AA [0xF2]

ON:
21:23:44.531 -> Protocol characteristics for a 50 us tick: 40, 179, 41, 12, 0, 10, 24
21:23:44.531 -> 
21:23:44.531 -> Protocol=PulseDistance Raw-Data=0x2 40 bits LSB first
21:23:44.531 -> Send with:
21:23:44.531 ->     uint32_t tRawData[]={0x242311AA, 0x2};
21:23:44.531 ->     IrSender.sendPulseDistanceWidthFromArray(38, 8950, 2050, 600, 1200, 600, 500, &tRawData[0], 40, PROTOCOL_IS_LSB_FIRST, <millisofRepeatPeriod>, <numberOfRepeats>);

OFF:
21:24:59.085 -> Protocol characteristics for a 50 us tick: 40, 179, 40, 13, 0, 10, 24
21:24:59.131 -> 
21:24:59.131 -> Protocol=PulseDistance Raw-Data=0xF2 40 bits LSB first
21:24:59.131 -> Send with:
21:24:59.131 ->     uint32_t tRawData[]={0x242301AA, 0xF2};
21:24:59.131 ->     IrSender.sendPulseDistanceWidthFromArray(38, 8950, 2000, 650, 1200, 650, 500, &tRawData[0], 40, PROTOCOL_IS_LSB_FIRST, <millisofRepeatPeriod>, <numberOfRepeats>);

// in MSB:
0x5588C4E8AF
0x55 = 85  // 55 = 85
0x88 = 136 // 88 = 136
0xC4 = 196 // 4C = 76
0xE8 = 232 // 8E = 142
0xAF = 175 // FA = 250

// in LSB:
0xF2 17 20 11 AA // AUTO, 17C, Fan HIGH
0xF3 17 21 11 AA // COOL, 17C, Fan HIGH
0xF4 17 22 11 AA // DRY,  17C, Fan HIGH
0xF6 17 24 11 AA // FAN,  17C, Fan HIGH
0xD5 17 03 11 AA // HEAT, 17C, Fan AUTO
0x35 17 63 11 AA // HEAT, 17C, Fan LOW
0x15 17 43 11 AA // HEAT, 17C, Fan MEDIUM

0x5  17 33 11 AA // HEAT, 17C, Fan HIGH, Flap->ON: First part: 3=FlapON_FanHIGH
0x25 17 53 11 AA // HEAT, 17C, Fan MEDIUM, Flap->ON: 5=FlapON_FanMEDIUM
0x45 17 73 11 AA // HEAT, 17C, Fan LOW, Flap->ON: 7=FlapON_FanLOW
0xE5 17 13 11 AA // HEAT, 17C, Fan AUTO, Flap->ON: 1=FlapON_FanAUTO

0x75 17 23 91 AA // HEAT, 17C, Fan HIGH, SleepMode->ON 	state[2][1]: 3 (heatSleep) + state[3][0]: 11->91 9=SleepMode
0x73 17 21 91 AA // COOL, 17C, Fan HIGH, SleepMode->ON 	state[2][1]: 1 (coolSleep) + state[3][0]: 11->91 9=SleepMode

0xFD 17 2B 11 AA // HEAT, 17C, Fan HIGH, FP->ON 		state[2][1] B(heatFP)
0xFB 17 29 11 AA // COOL, 17C, Fan HIGH, FP->ON 		state[2][1] 9(coolFP)

0x73 17 21 91 AA // COOL, 17C, Fan HIGH, SleepMode->ON 				state[2][1]: 1 + state[3][0]: 11->91 9=SleepMode
0x7B 17 29 91 AA // COOL, 17C, Fan HIGH, SleepMode->ON + FP->ON 	state[2][1]: 9(coolFP) + state[3][0]: 11->91 9=SleepMode
0x7D 17 2B 91 AA // HEAT, 17C, Fan HIGH, SleepMode->ON + FP->ON		state[2][1]: B(heatFP) + state[3][0]: 11->91 9=SleepMode



0xF5 17 23 11 AA // HEAT, 17C, Fan HIGH (for checksum check only)
0x25 17 23 41 AA // HEAT, 17C, Fan HIGH, On-Timer->1h	state[3][0] 4=On-Timer; state[3][1] 1-9, 0xA=10, 0xB=11 0xC=12
0x26 17 23 42 AA // HEAT, 17C, Fan HIGH, On-Timer->2h
0x27 17 23 43 AA // HEAT, 17C, Fan HIGH, On-Timer->3h
0x28 17 23 44 AA // HEAT, 17C, Fan HIGH, On-Timer->4h
0x29 17 23 45 AA // HEAT, 17C, Fan HIGH, On-Timer->5h
0x2A 17 23 46 AA // HEAT, 17C, Fan HIGH, On-Timer->6h
0x2B 17 23 47 AA // HEAT, 17C, Fan HIGH, On-Timer->7h
0x2C 17 23 48 AA // HEAT, 17C, Fan HIGH, On-Timer->8h
0x2D 17 23 49 AA // HEAT, 17C, Fan HIGH, On-Timer->9h
0x2E 17 23 4A AA // HEAT, 17C, Fan HIGH, On-Timer->10h
0x2F 17 23 4B AA // HEAT, 17C, Fan HIGH, On-Timer->11h
0x30 17 23 4C AA // HEAT, 17C, Fan HIGH, On-Timer->12h

0x15 17 23 31 AA // HEAT, 17C, Fan HIGH, Off-Timer->1h	state[3][0] 3=Off-Timer; state[3][1] 1-9, 0xA=10, 0xB=11 0xC=12
0x16 17 23 32 AA // HEAT, 17C, Fan HIGH, Off-Timer->2h
0x17 17 23 33 AA // HEAT, 17C, Fan HIGH, Off-Timer->3h
0x18 17 23 34 AA // HEAT, 17C, Fan HIGH, Off-Timer->4h
0x19 17 23 35 AA // HEAT, 17C, Fan HIGH, Off-Timer->5h
0x1A 17 23 36 AA // HEAT, 17C, Fan HIGH, Off-Timer->6h
0x1B 17 23 37 AA // HEAT, 17C, Fan HIGH, Off-Timer->7h
0x1C 17 23 38 AA // HEAT, 17C, Fan HIGH, Off-Timer->8h
0x1D 17 23 39 AA // HEAT, 17C, Fan HIGH, Off-Timer->9h
0x1E 17 23 3A AA // HEAT, 17C, Fan HIGH, Off-Timer->10h
0x1F 17 23 3B AA // HEAT, 17C, Fan HIGH, Off-Timer->11h
0x20 17 23 3C AA // HEAT, 17C, Fan HIGH, Off-Timer->12h

ON+OFF timers at the same time - not possible.

0x010 = 16
0x100 = 256


                  CSUM      TEMP      Emp   Fan    Flap   FP     FanOnly  MODE  Slp   OnTmr   OffTmr    Pow  Timer Padding

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



0xF5172311AA
11110101 00010111 00100011 00010001 10101010 // 40
0xF5 = 245  -> checksum state[0]: 0xF5 = 0x17 + 0x23 + 0x11 + 0xAA
	// checksum with first byte only 1 digit
	0x5 17 33 11 AA
	0101 00010111 00110011 00010001 10101010 // 36
	// if checksum is 1 digit (for example 0x2, 0x3, 0x4, 0x5, 0xD, etc =>> hex(100+hex2dec(DIGIT))
	-------

0x17 = 23	-> temperature: state[1] (in this case 17C)

0x23 =  	-> mode: state[2]
	First bit state[2][0]:
		- Fan speed (auto=0, high=2, medium=4, low=6) OR
		- Fan speed flap (1=Flap_Fan_Auto, 3=Flap_Fan_High, 5=Flap_Fan_Medium, 7=Flap_Fan_Low)
	Second bit state[2][1]:
		- Operating mode "normal" (0=auto, 1=cool, 2=dry, 3=heat, 4=fan)
		- Operating mode "fp" (B=heatFP, 9=coolFP) (Dry, Fan & Auto -> no FP & SLEEP modes)
		- Operating mode "sleep" (3=Heat_Sleep, 1=Cool_Sleep) + state[3][0] to 9
		- Combined "sleep" + "fp" (HEAT and COLD only): (B=heatFP_SLEEP, 9=coolFP_SLEEP) + state[3][0] to 9
		
0x11 = 17	-> state[3]
	First bit state[3][0]: 0=AC off; ON: 1=Normal, 9=Sleep mode(with or without FP), 3=Off-Timer, 4=On-Timer
	Second bit state[3][1]: on/off timer hours 1-9, 0xA=10, 0xB=11 0xC=12
	// ON+OFF timers at the same time - not possible.

0xAA = 170	-> state[4] always 0xAA (probably to ensure enough amount for checksum creation)
	--------
		245 = value of state[0] = state[1]+[2]+[3]+[4]

// checksum with first byte only 1 digit
0x5 17 33 11 AA
0101 00010111 00110011 00010001 10101010 // 36 digits
		// if 1 digit = hex(100+DIGIT)
0x5		= 5 	| 0x105 = 261
		-------
0x17	= 23
0x33	= 51
0x11	= 17
0xAA	= 170
		-------
			261

ON @ 27C, Auto, fan high: 0x2 27 20 11 AA
0x2 	= 2		| 0x102 = 258
0x27 	= 39
0x20	= 32
0x11	= 17
0xAA	= 170
		---------
			258


*/

/// Class constructor
/// @param[in] pin GPIO to be used when sending.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRIkedaAc::IRIkedaAc(const uint16_t pin, const bool inverted,
                         const bool use_modulation)
    : _irsend(pin, inverted, use_modulation) { stateReset(); }

/// Reset the state of the remote to a known good state/sequence.
/// @see https://docs.google.com/spreadsheets/d/1dYfLsnYvpjV-SgO8pdinpfuBIpSzm8Q1R5SabrLeskw/edit?ts=5f0190a5#gid=1050142776&range=A2:B2
void IRIkedaAc::stateReset(void) {
  // 0xEF241011AA == ON,  Mode: Auto; Fan: Auto, Temp: 24 C, Flap: ON <-
  // 0xDF241001AA == OFF, Mode: Auto; Fan: Auto, Temp: 24 C, Flap: ON
  //_.remote_state = 0xEF101011AA; // @ 16C
  _.remote_state = 0xAA111010EF;
}

/// Set up hardware to be able to send a message.
void IRIkedaAc::begin(void) { _irsend.begin(); }

#if SEND_IKEDA
/// Send the current internal state as IR messages.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRIkedaAc::send(const uint16_t repeat) {
  _irsend.sendIKEDA(getRaw(), kSanyoAc88StateLength, repeat);
}
#endif  // SEND_IKEDA

/// Get a copy of the internal state/code for this protocol.
/// @return The code for this protocol based on the current internal state.
uint64_t IRIkedaAc::getRaw(void) {
  //checksum();  // Ensure correct checksum before sending.
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

// /// Set the desired temperature.
// /// @param[in] degrees The temperature in degrees celsius.
// void IRIkedaAc::setTemp(const uint8_t degrees) {
//   uint8_t temp = std::max((uint8_t)ikedaMinTemp, degrees);
//   _.Temperature = std::min((uint8_t)ikedaMaxTemp, temp);
// }

/// Set the desired temperature.
/// @param[in] degrees The temperature in degrees celsius.
void IRIkedaAc::setTemp(const uint8_t degrees) {
  uint8_t temp = std::max((uint8_t)ikedaMinTemp, degrees);
  temp = std::min((uint8_t)ikedaMaxTemp, temp);
  _.Temperature = temp;
}

// /// Get the current desired temperature setting.
// /// @return The current setting for temp. in degrees celsius.
// uint8_t IRIkedaAc::getTemp(void) const { return _.Temperature; }

/// Get the current desired temperature setting.
/// @return The current setting for temp. in degrees celsius.
uint8_t IRIkedaAc::getTemp(void) const {
  return _.Temperature; // - kIkedaAcTempDelta;
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
    result.filter = false;
    result.turbo = _.Fp;
    result.sleep = _.Sleep ? 0 : -1;
    result.clock = false;
    // Not supported.
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
bool IRrecv::decodeIKEDA(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict) {
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

  // Success
  results->decode_type = decode_type_t::IKEDA;
  results->bits = nbits;
  results->value = data;

  return true;
}
#endif  // DECODE_IKEDA