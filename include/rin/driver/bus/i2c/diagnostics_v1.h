/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_BUS_I2C_DIAGNOSTICS_V1_H
#define RIN_DRIVER_BUS_I2C_DIAGNOSTICS_V1_H

#include <stdint.h>

/* Pointer-free I2C status records shared by device drivers and OS-Core.
 * Cookie fields identify a live generation-bound device or transfer; they do
 * not authorize hardware access. */
#define RIN_I2C_CONTROLLER_V1_VERSION UINT32_C(0x00010000)

typedef enum RinI2cControllerV1Profile {
    RIN_I2C_CONTROLLER_V1_PROFILE_NONE = 0,
    RIN_I2C_CONTROLLER_V1_PROFILE_INTEL_DESIGNWARE = 1,
    RIN_I2C_CONTROLLER_V1_PROFILE_AMD_DESIGNWARE = 2,
    RIN_I2C_CONTROLLER_V1_PROFILE_AMD_MP2 = 3
} RinI2cControllerV1Profile;

typedef enum RinI2cControllerV1State {
    RIN_I2C_CONTROLLER_V1_UNINITIALIZED = 0,
    RIN_I2C_CONTROLLER_V1_ACTIVE = 1,
    RIN_I2C_CONTROLLER_V1_SUSPENDED = 2,
    RIN_I2C_CONTROLLER_V1_RECOVERY_REQUIRED = 3,
    RIN_I2C_CONTROLLER_V1_DETACHED = 4
} RinI2cControllerV1State;

typedef struct RinI2cControllerV1Snapshot {
    uint32_t struct_size;
    uint32_t version;
    uint64_t device_cookie;
    uint64_t device_generation;
    uint64_t transfer_generation;
    uint32_t profile;
    uint32_t state;
    uint32_t last_error;
    uint32_t last_abort_source;
    uint64_t transfer_count;
    uint64_t byte_count;
    uint64_t reserved[3];
} RinI2cControllerV1Snapshot;

typedef enum RinI2cPlatformDmaCompletionV1State {
    RIN_I2C_PLATFORM_DMA_COMPLETION_V1_STATE_NONE = 0,
    RIN_I2C_PLATFORM_DMA_COMPLETION_V1_STATE_ACTIVE = 1,
    RIN_I2C_PLATFORM_DMA_COMPLETION_V1_STATE_DEGRADED = 2,
    RIN_I2C_PLATFORM_DMA_COMPLETION_V1_STATE_DETACHED = 3
} RinI2cPlatformDmaCompletionV1State;

typedef struct RinI2cPlatformDmaCompletionV1Event {
    uint32_t struct_size;
    uint32_t version;
    uint64_t device_generation;
    uint64_t transfer_generation;
    uint64_t dma_cookie;
    uint32_t status;
    uint32_t reserved0;
    uint64_t bytes;
    uint64_t reserved[2];
} RinI2cPlatformDmaCompletionV1Event;

#define RIN_I2C_HID_TOUCHPAD_DIAGNOSTIC_SNAPSHOT_V1_VERSION \
    UINT32_C(0x00010000)
#define RIN_I2C_PLATFORM_DMA_COMPLETION_V1_VERSION UINT32_C(0x00010000)
#define RIN_I2C_HID_TOUCHPAD_DIAGNOSTIC_FLAG_CONTROLLER_VALID \
    UINT32_C(0x01)
#define RIN_I2C_HID_TOUCHPAD_DIAGNOSTIC_FLAG_DESCRIPTOR_VALID \
    UINT32_C(0x02)
#define RIN_I2C_HID_TOUCHPAD_DIAGNOSTIC_FLAG_LAYOUT_VALID \
    UINT32_C(0x04)
#define RIN_I2C_HID_TOUCHPAD_DIAGNOSTIC_FLAG_DMA_BOUND UINT32_C(0x08)
#define RIN_I2C_HID_TOUCHPAD_DIAGNOSTIC_FLAG_GESTURE_ENABLED \
    UINT32_C(0x10)
#define RIN_I2C_HID_TOUCHPAD_DIAGNOSTIC_FLAG_KNOWN \
    (RIN_I2C_HID_TOUCHPAD_DIAGNOSTIC_FLAG_CONTROLLER_VALID | \
     RIN_I2C_HID_TOUCHPAD_DIAGNOSTIC_FLAG_DESCRIPTOR_VALID | \
     RIN_I2C_HID_TOUCHPAD_DIAGNOSTIC_FLAG_LAYOUT_VALID | \
     RIN_I2C_HID_TOUCHPAD_DIAGNOSTIC_FLAG_DMA_BOUND | \
     RIN_I2C_HID_TOUCHPAD_DIAGNOSTIC_FLAG_GESTURE_ENABLED)

typedef enum RinI2cHidTouchpadDriverV1State {
    RIN_I2C_HID_TOUCHPAD_DRIVER_V1_STATE_INACTIVE = 0,
    RIN_I2C_HID_TOUCHPAD_DRIVER_V1_STATE_ACTIVE = 1,
    RIN_I2C_HID_TOUCHPAD_DRIVER_V1_STATE_SUSPENDED = 2,
    RIN_I2C_HID_TOUCHPAD_DRIVER_V1_STATE_LOST = 3,
    RIN_I2C_HID_TOUCHPAD_DRIVER_V1_STATE_DETACHED = 4
} RinI2cHidTouchpadDriverV1State;

typedef struct RinI2cHidTouchpadDiagnosticSnapshotV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t driver_state;
    uint32_t flags;
    uint64_t controller_cookie;
    uint64_t touchpad_device_id;
    uint64_t device_generation;
    uint32_t controller_profile;
    uint32_t controller_state;
    uint32_t controller_last_error;
    uint32_t controller_last_abort_source;
    uint64_t transfer_generation;
    uint64_t transfer_count;
    uint64_t byte_count;
    uint32_t report_count;
    uint32_t valid_report_count;
    uint32_t report_descriptor_length;
    uint32_t max_input_length;
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t version_id;
    uint16_t reserved0;
    uint64_t report_sequence;
    uint32_t gpio_gsi;
    uint32_t dma_state;
    uint32_t dma_active;
    uint32_t dma_pending;
    uint64_t reserved[2];
} RinI2cHidTouchpadDiagnosticSnapshotV1;

#if defined(__cplusplus)
static_assert(sizeof(RinI2cControllerV1Snapshot) == 88u,
              "I2C controller diagnostic ABI drift");
static_assert(sizeof(RinI2cHidTouchpadDiagnosticSnapshotV1) == 144u,
              "I2C-HID touchpad diagnostic ABI drift");
static_assert(sizeof(RinI2cPlatformDmaCompletionV1Event) == 64u,
              "I2C DMA completion event ABI drift");
#else
_Static_assert(sizeof(RinI2cControllerV1Snapshot) == 88u,
               "I2C controller diagnostic ABI drift");
_Static_assert(sizeof(RinI2cHidTouchpadDiagnosticSnapshotV1) == 144u,
               "I2C-HID touchpad diagnostic ABI drift");
_Static_assert(sizeof(RinI2cPlatformDmaCompletionV1Event) == 64u,
               "I2C DMA completion event ABI drift");
#endif

#endif /* RIN_DRIVER_BUS_I2C_DIAGNOSTICS_V1_H */
