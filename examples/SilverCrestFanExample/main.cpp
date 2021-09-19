/*
 * IRremoteESP8266: SilverCrestFanExample
 *
 * Copyright 2021 Karol Brejna, IRremoteESP8266 developers
 *
 * Demonstrates the usage of IRremoteESP8266 to send commands to a SilverCrest SSVS 85 A1 Fan.
 *
 * This program will read serial input (expects number from 0 to 5), translate to a selected
 * command code and send it to the fan.
 *
 * Example circuit diagram:
 *     https://github.com/crankyoldgit/IRremoteESP8266/wiki#ir-sending
 *
 * Changes:
 *     September, 2021
 *         - Initial version of the example
 */

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

// GPIO to use to control the IR LED circuit. Recommended: 4 (D2).
const uint16_t kIrLed = 4;
// The Serial connection baud rate.
const uint32_t kBaudRate = 115200;

// define command/code tables
const uint16_t CODE_COUNT = 5;
String button_names[CODE_COUNT] = {"ON/OFF", "Speed", "Mist", "Timer", "OSC"};
uint64_t button_codes[CODE_COUNT] = {0x581, 0x582, 0x584, 0x588, 0x590};

IRsend irsend(kIrLed);

void initSerial(unsigned long rate)
{
#if defined(ESP8266)
    Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
#else  // ESP8266
    Serial.begin(115200, SERIAL_8N1);
#endif // ESP8266
    Serial.setTimeout(500);
    while (!Serial)
    {
        delay(50);
    }
}

void printOptions()
{
    Serial.println("Available options:");
    Serial.println("0 - Show program info");
    Serial.println("1 - ON/OFF");
    Serial.println("2 - Speed");
    Serial.println("3 - Mist");
    Serial.println("4 - Timer");
    Serial.println("5 - OSC");
    Serial.println("\nType your choice and confirm with ENTER (or wait for a while)...\n");
}

void setup()
{
    initSerial(kBaudRate);
    Serial.println("Setup finished.");
    printOptions();

    irsend.begin();
}

void publishSerialData(const int index)
{
    // get selected command name and code
    String command = button_names[index - 1];
    uint64_t code = button_codes[index - 1];

    // send the code
    Serial.printf("Sending command %s (code:  %llu)\n", command.c_str(), code);
    irsend.sendSymphony(code);
}

void loop()
{
    // wait for serial input
    if (Serial.available() > 0)
    {
        // read the input
        char buf[501];
        memset(buf, 0, 501);
        Serial.readBytesUntil('\n', buf, 500);
        Serial.printf("Received '%s'...\n", buf);

        // change it to integer value
        int index = atoi(buf);
        if (index == 0) // handle help/unknown option
        {
            printOptions();
        }
        else if (index > CODE_COUNT) // handle unknown options
        {
            Serial.println("Unknown option.");
        }
        else // handle IR sending
        {
            publishSerialData(index);
        }
    }

    delay(10);
}
