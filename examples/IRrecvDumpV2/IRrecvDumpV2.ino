/*
 * IRremoteESP8266: IRrecvDumpV2 - dump details of IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Example circuit diagram:
 *  https://github.com/markszabo/IRremoteESP8266/wiki#ir-receiving
 * Changes:
 *   Version 0.2 April, 2017
 *     - Decode from a copy of the data so we can start capturing faster thus
 *       reduce the likelihood of miscaptures.
 * Based on Ken Shirriff's IrsendDemo Version 0.1 July, 2009, Copyright 2009 Ken Shirriff, http://arcfn.com
 */

#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#if DECODE_AC
#include <ir_Daikin.h>
#include <ir_Kelvinator.h>
#include <ir_Midea.h>
#include <ir_Toshiba.h>
#endif  // DECODE_AC

// An IR detector/demodulator is connected to GPIO pin 14(D5 on a NodeMCU
// board).
uint16_t RECV_PIN = 14;
// As this program is a special purpose capture/decoder, let us use a larger
// than normal buffer so we can handle Air Conditioner remote codes.
uint16_t CAPTURE_BUFFER_SIZE = 1024;

// Nr. of milli-Seconds of no-more-data before we consider a message ended.
// NOTE: Don't exceed MAX_TIMEOUT_MS. Typically 130ms.
// #define TIMEOUT 90U  // Suits messages with big gaps like XMP-1 & some aircon
                        // units, but can accidently swallow repeated messages
                        // in the rawData[] output.
#if DECODE_AC
#define TIMEOUT 50U  // Some A/C units have gaps in their protocols of ~40ms.
                     // e.g. Kelvinator
                     // A value this large may swallow repeats of some protocols
#else  // DECODE_AC
#define TIMEOUT 15U  // Suits most messages, while not swallowing repeats.
#endif  // DECODE_AC

// Use turn on the save buffer feature for more complete capture coverage.
IRrecv irrecv(RECV_PIN, CAPTURE_BUFFER_SIZE, TIMEOUT, true);

decode_results results;  // Somewhere to store the results

void setup() {
  // Status message will be sent to the PC at 115200 baud
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  delay(500);  // Wait a bit for the serial connection to be establised.

  irrecv.enableIRIn();  // Start the receiver
}

// Dump out the decode_results structure.
//
void dumpInfo(decode_results *results) {
  if (results->overflow)
    Serial.printf("WARNING: IR code too big for buffer (>= %d). "
                  "These results shouldn't be trusted until this is resolved. "
                  "Edit & increase CAPTURE_BUFFER_SIZE.\n",
                  CAPTURE_BUFFER_SIZE);

  // Show Encoding standard
  Serial.println("Encoding  : " +
                 typeToString(results->decode_type, results->repeat));

  // Show Code & length
  Serial.print("Code      : ");
  if (hasACState(results->decode_type)) {
#if DECODE_AC
      for (uint16_t i = 0; results->bits > i * 8; i++)
        Serial.printf("%02X", results->state[i]);
#endif  // DECODE_AC
  } else {
    serialPrintUint64(results->value, HEX);
  }
  Serial.print(" (");
  Serial.print(results->bits, DEC);
  Serial.println(" bits)");

#if DECODE_AC
  // Display the human readable state of an A/C message if we can.
  IRDaikinESP daikin(0);
  IRKelvinatorAC kelvinator(0);
  IRToshibaAC toshiba(0);
  IRMideaAC midea(0);

  String description = "";
  switch (results->decode_type) {
    case DAIKIN:
      daikin.setRaw(results->state);
      description = daikin.toString();
    case KELVINATOR:
      kelvinator.setRaw(results->state);
      description = kelvinator.toString();
    case TOSHIBA_AC:
      toshiba.setRaw(results->state);
      description = toshiba.toString();
    case MIDEA:
      midea.setRaw(results->value);
      description = midea.toString();
  }
  // If we got a human-readable description of the message, display it.
  if (description != "")  Serial.println("Human     : " + description);
#endif  // DECODE_AC
  Serial.print("Library   : v");
  Serial.println(_IRREMOTEESP8266_VERSION_);
}

