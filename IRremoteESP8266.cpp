 /***************************************************
 * IRremote for ESP8266
 *
 * Based on the IRremote library for Arduino by Ken Shirriff 
 * Version 0.11 August, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 * Modified by Paul Stoffregen <paul@pjrc.com> to support other boards and timers
 * Modified  by Mitra Ardron <mitra@mitra.biz> 
 * Added Sanyo and Mitsubishi controllers
 * Modified Sony to spot the repeat codes that some Sony's send
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 *
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 * LG added by Darryl Smith (based on the JVC protocol)
 * Whynter A/C ARC-110WD added by Francesco Meschia
 * Global Cache IR format sender added by Hisham Khalifa (http://www.hishamkhalifa.com)
 * Coolix A/C / heatpump added by bakrus
 *
 * Updated by markszabo (https://github.com/markszabo/IRremoteESP8266) for sending IR code on ESP8266
 * Updated by Sebastien Warin (http://sebastien.warin.fr) for receiving IR code on ESP8266
 * Nov2016 updated bu marcosamarinho to fix times and Sony and NEC improvements to reduce UNKNOWN 
 *  GPL license, all text above must be included in any redistribution
 * Nov 2016 marcosamarinho split ir types on spared files and merged the changes from IRRemote 
 *  Changes clean up debug and code clean up , improvements and fixes  on Sony and NEC , decode SHARP ,SANYO, LG 32 bits  
 * Send Mitsubish 
 *  Note : Some Functions on IRRemote changed signature , and 
 *  include "IRremoteESP8266.h" removed and now  use IRRemote.h 
 ****************************************************/
// splited in other files , need delete 
