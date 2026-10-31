/* SPDX-License-Identifier: MIT */
/* Cross-process memory advice request ABI. */

#ifndef RIN_SDK_PROCESS_MADVISE_ABI_H
#define RIN_SDK_PROCESS_MADVISE_ABI_H

#include <stdint.h>

#define RIN_PROCESS_MADVISE_ABI_VERSION UINT16_C(1)
#define RIN_PROCESS_MADVISE_MAX_COUNT UINT32_C(64)
#define RIN_PROCESS_MADVISE_MAX_BYTES UINT64_C(16777216)

#define RIN_PROCESS_MADVISE_ADVICE_NORMAL 0
#define RIN_PROCESS_MADVISE_ADVICE_RANDOM 1
#define RIN_PROCESS_MADVISE_ADVICE_SEQUENTIAL 2
#define RIN_PROCESS_MADVISE_ADVICE_WILLNEED 3
#define RIN_PROCESS_MADVISE_ADVICE_DONTNEED 4

/* A non-empty iov_base must be page-aligned; iov_len rounds up to a page.
 * Each non-empty iovec is one atomic advice operation. The return value is
 * the byte total of fully accepted ranges before the first rejected range;
 * if no range was accepted, the syscall returns its negative errno. WILLNEED
 * reports policy acceptance and remains best-effort about page-in failures. */

typedef struct RinProcessMadviseIovec32V1 {
    uint32_t base;
    uint32_t length;
} RinProcessMadviseIovec32V1;

typedef struct RinProcessMadviseIovec64V1 {
    uint64_t base;
    uint64_t length;
} RinProcessMadviseIovec64V1;

typedef struct RinProcessMadviseRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved16;
    int32_t pidfd;
    int32_t advice;
    uint32_t flags;
    uint32_t iovec_count;
    uint32_t iovec_width;
    uint32_t reserved0;
    uint64_t iovec_address;
    uint64_t reserved[3];
} RinProcessMadviseRequestV1;

#if defined(__cplusplus)
static_assert(sizeof(RinProcessMadviseIovec32V1) == 8u,
              "RinProcessMadviseIovec32V1 ABI drift");
static_assert(sizeof(RinProcessMadviseIovec64V1) == 16u,
              "RinProcessMadviseIovec64V1 ABI drift");
static_assert(sizeof(RinProcessMadviseRequestV1) == 64u,
              "RinProcessMadviseRequestV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinProcessMadviseIovec32V1) == 8u,
               "RinProcessMadviseIovec32V1 ABI drift");
_Static_assert(sizeof(RinProcessMadviseIovec64V1) == 16u,
               "RinProcessMadviseIovec64V1 ABI drift");
_Static_assert(sizeof(RinProcessMadviseRequestV1) == 64u,
               "RinProcessMadviseRequestV1 ABI drift");
#endif

#endif /* RIN_SDK_PROCESS_MADVISE_ABI_H */
