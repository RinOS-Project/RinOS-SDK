/* SPDX-License-Identifier: MIT */

#ifndef RIN_DRIVER_BLUETOOTH_HCI_RUNTIME_V1_H
#define RIN_DRIVER_BLUETOOTH_HCI_RUNTIME_V1_H

#include <stdint.h>

#include <rin/driver/bluetooth/hci_transport_v1.h>

#define RIN_BLUETOOTH_HCI_VERSION 1u
#define RIN_BLUETOOTH_HCI_CONTROLLER_LAYOUT_VERSION 4u
#define RIN_BLUETOOTH_HCI_CONNECTION_LAYOUT_VERSION 2u
#define RIN_BLUETOOTH_HCI_ADVERTISING_CONFIG_VERSION 1u
#define RIN_BLUETOOTH_HCI_CONNECTION_CONFIG_VERSION 1u
#define RIN_BLUETOOTH_HCI_MAX_DEVICES 32u
#define RIN_BLUETOOTH_HCI_MAX_REPORTS 25u
#define RIN_BLUETOOTH_HCI_MAX_EXT_REPORTS 10u
#define RIN_BLUETOOTH_HCI_MAX_FRAGMENT_CHAINS 4u
#define RIN_BLUETOOTH_HCI_MAX_PRIVACY_PEERS 4u
#define RIN_BLUETOOTH_HCI_MAX_CONNECTIONS 4u
#define RIN_BLUETOOTH_HCI_IRK_SIZE 16u
#define RIN_BLUETOOTH_HCI_MAX_LEGACY_ADV_DATA 31u
#define RIN_BLUETOOTH_HCI_MAX_REPORT_DATA 229u
#define RIN_BLUETOOTH_HCI_MAX_ADV_DATA 1650u
#define RIN_BLUETOOTH_HCI_MAX_NAME 64u

#define RIN_BLUETOOTH_HCI_EVENT_COMMAND_COMPLETE 0x0eu
#define RIN_BLUETOOTH_HCI_EVENT_COMMAND_STATUS   0x0fu
#define RIN_BLUETOOTH_HCI_EVENT_DISCONNECTION_COMPLETE 0x05u
#define RIN_BLUETOOTH_HCI_EVENT_LE_META          0x3eu
#define RIN_BLUETOOTH_HCI_SUBEVENT_ENCRYPTION_CHANGE 0x08u
#define RIN_BLUETOOTH_HCI_SUBEVENT_LONG_TERM_KEY_REQUEST 0x05u
#define RIN_BLUETOOTH_HCI_SUBEVENT_CONNECTION_COMPLETE 0x01u
#define RIN_BLUETOOTH_HCI_SUBEVENT_ADV_REPORT    0x02u
#define RIN_BLUETOOTH_HCI_SUBEVENT_ENHANCED_CONNECTION_COMPLETE 0x0au
#define RIN_BLUETOOTH_HCI_SUBEVENT_EXT_ADV_REPORT 0x0du

