// Copyright 2019 - Martin (@finfinack)
// Locale/language file for German / Switzerland.
// This file will override the default values located in `defaults.h`.
#ifndef LOCALE_DE_CH_H_
#define LOCALE_DE_CH_H_

// Import German / Germany as default overwrites.
#include "locale/de_DE.h"

#define D_STR_ON "Ii"
#define D_STR_OFF "Us"
#define D_STR_TOGGLE "Umschalte"
#define D_STR_SLEEP "Schlafe"
#define D_STR_LIGHT "Liecht"
#define D_STR_POWERFUL "Starch"
#define D_STR_QUIET "Liislig"
#define D_STR_CLEAN "Reinige"
#define D_STR_PURIFY "Frische"
#define D_STR_HEALTH "Gsundheit"
#define D_STR_HUMID "Füecht"
#define D_STR_SAVE "Speichere"
#define D_STR_EYE "Aug"
#define D_STR_FOLLOW "Folge"
#define D_STR_HOLD "Halte"
#define D_STR_BUTTON "Chnopf"
#define D_STR_UP "Ufe"
#define D_STR_TEMPUP D_STR_TEMP " " D_STR_UP
#define D_STR_DOWN "Abe"
#define D_STR_TEMPDOWN D_STR_TEMP " " D_STR_DOWN
#define D_STR_CHANGE "Wechsele"
#define D_STR_MOVE "Verschiebe"
#define D_STR_SET "Setze"
#define D_STR_CANCEL "Abbreche"
#define D_STR_WEEKLY "Wüchentlich"
#define D_STR_WEEKLYTIMER D_STR_WEEKLY " " D_STR_TIMER
#define D_STR_OUTSIDE "Dusse"
#define D_STR_LOUD "Luut"
#define D_STR_UPPER "Obe"
#define D_STR_LOWER "Une"
#define D_STR_CIRCULATE "Zirkuliere"
#define D_STR_CEILING "Decki"
#define D_STR_6THSENSE "6te Sinn"

#define D_STR_COOL "Chüehle"
#define D_STR_HEAT "Heize"
#define D_STR_DRY "Tröchne"

#define D_STR_MED "Mit"
#define D_STR_MEDIUM "Mittel"

#define D_STR_HIGHEST "Höchscht"
#define D_STR_HIGH "Höch"
#define D_STR_HI "H"
#define D_STR_MID "M"
#define D_STR_MIDDLE "Mittel"
#define D_STR_LOW "Tüüf"
#define D_STR_LO "T"
#define D_STR_LOWEST "Tüfschte"
#define D_STR_MAXRIGHT D_STR_MAX " " D_STR_RIGHT
#define D_STR_RIGHTMAX_NOSPACE D_STR_RIGHT D_STR_MAX
#define D_STR_MAXLEFT D_STR_MAX " " D_STR_LEFT
#define D_STR_LEFTMAX_NOSPACE D_STR_LEFT D_STR_MAX
#define D_STR_CENTRE "Mitti"
#define D_STR_TOP "Obe"
#define D_STR_BOTTOM "Une"

#define D_STR_DAY "Tag"
#define D_STR_DAYS "Täg"
#define D_STR_HOUR "Stund"
#define D_STR_HOURS D_STR_HOUR "e"
#define D_STR_MINUTE "Minute"
#define D_STR_MINUTES D_STR_MINUTE
#define D_STR_SECONDS D_STR_SECOND
#define D_STR_NOW "Jetz"

#define D_STR_NO "Nei"

#define D_STR_REPEAT "Wiederhole"

// IRrecvDumpV2
#define D_STR_TIMESTAMP "Ziitstämpfel"
#define D_STR_IRRECVDUMP_STARTUP \
    "IRrecvDumpV2 lauft und wartet uf IR Iigab ufem Pin %d"
#define D_WARN_BUFFERFULL \
    "WARNUNG: IR Code isch zgross für de Buffer (>= %d). " \
    "Dem Resultat sött mer nöd vertraue bevor das behobe isch. " \
    "Bearbeite & vergrössere `kCaptureBufferSize`."

#endif  // LOCALE_DE_CH_H_
