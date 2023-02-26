// Copyright 2020 David Conran
/// @file
/// @brief Delonghi based protocol.

#include "ir_RadiatorDelonghi.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "IRtext.h"
#include "IRutils.h"
#include <algorithm>

#define DEBUG_LEVEL_DELONGHI  0

const uint16_t kDelonghiRadiatorHdrMark = 9400;
const uint16_t kDelonghiRadiatorHdrSpace = 4000;
const uint16_t kDelonghiRadiatorBitMark = 520;
const uint16_t kDelonghiRadiatorOneSpace = 1559;
const uint16_t kDelonghiRadiatorZeroSpace = 531;
const uint32_t kDelonghiRadiatorGap = kDefaultMessageGap; // A totally made-up guess.
const uint16_t kDelonghiRadiatorFreq = 38000;             // Hz. (Guess: most common frequency.)
const uint16_t kDelonghiRadiatorOverhead = 3;


  const static uint8_t CrcTable[256] = {
    0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83, 0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41,
    0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e, 0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc,
    0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0, 0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62,
    0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d, 0x7c, 0x22, 0xc0, 0x9e, 0x1d, 0x43, 0xa1, 0xff,
    0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5, 0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07,
    0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58, 0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a,
    0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6, 0xa7, 0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24,
    0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b, 0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9,
    0x8c, 0xd2, 0x30, 0x6e, 0xed, 0xb3, 0x51, 0x0f, 0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd,
    0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e, 0xcc, 0x92, 0xd3, 0x8d, 0x6f, 0x31, 0xb2, 0xec, 0x0e, 0x50,
    0xaf, 0xf1, 0x13, 0x4d, 0xce, 0x90, 0x72, 0x2c, 0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee,
    0x32, 0x6c, 0x8e, 0xd0, 0x53, 0x0d, 0xef, 0xb1, 0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73,
    0xca, 0x94, 0x76, 0x28, 0xab, 0xf5, 0x17, 0x49, 0x08, 0x56, 0xb4, 0xea, 0x69, 0x37, 0xd5, 0x8b,
    0x57, 0x09, 0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4, 0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16,
    0xe9, 0xb7, 0x55, 0x0b, 0x88, 0xd6, 0x34, 0x6a, 0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8,
    0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7, 0xb6, 0xe8, 0x0a, 0x54, 0xd7, 0x89, 0x6b, 0x35
  };

#if SEND_DELONGHI_RADIATOR
/// Send a Delonghi A/C formatted message.
/// Status: STABLE / Reported as working on a real device.
/// @param[in] data The message to be sent.
/// @param[in] nbits The number of bits of message to be sent.
/// @param[in] repeat The number of times the command is to be repeated, must be 0
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1096
void IRsend::sendDelonghiRadiator(const uint64_t data, const uint16_t nbits, const uint16_t repeat)
{
  sendGeneric(kDelonghiRadiatorHdrMark, kDelonghiRadiatorHdrSpace,
              kDelonghiRadiatorBitMark, kDelonghiRadiatorOneSpace,
              kDelonghiRadiatorBitMark, kDelonghiRadiatorZeroSpace,
              kDelonghiRadiatorBitMark, kDelonghiRadiatorGap,
              data, nbits, kDelonghiRadiatorFreq, true, // MSB First.
              repeat, kDutyDefault);
}
#endif // SEND_DELONGHI_RADIATOR