#define RIN_BLUETOOTH_HCI_OPCODE_RESET           0x0c03u
#define RIN_BLUETOOTH_HCI_OPCODE_READ_BD_ADDR    0x1009u
#define RIN_BLUETOOTH_HCI_OPCODE_LE_EVENT_MASK   0x2001u
#define RIN_BLUETOOTH_HCI_OPCODE_LE_RANDOM_ADDR  0x2005u
#define RIN_BLUETOOTH_HCI_OPCODE_LE_ADV_PARAMS   0x2006u
#define RIN_BLUETOOTH_HCI_OPCODE_LE_ADV_DATA     0x2008u
#define RIN_BLUETOOTH_HCI_OPCODE_LE_SCAN_RSP_DATA 0x2009u
#define RIN_BLUETOOTH_HCI_OPCODE_LE_ADV_ENABLE   0x200au
#define RIN_BLUETOOTH_HCI_OPCODE_LE_SCAN_PARAMS  0x200bu
#define RIN_BLUETOOTH_HCI_OPCODE_LE_SCAN_ENABLE  0x200cu
#define RIN_BLUETOOTH_HCI_OPCODE_LE_CREATE_CONNECTION 0x200du
#define RIN_BLUETOOTH_HCI_OPCODE_LE_CREATE_CONNECTION_CANCEL 0x200eu
#define RIN_BLUETOOTH_HCI_OPCODE_DISCONNECT      0x0406u
#define RIN_BLUETOOTH_HCI_OPCODE_LE_START_ENCRYPTION 0x2019u
#define RIN_BLUETOOTH_HCI_OPCODE_LE_LTK_REQUEST_REPLY 0x201au
#define RIN_BLUETOOTH_HCI_OPCODE_LE_LTK_REQUEST_NEGATIVE_REPLY 0x201bu
#define RIN_BLUETOOTH_HCI_OPCODE_LE_EXT_SCAN_PARAMS 0x2041u
#define RIN_BLUETOOTH_HCI_OPCODE_LE_EXT_SCAN_ENABLE 0x2042u
#define RIN_BLUETOOTH_HCI_OPCODE_LE_ADD_RESOLVING 0x2027u
#define RIN_BLUETOOTH_HCI_OPCODE_LE_CLEAR_RESOLVING 0x2029u
#define RIN_BLUETOOTH_HCI_OPCODE_LE_RESOLUTION_ENABLE 0x202du
#define RIN_BLUETOOTH_HCI_OPCODE_LE_RPA_TIMEOUT 0x202eu
#define RIN_BLUETOOTH_HCI_OPCODE_LE_PRIVACY_MODE 0x204eu

#define RIN_BLUETOOTH_HCI_SCAN_INACTIVE 0u
#define RIN_BLUETOOTH_HCI_SCAN_LEGACY   1u
#define RIN_BLUETOOTH_HCI_SCAN_EXTENDED 2u

#define RIN_BLUETOOTH_HCI_SCAN_PHY_1M    0x01u
#define RIN_BLUETOOTH_HCI_SCAN_PHY_CODED 0x04u

#define RIN_BLUETOOTH_HCI_REPORT_LEGACY   0u
#define RIN_BLUETOOTH_HCI_REPORT_EXTENDED 1u
#define RIN_BLUETOOTH_HCI_DATA_COMPLETE   0u
#define RIN_BLUETOOTH_HCI_DATA_MORE       1u
#define RIN_BLUETOOTH_HCI_DATA_TRUNCATED  2u

typedef enum RinBluetoothHciStatus {
    RIN_BLUETOOTH_HCI_OK = 0,
    RIN_BLUETOOTH_HCI_INVALID_ARGUMENT = -1,
    RIN_BLUETOOTH_HCI_INVALID_STATE = -2,
    RIN_BLUETOOTH_HCI_TRANSPORT_FAILURE = -3,
    RIN_BLUETOOTH_HCI_MALFORMED_EVENT = -4,
    RIN_BLUETOOTH_HCI_CONTROLLER_FAILURE = -5,
    RIN_BLUETOOTH_HCI_NOT_FOUND = -6,
    RIN_BLUETOOTH_HCI_LIMIT_EXCEEDED = -7
} RinBluetoothHciStatus;

