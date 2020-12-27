// Copyright 2009 Ken Shirriff
// Copyright 2017 David Conran

/// @file
/// @brief Support for Yamaha extensions to (Renesas) protocols.
/// NEC originally added from https://github.com/shirriff/Arduino-IRremote/
/// Yamaha protocol used in documents found around the web, such as the linked ir_vx81.xlsx spreadsheet
/// @see http://www.sbprojects.com/knowledge/ir/nec.php
/// @see https://planet.neeo.com/t/18kann?r=y7jn46

//Requires SEND_NEC to be available as an upstream capability for broadcasting.
#define __STDC_LIMIT_MACROS
#include "ir_NEC.h"
#include <stdint.h>
#include <algorithm>
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"


#if (SEND_YAMAHA)

/// Send a raw Yamaha formatted message.
/// This simply calls the sendNEC(), as the format is identical; only the actual code sent differs, in that it uses modified inversions in the final byte to add extra commands.
/// Status: STABLE / Known working.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated.
/// @note This protocol appears to have no header.
/// @see http://www.sbprojects.com/knowledge/ir/nec.php
void IRsend::sendYamaha(uint64_t data, uint16_t nbits, uint16_t repeat) {
  sendNEC(data, nbits, repeat)
}

/// Calculate the raw Yamaha data based on address and command.
/// Status: STABLE / Expected to work.
/// @param[in] address An address value. Yamaha utilises normal NEC address values.
/// @param[in] command A 16-bit command value, as defined by the published Yamaha codes. This includes the modified inversion of the command code; the modifications include a further inverted 0x1 for the second remote control, and a further inverted 0x80 for the alternate task.
/// @return A raw 32-bit NEC message suitable for use with `sendNEC()`.
/// @see http://www.sbprojects.com/knowledge/ir/nec.php
uint32_t IRsend::encodeYamaha(uint16_t address, uint16_t command) {
  // Yamaha commands are two bytes, with the second byte being a potentially modified inversion of the first.
  // sendNEC() sends MSB first, but protocol says this is LSB first.
  uint8_t commandHigh = (command>>8) & 0xFF;
  commandHigh = reverseBits(commandHigh, 8);
  uint8_t commandLow = command & 0xFF;
  commandLow = reverseBits(commandLow, 8);

  if (address > 0xFF) {                         // Is it Extended NEC?
    address = reverseBits(address, 16);
    return ((address << 16) + (commandHigh << 8) + commandLow);  // Extended.
  } else {
    address = reverseBits(address, 8);
    return (address << 24) + ((address ^ 0xFF) << 16) + (commandHigh << 8) + commandLow;  // Normal.
  }
}

