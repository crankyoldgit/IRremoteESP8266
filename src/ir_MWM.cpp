// Copyright 2018 Brett T. Warden

/// @file
/// @brief Disney Made With Magic (MWM) Support
/// derived from ir_Lasertag.cpp
/// @see https://github.com/crankyoldgit/IRremoteESP8266/pull/557

// Supports:
//   Brand: Disney,  Model: Made With Magic (Glow With The Show) wand

#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

// Constants
const uint16_t kMWMMinSamples = 6;  // Msgs are >=3 bytes, bytes have >=2
                                    // samples
const uint16_t kMWMTick = 417;
const uint32_t kMWMMinGap = 30000;  // Typical observed delay b/w commands
const uint8_t kMWMTolerance = 0;    // Percentage error margin.
const uint16_t kMWMExcess = 0;      // See kMarkExcess.
const uint16_t kMWMDelta = 150;     // Use instead of Excess and Tolerance.
const uint8_t kMWMMaxWidth = 9;     // Maximum number of successive bits at a
                                    // single level - worst case
const int16_t kSpace = 1;
const int16_t kMark = 0;

#if SEND_MWM
/// Send a MWM packet/message.
/// Status: Implemented.
/// @param[in] data The message to be sent.
/// @param[in] nbytes The number of bytes of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
/// @note This protocol is 2400 bps serial, 1 start bit (mark),
///   1 stop bit (space), no parity
void IRsend::sendMWM(const uint8_t data[], const uint16_t nbytes,
                     const uint16_t repeat) {
  if (nbytes < 3) return;  // Shortest possible message is 3 bytes

  // Set 38kHz IR carrier frequency & a 1/4 (25%) duty cycle.
  // NOTE: duty cycle is not confirmed. Just guessing based on RC5/6 protocols.
  enableIROut(38, 25);

  for (uint16_t r = 0; r <= repeat; r++) {
    // Data
    for (uint16_t i = 0; i < nbytes; i++) {
      uint8_t byte = data[i];

      // Start bit
      mark(kMWMTick);

      // LSB first, space=1
      for (uint8_t mask = 0x1; mask; mask <<= 1) {
        if (byte & mask) {  // 1
          space(kMWMTick);
        } else {  // 0
          mark(kMWMTick);
        }
      }
      // Stop bit
      space(kMWMTick);
    }
    // Footer
    space(kMWMMinGap);
  }
}
#endif  // SEND_MWM

