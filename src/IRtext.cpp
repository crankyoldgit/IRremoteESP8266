// Copyright 2019-2020 - David Conran (@crankyoldgit)

/// @file IRtext.cpp
/// @warning If you add or remove an entry in this file, you should run:
///   '../tools/generate_irtext_h.sh' to rebuild the `IRtext.h` file.

#ifndef UNIT_TEST
#include <Arduino.h>
#endif  // UNIT_TEST
#include "IRremoteESP8266.h"
#include "i18n.h"

#ifndef PROGMEM
#define PROGMEM  // Pretend we have the PROGMEM macro even if we really don't.
#endif

// Common
const PROGMEM char* kUnknownStr = D_STR_UNKNOWN;  ///< "Unknown"
const PROGMEM char* kProtocolStr = D_STR_PROTOCOL;  ///< "Protocol"
const PROGMEM char* kPowerStr = D_STR_POWER;  ///< "Power"
const PROGMEM char* kOnStr = D_STR_ON;  ///< "On"
const PROGMEM char* kOffStr = D_STR_OFF;  ///< "Off"
const PROGMEM char* kModeStr = D_STR_MODE;  ///< "Mode"
const PROGMEM char* kToggleStr = D_STR_TOGGLE;  ///< "Toggle"
const PROGMEM char* kTurboStr = D_STR_TURBO;  ///< "Turbo"
const PROGMEM char* kSuperStr = D_STR_SUPER;  ///< "Super"
const PROGMEM char* kSleepStr = D_STR_SLEEP;  ///< "Sleep"
const PROGMEM char* kLightStr = D_STR_LIGHT;  ///< "Light"
const PROGMEM char* kPowerfulStr = D_STR_POWERFUL;  ///< "Powerful"
const PROGMEM char* kQuietStr = D_STR_QUIET;  ///< "Quiet"
const PROGMEM char* kEconoStr = D_STR_ECONO;  ///< "Econo"
const PROGMEM char* kSwingStr = D_STR_SWING;  ///< "Swing"
const PROGMEM char* kSwingHStr = D_STR_SWINGH;  ///< "SwingH"
const PROGMEM char* kSwingVStr = D_STR_SWINGV;  ///< "SwingV"
const PROGMEM char* kBeepStr = D_STR_BEEP;  ///< "Beep"
const PROGMEM char* kZoneFollowStr = D_STR_ZONEFOLLOW;  ///< "Zone Follow"
const PROGMEM char* kFixedStr = D_STR_FIXED;  ///< "Fixed"
const PROGMEM char* kMouldStr = D_STR_MOULD;  ///< "Mould"
const PROGMEM char* kCleanStr = D_STR_CLEAN;  ///< "Clean"
const PROGMEM char* kPurifyStr = D_STR_PURIFY;  ///< "Purify"
const PROGMEM char* kTimerStr = D_STR_TIMER;  ///< "Timer"
const PROGMEM char* kOnTimerStr = D_STR_ONTIMER;  ///< "OnTimer"
const PROGMEM char* kOffTimerStr = D_STR_OFFTIMER;  ///< "OffTimer"
const PROGMEM char* kClockStr = D_STR_CLOCK;  ///< "Clock"
const PROGMEM char* kCommandStr = D_STR_COMMAND;  ///< "Command"
const PROGMEM char* kXFanStr = D_STR_XFAN;  ///< "XFan"
const PROGMEM char* kHealthStr = D_STR_HEALTH;  ///< "Health"
const PROGMEM char* kModelStr = D_STR_MODEL;  ///< "Model"
const PROGMEM char* kTempStr = D_STR_TEMP;  ///< "Temp"
const PROGMEM char* kIFeelStr = D_STR_IFEEL;  ///< "IFeel"
const PROGMEM char* kHumidStr = D_STR_HUMID;  ///< "Humid"
const PROGMEM char* kSaveStr = D_STR_SAVE;  ///< "Save"
const PROGMEM char* kEyeStr = D_STR_EYE;  ///< "Eye"
const PROGMEM char* kFollowStr = D_STR_FOLLOW;  ///< "Follow"
const PROGMEM char* kIonStr = D_STR_ION;  ///< "Ion"
const PROGMEM char* kFreshStr = D_STR_FRESH;  ///< "Fresh"
const PROGMEM char* kHoldStr = D_STR_HOLD;  ///< "Hold"
const PROGMEM char* kButtonStr = D_STR_BUTTON;  ///< "Button"
const PROGMEM char* k8CHeatStr = D_STR_8C_HEAT;  ///< "8CHeat"
const PROGMEM char* kNightStr = D_STR_NIGHT;  ///< "Night"
const PROGMEM char* kSilentStr = D_STR_SILENT;  ///< "Silent"
const PROGMEM char* kFilterStr = D_STR_FILTER;  ///< "Filter"
const PROGMEM char* k3DStr = D_STR_3D;  ///< "3D"
const PROGMEM char* kCelsiusStr = D_STR_CELSIUS;  ///< "Celsius"
const PROGMEM char* kTempUpStr = D_STR_TEMPUP;  ///< "Temp Up"
const PROGMEM char* kTempDownStr = D_STR_TEMPDOWN;  ///< "Temp Down"
const PROGMEM char* kStartStr = D_STR_START;  ///< "Start"
const PROGMEM char* kStopStr = D_STR_STOP;  ///< "Stop"
const PROGMEM char* kMoveStr = D_STR_MOVE;  ///< "Move"
const PROGMEM char* kSetStr = D_STR_SET;  ///< "Set"
const PROGMEM char* kCancelStr = D_STR_CANCEL;  ///< "Cancel"
const PROGMEM char* kUpStr = D_STR_UP;  ///< "Up"
const PROGMEM char* kDownStr = D_STR_DOWN;  ///< "Down"
const PROGMEM char* kChangeStr = D_STR_CHANGE;  ///< "Change"
const PROGMEM char* kComfortStr = D_STR_COMFORT;  ///< "Comfort"
const PROGMEM char* kSensorStr = D_STR_SENSOR;  ///< "Sensor"
const PROGMEM char* kWeeklyTimerStr = D_STR_WEEKLYTIMER;  ///< "WeeklyTimer"
const PROGMEM char* kWifiStr = D_STR_WIFI;  ///< "Wifi"
const PROGMEM char* kLastStr = D_STR_LAST;  ///< "Last"
const PROGMEM char* kFastStr = D_STR_FAST;  ///< "Fast"
const PROGMEM char* kSlowStr = D_STR_SLOW;  ///< "Slow"
const PROGMEM char* kAirFlowStr = D_STR_AIRFLOW;  ///< "Air Flow"
const PROGMEM char* kStepStr = D_STR_STEP;  ///< "Step"
const PROGMEM char* kNAStr = D_STR_NA;  ///< "N/A"
const PROGMEM char* kInsideStr = D_STR_INSIDE;  ///< "Inside"
const PROGMEM char* kOutsideStr = D_STR_OUTSIDE;  ///< "Outside"
const PROGMEM char* kLoudStr = D_STR_LOUD;  ///< "Loud"
const PROGMEM char* kLowerStr = D_STR_LOWER;  ///< "Lower"
const PROGMEM char* kUpperStr = D_STR_UPPER;  ///< "Upper"
const PROGMEM char* kBreezeStr = D_STR_BREEZE;  ///< "Breeze"
const PROGMEM char* kCirculateStr = D_STR_CIRCULATE;  ///< "Circulate"
const PROGMEM char* kCeilingStr = D_STR_CEILING;  ///< "Ceiling"
const PROGMEM char* kWallStr = D_STR_WALL;  ///< "Wall"
const PROGMEM char* kRoomStr = D_STR_ROOM;  ///< "Room"
const PROGMEM char* k6thSenseStr = D_STR_6THSENSE;  ///< "6th Sense"

