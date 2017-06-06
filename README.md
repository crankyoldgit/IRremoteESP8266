# IRremote ESP8266 Library

[![Build Status](https://travis-ci.org/markszabo/IRremoteESP8266.svg?branch=master)](https://travis-ci.org/markszabo/IRremoteESP8266)

This library enables you to **send and receive** infra-red signals on an ESP8266 using Arduino framework (https://github.com/esp8266/Arduino)

## v2.0.0-RC1 Now Available (Testers wanted!)
A new [release candidate](https://github.com/markszabo/IRremoteESP8266/releases/tag/v2.0.0-RC1) for the upcoming [v2.0 release](https://github.com/markszabo/IRremoteESP8266/tree/v2.0-dev) is now available for public testing.
Note: You will need to change your code slightly to work with the upcoming version. You can read more about the changes on our [wiki](https://github.com/markszabo/IRremoteESP8266/wiki/Upgrading-to-v2.0) page.
Feedback and user testing is appreciated. If there are no significant problems, the [v2.0 release](https://github.com/markszabo/IRremoteESP8266/tree/v2.0-dev) will be launched in a week or so.

## History
This library is based on Ken Shirriff's work (https://github.com/shirriff/Arduino-IRremote/)

[Mark Szabo](https://github.com/markszabo/IRremoteESP8266) has updated the IRsend class to work on ESP8266 and [Sebastien Warin](https://github.com/sebastienwarin/IRremoteESP8266) the receiving & decoding part (IRrecv class).

## Installation
1. Click "Download ZIP" 
2. Extract the downloaded zip file 
3. Rename the extracted folder to "IRremoteESP8266"
4. Move this folder to your libraries directory (under windows: C:\Users\YOURNAME\Documents\Arduino\libraries\)
5. Restart your Arduino ide
6. Check out the examples

###### Using Git to install library ( Linux )
```
cd ~/Arduino/libraries
git clone https://github.com/markszabo/IRremoteESP8266.git
```
###### To Update to the latest version of the library
`
cd ~/Arduino/libraries/IRremoteESP8266 && git pull
`
## Contributing
If you want to contribute to this project:
- Report bugs and errors
- Ask for enhancements
- Create issues and pull requests
- Tell other people about this library

## Contributors
Check [here](Contributors.md)
