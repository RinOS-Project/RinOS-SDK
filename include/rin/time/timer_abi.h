/* SPDX-License-Identifier: MIT */
/* Fixed-width POSIX timer queue ABI shared by libc and target kernels. */
#ifndef RIN_SDK_TIMER_ABI_H
#define RIN_SDK_TIMER_ABI_H

#if !defined(_STDINT_H)
#include <stdint.h>
#endif

#include "abi.h"
#include "../syscall_abi.h"

#ifndef MIDL_PASS
static inline int rin_timer_create_v1_valid(const RinTimerCreateV1* value)
{
    return value && value->struct_size == sizeof(*value) &&
           value->version == RIN_TIMER_ABI_VERSION && value->flags == 0u &&
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
           value->reserved0 == 0 && value->tv_sec >= 0 &&
           value->tv_nsec >= 0 && value->tv_nsec < INT64_C(1000000000);
}

static inline int rin_clock_settime_v1_to_ns(
    const RinClockSettimeV1* value, uint64_t* nanoseconds_out)
{
    uint64_t seconds;
    uint64_t nanoseconds;
    if (!nanoseconds_out || !rin_clock_settime_v1_valid(value)) return -1;
    seconds = (uint64_t)value->tv_sec;
    nanoseconds = (uint64_t)value->tv_nsec;
    if (seconds > (UINT64_MAX - nanoseconds) / UINT64_C(1000000000))
        return -2;
    *nanoseconds_out = seconds * UINT64_C(1000000000) + nanoseconds;
    return 0;
}
#endif /* !MIDL_PASS */

#endif /* RIN_SDK_TIMER_ABI_H */