const PROGMEM char* kAutoStr = D_STR_AUTO;  ///< "Auto"
const PROGMEM char* kAutomaticStr = D_STR_AUTOMATIC;  ///< "Automatic"
const PROGMEM char* kManualStr = D_STR_MANUAL;  ///< "Manual"
const PROGMEM char* kCoolStr = D_STR_COOL;  ///< "Cool"
const PROGMEM char* kHeatStr = D_STR_HEAT;  ///< "Heat"
const PROGMEM char* kFanStr = D_STR_FAN;  ///< "Fan"
const PROGMEM char* kDryStr = D_STR_DRY;  ///< "Dry"
const PROGMEM char* kFanOnlyStr = D_STR_FANONLY;  ///< "fan_only"

const PROGMEM char* kMaxStr = D_STR_MAX;  ///< "Max"
const PROGMEM char* kMaximumStr = D_STR_MAXIMUM;  ///< "Maximum"
const PROGMEM char* kMinStr = D_STR_MIN;  ///< "Min"
const PROGMEM char* kMinimumStr = D_STR_MINIMUM;  ///< "Minimum"
const PROGMEM char* kMedStr = D_STR_MED;  ///< "Med"
const PROGMEM char* kMediumStr = D_STR_MEDIUM;  ///< "Medium"

