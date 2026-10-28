/* SPDX-License-Identifier: MIT */
/* Versioned Wi-Fi regulatory data and service contract. */

#ifndef RIN_DRIVER_WIFI_REGULATORY_V1_H
#define RIN_DRIVER_WIFI_REGULATORY_V1_H

#include <stddef.h>
#include <stdint.h>

#include <rin/driver/wifi/discovery.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_WIFI_REGULATORY_VERSION UINT32_C(0x00010001)
#define RIN_WIFI_REGULATORY_SERVICE_V1_VERSION UINT32_C(0x00010000)
#define RIN_WIFI_REGULATORY_MAX_RULES 32u
#define RIN_WIFI_REGULATORY_MAX_AFC_CHANNELS 16u
#define RIN_WIFI_REGULATORY_MIN_DFS_DURATION_MS 1000u
#define RIN_WIFI_REGULATORY_MAX_DFS_DURATION_MS 86400000u
#define RIN_WIFI_REGULATORY_MIN_AFC_LIFETIME_S 60u
#define RIN_WIFI_REGULATORY_MAX_AFC_LIFETIME_S 172800u

typedef enum RinWifiRegulatoryStatus {
    RIN_WIFI_REGULATORY_OK = 0,
    RIN_WIFI_REGULATORY_INVALID_ARGUMENT = -1,
    RIN_WIFI_REGULATORY_MALFORMED_PROFILE = -2,
    RIN_WIFI_REGULATORY_NOT_PERMITTED = -3,
    RIN_WIFI_REGULATORY_CAC_REQUIRED = -4,
    RIN_WIFI_REGULATORY_CAC_INCOMPLETE = -5,
    RIN_WIFI_REGULATORY_NON_OCCUPANCY = -6,
    RIN_WIFI_REGULATORY_AFC_REQUIRED = -7,
    RIN_WIFI_REGULATORY_EXPIRED = -8,
    RIN_WIFI_REGULATORY_CLOCK_ROLLBACK = -9,
    RIN_WIFI_REGULATORY_STALE_GENERATION = -10,
    RIN_WIFI_REGULATORY_INVALID_STATE = -11
} RinWifiRegulatoryStatus;

typedef enum RinWifiRegulatoryRuleFlag {
    RIN_WIFI_REGULATORY_RULE_NO_ACTIVE_SCAN = 1u << 0,
    RIN_WIFI_REGULATORY_RULE_DFS_REQUIRED = 1u << 1,
    RIN_WIFI_REGULATORY_RULE_AFC_REQUIRED = 1u << 2,
    RIN_WIFI_REGULATORY_RULE_INDOOR_ONLY = 1u << 3
} RinWifiRegulatoryRuleFlag;

typedef enum RinWifiRegulatoryOperation {
    RIN_WIFI_REGULATORY_PASSIVE_SCAN = 0,
    RIN_WIFI_REGULATORY_ACTIVE_SCAN = 1,
    RIN_WIFI_REGULATORY_INITIATE_RADIATION = 2
} RinWifiRegulatoryOperation;

typedef enum RinWifiDfsStateKind {
    RIN_WIFI_DFS_UNAVAILABLE = 0,
    RIN_WIFI_DFS_CAC_IN_PROGRESS = 1,
    RIN_WIFI_DFS_AVAILABLE = 2,
    RIN_WIFI_DFS_NON_OCCUPANCY = 3
} RinWifiDfsStateKind;

typedef struct RinWifiRegulatoryRuleV1 {
    uint8_t band;
    uint8_t channel;
    uint16_t maximum_width_mhz;
    int16_t maximum_eirp_dbm_x100;
    uint16_t flags;
    uint64_t reserved;
} RinWifiRegulatoryRuleV1;

typedef struct RinWifiCountryProfileV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t generation;
    uint8_t country_code[2];
    uint16_t rule_count;
    uint32_t dfs_cac_duration_ms;
    uint32_t dfs_non_occupancy_duration_ms;
    uint32_t afc_maximum_lifetime_s;
    uint32_t reserved0;
    uint32_t reserved1;
    RinWifiRegulatoryRuleV1 rules[RIN_WIFI_REGULATORY_MAX_RULES];
} RinWifiCountryProfileV1;

typedef struct RinWifiAfcChannelV1 {
    uint8_t band;
    uint8_t channel;
    uint16_t maximum_width_mhz;
    int16_t maximum_eirp_dbm_x100;
    uint16_t reserved0;
    uint64_t reserved1;
} RinWifiAfcChannelV1;

typedef struct RinWifiAfcGrantV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t profile_generation;
    uint64_t grant_generation;
    uint64_t issued_unix_s;
    uint64_t expires_unix_s;
    uint8_t country_code[2];
    uint16_t channel_count;
    uint32_t reserved0;
    uint8_t grant_digest[32];
    RinWifiAfcChannelV1 channels[RIN_WIFI_REGULATORY_MAX_AFC_CHANNELS];
} RinWifiAfcGrantV1;

