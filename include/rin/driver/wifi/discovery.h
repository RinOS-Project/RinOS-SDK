/* SPDX-License-Identifier: MIT */
/* Bounded IEEE 802.11 Beacon/Probe Response admission for Wi-Fi discovery. */

#ifndef RIN_DRIVER_WIFI_DISCOVERY_H
#define RIN_DRIVER_WIFI_DISCOVERY_H

#include <stddef.h>
#include <stdint.h>

#define RIN_WIFI_DISCOVERY_VERSION UINT32_C(0x00010001)
#define RIN_WIFI_DISCOVERY_V2_VERSION UINT32_C(0x00020000)
#define RIN_WIFI_DISCOVERY_SIGNAL_V1_VERSION UINT32_C(0x00010000)
#define RIN_WIFI_DISCOVERY_SCAN_RESULT_V1_VERSION UINT32_C(0x00010000)
#define RIN_WIFI_DISCOVERY_SIGNAL_RSSI_VALID UINT32_C(0x00000001)
#define RIN_WIFI_DISCOVERY_MAX_FRAME_SIZE 2304u
#define RIN_WIFI_DISCOVERY_MAX_INFORMATION_ELEMENTS 64u
#define RIN_WIFI_DISCOVERY_MAX_SSID_SIZE 32u
#define RIN_WIFI_DISCOVERY_MAX_MLO_LINK_ID 14u

typedef enum RinWifiDiscoveryStatus {
    RIN_WIFI_DISCOVERY_OK = 0,
    RIN_WIFI_DISCOVERY_INVALID_ARGUMENT = -1,
    RIN_WIFI_DISCOVERY_MALFORMED = -2,
    RIN_WIFI_DISCOVERY_UNSUPPORTED = -3,
    RIN_WIFI_DISCOVERY_AMBIGUOUS = -4
} RinWifiDiscoveryStatus;

typedef enum RinWifiSecurityMode {
    RIN_WIFI_SECURITY_OPEN = 0,
    RIN_WIFI_SECURITY_WPA2_PSK = 1,
    RIN_WIFI_SECURITY_WPA3_SAE = 2,
    RIN_WIFI_SECURITY_WPA2_WPA3_TRANSITION = 3
} RinWifiSecurityMode;

typedef enum RinWifiBand {
    RIN_WIFI_BAND_2_4_GHZ = 1,
    RIN_WIFI_BAND_5_GHZ = 2,
    RIN_WIFI_BAND_6_GHZ = 3
} RinWifiBand;

typedef enum RinWifiChannelWidth {
    RIN_WIFI_CHANNEL_WIDTH_UNKNOWN = 0,
    RIN_WIFI_CHANNEL_WIDTH_20_MHZ = 20,
    RIN_WIFI_CHANNEL_WIDTH_40_MHZ = 40,
    RIN_WIFI_CHANNEL_WIDTH_80_MHZ = 80,
    RIN_WIFI_CHANNEL_WIDTH_160_MHZ = 160
} RinWifiChannelWidth;

typedef struct RinWifiDiscoveredNetworkV1 {
    uint32_t struct_size;
    uint32_t version;
    uint8_t bssid[6];
    uint8_t ssid_length;
    uint8_t hidden;
    uint8_t ssid[RIN_WIFI_DISCOVERY_MAX_SSID_SIZE];
    uint8_t channel;
    uint8_t security;
    uint8_t pmf_capable;
    uint8_t pmf_required;
    int8_t signal_dbm;
    uint8_t frame_subtype;
    uint16_t sequence_number;
    uint16_t beacon_interval_tu;
    uint16_t capability;
    uint32_t information_element_count;
    uint64_t timestamp_tsf;
    uint8_t band;
    uint8_t channel_width_mhz;
    uint8_t center_channel0;
    uint8_t center_channel1;
    uint8_t regulatory_info;
    uint8_t minimum_rate_mbps;
    uint8_t sae_h2e;
    uint8_t reserved0;
    uint64_t reserved[2];
} RinWifiDiscoveredNetworkV1;

/* V1 intentionally has no representable 320 MHz width or MLD identity.  V2
 * carries that evidence separately so an older association caller cannot
 * accidentally treat an EHT/MLO candidate as a legacy single-link BSS. */
typedef struct RinWifiDiscoveredNetworkV2 {
    uint32_t struct_size;
    uint32_t version;
    RinWifiDiscoveredNetworkV1 network;
    uint16_t eht_channel_width_mhz;
    uint16_t eht_disabled_subchannel_bitmap;
    uint8_t eht_present;
    uint8_t eht_center_channel0;
    uint8_t eht_center_channel1;
    uint8_t mlo_present;
    uint8_t mlo_link_id;
    uint8_t reserved0;
    uint8_t mld_address[6];
    uint8_t reserved[8];
} RinWifiDiscoveredNetworkV2;

/* Versioned RSSI metadata is separate from the stable V1 network record so
 * older callers keep their layout.  Unknown RSSI has flags == 0 and a zero
 * signal_dbm; it is never encoded as an estimated -127 dBm measurement. */
typedef struct RinWifiDiscoverySignalV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    int32_t signal_dbm;
    uint64_t reserved[2];
} RinWifiDiscoverySignalV1;

