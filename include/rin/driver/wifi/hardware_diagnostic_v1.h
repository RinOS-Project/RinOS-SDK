/* SPDX-License-Identifier: MIT */
/* Pointer-free Wi-Fi diagnostic records; these values do not grant access. */

#ifndef RIN_DRIVER_WIFI_HARDWARE_DIAGNOSTIC_V1_H
#define RIN_DRIVER_WIFI_HARDWARE_DIAGNOSTIC_V1_H

#include <stdint.h>

#define RIN_WIFI_PHYSICAL_DRIVER_V1_VERSION UINT32_C(0x00010006)
#define RIN_WIFI_HARDWARE_TRANSPORT_V1_VERSION UINT32_C(0x00010001)
#define RIN_WIFI_HARDWARE_VENDOR_INTEL UINT16_C(0x8086)
#define RIN_WIFI_HARDWARE_VENDOR_REALTEK UINT16_C(0x10ec)
#define RIN_WIFI_HARDWARE_INTEL_6E_7_DEVICE UINT16_C(0x2725)
#define RIN_WIFI_HARDWARE_INTEL_6E_7_SUBSYSTEM_VENDOR UINT16_C(0x8086)
#define RIN_WIFI_HARDWARE_INTEL_6E_7_SUBSYSTEM_DEVICE UINT16_C(0x0090)
#define RIN_WIFI_HARDWARE_REALTEK_8852_DEVICE UINT16_C(0x8852)
#define RIN_WIFI_HARDWARE_REALTEK_8852_SUBSYSTEM_VENDOR UINT16_C(0x10ec)
#define RIN_WIFI_HARDWARE_REALTEK_8852_SUBSYSTEM_DEVICE UINT16_C(0xc852)
#define RIN_WIFI_VENDOR_BACKEND_V1_VERSION UINT32_C(0x00010000)
#define RIN_WIFI_VENDOR_BACKEND_V1_MIN_MMIO_LENGTH UINT64_C(0x1000)
#define RIN_WIFI_VENDOR_BACKEND_V1_MAX_MMIO_LENGTH UINT64_C(0x1000000)

typedef enum RinWifiVendorBackendV1State {
    RIN_WIFI_VENDOR_BACKEND_V1_STATE_NONE = 0,
    RIN_WIFI_VENDOR_BACKEND_V1_STATE_ACTIVE = 1,
    RIN_WIFI_VENDOR_BACKEND_V1_STATE_DEGRADED = 2,
    RIN_WIFI_VENDOR_BACKEND_V1_STATE_DETACHED = 3
} RinWifiVendorBackendV1State;

typedef enum RinWifiHardwareFamilyV1 {
    RIN_WIFI_HARDWARE_FAMILY_NONE = 0,
    RIN_WIFI_HARDWARE_FAMILY_INTEL_6E_7 = 1,
    RIN_WIFI_HARDWARE_FAMILY_REALTEK_8852 = 2
} RinWifiHardwareFamilyV1;

typedef struct RinWifiHardwareProfileV1 {
    uint32_t struct_size;
    uint32_t version;
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_device_id;
    uint8_t minimum_revision;
    uint8_t maximum_revision;
    uint8_t reserved0[2];
    uint32_t family;
    uint64_t device_cookie;
    uint64_t device_generation;
    uint64_t reserved[2];
} RinWifiHardwareProfileV1;

/* Shared validation for exact, supported PCI identities. Keeping this
 * contract inline lets kernel consumers validate diagnostics without linking
 * against the Wi-Fi transport implementation. */
static inline int rin_wifi_hardware_profile_valid_contract(
    const RinWifiHardwareProfileV1* profile)
{
    uint32_t family_vendor_valid;
    if (!profile || profile->struct_size != sizeof(*profile) ||
        profile->version != RIN_WIFI_HARDWARE_TRANSPORT_V1_VERSION ||
        profile->vendor_id == 0u || profile->device_id == 0u ||
        profile->subsystem_vendor_id == 0u ||
        profile->subsystem_device_id == 0u ||
        profile->minimum_revision > profile->maximum_revision ||
        profile->device_cookie == 0u || profile->device_generation == 0u ||
        profile->reserved0[0] != 0u || profile->reserved0[1] != 0u ||
        profile->reserved[0] != 0u || profile->reserved[1] != 0u)
        return 0;
    family_vendor_valid =
        (profile->family == RIN_WIFI_HARDWARE_FAMILY_INTEL_6E_7 &&
         profile->vendor_id == RIN_WIFI_HARDWARE_VENDOR_INTEL) ||
        (profile->family == RIN_WIFI_HARDWARE_FAMILY_REALTEK_8852 &&
         profile->vendor_id == RIN_WIFI_HARDWARE_VENDOR_REALTEK);
    return family_vendor_valid != 0u;
}

