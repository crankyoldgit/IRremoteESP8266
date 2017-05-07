/* Copyright 2016 sillyfrog */
#include <IRsend.h>
#include <ir_Daikin.h>

IRDaikinESP dakinir(D1);

void setup() {
  dakinir.begin();
  Serial.begin(115200);
}


void loop() {
  Serial.println("Sending...");

  // Set up what we want to send. See ir_Daikin.cpp for all the options.
  dakinir.on();
  dakinir.setFan(1);
  dakinir.setMode(DAIKIN_COOL);
  dakinir.setTemp(25);
  dakinir.setSwingVertical(0);
  dakinir.setSwingHorizontal(0);

  // Now send the IR signal.
  dakinir.send();

  delay(5000);
}
