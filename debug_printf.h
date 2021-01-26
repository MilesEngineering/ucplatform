#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

// uncomment to use normal printf to stdio instead of redirecting it
//#define USE_PRINTF_DIRECTLY

#ifdef USE_PRINTF_DIRECTLY

#define debugPrintf printf
#define debugWarn printf
#define debugError printf

#else

#include "printf_dictionary.h"

// For concatenating file and line into a single token.  Needs multiple
// levels of macro invocation for some strange reason.
#define TOKENPASTE(x, y) x ## _line_ ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)

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

#endif

#ifdef __cplusplus
}
#endif