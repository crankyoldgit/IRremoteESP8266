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
#include "ir_Argo.h"
#include "ir_Coolix.h"
#include "ir_Daikin.h"
#include "ir_Fujitsu.h"
#include "ir_Haier.h"
#include "ir_Hitachi.h"
#include "ir_Kelvinator.h"
#include "ir_Midea.h"
#include "ir_Mitsubishi.h"
#include "ir_Panasonic.h"
#include "ir_Samsung.h"
#include "ir_Tcl.h"
#include "ir_Teco.h"
#include "ir_Toshiba.h"
#include "ir_Trotec.h"
#include "ir_Vestel.h"
#include "ir_Whirlpool.h"

IRac::IRac(uint8_t pin) { _pin = pin; }

#if SEND_ARGO
void IRac::argo(IRArgoAC *ac,
                bool on, stdAc::opmode_t mode, float degrees,
                stdAc::fanspeed_t fan, stdAc::swingv_t swingv,
                bool turbo, int16_t sleep) {
  ac->setPower(on);
  switch (mode) {
    case stdAc::opmode_t::kCool:
      ac->setCoolMode(kArgoCoolOn);
      break;
    case stdAc::opmode_t::kHeat:
      ac->setHeatMode(kArgoHeatOn);
      break;
    case stdAc::opmode_t::kDry:
      ac->setCoolMode(kArgoCoolHum);
      break;
    default:  // No idea how to set Fan mode.
      ac->setCoolMode(kArgoCoolAuto);
  }
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  ac->setFlap(ac->convertSwingV(swingv));
  // No Quiet setting available.
  // No Light setting available.
  // No Filter setting available.
  ac->setMax(turbo);
  // No Economy setting available.
  // No Clean setting available.
  // No Beep setting available.
  ac->setNight(sleep >= 0);  // Convert to a boolean.
  ac->send();
}
#endif  // SEND_ARGO

#if SEND_COOLIX
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
#endif  // SEND_COOLIX

#if SEND_DAIKIN
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
#endif  // SEND_DAIKIN

#if SEND_DAIKIN2
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
  if (clock >= 0) ac->setCurrentTime(clock);
  ac->send();
}
#endif  // SEND_DAIKIN2

#if SEND_FUJITSU_AC
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
#endif  // SEND_FUJITSU_AC

#if SEND_GREE
void IRac::gree(IRGreeAC *ac,
                bool on, stdAc::opmode_t mode, float degrees,
                stdAc::fanspeed_t fan, stdAc::swingv_t swingv,
                bool turbo, bool light, bool clean, int16_t sleep) {
  ac->setPower(on);
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  ac->setSwingVertical(swingv == stdAc::swingv_t::kAuto,  // Set auto flag.
                       ac->convertSwingV(swingv));
  ac->setLight(light);
  ac->setTurbo(turbo);
  ac->setXFan(clean);
  ac->setSleep(sleep >= 0);  // Sleep on this A/C is either on or off.
  // No Horizontal Swing setting available.
  // No Filter setting available.
  // No Beep setting available.
  // No Quiet setting available.
  // No Clock setting available.
  ac->send();
}
#endif  // SEND_GREE

#if SEND_HAIER_AC
void IRac::haier(IRHaierAC *ac,
                 bool on, stdAc::opmode_t mode, float degrees,
                 stdAc::fanspeed_t fan, stdAc::swingv_t swingv,
                 bool filter, int16_t sleep, int16_t clock) {
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  ac->setSwing(ac->convertSwingV(swingv));
  // No Horizontal Swing setting available.
  // No Quiet setting available.
  // No Turbo setting available.
  // No Light setting available.
  ac->setHealth(filter);
  // No Clean setting available.
  // No Beep setting available.
  ac->setSleep(sleep >= 0);  // Sleep on this A/C is either on or off.
  if (clock >=0) ac->setCurrTime(clock);
  if (on)
    ac->setCommand(kHaierAcCmdOn);
  else
    ac->setCommand(kHaierAcCmdOff);
  ac->send();
}
#endif  // SEND_HAIER_AC

#if SEND_HAIER_AC_YRW02
void IRac::haierYrwo2(IRHaierACYRW02 *ac,
                      bool on, stdAc::opmode_t mode, float degrees,
                      stdAc::fanspeed_t fan, stdAc::swingv_t swingv,
                      bool turbo, bool filter, int16_t sleep) {
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  ac->setSwing(ac->convertSwingV(swingv));
  // No Horizontal Swing setting available.
  // No Quiet setting available.
  ac->setTurbo(turbo);
  // No Light setting available.
  ac->setHealth(filter);
  // No Clean setting available.
  // No Beep setting available.
  ac->setSleep(sleep >= 0);  // Sleep on this A/C is either on or off.
  ac->setPower(on);
  ac->send();
}
#endif  // SEND_HAIER_AC_YRW02