#if DECODE_MWM
/// Decode the supplied MWM message.
/// Status: Implemented.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return True if it can decode it, false if it can't.
/// @note This protocol is 2400 bps serial, 1 start bit (mark),
///   1 stop bit (space), no parity
/// @note Real MWM remotes transmit each command as an A-B-A' bundle of three
///   frames (a command, a status frame, then a repeat of the command)
///   separated by ~21-40ms gaps. Because that is shorter than the receiver's
///   kTimeout (default 50ms), the whole bundle lands in ONE capture buffer.
///   We decode every complete frame and return them concatenated in the one
///   result. Each frame is self-describing (its first byte declares its
///   length), so callers can split the bundle back apart:
///   length = (results->state[0] & 0x0f) + 3, then repeat from state+length.
///   Frames truncated by the end of the capture are dropped, not mis-decoded.
bool IRrecv::decodeMWM(decode_results *results, uint16_t offset,
                       const uint16_t nbits, const bool strict) {
  DPRINTLN("DEBUG: decodeMWM");

  // Compliance
  if (results->rawlen <= kMWMMinSamples + offset) {
    DPRINTLN("DEBUG: decodeMWM: too few samples");
    return false;
  }

  uint16_t used = 0;
  uint64_t data = 0;              // Bits of the current, not-yet-stored byte.
  uint16_t frame_bits = 0;        // Frames of the current message's framing.
  uint16_t frame_data_bits = 0;   // Data bits read in the current frame.
  uint16_t data_bits = 0;         // Data bits accepted & stored so far.
  uint16_t frame_bytes = 0;       // Bytes assembled in the current frame.
  uint8_t bits_per_frame = 10;    // 1 start + 8 data + 1 stop bit per byte.
  uint8_t frame_state[kStateSizeMax];  // Bytes assembled in the current frame.

  // No Header

  // Data
  // Frames of a bundle are read iteratively. Each frame's bytes are first
  // assembled in `frame_state` and only copied into `results` once the frame
  // has delivered the length its first byte promises (see `finish_frame`).
  // This keeps already-complete frames while dropping a final frame that the
  // capture truncated mid-frame.
  for (; offset < results->rawlen && data_bits < 8 * kStateSizeMax;
       frame_bits++) {
    // A frame's last byte's stop bit was stored at the % 10 == 9 framing
    // cell. If that was the frame's final byte (it delivered the length its
    // first byte declared), retire the frame now, at the start of the next
    // framing cell. A frame that hasn't reached its declared length yet just
    // passes this boundary; it's the middle of a message.
    if (frame_bits % bits_per_frame == 0 &&
        (frame_bytes != 0 || frame_data_bits != 0) &&
        finish_frame(results, &data_bits, data, frame_state, &frame_bytes,
                     frame_data_bits)) {
      data = 0;
      frame_data_bits = 0;
      frame_bytes = 0;
    }

    DPRINT("DEBUG: decodeMWM: offset = ");
    DPRINTLN(offset);
    uint16_t before = offset;
    int16_t level = getRClevel(results, &offset, &used, kMWMTick, kMWMTolerance,
                               kMWMExcess, kMWMDelta, kMWMMaxWidth);
    if (level < 0) goto done;

    switch (frame_bits % bits_per_frame) {
      case 0:
        // Start bit
        if (level == kSpace) {
          // A normal (short) space can't start a frame: invalid.
          if (offset != before || used != 0) goto done;
          // An inter-message (>=20ms) or trailing gap, or the end of the
          // buffer, was reported without advancing past it. Anything still
          // assembled here was cut short by the gap, so drop that frame. Then
          // skip the gap entry and retry this same framing cell for the next
          // frame's start mark.
          if (frame_bytes != 0 || frame_data_bits != 0) {
            if (!finish_frame(results, &data_bits, data, frame_state,
                              &frame_bytes, frame_data_bits))
              goto done;  // Truncated; keep the complete frames that were
                          // decoded before it.
            data = 0;
            frame_data_bits = 0;
            frame_bytes = 0;
          }
          offset++;
          frame_bits--;
          break;
        }
        if (level != kMark) {
          DPRINTLN("DEBUG: decodeMWM: framing error - invalid start bit");
          goto done;
        }
        // A new byte of the current frame begins here. The frame itself
        // carries on until it has delivered its declared length (or the
        // capture ends); it is retired, not here, but at a `finish_frame()`
        // call site above or at a gap.
        break;
      case 9:
        // Stop bit
        if (level != kSpace) {
          DPRINTLN("DEBUG: decodeMWM: framing error - invalid stop bit");
          goto done;
        }
        if (frame_bytes >= kStateSizeMax) goto done;  // Overflow safety.
        frame_state[frame_bytes++] = data & 0xFF;
        data = 0;
        break;
      default:
        // Data bits
        DPRINT("DEBUG: decodeMWM: Storing bit: ");
        DPRINTLN((level == kSpace));
        // Transmission is LSB-first, space=1
        data |= ((level == kSpace)) << 8;
        data >>= 1;
        frame_data_bits++;
        break;
    }
  }

  // Close out the final frame, which the end of the capture may have cut
  // short. A truncated frame is dropped, keeping the complete frames before it.
  if (frame_bytes != 0 || frame_data_bits != 0) {
    if (!finish_frame(results, &data_bits, data, frame_state, &frame_bytes,
                      frame_data_bits))
      goto done;
  }

done:
  // Footer (None)

  // Compliance
  DPRINT("DEBUG: decodeMWM: frame_bits = ");
  DPRINTLN(frame_bits);
  DPRINT("DEBUG: decodeMWM: data_bits = ");
  DPRINTLN(data_bits);
  results->bits = data_bits;
  if (data_bits < nbits) {
    DPRINT("DEBUG: decodeMWM: too few bits; expected ");
    DPRINTLN(nbits);
    return false;  // Less data than we expected.
  }

  uint16_t payload_length = 0;
  switch (results->state[0] & 0xf0) {
    case 0x90:
    case 0xf0:
      // Normal commands
      payload_length = results->state[0] & 0x0f;
      DPRINT("DEBUG: decodeMWM: payload_length = ");
      DPRINTLN(payload_length);
      break;
    default:
      if (strict) {
        // Show commands
        if (results->state[0] != 0x55 && results->state[1] != 0xAA) {
          return false;
        }
      }
      break;
  }
  if (data_bits < (payload_length + 3) * 8) {
    DPRINT("DEBUG: decodeMWM: too few bytes; expected ");
    DPRINTLN((payload_length + 3));
    return false;
  }

  // Success
  results->decode_type = MWM;
  results->repeat = false;
  return true;
}