#if DECODE_DELONGHI_RADIATOR
/// Decode the supplied Delonghi A/C message.
/// Status: STABLE / Expected to be working.
/// @param[in,out] results Ptr to the data to decode & where to store the decode
///   result.
/// @param[in] offset The starting index to use when attempting to decode the
///   raw data. Typically/Defaults to kStartOffset.
/// @param[in] nbits The number of data bits to expect.
/// @param[in] strict Flag indicating if we should perform strict matching.
/// @return A boolean. True if it can decode it, false if it can't.
/// @see https://github.com/crankyoldgit/IRremoteESP8266/issues/1096
bool IRrecv::decodeDelonghiRadiator(decode_results *results, uint16_t offset, const uint16_t nbits, const bool strict)
{
#if DEBUG_LEVEL_DELONGHI > 1
  Serial.printf("Enter decodeDelonghiRadiator, offset=%d, nbits=%d, strict=%d\n", offset, nbits, strict);
  Serial.printf("rawlen=%d,  kDelonghiRadiatorOverhead=%d, offset=%d\n", results->rawlen, kDelonghiRadiatorOverhead, offset);
#endif
  if (results->rawlen < 2 * nbits + kDelonghiRadiatorOverhead - offset)
  {
#if DEBUG_LEVEL_DELONGHI > 0
    Serial.printf("Delonghi radiator decoding fails : not enough data, received %d, expecting at least %d\n", results->rawlen, 2 * nbits + kDelonghiRadiatorOverhead - offset);
#endif
    return false; // Too short a message to match.

  }
  if (strict && nbits != kDelonghiRadiatorBits)
  {
#if DEBUG_LEVEL_DELONGHI > 0
    Serial.printf("Delonghi radiator decoding fails : nbits=%d, expecting exactly %d\n", nbits, kDelonghiRadiatorBits);
#endif
    return false;

  }

  uint64_t data = 0;
  // Use specific match (at least for now)
  uint16_t i = results->rawlen - 2*nbits;
  for (; i < results->rawlen; i += 2) 
  {
    uint32_t usecs;
    usecs = results->rawbuf[i] * kRawTick;
    data <<= 1;
    if ( usecs > 1000 )
    {
      data |= 1;
    }
  }

#if DEBUG_LEVEL_DELONGHI > 1
  Serial.printf("Delonghi radiator decode return data %016llx\n", data);
#endif
  // Compliance
  if (strict && !IRDelonghiRadiator::validChecksum(data))
    {
#if DEBUG_LEVEL_DELONGHI > 0
      DelonghiRadiatorProtocol dp;
      dp.raw = data;
      Serial.printf("Delonghi radiator decoding fails because of checksum, received data %016llx, checksum %02x expected %02x\n", dp.raw, dp.WordHour.CheckSum, IRDelonghiRadiator::calcChecksum(data));
#endif
      return false;    
    }
  if ( results->rawlen == 82 )
  {
    //Serial.printf("Trying matchGeneric, rawlen=%d without header\n", results->rawlen);
    // Now try Match Header + Data + Footer
    if (!matchGeneric(results->rawbuf + offset, &data,
                      results->rawlen - offset, nbits,
                      0, 0,
                      kDelonghiRadiatorBitMark, kDelonghiRadiatorOneSpace,
                      kDelonghiRadiatorBitMark, kDelonghiRadiatorZeroSpace,
                      kDelonghiRadiatorBitMark, kDelonghiRadiatorGap, true,
                      _tolerance, kMarkExcess, true))
                      {
                        DPRINTLN("match generic fails");
                        return false;
                      }
  }
  else
  {
    //Serial.printf("Trying matchGeneric, rawlen=%d with header\n", results->rawlen);
    // Now try Match Header + Data + Footer
    if (!matchGeneric(results->rawbuf + offset, &data,
                      results->rawlen - offset, nbits,
                      kDelonghiRadiatorHdrMark, kDelonghiRadiatorHdrSpace,
                      kDelonghiRadiatorBitMark, kDelonghiRadiatorOneSpace,
                      kDelonghiRadiatorBitMark, kDelonghiRadiatorZeroSpace,
                      kDelonghiRadiatorBitMark, kDelonghiRadiatorGap, true,
                      _tolerance, kMarkExcess, true))
                      {
                        DPRINTLN("match generic fails");
                        return false;
                      }

  }
  // Success
  results->decode_type = decode_type_t::DELONGHI_RADIATOR;
  results->bits = nbits;
  results->value = data;
  results->command = 0;
  results->address = 0;
  return true;
}
#endif // DECODE_DELONGHI_RADIATOR

/// Class constructor.
/// @param[in] pin GPIO to be used when sending.
/// @param[in] inverted Is the output signal to be inverted?
/// @param[in] use_modulation Is frequency modulation to be used?
IRDelonghiRadiator::IRDelonghiRadiator(const uint16_t pin, const bool inverted, const bool use_modulation) : _irsend(pin, inverted, use_modulation) 
{ 
  stateReset(); 
}

