// Copyright 2026 IRremoteESP8266 contributors

#include <cstring>
#include <string>
#include "ir_Bluestar.h"
#include "IRac.h"
#include "IRrecv.h"
#include "IRrecv_test.h"
#include "IRsend_test.h"
#include "IRutils.h"
#include "gtest/gtest.h"

namespace {
// Real ESP32 IRrecvDumpV2 captures of Blue Star IE318YKU.
struct BluestarCapture {
  const char *label;
  uint16_t raw[185];
  uint8_t state[kBluestarStateLength];
};

const BluestarCapture kBluestarCaptures[] = {
  {"light:press-1", {
    1018, 554, 1024, 552, 1022, 2566, 1020, 2568, 1022, 554, 1024, 552,
    1024, 2566, 1024, 2566, 1024, 2568, 1022, 2568, 1022, 2566, 1024, 554,
    1022, 2566, 1022, 556, 1022, 554, 1022, 554, 1022, 2566, 1022, 554,
    1022, 552, 1022, 554, 1022, 2568, 1020, 554, 1022, 2566, 1022, 2568,
    996, 580, 1022, 554, 1022, 554, 1020, 2570, 996, 578, 998, 578,
    996, 578, 998, 578, 998, 578, 1000, 576, 998, 2590, 998, 2590,
    996, 2594, 996, 578, 996, 2592, 996, 580, 996, 2592, 994, 580,
    996, 2592, 996, 2592, 1020, 2568, 996, 578, 1020, 2570, 996, 580,
    996, 580, 994, 582, 994, 582, 994, 580, 996, 582, 994, 580,
    996, 2594, 1018, 558, 994, 2594, 996, 580, 1000, 2590, 994, 582,
    996, 580, 994, 2594, 994, 582, 994, 582, 994, 582, 994, 584,
    994, 586, 992, 588, 992, 586, 992, 2600, 992, 588, 990, 2602,
    990, 2622, 970, 608, 968, 2622, 970, 610, 968, 2624, 968, 610,
    968, 2626, 966, 612, 966, 8948, 672, 3668, 672, 3668, 670, 3668,
    670, 3670, 670, 3668, 696, 3644, 674, 3666, 694, 3642, 694, 3644,
    692, 3644, 694, 3644, 668,
  }, {0x33, 0xE8, 0x8B, 0x10, 0x3A, 0xBA, 0x02, 0xA4, 0x05, 0xAA, 0x00, 0x00}},
  {"horizontal-swing:press-1:expected-off", {
    996, 578, 998, 580, 1020, 2568, 998, 2592, 1000, 578, 1000, 576,
    1000, 2592, 1020, 2570, 1024, 2568, 998, 2592, 1020, 2568, 1000, 576,
    1008, 2584, 998, 578, 998, 578, 998, 578, 1022, 2570, 998, 580,
    998, 578, 998, 578, 1024, 554, 1022, 554, 1000, 2592, 998, 2592,
    1000, 578, 998, 578, 1022, 554, 1024, 2570, 1022, 554, 1024, 552,
    1024, 554, 1020, 556, 998, 578, 998, 578, 998, 2592, 998, 578,
    1000, 2592, 1020, 2570, 998, 578, 998, 2594, 998, 2594, 996, 578,
    998, 2592, 1000, 576, 998, 2592, 1010, 2578, 1026, 550, 1024, 2566,
    1028, 552, 1026, 552, 1026, 554, 1026, 554, 1026, 2568, 1024, 2570,
    1024, 2568, 1024, 2570, 1024, 2570, 1024, 556, 1022, 2572, 1020, 560,
    1020, 560, 1020, 2572, 1018, 562, 1020, 560, 1020, 560, 1020, 562,
    1018, 562, 1018, 562, 996, 586, 994, 2602, 1018, 564, 1016, 2578,
    994, 2604, 992, 588, 992, 2604, 994, 2618, 972, 2622, 972, 2624,
    970, 2624, 970, 2624, 970, 8946, 678, 6496, 678, 3664, 676, 6498,
    674, 6496, 674, 6502, 672, 3670, 670, 3670, 668, 6526, 670, 3674,
    670, 3674, 666, 3676, 670,
  }, {0x33, 0xE8, 0x83, 0x10, 0x2D, 0xAD, 0x0F, 0xA4, 0x05, 0xBF, 0x05, 0xC8}},
  {"energy-save:press-1:expected-on", {
    992, 586, 994, 584, 994, 2602, 992, 2602, 992, 586, 1016, 564,
    1000, 2592, 1018, 2596, 976, 2600, 994, 586, 996, 586, 1016, 562,
    998, 2598, 1016, 564, 994, 584, 994, 586, 1018, 2576, 996, 586,
    994, 586, 994, 586, 1018, 562, 994, 584, 1004, 2600, 982, 586,
    994, 586, 996, 582, 1018, 2596, 998, 2576, 1018, 2576, 1018, 564,
    994, 586, 1002, 578, 1018, 562, 1020, 560, 1020, 2576, 1000, 582,
    994, 586, 994, 2600, 1018, 580, 1000, 580, 998, 2578, 1018, 580,
    974, 2620, 974, 606, 974, 606, 974, 2602, 994, 604, 976, 604,
    974, 604, 976, 604, 974, 606, 974, 2620, 974, 2622, 974, 606,
    974, 588, 992, 608, 972, 2620, 974, 606, 972, 2622, 972, 608,
    972, 606, 972, 2622, 972, 608, 972, 612, 968, 610, 972, 610,
    970, 610, 970, 612, 968, 612, 968, 2628, 966, 614, 966, 2628,
    964, 618, 962, 618, 962, 620, 960, 644, 936, 644, 936, 644,
    936, 644, 936, 2660, 936, 8982, 668, 6506, 668, 6508, 644, 6532,
    644, 6536, 638, 6534, 642, 3724, 618, 3722, 616, 3722, 618, 3724,
    616, 3724, 618, 6558, 616,
  }, {0x33, 0x88, 0x82, 0x38, 0x24, 0xA4, 0x18, 0xA4, 0x05, 0x01, 0x07, 0xC1}},
  {"fan:press-1:expected-medium", {
    1016, 558, 1020, 556, 1020, 2572, 1020, 2574, 1020, 558, 1022, 556,
    996, 2596, 1020, 2574, 1000, 576, 1022, 2572, 1020, 558, 1020, 556,
    1020, 2572, 1022, 556, 1022, 556, 1020, 556, 1022, 2592, 1000, 558,
    1020, 558, 1020, 556, 1022, 556, 1020, 556, 1022, 2572, 1020, 558,
    996, 580, 1022, 558, 1020, 556, 996, 2596, 996, 582, 998, 580,
    996, 582, 1020, 556, 1022, 558, 994, 582, 994, 582, 994, 2614,
    976, 2596, 996, 2596, 996, 582, 1020, 2570, 1022, 2568, 1022, 556,
    1022, 556, 996, 2594, 1020, 2570, 1018, 2572, 994, 582, 994, 2596,
    994, 584, 994, 584, 994, 584, 994, 2598, 994, 2598, 992, 2598,
    992, 2600, 992, 2618, 972, 2620, 972, 604, 972, 2620, 972, 610,
    972, 610, 970, 2624, 970, 612, 968, 612, 968, 614, 966, 614,
    966, 616, 964, 616, 964, 616, 962, 2658, 936, 644, 936, 2660,
    936, 644, 936, 2660, 936, 2660, 938, 2658, 960, 620, 960, 622,
    960, 620, 962, 620, 960, 8962, 644, 6534, 644, 6532, 644, 6530,
    644, 6558, 618, 6558, 618, 3724, 618, 6558, 616, 3726, 616, 3728,
    594, 3750, 592, 3748, 590,
  }, {0x33, 0x48, 0x82, 0x10, 0x1D, 0x9D, 0x1F, 0xA4, 0x05, 0x70, 0x07, 0xD0}},
  {"mode:press-2:expected-fan-only", {
    972, 584, 996, 584, 1004, 2588, 1004, 2590, 1018, 560, 1020, 558,
    1022, 2592, 1000, 2576, 1020, 558, 1022, 558, 1022, 2574, 1020, 560,
    1022, 558, 1022, 558, 1022, 558, 1022, 2574, 1020, 2594, 1000, 560,
    1020, 558, 1022, 558, 1020, 558, 1022, 558, 1022, 2574, 1020, 560,
    1022, 558, 1022, 558, 1022, 558, 1018, 2576, 998, 2598, 996, 582,
    998, 582, 996, 584, 1020, 560, 996, 604, 976, 584, 996, 2598,
    996, 602, 976, 2598, 996, 2598, 998, 582, 1020, 2574, 998, 582,
    996, 582, 1020, 2576, 994, 584, 994, 2600, 996, 2600, 996, 584,
    994, 586, 992, 586, 992, 2620, 974, 606, 972, 606, 974, 2622,
    972, 2624, 972, 608, 972, 2624, 970, 610, 972, 2624, 970, 612,
    970, 612, 966, 2630, 964, 616, 964, 618, 962, 620, 962, 620,
    958, 622, 958, 644, 936, 644, 936, 2660, 960, 620, 960, 2636,
    960, 2636, 936, 646, 934, 646, 936, 2660, 934, 646, 934, 2660,
    936, 2660, 934, 646, 934, 8982, 644, 6534, 642, 3700, 642, 6534,
    640, 6560, 616, 6558, 616, 3726, 592, 3752, 590, 3752, 592, 3752,
    590, 3778, 566, 3778, 566,
  }, {0x33, 0x21, 0x82, 0x18, 0x16, 0x96, 0x26, 0xA4, 0x05, 0x96, 0x05, 0xC0}},
  {"sleep:press-1:expected-on", {
    970, 586, 992, 584, 992, 2616, 974, 2618, 996, 562, 1016, 560,
    1006, 2588, 992, 2618, 998, 2592, 980, 580, 1010, 566, 994, 582,
    1010, 2584, 1018, 560, 994, 584, 1018, 560, 994, 2598, 994, 2600,
    992, 584, 996, 580, 1018, 560, 996, 582, 994, 2616, 998, 560,
    1018, 560, 1016, 560, 1018, 560, 1016, 2576, 992, 584, 998, 578,
    998, 580, 1012, 566, 992, 584, 994, 584, 1018, 560, 1004, 2606,
    978, 2616, 974, 584, 994, 582, 994, 2596, 1016, 2574, 994, 582,
    994, 584, 1018, 2592, 1000, 2594, 974, 602, 974, 602, 976, 2618,
    976, 602, 976, 602, 976, 2596, 994, 604, 972, 604, 974, 602,
    974, 2618, 974, 2600, 992, 2618, 976, 600, 976, 2614, 974, 602,
    976, 602, 976, 2616, 976, 602, 974, 604, 974, 604, 974, 604,
    974, 604, 974, 604, 972, 604, 974, 2620, 972, 604, 972, 2622,
    972, 2624, 972, 2622, 972, 2624, 972, 2626, 972, 610, 970, 2628,
    970, 614, 966, 616, 966, 8954, 674, 6506, 670, 6530, 646, 6536,
    642, 6532, 644, 6530, 666, 3674, 668, 3674, 668, 6510, 644, 6534,
    644, 3700, 644, 3700, 642,
  }, {0x33, 0x88, 0xC2, 0x10, 0x19, 0x99, 0x23, 0xA4, 0x05, 0xF4, 0x07, 0xCC}},
  {"power:press-1:expected-off", {
    992, 584, 996, 582, 998, 2600, 972, 2624, 998, 584, 998, 582,
    998, 2598, 998, 2600, 998, 2596, 996, 586, 996, 584, 1000, 580,
    1000, 2594, 998, 584, 996, 584, 1024, 556, 996, 586, 996, 582,
    996, 584, 996, 584, 998, 580, 994, 586, 998, 2598, 996, 584,
    996, 584, 996, 586, 994, 586, 998, 2598, 996, 2598, 994, 586,
    996, 586, 996, 584, 996, 584, 996, 586, 994, 586, 994, 2600,
    994, 586, 994, 586, 994, 2602, 994, 586, 994, 2602, 994, 586,
    994, 586, 994, 2602, 994, 588, 992, 588, 994, 2602, 994, 588,
    994, 588, 992, 588, 992, 2604, 992, 588, 992, 2604, 992, 588,
    992, 2602, 992, 590, 990, 2606, 990, 590, 990, 2606, 990, 590,
    988, 592, 988, 2610, 986, 596, 984, 618, 964, 618, 964, 618,
    964, 618, 964, 616, 964, 616, 964, 2632, 964, 616, 966, 2628,
    964, 2632, 964, 618, 964, 2632, 988, 2610, 964, 618, 964, 618,
    962, 2630, 962, 2630, 962, 8954, 672, 6504, 672, 3668, 672, 6502,
    672, 6504, 672, 6504, 672, 3670, 672, 3670, 672, 6500, 672, 6502,
    672, 3670, 672, 6504, 672,
  }, {0x33, 0x88, 0x02, 0x18, 0x12, 0x92, 0x2A, 0xA4, 0x05, 0xB3, 0x05, 0xCD}},
  {"turbo:press-1:expected-on", {
    990, 586, 996, 582, 996, 2596, 996, 2596, 1022, 560, 996, 584,
    1000, 2590, 996, 2598, 996, 2596, 1020, 560, 996, 584, 996, 584,
    994, 2598, 998, 582, 998, 582, 996, 584, 996, 2598, 998, 582,
    996, 582, 996, 2596, 996, 584, 996, 582, 998, 2594, 1024, 556,
    1000, 580, 1000, 580, 998, 578, 1000, 2594, 998, 2594, 1000, 580,
    1022, 558, 1000, 580, 1000, 580, 1000, 580, 1022, 556, 1024, 556,
    1000, 578, 1000, 2594, 1024, 2572, 1022, 554, 998, 2596, 1022, 556,
    998, 580, 998, 582, 998, 580, 998, 2596, 998, 2598, 998, 582,
    1000, 580, 1024, 556, 1026, 2568, 1026, 2570, 1026, 554, 1026, 2568,
    1028, 2570, 1024, 554, 1024, 2570, 1026, 556, 1022, 2570, 1024, 558,
    1022, 558, 1024, 2572, 1022, 558, 1020, 560, 1020, 560, 1020, 560,
    1020, 560, 1020, 562, 1018, 560, 1018, 2576, 1020, 560, 994, 2600,
    994, 586, 1018, 562, 994, 2602, 994, 586, 994, 2600, 994, 2602,
    992, 2602, 990, 2622, 972, 8942, 680, 6490, 680, 6492, 680, 6494,
    678, 6494, 678, 6498, 676, 3666, 676, 3666, 674, 6502, 674, 3670,
    672, 3670, 670, 6528, 644,
  }, {0x33, 0x88, 0x92, 0x18, 0x06, 0x86, 0x36, 0xA4, 0x05, 0x2F, 0x07, 0xC9}},
  {"convertible:press-1:expected-turbo", {
    990, 584, 996, 584, 996, 2596, 1016, 2578, 994, 584, 996, 584,
    998, 2596, 994, 2620, 980, 580, 994, 584, 996, 584, 996, 584,
    996, 2620, 976, 584, 1010, 570, 996, 584, 994, 2620, 976, 582,
    994, 586, 1000, 578, 1014, 566, 1018, 562, 1018, 2596, 998, 560,
    996, 584, 996, 584, 996, 584, 994, 2600, 994, 2600, 1018, 562,
    1018, 562, 1018, 562, 996, 584, 1018, 562, 1018, 580, 974, 586,
    1018, 582, 974, 604, 976, 604, 976, 604, 974, 2600, 992, 606,
    972, 606, 972, 606, 972, 606, 974, 604, 976, 602, 976, 584,
    994, 604, 976, 604, 974, 586, 992, 606, 974, 586, 992, 586,
    992, 586, 992, 586, 990, 2604, 990, 588, 990, 2622, 972, 606,
    974, 606, 972, 2620, 974, 608, 972, 610, 972, 610, 970, 610,
    970, 610, 970, 612, 968, 614, 968, 614, 966, 614, 966, 2630,
    964, 618, 962, 618, 960, 620, 960, 644, 936, 644, 936, 2660,
    936, 644, 936, 2660, 934, 8982, 644, 3698, 644, 3698, 644, 3698,
    644, 3700, 644, 3700, 644, 3700, 642, 3702, 642, 3724, 618, 3724,
    618, 3724, 618, 3726, 618,
  }, {0x33, 0x08, 0x82, 0x18, 0x00, 0x80, 0x00, 0xA4, 0x01, 0x05, 0x00, 0x00}},
  {"clock:step-1:00:00 AM", {
    992, 602, 974, 580, 996, 2596, 996, 2596, 996, 582, 996, 580,
    996, 2596, 994, 2596, 998, 2598, 998, 580, 996, 582, 998, 580,
    998, 2596, 1020, 556, 996, 582, 996, 580, 996, 2596, 996, 582,
    1018, 560, 998, 580, 996, 582, 996, 580, 996, 2598, 996, 582,
    996, 582, 996, 582, 996, 580, 998, 2594, 996, 580, 1000, 580,
    1024, 556, 1026, 552, 1024, 552, 1026, 552, 1028, 550, 1026, 552,
    1026, 552, 1024, 552, 1026, 552, 1024, 554, 1022, 556, 1022, 556,
    1022, 554, 1022, 554, 1022, 554, 1022, 2570, 1020, 2572, 1020, 558,
    1020, 558, 1020, 558, 1020, 558, 996, 582, 994, 582, 1018, 560,
    1018, 558, 994, 584, 994, 584, 992, 584, 994, 586, 992, 604,
    972, 604, 972, 2620, 972, 2622, 972, 606, 970, 608, 970, 606,
    970, 608, 970, 608, 968, 610, 968, 2626, 966, 610, 964, 2626,
    964, 2626, 962, 616, 960, 2632, 960, 618, 980, 620, 960, 596,
    958, 640, 960, 2632, 960, 8960, 670, 6508, 668, 3674, 644, 6532,
    644, 6532, 644, 6532, 644, 3698, 644, 3700, 644, 3698, 642, 6530,
    642, 3722, 618, 3726, 620,
  }, {0x33, 0x88, 0x82, 0x10, 0x00, 0x06, 0x00, 0x06, 0x05, 0xA1, 0x05, 0xC4}},
  {"clock:step-4:01:01 PM", {
    1018, 556, 998, 578, 996, 2594, 1024, 2568, 998, 580, 998, 578,
    1022, 2570, 996, 2594, 1022, 2568, 1022, 556, 998, 578, 1022, 554,
    998, 2596, 1022, 554, 998, 580, 1000, 578, 1022, 2570, 996, 582,
    1000, 576, 998, 580, 998, 580, 998, 578, 998, 2596, 1020, 556,
    996, 580, 1024, 554, 1024, 554, 998, 2594, 998, 578, 998, 580,
    996, 580, 998, 580, 998, 2594, 998, 580, 998, 2594, 996, 2594,
    1020, 2568, 1020, 556, 996, 2592, 1020, 2572, 998, 580, 996, 582,
    996, 582, 994, 582, 996, 580, 994, 2596, 996, 582, 994, 2596,
    994, 582, 994, 584, 994, 584, 996, 582, 994, 584, 992, 584,
    992, 588, 992, 2602, 992, 2600, 992, 590, 990, 2604, 990, 2624,
    970, 610, 970, 2626, 968, 612, 966, 614, 966, 614, 966, 616,
    964, 618, 964, 616, 964, 616, 964, 2632, 964, 616, 964, 2632,
    962, 618, 962, 616, 986, 2606, 962, 2630, 964, 2632, 986, 594,
    986, 594, 986, 594, 988, 8930, 672, 6502, 672, 3670, 672, 6504,
    672, 6504, 670, 6506, 668, 3674, 644, 3720, 622, 3718, 620, 6554,
    618, 3724, 618, 3724, 618,
  }, {0x33, 0x88, 0x82, 0x10, 0xBB, 0x05, 0x01, 0xB4, 0x05, 0x38, 0x05, 0xC4}},
};

const uint8_t kCool16[kBluestarStateLength] = {
    0x33, 0x88, 0x82, 0x10, 0x19, 0x99, 0x23, 0xA4, 0x05, 0x34, 0x05, 0xC4};
const uint8_t kCool24[kBluestarStateLength] = {
    0x33, 0x88, 0x82, 0x18, 0x06, 0x86, 0x36, 0xA4, 0x05, 0x3F, 0x05, 0xC4};
}  // namespace

