#ifndef IR_MAGIQUEST_H_
#define IR_MAGIQUEST_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include "IRremoteESP8266.h"
#include "IRsend.h"

// MagiQuest packet is both Wand ID and magnitude of swish and flick
union magiquest {
  uint64_t llword;
  uint8_t    byte[8];
//  uint16_t   word[4];
  uint32_t  lword[2];
  struct {
    uint16_t magnitude;
    uint32_t wand_id;
    uint8_t  padding;
    uint8_t  scrap;
  } cmd ;
} ;

#define MAGIQUEST_BITS               56
#define MAGIQUEST_TOTAL_USEC       1150
#define MAGIQUEST_ZERO_RATIO         30  // usually <= ~25%
#define MAGIQUEST_ONE_RATIO          38  // usually >= ~50%

#define MAGIQUEST_PERIOD             1150
#define MAGIQUEST_MARK_ZERO  280
#define MAGIQUEST_SPACE_ZERO 850
#define MAGIQUEST_MARK_ONE   580
#define MAGIQUEST_SPACE_ONE  600

#endif // IR_MAGIQUEST_H_