/// Retire a frame that has just completed - or that a truncated capture cut
/// short. If the frame delivered exactly the data its first byte promised,
/// append it to `results`; otherwise drop it.
/// @param[in,out] results Ptr to the data & where to store the result
/// @param[in,out] data_bits Total data bits accepted & stored so far.
/// @param[in] data Bits of the current, not-yet-stored byte.
/// @param[in] frame_state The current frame's assembled bytes.
/// @param[in,out] frame_bytes The current frame's assembled byte count.
/// @param[in] frame_data_bits Data bits read in the current frame.
/// @return true if the frame was complete and has been appended; false if it
///   was truncated and dropped (its bytes were not added to `results`).
bool IRrecv::finish_frame(decode_results *results, uint16_t *data_bits,
                          const uint64_t data, uint8_t frame_state[],
                          uint16_t *frame_bytes,
                          const uint16_t frame_data_bits) {
  uint16_t needed = 0;
  if (*frame_bytes > 0) {
    switch (frame_state[0] & 0xf0) {
      case 0x90:
      case 0xf0:
        // Normal commands: total frame length = payload length + 3 bytes.
        needed = ((frame_state[0] & 0x0f) + 3) * 8;
        break;
      default:
        break;
    }
  }
  // A length-declared frame is only complete when it delivered exactly the
  // length it declared: fewer bits means the capture cut the frame short
  // mid-byte, more means a bogus trailing byte crept in from a truncated
  // capture's stray gap bits - both decode to garbage. A frame that reads
  // exactly its `needed` bits but is cropped on its last byte's stop bit
  // (crankyoldgit/IRremoteESP8266#2289) is handled by the pending byte logic
  // below and is still complete.
  if (needed && frame_data_bits != needed) {
    DPRINT("DEBUG: decodeMWM: truncated frame - skipping incomplete message (");
    DPRINT(frame_data_bits);
    DPRINT(" != ");
    DPRINTLN(needed);
    return false;  // Frame is incomplete; keep only the complete frames
                   // decoded before it.
  }
  // The capture may run out just after the last byte's final data bit but
  // before its stop bit: store the pending, fully-assembled byte.
  if (frame_data_bits > *frame_bytes * 8 && frame_data_bits % 8 == 0 &&
      *frame_bytes < kStateSizeMax)
    frame_state[(*frame_bytes)++] = data & 0xFF;
  // Append the frame to the results.
  // All complete frames are concatenated into `results->state`, capped at
  // kStateSizeMax bytes (= kHitachiAc2StateLength; the largest state buffer
  // the library provisions). In every bundle captured so far the final frame
  // (A') is an exact copy of the first (A), and the largest bundle observed
  // is a 33-byte command+status+command triple, so we have never seen
  // anything near the cap; truncating here is purely a safety bound against
  // corrupting results->state.
  for (uint8_t i = 0; i < *frame_bytes && *data_bits < 8 * kStateSizeMax;
       i++) {
    results->state[*data_bits / 8] = frame_state[i];
    *data_bits += 8;
  }
  return true;
}
#endif  // DECODE_MWM

// vim: et:ts=2:sw=2
