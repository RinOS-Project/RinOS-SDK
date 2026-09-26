/* SPDX-License-Identifier: MIT */
#ifndef RIN_API_RIN_LEGACY_DRIVER_V1_CLOCK_ABI_H
#define RIN_API_RIN_LEGACY_DRIVER_V1_CLOCK_ABI_H

#include <stdint.h>

/* Canonical legacy SDK contract; retained only for the legacy namespace. */

#define RIN_LEGACY_DRIVER_V1_CLOCK_MAGIC UINT32_C(0x314B4C43) /* CLK1 */
#define RIN_LEGACY_DRIVER_V1_CLOCK_VERSION UINT16_C(1)

/*
 * The v1 driver is an i386 image and cannot atomically load one uint64_t on
 * every supported toolchain.  The kernel therefore publishes milliseconds
 * as two words protected by an odd/even 32-bit sequence.
 */
typedef struct RinLegacyDriverV1ClockPage {
    uint32_t magic;
    uint16_t version;
    uint16_t struct_size;
    volatile uint32_t sequence;
    volatile uint32_t ticks_low;
    volatile uint32_t ticks_high;
    uint32_t reserved;
} RinLegacyDriverV1ClockPage;

#if defined(__cplusplus)
static_assert(sizeof(RinLegacyDriverV1ClockPage) == 24u,
              "legacy v1 clock page ABI drift");
static_assert(__builtin_offsetof(RinLegacyDriverV1ClockPage, sequence) == 8u &&
                  __builtin_offsetof(RinLegacyDriverV1ClockPage, ticks_low) == 12u &&
                  __builtin_offsetof(RinLegacyDriverV1ClockPage, ticks_high) == 16u &&
                  __builtin_offsetof(RinLegacyDriverV1ClockPage, reserved) == 20u,
              "legacy v1 clock page field offset drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinLegacyDriverV1ClockPage) == 24u,
               "legacy v1 clock page ABI drift");
_Static_assert(__builtin_offsetof(RinLegacyDriverV1ClockPage, sequence) == 8u &&
                   __builtin_offsetof(RinLegacyDriverV1ClockPage, ticks_low) == 12u &&
                   __builtin_offsetof(RinLegacyDriverV1ClockPage, ticks_high) == 16u &&
                   __builtin_offsetof(RinLegacyDriverV1ClockPage, reserved) == 20u,
               "legacy v1 clock page field offset drift");
#endif

#endif /* RIN_API_RIN_LEGACY_DRIVER_V1_CLOCK_ABI_H */
