/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_DRIVERS_USB_AUDIO_UAC1_RUNTIME_V1_H
#define RIN_DRIVERS_USB_AUDIO_UAC1_RUNTIME_V1_H

#include "uac1_descriptor_v1.h"

#include <stdint.h>

#define RIN_USB_AUDIO_UAC1_RUNTIME_ABI_VERSION UINT32_C(0x00010000)
#define RIN_USB_AUDIO_UAC1_RUNTIME_STORAGE_BYTES 1152u

#define RIN_USB_AUDIO_UAC1_INTERFACE_ACTIVE UINT32_C(0x00000001)
#define RIN_USB_AUDIO_UAC1_INTERFACE_RECOVERY_REQUIRED UINT32_C(0x00000002)
#define RIN_USB_AUDIO_UAC1_INTERFACE_FLAGS                              \
    (RIN_USB_AUDIO_UAC1_INTERFACE_ACTIVE |                             \
     RIN_USB_AUDIO_UAC1_INTERFACE_RECOVERY_REQUIRED)

#define RIN_USB_AUDIO_UAC1_RUNTIME_HAS_ACTIVE UINT32_C(0x00000001)
#define RIN_USB_AUDIO_UAC1_RUNTIME_RECOVERY_REQUIRED UINT32_C(0x00000002)
#define RIN_USB_AUDIO_UAC1_RUNTIME_FLAGS                                \
    (RIN_USB_AUDIO_UAC1_RUNTIME_HAS_ACTIVE |                           \
     RIN_USB_AUDIO_UAC1_RUNTIME_RECOVERY_REQUIRED)

typedef enum RinUsbAudioUac1RuntimeResultV1 {
    RIN_USB_AUDIO_UAC1_RUNTIME_OK = 0,
    RIN_USB_AUDIO_UAC1_RUNTIME_INVALID_ARGUMENT = -1,
    RIN_USB_AUDIO_UAC1_RUNTIME_INVALID_CATALOG = -2,
    RIN_USB_AUDIO_UAC1_RUNTIME_STALE_GENERATION = -3,
    RIN_USB_AUDIO_UAC1_RUNTIME_BUSY = -4,
    RIN_USB_AUDIO_UAC1_RUNTIME_NOT_FOUND = -5,
    RIN_USB_AUDIO_UAC1_RUNTIME_RATE_UNSUPPORTED = -6,
    RIN_USB_AUDIO_UAC1_RUNTIME_CONTROL_UNAVAILABLE = -7,
    RIN_USB_AUDIO_UAC1_RUNTIME_INTERFACE_ACTIVE = -8,
    RIN_USB_AUDIO_UAC1_RUNTIME_BACKEND_FAILURE = -9,
    RIN_USB_AUDIO_UAC1_RUNTIME_SHORT_TRANSFER = -10,
    RIN_USB_AUDIO_UAC1_RUNTIME_VERIFY_MISMATCH = -11,
    RIN_USB_AUDIO_UAC1_RUNTIME_CALLBACK_CORRUPTION = -12,
    RIN_USB_AUDIO_UAC1_RUNTIME_RECOVERY_REQUIRED_RESULT = -13,
    RIN_USB_AUDIO_UAC1_RUNTIME_GENERATION_EXHAUSTED = -14
} RinUsbAudioUac1RuntimeResultV1;

typedef struct RinUsbAudioUac1SetupPacketV1 {
    uint8_t request_type;
    uint8_t request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
} RinUsbAudioUac1SetupPacketV1;

typedef int (*RinUsbAudioUac1ControlTransferV1)(
    void* context,
    const RinUsbAudioUac1SetupPacketV1* setup,
    const uint8_t* output_data,
    uint32_t output_length,
    uint8_t* input_data,
    uint32_t input_capacity,
    uint32_t* transferred);

typedef struct RinUsbAudioUac1ControlBackendV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    void* context;
    RinUsbAudioUac1ControlTransferV1 control_transfer;
    uint32_t reserved[4];
} RinUsbAudioUac1ControlBackendV1;

