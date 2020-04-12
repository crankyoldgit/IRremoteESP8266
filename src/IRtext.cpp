// Copyright 2019 - David Conran (@crankyoldgit)

#ifndef UNIT_TEST
#include <Arduino.h>
#endif  // UNIT_TEST
#include "IRremoteESP8266.h"
#include "i18n.h"

#ifndef PROGMEM
#define PROGMEM  // Pretend we have the PROGMEM macro even if we really don't.
#endif

// Common

const PROGMEM char* kUnknownStr = D_STR_UNKNOWN;
const PROGMEM char* kProtocolStr = D_STR_PROTOCOL;
const PROGMEM char* kPowerStr = D_STR_POWER;
const PROGMEM char* kOnStr = D_STR_ON;
const PROGMEM char* kOffStr = D_STR_OFF;
const PROGMEM char* kModeStr = D_STR_MODE;
const PROGMEM char* kToggleStr = D_STR_TOGGLE;
const PROGMEM char* kTurboStr = D_STR_TURBO;
const PROGMEM char* kSuperStr = D_STR_SUPER;
const PROGMEM char* kSleepStr = D_STR_SLEEP;
const PROGMEM char* kLightStr = D_STR_LIGHT;
const PROGMEM char* kPowerfulStr = D_STR_POWERFUL;
const PROGMEM char* kQuietStr = D_STR_QUIET;
const PROGMEM char* kEconoStr = D_STR_ECONO;
const PROGMEM char* kSwingStr = D_STR_SWING;
const PROGMEM char* kSwingHStr = D_STR_SWINGH;
const PROGMEM char* kSwingVStr = D_STR_SWINGV;
const PROGMEM char* kBeepStr = D_STR_BEEP;
const PROGMEM char* kZoneFollowStr = D_STR_ZONEFOLLOW;
const PROGMEM char* kFixedStr = D_STR_FIXED;
const PROGMEM char* kMouldStr = D_STR_MOULD;
const PROGMEM char* kCleanStr = D_STR_CLEAN;
const PROGMEM char* kPurifyStr = D_STR_PURIFY;
const PROGMEM char* kTimerStr = D_STR_TIMER;
const PROGMEM char* kOnTimerStr = D_STR_ONTIMER;
const PROGMEM char* kOffTimerStr = D_STR_OFFTIMER;
const PROGMEM char* kClockStr = D_STR_CLOCK;
const PROGMEM char* kCommandStr = D_STR_COMMAND;
const PROGMEM char* kXFanStr = D_STR_XFAN;
const PROGMEM char* kHealthStr = D_STR_HEALTH;
const PROGMEM char* kModelStr = D_STR_MODEL;
const PROGMEM char* kTempStr = D_STR_TEMP;
const PROGMEM char* kIFeelStr = D_STR_IFEEL;
const PROGMEM char* kHumidStr = D_STR_HUMID;
const PROGMEM char* kSaveStr = D_STR_SAVE;
const PROGMEM char* kEyeStr = D_STR_EYE;
const PROGMEM char* kFollowStr = D_STR_FOLLOW;
const PROGMEM char* kIonStr = D_STR_ION;
const PROGMEM char* kFreshStr = D_STR_FRESH;
const PROGMEM char* kHoldStr = D_STR_HOLD;
const PROGMEM char* kButtonStr = D_STR_BUTTON;
const PROGMEM char* k8CHeatStr = D_STR_8C_HEAT;
const PROGMEM char* kNightStr = D_STR_NIGHT;
const PROGMEM char* kSilentStr = D_STR_SILENT;
const PROGMEM char* kFilterStr = D_STR_FILTER;
const PROGMEM char* k3DStr = D_STR_3D;
const PROGMEM char* kCelsiusStr = D_STR_CELSIUS;
const PROGMEM char* kTempUpStr = D_STR_TEMPUP;
const PROGMEM char* kTempDownStr = D_STR_TEMPDOWN;
const PROGMEM char* kStartStr = D_STR_START;
const PROGMEM char* kStopStr = D_STR_STOP;
const PROGMEM char* kMoveStr = D_STR_MOVE;
const PROGMEM char* kSetStr = D_STR_SET;
const PROGMEM char* kCancelStr = D_STR_CANCEL;
const PROGMEM char* kUpStr = D_STR_UP;
const PROGMEM char* kDownStr = D_STR_DOWN;
const PROGMEM char* kChangeStr = D_STR_CHANGE;
const PROGMEM char* kComfortStr = D_STR_COMFORT;
const PROGMEM char* kSensorStr = D_STR_SENSOR;
const PROGMEM char* kWeeklyTimerStr = D_STR_WEEKLYTIMER;
const PROGMEM char* kWifiStr = D_STR_WIFI;
const PROGMEM char* kLastStr = D_STR_LAST;
const PROGMEM char* kFastStr = D_STR_FAST;
const PROGMEM char* kSlowStr = D_STR_SLOW;
const PROGMEM char* kAirFlowStr = D_STR_AIRFLOW;
const PROGMEM char* kStepStr = D_STR_STEP;
const PROGMEM char* kNAStr = D_STR_NA;
const PROGMEM char* kOutsideStr = D_STR_OUTSIDE;
const PROGMEM char* kLoudStr = D_STR_LOUD;
const PROGMEM char* kLowerStr = D_STR_LOWER;
const PROGMEM char* kUpperStr = D_STR_UPPER;
const PROGMEM char* kBreezeStr = D_STR_BREEZE;
const PROGMEM char* kCirculateStr = D_STR_CIRCULATE;
const PROGMEM char* kCeilingStr = D_STR_CEILING;
const PROGMEM char* kWallStr = D_STR_WALL;
const PROGMEM char* kRoomStr = D_STR_ROOM;
const PROGMEM char* k6thSenseStr = D_STR_6THSENSE;