uint16_t getCookedLength(decode_results *results) {
  uint16_t length = results->rawlen - 1;
  for (uint16_t i = 0; i < results->rawlen - 1; i++) {
    uint32_t usecs = results->rawbuf[i] * RAWTICK;
    // Add two extra entries for multiple larger than UINT16_MAX it is.
    length += (usecs / UINT16_MAX) * 2;
  }
  return length;
}

// Dump out the decode_results structure.
//
void dumpRaw(decode_results *results) {
  // Print Raw data
  Serial.print("Timing[");
  Serial.print(results->rawlen - 1, DEC);
  Serial.println("]: ");

  for (uint16_t i = 1; i < results->rawlen; i++) {
    if (i % 100 == 0)
      yield();  // Preemptive yield every 100th entry to feed the WDT.
    if (i % 2 == 0) {  // even
      Serial.print("-");
    } else {  // odd
      Serial.print("   +");
    }
    Serial.printf("%6d", results->rawbuf[i] * RAWTICK);
    if (i < results->rawlen - 1)
      Serial.print(", ");  // ',' not needed for last one
    if (!(i % 8)) Serial.println("");
  }
  Serial.println("");  // Newline
}

// Dump out the decode_results structure.
//
void dumpCode(decode_results *results) {
  // Start declaration
  Serial.print("uint16_t ");               // variable type
  Serial.print("rawData[");                // array name
  Serial.print(getCookedLength(results), DEC);  // array size
  Serial.print("] = {");                   // Start declaration

  // Dump data
  for (uint16_t i = 1; i < results->rawlen; i++) {
    uint32_t usecs;
    for (usecs = results->rawbuf[i] * RAWTICK;
         usecs > UINT16_MAX;
         usecs -= UINT16_MAX)
      Serial.printf("%d, 0, ", UINT16_MAX);
    Serial.print(usecs, DEC);
    if (i < results->rawlen - 1)
      Serial.print(", ");  // ',' not needed on last one
    if (i % 2 == 0) Serial.print(" ");  // Extra if it was even.
  }

  // End declaration
  Serial.print("};");  //

  // Comment
  Serial.println("  // " + typeToString(results->decode_type, results->repeat) +
                 " " + uint64ToString(results->value, HEX));

  // Now dump "known" codes
  if (results->decode_type != UNKNOWN) {
    if (hasACState(results->decode_type)) {
#if DECODE_AC
      uint16_t nbytes = results->bits / 8;
      Serial.printf("uint8_t state[%d] = {", nbytes);
      for (uint16_t i = 0; i < nbytes; i++) {
        Serial.printf("0x%02X", results->state[i]);
        if (i < nbytes - 1)
          Serial.print(", ");
      }
      Serial.println("};");
#endif  // DECODE_AC
    } else {
      // Simple protocols
      // Some protocols have an address &/or command.
      // NOTE: It will ignore the atypical case when a message has been
      // decoded but the address & the command are both 0.
      if (results->address > 0 || results->command > 0) {
        Serial.println("uint32_t address = 0x" +
                       uint64ToString(results->address, HEX) + ";");
        Serial.println("uint32_t command = 0x" +
                       uint64ToString(results->command, HEX) + ";");
      }
      // Most protocols have data
      Serial.println("uint64_t data = 0x" +
                     uint64ToString(results->value, HEX) + ";");
    }
  }
}

// The repeating section of the code
//
void loop() {
  // Check if the IR code has been received.
  if (irrecv.decode(&results)) {
    dumpInfo(&results);           // Output the results
    dumpRaw(&results);            // Output the results in RAW format
    dumpCode(&results);           // Output the results as source code
    Serial.println("");           // Blank line between entries
  }
}