/// Calculate the raw Yamaha data based on a code in the published Yamaha format.
/// Status: BETA / Expected to work with 32-bit codes; 16 and 8-bit codes are untested.
/// @param[in] yamahaCode A code as documented by Yamaha. This includes the address and command, along with any modifications in the lower byte.
/// @return A raw 32-bit NEC message suitable for use with `sendNEC()`.
/// @see https://planet.neeo.com/t/18kann?r=y7jn46
/// @see https://nobbin.net/tag/yamahanec-ir-codes/
uint32_t IRsend::encodeYamaha(uint32_t yamahaCode) {
	if(yamahaCode > 0xFFFFFF) {
	//32-bit Yamaha code
		uint8_t addressHigh = (yamahaCode >> 24) & 0xFF; //high 16 bytes are an NEC address. Parity is not checked or calculated, as the low byte is defined as part of the code.
		addressHigh = reverseBits(addressHigh, 8);
		uint8_t addressLow = (yamahaCode >> 16) & 0xFF;
		addressLow = reverseBits(addressLow, 8);
		uint16_t command = yamahaCode & 0xFFFF; // low 16 bits are the command, including address bit flip and command bit flip.
		uint8_t commandHigh = (command >> 8) & 0xFF;
		commandHigh = reverseBits(commandHigh, 8);
		uint8_t commandLow = command & 0xFF;
		commandLow = reverseBits(commandLow, 8);
	} else if(yamahaCode > 0xFFFF) {
		//24-bit Yamaha code; need to determine which one, ie 1-byte address, 2-byte command OR 2-byte address, 1-byte command
		//the first byte will always be part of the address
		uint8_t addressHigh = (yamahaCode >> 16) & 0xFF;
		addressHigh = reverseBits(addressHigh, 8);
		//the second byte could be either address or command
		uint8_t unknownValue = (yamahaCode >> 8) & 0xFF;
		if(addressHigh + unknownValue == 0xFF) {
			//in this case, the unknown value is a match for an inverse address, so we have a 2-byte address, 1-byte command
			uint8_t addressLow = unknownValue;
			addressLow = reverseBits(addressLow, 8);
			uint8_t command = yamahaCode & 0xFF;
			uint8_t commandHigh = command;
			commandHigh = reverseBits(commandHigh, 8);
			uint8_t commandLow = 0xFF - commandHigh;
		} else {
			//the addresses don't match up, so we have a 1-byte address, 2 byte command
			uint8_t addressLow = 0xFF - addressHigh;
			uint16_t command = yamahaCode & 0xFFFF; // low 16 bits are the command, including address bit flip and command bit flip.
			uint8_t commandHigh = (command >> 8) & 0xFF;
			commandHigh = reverseBits(commandHigh, 8);
			uint8_t commandLow = command & 0xFF;
			commandLow = reverseBits(commandLow, 8);
		}
	} else {
		//16-bit Yamaha command
		uint8_t addressHigh = (yamahaCode >> 8) & 0xFF;
		addressHigh = reverseBits(addressHigh, 8);
		uint8_t addressLow = addressHigh ^ 0xFF;
		uint8_t commandHigh = yamahaCode & 0xFF;
		commandHigh = reverseBits(commandHigh, 8);
		uint8_t commandLow = commandHigh ^ 0xFF;
	}		
	return (addressHigh << 24) + (addressLow << 16) + (commandHigh << 8) + commandLow;
}

/// Calculate the raw Yamaha data based on a command
/// Status: BETA / Expected to work, but could be incorrect per testing.
/// @param[in] address An address value. Yamaha utilises normal NEC address values. Maximum of two bytes; will automatically transform one-byte addresses.
/// @param[in] command A one-byte command. This does not include space for any alterations, as they are covered in the following parameters.
/// @param[in] altCommand Whether to use the alternate command option; certain commands utilise this. Defaults to false.
/// @param[in] remoteID The remote ID to emulate; determines part of the breaks in parity. Set false to use ID 1; set true to use ID 2. Defaults to false (ID 1).
/// @return A raw 32-bit message suitable for use with `sendYamaha()`.
uint32_t IRsend::encodeYamaha(uint16_t address, uint8_t command, bool altCommand = false, bool remoteID = false) {
	//All extra commands act upon the low byte of the command
	uint8_t commandHigh = command & 0xFF;
	commandHigh = reverseBits(commandHigh, 8);
	uint8_t commandLow = commandHigh ^ 0xFF; //true parity at this point
	if (remoteID == true) {
		commandLow = commandLow ^ 0x80; // alter high bit for remote ID, since we've already gone to LSB order
	}
	if (altCommand == true) {
		commandLow = commandLow ^ 0x01; // alter low bit, since we've already gone to LSB order
	}
	
  if (address > 0xFF) {                         // Is it Extended NEC?
    address = reverseBits(address, 16);
    return ((address << 16) + (commandHigh << 8) + commandLow);  // Extended.
  } else {
    address = reverseBits(address, 8);
    return (address << 24) + ((address ^ 0xFF) << 16) + (commandHigh << 8) + commandLow;  // Normal.
  }
}
#endif  // (SEND_YAMAHA)

