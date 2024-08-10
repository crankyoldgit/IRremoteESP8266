// Copyright 2024 Andrey Kravchenko (stellalupus)
/// @file
/// @brief Support for the Electrolux EACM protocols.

// Supports:
//   Brand: Electrolux,  Model: Electrolux EACM EZ/N3

#include "ir_Electrolux.h"
#include <algorithm>
#include "IRac.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "IRtext.h"
#include "IRutils.h"

// Constants
const uint16_t kElectroluxAcHdrMark = 2850;
const uint16_t kElectroluxAcBitMark = 752;
const uint16_t kElectroluxAcHdrSpace = 2700;
const uint16_t kElectroluxAcOneSpace = 2149;
const uint16_t kElectroluxAcZeroSpace = 756;
const uint16_t kElectroluxAcFreq = 38000;
const uint16_t kElectroluxAcOverhead = 3;

#if SEND_ELECTROLUX_AC
// Function should be safe up to 64 bits.
/// Send a Electrolux formatted message.
/// Status: ALPHA / Untested.
/// @param[in] data containing the IR command.
/// @param[in] nbits Nr. of bits to send. usually kElectroluxBits
/// @param[in] repeat Nr. of times the message is to be repeated.
void IRsend::sendElectroluxAc(
    const uint64_t data,
    const uint16_t nbits,
    const uint16_t repeat
) {
    enableIROut(kElectroluxAcFreq);
    for (uint16_t r = 0; r <= repeat; r++) {
        uint64_t send_data = data;
        // Header
        mark(kElectroluxAcHdrMark);
        space(kElectroluxAcHdrSpace);
        // Data Section
        sendData(kElectroluxAcBitMark, kElectroluxAcOneSpace,
                 kElectroluxAcBitMark, kElectroluxAcZeroSpace,
                 send_data, nbits, true);

        send_data >>= 32;
        // Footer
        mark(kElectroluxAcBitMark);

        // A 100% made up guess of the gap between messages.
        space(kDefaultMessageGap);
    }
}
#endif  // SEND_ELECTROLUX

#if DECODE_ELECTROLUX_AC
// Function should be safe up to 64 bits.
/// Decode the supplied Electrolux message.
/// Status: ALPHA / Untested.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
bool IRrecv::decodeElectroluxAc(
    decode_results *results,
    uint16_t offset,
    const uint16_t nbits,
    const bool strict
) {
    if (results->rawlen < 2 * nbits + kElectroluxAcOverhead - offset)
        return false;  // Too short a message to match.
    if (strict && nbits != kElectroluxAcBits)
        return false;

    uint64_t data = 0;

    // Header
    if (!matchMark(results->rawbuf[offset++], kElectroluxAcHdrMark))
        return false;
    if (!matchSpace(results->rawbuf[offset++], kElectroluxAcHdrSpace))
        return false;

    // Data Section #1
    // e.g. data_result.data = 0xED000004, nbits = 32
    match_result_t data_result = matchData(
        &(results->rawbuf[offset]), 32,
        kElectroluxAcBitMark, kElectroluxAcOneSpace,
        kElectroluxAcBitMark, kElectroluxAcZeroSpace);

    offset += data_result.used;
    if (data_result.success == false)
        return false;  // Fail
    data <<= 32;  // Make room for the new bits of data.
    data |= data_result.data;

    // Footer
    if (!matchMark(results->rawbuf[offset++], kElectroluxAcBitMark))
        return false;

    // Success
    results->decode_type = decode_type_t::ELECTROLUX_AC;
    results->bits = nbits;
    results->value = data;
    results->command = data & 0xFFF;
    results->address = 0;
    return true;
}
#endif  // DECODE_ELECTROLUX

/// Class constructor
/// @param[in] pin GPIO to be used when sending.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRElectroluxAc::IRElectroluxAc(
    const uint16_t pin,
    const bool inverted,
    const bool use_modulation
): _irsend(pin, inverted, use_modulation) {
    _ = ElectroluxAcProtocol();
    stateReset();
}

