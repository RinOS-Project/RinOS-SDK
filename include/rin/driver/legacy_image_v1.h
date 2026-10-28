/* SPDX-License-Identifier: MIT */
/* Signed, page-separated NDRV v1.1 compatibility image format and validator. */

#ifndef RIN_DRIVER_LEGACY_IMAGE_V1_H
#define RIN_DRIVER_LEGACY_IMAGE_V1_H

#include <stdint.h>

#define RIN_LEGACY_DRIVER_V1_MAGIC UINT32_C(0x5652444e) /* "NDRV" */
#define RIN_LEGACY_DRIVER_V1_VERSION UINT16_C(0x0101)
#define RIN_LEGACY_DRIVER_V1_HEADER_SIZE UINT32_C(96)
#define RIN_LEGACY_DRIVER_V1_FLAG_SIGNED UINT32_C(0x00000001)
#define RIN_LEGACY_DRIVER_V1_MAX_IMAGE_SIZE UINT32_C(0x01000000)
#define RIN_LEGACY_DRIVER_V1_PAGE_SIZE UINT32_C(4096)

#define RIN_LEGACY_DRIVER_V1_RELOC_ABS32 UINT16_C(1)
#define RIN_LEGACY_DRIVER_V1_RELOC_REL32 UINT16_C(2)

typedef struct __attribute__((packed)) RinLegacyDriverV1Header {
    uint32_t magic;
    uint16_t version;
    uint16_t driver_type;
    uint32_t code_offset;
    uint32_t code_size;
    uint32_t data_offset;
    uint32_t data_size;
    uint32_t relocation_offset;
    uint32_t relocation_count;
    uint32_t entry_offset;
    uint32_t exit_offset;
    char name[16];
    uint32_t api_offset;
    uint32_t prefix_reserved;
    uint32_t header_size;
    uint32_t flags;
    uint32_t rodata_offset;
    uint32_t rodata_size;
    uint32_t image_size;
    uint32_t signature_offset;
    uint32_t signature_size;
    uint32_t reserved;
} RinLegacyDriverV1Header;

typedef struct __attribute__((packed)) RinLegacyDriverV1Relocation {
    uint32_t offset;
    uint16_t type;
    uint16_t reserved;
} RinLegacyDriverV1Relocation;

typedef struct RinLegacyDriverV1ImageView {
    const RinLegacyDriverV1Header* header;
    const RinLegacyDriverV1Relocation* relocations;
    uint32_t code_runtime_offset;
    uint32_t rodata_runtime_offset;
    uint32_t data_runtime_offset;
} RinLegacyDriverV1ImageView;

typedef enum RinLegacyDriverV1ImageStatus {
    RIN_LEGACY_DRIVER_V1_IMAGE_OK = 0,
    RIN_LEGACY_DRIVER_V1_IMAGE_INVALID_ARGUMENT = -1,
    RIN_LEGACY_DRIVER_V1_IMAGE_BAD_HEADER = -2,
    RIN_LEGACY_DRIVER_V1_IMAGE_BAD_LAYOUT = -3,
    RIN_LEGACY_DRIVER_V1_IMAGE_BAD_RELOCATION = -4
} RinLegacyDriverV1ImageStatus;

#if defined(__cplusplus)
extern "C" {
#endif

RinLegacyDriverV1ImageStatus rin_legacy_driver_v1_image_validate(
    const void* image, uint32_t image_size,
    RinLegacyDriverV1ImageView* view);

#if defined(__cplusplus)
}
#endif

#if defined(__cplusplus)
static_assert(sizeof(RinLegacyDriverV1Header) ==
                  RIN_LEGACY_DRIVER_V1_HEADER_SIZE,
              "NDRV v1.1 header ABI drift");
static_assert(sizeof(RinLegacyDriverV1Relocation) == 8u,
              "NDRV v1.1 relocation ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinLegacyDriverV1Header) ==
                   RIN_LEGACY_DRIVER_V1_HEADER_SIZE,
               "NDRV v1.1 header ABI drift");
_Static_assert(sizeof(RinLegacyDriverV1Relocation) == 8u,
               "NDRV v1.1 relocation ABI drift");
#endif

#endif /* RIN_DRIVER_LEGACY_IMAGE_V1_H */