TEST(TestDecodeBluestar, RealCaptures) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  for (const BluestarCapture &frame : kBluestarCaptures) {
    SCOPED_TRACE(frame.label);
    irsend.reset();
    irsend.sendRaw(frame.raw, 185, 38);
    irsend.makeDecodeResult();
    ASSERT_TRUE(irrecv.decode(&irsend.capture));
    ASSERT_EQ(BLUESTAR, irsend.capture.decode_type);
    ASSERT_EQ(91, irsend.capture.bits);
    EXPECT_STATE_EQ(frame.state, irsend.capture.state, kBluestarBits);
    EXPECT_TRUE(IRBluestarAc::validChecksum(irsend.capture.state));
    EXPECT_FALSE(IRAcUtils::resultAcToString(&irsend.capture).empty());
    stdAc::state_t common;
    EXPECT_TRUE(IRAcUtils::decodeToState(&irsend.capture, &common));
    EXPECT_EQ(BLUESTAR, common.protocol);
  }
}

TEST(TestSendBluestar, CapturedStateRoundTrips) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  for (const BluestarCapture &frame : kBluestarCaptures) {
    SCOPED_TRACE(frame.label);
    irsend.reset();
    irsend.sendBluestar(frame.state);
    // 185 captured durations plus a trailing inter-message silence.
    EXPECT_EQ(185, irsend.last);
    EXPECT_EQ(38000, irsend.freq[0]);
    EXPECT_EQ(8950, irsend.output[161]);
    EXPECT_EQ(660, irsend.output[184]);
    irsend.makeDecodeResult();
    ASSERT_TRUE(irrecv.decode(&irsend.capture));
    ASSERT_EQ(BLUESTAR, irsend.capture.decode_type);
    EXPECT_STATE_EQ(frame.state, irsend.capture.state, kBluestarBits);
  }
}

