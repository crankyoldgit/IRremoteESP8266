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
};
#endif  // IRAC_H_