const PROGMEM char* kAutoStr = D_STR_AUTO;
const PROGMEM char* kAutomaticStr = D_STR_AUTOMATIC;
const PROGMEM char* kManualStr = D_STR_MANUAL;
const PROGMEM char* kCoolStr = D_STR_COOL;
const PROGMEM char* kHeatStr = D_STR_HEAT;
const PROGMEM char* kFanStr = D_STR_FAN;
const PROGMEM char* kDryStr = D_STR_DRY;
const PROGMEM char* kFanOnlyStr = D_STR_FANONLY;

const PROGMEM char* kMaxStr = D_STR_MAX;
const PROGMEM char* kMaximumStr = D_STR_MAXIMUM;
const PROGMEM char* kMinStr = D_STR_MIN;
const PROGMEM char* kMinimumStr = D_STR_MINIMUM;
const PROGMEM char* kMedStr = D_STR_MED;
const PROGMEM char* kMediumStr = D_STR_MEDIUM;

const PROGMEM char* kHighestStr = D_STR_HIGHEST;
const PROGMEM char* kHighStr = D_STR_HIGH;
const PROGMEM char* kHiStr = D_STR_HI;
const PROGMEM char* kMidStr = D_STR_MID;
const PROGMEM char* kMiddleStr = D_STR_MIDDLE;
const PROGMEM char* kLowStr = D_STR_LOW;
const PROGMEM char* kLoStr = D_STR_LO;
const PROGMEM char* kLowestStr = D_STR_LOWEST;
const PROGMEM char* kMaxRightStr = D_STR_MAXRIGHT;
const PROGMEM char* kRightMaxStr = D_STR_RIGHTMAX_NOSPACE;
const PROGMEM char* kRightStr = D_STR_RIGHT;
const PROGMEM char* kLeftStr = D_STR_LEFT;
const PROGMEM char* kMaxLeftStr = D_STR_MAXLEFT;
const PROGMEM char* kLeftMaxStr = D_STR_LEFTMAX_NOSPACE;
const PROGMEM char* kWideStr = D_STR_WIDE;
const PROGMEM char* kCentreStr = D_STR_CENTRE;
const PROGMEM char* kTopStr = D_STR_TOP;
const PROGMEM char* kBottomStr = D_STR_BOTTOM;