TEST(TestDecodeBluestar, ChecksumSignatureLengthAndSecondary) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  uint8_t state[kBluestarStateLength];
  std::memcpy(state, kCool24, sizeof(state));
  state[9] ^= 1;
  irsend.sendBluestar(state);
  irsend.makeDecodeResult();
  EXPECT_FALSE(irrecv.decodeBluestar(&irsend.capture));
  EXPECT_TRUE(irrecv.decodeBluestar(&irsend.capture, kStartOffset,
                                    kBluestarBits, false));
  EXPECT_FALSE(irrecv.decodeBluestar(&irsend.capture, kStartOffset, 80, false));
  irsend.reset();
  state[0] = 0x34;
  state[9] = IRBluestarAc::calcChecksum(state);
  irsend.sendBluestar(state);
  irsend.makeDecodeResult();
  EXPECT_FALSE(irrecv.decodeBluestar(&irsend.capture));
  EXPECT_TRUE(irrecv.decodeBluestar(&irsend.capture, kStartOffset,
                                    kBluestarBits, false));
  irsend.reset();
  irsend.sendBluestar(kCool24);
  irsend.makeDecodeResult();
  irsend.capture.rawlen = 163;  // Entire main frame, secondary missing.
  EXPECT_FALSE(irrecv.decodeBluestar(&irsend.capture));
  irsend.makeDecodeResult();
  irsend.capture.rawbuf[162] = 4000 / kRawTick;  // Wrong inter-section gap.
  EXPECT_FALSE(irrecv.decodeBluestar(&irsend.capture));
}

