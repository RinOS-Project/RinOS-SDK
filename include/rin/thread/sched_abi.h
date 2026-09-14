/* SPDX-License-Identifier: MIT */
/* Process-local POSIX thread scheduling request shared by libc and kernel. */

#ifndef RIN_SDK_THREAD_SCHED_ABI_H
#define RIN_SDK_THREAD_SCHED_ABI_H

#include <stdint.h>

#define RIN_THREAD_SCHED_ABI_VERSION UINT16_C(1)

#define RIN_THREAD_SCHED_OTHER    0
#define RIN_THREAD_SCHED_FIFO     1
#define RIN_THREAD_SCHED_RR       2
#define RIN_THREAD_SCHED_BATCH    3
#define RIN_THREAD_SCHED_IDLE     5

/* The structure is deliberately fixed-width: pthread_t is a target word,
 * while the syscall ABI itself carries a process-local TID and this record. */
typedef struct RinThreadSchedParametersV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    int32_t policy;
    int32_t priority;
    uint64_t reserved1;
} RinThreadSchedParametersV1;

#if defined(__cplusplus)
static_assert(sizeof(RinThreadSchedParametersV1) == 24u,
              "RinThreadSchedParametersV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinThreadSchedParametersV1) == 24u,
               "RinThreadSchedParametersV1 ABI drift");
#endif

#endif /* RIN_SDK_THREAD_SCHED_ABI_H */
