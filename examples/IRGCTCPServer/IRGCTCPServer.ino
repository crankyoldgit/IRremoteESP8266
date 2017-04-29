/*
 * IRremoteESP8266: IRGCTCPServer - send Global Cache-formatted codes via TCP.
 * An IR emitter must be connected to GPIO pin 4.
 * Version 0.1 1 April, 2016
 * Copyright 2016 Hisham Khalifa, http://www.hishamkhalifa.com
 *
 * Example command - Samsung TV power toggle: 38000,1,1,170,170,20,63,20,63,20,63,20,20,20,20,20,20,20,20,20,20,20,63,20,63,20,63,20,20,20,20,20,20,20,20,20,20,20,20,20,63,20,20,20,20,20,20,20,20,20,20,20,20,20,63,20,20,20,63,20,63,20,63,20,63,20,63,20,63,20,1798\r\n
 */

#include <IRremoteESP8266.h>
#include <IRremoteInt.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

const char* ssid = "...";
const char* password = "...";

WiFiServer server(4998);  // Uses port 4998.
WiFiClient client;

uint16_t *codeArray;
IRsend irsend(4);  // an IR emitter led is connected to GPIO pin 4

void parseString(String str) {
  int16_t nextIndex;
  uint16_t codeLength = 1;
  uint16_t currentIndex = 0;
  nextIndex = str.indexOf(',');

  // change to do/until and remove superfluous repetition below...
  while (nextIndex != -1) {
    if (codeLength > 1)
      codeArray = reinterpret_cast<uint16_t*>(
          realloc(codeArray, codeLength * sizeof(uint16_t)));
    else
      codeArray = reinterpret_cast<uint16_t*>(
          malloc(codeLength * sizeof(uint16_t)));

    codeArray[codeLength - 1] = (uint16_t) (
        str.substring(currentIndex, nextIndex).toInt());

    codeLength++;
    currentIndex = nextIndex + 1;
    nextIndex = str.indexOf(',', currentIndex);
  }
  codeArray = reinterpret_cast<uint16_t*>(
      realloc(codeArray, codeLength * sizeof(uint16_t)));
  codeArray[codeLength - 1] = (uint16_t) (
      str.substring(currentIndex, nextIndex).toInt());

  irsend.sendGC(codeArray, codeLength);
}

void setup() {
  // initialize serial:
  Serial.begin(115200);
  Serial.println(" ");
  Serial.println("IR TCP Server");

  while (WiFi.status() != WL_CONNECTED) {
    delay(900);
    Serial.print(".");
  }

  server.begin();
  IPAddress myAddress = WiFi.localIP();
  Serial.println(myAddress);
  irsend.begin();
}

void loop() {
  while (!client)
    client = server.available();

  while (!client.connected()) {
    delay(900);
    client = server.available();
  }

  if (client.available()) {
    String irCode = client.readStringUntil('\r');  // Exclusive of \r
    client.readStringUntil('\n');  // Skip new line as well
    client.flush();
    parseString(irCode);
  }
}
