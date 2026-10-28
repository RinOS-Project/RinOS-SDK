/* SPDX-License-Identifier: MIT */
/* Exact API-neutral timestamp period shared by GPU physical adapters. */

#ifndef RIN_DRIVER_GPU_TIMESTAMP_PERIOD_V2_H
#define RIN_DRIVER_GPU_TIMESTAMP_PERIOD_V2_H

#include <stdint.h>

#define RIN_GPU_PHYSICAL_TIMESTAMP_PERIOD_V2_VERSION UINT32_C(0x00020000)

/* A zero fractional denominator denotes an integral period. The clock-domain
 * cookie is supplied by the admitted hardware owner, never inferred from an
 * address or another device identity. */
typedef struct RinGpuPhysicalTimestampPeriodV2 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t device_generation;
    uint64_t clock_domain_cookie;
    uint64_t whole_nanoseconds;
    uint64_t fractional_numerator;
    uint64_t fractional_denominator;
    uint64_t reserved[2];
} RinGpuPhysicalTimestampPeriodV2;

#if defined(__cplusplus)
static_assert(sizeof(RinGpuPhysicalTimestampPeriodV2) == 64u,
              "RinGpuPhysicalTimestampPeriodV2 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinGpuPhysicalTimestampPeriodV2) == 64u,
               "RinGpuPhysicalTimestampPeriodV2 ABI drift");
#endif

#endif /* RIN_DRIVER_GPU_TIMESTAMP_PERIOD_V2_H */
