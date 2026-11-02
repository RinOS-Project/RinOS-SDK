/* SPDX-License-Identifier: MIT */
/* Fixed-width POSIX timer queue ABI shared by libc and target kernels. */
#ifndef RIN_SDK_TIMER_ABI_H
#define RIN_SDK_TIMER_ABI_H

#if !defined(_STDINT_H)
#include <stdint.h>
#endif

#include "abi.h"
#include "../syscall_abi.h"

#define RIN_TIMERFD_SET_ABI_VERSION 2u

/* timerfd_settime request with a signed timespec value. Keeping the deadline
 * in seconds/nanoseconds preserves CLOCK_REALTIME values before the Unix
 * epoch and lets the kernel distinguish an absolute zero disarm from a
 * nonzero negative deadline. */
typedef struct RinTimerFdSetSpecV2 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    int64_t value_sec;
    int64_t value_nsec;
    uint64_t interval_ns;
    uint64_t reserved0;
} RinTimerFdSetSpecV2;

static inline int rin_timerfd_set_spec_v2_valid(
    const RinTimerFdSetSpecV2* value)
{
    return value != 0 && value->struct_size == sizeof(*value) &&
           value->version == RIN_TIMERFD_SET_ABI_VERSION &&
           value->flags == 0u && value->value_nsec >= 0 &&
           value->value_nsec < INT64_C(1000000000) &&
           value->reserved0 == 0u;
}

#if !defined(MIDL_PASS) && defined(__cplusplus)
static_assert(sizeof(RinTimerFdSetSpecV2) == 40u,
              "RinTimerFdSetSpecV2 ABI drift");
#elif !defined(MIDL_PASS)
_Static_assert(sizeof(RinTimerFdSetSpecV2) == 40u,
               "RinTimerFdSetSpecV2 ABI drift");
#endif

#ifndef MIDL_PASS
static inline int rin_timer_create_v1_valid(const RinTimerCreateV1* value)
{
    return value && value->struct_size == sizeof(*value) &&
           value->version == RIN_TIMER_ABI_VERSION &&
           (value->flags & ~RIN_TIMER_CREATE_FLAG_VALUE_TIMER_ID) == 0u &&
           (value->flags == 0u ||
            value->notify == RIN_TIMER_NOTIFY_SIGNAL) &&
           value->reserved0 == 0 &&
           (value->notify == RIN_TIMER_NOTIFY_SIGNAL ||
            value->notify == RIN_TIMER_NOTIFY_NONE ||
            value->notify == RIN_TIMER_NOTIFY_THREAD);
}

static inline int rin_timer_spec_v1_valid(const RinTimerSpecV1* value)
{
    return value && value->struct_size == sizeof(*value) &&
           value->version == RIN_TIMER_ABI_VERSION && value->flags == 0u &&
           value->reserved0 == 0u && value->reserved1 == 0u;
}

static inline int rin_clock_settime_v1_valid(
    const RinClockSettimeV1* value)
{
    return value && value->struct_size == sizeof(*value) &&
           value->version == RIN_CLOCK_SETTIME_ABI_VERSION &&
           value->flags == 0u && value->clock_id == 0 &&
           value->reserved0 == 0 &&
           value->tv_nsec >= 0 && value->tv_nsec < INT64_C(1000000000);
}

static inline int rin_clock_gettime_v1_valid(
    const RinClockGettimeV1* value, int32_t expected_clock_id)
{
    return value && value->struct_size == sizeof(*value) &&
           value->version == RIN_CLOCK_GETTIME_ABI_VERSION &&
           value->flags == 0u && value->clock_id == expected_clock_id &&
           value->reserved0 == 0 && value->tv_nsec >= 0 &&
           value->tv_nsec < INT64_C(1000000000);
}

static inline int rin_clock_settime_v1_to_ns(
    const RinClockSettimeV1* value, uint64_t* nanoseconds_out)
{
    uint64_t seconds;
    uint64_t nanoseconds;
    if (!nanoseconds_out || !rin_clock_settime_v1_valid(value) ||
        value->tv_sec < 0) return -1;
    seconds = (uint64_t)value->tv_sec;
    nanoseconds = (uint64_t)value->tv_nsec;
    if (seconds > (UINT64_MAX - nanoseconds) / UINT64_C(1000000000))
        return -2;
    *nanoseconds_out = seconds * UINT64_C(1000000000) + nanoseconds;
    return 0;
}
#endif /* !MIDL_PASS */

#endif /* RIN_SDK_TIMER_ABI_H */
