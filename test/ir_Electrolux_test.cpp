// Copyright 2024 Andrey Kravchenko (StellaLupus)

#include "ir_Electrolux.h"
#include "IRac.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend.h"
#include "IRsend_test.h"
#include "gtest/gtest.h"


TEST(TestUtils, Housekeeping) {
  ASSERT_EQ("ELETROLUX_AC", typeToString(decode_type_t::ELECTROLUX_AC));
  ASSERT_EQ(decode_type_t::ELECTROLUX_AC, strToDecodeType("ELETROLUX_AC"));
  ASSERT_FALSE(hasACState(decode_type_t::ELECTROLUX_AC));
  ASSERT_TRUE(IRac::isProtocolSupported(decode_type_t::ELECTROLUX_AC));
  ASSERT_EQ(kElectroluxAcBits, IRsend::defaultBits(decode_type_t::ELECTROLUX_AC));
  ASSERT_EQ(kElectroluxAcDefaultRepeat, IRsend::minRepeats(decode_type_t::ELECTROLUX_AC));
}