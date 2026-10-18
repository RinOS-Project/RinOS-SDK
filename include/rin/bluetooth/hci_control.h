/* SPDX-License-Identifier: MIT */
/* Versioned, capability-gated user ABI for the admitted Bluetooth HCI owner. */
#ifndef RIN_SDK_BLUETOOTH_HCI_CONTROL_H
#define RIN_SDK_BLUETOOTH_HCI_CONTROL_H

#include <stddef.h>
#include <stdint.h>
#include <rin/syscall_abi.h>

#define RIN_BLUETOOTH_HCI_CONTROL_VERSION UINT32_C(1)
#define RIN_BLUETOOTH_HCI_CONTROL_CONNECTION_CONFIG_VERSION UINT32_C(1)
#define RIN_BLUETOOTH_HCI_CONTROL_MAX_CONTROLLERS UINT32_C(32)
#define RIN_BLUETOOTH_HCI_CONTROL_MAX_REPORTS UINT32_C(32)
#define RIN_BLUETOOTH_HCI_CONTROL_MAX_CONNECTIONS UINT32_C(4)
#define RIN_BLUETOOTH_HCI_CONTROL_MAX_REPORT_DATA UINT32_C(1650)
#define RIN_BLUETOOTH_HCI_CONTROL_MAX_REPORT_NAME UINT32_C(64)
#define RIN_BLUETOOTH_HCI_CONTROL_MAX_LEGACY_ADV_DATA UINT32_C(31)
#define RIN_BLUETOOTH_HCI_CONTROL_REQUEST_MAX_BYTES UINT32_C(2048)

/* The HCI control syscall maps RIN_RESULT_DEVICE_LOST to -ESTALE. Keep the
 * established RinOS errno value in this ABI header so portable clients can
 * handle a stale report generation without depending on host errno headers. */
#define RIN_BLUETOOTH_HCI_CONTROL_ERRNO_STALE UINT32_C(116)

typedef enum RinBluetoothHciControlOperationV1 {
    RIN_BLUETOOTH_HCI_CONTROL_LIST = 1,
    RIN_BLUETOOTH_HCI_CONTROL_START_SCAN = 2,
    RIN_BLUETOOTH_HCI_CONTROL_STOP_SCAN = 3,
    RIN_BLUETOOTH_HCI_CONTROL_START_ADVERTISING = 4,
    RIN_BLUETOOTH_HCI_CONTROL_STOP_ADVERTISING = 5,
    RIN_BLUETOOTH_HCI_CONTROL_GET_REPORT = 6,
    RIN_BLUETOOTH_HCI_CONTROL_START_CONNECTION = 7,
    RIN_BLUETOOTH_HCI_CONTROL_CANCEL_CONNECTION = 8,
    RIN_BLUETOOTH_HCI_CONTROL_DISCONNECT = 9,
    RIN_BLUETOOTH_HCI_CONTROL_GET_CONNECTION = 10
} RinBluetoothHciControlOperationV1;

#define RIN_BLUETOOTH_HCI_CONTROL_FLAG_SCANNING UINT32_C(1)
#define RIN_BLUETOOTH_HCI_CONTROL_FLAG_ADVERTISING UINT32_C(2)
#define RIN_BLUETOOTH_HCI_CONTROL_FLAG_CONNECTED UINT32_C(4)

/* LE scan intervals use 0.625 ms units, connection intervals use 1.25 ms,
 * and supervision timeout uses 10 ms. Addresses are in display order. */
typedef struct RinBluetoothHciControlConnectionConfigV1 {
    uint32_t version;
    uint32_t struct_size;
    uint16_t scan_interval;
    uint16_t scan_window;
    uint8_t peer_address_type;
    uint8_t peer_address[6];
    uint8_t own_address_type;
    uint16_t connection_interval_min;
    uint16_t connection_interval_max;
    uint16_t connection_latency;
    uint16_t supervision_timeout;
    uint16_t min_connection_event_length;
    uint16_t max_connection_event_length;
} RinBluetoothHciControlConnectionConfigV1;

