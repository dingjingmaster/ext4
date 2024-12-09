#ifndef EXT4_COMMON_H
#define EXT4_COMMON_H
#include "3thrd/macros/macros.h"

#define ALIGN_TO(__n, __align) ({                           \
    typeof (__n) __ret;                                     \
    if ((__n) % (__align)) {                                \
        __ret = ((__n) & (~((__align) - 1))) + (__align);   \
    } else __ret = (__n);                                   \
    __ret;                                                  \
})

#define UNUSED(__x)     ((void)(__x))
#define MIN(x, y)   ({                  \
    typeof (x) __x = (x);               \
    typeof (y) __y = (y);               \
    __x < __y ? __x : __y;              \
})

#define STATIC_ASSERT(e) static_assert(e)

#include <limits.h>
#if ULONG_MAX == 0xFFFFFFFF
#define __32BITS
#else
#define __64BITS
#endif

#endif
