// Copyright 2019-2021 - David Conran (@crankyoldgit)
// This header file is to be included in files **other than** 'IRtext.cpp'.
//
// WARNING: Do not edit this file! This file is automatically generated by
//          '../tools/generate_irtext_h.sh'.

#ifndef IRTEXT_H_
#define IRTEXT_H_

#include "i18n.h"

// Constant text to be shared across all object files.
// This means there is only one copy of the character/string/text etc.

#ifdef ESP8266
class __FlashStringHelper;
#define IRTEXT_CONST_PTR_CAST(PTR)\
    reinterpret_cast<const __FlashStringHelper*>(PTR)
#define IRTEXT_CONST_PTR(NAME) const __FlashStringHelper* const NAME
#else  // ESP8266
#define IRTEXT_CONST_PTR_CAST(PTR) PTR
#define IRTEXT_CONST_PTR(NAME) const char* const NAME
#endif  // ESP8266

extern const char kTimeSep;
extern IRTEXT_CONST_PTR(k10CHeatStr);
extern IRTEXT_CONST_PTR(k3DStr);
extern IRTEXT_CONST_PTR(k6thSenseStr);
extern IRTEXT_CONST_PTR(k8CHeatStr);
extern IRTEXT_CONST_PTR(kAirFlowStr);
extern IRTEXT_CONST_PTR(kAutomaticStr);
extern IRTEXT_CONST_PTR(kAutoStr);
extern IRTEXT_CONST_PTR(kBeepStr);
extern IRTEXT_CONST_PTR(kBitsStr);
extern IRTEXT_CONST_PTR(kBottomStr);
extern IRTEXT_CONST_PTR(kBreezeStr);
extern IRTEXT_CONST_PTR(kButtonStr);
extern IRTEXT_CONST_PTR(kCancelStr);
extern IRTEXT_CONST_PTR(kCeilingStr);
extern IRTEXT_CONST_PTR(kCelsiusFahrenheitStr);
extern IRTEXT_CONST_PTR(kCelsiusStr);
extern IRTEXT_CONST_PTR(kCentreStr);
extern IRTEXT_CONST_PTR(kChangeStr);
extern IRTEXT_CONST_PTR(kCirculateStr);
extern IRTEXT_CONST_PTR(kCleanStr);
extern IRTEXT_CONST_PTR(kClockStr);
extern IRTEXT_CONST_PTR(kCodeStr);
extern IRTEXT_CONST_PTR(kColonSpaceStr);
extern IRTEXT_CONST_PTR(kComfortStr);
extern IRTEXT_CONST_PTR(kCommandStr);
extern IRTEXT_CONST_PTR(kCommaSpaceStr);
extern IRTEXT_CONST_PTR(kCoolStr);
extern IRTEXT_CONST_PTR(kDaysStr);
extern IRTEXT_CONST_PTR(kDayStr);
extern IRTEXT_CONST_PTR(kDisplayTempStr);
extern IRTEXT_CONST_PTR(kDownStr);
extern IRTEXT_CONST_PTR(kDryStr);
extern IRTEXT_CONST_PTR(kEconoStr);
extern IRTEXT_CONST_PTR(kEconoToggleStr);
extern IRTEXT_CONST_PTR(kEyeAutoStr);
extern IRTEXT_CONST_PTR(kEyeStr);
extern IRTEXT_CONST_PTR(kFalseStr);
extern IRTEXT_CONST_PTR(kFanOnlyNoSpaceStr);
extern IRTEXT_CONST_PTR(kFan_OnlyStr);
extern IRTEXT_CONST_PTR(kFanOnlyStr);
extern IRTEXT_CONST_PTR(kFanOnlyWithSpaceStr);
extern IRTEXT_CONST_PTR(kFanStr);
extern IRTEXT_CONST_PTR(kFastStr);
extern IRTEXT_CONST_PTR(kFilterStr);
extern IRTEXT_CONST_PTR(kFixedStr);
extern IRTEXT_CONST_PTR(kFollowStr);
extern IRTEXT_CONST_PTR(kFreshStr);
extern IRTEXT_CONST_PTR(kHealthStr);
extern IRTEXT_CONST_PTR(kHeatStr);
extern IRTEXT_CONST_PTR(kHighestStr);
extern IRTEXT_CONST_PTR(kHighStr);
extern IRTEXT_CONST_PTR(kHiStr);
extern IRTEXT_CONST_PTR(kHoldStr);
extern IRTEXT_CONST_PTR(kHoursStr);
extern IRTEXT_CONST_PTR(kHourStr);
extern IRTEXT_CONST_PTR(kHumidStr);
extern IRTEXT_CONST_PTR(kIdStr);
extern IRTEXT_CONST_PTR(kIFeelStr);
extern IRTEXT_CONST_PTR(kInsideStr);
extern IRTEXT_CONST_PTR(kIonStr);
extern IRTEXT_CONST_PTR(kLastStr);
extern IRTEXT_CONST_PTR(kLeftMaxStr);
extern IRTEXT_CONST_PTR(kLeftStr);
extern IRTEXT_CONST_PTR(kLightStr);
extern IRTEXT_CONST_PTR(kLightToggleStr);
extern IRTEXT_CONST_PTR(kLoStr);
extern IRTEXT_CONST_PTR(kLoudStr);
extern IRTEXT_CONST_PTR(kLowerStr);
extern IRTEXT_CONST_PTR(kLowestStr);
extern IRTEXT_CONST_PTR(kLowStr);
extern IRTEXT_CONST_PTR(kManualStr);
extern IRTEXT_CONST_PTR(kMaximumStr);
extern IRTEXT_CONST_PTR(kMaxLeftStr);
extern IRTEXT_CONST_PTR(kMaxRightStr);
extern IRTEXT_CONST_PTR(kMaxStr);
extern IRTEXT_CONST_PTR(kMediumStr);
extern IRTEXT_CONST_PTR(kMedStr);
extern IRTEXT_CONST_PTR(kMiddleStr);
extern IRTEXT_CONST_PTR(kMidStr);
extern IRTEXT_CONST_PTR(kMinimumStr);
extern IRTEXT_CONST_PTR(kMinStr);
extern IRTEXT_CONST_PTR(kMinutesStr);
extern IRTEXT_CONST_PTR(kMinuteStr);
extern IRTEXT_CONST_PTR(kModelStr);
extern IRTEXT_CONST_PTR(kModeStr);
extern IRTEXT_CONST_PTR(kMouldStr);
extern IRTEXT_CONST_PTR(kMoveStr);
extern IRTEXT_CONST_PTR(kNAStr);
extern IRTEXT_CONST_PTR(kNightStr);
extern IRTEXT_CONST_PTR(kNoStr);
extern IRTEXT_CONST_PTR(kNowStr);
extern IRTEXT_CONST_PTR(kOffStr);
extern IRTEXT_CONST_PTR(kOffTimerStr);
extern IRTEXT_CONST_PTR(kOnStr);
extern IRTEXT_CONST_PTR(kOnTimerStr);
extern IRTEXT_CONST_PTR(kOutsideQuietStr);
extern IRTEXT_CONST_PTR(kOutsideStr);
extern IRTEXT_CONST_PTR(kPowerButtonStr);
extern IRTEXT_CONST_PTR(kPowerfulStr);
extern IRTEXT_CONST_PTR(kPowerStr);
extern IRTEXT_CONST_PTR(kPowerToggleStr);
extern IRTEXT_CONST_PTR(kPreviousPowerStr);
extern IRTEXT_CONST_PTR(kProtocolStr);
extern IRTEXT_CONST_PTR(kPurifyStr);
extern IRTEXT_CONST_PTR(kQuietStr);
extern IRTEXT_CONST_PTR(kRecycleStr);
extern IRTEXT_CONST_PTR(kRepeatStr);
extern IRTEXT_CONST_PTR(kRightMaxStr);
extern IRTEXT_CONST_PTR(kRightStr);
extern IRTEXT_CONST_PTR(kRoomStr);
extern IRTEXT_CONST_PTR(kSaveStr);
extern IRTEXT_CONST_PTR(kSecondsStr);
extern IRTEXT_CONST_PTR(kSecondStr);
extern IRTEXT_CONST_PTR(kSensorStr);
extern IRTEXT_CONST_PTR(kSensorTempStr);
extern IRTEXT_CONST_PTR(kSetStr);
extern IRTEXT_CONST_PTR(kSilentStr);
extern IRTEXT_CONST_PTR(kSleepStr);
extern IRTEXT_CONST_PTR(kSleepTimerStr);
extern IRTEXT_CONST_PTR(kSlowStr);
extern IRTEXT_CONST_PTR(kSpaceLBraceStr);
extern IRTEXT_CONST_PTR(kSpecialStr);
extern IRTEXT_CONST_PTR(kStartStr);
extern IRTEXT_CONST_PTR(kStepStr);
extern IRTEXT_CONST_PTR(kStopStr);
extern IRTEXT_CONST_PTR(kSuperStr);
extern IRTEXT_CONST_PTR(kSwingHStr);
extern IRTEXT_CONST_PTR(kSwingStr);
extern IRTEXT_CONST_PTR(kSwingVModeStr);
extern IRTEXT_CONST_PTR(kSwingVStr);
extern IRTEXT_CONST_PTR(kSwingVToggleStr);
extern IRTEXT_CONST_PTR(kTempDownStr);
extern IRTEXT_CONST_PTR(kTempStr);
extern IRTEXT_CONST_PTR(kTempUpStr);
extern IRTEXT_CONST_PTR(kThreeLetterDayOfWeekStr);
extern IRTEXT_CONST_PTR(kTimerModeStr);
extern IRTEXT_CONST_PTR(kTimerStr);
extern IRTEXT_CONST_PTR(kToggleStr);
extern IRTEXT_CONST_PTR(kTopStr);
extern IRTEXT_CONST_PTR(kTrueStr);
extern IRTEXT_CONST_PTR(kTurboStr);
extern IRTEXT_CONST_PTR(kTurboToggleStr);
extern IRTEXT_CONST_PTR(kTypeStr);
extern IRTEXT_CONST_PTR(kUnknownStr);
extern IRTEXT_CONST_PTR(kUpperStr);
extern IRTEXT_CONST_PTR(kUpStr);
extern IRTEXT_CONST_PTR(kVaneStr);
extern IRTEXT_CONST_PTR(kWallStr);
extern IRTEXT_CONST_PTR(kWeeklyTimerStr);
extern IRTEXT_CONST_PTR(kWideStr);
extern IRTEXT_CONST_PTR(kWifiStr);
extern IRTEXT_CONST_PTR(kXFanStr);
extern IRTEXT_CONST_PTR(kYesStr);
extern IRTEXT_CONST_PTR(kZoneFollowStr);
extern IRTEXT_CONST_PTR(kAllProtocolNamesStr);

#endif  // IRTEXT_H_