// Compound words/phrases/descriptions from pre-defined words.
const PROGMEM char* kEyeAutoStr = D_STR_EYEAUTO;
const PROGMEM char* kLightToggleStr = D_STR_LIGHTTOGGLE;
const PROGMEM char* kOutsideQuietStr = D_STR_OUTSIDEQUIET;
const PROGMEM char* kPowerToggleStr = D_STR_POWERTOGGLE;
const PROGMEM char* kPreviousPowerStr = D_STR_PREVIOUSPOWER;
const PROGMEM char* kSensorTempStr = D_STR_SENSORTEMP;
const PROGMEM char* kSleepTimerStr = D_STR_SLEEP_TIMER;
const PROGMEM char* kSwingVModeStr = D_STR_SWINGVMODE;
const PROGMEM char* kSwingVToggleStr = D_STR_SWINGVTOGGLE;

// Separators
char kTimeSep = D_CHR_TIME_SEP;
const PROGMEM char* kSpaceLBraceStr = D_STR_SPACELBRACE;
const PROGMEM char* kCommaSpaceStr = D_STR_COMMASPACE;
const PROGMEM char* kColonSpaceStr = D_STR_COLONSPACE;

// IRutils
//  - Time
const PROGMEM char* kDayStr = D_STR_DAY;
const PROGMEM char* kDaysStr = D_STR_DAYS;
const PROGMEM char* kHourStr = D_STR_HOUR;
const PROGMEM char* kHoursStr = D_STR_HOURS;
const PROGMEM char* kMinuteStr = D_STR_MINUTE;
const PROGMEM char* kMinutesStr = D_STR_MINUTES;
const PROGMEM char* kSecondStr = D_STR_SECOND;
const PROGMEM char* kSecondsStr = D_STR_SECONDS;
const PROGMEM char* kNowStr = D_STR_NOW;
const PROGMEM char* kThreeLetterDayOfWeekStr = D_STR_THREELETTERDAYS;

const PROGMEM char* kYesStr = D_STR_YES;
const PROGMEM char* kNoStr = D_STR_NO;
const PROGMEM char* kTrueStr = D_STR_TRUE;
const PROGMEM char* kFalseStr = D_STR_FALSE;

const PROGMEM char* kRepeatStr = D_STR_REPEAT;
const PROGMEM char* kCodeStr = D_STR_CODE;
const PROGMEM char* kBitsStr = D_STR_BITS;

