# IRremote ESP8266 Library

This library enables you to **send and receive** infra-red signals on an ESP8266 using Arduino framework (https://github.com/esp8266/Arduino)

This library is based on Ken Shirriff's work (https://github.com/shirriff/Arduino-IRremote/)

[Mark Szabo](https://github.com/markszabo/IRremoteESP8266) has updated the IRsend class to work on ESP8266 and [Sebastien Warin](https://github.com/sebastienwarin/IRremoteESP8266) the receiving & decoding part (IRrecv class).

Seb's notes : I also changed the pulse parameters for Samsung, update the Panasonic and Samsung decoders and remove the SANYO decoders. The IR decoder was successfully tested with Panasonic and Samsung remote controls.

Marcos Marinho's notes :Many improvements and changes are done at this version,  Check.  [changelog.md](changelog.md)

To avoid changes use new functions so send IR . 


Use for 32 bits  larger protocols  . 

     send_address("PANASONIC",0x4004,0x100bcbd); 
     send_address("SANYO",0x1FC0,0x00); 

Others .

     send_raw("NEC",0xc53a9966,32) ; 
     send_raw("SONY",0x090,12) ; 

Protocol is decoded to String at results->protocol. 


## Installation
1. Click "Download ZIP" 
2. Extract the downloaded zip file 
3. Rename the extracted folder to "IRremoteESP8266"
4. Move this folder to your libraries directory (under windows: C:\Users\YOURNAME\Documents\Arduino\libraries\)
5. Restart your Arduino ide
6. Check out the examples

## Contributing
If you want to contribute to this project:
- Report bugs and errors
- Ask for enhancements
- Create issues and pull requests
- Tell other people about this library

## Contributors
Check [here](Contributors.md)