/// Set up hardware to be able to send a message.
void IRDelonghiRadiator::begin(void) { 
  stateReset(); 
  _irsend.begin();
}

void IRDelonghiRadiator::stateReset(void)
{
  for ( int i = 0; i < 10; i++ )
  {
    FullSequence[i].raw = 0;
    FullSequence[i].WordMode.Type = 0xFF;     //  Unknown type, also make the crc fails !
  }
  ReceiveSequence = 0;
}

#if SEND_DELONGHI_RADIATOR
/// Send the current internal state as an IR message.
/// @param[in] repeat Nr. of times the message will be repeated.
void IRDelonghiRadiator::FullSequenceSend()
{
  for (int i = 0; i < 10; i++ )
  {
    // Serial.printf("Envoi %010llx (%d bit)\n",getRaw(i), kDelonghiRadiatorBits);
    _irsend.sendDelonghiRadiator(getRaw(i), kDelonghiRadiatorBits, 0);
    delay(10);    //  Small delay between words.
  }
}
#endif // SEND_DELONGHI_RADIATOR

/// Calculate the checksum for a given state.
/// @param[in] state The value to calc the checksum of.
/// @return A valid checksum value.
uint8_t IRDelonghiRadiator::calcChecksum(const uint64_t state)
{
    uint8_t crc = 0;
    uint8_t data[8];
    uint64_t *Fake = (uint64_t *) data;

    *Fake = state; 
    for (int i = 0; i < 4; i++)
    {
        crc = CrcTable[crc ^ data[4-i]];
    }
    return crc;
}

/// Verify the checksum is valid for a given state.
/// @param[in] state The state to verify the checksum of.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool IRDelonghiRadiator::validChecksum(const uint64_t state)
{
  DelonghiRadiatorProtocol dp;
  dp.raw = state;
  return (dp.WordMode.CheckSum == IRDelonghiRadiator::calcChecksum(state));
}

/// Calculate and set the checksum values for the internal state.
void IRDelonghiRadiator::checksum(int Index)
{
  FullSequence[Index].WordMode.CheckSum = IRDelonghiRadiator::calcChecksum(FullSequence[Index].raw);   //  WordMode doesn't matter, same place in every structure
}

/// Get a copy of the internal state as a valid code for this protocol.
/// @return A valid code for this protocol based on the current internal state.
uint64_t IRDelonghiRadiator::getRaw(int Index)
{
  checksum(Index); // Ensure correct bit array before returning
  return FullSequence[Index].raw;
}

const char *StrDay[7] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };

const char *IRDelonghiRadiator::Mode2Str(const uint8_t  Mode)
{
  switch ( Mode )
  {
    case 0: return("Forced");
    case 1: return("Comfort");
    case 2: return("Reduced");
    case 4: return("Off");
    case 8: return("Safe");
    case 16: return("Remote");
    case 47: return("Clock");
    default: return("Unknown Mode");
  }
}

char *IRDelonghiRadiator::OutputSlot(uint32_t Val)
{
static char Result[28];
  for ( int i = 0; i < 24; i++ )
  {
    if (Val & (1 <<i) )
      Result[i] = '1';
    else
      Result[i] = '0';
  }
  return Result;
}


