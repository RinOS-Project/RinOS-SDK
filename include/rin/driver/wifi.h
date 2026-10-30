/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_DRIVER_WIFI_H
#define RIN_DRIVER_WIFI_H

#include <rin/driver/module_api_v3.h>

#define RIN_DRIVER_V3_WIFI_INFO_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_WIFI_CHANNEL_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_WIFI_SCAN_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_WIFI_RESULT_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_WIFI_CONNECT_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_WIFI_EVENT_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_WIFI_SSID_MAX 32u
#define RIN_DRIVER_V3_WIFI_CREDENTIAL_MAX 63u
#define RIN_DRIVER_V3_WIFI_INFO_FLAG_CONNECTED UINT32_C(0x00000001)
#define RIN_DRIVER_V3_WIFI_SECURITY_OPEN UINT32_C(0)
#define RIN_DRIVER_V3_WIFI_SECURITY_WPA2_PERSONAL UINT32_C(1)
#define RIN_DRIVER_V3_WIFI_SECURITY_WPA3_SAE UINT32_C(2)
#define RIN_DRIVER_V3_WIFI_SECURITY_MASK UINT32_C(0x00000007)
#define RIN_DRIVER_V3_WIFI_CHANNEL_ALLOWED UINT32_C(0x00000001)
#define RIN_DRIVER_V3_WIFI_CHANNEL_DFS UINT32_C(0x00000002)
#define RIN_DRIVER_V3_WIFI_CHANNEL_FLAG_MASK UINT32_C(0x00000003)
#define RIN_DRIVER_V3_WIFI_SCAN_ACTIVE UINT32_C(0x00000001)
#define RIN_DRIVER_V3_WIFI_SCAN_FLAG_MASK RIN_DRIVER_V3_WIFI_SCAN_ACTIVE
#define RIN_DRIVER_V3_WIFI_CONNECT_BSSID UINT32_C(0x00000001)
#define RIN_DRIVER_V3_WIFI_CONNECT_FLAG_MASK RIN_DRIVER_V3_WIFI_CONNECT_BSSID
#define RIN_DRIVER_V3_WIFI_EVENT_SCAN_COMPLETE UINT32_C(1)
#define RIN_DRIVER_V3_WIFI_EVENT_CONNECTED UINT32_C(2)
#define RIN_DRIVER_V3_WIFI_EVENT_DISCONNECTED UINT32_C(3)
#define RIN_DRIVER_V3_WIFI_EVENT_AUTHENTICATION_FAILED UINT32_C(4)

typedef struct RinDriverV3WifiInfo {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    uint32_t security_mask;
    uint32_t channel_count;
    int32_t maximum_tx_power_mbm;
    uint64_t regulatory_generation;
    uint8_t mac[6];
    char country[2];
    uint32_t scan_result_count;
    uint32_t reserved0;
    uint64_t scan_generation;
} RinDriverV3WifiInfo;

typedef struct RinDriverV3WifiChannel {
    uint32_t struct_size;
    uint32_t version;
    uint32_t channel;
    uint32_t frequency_mhz;
    int32_t maximum_tx_power_mbm;
    uint32_t flags;
    uint64_t reserved[2];
} RinDriverV3WifiChannel;

typedef struct RinDriverV3WifiScanRequest {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    uint32_t ssid_length;
    uint8_t ssid[RIN_DRIVER_V3_WIFI_SSID_MAX];
    uint64_t deadline_ns;
    uint64_t reserved[2];
} RinDriverV3WifiScanRequest;

typedef struct RinDriverV3WifiScanResult {
    uint32_t struct_size;
    uint32_t version;
    uint8_t bssid[6];
    uint8_t ssid_length;
    uint8_t reserved0;
    uint8_t ssid[RIN_DRIVER_V3_WIFI_SSID_MAX];
    uint32_t channel;
    uint32_t frequency_mhz;
    int32_t rssi_mbm;
    uint32_t security;
    uint64_t reserved[2];
} RinDriverV3WifiScanResult;

typedef struct RinDriverV3WifiConnectRequest {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    uint32_t security;
    uint32_t ssid_length;
    uint32_t credential_length;
    uint8_t ssid[RIN_DRIVER_V3_WIFI_SSID_MAX];
    uint8_t bssid[6];
    uint16_t reserved0;
    uint64_t credential_memory_lease;
    uint64_t credential_offset;
    uint64_t deadline_ns;
    uint64_t reserved[1];
} RinDriverV3WifiConnectRequest;

