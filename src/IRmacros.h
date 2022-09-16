#ifndef IRMACROS_H_
#define IRMACROS_H_
/****************************************************************
 * Copyright 2022 IRremoteESP8266 project and others
 */
/// @file IRmacros.h

/**
 * PP_NARG() macro get number of arguments provided
 * Source:
 * http://jonjagger.blogspot.com/2010/11/c-macro-magic-ppnarg.html
 * ('base' version)
 */
/// @cond TEST
#define PP_NARG(...) \
    PP_NARG_(__VA_ARGS__, PP_RSEQ_N())

#define PP_NARG_(...) \
    PP_ARG_N(__VA_ARGS__)

#define PP_ARG_N( \
    _1, _2, _3, _4, _5, _6, N, ...)   (N)

#define PP_RSEQ_N() \
    6,5,4,3,2,1,0
/// @endcond
/**
 * PP_NARG() end
 */

/**
 * COND() Set of macros to facilitate single-line conditional compilation
 * argument checking.
 * Found here: https://www.reddit.com/r/C_Programming/comments/ud3xrv/
 * conditional_preprocessor_macro_anyone/
 * 
 * Usage:
 * COND(<define_to_test>[||/&&<more_define>...], <true_result>, <false_result>)
 * 
 * NB: If __VA_OPT__ macro not supported, the <true_result> will be expanded!
 */
/// @cond TEST
#if PP_NARG(__VA_OPT__(,)) != 2
#define COND(cond, a, b) a
#else
#define NOTHING
#define EXPAND(...) __VA_ARGS__
#define STUFF_P(a, ...) __VA_OPT__(a)
#define STUFF(...) STUFF_P(__VA_ARGS__)
#define VA_TEST_P(a, ...) __VA_OPT__(NO)##THING
#define VA_TEST(...) VA_TEST_P(__VA_ARGS__)
#define NEGATE(a) VA_TEST(a, a)
#define COND_P(cond, a, b) STUFF(a, cond)STUFF(b, NEGATE(cond))
#define COND(cond, a, b) EXPAND(COND_P(cond, a, b))
#endif
/// @endcond
/**
 * end of COND() set of macros
 */

#endif  // IRMACROS_H_
