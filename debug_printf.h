#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif


// Define some of the following in your Makefile to control where debug output goes.
// You can define more than one, but be wary that anything using strings may result
// in significant memory allocation, execution time, and increased firmware size.

// 1) ENABLE_DEBUG_MSGS
//    Output debug messages, with format string ID and arg list instead of strings.
//    This is the most efficient way to get debug data out of a real embedded system.
// 2) ENABLE_DEBUG_MSG_STRINGS
//    Output strings inside messages, instead of format string ID and arg list.
//    This works in a real embedded system, and doesn't require debug dictionaries, but isn't as efficient.
// 3) ENABLE_DEBUG_SEGGER_RTT
//    Output format string ID and arg list to Segger J-Link RTT.
//    This is the highest performance and lowest overhead, but only works with J-Link attached.
// 4) ENABLE_DEBUG_SEGGER_RTT_STRINGS
//    Output strings to Segger J-Link RTT.  This isn't as efficient but doesn't need
//    debug dictionaries.
// 5) ENABLE_DEBUG_STDOUT
//    Use printf, don't output messages.  Depending on your platform stdout may
//    go to the screen (Linux), a UART or USB CDC (embedded platforms).
//    This is sometimes convenient, but is the lower performance and least efficient.

#ifdef ENABLE_DEBUG_MSGS
#include "printf_dictionary.h"
// For concatenating file and line into a single token.  Needs multiple
// levels of macro invocation for some strange reason.
#define TOKENPASTE(x, y) x ## _line_ ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)

#else
#define TOKENPASTE2(x,y) (-1)
#endif

// Function declarations, with GCC attribute to perform printf-style
// argument type checking as described at.  Note the params are counts
// of fmt string and param list, starting from 1 as first param.
void _debug_printf(int count, int format_id, const char *fmt, ...)
__attribute__ ((format (printf, 3, 4)));
void _debug_warn(int count, int format_id, const char *fmt, ...)
__attribute__ ((format (printf, 3, 4)));
void _debug_error(int count, int format_id, const char *fmt, ...)
__attribute__ ((format (printf, 3, 4)));

// Macro trick to count number of arguments, and pass it as first parameter
#define PP_NARG(...)  PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) PP_NTH_ARG(__VA_ARGS__)
#define PP_NTH_ARG( _1, _2, _3, _4, _5, _6, _7, _8, _9,_10,_11,_12,_13,_14,_15,N,...) N
#define PP_RSEQ_N() 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0

// defines for macros called by user, and invocation of functions with count as first param
#define debugPrintf(...) _debug_printf(PP_NARG(__VA_ARGS__), TOKENPASTE2(ESCAPED_FILE_PATH, __LINE__), __VA_ARGS__)
#define debugWarn(...)   _debug_warn  (PP_NARG(__VA_ARGS__), TOKENPASTE2(ESCAPED_FILE_PATH, __LINE__), __VA_ARGS__)
#define debugError(...)  _debug_error (PP_NARG(__VA_ARGS__), TOKENPASTE2(ESCAPED_FILE_PATH, __LINE__), __VA_ARGS__)

#ifdef __cplusplus
}
#endif