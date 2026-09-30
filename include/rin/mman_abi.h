/* SPDX-License-Identifier: MIT */
/* Versioned compat-safe memory mapping request records. */

#ifndef RIN_SDK_RIN_MMAN_ABI_H
#define RIN_SDK_RIN_MMAN_ABI_H

#include <stdint.h>

#define RIN_MMAP_FD_CALL_VERSION 1u

typedef struct RinMmapFdCallV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint64_t address;
    uint64_t size;
    uint32_t protection;
    uint32_t flags;
    int32_t descriptor;
    uint32_t reserved1;
    uint64_t offset;
} RinMmapFdCallV1;

#if defined(__cplusplus) && !defined(MIDL_PASS)
static_assert(sizeof(RinMmapFdCallV1) == 48u,
              "RinMmapFdCallV1 ABI size");
#elif !defined(MIDL_PASS)
_Static_assert(sizeof(RinMmapFdCallV1) == 48u,
               "RinMmapFdCallV1 ABI size");
#endif

#endif /* RIN_SDK_RIN_MMAN_ABI_H */
