// Copyright 2022,2024 Daniel Wallner

/// @file
/// @brief Bang & Olufsen remote emulation
/// @see https://www.mikrocontroller.net/attachment/33137/datalink.pdf

// Supports:
//   Brand: Bang & Olufsen,  Model: Beomaster 3500/4500/5500/6500/7000
//   Brand: Bang & Olufsen,  Model: Beolink 1000 remote
//   Brand: Bang & Olufsen,  Model: Beolink 5000 remote
//   Brand: Bang & Olufsen,  Model: Beo4 remote

// This protocol is unusual in three ways:

// 1. The carrier frequency is 455 kHz

//   You can build your own receiver as Bang & Olufsen did (check old schematics) or use a TSOP7000.
//   Vishay stopped producing TSOP7000 a long time ago so you will likely only find counterfeits:
//   https://www.vishay.com/files/whatsnew/doc/ff_FastFacts_CounterfeitTSOP7000_Dec72018.pdf
//   It is also likely that you will need an oscilloscope to debug a counterfeit TSOP7000.
//   The specimen used to test this code was very noisy and had a very low output current.
//   A somewhat working fix was to put a 4n7 capacitor across the output and ground followed by a pnp emitter follower.
//   Other samples may require a different treatment.
//   This particular receiver also did receive lower frequencies but rather poorly and with a lower delay than usual.
//   This makes it hard to create a functional universal receiver by paralleling a TSOP7000 with another receiver.
//
//   If the transmitter is close enough receivers will still pick up the signal even if the modulation frequency is 200 kHz or so.

//  IOREF -----------------*---
//                         |
//                      R pull-up
//                         |
//                         *---- OUT
//                   PNP |v E
//  TSOP out -> ----*----|  B
//                  |    |\ C
//                C 4n7   |
//                  |     |
//  GND ------------*-----*---

// 2. Some remotes use two-way communication

//  One-way remotes such as Beolink 1000 use a 16-bit protocol (or 17 depending on how you count, see below)
//  Some remotes like Beolink 5000 use a different protocol where the remote sends 21-bits and receives
//  messages of different size of up to 58-bits.
//  To receive 58-bit messages, kRawBuf must be increased

// 3. A stream of messages can be sent back to back with a new message immediately following the previous stop space

//   The stop space is only 12.3 ms long which requires kTimeoutMs to be lower than this for a timeout before the next message.
//   The start space is 15.425 ms and this would then break a single message into three pieces:
//     The three AGC marks, the single stop mark, and the header and data part.
//   This seems to only happen on one-way remote messages with repeat like volume up/down.

// This can be handled in two ways:

// Alt 1: Partial message decode (default)

//   Set kTimeoutMs to 12 to reliably treat the start space as a gap between messages.
//   The start of a transmision will result in a dummy decode with 0 bits of data followed by the actual messages.
//   If the receiver is not resumed within a ms or so partial messages will be decoded.
//   Printing in the wrong place is very likely to break reception.
//   Make sure to check the number of bits to filter dummy and incomplete messages!

// Alt 1: Strict mode

//   Define BANG_OLUFSEN_STRICT and set kTimeoutMs to at least 16 to accomodate the unusually long start space.
//   This is the most robust mode but will only receive single messages and repeats will result in overflow.


// Note that the datalink document shows a message with 22 bits where the top bit is zero.
// It is assumed here that the first bit is an always zero start bit that should not be counted
// and this message is treated here as a 21-bit message.
// It is still possible to encode this bit if it would be needed as encoding includes one extra bit.

#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

// Constants
const uint8_t kBeoDataBits = 8;
const uint16_t kBeoPulseLengthT1 = 3125;  ///< uSeconds.
const uint16_t kBeoPulseLengthT2 = kBeoPulseLengthT1 * 2;  ///< uSeconds.
const uint16_t kBeoPulseLengthT3 = kBeoPulseLengthT1 * 3;  ///< uSeconds.
const uint16_t kBeoPulseLengthT4 = kBeoPulseLengthT1 * 4;  ///< uSeconds.
const uint16_t kBeoPulseLengthT5 = kBeoPulseLengthT1 * 5;  ///< uSeconds.
const uint16_t kBeoIRMark = 200;  ///< uSeconds.
const uint16_t kBeoDatalinkMark = kBeoPulseLengthT1 / 2;  ///< uSeconds.
const uint32_t kBeoFreq = 455000;  // kHz.

