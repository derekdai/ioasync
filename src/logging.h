#include <stdio.h>

#ifndef _LOGGING_H_
#define _LOGGING_H_

#define log(fmt, ...) printf("\e[2;34m" fmt "\e[0m\n", __VA_ARGS__)
#define trace(fmt, ...) log(" T " fmt, __VA_ARGS__)
#define debug(fmt, ...) log(" D " fmt, __VA_ARGS__)
#define info(fmt, ...) log(" I " fmt, __VA_ARGS__)
#define warn(fmt, ...) log(" W " fmt, __VA_ARGS__)
#define error(fmt, ...) {       \
  log(" E " fmt, __VA_ARGS__);  \
  exit(1);                      \
}
#define fatal(fmt, ...) {       \
  log(" F " fmt, __VA_ARGS__);  \
  abort();                      \
}

#endif /* _LOGGING_H_ */
