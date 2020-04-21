#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "common.h"
#include <stdio.h>
#include <assert.h>

#ifdef DEBUG

#define printflog(format, ...) \
  do { \
    fprintf(stderr, format, ## __VA_ARGS__); \
    fflush(stderr); \
  } while (0)

#else

#define printflog(format, ...)

#endif

#define Log(format, ...) \
    printflog("\33[1;34m[%s,%d,%s] " format "\33[0m\n", \
        __FILE__, __LINE__, __func__, ## __VA_ARGS__)

#define InfoN(format, ...) \
		printflog("[\33[1;35mInfo\33[0m] " format "\33[0m\n", \
				 ## __VA_ARGS__)
#define WarningN(format, ...) \
		printflog("[\33[1;35mInfo\33[0m] " format "\33[0m\n", \
				 ## __VA_ARGS__)
#define ErrorN(format, ...) \
		printflog("[\33[1;35mInfo\33[0m] " format "\33[0m\n", \
				 ## __VA_ARGS__)

#define Info(format, ...) \
		printflog("[\33[1;35mInfo\33[0m]\33[1;34m(%s,%d,%s)\33[0m " format "\33[0m\n", \
				__FILE__, __LINE__, __func__, ## __VA_ARGS__)

#define Warning(format, ...) \
		printflog("[\33[1;33mWarning\33[0m]\33[1;34m(%s,%d,%s)\33[0m " format "\33[0m\n", \
				__FILE__, __LINE__, __func__, ## __VA_ARGS__)

#define Error(format, ...) \
		printflog("[\33[1;31mError\33[0m]\33[1;34m(%s,%d,%s)\33[0m " format "\33[0m\n", \
				__FILE__, __LINE__, __func__, ## __VA_ARGS__)

#define Assert(cond, ...) \
  do { \
    if (!(cond)) { \
      fflush(stdout); \
      fprintf(stderr, "\33[1;31m"); \
      fprintf(stderr, __VA_ARGS__); \
      fprintf(stderr, "\33[0m\n"); \
      assert(cond); \
    } \
  } while (0)

#define AssertEq(a, b) \
  Assert(a == b, "%s != %s", #a, #b)

#define AssertNeq(a, b) \
  Assert(a != b, "%s == %s", #a, #b)

#define AssertNotNull(ptr) \
  AssertNeq(ptr, NULL)

#define AssertIsNull(ptr) \
  AssertEq(ptr, NULL)

#define panic(format, ...) \
  Assert(0, format, ## __VA_ARGS__)

#define TODO() panic("please implement me")

#endif