const PROGMEM char* kHighestStr = D_STR_HIGHEST;  ///< "Highest"
const PROGMEM char* kHighStr = D_STR_HIGH;  ///< "High"
const PROGMEM char* kHiStr = D_STR_HI;  ///< "Hi"
const PROGMEM char* kMidStr = D_STR_MID;  ///< "Mid"
const PROGMEM char* kMiddleStr = D_STR_MIDDLE;  ///< "Middle"
const PROGMEM char* kLowStr = D_STR_LOW;  ///< "Low"
const PROGMEM char* kLoStr = D_STR_LO;  ///< "Lo"
const PROGMEM char* kLowestStr = D_STR_LOWEST;  ///< "Lowest"
const PROGMEM char* kMaxRightStr = D_STR_MAXRIGHT;  ///< "Max Right"
const PROGMEM char* kRightMaxStr = D_STR_RIGHTMAX_NOSPACE;  ///< "RightMax"
const PROGMEM char* kRightStr = D_STR_RIGHT;  ///< "Right"
const PROGMEM char* kLeftStr = D_STR_LEFT;  ///< "Left"
const PROGMEM char* kMaxLeftStr = D_STR_MAXLEFT;  ///< "Max Left"
const PROGMEM char* kLeftMaxStr = D_STR_LEFTMAX_NOSPACE;  ///< "LeftMax"
const PROGMEM char* kWideStr = D_STR_WIDE;  ///< "Wide"
const PROGMEM char* kCentreStr = D_STR_CENTRE;  ///< "Centre"
const PROGMEM char* kTopStr = D_STR_TOP;  ///< "Top"
const PROGMEM char* kBottomStr = D_STR_BOTTOM;  ///< "Bottom"

// Compound words/phrases/descriptions from pre-defined words.
const PROGMEM char* kEconoToggleStr = D_STR_ECONOTOGGLE;  ///< "Econo Toggle"
const PROGMEM char* kEyeAutoStr = D_STR_EYEAUTO;  ///< "Eye Auto"
const PROGMEM char* kLightToggleStr = D_STR_LIGHTTOGGLE;  ///< "Light Toggle"
const PROGMEM char* kOutsideQuietStr = D_STR_OUTSIDEQUIET;  ///< "Outside Quiet"
const PROGMEM char* kPowerToggleStr = D_STR_POWERTOGGLE;  ///< "Power Toggle"
const PROGMEM char* kPowerButtonStr = D_STR_POWERBUTTON;  ///< "Power Button"
const PROGMEM char* kPreviousPowerStr = D_STR_PREVIOUSPOWER;  ///<
///< "Previous Power"
const PROGMEM char* kDisplayTempStr = D_STR_DISPLAYTEMP;  ///< "Display Temp"
const PROGMEM char* kSensorTempStr = D_STR_SENSORTEMP;  ///< "Sensor Temp"
const PROGMEM char* kSleepTimerStr = D_STR_SLEEP_TIMER;  ///< "Sleep Timer"
const PROGMEM char* kSwingVModeStr = D_STR_SWINGVMODE;  ///< "Swing(V) Mode"
const PROGMEM char* kSwingVToggleStr = D_STR_SWINGVTOGGLE;  ///<
///< "Swing(V) Toggle"

// Separators
char kTimeSep = D_CHR_TIME_SEP;  ///< ':'
const PROGMEM char* kSpaceLBraceStr = D_STR_SPACELBRACE;  ///< " ("
const PROGMEM char* kCommaSpaceStr = D_STR_COMMASPACE;  ///< ", "
const PROGMEM char* kColonSpaceStr = D_STR_COLONSPACE;  ///< ": "

// IRutils
//  - Time
const PROGMEM char* kDayStr = D_STR_DAY;  ///< "Day"
const PROGMEM char* kDaysStr = D_STR_DAYS;  ///< "Days"
const PROGMEM char* kHourStr = D_STR_HOUR;  ///< "Hour"
const PROGMEM char* kHoursStr = D_STR_HOURS;  ///< "Hours"
const PROGMEM char* kMinuteStr = D_STR_MINUTE;  ///< "Minute"
const PROGMEM char* kMinutesStr = D_STR_MINUTES;  ///< "Minutes"
const PROGMEM char* kSecondStr = D_STR_SECOND;  ///< "Second"
const PROGMEM char* kSecondsStr = D_STR_SECONDS;  ///< "Seconds"
const PROGMEM char* kNowStr = D_STR_NOW;  ///< "Now"
const PROGMEM char* kThreeLetterDayOfWeekStr = D_STR_THREELETTERDAYS;  ///<
///< "SunMonTueWedThuFriSat"
const PROGMEM char* kYesStr = D_STR_YES;  ///< "Yes"
const PROGMEM char* kNoStr = D_STR_NO;  ///< "No"
const PROGMEM char* kTrueStr = D_STR_TRUE;  ///< "True"
const PROGMEM char* kFalseStr = D_STR_FALSE;  ///< "False"