/// @brief Convert a single word  into a human readable string.
/// @param State : Word content
/// @return : Human readable string
char *IRDelonghiRadiator::SingleWordtoString(uint64_t State)
{
DelonghiRadiatorProtocol dp;
static char Buffer[64];

  dp.raw = State;
  switch (dp.WordMode.Type)
  {
    case 0:
      snprintf(Buffer, 64, "Word D/H %s %02d:%02d:%02d\n", StrDay[dp.WordHour.Day], dp.WordHour.Hour, dp.WordHour.Minute, dp.WordHour.Second);
      break;
    case 1:
      snprintf(Buffer, 64, "Word RadiatorMode %s TComfort=%.1f TReduced=%.1f\n", Mode2Str(dp.WordMode.Mode), dp.WordMode.TempComfort/20.0, dp.WordMode.TempReduced/10.0);
      break;
    case 2:
      snprintf(Buffer, 64, "Word FanTime %d\n", dp.LastWord.FanTime*15);
      break;
    case 8:
      snprintf(Buffer, 64, "Word Monday %s\n", OutputSlot(dp.WordDay.SlotHour));
      break;
    case 9:
      snprintf(Buffer, 64, "Word Tuesday %s\n", OutputSlot(dp.WordDay.SlotHour));
      break;
    case 10:
      snprintf(Buffer, 64, "Word Wednesday %s\n", OutputSlot(dp.WordDay.SlotHour));
      break;
    case 11:
      snprintf(Buffer, 64, "Word Thursday %s\n", OutputSlot(dp.WordDay.SlotHour));
      break;
    case 12:
      snprintf(Buffer, 64, "Word Friday %s\n", OutputSlot(dp.WordDay.SlotHour));
      break;
    case 13:
      snprintf(Buffer, 64, "Word Saturday %s\n", OutputSlot(dp.WordDay.SlotHour));
      break;
    case 14:
      snprintf(Buffer, 64, "Word Sunday %s\n", OutputSlot(dp.WordDay.SlotHour));
      break;
    default:
      snprintf(Buffer, 64, "Unknown Type %d\n", OutputSlot(dp.WordHour.Type));
      break;
  }
  return(Buffer);
} 
/// Convert the current full structure state into a human readable string.
/// @return A human readable string.
String IRDelonghiRadiator::FullSequencetoString()
{
  String result = "";
  result.reserve(200);
  for (int i = 0; i < 10; i++)
  {
    result += SingleWordtoString(FullSequence[i].raw);
  }
  return result;
}

/// @brief Set the current Date Time before sending data. Should be called every time !
/// Also set the valid type and checksum 
/// @param Day : 0 monday .. 6 Sunday
/// @param Hour : 0..23
/// @param Minute : 0..59
/// @param Second : 0..59
void IRDelonghiRadiator::setDateTime(const uint8_t Day, const uint8_t Hour, const uint8_t Minute, const uint8_t Second)
{
  FullSequence[0].WordHour.Type = 0;
  FullSequence[0].WordHour.Day = Day;
  FullSequence[0].WordHour.Hour = Hour;
  FullSequence[0].WordHour.Minute = Minute;
  FullSequence[0].WordHour.Second = Second;
  checksum(0);
}

/// @brief  Get the current date and time from the Full Sequence structure
/// @param Day : Day (0 Monday, 6 Sunday)
/// @param Hour 
/// @param Minute 
/// @param Second 
void IRDelonghiRadiator::getDateTime(uint8_t &Day, uint8_t &Hour, uint8_t &Minute, uint8_t &Second)
{
  Day = FullSequence[0].WordHour.Day;
  Hour = FullSequence[0].WordHour.Hour;
  Minute = FullSequence[0].WordHour.Minute;
  Second = FullSequence[0].WordHour.Second;
}

/// @brief Set the current period (clock mode) for the day
/// Also set the valid type and checksum 
/// @param Day : From 0 (monday) to 6 (sunday)
/// @param SlotVal : Bit field, 1 means comfort temperature for this hour
void IRDelonghiRadiator::setSlotDay(const uint8_t Day, const uint32_t SlotVal)
{
  FullSequence[Day+2].WordDay.Type = Day + 8;
  FullSequence[Day+2].WordDay.SlotHour = SlotVal;
  checksum(Day+2);
}

/// @brief Get the current temperature consign (clock mode) for the day
/// @param Day 
/// @return a bitfield giving temperature by hour
uint32_t IRDelonghiRadiator::getSlotDay(const uint8_t Day) const
{
  return(FullSequence[Day+2].WordDay.SlotHour);
}