typedef struct RinBluetoothHciControlConnectionInfoV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t generation;
    uint16_t handle;
    uint16_t connection_interval;
    uint16_t connection_latency;
    uint16_t supervision_timeout;
    uint8_t present;
    uint8_t role;
    uint8_t peer_address_type;
    uint8_t disconnect_pending;
    uint8_t peer_address[6];
    uint8_t reserved[2];
} RinBluetoothHciControlConnectionInfoV1;

typedef struct RinBluetoothHciControllerInfoV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    uint32_t report_count;
    uint64_t controller_id;
    uint64_t controller_generation;
} RinBluetoothHciControllerInfoV1;

/* Pointer-free copy of the bounded LE report retained by the kernel HCI
 * controller. report_generation detects replacement between list/get calls. */
typedef struct __attribute__((aligned(8))) RinBluetoothHciReportV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t report_generation;
    uint8_t present;
    uint8_t address_type;
    uint8_t event_type;
    int8_t rssi_dbm;
    uint8_t address[6];
    uint8_t advertising_flags;
    uint8_t advertising_flags_present;
    uint16_t data_size;
    uint8_t name_size;
    uint8_t report_format;
    uint8_t data_status;
    uint8_t primary_phy;
    uint8_t secondary_phy;
    uint8_t advertising_sid;
    int8_t tx_power_dbm;
    uint16_t periodic_advertising_interval;
    uint8_t direct_address_type;
    uint8_t direct_address_present;
    uint8_t direct_address[6];
    /* Keep the timestamp at offset 48 on both 32-bit and 64-bit targets. */
    uint32_t reserved0;
    uint64_t last_seen_ms;
    uint8_t data[RIN_BLUETOOTH_HCI_CONTROL_MAX_REPORT_DATA];
    char name[RIN_BLUETOOTH_HCI_CONTROL_MAX_REPORT_NAME];
    uint8_t reserved1[6];
} RinBluetoothHciReportV1;

typedef union RinBluetoothHciControlPayloadV1 {
    RinBluetoothHciControllerInfoV1 controllers[
        RIN_BLUETOOTH_HCI_CONTROL_MAX_CONTROLLERS];
    RinBluetoothHciReportV1 report;
    RinBluetoothHciControlConnectionConfigV1 connection_config;
    RinBluetoothHciControlConnectionInfoV1 connection;
} RinBluetoothHciControlPayloadV1;

/* LIST discovers only controllers that the kernel has actually enumerated.
 * All control/report operations must supply both the returned controller ID
 * and generation; stale generations are rejected. Every operation requires
 * RIN_CAP_BLUETOOTH_CONTROL. No RF, peer identity, or Apple compatibility is
 * implied by this generic HCI contract. */
typedef struct RinBluetoothHciControlRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t operation;
    uint32_t flags;
    uint64_t controller_id;
    uint64_t controller_generation;
    uint32_t report_index;
    uint32_t expected_report_generation;
    uint32_t controller_capacity;
    uint32_t controller_count;
    uint16_t scan_interval;
    uint16_t scan_window;
    uint8_t active_scan;
    uint8_t filter_duplicates;
    uint16_t advertising_interval_min;
    uint16_t advertising_interval_max;
    uint8_t advertising_type;
    uint8_t own_address_type;
    uint8_t advertising_channel_map;
    uint8_t advertising_filter_policy;
    uint8_t advertising_data_size;
    uint8_t scan_response_data_size;
    uint8_t reserved0[8];
    uint8_t advertising_data[RIN_BLUETOOTH_HCI_CONTROL_MAX_LEGACY_ADV_DATA];
    uint8_t scan_response_data[RIN_BLUETOOTH_HCI_CONTROL_MAX_LEGACY_ADV_DATA];
    union {
        /* Retained for source compatibility with earlier V1 clients. */
        uint8_t reserved1[6];
        /* Used only by DISCONNECT; zero for every other operation. */
        struct __attribute__((packed)) {
            uint16_t connection_handle;
            uint32_t connection_generation;
        } disconnect;
    };
    RinBluetoothHciControlPayloadV1 payload;
    uint64_t reserved[2];
} RinBluetoothHciControlRequestV1;