TEST(TestSendBluestar, CompleteRepeatsAndInvalidLengths) {
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  irsend.sendBluestar(kCool24, kBluestarStateLength, 1);
  EXPECT_EQ(371, irsend.last);
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decodeBluestar(&irsend.capture));
  EXPECT_STATE_EQ(kCool24, irsend.capture.state, kBluestarBits);
  irsend.makeDecodeResult(186);
  ASSERT_TRUE(irrecv.decodeBluestar(&irsend.capture));
  EXPECT_STATE_EQ(kCool24, irsend.capture.state, kBluestarBits);
  irsend.reset();
  irsend.sendBluestar(kCool24, 10);
  EXPECT_EQ("", irsend.outputStr());
  irsend.sendBluestar(nullptr);
  EXPECT_EQ("", irsend.outputStr());
  uint8_t state[kBluestarStateLength];
  std::memcpy(state, kCool24, sizeof(state));
  state[10] = 0x80;  // Invalid command storage must not be silently truncated.
  irsend.sendBluestar(state);
  EXPECT_EQ("", irsend.outputStr());
}

TEST(TestBluestar, HousekeepingAndStateSerialization) {
  EXPECT_EQ("BLUESTAR", typeToString(BLUESTAR));
  EXPECT_EQ(BLUESTAR, strToDecodeType("BLUESTAR"));
  EXPECT_TRUE(hasACState(BLUESTAR));
  EXPECT_TRUE(IRac::isProtocolSupported(BLUESTAR));
  EXPECT_EQ(91, IRsend::defaultBits(BLUESTAR));
  EXPECT_EQ(kNoRepeat, IRsend::minRepeats(BLUESTAR));
  IRsendTest irsend(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  ASSERT_TRUE(irsend.send(BLUESTAR, kCool24, kBluestarStateLength));
  irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&irsend.capture));
  EXPECT_EQ("0x33888218068636A4053F05C4",
            resultToHexidecimal(&irsend.capture));
  EXPECT_NE(std::string::npos, resultToSourceCode(&irsend.capture).find(
      "uint8_t state[12] = {0x33, 0x88, 0x82, 0x18, 0x06, 0x86, 0x36, "
      "0xA4, 0x05, 0x3F, 0x05, 0xC4};"));
}

