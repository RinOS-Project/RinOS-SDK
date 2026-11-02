/* SPDX-License-Identifier: MIT */
/* Internal syscall extension for native POSIX timer output pointers. */
#ifndef RIN_SDK_TIMER_USERPTR_ABI_V1_H
#define RIN_SDK_TIMER_USERPTR_ABI_V1_H

#include <stddef.h>
#include <stdint.h>
#include "abi.h"

/* Selectors occupy bits unused by the POSIX timer flags and timer IDs. They
 * identify the caller's time_t width; the syscall entry supplies long width. */
#define RIN_TIMER_USERPTR_TIME32 UINT32_C(0x40000000)
#define RIN_TIMER_USERPTR_TIME64 UINT32_C(0x80000000)
#define RIN_TIMER_USERPTR_MODE_MASK UINT32_C(0xc0000000)
#define RIN_TIMER_CREATE_FLAG_USER_ID_OUTPUT UINT16_C(0x8000)

typedef struct RinTimerCreateUserOutputV1 {
    RinTimerCreateV1 request;
    uint64_t timer_id_user_address;
} RinTimerCreateUserOutputV1;

#if defined(__cplusplus)
static_assert(offsetof(RinTimerCreateUserOutputV1, timer_id_user_address) ==
                  sizeof(RinTimerCreateV1),
              "timer create output tail ABI drift");
#else
_Static_assert(offsetof(RinTimerCreateUserOutputV1, timer_id_user_address) ==
                   sizeof(RinTimerCreateV1),
               "timer create output tail ABI drift");
#endif

static inline uint32_t rin_timer_userptr_time_width(uint32_t mode)
{
    return mode == RIN_TIMER_USERPTR_TIME32 ? 4u :
           mode == RIN_TIMER_USERPTR_TIME64 ? 8u : 0u;
}

static inline uint32_t rin_timer_userptr_record_size(uint32_t mode,
                                                      uint32_t long_width)
{
    const uint32_t time_width = rin_timer_userptr_time_width(mode);
    const uint32_t fraction_offset =
        time_width == 4u && long_width == 8u ? 8u : time_width;
    if (time_width == 0u || (long_width != 4u && long_width != 8u))
        return 0u;
    return 2u * (fraction_offset + long_width);
}

static inline int rin_timer_userptr_write_unsigned(uint8_t* output,
                                                    uint32_t width,
                                                    uint64_t value)
{
    uint32_t index;
    if (!output || (width != 4u && width != 8u) ||
        (width == 4u && value > INT32_MAX))
        return 0;
    for (index = 0u; index < width; ++index)
        output[index] = (uint8_t)(value >> (index * 8u));
    return 1;
}

/* Native x86 time structures lay out each element as time_t then long, with
 * the usual alignment padding before an 8-byte long after a 4-byte time_t. */
static inline int rin_timer_userptr_encode_pair(
    uint8_t* output, uint32_t output_size, uint32_t mode,
    uint32_t long_width, uint64_t interval_units, uint64_t value_units,
    uint64_t units_per_second)
{
    const uint32_t time_width = rin_timer_userptr_time_width(mode);
    const uint32_t fraction_offset =
        time_width == 4u && long_width == 8u ? 8u : time_width;
    const uint32_t stride = fraction_offset + long_width;
    const uint32_t required = rin_timer_userptr_record_size(mode, long_width);
    uint64_t interval_seconds;
    uint64_t value_seconds;
    uint64_t interval_fraction;
    uint64_t value_fraction;

    if (!output || required == 0u || output_size < required ||
        units_per_second == 0u)
        return 0;
    interval_seconds = interval_units / units_per_second;
    value_seconds = value_units / units_per_second;
    interval_fraction = interval_units % units_per_second;
    value_fraction = value_units % units_per_second;
    if (!rin_timer_userptr_write_unsigned(output, time_width,
                                          interval_seconds) ||
        !rin_timer_userptr_write_unsigned(output + fraction_offset,
                                          long_width, interval_fraction) ||
        !rin_timer_userptr_write_unsigned(output + stride, time_width,
                                          value_seconds) ||
        !rin_timer_userptr_write_unsigned(output + stride + fraction_offset,
                                          long_width, value_fraction))
        return 0;
    return 1;
}

#endif /* RIN_SDK_TIMER_USERPTR_ABI_V1_H */