static inline int rin_bluetooth_hci_control_request_v1_zero(
    const void* value, uint32_t size)
{
    const uint8_t* bytes = (const uint8_t*)value;
    uint8_t combined = 0u;
    if (value == 0) return 0;
    while (size-- != 0u) combined |= *bytes++;
    return combined == 0u;
}

static inline int rin_bluetooth_hci_control_connection_config_v1_valid(
    const RinBluetoothHciControlConnectionConfigV1* config)
{
    uint8_t any_address = 0u;
    uint8_t all_ones = 0xffu;
    uint32_t index;
    uint64_t supervision_window;
    uint64_t required_timeout;
    if (config == 0 ||
        config->version !=
            RIN_BLUETOOTH_HCI_CONTROL_CONNECTION_CONFIG_VERSION ||
        config->struct_size != sizeof(*config) ||
        config->scan_interval < UINT16_C(4) ||
        config->scan_interval > UINT16_C(0x4000) ||
        config->scan_window < UINT16_C(4) ||
        config->scan_window > config->scan_interval ||
        config->peer_address_type > 3u || config->own_address_type > 3u ||
        config->connection_interval_min < UINT16_C(6) ||
        config->connection_interval_max > UINT16_C(0x0c80) ||
        config->connection_interval_min >
            config->connection_interval_max ||
        config->connection_latency > UINT16_C(0x01f3) ||
        config->supervision_timeout < UINT16_C(0x000a) ||
        config->supervision_timeout > UINT16_C(0x0c80) ||
        config->min_connection_event_length >
            config->max_connection_event_length)
        return 0;
    for (index = 0u; index < sizeof(config->peer_address); ++index) {
        any_address |= config->peer_address[index];
        all_ones &= config->peer_address[index];
    }
    if (any_address == 0u || all_ones == 0xffu) return 0;
    if (config->peer_address_type == 3u &&
        ((config->peer_address[0] & UINT8_C(0xc0)) != UINT8_C(0xc0) ||
         (config->peer_address[0] & UINT8_C(0x3f)) == 0u ||
         (config->peer_address[0] & UINT8_C(0x3f)) == UINT8_C(0x3f)))
        return 0;
    supervision_window = UINT64_C(2) *
        ((uint64_t)config->connection_latency + UINT64_C(1)) *
        config->connection_interval_max;
    required_timeout = (uint64_t)config->supervision_timeout * UINT64_C(8);
    return required_timeout > supervision_window;
}

