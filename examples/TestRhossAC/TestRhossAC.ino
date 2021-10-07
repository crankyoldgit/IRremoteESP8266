/* Copyright 2021 Tom Rosenback
*/


#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <ir_Rhoss.h>

const uint16_t kIrLed = 4;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
const uint16_t kIrRecvLed = 14;  // ESP8266 GPIO pin to use. Recommended: 14 (D5).
const uint16_t kIrMaxBuffer = 1024;
const uint16_t kIrTimeout = 50;
IRRhossAc ac(kIrLed);  // Set the GPIO to be used to sending the message.
IRrecv irrecv(kIrRecvLed, kIrMaxBuffer, kIrTimeout, true);
decode_results decode_results;  // Somewhere to store the results

void setup() {
  irrecv.enableIRIn();
  Serial.begin(115200);

#if SEND_RHOSS
  ac.begin();
#endif  // SEND_RHOSS
}

void loop() {
#if DECODE_RHOSS
  if(irrecv.decode(&decode_results)) {
    Serial.println("Decoded a message");
    
    if(decode_results.decode_type == RHOSS) {
      Serial.println("Rhoss message decoded");
      Serial.println(resultToSourceCode(&decode_results));
      Serial.println(resultToHumanReadableBasic(&decode_results));
      ac.setRaw(decode_results.state);
      Serial.println(ac.toString());

      // relay received signal
#if SEND_RHOSS
      // disable receiving while sending
      irrecv.disableIRIn();
      Serial.println("Sending...");
      // Now send the IR signal.
      ac.send();
      irrecv.enableIRIn();
#else  // SEND_RHOSS
      Serial.println("Can't send because SEND_RHOSS has been disabled.");
#endif  // SEND_RHOSS
    }
  }
#endif
}
