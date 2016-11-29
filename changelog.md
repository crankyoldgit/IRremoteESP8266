## 2.0.0 - 2016/11/25
- Marcos Marinho marcosamarinho
- Fix SHARP ,changed results->data to LONG LONG to allow return 48 bits. 
- Fix SONY and included automatic repeat see MIN_REPEAT and now decoding command and address , the protocol string name is SIRCS .
- Fix NEC and implement check integrity, and decoded command and addres for usual and extended nec 32 bits 
- Fix SHARP .
- Fix SANYO ,now AIWA can be decoded too , AIWA_RC_T501 was disabled in IRremote.h as it is a so specific control .
- RC5/RC6 Clean up Debug and include VERBOSE as optional , RC6 changed to long long to accepts larger then 32 bits . 
- PANASONIC 48 bits works now  see LONG LONG data Rawdata. 
- Implemented LG 32 bits and changed SAMSUNG to have the right decode based on Repeat times .
- Implemented DISH decode . 
- Implemented dutycycle as variable see IRsend::enableIROut(int khz,int dutycycle)  .
- Changed all  DEBUG ,messages  to behavior as error to have a clear and nice output. 
- Included dump of ir times when DEBUG to easier troubleshooting .
- Changed the wait time to 80 miliseconds at timer to allow dump , REPEAT treatment .
- Include the address and comand at decode process to know protocos  . 
- Changed Rawdata decode to long long to allow decode larger numbers . 
- Split files as z30t0/IRremote and merge changes . 
- Created internal functions to simplify the code and to allow the code reuse. 
- Changed USECPERTICK  to 1 to allow have times in microseconds without trunc on integer .
- Created OFFSET_START to allow take the fake read value and keep compatibility with IRRemote
- Created generic functions to be used at client to allow no changes when implementing new protocols '
- Please change your code to use it as String to allow new protocols ...
-    bool     send_raw    (String protocol, String  hexRawData, int bits) // this allows easy input rawData larger 32 bits  
-    or  bool send_raw    (String protocol, long long rawData , int bits) .  
-    To send alreacy decoded Device address and command 
-    bool     send_address(String protocol, int address, int command, int bits). 


## 1.0.0  - 
- Based on the IRremote library for Arduino by Ken Shirriff 
- Version 0.11 August, 2009
- Copyright 2009 Ken Shirriff
- For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
- Modified by Paul Stoffregen <paul@pjrc.com> to support other boards and timers
- Modified  by Mitra Ardron <mitra@mitra.biz> 
- Added Sanyo and Mitsubishi controllers
- Modified Sony to spot the repeat codes that some Sony's send
- Interrupt code based on NECIRrcv by Joe Knapp
- http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
- Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
- JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
- LG added by Darryl Smith (based on the JVC protocol)
- Whynter A/C ARC-110WD added by Francesco Meschia
- Global Cache IR format sender added by Hisham Khalifa (http://www.hishamkhalifa.com)
- Coolix A/C / heatpump added by bakrus
- Updated by markszabo (https://github.com/markszabo/IRremoteESP8266) for sending IR code on ESP8266
- Updated by Sebastien Warin (http://sebastien.warin.fr) for receiving IR code on ESP8266

