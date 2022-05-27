#include <stdio.h>

#ifndef _LOGGING_H_
#define _LOGGING_H_

#define FATAL 0
#define ERROR 1
#define WARN  2
#define INFO  3
#define DEBUG 4
#define TRACE 5

#ifndef LOG_LEVEL
#define LOG_LEVEL INFO
#endif

#define TRACE_STYLE "\e[35mT\e[90m"
#define DEBUG_STYLE "\e[34mD\e[90m"
#define INFO_STYLE  "\e[32mI\e[90m"
#define WARN_STYLE  "\e[33mW\e[90m"
#define ERROR_STYLE "\e[91mE\e[90m"
#define FATAL_STYLE "\e[93;41mF\e[90m"

#define log(prx, fmt, sfx, ...) printf(prx " %s:%d %s() " fmt sfx "\n", __BASE_FILE__, __LINE__, __func__, __VA_ARGS__)

#if LOG_LEVEL >= TRACE
#define trace(fmt, ...) log(TRACE_STYLE, fmt, "\e[0m", __VA_ARGS__)
#else
#define trace(fmt, ...)
#endif

#if LOG_LEVEL >= DEBUG
#define debug(fmt, ...) log(DEBUG_STYLE, fmt, "\e[0m", __VA_ARGS__)
#else
#define debug(fmt, ...)
#endif

#if LOG_LEVEL >= INFO
#define info(fmt, ...) log(INFO_STYLE, fmt, "\e[0m", __VA_ARGS__)
#else
#define info(fmt, ...)
#endif

#if LOG_LEVEL >= WARN
#define warn(fmt, ...) log(WARN_STYLE, fmt, "\e[0m", __VA_ARGS__)
#else
#define warn(fmt, ...)
#endif

#if LOG_LEVEL >= ERROR
#define error(fmt, ...) {       \
  log(ERROR_STYLE, fmt, "\e[0m", __VA_ARGS__);  \
  exit(1);                      \
}
#else
#define error(fmt, ...)
#endif

#if LOG_LEVEL >= FATAL
#define fatal(fmt, ...) {       \
  log(FATAL_STYLE, fmt, "\e[0m", __VA_ARGS__);  \
  abort();                      \
}
#else
#define fatal(fmt, ...)
#endif

#endif /* _LOGGING_H_ */
