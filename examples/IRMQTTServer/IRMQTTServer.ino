/*
 * Send arbitary IR codes via a web server or MQTT.
 * Copyright David Conran 2016
 * Version 0.3 Oct, 2017
 *
 * NOTE: An IR LED circuit *MUST* be connected to ESP8266 pin 4 (D2). See IR_LED
 *
 * WARN: This is very advanced & complicated example code. Not for beginners.
 *       You are strongly suggested to try & look at other example code first.
 *
 * # Instructions
 *
 * ## Before First Boot (i.e. Compile time)
 * - Set the MQTT_SERVER define below to the address of your MQTT server.
 * - Arduino IDE:
 *   o Install the following libraries via Library Manager
 *     - WiFiManager (https://github.com/tzapu/WiFiManager)
 *     - PubSubClient (https://pubsubclient.knolleary.net/)
 *   o You MUST change <PubSubClient.h> to have the following (or larger) value:
 *     #define MQTT_MAX_PACKET_SIZE 400
 * - PlatformIO IDE:
 *     If you are using PlatformIO, this should already been done for you in
 *     the accompanying platformio.ini file.
 *
 * ## First Boot (Initial setup)
 * The ESP8266 board will boot into the WiFiManager's AP mode.
 * i.e. It will create a WiFi Access Point with a SSID like: "ESP123456" etc.
 * Connect to that SSID. Then point your browser to http://192.168.4.1/ and
 * configure the ESP8266 to connect to your desired WiFi network.
 * It will remember the new WiFi connection details on next boot.
 * More information can be found here:
 *   https://github.com/tzapu/WiFiManager#how-it-works
 *
 * If you need to reset the WiFi settings, visit:
 *   http://<your_esp8266's_ip_address>/reset
 *
 * ## Normal Use (After setup)
 * Enter 'http://<your_esp8266's_ip_address/' in your browser & follow the
 * instructions there to send IR codes via HTTP/HTML.
 * You can send URLs like the following, with similar data type limitations as
 * the MQTT formating in the next section. e.g:
 *   http://<your_esp8266's_ip_address>/ir?type=7&code=E0E09966
 *   http://<your_esp8266's_ip_address>/ir?type=4&code=0xf50&bits=12
 *   http://<your_esp8266's_ip_address>/ir?code=C1A2E21D&repeats=8&type=19
 *   http://<your_esp8266's_ip_address>/ir?type=31&code=40000,1,1,96,24,24,24,48,24,24,24,24,24,48,24,24,24,24,24,48,24,24,24,24,24,24,24,24,1058
 *   http://<your_esp8266's_ip_address>/ir?type=18&code=190B8050000000E0190B8070000010f0
 *
 * or
 *
 * Send a MQTT message to the topic 'ir_server/send' using the following
 * format (Order is important):
 *   protocol_num,hexcode  e.g. 7,E0E09966 which is Samsung(7), Power On code,
 *                              default bit size, default nr. of repeats.
 *   protocol_num,hexcode,bits  e.g. 4,f50,12 which is Sony(4), Power Off code,
 *                               12 bits & default nr. of repeats.
 *   protocol_num,hexcode,bits,repeats  e.g. 19,C1A2E21D,0,8 which is
 *                                      Sherwood(19), Vol Up, default bit size &
 *                                      repeated 8 times.
 *   31,code_string  e.g. 31,40000,1,1,96,24,24,24,48,24,24,24,24,24,48,24,24,24,24,24,48,24,24,24,24,24,24,24,24,1058
 *                        GlobalCache (31) & "40000,1,1,96,..." (Sony Vol Up)
 *   18,really_long_hexcode  e.g. 18,190B8050000000E0190B8070000010f0
 *                           Kelvinator (18) Air Con on, Low Fan, 25 deg etc.
 *   In short:
 *     No spaces after/before commas.
 *     Values are comma separated.
 *     The first value is always in Decimal.
 *     For simple protocols, the next value (hexcode) is always hexidecimal.
 *     The optional bit size is in decimal.
 *
 *   Unix command line usage example:
 *     # Install a MQTT client
 *     $ sudo apt install mosquitto-clients
 *     # Send a 32-bit NEC code of 0x1234abcd via MQTT.
 *     $ mosquitto_pub -h 10.20.0.253 -t ir_server/send -m '3,1234abcd,32'
 *
 * This server will send (back) what ever IR message it just transmitted to
 * the MQTT topic 'ir_server/sent' to confirm it has been performed. This works
 * for messages requested via MQTT or via HTTP.
 * Note: Other status messages are also sent to 'ir_server/sent' from time to
 * time.
 *   Unix command line usage example:
 *     # Listen to MQTT acknowledgements.
 *     $ mosquitto_sub -h 10.20.0.253 -t ir_server/sent
 *
 * If DEBUG is turned on, there is additional information printed on the Serial
 * Port.
 *
 * ## Updates
 * You can upload new firmware over the air (OTA) via the form on the device's
 * main page. No need to connect to the device again via USB. \o/
 * Your WiFi settings should be remembered between updates. \o/ \o/
 *
 * Copyright Notice:
 *   Code for this has been borrowed from lots of other OpenSource projects &
 *   resources. I'm *NOT* claiming complete Copyright ownership of all the code.
 *   Likewise, feel free to borrow from this as much as you want.
 */

