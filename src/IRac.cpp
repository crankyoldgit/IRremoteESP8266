// Copyright 2019 David Conran

// Provide a universal/standard interface for sending A/C nessages.
// It does not provide complete and maximum granular control but tries
// to off most common functionallity across all supported devices.

#include "IRac.h"
#ifndef UNIT_TEST
#include <Arduino.h>
#endif

#ifndef ARDUINO
#include <string>
#endif
#include "IRsend.h"
#include "IRremoteESP8266.h"
#include "ir_Coolix.h"
#include "ir_Daikin.h"
#include "ir_Fujitsu.h"
#include "ir_Kelvinator.h"

IRac::IRac(uint8_t pin) { _pin = pin; }

void IRac::coolix(IRCoolixAC *ac,
                  bool on, stdAc::opmode_t mode, float degrees,
                  stdAc::fanspeed_t fan,
                  stdAc::swingv_t swingv, stdAc::swingh_t swingh,
                  bool turbo, bool light, bool clean, int16_t sleep) {
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  // No Filter setting available.
  // No Beep setting available.
  // No Clock setting available.
  // No Econo setting available.
  // No Quiet setting available.
  if (swingv != stdAc::swingv_t::kOff || swingh != stdAc::swingh_t::kOff) {
    // Swing has a special command that needs to be sent independently.
    ac->setSwing();
    ac->send();
  }
  if (turbo) {
    // Turbo has a special command that needs to be sent independently.
    ac->setTurbo();
    ac->send();
  }
  if (sleep > 0) {
    // Sleep has a special command that needs to be sent independently.
    ac->setSleep();
    ac->send();
  }
  if (light) {
    // Light has a special command that needs to be sent independently.
    ac->setLed();
    ac->send();
  }
  if (clean) {
    // Clean has a special command that needs to be sent independently.
    ac->setClean();
    ac->send();
  }
  // Power gets done last, as off has a special command.
  ac->setPower(on);
  ac->send();
}

void IRac::daikin(IRDaikinESP *ac,
                  bool on, stdAc::opmode_t mode, float degrees,
                  stdAc::fanspeed_t fan,
                  stdAc::swingv_t swingv, stdAc::swingh_t swingh,
                  bool quiet, bool turbo, bool econo, bool clean) {
  ac->setPower(on);
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  ac->setSwingVertical((int8_t)swingv >= 0);
  ac->setSwingHorizontal((int8_t)swingh >= 0);
  ac->setQuiet(quiet);
  // No Light setting available.
  // No Filter setting available.
  ac->setPowerful(turbo);
  ac->setEcono(econo);
  ac->setMold(clean);
  // No Beep setting available.
  // No Sleep setting available.
  // No Clock setting available.
  ac->send();
}

void IRac::daikin2(IRDaikin2 *ac,
                   bool on, stdAc::opmode_t mode, float degrees,
                   stdAc::fanspeed_t fan,
                   stdAc::swingv_t swingv, stdAc::swingh_t swingh,
                   bool quiet, bool turbo, bool light, bool econo, bool filter,
                   bool clean, bool beep, int16_t sleep, int32_t clock) {
  ac->setPower(on);
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  ac->setSwingVertical(ac->convertSwingV(swingv));
  ac->setSwingHorizontal((int8_t)swingh >= 0);
  ac->setQuiet(quiet);
  ac->setLight(light);
  ac->setPowerful(turbo);
  ac->setEcono(econo);
  ac->setPurify(filter);
  ac->setMold(clean);
  ac->setBeep(beep);
  if (sleep > 0) ac->enableSleepTimer(sleep);
  if (clock >= 0) ac->setCurrentTime(sleep);
  ac->send();
}

void IRac::fujitsu(IRFujitsuAC *ac, fujitsu_ac_remote_model_t model,
                   bool on, stdAc::opmode_t mode, float degrees,
                   stdAc::fanspeed_t fan,
                   stdAc::swingv_t swingv, stdAc::swingh_t swingh,
                   bool quiet) {
  ac->setModel(model);
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFanSpeed(ac->convertFan(fan));
  uint8_t swing = kFujitsuAcSwingOff;
  if (swingv > stdAc::swingv_t::kOff) swing |= kFujitsuAcSwingVert;
  if (swingh > stdAc::swingh_t::kOff) swing |= kFujitsuAcSwingHoriz;
  ac->setSwing(swing);
  if (quiet) ac->setFanSpeed(kFujitsuAcFanQuiet);
  // No Turbo setting available.
  // No Light setting available.
  // No Econo setting available.
  // No Filter setting available.
  // No Clean setting available.
  // No Beep setting available.
  // No Sleep setting available.
  // No Clock setting available.
  if (!on) ac->off();
  ac->send();
}

