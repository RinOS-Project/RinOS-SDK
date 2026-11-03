/* SPDX-License-Identifier: MIT */
/* Process-local POSIX thread scheduling request shared by libc and kernel. */

#ifndef RIN_SDK_THREAD_SCHED_ABI_H
#define RIN_SDK_THREAD_SCHED_ABI_H

#include <stddef.h>
#include <stdint.h>

#define RIN_THREAD_SCHED_ABI_VERSION UINT16_C(1)
#define RIN_THREAD_SCHED_ABI_VERSION_2 UINT16_C(2)
#define RIN_THREAD_SCHED_ABI_VERSION_3 UINT16_C(3)
#define RIN_THREAD_SCHED_GET_V3_REQUEST_MAGIC UINT16_C(0x5343)
#define RIN_THREAD_SCHED_RR_INTERVAL_USER_ABI_VERSION UINT16_C(1)
#define RIN_THREAD_SCHED_RR_INTERVAL_USER_REQUEST_MAGIC UINT16_C(0x5252)

#define RIN_THREAD_SCHED_OTHER    0
#define RIN_THREAD_SCHED_FIFO     1
#define RIN_THREAD_SCHED_RR       2
#define RIN_THREAD_SCHED_BATCH    3
#define RIN_THREAD_SCHED_IDLE     5

/* THREAD_SCHED_SET accepts this value for an atomic priority-only update.
 * The kernel checks the existing policy while applying the priority. */
#define RIN_THREAD_SCHED_POLICY_KEEP_CURRENT (-1)

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

/* V2 keeps the V1 record size while moving sched_param access into the
 * kernel. For SET, user_priority_address is copied in before scheduler
 * state changes. GETPARAM uses the dedicated scalar copyout syscall. */
typedef struct RinThreadSchedParametersV2 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    int32_t policy;
    int32_t priority;
    uint64_t user_priority_address;
} RinThreadSchedParametersV2;

/* V3 is a THREAD_SCHED_GET request. The kernel snapshots scheduler state,
 * pins both caller output ranges, then copies policy and priority together.
 * This prevents a bad second output pointer from leaving policy half-written. */
typedef struct RinThreadSchedParametersV3 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t request_magic;
    uint64_t user_policy_address;
    uint64_t user_priority_address;
} RinThreadSchedParametersV3;

/* THREAD_SCHED_RR_INTERVAL_USER_V1用request。kernelは固定幅descriptorを
 * 読み込み、呼出側配置のtimespec全体をpinしてからintervalをsnapshotし、
 * encode済みの値をpinしたrangeへ一括copyoutする。 */
typedef struct RinThreadSchedRrIntervalUserV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t request_magic;
    uint32_t time_width;
    uint32_t long_width;
    uint64_t timespec_address;
    uint64_t reserved[2];
} RinThreadSchedRrIntervalUserV1;

#if defined(__cplusplus)
static_assert(sizeof(RinThreadSchedParametersV1) == 24u,
              "RinThreadSchedParametersV1 ABI drift");
static_assert(sizeof(RinThreadSchedParametersV2) == 24u,
              "RinThreadSchedParametersV2 ABI drift");
static_assert(sizeof(RinThreadSchedParametersV3) == 24u,
              "RinThreadSchedParametersV3 ABI drift");
static_assert(sizeof(RinThreadSchedRrIntervalUserV1) == 40u,
              "RinThreadSchedRrIntervalUserV1 ABI drift");
static_assert(offsetof(RinThreadSchedRrIntervalUserV1, timespec_address) == 16u,
              "RinThreadSchedRrIntervalUserV1.timespec_address ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinThreadSchedParametersV1) == 24u,
               "RinThreadSchedParametersV1 ABI drift");
_Static_assert(sizeof(RinThreadSchedParametersV2) == 24u,
               "RinThreadSchedParametersV2 ABI drift");
_Static_assert(sizeof(RinThreadSchedParametersV3) == 24u,
               "RinThreadSchedParametersV3 ABI drift");
_Static_assert(sizeof(RinThreadSchedRrIntervalUserV1) == 40u,
               "RinThreadSchedRrIntervalUserV1 ABI drift");
_Static_assert(offsetof(RinThreadSchedRrIntervalUserV1, timespec_address) ==
                   16u,
               "RinThreadSchedRrIntervalUserV1.timespec_address ABI drift");
#endif

#endif /* RIN_SDK_THREAD_SCHED_ABI_H */