typedef struct RinWifiDfsChannelStateV1 {
    uint8_t state;
    uint8_t band;
    uint8_t channel;
    uint8_t reserved0;
    uint16_t width_mhz;
    uint16_t reserved1;
    uint64_t profile_generation;
    uint64_t started_monotonic_ms;
    uint64_t deadline_monotonic_ms;
} RinWifiDfsChannelStateV1;

typedef struct RinWifiRegulatoryDecisionV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t profile_generation;
    uint64_t afc_grant_generation;
    uint64_t remaining_constraint_ms;
    int16_t maximum_eirp_dbm_x100;
    uint16_t rule_flags;
    uint16_t maximum_width_mhz;
    uint8_t band;
    uint8_t channel;
    uint32_t reserved0;
    uint32_t reserved1;
} RinWifiRegulatoryDecisionV1;

typedef int (*RinWifiRegulatoryGetProfileGenerationFnV1)(
    void* context, uint64_t* generation_out);
typedef int (*RinWifiRegulatoryAuthorizeFnV1)(
    void* context, uint8_t band, uint8_t channel, uint16_t width_mhz,
    uint8_t operation, uint8_t is_indoor,
    int16_t requested_eirp_dbm_x100, uint64_t now_monotonic_ms,
    uint64_t trusted_unix_s, RinWifiRegulatoryDecisionV1* decision_out);
typedef int (*RinWifiRegulatoryDfsOperationFnV1)(
    void* context, uint8_t band, uint8_t channel, uint16_t width_mhz,
    uint64_t now_monotonic_ms);
typedef int (*RinWifiRegulatoryInstallAfcGrantFnV1)(
    void* context, const RinWifiAfcGrantV1* grant,
    uint64_t now_monotonic_ms, uint64_t trusted_unix_s);

/* The OS owns policy state and supplies this capability table to a driver.
 * Drivers can request bounded policy operations but cannot inspect or mutate
 * the OS-owned country, DFS, or AFC controller state. The table and its
 * context must remain valid for the device generation. */
typedef struct RinWifiRegulatoryServiceV1 {
    uint32_t struct_size;
    uint32_t version;
    void* context;
    RinWifiRegulatoryGetProfileGenerationFnV1 get_profile_generation;
    RinWifiRegulatoryAuthorizeFnV1 authorize;
    RinWifiRegulatoryDfsOperationFnV1 begin_cac;
    RinWifiRegulatoryDfsOperationFnV1 complete_cac;
    RinWifiRegulatoryDfsOperationFnV1 radar_detected;
    RinWifiRegulatoryInstallAfcGrantFnV1 install_afc_grant;
    uint64_t reserved[2];
} RinWifiRegulatoryServiceV1;

#if defined(__cplusplus)
#define RIN_WIFI_REGULATORY_STATIC_ASSERT static_assert
#else
#define RIN_WIFI_REGULATORY_STATIC_ASSERT _Static_assert
#endif

RIN_WIFI_REGULATORY_STATIC_ASSERT(sizeof(RinWifiRegulatoryRuleV1) == 16u,
                                  "Wi-Fi regulatory rule ABI drift");
RIN_WIFI_REGULATORY_STATIC_ASSERT(sizeof(RinWifiCountryProfileV1) == 552u,
                                  "Wi-Fi country profile ABI drift");
RIN_WIFI_REGULATORY_STATIC_ASSERT(
    offsetof(RinWifiCountryProfileV1, rules) == 40u,
    "Wi-Fi country rule offset drift");
RIN_WIFI_REGULATORY_STATIC_ASSERT(sizeof(RinWifiAfcChannelV1) == 16u,
                                  "Wi-Fi AFC channel ABI drift");
RIN_WIFI_REGULATORY_STATIC_ASSERT(sizeof(RinWifiAfcGrantV1) == 336u,
                                  "Wi-Fi AFC grant ABI drift");
RIN_WIFI_REGULATORY_STATIC_ASSERT(offsetof(RinWifiAfcGrantV1, channels) == 80u,
                                  "Wi-Fi AFC channel offset drift");
RIN_WIFI_REGULATORY_STATIC_ASSERT(sizeof(RinWifiDfsChannelStateV1) == 32u,
                                  "Wi-Fi DFS state ABI drift");
RIN_WIFI_REGULATORY_STATIC_ASSERT(sizeof(RinWifiRegulatoryDecisionV1) == 48u,
                                  "Wi-Fi regulatory decision ABI drift");

#undef RIN_WIFI_REGULATORY_STATIC_ASSERT

#ifdef __cplusplus
}
#endif

#endif /* RIN_DRIVER_WIFI_REGULATORY_V1_H */