#if SEND_HITACHI_AC
void IRac::hitachi(IRHitachiAc *ac,
                   bool on, stdAc::opmode_t mode, float degrees,
                   stdAc::fanspeed_t fan,
                   stdAc::swingv_t swingv, stdAc::swingh_t swingh) {
  ac->setPower(on);
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  ac->setSwingVertical(swingv != stdAc::swingv_t::kOff);
  ac->setSwingHorizontal(swingh != stdAc::swingh_t::kOff);
  // No Quiet setting available.
  // No Turbo setting available.
  // No Light setting available.
  // No Filter setting available.
  // No Clean setting available.
  // No Beep setting available.
  // No Sleep setting available.
  // No Clock setting available.
  ac->send();
}
#endif  // SEND_HITACHI_AC

#if SEND_KELVINATOR
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
#endif  // SEND_KELVINATOR

#if SEND_MIDEA
void IRac::midea(IRMideaAC *ac,
                 bool on, stdAc::opmode_t mode, float degrees,
                 stdAc::fanspeed_t fan, int16_t sleep) {
  ac->setPower(on);
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees, true);  // true means use Celsius.
  ac->setFan(ac->convertFan(fan));
  // No Vertical swing setting available.
  // No Horizontal swing setting available.
  // No Quiet setting available.
  // No Turbo setting available.
  // No Light setting available.
  // No Filter setting available.
  // No Clean setting available.
  // No Beep setting available.
  ac->setSleep(sleep >= 0);  // Sleep on this A/C is either on or off.
  // No Clock setting available.
  ac->send();
}
#endif  // SEND_MIDEA

#if SEND_MITSUBISHI_AC
void IRac::mitsubishi(IRMitsubishiAC *ac,
                      bool on, stdAc::opmode_t mode, float degrees,
                      stdAc::fanspeed_t fan, stdAc::swingv_t swingv,
                      bool quiet, int16_t clock) {
  ac->setPower(on);
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  ac->setVane(ac->convertSwingV(swingv));
  // No Horizontal swing setting available.
  if (quiet) ac->setFan(kMitsubishiAcFanSilent);
  // No Turbo setting available.
  // No Light setting available.
  // No Filter setting available.
  // No Clean setting available.
  // No Beep setting available.
  // No Sleep setting available.
  if (clock >= 0) ac->setClock(clock / 10);  // Clock is in 10 min increments.
  ac->send();
}
#endif  // SEND_MITSUBISHI_AC

#if SEND_PANASONIC_AC
void IRac::panasonic(IRPanasonicAc *ac, panasonic_ac_remote_model_t model,
                     bool on, stdAc::opmode_t mode, float degrees,
                     stdAc::fanspeed_t fan,
                     stdAc::swingv_t swingv, stdAc::swingh_t swingh,
                     bool quiet, bool turbo, int16_t clock) {
  ac->setModel(model);
  ac->setPower(on);
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  ac->setSwingVertical(ac->convertSwingV(swingv));
  ac->setSwingHorizontal(ac->convertSwingH(swingh));
  ac->setQuiet(quiet);
  ac->setPowerful(turbo);
  // No Light setting available.
  // No Econo setting available.
  // No Filter setting available.
  // No Clean setting available.
  // No Beep setting available.
  // No Sleep setting available.
  if (clock >= 0) ac->setClock(clock);
  ac->send();
}
#endif  // SEND_PANASONIC_AC

#if SEND_SAMSUNG_AC
void IRac::samsung(IRSamsungAc *ac,
                   bool on, stdAc::opmode_t mode, float degrees,
                   stdAc::fanspeed_t fan, stdAc::swingv_t swingv,
                   bool quiet, bool turbo, bool clean, bool beep,
                   bool sendOnOffHack) {
  if (sendOnOffHack) {
    // Use a hack to for the unit on or off.
    // See: https://github.com/markszabo/IRremoteESP8266/issues/604#issuecomment-475020036
    if (on)
      ac->sendOn();
    else
      ac->sendOff();
  }
  ac->setPower(on);
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  ac->setSwing(swingv != stdAc::swingv_t::kOff);
  // No Horizontal swing setting available.
  ac->setQuiet(quiet);
  if (turbo) ac->setFan(kSamsungAcFanTurbo);
  // No Light setting available.
  // No Econo setting available.
  // No Filter setting available.
  ac->setClean(clean);
  ac->setBeep(beep);
  // No Sleep setting available.
  // No Clock setting available.
  // Do setMode() again as it can affect fan speed.
  ac->setMode(ac->convertMode(mode));
  ac->send();
}
#endif  // SEND_SAMSUNG_AC