TEST(TestBluestarClass, ChecksumAndRawStorage) {
  IRBluestarAc ac(kGpioUnused);
  EXPECT_FALSE(ac.getPower());
  EXPECT_TRUE(IRBluestarAc::validChecksum(ac.getRaw()));
  EXPECT_TRUE(ac.setRaw(kCool24));
  EXPECT_STATE_EQ(kCool24, ac.getRaw(), kBluestarBits);
  EXPECT_EQ(
      "Power: On, Mode: 8 (Cool), Temp: 24C, Fan: 4 (Low), "
      "Swing(H): Off, Swing(V): On, Light: On, Turbo: Off, Econo: Off, "
      "Sleep: Off, Capacity: 100%, Command: 1476", ac.toString());
  EXPECT_EQ(0x3F, IRBluestarAc::calcChecksum(kCool24));
  EXPECT_FALSE(IRBluestarAc::validChecksum(kCool24, 9));
  EXPECT_FALSE(IRBluestarAc::validChecksum(nullptr));
  EXPECT_FALSE(ac.setRaw(nullptr));
  EXPECT_FALSE(ac.setRaw(kCool24, 9));
  EXPECT_FALSE(ac.setRaw(kCool24, 11));
  ac.setCommand(0x7FF);
  ASSERT_TRUE(ac.setRaw(kCool16, kBluestarMainStateLength));
  EXPECT_EQ(0x7FF, ac.getCommand());
  EXPECT_EQ(16, ac.getTemp());
  ac.setCommand(0x800);  // Out-of-range input must leave valid storage intact.
  EXPECT_EQ(0x7FF, ac.getCommand());
  EXPECT_TRUE(IRBluestarAc::validChecksum(ac.getRaw()));
}

