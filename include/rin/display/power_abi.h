// SPDX-License-Identifier: MIT
#ifndef RIN_SDK_DISPLAY_POWER_ABI_H
#define RIN_SDK_DISPLAY_POWER_ABI_H

#include <stddef.h>
#include <stdint.h>
#include "../syscall_abi.h"

#ifndef RIN_DISPLAY_CONFIG_DEFINED
#define RIN_DISPLAY_CONFIG_DEFINED 1
typedef struct RinDisplayConfig {
    int display_count;
    int primary_display;
    int32_t virtual_x;
    int32_t virtual_y;
    int virtual_width;
    int virtual_height;
} RinDisplayConfig;
#endif

#ifndef RIN_POWER_PROFILE_DEFINED
#define RIN_POWER_PROFILE_DEFINED 1
typedef enum RinPowerProfile {
    POWER_PROFILE_BALANCED = 0,
    POWER_PROFILE_PERFORMANCE,
    POWER_PROFILE_SAVER,
    POWER_PROFILE_CUSTOM
} RinPowerProfile;
#endif

#ifndef RIN_POWER_SETTINGS_DEFINED
#define RIN_POWER_SETTINGS_DEFINED 1
typedef struct RinPowerSettings {
    RinPowerProfile profile;
    int display_off_ac_minutes;
    int display_off_battery_minutes;
    int display_brightness_ac;
    int display_brightness_battery;
    int sleep_ac_minutes;
    int sleep_battery_minutes;
    int hibernate_ac_minutes;
    int hibernate_battery_minutes;
    int cpu_min_percent;
    int cpu_max_percent;
    int lid_close_action;
    int power_button_action;
    int sleep_button_action;
    int low_battery_action;
    int critical_battery_percent;
    int low_battery_percent;
} RinPowerSettings;
#endif

#define RIN_POWER_STATUS_VERSION_1 1u
#define RIN_POWER_STATUS_VERSION_2 2u
/* Source compatibility for callers of the original V1 entry point. */
#define RIN_POWER_STATUS_VERSION RIN_POWER_STATUS_VERSION_1
#define RIN_POWER_STATUS_FLAG_BACKEND_READY      0x00000001u
#define RIN_POWER_STATUS_FLAG_AC_CONNECTED       0x00000002u
#define RIN_POWER_STATUS_FLAG_BATTERY_PRESENT    0x00000004u
#define RIN_POWER_STATUS_FLAG_BATTERY_CHARGING   0x00000008u
#define RIN_POWER_STATUS_FLAG_THERMAL_VALID       0x00000010u
#define RIN_POWER_STATUS_FLAG_SUSPEND_SUPPORTED   0x00000020u
#define RIN_POWER_STATUS_FLAG_SHUTDOWN_SUPPORTED  0x00000040u
#define RIN_POWER_STATUS_FLAG_REBOOT_SUPPORTED    0x00000080u
#define RIN_POWER_STATUS_FLAG_LID_VALID            0x00000100u
#define RIN_POWER_STATUS_FLAG_LID_CLOSED           0x00000200u
#define RIN_POWER_STATUS_FLAG_BRIGHTNESS_VALID     0x00000400u

#define RIN_POWER_ACTION_SHUTDOWN 1u
#define RIN_POWER_ACTION_REBOOT   2u
#define RIN_POWER_ACTION_SUSPEND  3u

typedef struct RinPowerStatusV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    /* -1 is a present-but-unknown gauge value; it must not be coerced to 0%. */
    int32_t battery_percent;
    int32_t minutes_remaining;
    int32_t full_capacity_mwh;
    int32_t current_capacity_mwh;
    int32_t voltage_mv;
    int32_t current_ma;
    int32_t temperature_millic;
    int32_t health_percent;
    char battery_model[20];
} RinPowerStatusV1;

/*
 * V2 deliberately repeats the complete V1 prefix.  Existing V1 callers keep
 * receiving exactly 64 bytes; V2 callers opt in by placing this structure's
 * size and version in the first eight bytes before the syscall.
 */
typedef struct RinPowerStatusV2 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    /* -1 is a present-but-unknown gauge value; it must not be coerced to 0%. */
    int32_t battery_percent;
    int32_t minutes_remaining;
    int32_t full_capacity_mwh;
    int32_t current_capacity_mwh;
    int32_t voltage_mv;
    int32_t current_ma;
    int32_t temperature_millic;
    int32_t health_percent;
    char battery_model[20];
    int32_t brightness_percent;
    uint32_t reserved0;
    uint64_t sample_sequence;
} RinPowerStatusV2;

/* Ordered kernel power events.  A zero cursor means "start at the current
 * tail" so a newly started client cannot replay stale boot events.  On a
 * successful poll, cursor is advanced to sequence and event is one of the
 * POWER_EVENT_* values from rin.h. */
