/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_DISPLAY_POWER_SOURCE_ABI_H
#define RIN_SDK_DISPLAY_POWER_SOURCE_ABI_H

#include <stddef.h>
#include <stdint.h>

#include "power_abi.h"

#define RIN_POWER_STATUS_VERSION_3 3u
#define RIN_POWER_INPUT_SUMMARY_VERSION_1 1u
#define RIN_POWER_INPUT_MAX_UCSI_DEVICES 8u
#define RIN_POWER_INPUT_MAX_CONNECTORS 128u

#define RIN_POWER_INPUT_FLAG_DATA_VALID          UINT32_C(0x00000001)
#define RIN_POWER_INPUT_FLAG_SYSTEM_SOURCE_ONLINE UINT32_C(0x00000002)
#define RIN_POWER_INPUT_FLAG_MEASUREMENT_VALID   UINT32_C(0x00000004)
#define RIN_POWER_INPUT_FLAG_MULTIPLE_SYSTEM     UINT32_C(0x00000008)
#define RIN_POWER_INPUT_FLAG_MASK \
    (RIN_POWER_INPUT_FLAG_DATA_VALID | \
     RIN_POWER_INPUT_FLAG_SYSTEM_SOURCE_ONLINE | \
     RIN_POWER_INPUT_FLAG_MEASUREMENT_VALID | \
     RIN_POWER_INPUT_FLAG_MULTIPLE_SYSTEM)

#define RIN_POWER_INPUT_MODE_NONE         0u
#define RIN_POWER_INPUT_MODE_DEFAULT      1u
#define RIN_POWER_INPUT_MODE_BC           2u
#define RIN_POWER_INPUT_MODE_PD           3u
#define RIN_POWER_INPUT_MODE_TYPEC_1_5A   4u
#define RIN_POWER_INPUT_MODE_TYPEC_3_0A   5u

/* UCSI's ACPI power-supply scope and connector status summarized for the
 * system power API. Online counts include connected Sink connectors only.
 * Measurements are present only for one unambiguous system input source.
 * Charging capability is separate from the battery's actual charging state,
 * which continues to come from the battery provider. */
typedef struct RinPowerInputSummaryV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint32_t flags;
    uint32_t ucsi_device_count;
    uint32_t connector_count;
    uint32_t system_online_count;
    uint32_t device_online_count;
    uint32_t system_charging_capable_count;
    uint32_t system_pd_source_count;
    uint32_t system_power_operation_mode;
    uint32_t measured_input_power_mw;
    uint32_t measured_input_voltage_mv;
    uint32_t measured_input_current_ma;
    uint32_t reserved1;
} RinPowerInputSummaryV1;

/* V3 preserves the complete V2 prefix and appends an input-source summary.
 * base.struct_size is sizeof(RinPowerStatusV3), and base.version is 3. */
typedef struct RinPowerStatusV3 {
    RinPowerStatusV2 base;
    RinPowerInputSummaryV1 input;
    uint64_t reserved[2];
} RinPowerStatusV3;

#if defined(__cplusplus)
static_assert(sizeof(RinPowerInputSummaryV1) == 56u,
              "RinPowerInputSummaryV1 ABI drift");
static_assert(offsetof(RinPowerInputSummaryV1, flags) == 8u,
              "RinPowerInputSummaryV1.flags ABI drift");
static_assert(offsetof(RinPowerInputSummaryV1, measured_input_power_mw) == 40u,
              "RinPowerInputSummaryV1.measured_input_power_mw ABI drift");
static_assert(sizeof(RinPowerStatusV3) == 152u,
              "RinPowerStatusV3 ABI drift");
static_assert(offsetof(RinPowerStatusV3, input) == sizeof(RinPowerStatusV2),
              "RinPowerStatusV3.input ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinPowerInputSummaryV1) == 56u,
               "RinPowerInputSummaryV1 ABI drift");
_Static_assert(offsetof(RinPowerInputSummaryV1, flags) == 8u,
               "RinPowerInputSummaryV1.flags ABI drift");
_Static_assert(offsetof(RinPowerInputSummaryV1, measured_input_power_mw) == 40u,
               "RinPowerInputSummaryV1.measured_input_power_mw ABI drift");
_Static_assert(sizeof(RinPowerStatusV3) == 152u,
               "RinPowerStatusV3 ABI drift");
_Static_assert(offsetof(RinPowerStatusV3, input) == sizeof(RinPowerStatusV2),
               "RinPowerStatusV3.input ABI drift");
#endif

#if !defined(RIN_KERNEL)
static inline intptr_t rin_power_status_get_v3(RinPowerStatusV3* status)
{
    unsigned char* bytes;
    if (!status) return (intptr_t)-1;
    bytes = (unsigned char*)status;
    for (size_t index = 0u; index < sizeof(*status); ++index)
        bytes[index] = 0u;
    status->base.struct_size = sizeof(*status);
    status->base.version = RIN_POWER_STATUS_VERSION_3;
    return (intptr_t)syscall(RIN_SYS_POWER_GET_STATUS, status);
}
#endif

#endif /* RIN_SDK_DISPLAY_POWER_SOURCE_ABI_H */
