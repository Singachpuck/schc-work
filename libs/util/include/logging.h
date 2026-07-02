#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>

#define _INFO_COLOR   "\x1b[34m"
#define _WARN_COLOR   "\x1b[33m"
#define _ERROR_COLOR  "\x1b[31m"
#define _RESET_COLOR  "\x1b[0m"

/**
 * @brief Info logger macro
 * 
 * @param tag File tag for identification (eg. `__FILE__` macro, const TAG string)
 * @param msg Message to be logged
 */
#define LOGINFO(tag, msg, ...) printf(_INFO_COLOR "%s:%d > " _RESET_COLOR msg "\n", tag, __LINE__, ##__VA_ARGS__)

/**
 * @brief Warning logger macro
 * 
 * @param tag File tag for identification (eg. `__FILE__` macro, const TAG string)
 * @param msg Message to be logged
 */
#define LOGWARN(tag, msg, ...) printf(_WARN_COLOR "%s:%d > " _RESET_COLOR msg "\n", tag, __LINE__, ##__VA_ARGS__)

/**
 * @brief Error logger macro
 * 
 * @param tag File tag for identification (eg. `__FILE__` macro, const TAG string)
 * @param msg Message to be logged
 */
#define LOGERROR(tag, msg, ...) printf(_ERROR_COLOR "%s:%d > " _RESET_COLOR msg "\n", tag, __LINE__, ##__VA_ARGS__)

#endif // LOGGING_H