TEST(TestBluestarClass, UnknownClockAndFlagBitsArePreserved) {
  IRBluestarAc ac(kGpioUnused);
  uint8_t state[kBluestarStateLength];
  std::memcpy(state, kCool16, sizeof(state));
  state[1] |= 0x10;
  state[2] |= 0x24;
  state[3] |= 0xC0;
  state[8] |= 0xA8;
  ASSERT_TRUE(ac.setRaw(state));
  ac.setMode(kBluestarDry);
  ac.setTemp(29);
  ac.setFan(kBluestarFanMed);
  ac.setPower(true);
  ac.setLight(false);
  ac.setSleep(true);
  ac.setTurbo(true);
  ac.setEcono(true);
  ac.setCapacity(kBluestarCapacity60);
  ac.setSwingH(false);
  ac.setSwingV(true);
  const uint8_t *raw = ac.getRaw();
  EXPECT_EQ(0x10, raw[1] & 0x10);
  EXPECT_EQ(0x24, raw[2] & 0x24);
  EXPECT_EQ(0xC0, raw[3] & 0xC0);
  EXPECT_EQ(0xA8, raw[8] & 0xF8);
  for (uint8_t i = 4; i <= 7; i++) EXPECT_EQ(state[i], raw[i]);
  EXPECT_TRUE(IRBluestarAc::validChecksum(raw));
}

TEST(TestBluestarClass, PowerAndTemperatureBoundaries) {
  IRBluestarAc ac(kGpioUnused);
  ASSERT_TRUE(ac.setRaw(kCool16));
  ac.off();
  EXPECT_FALSE(ac.getPower());
  EXPECT_EQ(24, ac.getTemp());
  ac.on();
  EXPECT_TRUE(ac.getPower());
  EXPECT_EQ(24, ac.getTemp());
  ac.setTemp(16);  // Lower temperatures remain legal in ordinary On packets.
  EXPECT_EQ(16, ac.getTemp());
  ac.setTemp(0);
  EXPECT_EQ(16, ac.getTemp());
  ac.setTemp(255);
  EXPECT_EQ(30, ac.getTemp());
  ac.setTemp(25);
  ac.off();
  ac.on();
  EXPECT_EQ(25, ac.getTemp());
  EXPECT_EQ(kBluestarCommandPower, ac.getCommand());
}

TEST(TestBluestarClass, ModeAndFanOnlyHiddenTemperature) {
  IRBluestarAc ac(kGpioUnused);
  ASSERT_TRUE(ac.setRaw(kCool16));
  ac.setMode(kBluestarDry);
  EXPECT_EQ(kBluestarDry, ac.getMode());
  EXPECT_EQ(16, ac.getTemp());  // No invented remembered Dry preference.
  ac.setTemp(27);
  ac.setMode(kBluestarFanOnly);
  EXPECT_EQ(24, ac.getTemp());
  ac.setTemp(19);  // No user temperature control in Fan-only.
  EXPECT_EQ(24, ac.getTemp());
  ac.setMode(kBluestarAuto);
  ac.setTemp(23);
  EXPECT_EQ(23, ac.getTemp());
  ac.setMode(0xFF);
  EXPECT_EQ(kBluestarCool, ac.getMode());
  EXPECT_EQ(kBluestarCommandMode, ac.getCommand());
}

TEST(TestBluestarClass, CapturedFanCycle) {
  IRBluestarAc ac(kGpioUnused);
  const uint8_t medium[kBluestarStateLength] = {
      0x33, 0x48, 0x82, 0x10, 0x1D, 0x9D, 0x1F, 0xA4, 0x05, 0x70, 0x07, 0xD0};
  const uint8_t high[kBluestarStateLength] = {
      0x33, 0x28, 0x82, 0x10, 0x1D, 0x9D, 0x1F, 0xA4, 0x05, 0x90, 0x07, 0xD0};
  const uint8_t automatic[kBluestarStateLength] = {
      0x33, 0xE8, 0x82, 0x10, 0x1D, 0x9D, 0x1F, 0xA4, 0x05, 0xD0, 0x07, 0xD0};
  const uint8_t low[kBluestarStateLength] = {
      0x33, 0x88, 0x82, 0x10, 0x1D, 0x9D, 0x1F, 0xA4, 0x05, 0x30, 0x07, 0xD0};
  ASSERT_TRUE(ac.setRaw(medium));
  ac.setFan(kBluestarFanHigh);
  EXPECT_STATE_EQ(high, ac.getRaw(), kBluestarBits);
  ac.setFan(kBluestarFanAuto);
  EXPECT_STATE_EQ(automatic, ac.getRaw(), kBluestarBits);
  ac.setFan(kBluestarFanLow);
  EXPECT_STATE_EQ(low, ac.getRaw(), kBluestarBits);
  ac.setFan(kBluestarFanMed);
  EXPECT_STATE_EQ(medium, ac.getRaw(), kBluestarBits);
}