#define MQTT_ENABLE  // Comment this out if you don't want to use MQTT at all.

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266mDNS.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRutils.h>
#ifdef MQTT_ENABLE
// --------------------------------------------------------------------
// * * * IMPORTANT * * *
// You must change <PubSubClient.h> to have the following value.
// #define MQTT_MAX_PACKET_SIZE 400
// --------------------------------------------------------------------
#include <PubSubClient.h>
#endif
#include <algorithm>
#include <string>

// Configuration paramters
#define IR_LED 4  // GPIO the IR LED is connected to/controlled by. GPIO 4 = D2.
#define HTTP_PORT 80  // The port the HTTP server is listening on.

#ifdef MQTT_ENABLE
// Address of your MQTT server.
#define MQTT_SERVER "10.20.0.253"  // <=- CHANGE ME
#define MQTT_PORT 1883  // Default port used by MQTT servers.
// Set if your MQTT server requires a Username & Password to connect.
const char* mqtt_user = "";
const char* mqtt_password = "";
#define MQTT_RECONNECT_TIME 5000  // Delay(ms) between reconnect tries.

#define MQTTprefix "ir_server"
#define MQTTack MQTTprefix "/sent"  // Topic we send back acknowledgements on
#define MQTTcommand MQTTprefix "/send"  // Topic we get new commands from.
#endif

// HTML arguments we will parse for IR code information.
#define argType "type"
#define argData "code"
#define argBits "bits"
#define argRepeat "repeats"
#define DEBUG True

// Globals
ESP8266WebServer server(HTTP_PORT);
IRsend irsend = IRsend(IR_LED);
MDNSResponder mdns;
WiFiClient espClient;
WiFiManager wifiManager;

uint16_t *codeArray;
uint32_t lastReconnectAttempt = 0;  // MQTT last attemps reconnection number
bool boot = true;
bool ir_lock = false;  // Primative locking for gating the IR LED.

#ifdef MQTT_ENABLE
// MQTT client parameters
void callback(char* topic, byte* payload, unsigned int length);
PubSubClient mqtt_client(MQTT_SERVER, MQTT_PORT, callback, espClient);
// Create a unique MQTT client id.
const char* mqtt_clientid = String(MQTTprefix +
                                   String(ESP.getChipId(), HEX)).c_str();
#endif

// Debug messages get sent to the serial port.
void debug(String str) {
#ifdef DEBUG
  Serial.println(str);
#endif
}