static inline int rin_bluetooth_hci_control_request_v1_valid(
    const RinBluetoothHciControlRequestV1* request)
{
    uint32_t index;
    if (request == 0 || request->struct_size != sizeof(*request) ||
        request->version != RIN_BLUETOOTH_HCI_CONTROL_VERSION ||
        request->flags != 0u || request->reserved[0] != 0u ||
        request->reserved[1] != 0u ||
        !rin_bluetooth_hci_control_request_v1_zero(
            request->reserved0, sizeof(request->reserved0)) ||
        (request->operation != RIN_BLUETOOTH_HCI_CONTROL_DISCONNECT &&
         !rin_bluetooth_hci_control_request_v1_zero(
             request->reserved1, sizeof(request->reserved1))) ||
        request->disconnect.connection_handle > UINT16_C(0x0eff))
        return 0;
    if (request->operation != RIN_BLUETOOTH_HCI_CONTROL_DISCONNECT &&
        (request->disconnect.connection_handle != 0u ||
         request->disconnect.connection_generation != 0u))
        return 0;
    if (request->operation == RIN_BLUETOOTH_HCI_CONTROL_DISCONNECT &&
        request->disconnect.connection_generation == 0u)
        return 0;
    switch (request->operation) {
    case RIN_BLUETOOTH_HCI_CONTROL_LIST:
        return request->controller_id == 0u &&
               request->controller_generation == 0u &&
               request->report_index == 0u &&
               request->expected_report_generation == 0u &&
               request->controller_capacity != 0u &&
               request->controller_capacity <=
                   RIN_BLUETOOTH_HCI_CONTROL_MAX_CONTROLLERS &&
               request->controller_count == 0u &&
               request->scan_interval == 0u && request->scan_window == 0u &&
               request->active_scan == 0u &&
               request->filter_duplicates == 0u &&
               request->advertising_interval_min == 0u &&
               request->advertising_interval_max == 0u &&
               request->advertising_type == 0u &&
               request->own_address_type == 0u &&
               request->advertising_channel_map == 0u &&
               request->advertising_filter_policy == 0u &&
               request->advertising_data_size == 0u &&
               request->scan_response_data_size == 0u &&
               rin_bluetooth_hci_control_request_v1_zero(
                   request->advertising_data,
                   sizeof(request->advertising_data)) &&
               rin_bluetooth_hci_control_request_v1_zero(
                   request->scan_response_data,
                   sizeof(request->scan_response_data)) &&
               rin_bluetooth_hci_control_request_v1_zero(
                   &request->payload, sizeof(request->payload));
    case RIN_BLUETOOTH_HCI_CONTROL_START_SCAN:
        return request->controller_id != 0u &&
               request->controller_generation != 0u &&
               request->report_index == 0u &&
               request->expected_report_generation == 0u &&
               request->controller_capacity == 0u &&
               request->controller_count == 0u &&
               request->scan_interval >= 4u &&
               request->scan_interval <= UINT16_C(0x4000) &&
               request->scan_window >= 4u &&
               request->scan_window <= request->scan_interval &&
               request->active_scan <= 1u &&
               request->filter_duplicates <= 1u &&
               request->advertising_interval_min == 0u &&
               request->advertising_interval_max == 0u &&
               request->advertising_type == 0u &&
               request->own_address_type == 0u &&
               request->advertising_channel_map == 0u &&
               request->advertising_filter_policy == 0u &&
               request->advertising_data_size == 0u &&
               request->scan_response_data_size == 0u &&
               rin_bluetooth_hci_control_request_v1_zero(
                   request->advertising_data,
                   sizeof(request->advertising_data)) &&
               rin_bluetooth_hci_control_request_v1_zero(
                   request->scan_response_data,
                   sizeof(request->scan_response_data)) &&
               rin_bluetooth_hci_control_request_v1_zero(
                   &request->payload, sizeof(request->payload));
    case RIN_BLUETOOTH_HCI_CONTROL_STOP_SCAN:
    case RIN_BLUETOOTH_HCI_CONTROL_STOP_ADVERTISING:
        return request->controller_id != 0u &&
               request->controller_generation != 0u &&
               request->report_index == 0u &&
               request->expected_report_generation == 0u &&
               request->controller_capacity == 0u &&
               request->controller_count == 0u &&
               request->scan_interval == 0u && request->scan_window == 0u &&
               request->active_scan == 0u &&
               request->filter_duplicates == 0u &&
               request->advertising_interval_min == 0u &&
               request->advertising_interval_max == 0u &&
               request->advertising_type == 0u &&
               request->own_address_type == 0u &&
               request->advertising_channel_map == 0u &&
               request->advertising_filter_policy == 0u &&
               request->advertising_data_size == 0u &&
               request->scan_response_data_size == 0u &&
               rin_bluetooth_hci_control_request_v1_zero(
                   request->advertising_data,
                   sizeof(request->advertising_data)) &&
               rin_bluetooth_hci_control_request_v1_zero(
                   request->scan_response_data,
                   sizeof(request->scan_response_data)) &&
               rin_bluetooth_hci_control_request_v1_zero(
                   &request->payload, sizeof(request->payload));
    case RIN_BLUETOOTH_HCI_CONTROL_START_ADVERTISING:
        if (request->controller_id == 0u ||
            request->controller_generation == 0u ||
            request->report_index != 0u ||
            request->expected_report_generation != 0u ||
            request->controller_capacity != 0u ||
            request->controller_count != 0u || request->scan_interval != 0u ||
            request->scan_window != 0u || request->active_scan != 0u ||
            request->filter_duplicates != 0u ||
            request->advertising_interval_min < UINT16_C(0x0020) ||
            request->advertising_interval_max <
                request->advertising_interval_min ||
            request->advertising_interval_max > UINT16_C(0x4000) ||
            request->advertising_type > 3u ||
            request->own_address_type > 1u ||
            request->advertising_channel_map == 0u ||
            (request->advertising_channel_map & ~UINT8_C(7)) != 0u ||
            request->advertising_filter_policy > 3u ||
            request->advertising_data_size >
                RIN_BLUETOOTH_HCI_CONTROL_MAX_LEGACY_ADV_DATA ||
            request->scan_response_data_size >
                RIN_BLUETOOTH_HCI_CONTROL_MAX_LEGACY_ADV_DATA ||
            !rin_bluetooth_hci_control_request_v1_zero(
                &request->payload, sizeof(request->payload)))
            return 0;
        for (index = request->advertising_data_size;
             index < sizeof(request->advertising_data); ++index)
            if (request->advertising_data[index] != 0u) return 0;
        for (index = request->scan_response_data_size;
             index < sizeof(request->scan_response_data); ++index)
            if (request->scan_response_data[index] != 0u) return 0;
        return 1;
    case RIN_BLUETOOTH_HCI_CONTROL_GET_REPORT:
        return request->controller_id != 0u &&
               request->controller_generation != 0u &&
               request->report_index <
                   RIN_BLUETOOTH_HCI_CONTROL_MAX_REPORTS &&
               request->controller_capacity == 0u &&
               request->controller_count == 0u &&
               request->scan_interval == 0u && request->scan_window == 0u &&
               request->active_scan == 0u &&
               request->filter_duplicates == 0u &&
               request->advertising_interval_min == 0u &&
               request->advertising_interval_max == 0u &&
               request->advertising_type == 0u &&
               request->own_address_type == 0u &&
               request->advertising_channel_map == 0u &&
               request->advertising_filter_policy == 0u &&
               request->advertising_data_size == 0u &&
               request->scan_response_data_size == 0u &&
               rin_bluetooth_hci_control_request_v1_zero(
                   request->advertising_data,
                   sizeof(request->advertising_data)) &&
               rin_bluetooth_hci_control_request_v1_zero(
                   request->scan_response_data,
                   sizeof(request->scan_response_data)) &&
               rin_bluetooth_hci_control_request_v1_zero(
                   &request->payload, sizeof(request->payload));
    case RIN_BLUETOOTH_HCI_CONTROL_START_CONNECTION:
        return request->controller_id != 0u &&
               request->controller_generation != 0u &&
               request->report_index == 0u &&
               request->expected_report_generation == 0u &&
               request->controller_capacity == 0u &&
               request->controller_count == 0u &&
               request->scan_interval == 0u && request->scan_window == 0u &&
               request->active_scan == 0u &&
               request->filter_duplicates == 0u &&
               request->advertising_interval_min == 0u &&
               request->advertising_interval_max == 0u &&
               request->advertising_type == 0u &&
               request->own_address_type == 0u &&
               request->advertising_channel_map == 0u &&
               request->advertising_filter_policy == 0u &&
               request->advertising_data_size == 0u &&
               request->scan_response_data_size == 0u &&
               rin_bluetooth_hci_control_request_v1_zero(
                   request->advertising_data,
                   sizeof(request->advertising_data)) &&
               rin_bluetooth_hci_control_request_v1_zero(
                   request->scan_response_data,
                   sizeof(request->scan_response_data)) &&
               rin_bluetooth_hci_control_connection_config_v1_valid(
                   &request->payload.connection_config) &&
               rin_bluetooth_hci_control_request_v1_zero(
                   (const uint8_t*)&request->payload +
                       sizeof(request->payload.connection_config),
                   sizeof(request->payload) -
                       sizeof(request->payload.connection_config));
    case RIN_BLUETOOTH_HCI_CONTROL_CANCEL_CONNECTION:
        return request->controller_id != 0u &&
               request->controller_generation != 0u &&
               request->report_index == 0u &&
               request->expected_report_generation == 0u &&
               request->controller_capacity == 0u &&
               request->controller_count == 0u &&
               request->scan_interval == 0u && request->scan_window == 0u &&
               request->active_scan == 0u &&
               request->filter_duplicates == 0u &&
               request->advertising_interval_min == 0u &&
               request->advertising_interval_max == 0u &&
               request->advertising_type == 0u &&
               request->own_address_type == 0u &&
               request->advertising_channel_map == 0u &&
               request->advertising_filter_policy == 0u &&
               request->advertising_data_size == 0u &&
               request->scan_response_data_size == 0u &&
               rin_bluetooth_hci_control_request_v1_zero(
                   request->advertising_data,
                   sizeof(request->advertising_data)) &&
               rin_bluetooth_hci_control_request_v1_zero(
                   request->scan_response_data,
                   sizeof(request->scan_response_data)) &&
               rin_bluetooth_hci_control_request_v1_zero(
                   &request->payload, sizeof(request->payload));
    case RIN_BLUETOOTH_HCI_CONTROL_DISCONNECT:
        return request->controller_id != 0u &&
               request->controller_generation != 0u &&
               request->report_index == 0u &&
               request->expected_report_generation == 0u &&
               request->controller_capacity == 0u &&
               request->controller_count == 0u &&
               request->scan_interval == 0u && request->scan_window == 0u &&
               request->active_scan == 0u &&
               request->filter_duplicates == 0u &&
               request->advertising_interval_min == 0u &&
               request->advertising_interval_max == 0u &&
               request->advertising_type == 0u &&
               request->own_address_type == 0u &&
               request->advertising_channel_map == 0u &&
               request->advertising_filter_policy == 0u &&
               request->advertising_data_size == 0u &&
               request->scan_response_data_size == 0u &&
               rin_bluetooth_hci_control_request_v1_zero(
                   request->advertising_data,
                   sizeof(request->advertising_data)) &&
               rin_bluetooth_hci_control_request_v1_zero(
                   request->scan_response_data,
                   sizeof(request->scan_response_data)) &&
               rin_bluetooth_hci_control_request_v1_zero(
                   &request->payload, sizeof(request->payload));
    case RIN_BLUETOOTH_HCI_CONTROL_GET_CONNECTION:
        return request->controller_id != 0u &&
               request->controller_generation != 0u &&
               request->report_index <
                   RIN_BLUETOOTH_HCI_CONTROL_MAX_CONNECTIONS &&
               request->expected_report_generation == 0u &&
               request->controller_capacity == 0u &&
               request->controller_count == 0u &&
               request->scan_interval == 0u && request->scan_window == 0u &&
               request->active_scan == 0u &&
               request->filter_duplicates == 0u &&
               request->advertising_interval_min == 0u &&
               request->advertising_interval_max == 0u &&
               request->advertising_type == 0u &&
               request->own_address_type == 0u &&
               request->advertising_channel_map == 0u &&
               request->advertising_filter_policy == 0u &&
               request->advertising_data_size == 0u &&
               request->scan_response_data_size == 0u &&
               rin_bluetooth_hci_control_request_v1_zero(
                   request->advertising_data,
                   sizeof(request->advertising_data)) &&
               rin_bluetooth_hci_control_request_v1_zero(
                   request->scan_response_data,
                   sizeof(request->scan_response_data)) &&
               rin_bluetooth_hci_control_request_v1_zero(
                   &request->payload, sizeof(request->payload));
    default:
        return 0;
    }
}

