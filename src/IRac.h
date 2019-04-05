#ifndef IRAC_H_
#define IRAC_H_

// Copyright 2019 David Conran

#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#ifndef ARDUINO
#include <string>
#endif
#include "IRremoteESP8266.h"
#include "ir_Argo.h"
#include "ir_Coolix.h"
#include "ir_Daikin.h"
#include "ir_Fujitsu.h"
#include "ir_Gree.h"
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

class IRac {
 public:
  explicit IRac(uint8_t pin);
  bool sendAc(decode_type_t vendor, uint16_t model,
              bool on, stdAc::opmode_t mode, float degrees, bool celsius,
              stdAc::fanspeed_t fan,
              stdAc::swingv_t swingv, stdAc::swingh_t swingh,
              bool quiet, bool turbo, bool econo, bool light,
              bool filter, bool clean, bool beep,
              int16_t sleep, int32_t clock);
#ifndef UNIT_TEST

 private:
#endif
  uint8_t _pin;
#if SEND_ARGO
  void argo(IRArgoAC *ac,
            bool on, stdAc::opmode_t mode, float degrees, stdAc::fanspeed_t fan,
            stdAc::swingv_t swingv, bool turbo, int16_t sleep = -1);
#endif  // SEND_ARGO
#if SEND_COOLIX
  void coolix(IRCoolixAC *ac,
              bool on, stdAc::opmode_t mode, float degrees,
              stdAc::fanspeed_t fan,
              stdAc::swingv_t swingv, stdAc::swingh_t swingh,
              bool turbo, bool light, bool clean, int16_t sleep = -1);
#endif  // SEND_COOLIX
#if SEND_DAIKIN
  void daikin(IRDaikinESP *ac,
              bool on, stdAc::opmode_t mode, float degrees,
              stdAc::fanspeed_t fan,
              stdAc::swingv_t swingv, stdAc::swingh_t swingh,
              bool quiet, bool turbo, bool econo, bool clean);
#endif  // SEND_DAIKIN
#if SEND_DAIKIN2
  void daikin2(IRDaikin2 *ac,
               bool on, stdAc::opmode_t mode, float degrees,
               stdAc::fanspeed_t fan,
               stdAc::swingv_t swingv, stdAc::swingh_t swingh,
               bool quiet, bool turbo, bool light, bool econo, bool filter,
               bool clean, bool beep, int16_t sleep = -1, int32_t clock = -1);
#endif  // SEND_DAIKIN2
#if SEND_FUJITSU_AC
  void fujitsu(IRFujitsuAC *ac, fujitsu_ac_remote_model_t model,
               bool on, stdAc::opmode_t mode, float degrees,
               stdAc::fanspeed_t fan,
               stdAc::swingv_t swingv, stdAc::swingh_t swingh,
               bool quiet);
#endif  // SEND_FUJITSU_AC
#if SEND_GREE
  void gree(IRGreeAC *ac,
            bool on, stdAc::opmode_t mode, float degrees,
            stdAc::fanspeed_t fan, stdAc::swingv_t swingv,
            bool turbo, bool light, bool clean, int16_t sleep);
#endif  // SEND_GREE
#if SEND_HAIER_AC
  void haier(IRHaierAC *ac,
             bool on, stdAc::opmode_t mode, float degrees,
             stdAc::fanspeed_t fan, stdAc::swingv_t swingv,
             bool filter, int16_t sleep = -1, int16_t clock = -1);
#endif  // SEND_HAIER_AC
#if SEND_HAIER_AC_YRW02
  void haierYrwo2(IRHaierACYRW02 *ac,
                  bool on, stdAc::opmode_t mode, float degrees,
                  stdAc::fanspeed_t fan, stdAc::swingv_t swingv,
                  bool turbo, bool filter, int16_t sleep = -1);
#endif  // SEND_HAIER_AC_YRW02
#if SEND_HITACHI_AC
  void hitachi(IRHitachiAc *ac,
               bool on, stdAc::opmode_t mode, float degrees,
               stdAc::fanspeed_t fan,
               stdAc::swingv_t swingv, stdAc::swingh_t swingh);
#endif  // SEND_HITACHI_AC
#if SEND_KELVINATOR
  void kelvinator(IRKelvinatorAC *ac,
                  bool on, stdAc::opmode_t mode, float degrees,
                  stdAc::fanspeed_t fan,
                  stdAc::swingv_t swingv, stdAc::swingh_t swingh,
                  bool quiet, bool turbo, bool light,
                  bool filter, bool clean);
#endif  // SEND_KELVINATOR
#if SEND_MIDEA
  void midea(IRMideaAC *ac,
             bool on, stdAc::opmode_t mode, float degrees,
             stdAc::fanspeed_t fan, int16_t sleep);
#endif  // SEND_MIDEA
#if SEND_MITSUBISHI_AC
  void mitsubishi(IRMitsubishiAC *ac,
                  bool on, stdAc::opmode_t mode, float degrees,
                  stdAc::fanspeed_t fan, stdAc::swingv_t swingv,
                  bool quiet, int16_t clock = -1);
#endif  // SEND_MITSUBISHI_AC
#if SEND_PANASONIC_AC
  void panasonic(IRPanasonicAc *ac, panasonic_ac_remote_model_t model,
                 bool on, stdAc::opmode_t mode, float degrees,
                 stdAc::fanspeed_t fan,
                 stdAc::swingv_t swingv, stdAc::swingh_t swingh,
                 bool quiet, bool turbo, int16_t clock = -1);
#endif  // SEND_PANASONIC_AC
#if SEND_SAMSUNG_AC
  void samsung(IRSamsungAc *ac,
               bool on, stdAc::opmode_t mode, float degrees,
               stdAc::fanspeed_t fan, stdAc::swingv_t swingv,
               bool quiet, bool turbo, bool clean, bool beep,
               bool sendOnOffHack = true);
#endif  // SEND_SAMSUNG_AC
#if SEND_TCL112AC
  void tcl112(IRTcl112Ac *ac,
              bool on, stdAc::opmode_t mode, float degrees,
              stdAc::fanspeed_t fan,
              stdAc::swingv_t swingv, stdAc::swingh_t swingh,
              bool turbo, bool light, bool econo, bool filter);
#endif  // SEND_TCL112AC
#if SEND_TECO
  void teco(IRTecoAc *ac,
            bool on, stdAc::opmode_t mode, float degrees,
            stdAc::fanspeed_t fan, stdAc::swingv_t swingv, int16_t sleep = -1);
#endif  // SEND_TECO
#if SEND_TOSHIBA_AC
  void toshiba(IRToshibaAC *ac,
               bool on, stdAc::opmode_t mode, float degrees,
               stdAc::fanspeed_t fan);
#endif  // SEND_TOSHIBA_AC
#if SEND_TROTEC
  void trotec(IRTrotecESP *ac,
              bool on, stdAc::opmode_t mode, float degrees,
              stdAc::fanspeed_t fan, int16_t sleep = -1);
#endif  // SEND_TROTEC
#if SEND_VESTEL_AC
  void vestel(IRVestelAc *ac,
              bool on, stdAc::opmode_t mode, float degrees,
              stdAc::fanspeed_t fan, stdAc::swingv_t swingv,
              bool turbo, bool filter, int16_t sleep = -1, int16_t clock = -1,
              bool sendNormal = true);
#endif  // SEND_VESTEL_AC
#if SEND_WHIRLPOOL_AC
  void whirlpool(IRWhirlpoolAc *ac, whirlpool_ac_remote_model_t model,
                 bool on, stdAc::opmode_t mode, float degrees,
                 stdAc::fanspeed_t fan, stdAc::swingv_t swingv,
                 bool turbo, bool light,
                 int16_t sleep = -1, int16_t clock = -1);
#endif  // SEND_WHIRLPOOL_AC
};
#endif  // IRAC_H_
