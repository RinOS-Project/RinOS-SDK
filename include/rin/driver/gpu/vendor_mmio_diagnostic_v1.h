/* SPDX-License-Identifier: MIT */
/* Pointer-free GPU vendor-MMIO diagnostic projection. */
#ifndef RIN_DRIVER_GPU_VENDOR_MMIO_DIAGNOSTIC_V1_H
#define RIN_DRIVER_GPU_VENDOR_MMIO_DIAGNOSTIC_V1_H

#include <stdint.h>

#define RIN_GPU_VENDOR_MMIO_V1_VERSION UINT32_C(0x00010000)
#define RIN_GPU_VENDOR_MMIO_V1_FORCEWAKE_DOMAIN_COUNT 32u
#define RIN_GPU_VENDOR_MMIO_V1_MIN_LENGTH UINT64_C(0x1000)
#define RIN_GPU_VENDOR_MMIO_V1_MAX_LENGTH UINT64_C(0x1000000)
#define RIN_GPU_VENDOR_MMIO_V1_MAX_FORCEWAKE_DEPTH 8u

typedef enum RinGpuVendorMmioV1State {
    RIN_GPU_VENDOR_MMIO_V1_STATE_NONE = 0,
    RIN_GPU_VENDOR_MMIO_V1_STATE_ACTIVE = 1,
    RIN_GPU_VENDOR_MMIO_V1_STATE_LOST = 2,
    RIN_GPU_VENDOR_MMIO_V1_STATE_DETACHED = 3
} RinGpuVendorMmioV1State;

typedef enum RinGpuVendorMmioV1Result {
    RIN_GPU_VENDOR_MMIO_V1_OK = 0,
    RIN_GPU_VENDOR_MMIO_V1_INVALID = -1,
    RIN_GPU_VENDOR_MMIO_V1_BUSY = -2,
    RIN_GPU_VENDOR_MMIO_V1_RANGE = -3,
    RIN_GPU_VENDOR_MMIO_V1_STALE = -4,
    RIN_GPU_VENDOR_MMIO_V1_CALLBACK = -5,
    RIN_GPU_VENDOR_MMIO_V1_STATE = -6,
    RIN_GPU_VENDOR_MMIO_V1_BACKEND = -7
} RinGpuVendorMmioV1Result;

/* Callback contexts and register values never enter this diagnostic record.
 * Lease cookies are opaque observations and do not grant the named access. */
typedef struct RinGpuVendorMmioV1SnapshotV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t state;
    uint32_t family;
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_device_id;
    uint8_t minimum_revision;
    uint8_t maximum_revision;
    uint8_t reserved0[2];
    uint64_t device_generation;
    uint64_t bar_lease;
    uint64_t irq_lease;
    uint64_t dma_lease;
    uint64_t mmio_length;
    uint32_t forcewake_depth;
    uint32_t forcewake_domain_depth[
        RIN_GPU_VENDOR_MMIO_V1_FORCEWAKE_DOMAIN_COUNT];
    int32_t last_error;
    uint32_t operation_count;
    uint32_t reserved1;
    uint64_t reserved[2];
} RinGpuVendorMmioV1SnapshotV1;

#if defined(__cplusplus)
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(RinGpuVendorMmioV1SnapshotV1) == 228u,
              "GPU vendor MMIO diagnostic snapshot ABI drift");
#else
static_assert(sizeof(RinGpuVendorMmioV1SnapshotV1) == 232u,
              "GPU vendor MMIO diagnostic snapshot ABI drift");
#endif
#else
#if UINTPTR_MAX == UINT32_MAX
_Static_assert(sizeof(RinGpuVendorMmioV1SnapshotV1) == 228u,
               "GPU vendor MMIO diagnostic snapshot ABI drift");
#else
_Static_assert(sizeof(RinGpuVendorMmioV1SnapshotV1) == 232u,
               "GPU vendor MMIO diagnostic snapshot ABI drift");
#endif
#endif
#endif /* RIN_DRIVER_GPU_VENDOR_MMIO_DIAGNOSTIC_V1_H */