TEST(TestBluestarClass, CapturedEnergySaveCoupling) {
  IRBluestarAc ac(kGpioUnused);
  const uint8_t baseline[kBluestarStateLength] = {
      0x33, 0xE8, 0x82, 0x10, 0x24, 0xA4, 0x18, 0xA4, 0x05, 0xC9, 0x05, 0xC4};
  const uint8_t on[kBluestarStateLength] = {
      0x33, 0x88, 0x82, 0x38, 0x24, 0xA4, 0x18, 0xA4, 0x05, 0x01, 0x07, 0xC1};
  const uint8_t off[kBluestarStateLength] = {
      0x33, 0x88, 0x82, 0x18, 0x24, 0xA4, 0x18, 0xA4, 0x05, 0x21, 0x07, 0xC1};
  ASSERT_TRUE(ac.setRaw(baseline));
  ac.setEcono(true);
  EXPECT_TRUE(ac.getEcono());
  EXPECT_STATE_EQ(on, ac.getRaw(), kBluestarBits);
  ac.setEcono(false);
  EXPECT_FALSE(ac.getEcono());
  EXPECT_STATE_EQ(off, ac.getRaw(), kBluestarBits);
  EXPECT_EQ(24, ac.getTemp());
  EXPECT_EQ(kBluestarFanLow, ac.getFan());
}

TEST(TestBluestarClass, StandaloneTurboPreservesBaseSettings) {
  IRBluestarAc ac(kGpioUnused);
  const uint8_t on[kBluestarStateLength] = {
      0x33, 0x88, 0x92, 0x18, 0x06, 0x86, 0x36, 0xA4, 0x05, 0x2F, 0x07, 0xC9};
  ASSERT_TRUE(ac.setRaw(kCool24));
  ac.setTurbo(true);
  EXPECT_STATE_EQ(on, ac.getRaw(), kBluestarBits);
  EXPECT_EQ(24, ac.getTemp());
  EXPECT_EQ(kBluestarFanLow, ac.getFan());
  EXPECT_TRUE(ac.getTurbo());
  EXPECT_EQ(24, ac.toCommon().degrees);
  EXPECT_EQ(stdAc::fanspeed_t::kLow, ac.toCommon().fanspeed);
  EXPECT_EQ(kBluestarCapacityDefault, ac.getCapacity());
  ac.setTurbo(false);
  EXPECT_EQ(24, ac.getTemp());
  for (uint8_t i = 0; i < 10; i++) EXPECT_EQ(kCool24[i], ac.getRaw()[i]);
}

TEST(TestBluestarClass, CapturedConvertibleCycleIsDistinctFromTurbo) {
  IRBluestarAc ac(kGpioUnused);
  const uint8_t states[][kBluestarStateLength] = {
    {0x33, 0x88, 0x82, 0x18, 0x00, 0x80, 0x00, 0xA4, 0x05, 0x81, 0x05, 0xC4},
    {0x33, 0x08, 0x82, 0x18, 0x00, 0x80, 0x00, 0xA4, 0x01, 0x05, 0x00, 0x00},
    {0x33, 0x28, 0x82, 0x18, 0x00, 0x80, 0x00, 0xA4, 0x02, 0xE4, 0x00, 0x00},
    {0x33, 0x48, 0x82, 0x18, 0x00, 0x80, 0x00, 0xA4, 0x03, 0xC3, 0x00, 0x00},
    {0x33, 0x88, 0x82, 0x18, 0x00, 0x80, 0x00, 0xA4, 0x04, 0x82, 0x00, 0x00},
    {0x33, 0x88, 0x82, 0x18, 0x00, 0x80, 0x00, 0xA4, 0x05, 0x81, 0x00, 0x00}};
  ASSERT_TRUE(ac.setRaw(states[0]));
  for (uint8_t capacity = 1; capacity <= 5; capacity++) {
    ac.setCapacity(capacity);
    EXPECT_STATE_EQ(states[capacity], ac.getRaw(), kBluestarBits);
    EXPECT_FALSE(ac.getTurbo());
    EXPECT_EQ(24, ac.getTemp());
  }
  ac.setCapacity(kBluestarCapacityTurbo);
  EXPECT_EQ(kBluestarFanTurbo, ac.getFan());
  EXPECT_TRUE(ac.toCommon().turbo);
  EXPECT_EQ(24, ac.toCommon().degrees);
}

TEST(TestBluestarClass, CapturedHorizontalSwingPolarity) {
  IRBluestarAc ac(kGpioUnused);
  // Hardware testing confirmed byte 2 bit 0 set enables horizontal swing.
  // These real captured frames keep the other fields identical.
  const uint8_t on[kBluestarStateLength] = {
      0x33, 0xE8, 0x83, 0x10, 0x2D, 0xAD, 0x0F, 0xA4, 0x05, 0xBF, 0x05, 0xC8};
  const uint8_t off[kBluestarStateLength] = {
      0x33, 0xE8, 0x82, 0x10, 0x2D, 0xAD, 0x0F, 0xA4, 0x05, 0xC0, 0x05, 0xC8};
  ASSERT_TRUE(ac.setRaw(off));
  EXPECT_FALSE(ac.getSwingH());
  EXPECT_EQ(stdAc::swingh_t::kOff, ac.toCommon().swingh);
  ac.setSwingH(true);
  EXPECT_STATE_EQ(on, ac.getRaw(), kBluestarBits);
  EXPECT_TRUE(ac.getSwingH());
  EXPECT_TRUE(ac.getSwingV());
  EXPECT_EQ(stdAc::swingh_t::kAuto, ac.toCommon().swingh);
  ac.setSwingH(false);
  EXPECT_STATE_EQ(off, ac.getRaw(), kBluestarBits);
  EXPECT_FALSE(ac.getSwingH());
  EXPECT_TRUE(ac.getSwingV());
  EXPECT_TRUE(IRBluestarAc::validChecksum(ac.getRaw()));
}