static inline int rin_wifi_hardware_profile_exact_supported_contract(
    const RinWifiHardwareProfileV1* profile)
{
    if (!rin_wifi_hardware_profile_valid_contract(profile)) return 0;
    if (profile->family == RIN_WIFI_HARDWARE_FAMILY_INTEL_6E_7)
        return profile->device_id == RIN_WIFI_HARDWARE_INTEL_6E_7_DEVICE &&
               profile->subsystem_vendor_id ==
                   RIN_WIFI_HARDWARE_INTEL_6E_7_SUBSYSTEM_VENDOR &&
               profile->subsystem_device_id ==
                   RIN_WIFI_HARDWARE_INTEL_6E_7_SUBSYSTEM_DEVICE;
    if (profile->family == RIN_WIFI_HARDWARE_FAMILY_REALTEK_8852)
        return profile->device_id == RIN_WIFI_HARDWARE_REALTEK_8852_DEVICE &&
               profile->subsystem_vendor_id ==
                   RIN_WIFI_HARDWARE_REALTEK_8852_SUBSYSTEM_VENDOR &&
               profile->subsystem_device_id ==
                   RIN_WIFI_HARDWARE_REALTEK_8852_SUBSYSTEM_DEVICE;
    return 0;
}

typedef struct RinWifiHardwareSnapshotV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t state;
    uint32_t family;
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_device_id;
    uint64_t device_generation;
    uint32_t scan_result_count;
    uint32_t malformed_frame_count;
    int32_t last_error;
    uint32_t reserved0;
    uint64_t reserved[3];
} RinWifiHardwareSnapshotV1;

typedef struct RinWifiVendorBackendV1SnapshotV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t state;
    uint32_t reserved0;
    RinWifiHardwareProfileV1 profile;
    uint64_t bar_lease;
    uint64_t irq_lease;
    uint64_t dma_lease;
    uint64_t mmio_length;
    int32_t last_error;
    uint32_t operation_count;
    uint32_t reserved1;
    uint64_t reserved[2];
} RinWifiVendorBackendV1SnapshotV1;

typedef struct RinWifiPhysicalDriverV1Snapshot {
    uint32_t struct_size;
    uint32_t version;
    uint32_t driver_state;
    uint32_t family;
    RinWifiHardwareProfileV1 profile;
    RinWifiHardwareSnapshotV1 transport;
    RinWifiVendorBackendV1SnapshotV1 vendor_backend;
    uint64_t reserved[2];
} RinWifiPhysicalDriverV1Snapshot;

#if defined(__cplusplus)
static_assert(sizeof(RinWifiHardwareProfileV1) == 56u,
              "Wi-Fi hardware profile ABI drift");
static_assert(sizeof(RinWifiHardwareSnapshotV1) == 72u,
              "Wi-Fi hardware snapshot ABI drift");
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(RinWifiVendorBackendV1SnapshotV1) == 132u,
              "Wi-Fi vendor diagnostic snapshot ABI drift");
#else
static_assert(sizeof(RinWifiVendorBackendV1SnapshotV1) == 136u,
              "Wi-Fi vendor diagnostic snapshot ABI drift");
#endif
#else
_Static_assert(sizeof(RinWifiHardwareProfileV1) == 56u,
               "Wi-Fi hardware profile ABI drift");
_Static_assert(sizeof(RinWifiHardwareSnapshotV1) == 72u,
               "Wi-Fi hardware snapshot ABI drift");
#if UINTPTR_MAX == UINT32_MAX
_Static_assert(sizeof(RinWifiVendorBackendV1SnapshotV1) == 132u,
               "Wi-Fi vendor diagnostic snapshot ABI drift");
#else
_Static_assert(sizeof(RinWifiVendorBackendV1SnapshotV1) == 136u,
               "Wi-Fi vendor diagnostic snapshot ABI drift");
#endif
#endif

#endif /* RIN_DRIVER_WIFI_HARDWARE_DIAGNOSTIC_V1_H */
