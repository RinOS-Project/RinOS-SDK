/* SPDX-License-Identifier: MIT */
/* Versioned Wi-Fi management-frame I/O contract. */

#ifndef RIN_DRIVER_WIFI_MANAGEMENT_BRIDGE_V1_H
#define RIN_DRIVER_WIFI_MANAGEMENT_BRIDGE_V1_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_WIFI_HARDWARE_MANAGEMENT_BRIDGE_V1_VERSION UINT32_C(0x00010001)
#define RIN_WIFI_HARDWARE_MANAGEMENT_BRIDGE_V1_MAGIC UINT32_C(0x314d5748) /* HWM1 */
#define RIN_WIFI_HARDWARE_MANAGEMENT_MAX_FRAME_SIZE UINT32_C(2304)

#define RIN_WIFI_MANAGEMENT_IO_V1_VERSION UINT32_C(1)

typedef enum RinWifiManagementIoResultV1 {
    RIN_WIFI_MANAGEMENT_IO_OK = 0,
    RIN_WIFI_MANAGEMENT_IO_BUSY = -1,
    RIN_WIFI_MANAGEMENT_IO_STALE = -2,
    RIN_WIFI_MANAGEMENT_IO_NOT_READY = -3,
    RIN_WIFI_MANAGEMENT_IO_FAILURE = -4
} RinWifiManagementIoResultV1;

typedef int (*RinWifiManagementIoValidateFn)(
    void* context, uint64_t device_generation);
typedef int (*RinWifiManagementIoTransmitFn)(
    void* context, uint64_t device_generation, const uint8_t* frame,
    uint32_t frame_size);
typedef int (*RinWifiManagementIoReceiveFn)(
    void* context, uint64_t device_generation, uint8_t* frame,
    uint32_t frame_capacity, uint32_t* frame_size_out);

/* Drivers bind these callbacks to a physical transport. The kernel sees only
 * the bounded, generation-aware interface and never owns a driver object. */
typedef struct RinWifiManagementIoV1 {
    uint32_t struct_size;
    uint32_t version;
    RinWifiManagementIoValidateFn validate;
    RinWifiManagementIoTransmitFn transmit;
    RinWifiManagementIoReceiveFn receive;
    void* context;
    uint64_t reserved[2];
} RinWifiManagementIoV1;

typedef enum RinWifiHardwareManagementBridgeResultV1 {
    RIN_WIFI_HARDWARE_MANAGEMENT_BRIDGE_OK = 0,
    RIN_WIFI_HARDWARE_MANAGEMENT_BRIDGE_INVALID = -1,
    RIN_WIFI_HARDWARE_MANAGEMENT_BRIDGE_BUSY = -2,
    RIN_WIFI_HARDWARE_MANAGEMENT_BRIDGE_STALE = -3,
    RIN_WIFI_HARDWARE_MANAGEMENT_BRIDGE_MALFORMED = -4,
    RIN_WIFI_HARDWARE_MANAGEMENT_BRIDGE_SECURITY = -5,
    RIN_WIFI_HARDWARE_MANAGEMENT_BRIDGE_TRANSPORT = -6,
    RIN_WIFI_HARDWARE_MANAGEMENT_BRIDGE_NO_DATA = -7,
    RIN_WIFI_HARDWARE_MANAGEMENT_BRIDGE_NOT_READY = -8
} RinWifiHardwareManagementBridgeResultV1;

typedef enum RinWifiHardwareManagementBridgeDirectionV1 {
    RIN_WIFI_HARDWARE_MANAGEMENT_BRIDGE_TX = 1,
    RIN_WIFI_HARDWARE_MANAGEMENT_BRIDGE_RX = 2
} RinWifiHardwareManagementBridgeDirectionV1;