typedef struct RinDriverV3WifiEvent {
    uint32_t struct_size;
    uint32_t version;
    uint32_t type;
    int32_t result;
    uint64_t device_generation;
    uint64_t sequence;
    uint8_t bssid[6];
    uint16_t reserved0;
    uint64_t reserved[2];
} RinDriverV3WifiEvent;

typedef RinDriverV3Result (*RinDriverV3WifiGetInfoFn)(
    uint64_t driver_context, RinDriverV3WifiInfo* output);
typedef RinDriverV3Result (*RinDriverV3WifiGetChannelFn)(
    uint64_t driver_context, uint32_t index,
    RinDriverV3WifiChannel* output);
typedef RinDriverV3Result (*RinDriverV3WifiScanFn)(
    uint64_t driver_context, const RinDriverV3WifiScanRequest* request);
/* The driver copies the request before returning and reads credentials only
 * through the kernel-owned memory lease named in the request. */
typedef RinDriverV3Result (*RinDriverV3WifiConnectFn)(
    uint64_t driver_context, const RinDriverV3WifiConnectRequest* request);
typedef RinDriverV3Result (*RinDriverV3WifiDisconnectFn)(
    uint64_t driver_context, uint64_t deadline_ns);
typedef RinDriverV3Result (*RinDriverV3WifiPollEventsFn)(
    uint64_t driver_context, RinDriverV3WifiEvent* events,
    uint32_t capacity, uint32_t* event_count);
typedef RinDriverV3Result (*RinDriverV3WifiGetScanResultFn)(
    uint64_t driver_context, uint64_t scan_generation, uint32_t index,
    RinDriverV3WifiScanResult* output);

typedef struct RinDriverV3WifiOps {
    uint32_t struct_size;
    uint32_t version;
    /* The standard network data path and Wi-Fi management share one published
     * interface, with identical link and frame semantics. */
    RinDriverV3NetOps network;
    RinDriverV3WifiGetInfoFn get_info;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t get_info_padding;
#endif
    RinDriverV3WifiGetChannelFn get_channel;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t get_channel_padding;
#endif
    RinDriverV3WifiScanFn scan;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t scan_padding;
#endif
    RinDriverV3WifiConnectFn connect;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t connect_padding;
#endif
    RinDriverV3WifiDisconnectFn disconnect;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t disconnect_padding;
#endif
    RinDriverV3WifiPollEventsFn poll_events;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t poll_events_padding;
#endif
    RinDriverV3WifiGetScanResultFn get_scan_result;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t get_scan_result_padding;
#endif
    uint64_t reserved[1];
} RinDriverV3WifiOps;

static inline int rin_driver_v3_wifi_info_valid(
    const RinDriverV3WifiInfo* info) {
    uint8_t mac_nonzero = 0u;
    if (!info || info->struct_size < sizeof(*info) ||
        info->version != RIN_DRIVER_V3_WIFI_INFO_VERSION ||
        (info->flags & ~RIN_DRIVER_V3_WIFI_INFO_FLAG_CONNECTED) != 0u ||
        (info->security_mask & ~RIN_DRIVER_V3_WIFI_SECURITY_MASK) != 0u ||
        info->security_mask == 0u || info->channel_count == 0u ||
        info->channel_count > 256u || info->regulatory_generation == 0u ||
        info->maximum_tx_power_mbm < -10000 ||
        info->maximum_tx_power_mbm > 6000 ||
        info->country[0] < 'A' || info->country[0] > 'Z' ||
        info->country[1] < 'A' || info->country[1] > 'Z' ||
        info->scan_result_count > 256u || info->reserved0 != 0u ||
        (info->scan_generation == 0u && info->scan_result_count != 0u))
        return 0;
    for (uint32_t index = 0u; index < sizeof(info->mac); ++index)
        mac_nonzero |= info->mac[index];
    return mac_nonzero != 0u && (info->mac[0] & 1u) == 0u;
}

static inline int rin_driver_v3_wifi_channel_valid(
    const RinDriverV3WifiChannel* channel) {
    return channel && channel->struct_size >= sizeof(*channel) &&
           channel->version == RIN_DRIVER_V3_WIFI_CHANNEL_VERSION &&
           channel->channel != 0u && channel->frequency_mhz >= 2300u &&
           channel->frequency_mhz <= 7125u &&
           channel->maximum_tx_power_mbm >= -10000 &&
           channel->maximum_tx_power_mbm <= 6000 &&
           (channel->flags & ~RIN_DRIVER_V3_WIFI_CHANNEL_FLAG_MASK) == 0u &&
           (channel->flags & RIN_DRIVER_V3_WIFI_CHANNEL_ALLOWED) != 0u &&
           channel->reserved[0] == 0u && channel->reserved[1] == 0u;
}

