#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <time.h>

#define _DEBUG_COLOR  "\x1b[1;32m"
#define _INFO_COLOR   "\x1b[1;34m"
#define _WARN_COLOR   "\x1b[1;33m"
#define _ERROR_COLOR  "\x1b[1;31m"
#define _RESET_COLOR  "\x1b[0m"
#define _RESET_BOLD   "\x1b[22m"

typedef enum {
  LOG_LEVEL_NONE = 0,
  LOG_LEVEL_ERROR,
  LOG_LEVEL_WARN,
  LOG_LEVEL_INFO,
  LOG_LEVEL_DEBUG,
  LOG_LEVEL_ALL = 0xff
} log_level_t;

/**
 * @brief Debug logger macro
 * 
 * @param tag File tag for identification (eg. `__FILE__` macro, const TAG string)
 * @param msg Message to be logged
 */
#define LOGDEBUG(tag, msg, ...) do { _LOG_RAW_MACRO(tag, msg, LOG_LEVEL_DEBUG, _DEBUG_COLOR __VA_OPT__(,) ##__VA_ARGS__); } while (0)

/**
 * @brief Info logger macro
 * 
 * @param tag File tag for identification (eg. `__FILE__` macro, const TAG string)
 * @param msg Message to be logged
 */
#define LOGINFO(tag, msg, ...) do { _LOG_RAW_MACRO(tag, msg, LOG_LEVEL_INFO, _INFO_COLOR __VA_OPT__(,) ##__VA_ARGS__); } while (0)

/**
 * @brief Warning logger macro
 * 
 * @param tag File tag for identification (eg. `__FILE__` macro, const TAG string)
 * @param msg Message to be logged
 */
#define LOGWARN(tag, msg, ...) do { _LOG_RAW_MACRO(tag, msg, LOG_LEVEL_WARN, _WARN_COLOR __VA_OPT__(,) ##__VA_ARGS__); } while (0)

/**
 * @brief Error logger macro
 * 
 * @param tag File tag for identification (eg. `__FILE__` macro, const TAG string)
 * @param msg Message to be logged
 */
// #define LOGERROR(tag, msg, ...) printf(_ERROR_COLOR "%s:%d > " _RESET_BOLD msg _RESET_COLOR "\n", tag, __LINE__, ##__VA_ARGS__)
#define LOGERROR(tag, msg, ...) do { _LOG_RAW_MACRO(tag, msg, LOG_LEVEL_ERROR, _ERROR_COLOR __VA_OPT__(,) ##__VA_ARGS__); } while (0)

log_level_t get_log_level();
void set_log_level(log_level_t level);

#define _LOG_RAW_MACRO(tag, msg, LEVEL, LEVEL_COLOR, ...) if (LEVEL <= get_log_level()) \
  printf( LEVEL_COLOR "%s:%d > " _RESET_BOLD msg _RESET_COLOR "\n", tag, __LINE__, ##__VA_ARGS__ )

#endif // LOGGING_H