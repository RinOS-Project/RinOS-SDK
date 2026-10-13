/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_RIN_DYNLINK_UNWIND_ABI_H
#define RIN_SDK_RIN_DYNLINK_UNWIND_ABI_H

#include <stdint.h>

/* The native RIN image has no ELF program-header table.  This record is the
 * separate, generation-bound publication of the authenticated UNWIND section
 * for one member of the process image graph. */
#define RIN_DLINVENTORY_UNWIND_V1_ABI_VERSION 1u
#define RIN_DLINVENTORY_UNWIND_V1_FLAG_PRESENT 0x00000001u
#define RIN_DLINVENTORY_UNWIND_V1_FLAG_DWARF_DEBUG_FRAME 0x00000002u
#define RIN_DLINVENTORY_UNWIND_V1_KNOWN_FLAGS \
    (RIN_DLINVENTORY_UNWIND_V1_FLAG_PRESENT | \
     RIN_DLINVENTORY_UNWIND_V1_FLAG_DWARF_DEBUG_FRAME)

typedef struct RinDlInventoryUnwindRecordV1 {
    uint32_t abi_version;
    uint32_t flags;
    uint32_t record_index;
    uint32_t record_count;
    uint64_t launch_generation;
    uint64_t load_address;
    uint64_t mapped_size;
    uint64_t unwind_address;
    uint64_t unwind_size;
    uint64_t reserved[1];
} RinDlInventoryUnwindRecordV1;

#ifdef __cplusplus
static_assert(sizeof(RinDlInventoryUnwindRecordV1) == 64u,
              "RinDlInventoryUnwindRecordV1 ABI size");
#else
_Static_assert(sizeof(RinDlInventoryUnwindRecordV1) == 64u,
               "RinDlInventoryUnwindRecordV1 ABI size");
#endif

#endif /* RIN_SDK_RIN_DYNLINK_UNWIND_ABI_H */