// Root web page with example usage etc.
void handleRoot() {
  server.send(200, "text/html",
    "<html><head><title>IR MQTT server</title></head>"
    "<body>"
    "<center><h1>ESP8266 IR MQTT Server</h1></center>"
    "<br><hr>"
    "<h3>Hardcoded examples</h3>"
    "<p><a href=\"ir?code=38000,1,69,341,171,21,64,21,64,21,21,21,21,21,21,21,"
      "21,21,21,21,64,21,64,21,21,21,64,21,21,21,21,21,21,21,64,21,21,21,64,"
      "21,21,21,21,21,21,21,64,21,21,21,21,21,21,21,21,21,64,21,64,21,64,21,"
      "21,21,64,21,64,21,64,21,1600,341,85,21,3647&type=31\">"
      "Sherwood Amp On</a></p>"
    "<p><a href=\"ir?code=38000,1,69,340,171,21,64,21,64,21,21,21,21,21,21,21,"
      "21,21,21,21,64,21,64,21,21,21,64,21,21,21,21,21,21,21,64,21,21,21,64,"
      "21,21,21,64,21,64,21,64,21,21,21,21,21,21,21,21,21,64,21,21,21,21,21,"
      "21,21,64,21,64,21,64,21,1600,340,85,21,3647&type=31\">"
      "Sherwood Amp Off</a></p>"
    "<p><a href=\"ir?code=38000,1,69,341,170,21,64,21,64,21,21,21,21,21,21,21,"
      "21,21,21,21,64,21,64,21,21,21,64,21,21,21,21,21,21,21,64,21,21,21,21,"
      "21,64,21,64,21,21,21,21,21,21,21,21,21,21,21,64,21,21,21,21,21,64,21,"
      "64,21,64,21,64,21,64,21,1600,341,85,21,3648&type=31\">"
      "Sherwood Amp Input TAPE</a></p>"
    "<p><a href=\"ir?type=7&code=E0E09966\">Samsung TV on</a></p>"
    "<p><a href=\"ir?type=4&code=0xf50&bits=12\">Sony Power Off</a></p>"
    "<br><hr>"
    "<h3>Send a simple IR message</h3><p>"
    "<form method='POST' action='/ir' enctype='multipart/form-data'>"
      "Type: "
      "<select name='type'>"
        "<option value='9'>Aiwa RC T501</option>"
        "<option value='15'>Coolix</option>"
        "<option value='17'>Denon</option>"
        "<option value='13'>Dish</option>"
        "<option value='6'>JVC</option>"
        "<option value='10'>LG</option>"
        "<option value='12'>Mitsubishi</option>"
        "<option selected='selected' value='3'>NEC</option>"  // Default
        "<option value='29'>Nikai</option>"
        "<option value='5'>Panasonic</option>"
        "<option value='1'>RC-5</option>"
        "<option value='23'>RC-5X</option>"
        "<option value='2'>RC-6</option>"
        "<option value='21'>RC-MM</option>"
        "<option value='7'>Samsung</option>"
        "<option value='11'>Sanyo</option>"
        "<option value='22'>Sanyo LC7461</option>"
        "<option value='14'>Sharp</option>"
        "<option value='19'>Sherwood</option>"
        "<option value='4'>Sony</option>"
        "<option value='8'>Whynter</option>"
      "</select>"
      " Code: 0x<input type='text' name='code' min='0' value='0' size='16'"
        " maxlength='16'>"
      " Bit size: "
      "<select name='bits'>"
        "<option selected='selected' value='0'>Default</option>"  // Default
        // Common bit length options for most protocols.
        "<option value='12'>12</option>"
        "<option value='13'>13</option>"
        "<option value='14'>14</option>"
        "<option value='15'>15</option>"
        "<option value='16'>16</option>"
        "<option value='20'>20</option>"
        "<option value='21'>21</option>"
        "<option value='24'>24</option>"
        "<option value='28'>28</option>"
        "<option value='32'>32</option>"
        "<option value='36'>36</option>"
        "<option value='48'>48</option>"
      "</select>"
      " Repeats: <input type='number' name='repeats' min='0' max='99' value='0'"
        "size='2' maxlength='2'>"
      " <input type='submit' value='Send IR'>"
    "</form>"
    "<br><hr>"
    "<h3>Send a <a href='https://irdb.globalcache.com/'>GlobalCache</a>"
      " IR message</h3><p>"
    "<form method='POST' action='/ir' enctype='multipart/form-data'>"
      "<input type='hidden' name='type' value='31'>"
      "String: 1:1,1,<input type='text' name='code' size='132'"
      " value='38000,1,1,170,170,20,63,20,63,20,63,20,20,20,20,20,20,20,20,20,"
      "20,20,63,20,63,20,63,20,20,20,20,20,20,20,20,20,20,20,20,20,63,20,20,20,"
      "20,20,20,20,20,20,20,20,20,20,63,20,20,20,63,20,63,20,63,20,63,20,63,20,"
      "63,20,1798'>"
      " <input type='submit' value='Send GlobalCache'>"
    "</form>"
    "<br><hr>"
    "<h3>Send a Kelvinator A/C IR message</h3><p>"
    "<form method='POST' action='/ir' enctype='multipart/form-data'>"
      "<input type='hidden' name='type' value='18'>"
      "State code: 0x<input type='text' name='code' size='32' maxlength='32'"
      " value='190B8050000000E0190B8070000010f0'>"
      " <input type='submit' value='Send A/C State'>"
    "</form>"
    "<br><hr>"
    "<h3>Update IR Server firmware</h3><p>"
    "<b><mark>Warning:</mark></b><br> "
    "<i>Updating your firmware may screw up your access to the device. "
    "If you are going to use this, know what you are doing first "
    "(and you probably do).</i><br>"
    "<form method='POST' action='/update' enctype='multipart/form-data'>"
      "Firmware to upload: <input type='file' name='update'>"
      "<input type='submit' value='Update'>"
    "</form>"
    "</body></html>");
}

