// Copyright 2026 IRremoteESP8266 contributors
// Serial-controlled Blue Star IE318YKU sender. No commands are sent at boot.
// GPIO 15 is the transmitting IR LED pin for the capture/replay setup.
// See docs/BlueStar.md for packet layout, limitations, and verification steps.

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <ir_Bluestar.h>

const uint16_t kIrLed = 15;
IRBluestarAc ac(kIrLed);

void printHelp() {
  Serial.println(F("Blue Star sender, GPIO 15, 115200 baud. Commands:"));
  Serial.println(F("  on / off / temp 16..30 / fan auto|low|medium|high"));
  Serial.println(F("  mode auto|cool|dry|fan / capacity 100|110|80|60|40"));
  Serial.println(F("  horizontal on|off / vertical on|off / light on|off"));
  Serial.println(F("  turbo on|off / econo on|off / sleep on|off"));
  Serial.println(F("  state / send / load <24 hexadecimal digits> / help"));
  Serial.println(F("load: 10 main bytes + 2 secondary command bytes."));
}

int hexDigit(const char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

bool loadState(String hex) {
  hex.replace(" ", "");
  if (hex.length() != kBluestarStateLength * 2) return false;
  uint8_t state[kBluestarStateLength];
  for (uint8_t i = 0; i < kBluestarStateLength; i++) {
    const int high = hexDigit(hex[i * 2]);
    const int low = hexDigit(hex[i * 2 + 1]);
    if (high < 0 || low < 0) return false;
    state[i] = (high << 4) | low;
  }
  return IRBluestarAc::validChecksum(state) && ac.setRaw(state);
}

void printState() {
  Serial.println(ac.toString());
  Serial.print(F("State: "));
  uint8_t *state = ac.getRaw();
  for (uint8_t i = 0; i < kBluestarStateLength; i++) {
    if (state[i] < 16) Serial.print('0');
    Serial.print(state[i], HEX);
    if (i + 1 < kBluestarStateLength) Serial.print(' ');
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(250);
  ac.begin();
  printHelp();
  printState();
}

void loop() {
  if (!Serial.available()) return;
  String line = Serial.readStringUntil('\n');
  line.trim();
  line.toLowerCase();
  const int space = line.indexOf(' ');
  const String command = space < 0 ? line : line.substring(0, space);
  String value = space < 0 ? "" : line.substring(space + 1);
  value.trim();
  bool transmit = true;
  if (command == "on") {
    ac.on();
  } else if (command == "off") {
    ac.off();
  } else if (command == "temp") {
    const int temp = value.toInt();
    if (temp < kBluestarMinTemp || temp > kBluestarMaxTemp) {
      Serial.println(F("Temperature must be 16..30 C."));
      return;
    }
    ac.setTemp(temp);
  } else if (command == "fan") {
    if (value == "auto") ac.setFan(kBluestarFanAuto);
    else if (value == "low") ac.setFan(kBluestarFanLow);
    else if (value == "medium") ac.setFan(kBluestarFanMed);
    else if (value == "high") ac.setFan(kBluestarFanHigh);
    else
      return printHelp();
  } else if (command == "mode") {
    if (value == "auto") ac.setMode(kBluestarAuto);
    else if (value == "cool") ac.setMode(kBluestarCool);
    else if (value == "dry") ac.setMode(kBluestarDry);
    else if (value == "fan") ac.setMode(kBluestarFanOnly);
    else
      return printHelp();
  } else if (command == "capacity") {
    if (value == "100") ac.setCapacity(kBluestarCapacityDefault);
    else if (value == "110") ac.setCapacity(kBluestarCapacityTurbo);
    else if (value == "80") ac.setCapacity(kBluestarCapacity80);
    else if (value == "60") ac.setCapacity(kBluestarCapacity60);
    else if (value == "40") ac.setCapacity(kBluestarCapacity40);
    else
      return printHelp();
  } else if (command == "horizontal" || command == "vertical" ||
             command == "light" || command == "sleep" ||
             command == "turbo" || command == "econo") {
    if (value != "on" && value != "off") return printHelp();
    const bool on = value == "on";
    if (command == "horizontal") ac.setSwingH(on);
    else if (command == "vertical") ac.setSwingV(on);
    else if (command == "light") ac.setLight(on);
    else if (command == "sleep") ac.setSleep(on);
    else if (command == "turbo") ac.setTurbo(on);
    else ac.setEcono(on);
  } else if (command == "load") {
    if (!loadState(value)) {
      Serial.println(F("Invalid length, hexadecimal value, or checksum."));
      return;
    }
    transmit = false;  // Explicit 'send' replays the loaded capture.
  } else if (command == "state") {
    transmit = false;
  } else if (command == "help") {
    printHelp();
    return;
  } else if (command != "send") {
    printHelp();
    return;
  }
  if (transmit) {
    ac.send();
    Serial.println(F("Sent."));
  }
  printState();
}
