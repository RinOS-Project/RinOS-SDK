/* SPDX-License-Identifier: MIT */
/* Versioned, capability-gated user ABI for the admitted Bluetooth HCI owner. */
#ifndef RIN_SDK_BLUETOOTH_HCI_CONTROL_H
#define RIN_SDK_BLUETOOTH_HCI_CONTROL_H

#include <stdint.h>
#include <rin/syscall_abi.h>

#define RIN_BLUETOOTH_HCI_CONTROL_VERSION UINT32_C(1)
#define RIN_BLUETOOTH_HCI_CONTROL_MAX_CONTROLLERS UINT32_C(32)
#define RIN_BLUETOOTH_HCI_CONTROL_MAX_REPORTS UINT32_C(32)
#define RIN_BLUETOOTH_HCI_CONTROL_MAX_REPORT_DATA UINT32_C(1650)
#define RIN_BLUETOOTH_HCI_CONTROL_MAX_REPORT_NAME UINT32_C(64)
#define RIN_BLUETOOTH_HCI_CONTROL_MAX_LEGACY_ADV_DATA UINT32_C(31)
#define RIN_BLUETOOTH_HCI_CONTROL_REQUEST_MAX_BYTES UINT32_C(2048)

typedef enum RinBluetoothHciControlOperationV1 {
    RIN_BLUETOOTH_HCI_CONTROL_LIST = 1,
    RIN_BLUETOOTH_HCI_CONTROL_START_SCAN = 2,
    RIN_BLUETOOTH_HCI_CONTROL_STOP_SCAN = 3,
    RIN_BLUETOOTH_HCI_CONTROL_START_ADVERTISING = 4,
    RIN_BLUETOOTH_HCI_CONTROL_STOP_ADVERTISING = 5,
    RIN_BLUETOOTH_HCI_CONTROL_GET_REPORT = 6
} RinBluetoothHciControlOperationV1;

#define RIN_BLUETOOTH_HCI_CONTROL_FLAG_SCANNING UINT32_C(1)
#define RIN_BLUETOOTH_HCI_CONTROL_FLAG_ADVERTISING UINT32_C(2)

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
typedef struct RinBluetoothHciReportV1 {
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
    uint64_t last_seen_ms;
    uint8_t data[RIN_BLUETOOTH_HCI_CONTROL_MAX_REPORT_DATA];
    char name[RIN_BLUETOOTH_HCI_CONTROL_MAX_REPORT_NAME];
} RinBluetoothHciReportV1;

typedef union RinBluetoothHciControlPayloadV1 {
    RinBluetoothHciControllerInfoV1 controllers[
        RIN_BLUETOOTH_HCI_CONTROL_MAX_CONTROLLERS];
    RinBluetoothHciReportV1 report;
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
    uint8_t reserved1[6];
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
        !rin_bluetooth_hci_control_request_v1_zero(
            request->reserved1, sizeof(request->reserved1)))
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
    default:
        return 0;
    }
}

#if defined(__cplusplus)
static_assert(sizeof(RinBluetoothHciReportV1) == 1776u,
              "Bluetooth HCI report ABI drift");
static_assert(sizeof(RinBluetoothHciControllerInfoV1) == 32u,
              "Bluetooth HCI controller info ABI drift");
static_assert(sizeof(RinBluetoothHciControlRequestV1) <=
                  RIN_BLUETOOTH_HCI_CONTROL_REQUEST_MAX_BYTES,
              "Bluetooth HCI request exceeds syscall copy bound");
#else
_Static_assert(sizeof(RinBluetoothHciReportV1) == 1776u,
               "Bluetooth HCI report ABI drift");
_Static_assert(sizeof(RinBluetoothHciControllerInfoV1) == 32u,
               "Bluetooth HCI controller info ABI drift");
_Static_assert(sizeof(RinBluetoothHciControlRequestV1) <=
                   RIN_BLUETOOTH_HCI_CONTROL_REQUEST_MAX_BYTES,
               "Bluetooth HCI request exceeds syscall copy bound");
#endif

#endif /* RIN_SDK_BLUETOOTH_HCI_CONTROL_H */