static inline int rin_driver_v3_wifi_scan_request_valid(
    const RinDriverV3WifiScanRequest* request) {
    if (!request || request->struct_size < sizeof(*request) ||
        request->version != RIN_DRIVER_V3_WIFI_SCAN_VERSION ||
        (request->flags & ~RIN_DRIVER_V3_WIFI_SCAN_FLAG_MASK) != 0u ||
        request->ssid_length > RIN_DRIVER_V3_WIFI_SSID_MAX ||
        request->deadline_ns == 0u || request->reserved[0] != 0u ||
        request->reserved[1] != 0u) return 0;
    for (uint32_t index = request->ssid_length;
         index < RIN_DRIVER_V3_WIFI_SSID_MAX; ++index) {
        if (request->ssid[index] != 0u) return 0;
    }
    return 1;
}

static inline int rin_driver_v3_wifi_scan_result_valid(
    const RinDriverV3WifiScanResult* result) {
    uint8_t bssid_nonzero = 0u;
    if (!result || result->struct_size < sizeof(*result) ||
        result->version != RIN_DRIVER_V3_WIFI_RESULT_VERSION ||
        result->ssid_length > RIN_DRIVER_V3_WIFI_SSID_MAX ||
        result->reserved0 != 0u || result->channel == 0u ||
        result->frequency_mhz < 2300u || result->frequency_mhz > 7125u ||
        result->rssi_mbm < -12000 || result->rssi_mbm > 0 ||
        result->security > RIN_DRIVER_V3_WIFI_SECURITY_WPA3_SAE ||
        result->reserved[0] != 0u || result->reserved[1] != 0u)
        return 0;
    for (uint32_t index = 0u; index < sizeof(result->bssid); ++index)
        bssid_nonzero |= result->bssid[index];
    if (bssid_nonzero == 0u || (result->bssid[0] & 1u) != 0u) return 0;
    for (uint32_t index = result->ssid_length;
         index < RIN_DRIVER_V3_WIFI_SSID_MAX; ++index) {
        if (result->ssid[index] != 0u) return 0;
    }
    return 1;
}

static inline int rin_driver_v3_wifi_connect_valid(
    const RinDriverV3WifiConnectRequest* request,
    const RinDriverV3WifiInfo* info) {
    uint8_t bssid_nonzero = 0u;
    if (!request || !rin_driver_v3_wifi_info_valid(info) ||
        request->struct_size < sizeof(*request) ||
        request->version != RIN_DRIVER_V3_WIFI_CONNECT_VERSION ||
        (request->flags & ~RIN_DRIVER_V3_WIFI_CONNECT_FLAG_MASK) != 0u ||
        request->security >= 32u ||
        (info->security_mask & (UINT32_C(1) << request->security)) == 0u ||
        request->ssid_length == 0u ||
        request->ssid_length > RIN_DRIVER_V3_WIFI_SSID_MAX ||
        request->credential_length > RIN_DRIVER_V3_WIFI_CREDENTIAL_MAX ||
        request->deadline_ns == 0u || request->reserved0 != 0u ||
        request->reserved[0] != 0u)
        return 0;
    for (uint32_t index = request->ssid_length;
         index < RIN_DRIVER_V3_WIFI_SSID_MAX; ++index) {
        if (request->ssid[index] != 0u) return 0;
    }
    if (request->security == RIN_DRIVER_V3_WIFI_SECURITY_OPEN) {
        if (request->credential_length != 0u ||
            request->credential_memory_lease != 0u ||
            request->credential_offset != 0u) return 0;
    } else if ((request->security == RIN_DRIVER_V3_WIFI_SECURITY_WPA2_PERSONAL &&
                request->credential_length < 8u) ||
               request->credential_length == 0u ||
               request->credential_memory_lease == 0u ||
               request->credential_offset >
                   UINT64_MAX - request->credential_length) {
        return 0;
    }
    if ((request->flags & RIN_DRIVER_V3_WIFI_CONNECT_BSSID) == 0u) {
        for (uint32_t index = 0u; index < sizeof(request->bssid); ++index) {
            if (request->bssid[index] != 0u) return 0;
        }
        return 1;
    }
    for (uint32_t index = 0u; index < sizeof(request->bssid); ++index)
        bssid_nonzero |= request->bssid[index];
    return bssid_nonzero != 0u && (request->bssid[0] & 1u) == 0u;
}