typedef enum RinWifiHardwareManagementBridgeStateV1 {
    RIN_WIFI_HARDWARE_MANAGEMENT_BRIDGE_UNINITIALIZED = 0,
    RIN_WIFI_HARDWARE_MANAGEMENT_BRIDGE_ACTIVE = 1,
    RIN_WIFI_HARDWARE_MANAGEMENT_BRIDGE_STALE_STATE = 2,
    RIN_WIFI_HARDWARE_MANAGEMENT_BRIDGE_FAILED = 3
} RinWifiHardwareManagementBridgeStateV1;

/* The security owner must authenticate the already protected PMF frame. SAE
 * cryptography and BIP/IGTK key ownership stay outside this transport bridge. */
typedef int (*RinWifiHardwareManagementAuthorizeFn)(
    void* context, uint32_t direction, const uint8_t* frame,
    uint32_t frame_size);

typedef struct RinWifiHardwareManagementBridgeConfigV1 {
    uint32_t struct_size;
    uint32_t version;
    RinWifiManagementIoV1 io;
    uint64_t device_generation;
    uint64_t association_generation;
    uint32_t selected_security;
    uint8_t bssid[6];
    uint8_t station_address[6];
    uint8_t pmf_required;
    uint8_t pmf_active;
    uint16_t reserved0;
    RinWifiHardwareManagementAuthorizeFn authorize;
    void* authorize_context;
    uint64_t reserved[2];
} RinWifiHardwareManagementBridgeConfigV1;

typedef struct RinWifiHardwareManagementBridgeV1 {
    uint32_t magic;
    uint32_t state;
    volatile uint32_t lock;
    uint32_t callback_active;
    RinWifiManagementIoV1 io;
    uint64_t device_generation;
    uint64_t association_generation;
    uint32_t selected_security;
    uint8_t bssid[6];
    uint8_t station_address[6];
    uint8_t pmf_required;
    uint8_t pmf_active;
    uint16_t reserved0;
    RinWifiHardwareManagementAuthorizeFn authorize;
    void* authorize_context;
    uint64_t reserved[2];
} RinWifiHardwareManagementBridgeV1;

int rin_wifi_hardware_management_bridge_init(
    RinWifiHardwareManagementBridgeV1* bridge,
    const RinWifiHardwareManagementBridgeConfigV1* config);

/* Transmit/receive the unprotected SAE Authentication exchange. The bridge
 * admits only algorithm 3, transaction 1 or 2, status 0, and exact BSS
 * addresses; the SAE payload is bounded but authenticated by the SAE owner. */
int rin_wifi_hardware_management_bridge_transmit_sae(
    RinWifiHardwareManagementBridgeV1* bridge, const uint8_t* frame,
    uint32_t frame_size);
int rin_wifi_hardware_management_bridge_receive_sae(
    RinWifiHardwareManagementBridgeV1* bridge, uint8_t* frame_out,
    uint32_t capacity, uint32_t* frame_size_out);

/* Transmit/receive robust protected management frames (Action, Disassociation,
 * or Deauthentication). A non-null authorize callback is required and must
 * accept only frames whose BIP/IGTK integrity was established by the security
 * owner; unprotected frames are never published as PMF traffic. */
int rin_wifi_hardware_management_bridge_transmit_pmf(
    RinWifiHardwareManagementBridgeV1* bridge, const uint8_t* frame,
    uint32_t frame_size);
int rin_wifi_hardware_management_bridge_receive_pmf(
    RinWifiHardwareManagementBridgeV1* bridge, uint8_t* frame_out,
    uint32_t capacity, uint32_t* frame_size_out);

/* Make all frame/key-adjacent operations stale after a device or firmware
 * reset. A new generation must initialize a new bridge instance. */
int rin_wifi_hardware_management_bridge_invalidate_for_device_reset(
    RinWifiHardwareManagementBridgeV1* bridge);

#ifdef __cplusplus
}
#endif

#endif /* RIN_DRIVER_WIFI_MANAGEMENT_BRIDGE_V1_H */
