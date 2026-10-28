/* SPDX-License-Identifier: MIT */
/* Pointer-free GPU firmware provenance; presence does not grant authority. */

#ifndef RIN_DRIVER_GPU_FIRMWARE_IDENTITY_V1_H
#define RIN_DRIVER_GPU_FIRMWARE_IDENTITY_V1_H

#include <stdint.h>

#define RIN_GPU_FIRMWARE_V1_MAX_IMAGE_BYTES UINT32_C(4194304)
#define RIN_GPU_FIRMWARE_V1_SHA256_BYTES UINT32_C(32)
#define RIN_GPU_FIRMWARE_IDENTITY_V1_VERSION UINT32_C(0x00010000)
#define RIN_GPU_FIRMWARE_IDENTITY_FLAG_VERSION_VALID UINT32_C(1)
#define RIN_GPU_FIRMWARE_IDENTITY_FLAG_DIGEST_VALID UINT32_C(2)
#define RIN_GPU_FIRMWARE_IDENTITY_FLAG_AUTHENTICATED UINT32_C(4)
#define RIN_GPU_FIRMWARE_IDENTITY_FLAG_REQUIRED \
    (RIN_GPU_FIRMWARE_IDENTITY_FLAG_VERSION_VALID | \
     RIN_GPU_FIRMWARE_IDENTITY_FLAG_DIGEST_VALID | \
     RIN_GPU_FIRMWARE_IDENTITY_FLAG_AUTHENTICATED)

typedef enum RinGpuFirmwareFamilyV1 {
    RIN_GPU_FIRMWARE_FAMILY_NONE = 0,
    RIN_GPU_FIRMWARE_FAMILY_INTEL_XE_LPG = 1,
    RIN_GPU_FIRMWARE_FAMILY_AMD_RDNA3 = 2,
    RIN_GPU_FIRMWARE_FAMILY_NVIDIA_ADA = 3
} RinGpuFirmwareFamilyV1;

/* Pointer-free provenance published only after signed firmware admission. */
typedef struct RinGpuFirmwareIdentityV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    uint32_t family;
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_device_id;
    uint64_t device_cookie;
    uint64_t device_generation;
    uint64_t firmware_version;
    uint64_t firmware_generation;
    uint32_t image_size;
    uint32_t reserved0;
    uint8_t content_hash[RIN_GPU_FIRMWARE_V1_SHA256_BYTES];
    uint8_t trusted_key_id[RIN_GPU_FIRMWARE_V1_SHA256_BYTES];
    uint64_t reserved[2];
} RinGpuFirmwareIdentityV1;

#if defined(__cplusplus)
static_assert(sizeof(RinGpuFirmwareIdentityV1) == 144u,
              "GPU firmware identity ABI drift");
#else
_Static_assert(sizeof(RinGpuFirmwareIdentityV1) == 144u,
               "GPU firmware identity ABI drift");
#endif

#endif /* RIN_DRIVER_GPU_FIRMWARE_IDENTITY_V1_H */
