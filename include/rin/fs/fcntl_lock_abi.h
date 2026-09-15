/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_FCNTL_LOCK_ABI_H
#define RIN_SDK_FCNTL_LOCK_ABI_H

#include <stdint.h>

/* Linux-compatible wire layouts with no target-width long/pointer fields. */
#if !defined(MIDL_PASS)
#define RIN_FCNTL_LOCK_ALIGN __attribute__((aligned(8)))
#else
#define RIN_FCNTL_LOCK_ALIGN
#endif

typedef struct RIN_FCNTL_LOCK_ALIGN RinNativeFlockAbi {
    int16_t type;
    int16_t whence;
    int64_t start;
    int64_t length;
    int32_t pid;
    int32_t reserved;
} RinNativeFlockAbi;

typedef struct RinCompat32FlockAbi {
    int16_t type;
    int16_t whence;
    int32_t start;
    int32_t length;
    int32_t pid;
} RinCompat32FlockAbi;

#if defined(__cplusplus) && !defined(MIDL_PASS)
static_assert(sizeof(RinNativeFlockAbi) == 32u,
              "native fcntl lock ABI drift");
static_assert(sizeof(RinCompat32FlockAbi) == 16u,
              "compat32 fcntl lock ABI drift");
#elif !defined(MIDL_PASS)
_Static_assert(sizeof(RinNativeFlockAbi) == 32u,
               "native fcntl lock ABI drift");
_Static_assert(sizeof(RinCompat32FlockAbi) == 16u,
               "compat32 fcntl lock ABI drift");
#endif

#undef RIN_FCNTL_LOCK_ALIGN

#endif