#if defined(__cplusplus)
static_assert(sizeof(RinBluetoothHciReportV1) == 1776u,
              "Bluetooth HCI report ABI drift");
static_assert(alignof(RinBluetoothHciReportV1) == 8u,
              "Bluetooth HCI report alignment ABI drift");
static_assert(offsetof(RinBluetoothHciReportV1, last_seen_ms) == 48u,
              "Bluetooth HCI report timestamp ABI drift");
static_assert(sizeof(RinBluetoothHciControllerInfoV1) == 32u,
              "Bluetooth HCI controller info ABI drift");
static_assert(sizeof(RinBluetoothHciControlConnectionConfigV1) == 32u,
              "Bluetooth HCI connection config ABI drift");
static_assert(sizeof(RinBluetoothHciControlConnectionInfoV1) == 32u,
              "Bluetooth HCI connection info ABI drift");
static_assert(sizeof(RinBluetoothHciControlRequestV1) == 1936u,
              "Bluetooth HCI request ABI drift");
static_assert(offsetof(RinBluetoothHciControlRequestV1,
                       disconnect.connection_handle) == 136u &&
                  offsetof(RinBluetoothHciControlRequestV1,
                           disconnect.connection_generation) == 138u &&
                  offsetof(RinBluetoothHciControlRequestV1, payload) == 144u,
              "Bluetooth HCI connection-control offsets ABI drift");