#if SEND_TCL112AC
void IRac::tcl112(IRTcl112Ac *ac,
                  bool on, stdAc::opmode_t mode, float degrees,
                  stdAc::fanspeed_t fan,
                  stdAc::swingv_t swingv, stdAc::swingh_t swingh,
                  bool turbo, bool light, bool econo, bool filter) {
  ac->setPower(on);
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  ac->setSwingVertical(swingv != stdAc::swingv_t::kOff);
  ac->setSwingHorizontal(swingh != stdAc::swingh_t::kOff);
  // No Quiet setting available.
  ac->setTurbo(turbo);
  ac->setLight(light);
  ac->setEcono(econo);
  ac->setHealth(filter);
  // No Clean setting available.
  // No Beep setting available.
  // No Sleep setting available.
  // No Clock setting available.
  ac->send();
}
#endif  // SEND_TCL112AC

#if SEND_TECO
void IRac::teco(IRTecoAc *ac,
                bool on, stdAc::opmode_t mode, float degrees,
                stdAc::fanspeed_t fan, stdAc::swingv_t swingv, int16_t sleep) {
  ac->setPower(on);
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  ac->setSwing(swingv != stdAc::swingv_t::kOff);
  // No Horizontal swing setting available.
  // No Quiet setting available.
  // No Turbo setting available.
  // No Light setting available.
  // No Filter setting available.
  // No Clean setting available.
  // No Beep setting available.
  ac->setSleep(sleep >= 0);  // Sleep is either on/off, so convert to boolean.
  // No Clock setting available.
  ac->send();
}
#endif  // SEND_TECO

#if SEND_TOSHIBA_AC
void IRac::toshiba(IRToshibaAC *ac,
                   bool on, stdAc::opmode_t mode, float degrees,
                   stdAc::fanspeed_t fan) {
  ac->setPower(on);
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  // No Vertical swing setting available.
  // No Horizontal swing setting available.
  // No Quiet setting available.
  // No Turbo setting available.
  // No Light setting available.
  // No Filter setting available.
  // No Clean setting available.
  // No Beep setting available.
  // No Sleep setting available.
  // No Clock setting available.
  ac->send();
}
#endif  // SEND_TOSHIBA_AC

#if SEND_TROTEC
void IRac::trotec(IRTrotecESP *ac,
                  bool on, stdAc::opmode_t mode, float degrees,
                  stdAc::fanspeed_t fan, int16_t sleep) {
  ac->setPower(on);
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setSpeed(ac->convertFan(fan));
  // No Vertical swing setting available.
  // No Horizontal swing setting available.
  // No Quiet setting available.
  // No Turbo setting available.
  // No Light setting available.
  // No Filter setting available.
  // No Clean setting available.
  // No Beep setting available.
  ac->setSleep(sleep >= 0);  // Sleep is either on/off, so convert to boolean.
  // No Clock setting available.
  ac->send();
}
#endif  // SEND_TROTEC

#if SEND_VESTEL_AC
void IRac::vestel(IRVestelAc *ac,
                  bool on, stdAc::opmode_t mode, float degrees,
                  stdAc::fanspeed_t fan, stdAc::swingv_t swingv,
                  bool turbo, bool filter, int16_t sleep, int16_t clock,
                  bool sendNormal) {
  ac->setPower(on);
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  ac->setSwing(swingv != stdAc::swingv_t::kOff);
  // No Horizontal swing setting available.
  // No Quiet setting available.
  ac->setTurbo(turbo);
  // No Light setting available.
  ac->setIon(filter);
  // No Clean setting available.
  // No Beep setting available.
  ac->setSleep(sleep >= 0);  // Sleep is either on/off, so convert to boolean.
  if (sendNormal) ac->send();  // Send the normal message.
  if (clock >= 0) {
    ac->setTime(clock);
    ac->send();  // Setting the clock requires a different "timer" message.
  }
}
#endif  // SEND_VESTEL_AC

#if SEND_WHIRLPOOL_AC
void IRac::whirlpool(IRWhirlpoolAc *ac, whirlpool_ac_remote_model_t model,
                     bool on, stdAc::opmode_t mode, float degrees,
                     stdAc::fanspeed_t fan, stdAc::swingv_t swingv,
                     bool turbo, bool light, int16_t sleep, int16_t clock) {
  ac->setModel(model);
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  ac->setSwing(swingv != stdAc::swingv_t::kOff);
  // No Horizontal swing setting available.
  // No Quiet setting available.
  ac->setSuper(turbo);
  ac->setLight(light);
  // No Filter setting available
  // No Clean setting available.
  // No Beep setting available.
  ac->setSleep(sleep >= 0);  // Sleep is either on/off, so convert to boolean.
  if (clock >= 0) ac->setClock(clock);
  ac->setPowerToggle(on);
  ac->send();
}
#endif  // SEND_WHIRLPOOL_AC

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
#if SEND_ARGO
    case ARGO:
    {
      IRArgoAC ac(_pin);
      argo(&ac, on, mode, degC, fan, swingv, turbo, sleep);
      break;
    }