// Protocol Names
const PROGMEM char* kAirwellStr = D_STR_AIRWELL;
const PROGMEM char* kAiwaRcT501Str = D_STR_AIWA_RC_T501;
const PROGMEM char* kAmcorStr = D_STR_AMCOR;
const PROGMEM char* kArgoStr = D_STR_ARGO;
const PROGMEM char* kCarrierAcStr = D_STR_CARRIER_AC;
const PROGMEM char* kCoolixStr = D_STR_COOLIX;
const PROGMEM char* kDaikinStr = D_STR_DAIKIN;
const PROGMEM char* kDaikin128Str = D_STR_DAIKIN128;
const PROGMEM char* kDaikin152Str = D_STR_DAIKIN152;
const PROGMEM char* kDaikin160Str = D_STR_DAIKIN160;
const PROGMEM char* kDaikin176Str = D_STR_DAIKIN176;
const PROGMEM char* kDaikin2Str = D_STR_DAIKIN2;
const PROGMEM char* kDaikin216Str = D_STR_DAIKIN216;
const PROGMEM char* kDaikin64Str = D_STR_DAIKIN64;
const PROGMEM char* kDenonStr = D_STR_DENON;
const PROGMEM char* kDishStr = D_STR_DISH;
const PROGMEM char* kElectraAcStr = D_STR_ELECTRA_AC;
const PROGMEM char* kEpsonStr = D_STR_EPSON;
const PROGMEM char* kFujitsuAcStr = D_STR_FUJITSU_AC;
const PROGMEM char* kGICableStr = D_STR_GICABLE;
const PROGMEM char* kGlobalCacheStr = D_STR_GLOBALCACHE;
const PROGMEM char* kGoodweatherStr = D_STR_GOODWEATHER;
const PROGMEM char* kGreeStr = D_STR_GREE;
const PROGMEM char* kHaierAcStr = D_STR_HAIER_AC;
const PROGMEM char* kHaierAcYrw02Str = D_STR_HAIER_AC_YRW02;
const PROGMEM char* kHitachiAcStr = D_STR_HITACHI_AC;
const PROGMEM char* kHitachiAc1Str = D_STR_HITACHI_AC1;
const PROGMEM char* kHitachiAc2Str = D_STR_HITACHI_AC2;
const PROGMEM char* kHitachiAc3Str = D_STR_HITACHI_AC3;
const PROGMEM char* kHitachiAc424Str = D_STR_HITACHI_AC424;
const PROGMEM char* kInaxStr = D_STR_INAX;
const PROGMEM char* kJvcStr = D_STR_JVC;
const PROGMEM char* kKelvinatorStr = D_STR_KELVINATOR;
const PROGMEM char* kLasertagStr = D_STR_LASERTAG;
const PROGMEM char* kLegopfStr = D_STR_LEGOPF;
const PROGMEM char* kLgStr = D_STR_LG;
const PROGMEM char* kLg2Str = D_STR_LG2;
const PROGMEM char* kLutronStr = D_STR_LUTRON;
const PROGMEM char* kMagiquestStr = D_STR_MAGIQUEST;
const PROGMEM char* kMideaStr = D_STR_MIDEA;
const PROGMEM char* kMitsubishiStr = D_STR_MITSUBISHI;
const PROGMEM char* kMitsubishi112Str = D_STR_MITSUBISHI112;
const PROGMEM char* kMitsubishi136Str = D_STR_MITSUBISHI136;
const PROGMEM char* kMitsubishi2Str = D_STR_MITSUBISHI2;
const PROGMEM char* kMitsubishiAcStr = D_STR_MITSUBISHI_AC;
const PROGMEM char* kMitsubishiHeavy152Str = D_STR_MITSUBISHI_HEAVY_152;
const PROGMEM char* kMitsubishiHeavy88Str = D_STR_MITSUBISHI_HEAVY_88;
const PROGMEM char* kMwmStr = D_STR_MWM;
const PROGMEM char* kNecStr = D_STR_NEC;
const PROGMEM char* kNecLikeStr = D_STR_NEC_LIKE;
const PROGMEM char* kNecNonStrictStr = D_STR_NEC " (NON-STRICT)";
const PROGMEM char* kNeoclimaStr = D_STR_NEOCLIMA;
const PROGMEM char* kNikaiStr = D_STR_NIKAI;
const PROGMEM char* kPanasonicStr = D_STR_PANASONIC;
const PROGMEM char* kPanasonicAcStr = D_STR_PANASONIC_AC;
const PROGMEM char* kPioneerStr = D_STR_PIONEER;
const PROGMEM char* kProntoStr = D_STR_PRONTO;
const PROGMEM char* kRawStr = D_STR_RAW;
const PROGMEM char* kRc5Str = D_STR_RC5;
const PROGMEM char* kRc5XStr = D_STR_RC5X;
const PROGMEM char* kRc6Str = D_STR_RC6;
const PROGMEM char* kRcmmStr = D_STR_RCMM;
const PROGMEM char* kSamsungStr = D_STR_SAMSUNG;
const PROGMEM char* kSamsung36Str = D_STR_SAMSUNG36;
const PROGMEM char* kSamsungAcStr = D_STR_SAMSUNG_AC;
const PROGMEM char* kSanyoStr = D_STR_SANYO;
const PROGMEM char* kSanyoLc7461Str = D_STR_SANYO_LC7461;
const PROGMEM char* kSharpStr = D_STR_SHARP;
const PROGMEM char* kSharpAcStr = D_STR_SHARP_AC;
const PROGMEM char* kSherwoodStr = D_STR_SHERWOOD;
const PROGMEM char* kSonyStr = D_STR_SONY;
const PROGMEM char* kSony38KStr = D_STR_SONY_38K;
const PROGMEM char* kSymphonyStr = D_STR_SYMPHONY;
const PROGMEM char* kTcl112AcStr = D_STR_TCL112AC;
const PROGMEM char* kTecoStr = D_STR_TECO;
const PROGMEM char* kToshibaAcStr = D_STR_TOSHIBA_AC;
const PROGMEM char* kTrotecStr = D_STR_TROTEC;
const PROGMEM char* kUnusedStr = D_STR_UNUSED;
const PROGMEM char* kVestelAcStr = D_STR_VESTEL_AC;
const PROGMEM char* kWhirlpoolAcStr = D_STR_WHIRLPOOL_AC;
const PROGMEM char* kWhynterStr = D_STR_WHYNTER;