static inline int rin_driver_v3_wifi_event_valid(
    const RinDriverV3WifiEvent* event) {
    uint8_t bssid_nonzero = 0u;
    if (!event || event->struct_size < sizeof(*event) ||
        event->version != RIN_DRIVER_V3_WIFI_EVENT_VERSION ||
        event->type < RIN_DRIVER_V3_WIFI_EVENT_SCAN_COMPLETE ||
        event->type > RIN_DRIVER_V3_WIFI_EVENT_AUTHENTICATION_FAILED ||
        event->result > RIN_DRIVER_V3_OK ||
        event->result < RIN_DRIVER_V3_UNSUPPORTED ||
        event->device_generation == 0u || event->sequence == 0u ||
        event->reserved0 != 0u || event->reserved[0] != 0u ||
        event->reserved[1] != 0u) return 0;
    for (uint32_t index = 0u; index < sizeof(event->bssid); ++index)
        bssid_nonzero |= event->bssid[index];
    if (bssid_nonzero == 0u) {
        return event->type == RIN_DRIVER_V3_WIFI_EVENT_SCAN_COMPLETE ||
               event->type == RIN_DRIVER_V3_WIFI_EVENT_DISCONNECTED;
    }
    return (event->bssid[0] & 1u) == 0u;
}

static inline int rin_driver_v3_wifi_scan_result_index_valid(
    const RinDriverV3WifiInfo* info, uint64_t scan_generation,
    uint32_t index) {
    return rin_driver_v3_wifi_info_valid(info) && scan_generation != 0u &&
           scan_generation == info->scan_generation &&
           index < info->scan_result_count;
}

static inline int rin_driver_v3_wifi_ops_valid(
    const RinDriverV3WifiOps* ops) {
    if (!ops || ops->struct_size != sizeof(*ops) ||
        ops->version != RIN_DRIVER_V3_OPS_VERSION ||
        !rin_driver_v3_net_ops_valid(&ops->network) || !ops->get_info ||
        !ops->get_channel || !ops->scan || !ops->connect ||
        !ops->disconnect || !ops->poll_events || !ops->get_scan_result ||
        ops->reserved[0] != 0u)
        return 0;
#if UINTPTR_MAX == UINT32_MAX
    if (ops->get_info_padding != 0u ||
        ops->get_channel_padding != 0u || ops->scan_padding != 0u ||
        ops->connect_padding != 0u || ops->disconnect_padding != 0u ||
        ops->poll_events_padding != 0u ||
        ops->get_scan_result_padding != 0u)
        return 0;
#endif
    return 1;
}

#if defined(__cplusplus)
static_assert(sizeof(RinDriverV3WifiInfo) == 56u,
              "NDRV v3 Wi-Fi info ABI drift");
static_assert(sizeof(RinDriverV3WifiChannel) == 40u,
              "NDRV v3 Wi-Fi channel ABI drift");
static_assert(sizeof(RinDriverV3WifiScanRequest) == 72u,
              "NDRV v3 Wi-Fi scan request ABI drift");
static_assert(sizeof(RinDriverV3WifiScanResult) == 80u,
              "NDRV v3 Wi-Fi scan result ABI drift");
static_assert(sizeof(RinDriverV3WifiConnectRequest) == 96u,
              "NDRV v3 Wi-Fi connect request ABI drift");
static_assert(sizeof(RinDriverV3WifiEvent) == 56u,
              "NDRV v3 Wi-Fi event ABI drift");
static_assert(sizeof(RinDriverV3WifiOps) == 136u,
              "NDRV v3 Wi-Fi operation ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinDriverV3WifiInfo) == 56u,
               "NDRV v3 Wi-Fi info ABI drift");
_Static_assert(sizeof(RinDriverV3WifiChannel) == 40u,
               "NDRV v3 Wi-Fi channel ABI drift");
_Static_assert(sizeof(RinDriverV3WifiScanRequest) == 72u,
               "NDRV v3 Wi-Fi scan request ABI drift");
_Static_assert(sizeof(RinDriverV3WifiScanResult) == 80u,
               "NDRV v3 Wi-Fi scan result ABI drift");
_Static_assert(sizeof(RinDriverV3WifiConnectRequest) == 96u,
               "NDRV v3 Wi-Fi connect request ABI drift");
_Static_assert(sizeof(RinDriverV3WifiEvent) == 56u,
               "NDRV v3 Wi-Fi event ABI drift");
_Static_assert(sizeof(RinDriverV3WifiOps) == 136u,
               "NDRV v3 Wi-Fi operation ABI drift");
#endif

#endif /* RIN_DRIVER_WIFI_H */