typedef enum RinBluetoothHciState {
    RIN_BLUETOOTH_HCI_UNINITIALIZED = 0,
    RIN_BLUETOOTH_HCI_IDLE = 1,
    RIN_BLUETOOTH_HCI_RESET_PENDING = 2,
    RIN_BLUETOOTH_HCI_ADDRESS_PENDING = 3,
    RIN_BLUETOOTH_HCI_EVENT_MASK_PENDING = 4,
    RIN_BLUETOOTH_HCI_READY = 5,
    RIN_BLUETOOTH_HCI_SCAN_PARAMS_PENDING = 6,
    RIN_BLUETOOTH_HCI_SCAN_ENABLE_PENDING = 7,
    RIN_BLUETOOTH_HCI_SCANNING = 8,
    RIN_BLUETOOTH_HCI_SCAN_DISABLE_PENDING = 9,
    RIN_BLUETOOTH_HCI_FAILED = 10,
    RIN_BLUETOOTH_HCI_RANDOM_ADDRESS_PENDING = 11,
    RIN_BLUETOOTH_HCI_PRIVACY_CLEAR_PENDING = 12,
    RIN_BLUETOOTH_HCI_PRIVACY_ADD_PENDING = 13,
    RIN_BLUETOOTH_HCI_PRIVACY_MODE_PENDING = 14,
    RIN_BLUETOOTH_HCI_PRIVACY_TIMEOUT_PENDING = 15,
    RIN_BLUETOOTH_HCI_PRIVACY_ENABLE_PENDING = 16,
    RIN_BLUETOOTH_HCI_ADV_PARAMS_PENDING = 17,
    RIN_BLUETOOTH_HCI_ADV_DATA_PENDING = 18,
    RIN_BLUETOOTH_HCI_SCAN_RSP_DATA_PENDING = 19,
    RIN_BLUETOOTH_HCI_ADV_ENABLE_PENDING = 20,
    RIN_BLUETOOTH_HCI_ADV_DISABLE_PENDING = 21,
    RIN_BLUETOOTH_HCI_CONNECT_PENDING = 22,
    RIN_BLUETOOTH_HCI_CONNECTING = 23,
    RIN_BLUETOOTH_HCI_CONNECT_CANCEL_PENDING = 24,
    RIN_BLUETOOTH_HCI_DISCONNECT_PENDING = 25,
    RIN_BLUETOOTH_HCI_ENCRYPTION_START_PENDING = 26,
    RIN_BLUETOOTH_HCI_LTK_REPLY_PENDING = 27
} RinBluetoothHciState;

typedef struct RinBluetoothHciConnectionConfigV1 {
    uint32_t version;
    uint32_t struct_size;
    /* Scan units are 0.625 ms; connection interval units are 1.25 ms. */
    uint16_t scan_interval;
    uint16_t scan_window;
    uint8_t peer_address_type;
    /* Addresses use display order; HCI command/event octets are reversed. */
    uint8_t peer_address[6];
    uint8_t own_address_type;
    uint16_t connection_interval_min;
    uint16_t connection_interval_max;
    uint16_t connection_latency;
    /* Supervision timeout units are 10 ms. */
    uint16_t supervision_timeout;
    uint16_t min_connection_event_length;
    uint16_t max_connection_event_length;
} RinBluetoothHciConnectionConfigV1;

typedef struct RinBluetoothHciConnectionV1 {
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
    /* HCI encryption is reported separately from SMP authentication. */
    uint8_t encrypted;
    uint8_t encryption_pending;
    uint8_t encryption_command_pending;
    uint8_t ltk_request_pending;
    uint8_t ltk_reply_command_pending;
    uint8_t security_reserved[3];
    uint8_t ltk_random[8];
    uint16_t ltk_ediv;
    uint16_t security_reserved2;
} RinBluetoothHciConnectionV1;

typedef struct RinBluetoothHciExtendedScanConfigV1 {
    uint32_t version;
    uint32_t struct_size;
    uint8_t scanning_phys;
    uint8_t filter_duplicates;
    uint8_t active_1m;
    uint8_t active_coded;
    uint16_t interval_1m;
    uint16_t window_1m;
    uint16_t interval_coded;
    uint16_t window_coded;
} RinBluetoothHciExtendedScanConfigV1;

typedef struct RinBluetoothHciPrivacyPeerV1 {
    uint8_t identity_address_type;
    uint8_t privacy_mode;
    uint8_t identity_address[6];
    /* IRKs use the 16-octet order carried by HCI command parameters. */
    uint8_t peer_irk_hci[RIN_BLUETOOTH_HCI_IRK_SIZE];
} RinBluetoothHciPrivacyPeerV1;

typedef struct RinBluetoothHciPrivacyConfigV1 {
    uint32_t version;
    uint32_t struct_size;
    uint16_t rpa_timeout_seconds;
    uint8_t peer_count;
    uint8_t reserved;
    uint8_t local_irk_hci[RIN_BLUETOOTH_HCI_IRK_SIZE];
    RinBluetoothHciPrivacyPeerV1 peers[
        RIN_BLUETOOTH_HCI_MAX_PRIVACY_PEERS];
} RinBluetoothHciPrivacyConfigV1;