/// Reset the internal state to a fixed known good state.
/// @note The state is powered off.
void IRElectroluxAc::stateReset() { _.raw = 0xF3008005; }

#if SEND_ELECTROLUX_AC
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRElectroluxAc::send(const uint16_t repeat) {
    _irsend.sendElectroluxAc(getRaw(), kElectroluxAcBits, repeat);
}
#endif  // SEND_ELECTROLUX_AC

/// Set up hardware to be able to send a message.
void IRElectroluxAc::begin() { _irsend.begin(); }

/// Turn on/off the Power Airwell setting.
/// @param[in] on The desired setting state.
void IRElectroluxAc::setPower(const bool on) { _.Power = on; }

/// Get the power toggle setting from the internal state.
/// @return A boolean indicating the setting.
bool IRElectroluxAc::getPower() const { return _.Power; }

/// Turn on/off the fahrenheit temp mode.
/// @param[in] on The desired setting state.
void IRElectroluxAc::setTempModeFahrenheit(const bool on) {
    _.TempModeFahrenheit = on;
}

/// Get the fahrenheit temp mode set from the internal state.
/// @return A boolean indicating the setting.
bool IRElectroluxAc::getTempModeFahrenheit() const {
    return _.TempModeFahrenheit;
}

/// Set the temperature.
/// @param[in] degrees The temperature in celsius or fahrenheit.
void IRElectroluxAc::setTemp(const uint8_t degrees) {
    if (getTempModeFahrenheit()) {
        uint8_t temp = max(kElectroluxAcMinFTemp, degrees);
        temp = min(kElectroluxAcMaxFTemp, temp);
        _.Temp = (temp - kElectroluxAcMinFTemp);
    } else {
        uint8_t temp = max(kElectroluxAcMinTemp, degrees);
        temp = min(kElectroluxAcMaxTemp, temp);
#ifndef UNIT_TEST
        temp = map(temp, kElectroluxAcMinTemp, kElectroluxAcMaxTemp,
                   kElectroluxAcMinFTemp, kElectroluxAcMaxFTemp);
#else
    temp = temp * 9 / 5 + 32;
#endif
        _.Temp = temp - kElectroluxAcMinFTemp;
    }
}

/// Get the current temperature from the internal state.
/// @return The current temperature in Celsius.
uint8_t IRElectroluxAc::getTemp() const {
    if (getTempModeFahrenheit()) {
        return _.Temp + kElectroluxAcMinFTemp;
    } else {
#ifndef UNIT_TEST
        uint8_t temp = map(_.Temp + kElectroluxAcMinFTemp,
                           kElectroluxAcMinFTemp, kElectroluxAcMaxFTemp,
                           kElectroluxAcMinTemp, kElectroluxAcMaxTemp);
#else
    uint8_t temp = ((_.Temp + kElectroluxAcMinFTemp) - 32) * 5 / 9;
#endif
        return temp;
    }
}

/// Set the speed of the fan.
/// @param[in] speed The desired setting.
/// @note The speed is locked to Low when in Dry mode.
void IRElectroluxAc::setFan(const uint8_t speed) {
    _.Fan = (_.Mode == kElectroluxModeAuto)
                ? kElectroluxFanAuto
                : std::min(speed, kElectroluxFanAuto);
}

/// Get the current fan speed setting.
/// @return The current fan speed.
uint8_t IRElectroluxAc::getFan() const { return _.Fan; }

/// Set the desired operation mode.
/// @param[in] mode The desired operation mode.
void IRElectroluxAc::setMode(const uint8_t mode) {
    switch (mode) {
        case kElectroluxModeCool:
        case kElectroluxModeDry:
        case kElectroluxModeFan:
        case kElectroluxModeAuto:
            _.Mode = mode;
            break;
        default:
            _.Mode = kElectroluxModeAuto;
    }
}