const PROGMEM char* kRepeatStr = D_STR_REPEAT;  ///< "Repeat"
const PROGMEM char* kCodeStr = D_STR_CODE;  ///< "Code"
const PROGMEM char* kBitsStr = D_STR_BITS;  ///< "Bits"

// Protocol Names
// Needs to be in decode_type_t order.
const PROGMEM char *kAllProtocolNamesStr =
    D_STR_UNUSED "\x0"
    D_STR_RC5 "\x0"
    D_STR_RC6 "\x0"
    D_STR_NEC "\x0"
    D_STR_SONY "\x0"
    D_STR_PANASONIC "\x0"
    D_STR_JVC "\x0"
    D_STR_SAMSUNG "\x0"
    D_STR_WHYNTER "\x0"
    D_STR_AIWA_RC_T501 "\x0"
    D_STR_LG "\x0"
    D_STR_SANYO "\x0"
    D_STR_MITSUBISHI "\x0"
    D_STR_DISH "\x0"
    D_STR_SHARP "\x0"
    D_STR_COOLIX "\x0"
    D_STR_DAIKIN "\x0"
    D_STR_DENON "\x0"
    D_STR_KELVINATOR "\x0"
    D_STR_SHERWOOD "\x0"
    D_STR_MITSUBISHI_AC "\x0"
    D_STR_RCMM "\x0"
    D_STR_SANYO_LC7461 "\x0"
    D_STR_RC5X "\x0"
    D_STR_GREE "\x0"
    D_STR_PRONTO "\x0"
    D_STR_NEC_LIKE "\x0"
    D_STR_ARGO "\x0"
    D_STR_TROTEC "\x0"
    D_STR_NIKAI "\x0"
    D_STR_RAW "\x0"
    D_STR_GLOBALCACHE "\x0"
    D_STR_TOSHIBA_AC "\x0"
    D_STR_FUJITSU_AC "\x0"
    D_STR_MIDEA "\x0"
    D_STR_MAGIQUEST "\x0"
    D_STR_LASERTAG "\x0"
    D_STR_CARRIER_AC "\x0"
    D_STR_HAIER_AC "\x0"
    D_STR_MITSUBISHI2 "\x0"
    D_STR_HITACHI_AC "\x0"
    D_STR_HITACHI_AC1 "\x0"
    D_STR_HITACHI_AC2 "\x0"
    D_STR_GICABLE "\x0"
    D_STR_HAIER_AC_YRW02 "\x0"
    D_STR_WHIRLPOOL_AC "\x0"
    D_STR_SAMSUNG_AC "\x0"
    D_STR_LUTRON "\x0"
    D_STR_ELECTRA_AC "\x0"
    D_STR_PANASONIC_AC "\x0"
    D_STR_PIONEER "\x0"
    D_STR_LG2 "\x0"
    D_STR_MWM "\x0"
    D_STR_DAIKIN2 "\x0"
    D_STR_VESTEL_AC "\x0"
    D_STR_TECO "\x0"
    D_STR_SAMSUNG36 "\x0"
    D_STR_TCL112AC "\x0"
    D_STR_LEGOPF "\x0"
    D_STR_MITSUBISHI_HEAVY_88 "\x0"
    D_STR_MITSUBISHI_HEAVY_152 "\x0"
    D_STR_DAIKIN216 "\x0"
    D_STR_SHARP_AC "\x0"
    D_STR_GOODWEATHER "\x0"
    D_STR_INAX "\x0"
    D_STR_DAIKIN160 "\x0"
    D_STR_NEOCLIMA "\x0"
    D_STR_DAIKIN176 "\x0"
    D_STR_DAIKIN128 "\x0"
    D_STR_AMCOR "\x0"
    D_STR_DAIKIN152 "\x0"
    D_STR_MITSUBISHI136 "\x0"
    D_STR_MITSUBISHI112 "\x0"
    D_STR_HITACHI_AC424 "\x0"
    D_STR_SONY_38K "\x0"
    D_STR_EPSON "\x0"
    D_STR_SYMPHONY "\x0"
    D_STR_HITACHI_AC3 "\x0"
    D_STR_DAIKIN64 "\x0"
    D_STR_AIRWELL "\x0"
    D_STR_DELONGHI_AC "\x0"
    D_STR_DOSHISHA "\x0"
    D_STR_MULTIBRACKETS "\x0"
    D_STR_CARRIER_AC40 "\x0"
    D_STR_CARRIER_AC64 "\x0"
    D_STR_HITACHI_AC344 "\x0"
    D_STR_CORONA_AC "\x0"
    D_STR_MIDEA24 "\x0"
    D_STR_ZEPEAL "\x0"
    D_STR_SANYO_AC "\x0"
    D_STR_VOLTAS "\x0"
    ///< New protocol strings should be added just above this line.
    "\x0";  ///< This string requires double null termination.