typedef struct RinUsbAudioUac1InterfaceStateV1 {
    uint32_t flags;
    uint32_t profile_index;
    uint32_t sample_rate;
    uint32_t reserved0;
    uint8_t interface_number;
    uint8_t alternate_setting;
    uint8_t endpoint_address;
    uint8_t reserved1[5];
} RinUsbAudioUac1InterfaceStateV1;

typedef struct RinUsbAudioUac1RuntimeSnapshotV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t generation;
    uint32_t interface_count;
    uint32_t catalog_profile_count;
    uint32_t flags;
    uint32_t reserved[2];
    RinUsbAudioUac1InterfaceStateV1
        interfaces[RIN_USB_AUDIO_UAC1_MAX_PROFILES];
} RinUsbAudioUac1RuntimeSnapshotV1;

typedef union RinUsbAudioUac1RuntimeV1 {
    uint64_t alignment;
    uint8_t bytes[RIN_USB_AUDIO_UAC1_RUNTIME_STORAGE_BYTES];
} RinUsbAudioUac1RuntimeV1;

/*
 * The backend callback executes synchronous USB control transfers. Successful
 * callbacks must report the exact data-stage byte count through transferred.
 * Initialization snapshots the admitted catalog and callback table.
 */
RinUsbAudioUac1RuntimeResultV1 rin_usb_audio_uac1_runtime_init_v1(
    RinUsbAudioUac1RuntimeV1* runtime,
    const RinUsbAudioUac1CatalogV1* catalog,
    const RinUsbAudioUac1ControlBackendV1* backend);

RinUsbAudioUac1RuntimeResultV1 rin_usb_audio_uac1_runtime_snapshot_v1(
    RinUsbAudioUac1RuntimeV1* runtime,
    RinUsbAudioUac1RuntimeSnapshotV1* snapshot);

RinUsbAudioUac1RuntimeResultV1 rin_usb_audio_uac1_runtime_activate_v1(
    RinUsbAudioUac1RuntimeV1* runtime,
    uint32_t profile_index,
    uint32_t sample_rate,
    uint32_t expected_generation);

RinUsbAudioUac1RuntimeResultV1 rin_usb_audio_uac1_runtime_deactivate_v1(
    RinUsbAudioUac1RuntimeV1* runtime,
    uint8_t interface_number,
    uint32_t expected_generation);

RinUsbAudioUac1RuntimeResultV1 rin_usb_audio_uac1_runtime_recover_v1(
    RinUsbAudioUac1RuntimeV1* runtime,
    uint8_t interface_number,
    uint32_t expected_generation);

#if defined(__cplusplus)
static_assert(sizeof(RinUsbAudioUac1SetupPacketV1) == 8u,
              "UAC1 setup packet ABI drift");
static_assert(sizeof(RinUsbAudioUac1InterfaceStateV1) == 24u,
              "UAC1 interface state ABI drift");
static_assert(sizeof(RinUsbAudioUac1RuntimeSnapshotV1) == 224u,
              "UAC1 runtime snapshot ABI drift");
static_assert(sizeof(RinUsbAudioUac1RuntimeV1) ==
                  RIN_USB_AUDIO_UAC1_RUNTIME_STORAGE_BYTES,
              "UAC1 runtime storage ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinUsbAudioUac1SetupPacketV1) == 8u,
               "UAC1 setup packet ABI drift");
_Static_assert(sizeof(RinUsbAudioUac1InterfaceStateV1) == 24u,
               "UAC1 interface state ABI drift");
_Static_assert(sizeof(RinUsbAudioUac1RuntimeSnapshotV1) == 224u,
               "UAC1 runtime snapshot ABI drift");
_Static_assert(sizeof(RinUsbAudioUac1RuntimeV1) ==
                   RIN_USB_AUDIO_UAC1_RUNTIME_STORAGE_BYTES,
               "UAC1 runtime storage ABI drift");
#endif

#endif /* RIN_DRIVERS_USB_AUDIO_UAC1_RUNTIME_V1_H */