void IRac::kelvinator(IRKelvinatorAC *ac,
                      bool on, stdAc::opmode_t mode, float degrees,
                      stdAc::fanspeed_t fan,
                      stdAc::swingv_t swingv, stdAc::swingh_t swingh,
                      bool quiet, bool turbo, bool light,
                      bool filter, bool clean) {
  ac->setPower(on);
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan((uint8_t)fan);  // No conversion needed.
  ac->setSwingVertical((int8_t)swingv >= 0);
  ac->setSwingHorizontal((int8_t)swingh >= 0);
  ac->setQuiet(quiet);
  ac->setTurbo(turbo);
  ac->setLight(light);
  ac->setIonFilter(filter);
  ac->setXFan(clean);
  // No Beep setting available.
  // No Sleep setting available.
  // No Clock setting available.
  ac->send();
}

// Send A/C message for a given device using common A/C settings.
// Args:
//   vendor:  The type of A/C protocol to use.
//   model:   The specific model of A/C if applicable.
//   on:      Should the unit be powered on?
//   mode:    What operating mode should the unit perform? e.g. Cool, Heat etc.
//   degrees: What temperature should the unit be set to?
//   celsius: Use degreees Celsius, otherwise Fahrenheit.
//   fan:     Fan speed.
//   swingv:  Control the vertical swing of the vanes.
//   swingh:  Control the horizontal swing of the vanes.
//   quiet:   Set the unit to quiet (fan) operation mode.
//   turbo:   Set the unit to turbo operating mode. e.g. Max fan & cooling etc.
//   econo:   Set the unit to economical operating mode.
//   light:   Turn on the display/LEDs etc.
//   filter:  Turn on any particle/ion filter etc.
//   clean:   Turn on any settings to reduce mold etc. (Not self-clean mode.)
//   beep:    Control if the unit beeps upon receiving commands.
//   sleep:   Nr. of mins of sleep mode, or use sleep mode. (<= 0 means off.)
//   clock:   Nr. of mins past midnight to set the clock to. (< 0 means off.)
// Returns:
//   boolean: True is accepted/converted/attempted. False is unsupported.
bool IRac::sendAc(decode_type_t vendor, uint16_t model,
                  bool on, stdAc::opmode_t mode, float degrees, bool celsius,
                  stdAc::fanspeed_t fan,
                  stdAc::swingv_t swingv, stdAc::swingh_t swingh,
                  bool quiet, bool turbo, bool econo, bool light,
                  bool filter, bool clean, bool beep,
                  int16_t sleep, int32_t clock) {
  // Convert the temperature to Celsius.
  float degC;
  if (celsius)
    degC = degrees;
  else
    degC = (degrees - 32.0) * (5.0 / 9.0);

  // Per vendor settings & setup.
  switch (vendor) {
#if SEND_COOLIX
    case COOLIX:
    {
      IRCoolixAC ac(_pin);
      coolix(&ac, on, mode, degC, fan, swingv, swingh,
             quiet, turbo, econo, clean);
      break;
    }
#endif  // SEND_DAIKIN
#if SEND_DAIKIN
    case DAIKIN:
    {
      IRDaikinESP ac(_pin);
      daikin(&ac, on, mode, degC, fan, swingv, swingh,
             quiet, turbo, econo, clean);
      break;
    }
#endif  // SEND_DAIKIN
#if SEND_DAIKIN2
    case DAIKIN2:
    {
      IRDaikin2 ac(_pin);
      daikin2(&ac, on, mode, degC, fan, swingv, swingh, quiet, turbo,
              light, econo, filter, clean, beep, sleep, clock);
      break;
    }
#endif  // SEND_DAIKIN2
#if SEND_FUJITSU_AC
    case FUJITSU_AC:
    {
      IRFujitsuAC ac(_pin);
      ac.begin();
      fujitsu(&ac, (fujitsu_ac_remote_model_t)model, on, mode, degC, fan,
              swingv, swingh, quiet);
      break;
    }
#endif  // SEND_FUJITSU_AC
#if SEND_KELVINATOR
    case KELVINATOR:
    {
      IRKelvinatorAC ac(_pin);
      ac.begin();
      kelvinator(&ac, on, mode, degC, fan, swingv, swingh, quiet, turbo,
                 light, filter, clean);
      break;
    }
#endif  // SEND_KELVINATOR
    default:
      return false;  // Fail, didn't match anything.
  }
  return true;  // Success.
}