// Reset web page
void handleReset() {
  server.send(200, "text/html",
    "<html><head><title>Reset Config</title></head>"
    "<body>"
    "<h1>Resetting the WiFiManager config back to defaults.</h1>"
    "<p>Device restarting. Try connecting in a few seconds.</p>"
    "</body></html>");
    // Do the reset.
  wifiManager.resetSettings();
  delay(10);
  ESP.restart();
  delay(1000);
}

// Parse a Kelvinator A/C Hex String/code and send it.
void parseStringAndSendKelv(const String str) {
  // str should be a 32 digit hexidecimal string.
  uint8_t offset = 0;
  uint8_t codeArray[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  if (str.startsWith("0x") || str.startsWith("0X"))
    offset = 2;
  for (int i = 0; i < 32; i++) {
    unsigned char c = tolower(str[i + offset]);
    uint8_t entry = 0;
    if (isxdigit(c)) {
      if (isdigit(c))
        codeArray[i / 2] += c - '0';
      else
        codeArray[i / 2] += c - 'a' + 10;
    }
    if (i % 2 == 0)
      codeArray[i / 2] <<= 4;
  }
  irsend.sendKelvinator(reinterpret_cast<uint8_t *>(codeArray));
}

void parseStringAndSendGC(const String str) {
  int16_t index;
  uint16_t count;
  uint16_t *code_array;

  // Find out how many items there are in the string.
  index = -1;
  count = 1;
  do {
    index = str.indexOf(',', index + 1);
    count++;
  } while (index != -1);

  int16_t nextIndex = 0;
  uint16_t codeLength = 0;
  int16_t currentIndex = 0;

  // Now we know how many there are, allocate the memory to store them all.
  code_array = reinterpret_cast<uint16_t*>(malloc(count * sizeof(uint16_t)));
  // Check we malloc'ed successfully.
  if (code_array == NULL) {  // malloc failed, so give up.
    Serial.printf("\nCan't allocate %d bytes. (%d bytes free)\n",
                  count * sizeof(uint16_t), ESP.getFreeHeap());
    Serial.println("Giving up & forcing a reboot.");
    ESP.restart();  // Reboot.
    delay(500);  // Wait for the restart to happen.
    return;  // Should never get here, but just in case.
  }

  // Now convert the strings to integers and place them in code_array.
  count = 0;
  uint16_t start_from = 0;
  do {
    index = str.indexOf(',', start_from);
    code_array[count] = str.substring(start_from, index).toInt();
    start_from = index + 1;
    count++;
  } while (index != -1);

  irsend.sendGC(code_array, count);  // All done. Send it.
  free(code_array);  // Free up the memory allocated.
}

// Parse the URL args to find the IR code.
void handleIr() {
  uint64_t data = 0;
  String data_str = "";
  int ir_type = 3;  // Default to NEC codes.
  uint16_t nbits = 0;
  uint16_t repeat = 0;

  for (uint16_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == argType)
      ir_type = atoi(server.arg(i).c_str());
    if (server.argName(i) == argData) {
      data = getUInt64fromHex(server.arg(i).c_str());
      data_str = server.arg(i);
    }
    if (server.argName(i) == argBits)
      nbits = atoi(server.arg(i).c_str());
    if (server.argName(i) == argRepeat)
      repeat = atoi(server.arg(i).c_str());
  }
  debug("New code received via HTTP");
  sendIRCode(ir_type, data, data_str.c_str(), nbits, repeat);
  handleRoot();
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i < server.args(); i++)
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  server.send(404, "text/plain", message);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network

  wifiManager.setTimeout(300);  // Time out after 5 mins.
  if (!wifiManager.autoConnect()) {
    debug("Failed to connect and hit timeout.");
    delay(3000);
    // Reboot. A.k.a. "Have you tried turning it Off and On again?"
    ESP.reset();
    delay(5000);
  }

  debug("WiFi connected. IP address: " + WiFi.localIP());
}