#if (DECODE_YAMAHA)
/// Decode the supplied Yamaha message.
/// Status: STABLE / Known good.
/// @param[in,out] results Ptr to the data to decode & where to store the result
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return True if it can decode it, false if it can't.
/// @note NEC protocol has three variants/forms.
///   Normal:   an 8 bit address & an 8 bit command in 32 bit data form.
///             i.e. address + inverted(address) + command + inverted(command)
///   Extended: a 16 bit address & an 8 bit command in 32 bit data form.
///             i.e. address + command + inverted(command)
///   Yamaha:   an address (either normal or extended) and an 8-bit command, with potentially deliberately incorrect parity.
///             i.e. address + command + alteredinverted(command)
///   Repeat:   a 0-bit code. i.e. No data bits. Just the header + footer.
/// @see http://www.sbprojects.com/knowledge/ir/nec.php
bool IRrecv::decodeYamaha(decode_results *results, uint16_t offset,
                       const uint16_t nbits, const bool strict) {
  if (results->rawlen < kNecRptLength + offset - 1)
    return false;  // Can't possibly be a valid NEC message.
  if (strict && nbits != kNECBits)
    return false;  // Not strictly an NEC message.

  uint64_t data = 0;

  // Header - All NEC messages have this Header Mark.
  if (!matchMark(results->rawbuf[offset++], kNecHdrMark)) return false;
  // Check if it is a repeat code.
  // Disabled due to being a direct copy of the NEC code, which would be overriden and cause all repeats to show up as being Yamaha protocol. 
  // Since this is implemented as an extension of the NEC protocol, it is better to defer in this case to the NEC handling.
  /*if (matchSpace(results->rawbuf[offset], kNecRptSpace) &&
      matchMark(results->rawbuf[offset + 1], kNecBitMark) &&
      (offset + 2 <= results->rawlen ||
       matchAtLeast(results->rawbuf[offset + 2], kNecMinGap))) {
    results->value = kRepeat;
    results->decode_type = YAMAHA;
    results->bits = 0;
    results->address = 0;
    results->command = 0;
    results->repeat = true;
    return true;
  }*/

  // Match Header (cont.) + Data + Footer
  if (!matchGeneric(results->rawbuf + offset, &data,
                    results->rawlen - offset, nbits,
                    0, kNecHdrSpace,
                    kNecBitMark, kNecOneSpace,
                    kNecBitMark, kNecZeroSpace,
                    kNecBitMark, kNecMinGap, true)) return false;
  // Compliance
  // Calculate command and optionally enforce integrity checking.
  uint8_t command = (data & 0xFF00) >> 8;
  uint8_t command2 = data & 0xFF;
  // Command is sent twice, once as plain and then inverted.
  if ((command ^ 0xFF) != (command2)) { 
	  if (command ^ 0xFF) = ((command2) ^ 0x01) { // alternate command
	    command2 = ((command2) ^ 0x01);
	  }
	  else if (command ^ 0xFF) = ((command2) ^ 0x80) { // remoteID = 2
	    command2 = ((command2) ^ 0x80);
	  }
	  else if (command ^ 0xFF) = ((command2) ^ 0x81) { // alternate command and remoteID = 2
	    command2 = ((command2) ^ 0x81);
	  }
	  else {
        if (strict) return false;  // Command integrity failed in a way that is not known as being deliberate.
        command = 0;  // The command value isn't valid, so default to zero.
		command2 = 0;
	  }
  }

  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = YAMAHA;
  // NEC command and address are technically in LSB first order so the
  // final versions have to be reversed.
  results->command = reverseBits(command, 8)<<8 + reverseBits(command2, 8);
  // Normal NEC protocol has an 8 bit address sent, followed by it inverted.
  uint8_t address = (data & 0xFF000000) >> 24;
  uint8_t address_inverted = (data & 0x00FF0000) >> 16;
  if (address == (address_inverted ^ 0xFF))
    // Inverted, so it is normal NEC protocol.
    results->address = reverseBits(address, 8);
  else  // Not inverted, so must be Extended NEC protocol, thus 16 bit address.
    results->address = reverseBits((data >> 16) & UINT16_MAX, 16);
  return true;
}
#endif  // (DECODE_YAMAHA)
