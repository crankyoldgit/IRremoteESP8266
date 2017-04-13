/*
 * IRremoteESP8266: IRrecvDemo - demonstrates receiving IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 Sept, 2015
 * Based on Ken Shirriff's IrsendDemo Version 0.1 July, 2009, Copyright 2009 Ken Shirriff, http://arcfn.com
 */

#include <IRremoteESP8266.h>

int RECV_PIN = 2; //an IR detector/demodulatord is connected to GPIO pin 2

IRrecv irrecv(RECV_PIN);

decode_results results;

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
}

void loop() {
  if (irrecv.decode(&results)) {
    // print() & println() can't handle printing long longs. (uint64_t)
    // So we have to print the top and bottom halves separately.
    if (results.value >> 32)
      Serial.print((unsigned long) (results.value >> 32), HEX);
    Serial.println((unsigned long) (results.value & 0xFFFFFFFF), HEX);
    irrecv.resume(); // Receive the next value
  }
  delay(100);
}
