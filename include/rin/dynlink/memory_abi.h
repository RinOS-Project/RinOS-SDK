/* SPDX-License-Identifier: MIT */
/* Versioned legacy RLL caller-memory request ABI. */
#ifndef RIN_SDK_DYNLINK_MEMORY_ABI_H
#define RIN_SDK_DYNLINK_MEMORY_ABI_H

#include <stddef.h>
#include <stdint.h>

#define RIN_DYNLINK_OPEN_MEMORY_V1_VERSION 1u
#define RIN_DYNLINK_OPEN_MEMORY_V1_NAME_BYTES 64u
#define RIN_DYNLINK_OPEN_MEMORY_V1_MAX_IMAGE_BYTES UINT64_C(67108864)

typedef struct RinDynlinkOpenMemoryV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags; /* must be zero */
    uint64_t image_address; /* caller address in a legacy 32-bit process */
    uint64_t image_size; /* 1..RIN_DYNLINK_OPEN_MEMORY_V1_MAX_IMAGE_BYTES */
    char library_name[RIN_DYNLINK_OPEN_MEMORY_V1_NAME_BYTES];
    uint64_t reserved[2]; /* must be zero */
} RinDynlinkOpenMemoryV1;

#if defined(__cplusplus)
static_assert(sizeof(RinDynlinkOpenMemoryV1) == 104u,
              "RinDynlinkOpenMemoryV1 ABI size drift");
static_assert(offsetof(RinDynlinkOpenMemoryV1, image_address) == 8u,
              "RinDynlinkOpenMemoryV1.image_address ABI drift");
static_assert(offsetof(RinDynlinkOpenMemoryV1, library_name) == 24u,
              "RinDynlinkOpenMemoryV1.library_name ABI drift");
#else
_Static_assert(sizeof(RinDynlinkOpenMemoryV1) == 104u,
               "RinDynlinkOpenMemoryV1 ABI size drift");
_Static_assert(offsetof(RinDynlinkOpenMemoryV1, image_address) == 8u,
               "RinDynlinkOpenMemoryV1.image_address ABI drift");
_Static_assert(offsetof(RinDynlinkOpenMemoryV1, library_name) == 24u,
               "RinDynlinkOpenMemoryV1.library_name ABI drift");
#endif

#endif /* RIN_SDK_DYNLINK_MEMORY_ABI_H */
