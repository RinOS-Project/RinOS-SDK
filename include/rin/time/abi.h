/* SPDX-License-Identifier: MIT */
/* Public POSIX clock, timer, and CPU-time record layouts. */
#ifndef RIN_SDK_TIME_ABI_H
#define RIN_SDK_TIME_ABI_H

#include <stdint.h>

#define RIN_TIMER_ABI_VERSION 1u
#define RIN_TIMER_SET_ABI_VERSION 2u
#define RIN_TIMER_CREATE_FLAG_VALUE_TIMER_ID 1u
#define RIN_ITIMER_ABI_VERSION 1u
#define RIN_TIMER_MAX_SLOTS 32u
#define RIN_TIMER_NOTIFY_SIGNAL 0u
#define RIN_TIMER_NOTIFY_NONE 1u
#define RIN_TIMER_NOTIFY_THREAD 2u
#define RIN_TIMER_THREAD_SIGNAL 62u
#define RIN_CLOCK_SETTIME_ABI_VERSION 1u
#define RIN_CLOCK_GETTIME_ABI_VERSION 1u
#define RIN_CLOCK_ID_REALTIME 0
#define RIN_CLOCK_ID_MONOTONIC 1
#define RIN_CLOCK_ID_PROCESS_CPUTIME 2
#define RIN_CLOCK_ID_THREAD_CPUTIME 3
#define RIN_TIMER_ABSTIME 1u
#define RIN_CPU_TIME_ABI_VERSION 1u
#define RIN_TIMES_ABI_VERSION 1u
#define RIN_TIMES_CLOCK_TICKS_PER_SECOND 100u

#ifndef RIN_SDK_TIME_ABI_DEFINED
#define RIN_SDK_TIME_ABI_DEFINED
typedef struct RinCpuTimeV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t scope;
    uint64_t cpu_time_ns;
    uint64_t reserved0;
    uint64_t reserved1;
} RinCpuTimeV1;

typedef struct RinItimerV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint64_t value_us;
    uint64_t interval_us;
    uint64_t reserved;
} RinItimerV1;

typedef struct RinTimerCreateV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    int32_t clock_id;
    int32_t notify;
    int32_t signo;
    int32_t reserved0;
    uint64_t value; /* Raw union sigval bits for SIGEV_SIGNAL. */
} RinTimerCreateV1;

typedef struct RinTimerSpecV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t value_ns;
    uint64_t interval_ns;
    uint64_t reserved0;
    uint64_t reserved1;
} RinTimerSpecV1;

/* POSIX timer_settime input. Absolute values remain split into signed seconds
 * and normalized nanoseconds so CLOCK_REALTIME deadlines before the Unix epoch
 * and deadlines across that epoch are representable. Relative values use
 * value_ns; absolute requests use absolute_value_sec/nsec. */
typedef struct RinTimerSetSpecV2 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t value_ns;
    uint64_t interval_ns;
    int64_t absolute_value_sec;
    int64_t absolute_value_nsec;
    uint64_t reserved0;
} RinTimerSetSpecV2;

static inline int rin_timer_set_spec_v2_valid(
    const RinTimerSetSpecV2* value)
{
    return value != 0 && value->struct_size == sizeof(*value) &&
           value->version == RIN_TIMER_SET_ABI_VERSION &&
           value->flags == 0u && value->absolute_value_nsec >= 0 &&
           value->absolute_value_nsec < INT64_C(1000000000) &&
           value->reserved0 == 0u;
}

typedef struct RinClockSettimeV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    int32_t clock_id;
    int32_t reserved0;
    int64_t tv_sec;
    int64_t tv_nsec;
} RinClockSettimeV1;

typedef struct RinClockGettimeV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    int32_t clock_id;
    int32_t reserved0;
    int64_t tv_sec;
    int64_t tv_nsec;
} RinClockGettimeV1;

static inline int rin_clock_gettime_v1_request_valid(
    const RinClockGettimeV1* value)
{
    return value && value->struct_size == sizeof(*value) &&
           value->version == RIN_CLOCK_GETTIME_ABI_VERSION &&
           value->flags == 0u && value->reserved0 == 0 &&
           value->tv_sec == 0 && value->tv_nsec == 0 &&
           value->clock_id >= RIN_CLOCK_ID_REALTIME &&
           value->clock_id <= RIN_CLOCK_ID_THREAD_CPUTIME;
}