void setup(void) {
  irsend.begin();

  #ifdef DEBUG
  Serial.begin(115200);
  #endif

  setup_wifi();

  // Wait a bit for things to settle.
  delay(1500);

  lastReconnectAttempt = 0;

  if (mdns.begin(MQTTprefix, WiFi.localIP())) {
    debug("MDNS responder started");
  }

  // Setup the root web page.
  server.on("/", handleRoot);
  // Setup the page to handle web-based IR codes.
  server.on("/ir", handleIr);
  // Setup a reset page to cause WiFiManager information to be reset.
  server.on("/reset", handleReset);

  // Setup the URL to allow Over-The-Air (OTA) firmware updates.
  server.on("/update", HTTP_POST, [](){
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
      ESP.restart();
    }, [](){
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        WiFiUDP::stopAll();
        debug("Update: " + upload.filename);
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) &
            0xFFFFF000;
        if (!Update.begin(maxSketchSpace)) {  // start with max available size
#ifdef DEBUG
          Update.printError(Serial);
#endif
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) !=
            upload.currentSize) {
#ifdef DEBUG
          Update.printError(Serial);
#endif
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {  // true to set the size to the current progress
          debug("Update Success: " + (String) upload.totalSize +
                "\nRebooting...");
        }
      }
      yield();
    });

  // Set up an error page.
  server.onNotFound(handleNotFound);

  server.begin();
  debug("HTTP server started");
}

#ifdef MQTT_ENABLE
// MQTT subscribing to topic
void subscribing(const String topic_name) {
  // subscription to topic for receiving data
  if (mqtt_client.subscribe(topic_name.c_str())) {
    debug("Subscription OK to " + topic_name);
  }
}

bool reconnect() {
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    int connected = false;
    // Attempt to connect
    debug("Attempting MQTT connection ... ");
    if (mqtt_user && mqtt_password)
      connected = mqtt_client.connect(mqtt_clientid, mqtt_user, mqtt_password);
    else
      connected = mqtt_client.connect(mqtt_clientid);
    if (connected) {
    // Once connected, publish an announcement...
      mqtt_client.publish(MQTTack, "Connected");
      debug("connected.");
    // Subscribing to topic(s)
    subscribing(MQTTcommand);
    } else {
      debug("failed, rc=" + String(mqtt_client.state()) +
            " try again in a bit.");
      // Wait for a bit before retrying
      delay(MQTT_RECONNECT_TIME);
    }
  }
  return mqtt_client.connected();
}
#endif

void loop(void) {
  server.handleClient();

#ifdef MQTT_ENABLE
  // MQTT client connection management
  if (!mqtt_client.connected()) {
    uint32_t now = millis();
    if (now - lastReconnectAttempt > MQTT_RECONNECT_TIME) {
      lastReconnectAttempt = now;
      debug("client mqtt not connected, trying to connect");
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
        if (boot) {
          mqtt_client.publish(MQTTack, "IR Server just booted");
          boot = false;
        } else {
          mqtt_client.publish(MQTTack, "IR Server just (re)connected to MQTT");
        }
      }
    }
  } else {
    // MQTT loop
    mqtt_client.loop();
  }
#endif
  delay(100);
}