//#define BANG_OLUFSEN_STRICT
//#define BANG_OLUFSEN_LOCAL_DEBUG
//#define BANG_OLUFSEN_LOCAL_TRACE
//#define BANG_OLUFSEN_CHECK_MODULATION
//#define BANG_OLUFSEN_ACCEPT_NON_STRICT_ONE_START_BIT

#ifdef BANG_OLUFSEN_LOCAL_DEBUG
#undef DPRINT
#undef DPRINTLN
#define DPRINT(x) do { Serial.print(x); } while (0)
#define DPRINTLN(x) do { Serial.println(x); } while (0)
#endif

#ifdef BANG_OLUFSEN_LOCAL_TRACE
#define TPRINT(...) Serial.print(__VA_ARGS__)
#define TPRINTLN(...) Serial.println(__VA_ARGS__)
#else
#define TPRINT(...) void()
#define TPRINTLN(...) void()
#endif

#if SEND_BANG_OLUFSEN
void IRsend::sendBangOlufsen(const uint64_t data,
                             const uint16_t nbits,
                             const uint16_t repeat)
{
    for (uint_fast8_t i = 0; i < repeat + 1; ++i) {
        sendBangOlufsenRaw(data, nbits + 1, false, i != 0);
    }
}

void IRsend::sendBangOlufsenRaw(const uint64_t rawData,
                                const uint16_t bits,
                                const bool datalink,
                                const bool backToBack)
{
    uint16_t markLength = datalink ? kBeoDatalinkMark : kBeoIRMark;

    const uint32_t modulationFrequency = kBeoFreq;
    enableIROut(modulationFrequency, datalink ? kDutyMax : kDutyDefault);

#ifdef BANG_OLUFSEN_CHECK_MODULATION
    // Don't send, just measure during 65 ms
    Serial.print("Clock frequency: ");
    Serial.print(ESP.getCpuFreqMHz());
    unsigned long startTime = micros();
    uint16_t testTime = -1;
    uint16_t pulses = mark(testTime);
    uint32_t timespent = micros() - startTime;
    uint32_t measuredFrequency = (pulses * 1000000ULL + (timespent >> 2)) / timespent;
    Serial.print("Modulation frequency: ");
    Serial.print(measuredFrequency);
    Serial.print(" (Target: ");
    Serial.print(modulationFrequency);
    Serial.println(")");
    Serial.print("Mark time: ");
    Serial.print(timespent);
    Serial.print(" (Target: ");
    Serial.print(testTime);
    Serial.println(")");
    Serial.print("Number of pulses: ");
    Serial.print(pulses);
    Serial.print(" (Target: ");
    Serial.print(uint32_t((testTime * uint64_t(modulationFrequency) + 500000UL) / 1000000UL));
    Serial.println(")");
    return;
#endif

    // AGC / Start
    if (!backToBack) {
        mark(markLength);
    }
    space(kBeoPulseLengthT1 - markLength);
    mark(markLength);
    space(kBeoPulseLengthT1 - markLength);
    mark(markLength);
    space(kBeoPulseLengthT5 - markLength);

    bool lastBit = true;

    // Header / Data
    uint64_t mask = 1UL << (bits - 1);
    for (; mask; mask >>= 1) {
        if (lastBit && !(rawData & mask)) {
            mark(markLength);
            space(kBeoPulseLengthT1 - markLength);
            lastBit = false;
        } else if (!lastBit && (rawData & mask)) {
            mark(markLength);
            space(kBeoPulseLengthT3 - markLength);
            lastBit = true;
        } else {
            mark(markLength);
            space(kBeoPulseLengthT2 - markLength);
        }
    }

    // Stop
    mark(markLength);
    space(kBeoPulseLengthT4 - markLength);
    mark(markLength);
}

#endif  // SEND_BANG_OLUFSEN

#if DECODE_BANG_OLUFSEN
static bool matchBeoLength(IRrecv *irr, uint16_t measuredTicks, uint16_t desiredMicros)
{
    return irr->match(measuredTicks, desiredMicros, 0, kBeoDatalinkMark);
}