static_assert(sizeof(RinBluetoothHciControlRequestV1) <=
                  RIN_BLUETOOTH_HCI_CONTROL_REQUEST_MAX_BYTES,
              "Bluetooth HCI request exceeds syscall copy bound");
#else
_Static_assert(sizeof(RinBluetoothHciReportV1) == 1776u,
               "Bluetooth HCI report ABI drift");
_Static_assert(_Alignof(RinBluetoothHciReportV1) == 8u,
               "Bluetooth HCI report alignment ABI drift");
_Static_assert(offsetof(RinBluetoothHciReportV1, last_seen_ms) == 48u,
               "Bluetooth HCI report timestamp ABI drift");
_Static_assert(sizeof(RinBluetoothHciControllerInfoV1) == 32u,
               "Bluetooth HCI controller info ABI drift");
_Static_assert(sizeof(RinBluetoothHciControlConnectionConfigV1) == 32u,
               "Bluetooth HCI connection config ABI drift");
_Static_assert(sizeof(RinBluetoothHciControlConnectionInfoV1) == 32u,
               "Bluetooth HCI connection info ABI drift");
_Static_assert(sizeof(RinBluetoothHciControlRequestV1) == 1936u,
               "Bluetooth HCI request ABI drift");
_Static_assert(offsetof(RinBluetoothHciControlRequestV1,
                        disconnect.connection_handle) == 136u &&
                   offsetof(RinBluetoothHciControlRequestV1,
                            disconnect.connection_generation) == 138u &&
                   offsetof(RinBluetoothHciControlRequestV1, payload) == 144u,
               "Bluetooth HCI connection-control offsets ABI drift");
_Static_assert(sizeof(RinBluetoothHciControlRequestV1) <=
                   RIN_BLUETOOTH_HCI_CONTROL_REQUEST_MAX_BYTES,
               "Bluetooth HCI request exceeds syscall copy bound");
#endif

#endif /* RIN_SDK_BLUETOOTH_HCI_CONTROL_H */