// Arduino framework doesn't support strtoull(), so make our own one.
uint64_t getUInt64fromHex(char const *str) {
  uint64_t result = 0;
  uint16_t offset = 0;
  // Skip any leading '0x' or '0X' prefix.
  if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    offset = 2;
  for (; isxdigit((unsigned char)str[offset]); offset++) {
    char c = str[offset];
    result *= 16;
    if (isdigit(c)) /* '0' .. '9' */
      result += c - '0';
    else if (isupper(c)) /* 'A' .. 'F' */
      result += c - 'A' + 10;
    else /* 'a' .. 'f'*/
      result += c - 'a' + 10;
  }
  return result;
}

// Transmit the given IR message.
//
// Args:
//   ir_type:  enum of the protocol to be sent.
//   code:     Numeric payload of the IR message. Most protocols use this.
//   code_str: The unparsed code to be sent. Used by complex protocol encodings.
//   nbits:    Nr. of bits in the protocol. 0 means use the protocol's default.
void sendIRCode(int const ir_type, uint64_t const code, char const * code_str,
                uint16_t bits, uint16_t repeat) {
  // Create a pseudo-lock so we don't try to send two codes at the same time.
  while (ir_lock)
    delay(20);
  ir_lock = true;

  // send the IR message.
  switch (ir_type) {
    case RC5:  // 1
      if (bits == 0)
        bits = RC5_BITS;
      irsend.sendRC5(code, bits, repeat);
      break;
    case RC6:  // 2
      if (bits == 0)
        bits = RC6_MODE0_BITS;
      irsend.sendRC6(code, bits, repeat);
      break;
    case NEC:  // 3
      if (bits == 0)
        bits = NEC_BITS;
      irsend.sendNEC(code, bits, repeat);
      break;
    case SONY:  // 4
      if (bits == 0)
        bits = SONY_12_BITS;
      repeat = std::max(repeat, (uint16_t) SONY_MIN_REPEAT);
      irsend.sendSony(code, bits, repeat);
      break;
    case PANASONIC:  // 5
      if (bits == 0)
        bits = PANASONIC_BITS;
      irsend.sendPanasonic64(code, bits, repeat);
      break;
    case JVC:  // 6
      if (bits == 0)
        bits = JVC_BITS;
      irsend.sendJVC(code, bits, repeat);
      break;
    case SAMSUNG:  // 7
      if (bits == 0)
        bits = SAMSUNG_BITS;
      irsend.sendSAMSUNG(code, bits, repeat);
      break;
    case WHYNTER:  // 8
      if (bits == 0)
        bits = WHYNTER_BITS;
      irsend.sendWhynter(code, bits, repeat);
      break;
    case AIWA_RC_T501:  // 9
      if (bits == 0)
        bits = AIWA_RC_T501_BITS;
      repeat = std::max(repeat, (uint16_t) AIWA_RC_T501_MIN_REPEAT);
      irsend.sendAiwaRCT501(code, bits, repeat);
      break;
    case LG:  // 10
      if (bits == 0)
        bits = LG_BITS;
      irsend.sendLG(code, bits, repeat);
      break;
    case MITSUBISHI:  // 12
      if (bits == 0)
        bits = MITSUBISHI_BITS;
      repeat = std::max(repeat, (uint16_t) MITSUBISHI_MIN_REPEAT);
      irsend.sendMitsubishi(code, bits, repeat);
      break;
    case DISH:  // 13
      if (bits == 0)
        bits = DISH_BITS;
      repeat = std::max(repeat, (uint16_t) DISH_MIN_REPEAT);
      irsend.sendDISH(code, bits, repeat);
      break;
    case SHARP:  // 14
      if (bits == 0)
        bits = SHARP_BITS;
      irsend.sendSharpRaw(code, bits, repeat);
      break;
    case COOLIX:  // 15
      if (bits == 0)
        bits = COOLIX_BITS;
      irsend.sendCOOLIX(code, bits, repeat);
      break;
    case DENON:  // 17
      if (bits == 0)
        bits = DENON_BITS;
      irsend.sendDenon(code, bits, repeat);
      break;
    case KELVINATOR:  // 18
      parseStringAndSendKelv(code_str);
      break;
    case SHERWOOD:  // 19
      if (bits == 0)
        bits = SHERWOOD_BITS;
      repeat = std::max(repeat, (uint16_t) SHERWOOD_MIN_REPEAT);
      irsend.sendSherwood(code, bits, repeat);
      break;
    case RCMM:  // 21
      if (bits == 0)
        bits == RCMM_BITS;
      irsend.sendRCMM(code, bits, repeat);
      break;
    case SANYO_LC7461:  // 22
      if (bits == 0)
        bits = SANYO_LC7461_BITS;
      irsend.sendSanyoLC7461(code, bits, repeat);
      break;
    case RC5X:  // 23
      if (bits == 0)
        bits = RC5X_BITS;
      irsend.sendRC5(code, bits, repeat);
    case NIKAI:  // 29
      if (bits == 0)
        bits = NIKAI_BITS;
      irsend.sendNikai(code, bits, repeat);
      break;
    case GLOBALCACHE:  // 31
      parseStringAndSendGC(code_str);
      break;
  }

  // Release the lock.
  ir_lock = false;

  // Indicate that we sent the message.
  debug("Sent the IR message.");
  debug("Type: " + String(ir_type));
  switch (ir_type) {
    case KELVINATOR:
    case 31:  // GlobalCache
      debug("Code: ");
      debug(code_str);
      // Confirm what we were asked to send was sent.
  #ifdef MQTT_ENABLE
      mqtt_client.publish(MQTTack, (String(ir_type) + "," +
                                    String(code_str)).c_str());
  #endif
      break;
    default:
      debug("Code: 0x" + uint64ToString(code, 16));
      debug("Bits: " + String(bits));
      debug("Repeats: " + String(repeat));

#ifdef MQTT_ENABLE
      mqtt_client.publish(MQTTack, (String(ir_type) + "," +
                                    uint64ToString(code, 16)
                                    + "," + String(bits) + "," +
                                    String(repeat)).c_str());
#endif
  }
}