#define RIN_POWER_EVENT_POLL_VERSION 1u
#define RIN_POWER_EVENT_POLL_FLAG_OVERFLOW 0x00000001u
/* Values mirror RinPowerEvent without exposing kernel-only callback types. */
#define RIN_POWER_EVENT_AC_CONNECTED 1u
#define RIN_POWER_EVENT_AC_DISCONNECTED 2u
#define RIN_POWER_EVENT_SLEEP_REQUEST 9u
#define RIN_POWER_EVENT_WAKE_REQUEST 10u
#define RIN_POWER_EVENT_BATTERY_CONNECTED 12u
#define RIN_POWER_EVENT_BATTERY_DISCONNECTED 13u
#define RIN_POWER_EVENT_DOCK_CONNECTED 14u
#define RIN_POWER_EVENT_DOCK_DISCONNECTED 15u
typedef struct RinPowerEventPollV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    uint32_t event;
    uint64_t sequence;
    uint64_t cursor;
} RinPowerEventPollV1;

#if defined(__cplusplus)
static_assert(sizeof(RinPowerStatusV1) == 64,
              "RinPowerStatusV1 ABI size changed");
static_assert(sizeof(RinPowerStatusV2) == 80,
              "RinPowerStatusV2 ABI size changed");
static_assert(sizeof(RinPowerEventPollV1) == 32,
              "RinPowerEventPollV1 ABI size changed");
#define RIN_POWER_STATUS_PREFIX_ASSERT(field) \
    static_assert(offsetof(RinPowerStatusV2, field) == \
                      offsetof(RinPowerStatusV1, field), \
                  "RinPowerStatusV2 prefix mismatch: " #field)
RIN_POWER_STATUS_PREFIX_ASSERT(struct_size);
RIN_POWER_STATUS_PREFIX_ASSERT(version);
RIN_POWER_STATUS_PREFIX_ASSERT(flags);
RIN_POWER_STATUS_PREFIX_ASSERT(battery_percent);
RIN_POWER_STATUS_PREFIX_ASSERT(minutes_remaining);
RIN_POWER_STATUS_PREFIX_ASSERT(full_capacity_mwh);
RIN_POWER_STATUS_PREFIX_ASSERT(current_capacity_mwh);
RIN_POWER_STATUS_PREFIX_ASSERT(voltage_mv);
RIN_POWER_STATUS_PREFIX_ASSERT(current_ma);
RIN_POWER_STATUS_PREFIX_ASSERT(temperature_millic);
RIN_POWER_STATUS_PREFIX_ASSERT(health_percent);
RIN_POWER_STATUS_PREFIX_ASSERT(battery_model);
#undef RIN_POWER_STATUS_PREFIX_ASSERT
static_assert(offsetof(RinPowerStatusV2, brightness_percent) ==
                  sizeof(RinPowerStatusV1),
              "RinPowerStatusV2 no longer has the V1 prefix");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinPowerStatusV1) == 64,
               "RinPowerStatusV1 ABI size changed");
_Static_assert(sizeof(RinPowerStatusV2) == 80,
               "RinPowerStatusV2 ABI size changed");
_Static_assert(sizeof(RinPowerEventPollV1) == 32,
               "RinPowerEventPollV1 ABI size changed");
#define RIN_POWER_STATUS_PREFIX_ASSERT(field) \
    _Static_assert(offsetof(RinPowerStatusV2, field) == \
                       offsetof(RinPowerStatusV1, field), \
                   "RinPowerStatusV2 prefix mismatch: " #field)
RIN_POWER_STATUS_PREFIX_ASSERT(struct_size);
RIN_POWER_STATUS_PREFIX_ASSERT(version);
RIN_POWER_STATUS_PREFIX_ASSERT(flags);
RIN_POWER_STATUS_PREFIX_ASSERT(battery_percent);
RIN_POWER_STATUS_PREFIX_ASSERT(minutes_remaining);
RIN_POWER_STATUS_PREFIX_ASSERT(full_capacity_mwh);
RIN_POWER_STATUS_PREFIX_ASSERT(current_capacity_mwh);
RIN_POWER_STATUS_PREFIX_ASSERT(voltage_mv);
RIN_POWER_STATUS_PREFIX_ASSERT(current_ma);
RIN_POWER_STATUS_PREFIX_ASSERT(temperature_millic);
RIN_POWER_STATUS_PREFIX_ASSERT(health_percent);
RIN_POWER_STATUS_PREFIX_ASSERT(battery_model);
#undef RIN_POWER_STATUS_PREFIX_ASSERT
_Static_assert(offsetof(RinPowerStatusV2, brightness_percent) ==
                   sizeof(RinPowerStatusV1),
               "RinPowerStatusV2 no longer has the V1 prefix");
#endif

#endif
