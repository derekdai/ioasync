#include <string.h>
#include <errno.h>

#ifndef __UTILS_H_
#define __UTILS_H_

#define fd_check(fd) ((fd) >= 0)

#define Managed(t, f) __attribute__((cleanup(f))) t

#define FdVar Managed(Fd, fd_close)

#define ccall(fn, ...) ({                                 \
  typeof(fn(__VA_ARGS__)) r = fn(__VA_ARGS__);            \
  if(r == -1) {                                           \
    warn(#fn "() == -1: %s", strerror(errno));           \
    return r;                                             \
  }                                                       \
  r;                                                      \
})

#endif /* __UTILS_H_ */