#endif  // SEND_DAIKIN
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
#if SEND_GREE
    case GREE:
    {
      IRGreeAC ac(_pin);
      ac.begin();
      gree(&ac, on, mode, degC, fan, swingv, light, turbo, clean, sleep);
      break;
    }
#endif  // SEND_GREE
#if SEND_HAIER_AC
    case HAIER_AC:
    {
      IRHaierAC ac(_pin);
      ac.begin();
      haier(&ac, on, mode, degC, fan, swingv, filter, sleep, clock);
      break;
    }
#endif  // SEND_HAIER_AC
#if SEND_HAIER_AC_YRW02
    case HAIER_AC_YRW02:
    {
      IRHaierACYRW02 ac(_pin);
      ac.begin();
      haierYrwo2(&ac, on, mode, degC, fan, swingv, turbo, filter, sleep);
      break;
    }
#endif  // SEND_HAIER_AC_YRW02
#if SEND_HITACHI_AC
    case HITACHI_AC:
    {
      IRHitachiAc ac(_pin);
      ac.begin();
      hitachi(&ac, on, mode, degC, fan, swingv, swingh);
      break;
    }
#endif  // SEND_HITACHI_AC
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
#if SEND_MIDEA
    case MIDEA:
    {
      IRMideaAC ac(_pin);
      ac.begin();
      midea(&ac, on, mode, degC, fan, sleep);
      break;
    }
#endif  // SEND_MIDEA
#if SEND_MITSUBISHI_AC
    case MITSUBISHI_AC:
    {
      IRMitsubishiAC ac(_pin);
      ac.begin();
      mitsubishi(&ac, on, mode, degC, fan, swingv, quiet, clock);
      break;
    }
#endif  // SEND_MITSUBISHI_AC
#if SEND_PANASONIC_AC
    case PANASONIC_AC:
    {
      IRPanasonicAc ac(_pin);
      ac.begin();
      panasonic(&ac, (panasonic_ac_remote_model_t)model, on, mode, degC, fan,
                swingv, swingh, quiet, turbo, clock);
      break;
    }
#endif  // SEND_PANASONIC_AC
#if SEND_SAMSUNG_AC
    case SAMSUNG_AC:
    {
      IRSamsungAc ac(_pin);
      ac.begin();
      samsung(&ac, on, mode, degC, fan, swingv, quiet, turbo, clean, beep);
      break;
    }
#endif  // SEND_SAMSUNG_AC
#if SEND_TCL112AC
    case TCL112AC:
    {
      IRTcl112Ac ac(_pin);
      ac.begin();
      tcl112(&ac, on, mode, degC, fan, swingv, swingh, turbo, light, econo,
             filter);
      break;
    }
#endif  // SEND_TCL112AC
#if SEND_TECO
    case TECO:
    {
      IRTecoAc ac(_pin);
      ac.begin();
      teco(&ac, on, mode, degC, fan, swingv, sleep);
      break;
    }
#endif  // SEND_TECO
#if SEND_TOSHIBA_AC
    case TOSHIBA_AC:
    {
      IRToshibaAC ac(_pin);
      ac.begin();
      toshiba(&ac, on, mode, degC, fan);
      break;
    }
#endif  // SEND_TOSHIBA_AC
#if SEND_TROTEC
    case TROTEC:
    {
      IRTrotecESP ac(_pin);
      ac.begin();
      trotec(&ac, on, mode, degC, fan, sleep);
      break;
    }
#endif  // SEND_TROTEC
#if SEND_VESTEL_AC
    case VESTEL_AC:
    {
      IRVestelAc ac(_pin);
      ac.begin();
      vestel(&ac, on, mode, degC, fan, swingv, turbo, filter, sleep, clock);
      break;
    }
#endif  // SEND_VESTEL_AC
#if SEND_WHIRLPOOL_AC
    case WHIRLPOOL_AC:
    {
      IRWhirlpoolAc ac(_pin);
      ac.begin();
      whirlpool(&ac, (whirlpool_ac_remote_model_t)model, on, mode, degC, fan,
                swingv, turbo, light, sleep, clock);
      break;
    }
#endif  // SEND_WHIRLPOOL_AC
    default:
      return false;  // Fail, didn't match anything.
  }
  return true;  // Success.
}