static bool matchBeoMark(IRrecv *irr, uint32_t measuredTicks, uint32_t desiredMicros)
{
    return irr->matchMark(measuredTicks, desiredMicros, 65, 40);
}

bool IRrecv::decodeBangOlufsen(decode_results *results,
                               uint16_t offset,
                               const uint16_t nbits,
                               const bool strict) {
#ifdef BANG_OLUFSEN_STRICT
    if (results->rawlen - offset < 43) { // 16 bits minimun
#else
    if (results->rawlen - offset != 5 && results->rawlen - offset < 35) { // 16 bits minimun
#endif
        return false;
    }

#if !defined(BANG_OLUFSEN_STRICT) && (defined(BANG_OLUFSEN_LOCAL_DEBUG) || defined(BANG_OLUFSEN_LOCAL_TRACE))
    if (results->rawlen - offset == 5) {
        // Short circuit when debugging to avoid spending too much time printing and then miss the actual message
        results->decode_type = BANG_OLUFSEN;
        results->value = 0;
        results->address = 0;
        results->command = 0;
        results->bits = 0;
        return true;
    }
#endif

    uint16_t protocolMarkLength = 0;
    uint8_t lastBit = 1;
    uint8_t pulseNum = 0;
    uint8_t bits = 0;
    uint64_t receivedData = 0;
    bool complete = false;

    for (uint8_t rawPos = offset; rawPos < results->rawlen; rawPos += 2) {
        uint16_t markLength = results->rawbuf[rawPos];
        uint16_t spaceLength = rawPos + 1 < results->rawlen ? results->rawbuf[rawPos + 1] : 0;

        if (pulseNum == 0) {
            TPRINT("Pre space: ");
            TPRINT(results->rawbuf[rawPos - 1] * kRawTick);
            TPRINT(" raw len: ");
            TPRINTLN(results->rawlen);
        }

        TPRINT(pulseNum);
        TPRINT(" ");
        TPRINT(markLength * kRawTick);
        TPRINT(" ");
        TPRINT(spaceLength * kRawTick);
        TPRINT(" (");
        TPRINT((markLength + spaceLength) * kRawTick);
        TPRINTLN(") ");

#ifndef BANG_OLUFSEN_STRICT
        if (bits == 0 && rawPos + 1 == results->rawlen) {
            TPRINTLN(": Jump to end");
            pulseNum = 3;
            complete = true;
            continue;
        }
#endif

        // Check start
        if (pulseNum < 3) {
            if (protocolMarkLength == 0) {
                if (matchBeoMark(this, markLength, kBeoIRMark)) {
                    protocolMarkLength = kBeoIRMark;
                }
                if (matchBeoMark(this, markLength, kBeoDatalinkMark)) {
                    protocolMarkLength = kBeoDatalinkMark;
                }
                if (!protocolMarkLength) {
                    DPRINTLN("DEBUG: decodeBangOlufsen: Start mark length 1 is wrong");
                    return false;
                }
            } else {
                if (!matchBeoMark(this, markLength, protocolMarkLength)) {
                    DPRINTLN("DEBUG: decodeBangOlufsen: Start mark length is wrong");
                    return false;
                }
            }
#ifdef BANG_OLUFSEN_STRICT
            if (!matchBeoLength(this, markLength + spaceLength, (pulseNum == 2) ? kBeoPulseLengthT5 : kBeoPulseLengthT1)) {
                DPRINT("DEBUG: decodeBangOlufsen: Start length is wrong: ");
                DPRINT(markLength * kRawTick);
                DPRINT("/");
                DPRINT(spaceLength * kRawTick);
                DPRINT(" (");
                DPRINT(pulseNum);
                DPRINTLN(")");
                return false;
            }
#else
            if (matchSpace(results->rawbuf[rawPos - 1], kBeoPulseLengthT5 - kBeoIRMark)) {
                // Jump to bits
                TPRINTLN(": Jump to bits 1");
                pulseNum = 2;
                rawPos = offset - 2;
            } else if (matchBeoLength(this, markLength + spaceLength, kBeoPulseLengthT1)
#ifdef BANG_OLUFSEN_ACCEPT_NON_STRICT_ONE_START_BIT
                    // Decode messages that have a top/start bit that isn't zero
                       || matchBeoLength(this, markLength + spaceLength, kBeoPulseLengthT2)
#endif
                ) {
                if (pulseNum == 0 && rawPos + 3 < results->rawlen) {
                    // Check that we are not in start
                    uint16_t nextMarkLength = results->rawbuf[rawPos + 2];
                    uint16_t nextSpaceLength = results->rawbuf[rawPos + 3];
                    if (!(matchBeoLength(this, nextMarkLength + nextSpaceLength, kBeoPulseLengthT1) &&
                          matchBeoLength(this, markLength + spaceLength, kBeoPulseLengthT1))) {
                        // This can be a false positive if we started in the middle of the message!
                        TPRINTLN(": Jump to bits 2");
                        pulseNum = 2;
                        rawPos = offset - 2;
                    }
                } else if (pulseNum == 2) {
                    DPRINTLN("DEBUG: decodeBangOlufsen: Start sequence is wrong");
                    return false;
                }
            } else if (matchBeoLength(this, markLength + spaceLength, kBeoPulseLengthT5)) {
                // Jump to bits
                pulseNum = 2;
                TPRINTLN(": Jump to bits 3");
            } else {
                DPRINT("DEBUG: decodeBangOlufsen: Start length is wrong: ");
                DPRINT(markLength * kRawTick);
                DPRINT("/");
                DPRINT(spaceLength * kRawTick);
                DPRINT(" (");
                DPRINT(pulseNum);
                DPRINTLN(")");
                return false;
            }
#endif
        }
        // Decode header / data
        else {
            if (!matchBeoMark(this, markLength, protocolMarkLength)) {
                DPRINTLN("DEBUG: decodeBangOlufsen: Mark length is wrong");
                return false;
            }
            if (complete) {
#ifdef BANG_OLUFSEN_STRICT
                if (rawPos + 1 != results->rawlen) {
                    DPRINTLN("DEBUG: decodeBangOlufsen: Extra data");
                    return false;
                }
#endif
                break;
            }
            if (bits > kBeoDataBits) {
                // Check for stop
                if (matchBeoLength(this, markLength + spaceLength, kBeoPulseLengthT4)) {
                    if (rawPos + 2 < results->rawlen) {
                        complete = true;
                        continue;
                    }
                    DPRINTLN("DEBUG: decodeBangOlufsen: Incomplete");
                    return false;
                }
            }
            if (lastBit == 0 && matchBeoLength(this, markLength + spaceLength, kBeoPulseLengthT3)) {
                lastBit = 1;
            } else if (lastBit == 1 && matchBeoLength(this, markLength + spaceLength, kBeoPulseLengthT1)) {
                lastBit = 0;
#ifndef BANG_OLUFSEN_STRICT
            } else if (rawPos + 1 == results->rawlen && spaceLength == 0) {
                DPRINTLN("Ignoring missing stop");
                complete = true;
                continue;
#endif
            } else if (!matchBeoLength(this, markLength + spaceLength, kBeoPulseLengthT2)) {
                DPRINT("DEBUG: decodeBangOlufsen: Length ");
                DPRINT((markLength + spaceLength) * kRawTick);
                DPRINTLN(" is wrong");
                return false;
            }
            receivedData <<= 1;
            receivedData |= lastBit;
            ++bits;
            TPRINT("Bits ");
            TPRINT(bits);
            TPRINT(" ");
            TPRINT(uint32_t(receivedData >> kBeoDataBits), HEX);
            TPRINT(" ");
            TPRINTLN(uint8_t(receivedData & ((1 << kBeoDataBits) - 1)), HEX);
        }

        ++pulseNum;
    }

    if (!complete) {
        DPRINTLN("DEBUG: decodeBangOlufsen: Not enough bits");
        return false;
    }

    if (strict && bits < kBangOlufsenBits + 1)
      return false;
    if (strict && bits != nbits + 1)
      return false;

    results->decode_type = BANG_OLUFSEN;
    results->value = receivedData;
    results->address = receivedData >> kBeoDataBits;  // header bits
    results->command = receivedData & ((1 << kBeoDataBits) - 1);    // lower 8 bits
    results->bits = bits ? bits - 1 : 0;

    return true;
}

#endif  // DECODE_BANG_OLUFSEN
