# IRremote ESP8266 Library

This library enables you to **send and receive** infra-red signals on an ESP8266 using Arduino framework (https://github.com/esp8266/Arduino)

This library is based on Ken Shirriff's work (https://github.com/shirriff/Arduino-IRremote/)

[Mark Szabo](https://github.com/markszabo/IRremoteESP8266) has updated the IRsend class to work on ESP8266 and [Sebastien Warin](https://github.com/sebastienwarin/IRremoteESP8266) the receiving & decoding part (IRrecv class).

Seb's notes : I also changed the pulse parameters for Samsung, update the Panasonic and Samsung decoders and remove the SANYO decoders. The IR decoder was successfully tested with Panasonic and Samsung remote controls.

Marcos Marinho's notes :Many improvements and changes are done at this new version,  Check.  [changelog.md](changelog.md)

To avoid changes at new versions use new functions so send IR . 

Use for 32 bits  larger protocols , need bits to allow generic protocols  . 

     send_address("PANASONIC",0x4004,0x100bcbd,48); 
     send_address("SANYO",0x1FC0,0x00,42); 

This two commands allows the same result. 

     send_address("NEC",0x20,0x10,32); 
     send_raw("NEC",0x20DF10EF,32) ; 

Others protocols use send_raw .
   
     send_raw("SIRCS",0x090      ,12) ;  // SONY uses SIRCS protocol as String
     send_raw("JVC"  ,0xf900     ,16) ;
     or rawData as String 
     send_raw("SIRCS","90"       ,12) ;  // SONY uses SIRCS protocol 
     send_raw("RC6   ,"F800F8416",36) ; 
     Note , This signature as String allows larger 32bits raw data, and convert hex string to long long 
     
             
Protocol number is decoded to String at results->protocol. 

## Installation
1. Click "Download ZIP" 
2. Extract the downloaded zip file 
3. Rename the extracted folder to "IRremoteESP8266"
4. Move this folder to your libraries directory (under windows: C:\Users\YOURNAME\Documents\Arduino\libraries\)
5. Restart your Arduino ide
6. Check out the examples

## Contributing
If you want to contribute to this project:
- Report bugs and errors ,adding the dump if possible . 
- Ask for enhancements
- Create issues and pull requests
- Tell other people about this library

## Contributors
Check [here](Contributors.md)