#if !defined(MIDL_PASS) && defined(__cplusplus)
static_assert(sizeof(RinCpuTimeV1) == 32u, "RinCpuTimeV1 ABI drift");
static_assert(sizeof(RinItimerV1) == 32u, "RinItimerV1 ABI drift");
static_assert(sizeof(RinTimerCreateV1) == 32u, "RinTimerCreateV1 ABI drift");
static_assert(sizeof(RinTimerSpecV1) == 40u, "RinTimerSpecV1 ABI drift");
static_assert(sizeof(RinTimerSetSpecV2) == 48u,
              "RinTimerSetSpecV2 ABI drift");
static_assert(sizeof(RinClockSettimeV1) == 32u,
              "RinClockSettimeV1 ABI drift");
static_assert(sizeof(RinClockGettimeV1) == 32u,
              "RinClockGettimeV1 ABI drift");
#elif !defined(MIDL_PASS)
_Static_assert(sizeof(RinCpuTimeV1) == 32u, "RinCpuTimeV1 ABI drift");
_Static_assert(sizeof(RinItimerV1) == 32u, "RinItimerV1 ABI drift");
_Static_assert(sizeof(RinTimerCreateV1) == 32u, "RinTimerCreateV1 ABI drift");
_Static_assert(sizeof(RinTimerSpecV1) == 40u, "RinTimerSpecV1 ABI drift");
_Static_assert(sizeof(RinClockSettimeV1) == 32u,
               "RinClockSettimeV1 ABI drift");
_Static_assert(sizeof(RinClockGettimeV1) == 32u,
               "RinClockGettimeV1 ABI drift");
#endif
#endif /* RIN_SDK_TIME_ABI_DEFINED */

/* `times` writes the target-width POSIX struct tms. Keep these newer V1
 * declarations independently guarded because contract_abi.h also publishes
 * older time records under RIN_SDK_TIME_ABI_DEFINED. */
#ifndef RIN_SDK_TIMES_ABI_DEFINED
#define RIN_SDK_TIMES_ABI_DEFINED
typedef struct RinTimesTms32V1 {
    int32_t user_ticks;
    int32_t system_ticks;
    int32_t child_user_ticks;
    int32_t child_system_ticks;
} RinTimesTms32V1;

typedef struct RinTimesTms64V1 {
    int64_t user_ticks;
    int64_t system_ticks;
    int64_t child_user_ticks;
    int64_t child_system_ticks;
} RinTimesTms64V1;

static inline int rin_times_ns_to_ticks_v1(uint64_t nanoseconds,
                                           uint64_t maximum,
                                           uint64_t* ticks_out)
{
    const uint64_t nanoseconds_per_second = UINT64_C(1000000000);
    uint64_t seconds;
    uint64_t fraction;
    uint64_t whole_ticks;
    uint64_t fractional_ticks;
    if (ticks_out == 0) return 0;
    seconds = nanoseconds / nanoseconds_per_second;
    fraction = nanoseconds % nanoseconds_per_second;
    if (seconds > maximum / RIN_TIMES_CLOCK_TICKS_PER_SECOND) return 0;
    whole_ticks = seconds * RIN_TIMES_CLOCK_TICKS_PER_SECOND;
    fractional_ticks =
        (fraction * RIN_TIMES_CLOCK_TICKS_PER_SECOND) /
        nanoseconds_per_second;
    if (whole_ticks > maximum - fractional_ticks) return 0;
    *ticks_out = whole_ticks + fractional_ticks;
    return 1;
}

#if !defined(MIDL_PASS) && defined(__cplusplus)
static_assert(sizeof(RinTimesTms32V1) == 16u,
              "RinTimesTms32V1 ABI drift");
static_assert(sizeof(RinTimesTms64V1) == 32u,
              "RinTimesTms64V1 ABI drift");
#elif !defined(MIDL_PASS)
_Static_assert(sizeof(RinTimesTms32V1) == 16u,
               "RinTimesTms32V1 ABI drift");
_Static_assert(sizeof(RinTimesTms64V1) == 32u,
               "RinTimesTms64V1 ABI drift");
#endif
#endif /* RIN_SDK_TIMES_ABI_DEFINED */

#endif
