/* SPDX-License-Identifier: MIT */
/* Checked-copy clock and timeval APIs for caller-width time records. */
#ifndef RIN_SDK_TIME_USER_TIMESPEC_ABI_H
#define RIN_SDK_TIME_USER_TIMESPEC_ABI_H

#include <stddef.h>
#include <stdint.h>
#include "abi.h"
#include <rin/nanosleep_abi.h>

#define RIN_CLOCK_TIMESPEC_USER_ABI_VERSION 1u
#define RIN_CLOCK_TIMESPEC_USER_GETTIME 1u
#define RIN_CLOCK_TIMESPEC_USER_GETRES 2u
#define RIN_CLOCK_TIMESPEC_USER_SETTIME 3u
#define RIN_CLOCK_TIMESPEC_USER_GETTIMEOFDAY 4u

typedef struct RinClockTimespecUserV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    int32_t clock_id;
    uint32_t time_width;
    uint32_t long_width;
    uint32_t reserved0;
    uint64_t timespec_address;
    uint64_t reserved[2];
} RinClockTimespecUserV1;

#if defined(__cplusplus)
static_assert(sizeof(RinClockTimespecUserV1) == 48u,
              "RinClockTimespecUserV1 ABI drift");
static_assert(offsetof(RinClockTimespecUserV1, timespec_address) == 24u,
              "RinClockTimespecUserV1.timespec_address ABI drift");
#else
_Static_assert(sizeof(RinClockTimespecUserV1) == 48u,
               "RinClockTimespecUserV1 ABI drift");
_Static_assert(offsetof(RinClockTimespecUserV1, timespec_address) == 24u,
               "RinClockTimespecUserV1.timespec_address ABI drift");
#endif

static inline int rin_clock_timespec_user_v1_valid(
    const RinClockTimespecUserV1* request)
{
    return request != 0 &&
           request->struct_size == sizeof(*request) &&
           request->version == RIN_CLOCK_TIMESPEC_USER_ABI_VERSION &&
           request->operation >= RIN_CLOCK_TIMESPEC_USER_GETTIME &&
           request->operation <= RIN_CLOCK_TIMESPEC_USER_GETTIMEOFDAY &&
           request->clock_id >= RIN_CLOCK_ID_REALTIME &&
           request->clock_id <= RIN_CLOCK_ID_THREAD_CPUTIME &&
           rin_nanosleep_timespec_user_size(
               request->time_width, request->long_width) != 0u &&
           request->reserved0 == 0u && request->reserved[0] == 0u &&
           request->reserved[1] == 0u &&
           (request->timespec_address != 0u ||
            request->operation == RIN_CLOCK_TIMESPEC_USER_GETRES);
}

static inline int rin_clock_timespec_user_encode_signed(
    uint8_t* bytes, uint32_t width, int64_t value)
{
    uint64_t encoded;
    uint32_t index;
    if (bytes == NULL || (width != 4u && width != 8u) ||
        (width == 4u && (value < INT32_MIN || value > INT32_MAX)))
        return 0;
    encoded = (uint64_t)value;
    for (index = 0u; index < width; ++index)
        bytes[index] = (uint8_t)(encoded >> (index * 8u));
    return 1;
}

static inline int rin_clock_timespec_user_encode_value(
    uint8_t* bytes, uint32_t time_width, uint32_t long_width,
    int64_t seconds, int64_t nanoseconds)
{
    const uint32_t nsec_offset = time_width == 4u && long_width == 8u
        ? 8u : time_width;
    if (rin_nanosleep_timespec_user_size(time_width, long_width) == 0u ||
        !rin_clock_timespec_user_encode_signed(
            bytes, time_width, seconds) ||
        !rin_clock_timespec_user_encode_signed(
            bytes + nsec_offset, long_width, nanoseconds))
        return 0;
    return 1;
}

#endif /* RIN_SDK_TIME_USER_TIMESPEC_ABI_H */
