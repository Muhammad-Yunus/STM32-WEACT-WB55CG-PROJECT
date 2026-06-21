/*
 * logger.h
 *
 * Unified logging system with configurable levels.
 * Log levels: NONE, INFO, DEBUG (in increasing verbosity)
 *
 * Usage:
 *   LOG_INFO("card init OK, sectors=%lu\n", n);
 *   LOG_DEBUG("sector read at %lu\n", addr);
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdio.h>
#include <stdint.h>

/* ============================================================
 * LOG LEVEL CONFIG — change one macro to control everything
 * ============================================================
 *   LOG_LEVEL_NONE   -> no logging at all (compile out)
 *   LOG_LEVEL_INFO   -> INFO + DEBUG
 *   LOG_LEVEL_DEBUG  -> INFO + DEBUG (full verbosity)
 * ============================================================
 */
#define LOG_LEVEL_INFO

/* Level integers for comparison */
#define _LOG_NONE   0
#define _LOG_INFO   1
#define _LOG_DEBUG  2

#ifdef LOG_LEVEL_DEBUG
  #define _LOG_LEVEL _LOG_DEBUG
#elif defined(LOG_LEVEL_INFO)
  #define _LOG_LEVEL _LOG_INFO
#else
  #define _LOG_LEVEL _LOG_NONE
#endif


/* Same as above but prints level prefix for clarity */
#if _LOG_LEVEL >= _LOG_DEBUG
  #define LOG_D(...)  do { printf("[D] "); printf(__VA_ARGS__); } while(0)
#else
  #define LOG_D(...)
#endif

#if _LOG_LEVEL >= _LOG_INFO
  #define LOG_I(...)  do { printf("[I] "); printf(__VA_ARGS__); } while(0)
#else
  #define LOG_I(...)
#endif

#endif /* LOGGER_H_ */