TEST(TestBluestarClass, CapturedVerticalSwingPolarity) {
  IRBluestarAc ac(kGpioUnused);
  // Real captured swing frames. The user confirmed on the A/C that byte 2
  // bit 1 set enables vertical swing; earlier planned labels were reversed.
  const uint8_t on[kBluestarStateLength] = {
      0x33, 0xE8, 0x82, 0x10, 0x28, 0xA8, 0x14, 0xA4, 0x05, 0xC5, 0x05, 0xC8};
  const uint8_t off[kBluestarStateLength] = {
      0x33, 0xE8, 0x80, 0x10, 0x28, 0xA8, 0x14, 0xA4, 0x05, 0xC7, 0x05, 0xC8};
  ASSERT_TRUE(ac.setRaw(off));
  EXPECT_FALSE(ac.getSwingV());
  EXPECT_EQ(stdAc::swingv_t::kOff, ac.toCommon().swingv);
  ac.setSwingV(true);
  EXPECT_STATE_EQ(on, ac.getRaw(), kBluestarBits);
  EXPECT_TRUE(ac.getSwingV());
  EXPECT_FALSE(ac.getSwingH());
  EXPECT_EQ(stdAc::swingv_t::kAuto, ac.toCommon().swingv);
  ac.setSwingV(false);
  EXPECT_STATE_EQ(off, ac.getRaw(), kBluestarBits);
  EXPECT_FALSE(ac.getSwingV());
  EXPECT_FALSE(ac.getSwingH());
  EXPECT_TRUE(IRBluestarAc::validChecksum(ac.getRaw()));
}

TEST(TestBluestarClass, IndependentFlagsAndCommonConversion) {
  IRBluestarAc ac(kGpioUnused);
  ASSERT_TRUE(ac.setRaw(kCool16));
  ac.setSwingH(true);
  EXPECT_TRUE(ac.getSwingH());
  EXPECT_TRUE(ac.getSwingV());
  ac.setSwingV(false);
  EXPECT_FALSE(ac.getSwingV());
  EXPECT_TRUE(ac.getSwingH());
  ac.setLight(false);
  ac.setSleep(true);
  EXPECT_FALSE(ac.getLight());
  EXPECT_TRUE(ac.getSleep());
  EXPECT_EQ(kBluestarCommandSleep, ac.getCommand());
  const stdAc::state_t common = ac.toCommon();
  EXPECT_EQ(BLUESTAR, common.protocol);
  EXPECT_EQ(-1, common.model);
  EXPECT_EQ(stdAc::swingh_t::kAuto, common.swingh);
  EXPECT_EQ(stdAc::swingv_t::kOff, common.swingv);
  EXPECT_EQ(0, common.sleep);
  EXPECT_FALSE(common.light);
  EXPECT_EQ(-1, common.clock);
  ac.setSleep(false);
  ac.setLight(true);
  ac.setSwingH(false);
  ac.setSwingV(true);
  for (uint8_t i = 0; i < 10; i++) EXPECT_EQ(kCool16[i], ac.getRaw()[i]);
}

TEST(TestBluestarClass, CommonSenderUsesDesiredState) {
  IRac common(kGpioUnused);
  IRBluestarAc ac(kGpioUnused);
  IRrecv irrecv(kGpioUnused);
  common.bluestar(&ac, true, stdAc::opmode_t::kCool, 18,
                   stdAc::fanspeed_t::kHigh, stdAc::swingv_t::kOff,
                   stdAc::swingh_t::kAuto, false, false, true, -1);
  ac._irsend.makeDecodeResult();
  ASSERT_TRUE(irrecv.decode(&ac._irsend.capture));
  ASSERT_EQ(BLUESTAR, ac._irsend.capture.decode_type);
  // Power On, horizontal On and vertical Off; both swing bits are active high.
  EXPECT_EQ(0x81, ac._irsend.capture.state[2]);
  stdAc::state_t decoded;
  ASSERT_TRUE(IRAcUtils::decodeToState(&ac._irsend.capture, &decoded));
  EXPECT_TRUE(decoded.power);
  EXPECT_EQ(18, decoded.degrees);
  EXPECT_EQ(stdAc::fanspeed_t::kHigh, decoded.fanspeed);
  EXPECT_EQ(stdAc::swingh_t::kAuto, decoded.swingh);
  EXPECT_EQ(stdAc::swingv_t::kOff, decoded.swingv);
  EXPECT_TRUE(decoded.light);
  EXPECT_FALSE(decoded.turbo);
  EXPECT_FALSE(decoded.econo);
  EXPECT_EQ(-1, decoded.sleep);
  stdAc::state_t desired = decoded;
  desired.protocol = BLUESTAR;
  EXPECT_TRUE(common.sendAc(desired));
}