/// Get the operating mode setting of the A/C.
/// @return The current operating mode setting.
uint8_t IRElectroluxAc::getMode() const { return _.Mode; }

/// Set the On/Off Timer time.
/// @param[in] nr_of_mins Number of minutes to set the timer to.
///  (< 60 is disable).
/// @note The A/C protocol only supports one hour increments.
void IRElectroluxAc::setOnOffTimer(const uint16_t nr_of_mins) {
    const uint8_t hours = std::min(
        static_cast<uint8_t>(nr_of_mins / 60),
        kElectroluxTimerMax);

    // The time can be changed in sleep mode, but doesn't set the flag.
    _.TimerEnabled = hours > 0;
    _.Timer = std::max(kElectroluxTimerMin, hours);  // Hours
}

/// Get the current On/Off Timer time.
/// @return The number of minutes it is set for. 0 means it's off.
/// @note The A/C protocol only supports one hour increments.
uint16_t IRElectroluxAc::getOnOffTimer() const {
    return _.TimerEnabled > 0 ? _.Timer * 60 : 0;
}

/// Set the Quiet setting of the A/C.
/// @param[in] on true, the setting is on. false, the setting is off.
void IRElectroluxAc::setQuiet(const bool on) { _.Quiet = on; }

/// Get the Quiet setting of the A/C.
/// @return true, the setting is on. false, the setting is off.
bool IRElectroluxAc::getQuiet() const { return _.Quiet; }

/// Get a copy of the internal state as a valid code for this protocol.
/// @return A valid code for this protocol based on the current internal state.
uint64_t IRElectroluxAc::getRaw() {
    checksum();  // Ensure correct settings before sending.
    return _.raw;
}

/// Set the internal state from a valid code for this protocol.
/// @param[in] state A valid code for this protocol.
void IRElectroluxAc::setRaw(const uint64_t state) { _.raw = state; }

/// Calculate the checksum for a given state.
/// @param[in] state The value to calc the checksum of.
/// @return The 4-bit checksum stored in a uint_8.
uint8_t IRElectroluxAc::calcChecksum(const uint64_t state) {
    uint32_t data = GETBITS64(
        state,
        kElectroluxAcChecksumSize + kElectroluxAcChecksumOffset,
        kElectroluxAcBits - 4);

    uint8_t result = 0;
    for (; data; data >>= 4)  // Add each nibble together.
        result += GETBITS8(data, 0, 4);
    return (result ^ 0xF) & 0xF;
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool IRElectroluxAc::validChecksum(const uint64_t state) {
    // Validate the checksum of the given state.
    return (GETBITS8(state, kElectroluxAcChecksumOffset,
                     kElectroluxAcChecksumSize) == calcChecksum(state));
}

/// Convert a stdAc::opmode_t enum into its native mode.
/// @param[in] mode The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRElectroluxAc::convertMode(const stdAc::opmode_t mode) {
    switch (mode) {
        case stdAc::opmode_t::kCool:
            return kElectroluxModeCool;
        case stdAc::opmode_t::kDry:
            return kElectroluxModeDry;
        case stdAc::opmode_t::kFan:
            return kElectroluxModeFan;
        default:
            return kElectroluxModeAuto;
    }
}

/// Convert a stdAc::fanspeed_t enum into it's native speed.
/// @param[in] speed The enum to be converted.
/// @return The native equivalent of the enum.
uint8_t IRElectroluxAc::convertFan(const stdAc::fanspeed_t speed) {
    switch (speed) {
        case stdAc::fanspeed_t::kMin:
        case stdAc::fanspeed_t::kLow:
            return kElectroluxFanLow;
        case stdAc::fanspeed_t::kMedium:
        case stdAc::fanspeed_t::kMediumHigh:
            return kElectroluxFanMedium;
        case stdAc::fanspeed_t::kHigh:
        case stdAc::fanspeed_t::kMax:
            return kElectroluxFanHigh;
        default:
            return kElectroluxFanAuto;
    }
}