/// Set the temperature.
/// Also set the valid type and checksum 
/// @param[in] degrees_comfort The temperature in degrees for Day/Comfort Mode
/// @param[in] degrees_reduced The temperature in degrees for Night/Reduced Mode
void IRDelonghiRadiator::setTemp(const float degrees_comfort, const float degrees_reduced, const bool force)
{
  uint8_t temp_comfort;
  uint8_t temp_reduced;
  if (force)
  {
    temp_comfort = degrees_comfort; // We've been asked to force set this value.
    temp_reduced = degrees_reduced; // We've been asked to force set this value.
  }
  else
  {
    uint8_t temp_min = kDelonghiRadiatorTempMinC;
    uint8_t temp_max = kDelonghiRadiatorTempMaxC;

    temp_comfort = temp_min > degrees_comfort ? temp_min : degrees_comfort;
    temp_comfort = temp_max < temp_comfort ? temp_max : temp_comfort;

    temp_reduced = temp_min > degrees_reduced ? temp_min : degrees_reduced;
    temp_reduced = temp_max < temp_reduced ? temp_max : temp_reduced;
    temp_reduced = temp_comfort < temp_reduced ? temp_comfort : temp_reduced;
  }
  FullSequence[1].WordMode.Type = 1;
  FullSequence[1].WordMode.TempComfort = (uint16_t) temp_comfort * 20;
  FullSequence[1].WordMode.TempReduced = (uint8_t) temp_reduced * 10;
  checksum(1);
}

/// Get the current temperature setting.
/// @return The current setting for temp. in currently configured units/scale.
float IRDelonghiRadiator::getTempComfort(void) const
{
  return FullSequence[1].WordMode.TempComfort / 20.0;
}

/// Get the current temperature setting.
/// @return The current setting for temp. in currently configured units/scale.
float IRDelonghiRadiator::getTempReduced(void) const
{
  return FullSequence[1].WordMode.TempReduced / 10.0;
}

/// Set the speed of the fan.
/// Also set the valid type and checksum 
/// @param[in] Duration The desired duration in minutes.
void IRDelonghiRadiator::setFanDuration(uint8_t Duration)
{

    if ( Duration > 60 ) Duration = 60;
    FullSequence[9].LastWord.Type = 2;
    FullSequence[9].LastWord.FanTime = Duration / 15;
    checksum(9);
}

/// Get the current native fan speed setting.
/// @return The current fan speed.
uint8_t IRDelonghiRadiator::getFanDuration(void) const
{
  return FullSequence[9].LastWord.FanTime * 15;
}

/// Get the operating mode setting of the radiator
/// @return The current operating mode setting.
uint8_t IRDelonghiRadiator::getMode(void) const
{
  return FullSequence[1].WordMode.Mode;
}

/// Set the operating mode of the radiator. 
/// Also set the valid type and checksum 
/// @param[in] mode The desired native operating mode. No check is performed, be sur to use the right bit field
void IRDelonghiRadiator::setMode(const uint8_t mode)
{
  FullSequence[1].WordMode.Type = 1;
  FullSequence[1].WordMode.Mode = mode;
  checksum(1);
}

/// @brief Store the received word in the FullSequenceArray
/// @param state Received word
void IRDelonghiRadiator::StoreSequence(const uint64_t state)
{
DelonghiRadiatorProtocol dp;

  dp.raw = state;
  switch (dp.WordHour.Type)
  {
    case 0:
      FullSequence[0].raw = state;
      ReceiveSequence |= 1;
      break;
    case 1:
      FullSequence[1].raw = state;
      ReceiveSequence |= 2;
      break;
    case 2:
      FullSequence[9].raw = state;
      ReceiveSequence |= 0x200;
      break;
    case 0x08:
      FullSequence[2].raw = state;
      ReceiveSequence |= 0x04;
      break;
    case 0x09:
      FullSequence[3].raw = state;
      ReceiveSequence |= 0x08;
      break;
    case 0x0A:
      FullSequence[4].raw = state;
      ReceiveSequence |= 0x10;
      break;
    case 0x0B:
      FullSequence[5].raw = state;
      ReceiveSequence |= 0x20;
      break;
    case 0x0C:
      FullSequence[6].raw = state;
      ReceiveSequence |= 0x40;
      break;
    case 0x0D:
      FullSequence[7].raw = state;
      ReceiveSequence |= 0x80;
      break;
    case 0x0E:
      FullSequence[8].raw = state;
      ReceiveSequence |= 0x100;
      break;
    default:
      DPRINT("Bad Type in StoreSequence "); DPRINTLN(dp.WordHour.Type);
  }
}

/// @brief Check if the received sequence is complete
/// @return 1 if the sequence is complete

bool IRDelonghiRadiator::IsTransmissionComplete()
{
    return ReceiveSequence == 0x3FF;    //  10 words received
}