/* Legacy LE advertising command set. Data remains controller-owned and is
 * copied before the first HCI command is submitted. Directed advertising is
 * intentionally excluded: it requires an explicit peer-address contract. */
typedef struct RinBluetoothHciAdvertisingConfigV1 {
    uint32_t version;
    uint32_t struct_size;
    uint16_t interval_min;
    uint16_t interval_max;
    uint8_t advertising_type;
    uint8_t own_address_type;
    uint8_t channel_map;
    uint8_t filter_policy;
    uint8_t advertising_data_size;
    uint8_t scan_response_data_size;
    uint8_t reserved[2];
    uint8_t advertising_data[RIN_BLUETOOTH_HCI_MAX_LEGACY_ADV_DATA];
    uint8_t scan_response_data[RIN_BLUETOOTH_HCI_MAX_LEGACY_ADV_DATA];
} RinBluetoothHciAdvertisingConfigV1;

typedef struct RinBluetoothHciDeviceV1 {
    uint32_t version;
    uint32_t generation;
    uint8_t present;
    uint8_t address_type;
    uint8_t event_type;
    int8_t rssi_dbm;
    uint8_t address[6];
    uint8_t advertising_flags;
    uint8_t advertising_flags_present;
    uint16_t data_size;
    uint8_t name_size;
    uint8_t data[RIN_BLUETOOTH_HCI_MAX_ADV_DATA];
    char name[RIN_BLUETOOTH_HCI_MAX_NAME];
    uint64_t last_seen_ms;
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
} RinBluetoothHciDeviceV1;

typedef struct RinBluetoothHciFragmentV1 {
    uint8_t active;
    uint8_t address_type;
    uint8_t address[6];
    uint8_t event_type;
    uint8_t primary_phy;
    uint8_t secondary_phy;
    uint8_t advertising_sid;
    uint16_t periodic_advertising_interval;
    uint8_t direct_address_type;
    uint8_t direct_address_present;
    uint8_t direct_address[6];
    uint16_t data_size;
    uint64_t last_update_ms;
    uint8_t data[RIN_BLUETOOTH_HCI_MAX_ADV_DATA];
} RinBluetoothHciFragmentV1;

/* Kernel-private controller layout v3; the V1 type name is retained to avoid
 * renaming the existing internal owner family. All users validate version
 * and struct_size, so layout-v1 instances are rejected. */
typedef struct RinBluetoothHciControllerV1 {
    uint32_t version;
    uint32_t struct_size;
    RinBluetoothHciState state;
    uint16_t pending_opcode;
    uint8_t last_controller_status;
    uint8_t scan_active;
    uint8_t scan_filter_duplicates;
    uint8_t scan_command_family;
    uint8_t local_address_present;
    uint8_t local_address[6];
    uint8_t local_address_type;
    uint8_t static_random_address_configured;
    uint8_t static_random_address[6];
    uint8_t privacy_configured;
    uint8_t address_resolution_enabled;
    uint8_t privacy_peer_index;
    RinBluetoothHciPrivacyConfigV1 privacy;
    uint32_t generation;
    uint32_t device_count;
    uint32_t fragment_count;
    RinBluetoothHciTransportV1 transport;
    RinBluetoothHciFragmentV1 fragments[
        RIN_BLUETOOTH_HCI_MAX_FRAGMENT_CHAINS];
    RinBluetoothHciFragmentV1 fragment_scratch[
        RIN_BLUETOOTH_HCI_MAX_FRAGMENT_CHAINS];
    uint8_t fragment_assembled_scratch[
        RIN_BLUETOOTH_HCI_MAX_FRAGMENT_CHAINS]
        [RIN_BLUETOOTH_HCI_MAX_ADV_DATA];
    RinBluetoothHciDeviceV1 devices[RIN_BLUETOOTH_HCI_MAX_DEVICES];
    uint8_t advertising_configured;
    uint8_t advertising_active;
    uint8_t advertising_reserved[2];
    RinBluetoothHciAdvertisingConfigV1 advertising;
    uint32_t connection_generation;
    uint32_t connection_count;
    uint16_t disconnect_handle;
    uint16_t connection_reserved;
    uint32_t disconnect_generation;
    RinBluetoothHciState disconnect_previous_state;
    uint8_t connection_pending;
    uint8_t connection_cancel_pending;
    uint8_t disconnect_pending;
    uint8_t connection_reserved_flags;
    RinBluetoothHciConnectionConfigV1 pending_connection;
    RinBluetoothHciConnectionV1 connections[
        RIN_BLUETOOTH_HCI_MAX_CONNECTIONS];
} RinBluetoothHciControllerV1;

