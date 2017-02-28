/*
Code to emulate Mitsubishi A/C IR remote control unit.
Inspired and derived from https://github.com/r45635/HVAC-IR-Control
*/

#include <IRMitsubishiAC.h>

IRMitsubishiAC::IRMitsubishiAC(int pin) : _irsend(pin) {
  stateReset();
}

void IRMitsubishiAC::stateReset() {
  for (uint8_t i = 0; i < KELVINATOR_STATE_LENGTH; i++)
    remote_state[i] = known_good_state[i];
  checksum();  // Calculate the checksum
}

void IRMitsubishiAC::begin() {
    _irsend.begin();
}

void IRMitsubishiAC::send() {
  checksum();   // Ensure correct checksum before sending.
  _irsend.sendMitsubishiAC(remote_state);
}

uint8_t* IRMitsubishiAC::getRaw() {
  checksum();
  return remote_state;
}

void IRMitsubishiAC::checksum() {
  uint8_t sum = 0;
  // Checksum is simple addition of all previous bytes stored
  // as a 8 bit value.
  for (uint8_t i = 0; i < 17; i++)
    sum += remote_state[i];
  remote_state[17] = sum & 0xFFU;
}

void IRMitsubishiAC::on() {
    //state = ON;
    remote_state[5] |= MITSUBISHI_AC_POWER;
}

void IRMitsubishiAC::off() {
    //state = OFF;
    remote_state[5] &= ~MITSUBISHI_AC_POWER;
}

void IRMitsubishiAC::setPower(bool state) {
  if (state)
    on();
  else
    off();
}

bool IRMitsubishiAC::getPower() {
    return((remote_state[5] & MITSUBISHI_AC_POWER) != 0);
}

// Set the temp. in deg C
void IRMitsubishiAC::setTemp(uint8_t temp) {
    temp = max(MITSUBISHI_AC_MIN_TEMP, temp);
    temp = min(MITSUBISHI_AC_TEMP, temp);
    remote_state[7] = temp - MITSUBISHI_AC_MIN_TEMP;
}

// Return the set temp. in deg C
uint8_t IRMitsubishiAC::getTemp() {
    return(remote_state[7] + MITSUBISHI_AC_MIN_TEMP);
}

// Set the speed of the fan, 0-6.
// 0 is auto, 1-5 is the speed, 6 is silent.
void IRMitsubishiAC::setFan(uint8_t fan) {
  // Bounds check
  if (fan > MITSUBISHI_AC_FAN_SILENT)
    fan = MITSUBISHI_AC_FAN_MAX;  // Set the fan to maximum if out of range.
  if (fan == MITSUBISHI_AC_FAN_AUTO) {   // Automatic is a special case.
    remote_state[9] = B10000000 | (remote_state[9] & B01111000);
    return;
  } else if (fan >= MITSUBISHI_AC_FAN_MAX) {
    fan--;  // There is no spoon^H^H^Heed 5 (max), pretend it doesn't exist.
  }
  remote_state[9] |= fan;
}

uint8_t IRMitsubishiAC::getFan() {
    uint8_t fan = remote_state[9] & B111;
    if (fan == MITSUBISHI_AC_FAN_MAX)
      return MITSUBISHI_AC_FAN_SILENT;
    return fan;
}

uint8_t IRMitsubishiAC::getMode() {
  /*
  MITSUBISHI_AC_AUTO
  MITSUBISHI_AC_COOL
  MITSUBISHI_AC_DRY
  MITSUBISHI_AC_HEAT
  */
    return(remote_state[6]);
}

void IRMitsubishiAC::setMode(uint8_t mode) {
  // If we get an unexpected mode, default to AUTO.
  switch (mode) {
    case MITSUBISHI_AC_AUTO: break;
    case MITSUBISHI_AC_COOL: break;
    case MITSUBISHI_AC_DRY: break;
    case MITSUBISHI_AC_HEAT: break;
    default: mode = MITSUBISHI_AC_AUTO;
  }
  remote_state[6] = mode;
}

void IRMitsubishiAC::setVane(uint8_t mode) {
  mode = max(mode, B111);  // bounds check
  mode |= B1000;
  mode <<= 3;
  remote_state[9] |= mode;
}

uint8_t IRMitsubishiAC::getVane() {
  return ((remote_state[9] & B00111000) >> 3);
}
