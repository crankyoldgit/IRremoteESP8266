 /***************************************************
 * IRremote for ESP8266
 *
 * Based on the IRremote library for Arduino by Ken Shirriff
 * Version 0.11 August, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 * Edited by Mitra to add new controller SANYO
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 *
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 * LG added by Darryl Smith (based on the JVC protocol)
 * Whynter A/C ARC-110WD added by Francesco Meschia
 * Coolix A/C / heatpump added by bakrus
 *
 * Updated by markszabo (https://github.com/markszabo/IRremoteESP8266) for sending IR code on ESP8266
 * Updated by Sebastien Warin (http://sebastien.warin.fr) for receiving IR code on ESP8266
 *
 *  Updated by sillyfrog for Daikin, adopted from
 * (https://github.com/mharizanov/Daikin-AC-remote-control-over-the-Internet/)
 *
 *  GPL license, all text above must be included in any redistribution
 *  Nov2016 marcosamarinho 
 *  Note : with the changes to allow spare files to update from IRRemote 
 * the things are moved to the include IRRemote.h 
 ****************************************************/

#ifndef IRremote_h
#define IRremote_h

#define SEND_PROTOCOL_NEC     case NEC:     sendNEC(data, nbits); break;
#define SEND_PROTOCOL_SONY    case SONY:    sendSony(data, nbits); break;
#define SEND_PROTOCOL_RC5     case RC5:     sendRC5(data, nbits); break;
#define SEND_PROTOCOL_RC6     case RC6:     sendRC6(data, nbits); break;
#define SEND_PROTOCOL_DISH    case DISH:    sendDISH(data, nbits); break;
#define SEND_PROTOCOL_JVC     case JVC:     sendJVC(data, nbits, 0); break;
#define SEND_PROTOCOL_SAMSUNG case SAMSUNG: sendSAMSUNG(data, nbits); break;
#define SEND_PROTOCOL_LG      case LG:      sendLG(data, nbits); break;
#define SEND_PROTOCOL_WHYNTER case WHYNTER: sendWhynter(data, nbits); break;
#define SEND_PROTOCOL_COOLIX  case COOLIX:  sendCOOLIX(data, nbits); break;
// The following are compile-time library options.
// If you change them, recompile the library.
// If DEBUG is defined, a lot of debugging output will be printed during decoding.
// TEST must be defined for the IRtest unittests to work.  It will make some
// methods virtual, which will be slightly slower, which is why it is optional.
//#define DEBUG
//#define TEST

 //Only used for testing; can remove virtual for shorter code
#ifdef TEST
#define VIRTUAL virtual
#else
#define VIRTUAL
#endif


#endif