#ifdef MQTT_ENABLE
void receivingMQTT(String const topic_name, String const callback_str) {
  char* tok_ptr;
  uint64_t code = 0;
  uint16_t nbits = 0;
  uint16_t repeat = 0;

  debug("Receiving data by MQTT topic " + topic_name);

  // Make a copy of the callback string as strtok destorys it.
  char* callback_c_str = strdup(callback_str.c_str());
  debug("MQTT Payload (raw): " + callback_str);

  // Get the numeric protocol type.
  int ir_type = strtoul(strtok_r(callback_c_str, ",", &tok_ptr), NULL, 10);
  char* next = strtok_r(NULL, ",", &tok_ptr);
  // If there is unparsed string left, try to convert it assuming it's hex.
  if (next != NULL) {
    code = getUInt64fromHex(next);
    next = strtok_r(NULL, ",", &tok_ptr);
  } else {
    // We require at least two value in the string. Give up.
    return;
  }
  // If there is still string left, assume it is the bit size.
  if (next != NULL) {
    nbits = atoi(next);
    next = strtok_r(NULL, ",", &tok_ptr);
  }
  // If there is still string left, assume it is the repeat count.
  if (next != NULL)
    repeat = atoi(next);

  free(callback_c_str);


  // send received MQTT value by IR signal
  sendIRCode(ir_type, code,
             callback_str.substring(callback_str.indexOf(",") + 1).c_str(),
             nbits, repeat);
}

// Callback function, when the gateway receive an MQTT value on the topics
// subscribed this function is called
void callback(char* topic, byte* payload, unsigned int length) {
  // In order to republish this payload, a copy must be made
  // as the orignal payload buffer will be overwritten whilst
  // constructing the PUBLISH packet.
  // Allocate the correct amount of memory for the payload copy
  byte* payload_copy = reinterpret_cast<byte*>(malloc(length));
  // Copy the payload to the new buffer
  memcpy(payload_copy, payload, length);

  // Conversion to a printable string
  payload_copy[length] = '\0';
  String callback_string = String(reinterpret_cast<char*>(payload_copy));
  String topic_name = String(reinterpret_cast<char*>(topic));

  // launch the function to treat received data
  receivingMQTT(topic_name, callback_string);

  // Free the memory
  free(payload_copy);
}
#endif