// Needs to be in decode_type_t order.
const PROGMEM char *kProtocolNamesStrArray[kLastDecodeType + 1] = {
  kUnusedStr,
  kRc5Str,
  kRc6Str,
  kNecStr,
  kSonyStr,
  kPanasonicStr,
  kJvcStr,
  kSamsungStr,
  kWhynterStr,
  kAiwaRcT501Str,
  kLgStr,
  kSanyoStr,
  kMitsubishiStr,
  kDishStr,
  kSharpStr,
  kCoolixStr,
  kDaikinStr,
  kDenonStr,
  kKelvinatorStr,
  kSherwoodStr,
  kMitsubishiAcStr,
  kRcmmStr,
  kSanyoLc7461Str,
  kRc5XStr,
  kGreeStr,
  kProntoStr,
  kNecLikeStr,
  kArgoStr,
  kTrotecStr,
  kNikaiStr,
  kRawStr,
  kGlobalCacheStr,
  kToshibaAcStr,
  kFujitsuAcStr,
  kMideaStr,
  kMagiquestStr,
  kLasertagStr,
  kCarrierAcStr,
  kHaierAcStr,
  kMitsubishi2Str,
  kHitachiAcStr,
  kHitachiAc1Str,
  kHitachiAc2Str,
  kGICableStr,
  kHaierAcYrw02Str,
  kWhirlpoolAcStr,
  kSamsungAcStr,
  kLutronStr,
  kElectraAcStr,
  kPanasonicAcStr,
  kPioneerStr,
  kLg2Str,
  kMwmStr,
  kDaikin2Str,
  kVestelAcStr,
  kTecoStr,
  kSamsung36Str,
  kTcl112AcStr,
  kLegopfStr,
  kMitsubishiHeavy88Str,
  kMitsubishiHeavy152Str,
  kDaikin216Str,
  kSharpAcStr,
  kGoodweatherStr,
  kInaxStr,
  kDaikin160Str,
  kNeoclimaStr,
  kDaikin176Str,
  kDaikin128Str,
  kAmcorStr,
  kDaikin152Str,
  kMitsubishi136Str,
  kMitsubishi112Str,
  kHitachiAc424Str,
  kSony38KStr,
  kEpsonStr,
  kSymphonyStr,
  kHitachiAc3Str,
  kDaikin64Str,
  kAirwellStr,
};
