# IRremote ESP8266 Library

[![Build Status](https://travis-ci.org/markszabo/IRremoteESP8266.svg?branch=master)](https://travis-ci.org/markszabo/IRremoteESP8266)

This library enables you to **send and receive** infra-red signals on an ESP8266 using Arduino framework (https://github.com/esp8266/Arduino)

The library was originally based on Ken Shirriff's work (https://github.com/shirriff/Arduino-IRremote/)

[Mark Szabo](https://github.com/markszabo/IRremoteESP8266) has updated the IRsend class to work on ESP8266 and [Sebastien Warin](https://github.com/sebastienwarin/IRremoteESP8266) the receiving & decoding part (IRrecv class).

## Installation
1. Click "Download ZIP"
2. Extract the downloaded zip file
3. Rename the extracted folder to "IRremoteESP8266"
4. Move this folder to your libraries directory (under windows: C:\Users\YOURNAME\Documents\Arduino\libraries\)
5. Restart your Arduino ide
6. Check out the examples

## Unit Tests
The [Unit Tests](https://en.wikipedia.org/wiki/Unit_testing) under the test/ directory are for a Unix machine, **not** the micro-controller (ESP8266).
This allows execution under Travis and on the developer's machine.
We can do this from v2.0 of the library onwards, as everything now uses c98-style type definitions.
e.g. uint16_t etc.
Any Arduino/ESP8266 specific code needs to disabling using the following lines:
```
#ifndef UNIT_TEST
<Arduino specific code ...>
#endif
```
This is not a perfect situation as we can not obvious emulate hardware specific features and differences. e.g. Interrupts, GPIOs, CPU instruction timing etc, etc.

If you want to run the tests yourself, try the following:
```
$ cd test
$ make
$ for UNITTEST in *_test; do if [ -x "./${UNITTEST}" ]; then ./${UNITTEST}; fi; done
```

## Contributing
If you want to contribute to this project:
- Report bugs and errors
- Ask for enhancements
- Create issues and pull requests
- Tell other people about this library

## Contributors
Available [here](Contributors.md)
