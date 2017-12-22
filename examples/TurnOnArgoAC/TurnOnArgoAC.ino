/* Copyright 2017 crankyoldgit
* An IR LED circuit *MUST* be connected to ESP8266 GPIO4.
*
* TL;DR: The IR LED needs to be driven by a transistor for a good result.
*
* Suggested circuit:
*     https://github.com/markszabo/IRremoteESP8266/wiki#ir-sending
*
* Common mistakes & tips:
*   * Don't just connect the IR LED directly to the pin, it won't
*     have enough current to drive the IR LED effectively.
*   * Make sure you have the IR LED polarity correct.
*     See: https://learn.sparkfun.com/tutorials/polarity/diode-and-led-polarity
*   * Typical digital camera/phones can be used to see if the IR LED is flashed.
*     Replace the IR LED with a normal LED if you don't have a digital camera
*     when debugging.
*   * Avoid using the following pins unless you really know what you are doing:
*     * Pin 0/D3: Can interfere with the boot/program mode & support circuits.
*     * Pin 1/TX/TXD0: Any serial transmissions from the ESP8266 will interfere.
*     * Pin 3/RX/RXD0: Any serial transmissions to the ESP8266 will interfere.
*   * ESP-01 modules are tricky. We suggest you use a module with more GPIOs
*     for your first time. e.g. ESP-12 etc.
*/

#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Argo.h>

IRArgoAC argoir(4);  // An IR LED is controlled by GPIO4, NodeMCU D2

void setup() {
  argoir.begin();
  Serial.begin(115200);
}

void loop() {
  Serial.println("Sending...");

  // Set up what we want to send. See ir_Argo.cpp for all the options.
  argoir.setPower(true);
  argoir.setFan(ARGO_FAN_1);
  argoir.setCoolMode(ARGO_COOL_AUTO);
  argoir.setTemp(25);

#if SEND_ARGO
  // Now send the IR signal.
  argoir.send();
#else  // SEND_ARGO
  Serial.println("Can't send because SEND_ARGO has been disabled.");
#endif  // SEND_ARGO

  delay(5000);
}