/// Convert a native mode into its stdAc equivalent.
/// @param[in] mode The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::opmode_t IRElectroluxAc::toCommonMode(const uint8_t mode) {
    switch (mode) {
        case kElectroluxModeCool:
            return stdAc::opmode_t::kCool;
        case kElectroluxModeDry:
            return stdAc::opmode_t::kDry;
        case kElectroluxModeFan:
            return stdAc::opmode_t::kFan;
        default:
            return stdAc::opmode_t::kAuto;
    }
}

/// Convert a native fan speed into its stdAc equivalent.
/// @param[in] speed The native setting to be converted.
/// @return The stdAc equivalent of the native setting.
stdAc::fanspeed_t IRElectroluxAc::toCommonFanSpeed(const uint8_t speed) {
    switch (speed) {
        case kElectroluxFanHigh:
            return stdAc::fanspeed_t::kMax;
        case kElectroluxFanMedium:
            return stdAc::fanspeed_t::kMedium;
        case kElectroluxFanLow:
            return stdAc::fanspeed_t::kMin;
        default:
            return stdAc::fanspeed_t::kAuto;
    }
}

/// Convert the current internal state into its stdAc::state_t equivalent.
/// @param[in] prev Ptr to the previous state if required.
/// @return The stdAc equivalent of the native settings.
stdAc::state_t IRElectroluxAc::toCommon(const stdAc::state_t *prev) const {
    stdAc::state_t result{};
    // Start with the previous state if given it.
    if (prev != nullptr) {
        result = *prev;
    } else {
        // Set defaults for non-zero values that are not implicitly set for when
        // there is no previous state.
        // e.g. Any setting that toggles should probably go here.
        result.power = false;
    }
    result.protocol = ELECTROLUX_AC;
    result.power = _.Power;
    result.mode = toCommonMode(_.Mode);
    result.celsius = !getTempModeFahrenheit();
    result.degrees = getTemp();
    result.fanspeed = toCommonFanSpeed(_.Fan);
    // Not supported.
    result.model = -1;
    result.turbo = false;
    result.swingv = stdAc::swingv_t::kOff;
    result.swingh = stdAc::swingh_t::kOff;
    result.light = false;
    result.filter = false;
    result.econo = false;
    result.quiet = getQuiet();
    result.clean = false;
    result.beep = false;
    result.sleep = -1;
    result.clock = -1;
    return result;
}

/// Convert the internal state into a human readable string.
/// @return The current internal state expressed as a human readable String.
String IRElectroluxAc::toString() const {
    String result = "";
    result.reserve(120);  // Reserve heap for the string to reduce fragging.

    result += addBoolToString(
        _.Power,
        kPowerStr,
        false);

    result += addModeToString(
        _.Mode,
        kElectroluxModeAuto,
        kElectroluxModeCool,
        0xFF,
        kElectroluxModeDry,
        kElectroluxModeFan);

    result += addTempToString(
        getTemp(),
        !getTempModeFahrenheit());

    result += addFanToString(
        _.Fan,
        kElectroluxFanHigh,
        kElectroluxFanLow,
        kElectroluxFanAuto,
        kElectroluxFanAuto,
        kElectroluxFanMedium);

    result += addBoolToString(getQuiet(), kQuietStr);

    if (getPower()) {
        result += irutils::addLabeledString(
            irutils::minsToString(getOnOffTimer()),
            kOffTimerStr);
    } else {
        result += irutils::addLabeledString(
            irutils::minsToString(getOnOffTimer()),
            kOnTimerStr);
    }
    return result;
}

/// Calculate and set the checksum values for the internal state.
void IRElectroluxAc::checksum() {
    _.Sum = calcChecksum(_.raw);
}

/// Set the requested power state of the A/C to on.
void IRElectroluxAc::on() { setPower(true); }

/// Set the requested power state of the A/C to off.
void IRElectroluxAc::off() { setPower(false); }