/* The transport owner serializes calls and must queue commands rather than
 * re-entering the controller from send_command(). Event buffers exclude an
 * optional H:4 packet-type byte and begin with Event_Code, Parameter_Length. */
int rin_bluetooth_hci_init(
    RinBluetoothHciControllerV1* controller,
    const RinBluetoothHciTransportV1* transport);
int rin_bluetooth_hci_configure_static_random_address(
    RinBluetoothHciControllerV1* controller, const uint8_t address[6]);
int rin_bluetooth_hci_configure_privacy(
    RinBluetoothHciControllerV1* controller,
    const RinBluetoothHciPrivacyConfigV1* config);
int rin_bluetooth_hci_start(RinBluetoothHciControllerV1* controller);
int rin_bluetooth_hci_start_scan(
    RinBluetoothHciControllerV1* controller, int active_scan,
    uint16_t interval, uint16_t window, int filter_duplicates);
int rin_bluetooth_hci_start_extended_scan(
    RinBluetoothHciControllerV1* controller, int active_scan,
    uint16_t interval, uint16_t window, int filter_duplicates);
int rin_bluetooth_hci_start_extended_scan_config(
    RinBluetoothHciControllerV1* controller,
    const RinBluetoothHciExtendedScanConfigV1* config);
int rin_bluetooth_hci_stop_scan(RinBluetoothHciControllerV1* controller);
int rin_bluetooth_hci_start_connection(
    RinBluetoothHciControllerV1* controller,
    const RinBluetoothHciConnectionConfigV1* config);
int rin_bluetooth_hci_cancel_connection(
    RinBluetoothHciControllerV1* controller);
int rin_bluetooth_hci_disconnect(
    RinBluetoothHciControllerV1* controller, uint16_t handle,
    uint32_t generation);
/* Start central-side LE encryption with an LTK selected by the trusted
 * pairing/bond owner.  RAND octets use HCI wire order.  A successful command
 * submission is not encryption evidence; wait for LE Encryption Change. */
int rin_bluetooth_hci_start_encryption(
    RinBluetoothHciControllerV1* controller, uint16_t handle,
    uint32_t generation, const uint8_t rand[8], uint16_t ediv,
    const uint8_t ltk[16]);
/* Peripheral-side LTK requests are captured by process_event and exposed in
 * the generation-pinned connection record.  Reply only after the trusted key
 * owner has matched the requested link and key identity. */
int rin_bluetooth_hci_reply_ltk_request(
    RinBluetoothHciControllerV1* controller, uint16_t handle,
    uint32_t generation, const uint8_t ltk[16]);
int rin_bluetooth_hci_reject_ltk_request(
    RinBluetoothHciControllerV1* controller, uint16_t handle,
    uint32_t generation);
int rin_bluetooth_hci_get_connection(
    const RinBluetoothHciControllerV1* controller, uint32_t index,
    RinBluetoothHciConnectionV1* connection_out);
int rin_bluetooth_hci_start_advertising(
    RinBluetoothHciControllerV1* controller,
    const RinBluetoothHciAdvertisingConfigV1* config);
int rin_bluetooth_hci_stop_advertising(
    RinBluetoothHciControllerV1* controller);
int rin_bluetooth_hci_process_event(
    RinBluetoothHciControllerV1* controller, const uint8_t* event,
    uint32_t event_size);
int rin_bluetooth_hci_get_device(
    const RinBluetoothHciControllerV1* controller, uint32_t index,
    RinBluetoothHciDeviceV1* device_out);
void rin_bluetooth_hci_destroy(RinBluetoothHciControllerV1* controller);

#endif /* RIN_DRIVER_BLUETOOTH_HCI_RUNTIME_V1_H */