typedef struct RinWifiDiscoveryScanResultV1 {
    uint32_t struct_size;
    uint32_t version;
    RinWifiDiscoveredNetworkV1 network;
    RinWifiDiscoverySignalV1 signal;
} RinWifiDiscoveryScanResultV1;

static inline int rin_wifi_discovery_signal_v1_valid(
    const RinWifiDiscoverySignalV1* signal) {
    if (!signal || signal->struct_size != sizeof(*signal) ||
        signal->version != RIN_WIFI_DISCOVERY_SIGNAL_V1_VERSION ||
        (signal->flags & ~RIN_WIFI_DISCOVERY_SIGNAL_RSSI_VALID) != 0u ||
        signal->reserved[0] != 0u || signal->reserved[1] != 0u)
        return 0;
    if ((signal->flags & RIN_WIFI_DISCOVERY_SIGNAL_RSSI_VALID) == 0u)
        return signal->signal_dbm == 0;
    return signal->signal_dbm >= -127 && signal->signal_dbm <= 0;
}

static inline int rin_wifi_discovery_scan_result_v1_valid(
    const RinWifiDiscoveryScanResultV1* result) {
    if (!result || result->struct_size != sizeof(*result) ||
        result->version != RIN_WIFI_DISCOVERY_SCAN_RESULT_V1_VERSION ||
        !rin_wifi_discovery_signal_v1_valid(&result->signal) ||
        result->network.struct_size != sizeof(result->network) ||
        result->network.version != RIN_WIFI_DISCOVERY_VERSION ||
        result->network.ssid_length > RIN_WIFI_DISCOVERY_MAX_SSID_SIZE ||
        result->network.channel == 0u ||
        result->network.signal_dbm != result->signal.signal_dbm)
        return 0;
    return 1;
}

/* The input must be one complete 802.11 management MPDU without an FCS.  The
 * output is zeroed on every parse/admission failure.  Aliasing input and output
 * is an invalid caller contract and is rejected without modifying either. */
RinWifiDiscoveryStatus rin_wifi_discovery_admit(
    const uint8_t* frame, uint32_t frame_size, int32_t signal_dbm,
    RinWifiDiscoveredNetworkV1* network_out);
/* V2 additionally admits a validated EHT Operation element and a bounded
 * Basic Multi-Link identity.  It is discovery evidence only; it does not
 * associate, schedule links, or install keys. */
RinWifiDiscoveryStatus rin_wifi_discovery_admit_v2(
    const uint8_t* frame, uint32_t frame_size, int32_t signal_dbm,
    RinWifiDiscoveredNetworkV2* network_out);
/* Admit one management frame with explicit measured/unknown RSSI metadata.
 * The signal value must already include the backend's validated calibration. */
RinWifiDiscoveryStatus rin_wifi_discovery_admit_scan_result_v1(
    const uint8_t* frame, uint32_t frame_size,
    const RinWifiDiscoverySignalV1* signal,
    RinWifiDiscoveryScanResultV1* result_out);

#if defined(__cplusplus)
static_assert(sizeof(RinWifiDiscoveredNetworkV1) == 96u,
              "Wi-Fi discovery ABI drift");
static_assert(sizeof(RinWifiDiscoveredNetworkV2) == 128u,
              "Wi-Fi discovery V2 ABI drift");
static_assert(sizeof(RinWifiDiscoverySignalV1) == 32u,
              "Wi-Fi discovery signal ABI drift");
static_assert(sizeof(RinWifiDiscoveryScanResultV1) == 136u,
              "Wi-Fi discovery scan result ABI drift");
static_assert(offsetof(RinWifiDiscoveredNetworkV1, band) == 72u,
              "Wi-Fi discovery band offset drift");
static_assert(offsetof(RinWifiDiscoveredNetworkV1, sae_h2e) == 78u,
              "Wi-Fi discovery SAE offset drift");
static_assert(offsetof(RinWifiDiscoveredNetworkV1, reserved) == 80u,
              "Wi-Fi discovery reserved offset drift");
#else
_Static_assert(sizeof(RinWifiDiscoveredNetworkV1) == 96u,
               "Wi-Fi discovery ABI drift");
_Static_assert(sizeof(RinWifiDiscoveredNetworkV2) == 128u,
               "Wi-Fi discovery V2 ABI drift");
_Static_assert(sizeof(RinWifiDiscoverySignalV1) == 32u,
               "Wi-Fi discovery signal ABI drift");
_Static_assert(sizeof(RinWifiDiscoveryScanResultV1) == 136u,
               "Wi-Fi discovery scan result ABI drift");
_Static_assert(offsetof(RinWifiDiscoveredNetworkV1, band) == 72u,
               "Wi-Fi discovery band offset drift");
_Static_assert(offsetof(RinWifiDiscoveredNetworkV1, sae_h2e) == 78u,
               "Wi-Fi discovery SAE offset drift");
_Static_assert(offsetof(RinWifiDiscoveredNetworkV1, reserved) == 80u,
               "Wi-Fi discovery reserved offset drift");
#endif

#endif /* RIN_DRIVER_WIFI_DISCOVERY_H */
