/*
 * IRremoteESP8266: IRsendGCDemo - demonstrates sending Global Cache-formatted IR codes with IRsend
 * An IR LED must be connected to ESP8266 pin 0.
 * Version 0.1 30 March, 2016
 * Based on Ken Shirriff's IrsendDemo Version 0.1 July, 2009, Copyright 2009 Ken Shirriff, http://arcfn.com
 */

#include <IRremoteESP8266.h>

// Codes are in Global Cache format less the emitter ID and request ID. These codes can be found in GC's Control Tower database.

unsigned int Apple_menu[75] = {38000,1,69,341,172,21,21,21,65,21,65,21,65,21,21,21,65,21,65,21,65,21,65,21,65,21,65,21,21,21,21,21,21,21,21,21,65,21,65,21,65,21,21,21,21,21,21,21,21,21,21,21,21,21,65,21,65,21,21,21,21,21,21,21,65,21,21,21,21,21,1508,341,85,21,3648};
unsigned int Apple_menu_repeat_three_times[76] = {38000,3,1,69,341,172,21,21,21,65,21,65,21,65,21,21,21,65,21,65,21,65,21,65,21,65,21,65,21,21,21,21,21,21,21,21,21,65,21,65,21,65,21,21,21,21,21,21,21,21,21,21,21,21,21,65,21,65,21,21,21,21,21,21,21,65,21,21,21,21,21,1508,341,85,21,3648};

IRsend irsend(4); //an IR led is connected to GPIO pin 4

void setup()
{
  irsend.begin();
  Serial.begin(115200);
}

void loop() {
  Serial.println("Apple TV menu");
  irsend.sendGC(Apple_menu, 75);
  delay(2000);
  Serial.println("Apple TV menu repeated three times");
  irsend.sendGC(Apple_menu_repeat_three_times, 76);
  delay(2000);
}