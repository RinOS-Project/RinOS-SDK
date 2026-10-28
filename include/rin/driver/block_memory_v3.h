/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Versioned lease callbacks for publishing driver-owned transfer memory. */

#ifndef RIN_DRIVER_BLOCK_MEMORY_V3_H
#define RIN_DRIVER_BLOCK_MEMORY_V3_H

#include <stdint.h>

#define RIN_DRIVER_BLOCK_MEMORY_V3_VERSION UINT32_C(0x00010000)

/* OS-Core validates each published range and returns an opaque memory lease.
 * This contract does not expose physical addresses or grant DMA by itself. */
typedef struct RinDriverBlockMemoryV3 {
    uint32_t struct_size;
    uint32_t version;
    int (*publish)(void* context, const void* address, uint64_t length,
                   uint32_t dma_flags, uint64_t* memory_lease);
    int (*unpublish)(void* context, uint64_t memory_lease);
} RinDriverBlockMemoryV3;

#if defined(__cplusplus)
static_assert(sizeof(RinDriverBlockMemoryV3) ==
                  (UINTPTR_MAX == UINT32_MAX ? 16u : 24u),
              "RinDriverBlockMemoryV3 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinDriverBlockMemoryV3) ==
                   (UINTPTR_MAX == UINT32_MAX ? 16u : 24u),
               "RinDriverBlockMemoryV3 ABI drift");
#endif

#endif /* RIN_DRIVER_BLOCK_MEMORY_V3_H */
