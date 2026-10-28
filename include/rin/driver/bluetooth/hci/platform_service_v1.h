/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_BLUETOOTH_HCI_PLATFORM_SERVICE_V1_H
#define RIN_DRIVER_BLUETOOTH_HCI_PLATFORM_SERVICE_V1_H

#include <stdint.h>

#include <rin/contract_abi.h>

#define RIN_BLUETOOTH_HCI_PLATFORM_SERVICE_V1_VERSION UINT32_C(0x00010000)

struct RinBluetoothHciPhysicalControllerV1;

typedef RinResultCode (*RinBluetoothHciPlatformRegisterControllerV1Fn)(
    void* context, struct RinBluetoothHciPhysicalControllerV1* controller);
typedef RinResultCode (*RinBluetoothHciPlatformUnregisterControllerV1Fn)(
    void* context, struct RinBluetoothHciPhysicalControllerV1* controller);

/* Kernel-owned lifecycle service for an initialized physical HCI controller.
 * The controller remains owned by its provider; unregister may return BUSY
 * while Kernel clients still hold references. context is opaque provider state. */
typedef struct RinBluetoothHciPlatformServiceV1 {
    uint32_t version;
    uint32_t struct_size;
    void* context;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t context_padding;
#endif
    RinBluetoothHciPlatformRegisterControllerV1Fn register_controller;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t register_controller_padding;
#endif
    RinBluetoothHciPlatformUnregisterControllerV1Fn unregister_controller;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t unregister_controller_padding;
#endif
    uint64_t reserved[2];
} RinBluetoothHciPlatformServiceV1;

static inline int rin_bluetooth_hci_platform_service_v1_valid(
    const RinBluetoothHciPlatformServiceV1* service) {
    if (!service ||
        service->version != RIN_BLUETOOTH_HCI_PLATFORM_SERVICE_V1_VERSION ||
        service->struct_size != sizeof(*service) ||
        !service->register_controller || !service->unregister_controller ||
        service->reserved[0] != 0u || service->reserved[1] != 0u)
        return 0;
#if UINTPTR_MAX == UINT32_MAX
    if (service->context_padding != 0u ||
        service->register_controller_padding != 0u ||
        service->unregister_controller_padding != 0u)
        return 0;
#endif
    return 1;
}

#if defined(__cplusplus)
static_assert(sizeof(RinBluetoothHciPlatformServiceV1) == 48u,
              "Bluetooth HCI platform service ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinBluetoothHciPlatformServiceV1) == 48u,
               "Bluetooth HCI platform service ABI drift");
#endif

#endif /* RIN_DRIVER_BLUETOOTH_HCI_PLATFORM_SERVICE_V1_H */
