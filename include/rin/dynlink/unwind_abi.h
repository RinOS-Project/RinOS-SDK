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
#define RIN_DLINVENTORY_UNWIND_V1_FLAG_EH_FRAME 0x00000004u
#define RIN_DLINVENTORY_UNWIND_V1_KNOWN_FLAGS \
    (RIN_DLINVENTORY_UNWIND_V1_FLAG_PRESENT | \
     RIN_DLINVENTORY_UNWIND_V1_FLAG_DWARF_DEBUG_FRAME | \
     RIN_DLINVENTORY_UNWIND_V1_FLAG_EH_FRAME)

/* A lease is acquired before a consumer walks any published UNWIND bytes.
 * The kernel holds the process loader exclusion for the lifetime of the
 * lease, so a dynamic graph cannot unmap the bytes between the inventory
 * snapshot and the parser's final read.  The token is opaque to user space;
 * callers must return the exact record to LEASE_END on the same thread. */
#define RIN_DLINVENTORY_UNWIND_LEASE_V1_ABI_VERSION 1u
#define RIN_DLINVENTORY_UNWIND_LEASE_V1_FLAG_EXCLUSIVE_LOADER 0x00000001u
#define RIN_DLINVENTORY_UNWIND_LEASE_V1_KNOWN_FLAGS \
    RIN_DLINVENTORY_UNWIND_LEASE_V1_FLAG_EXCLUSIVE_LOADER

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

typedef struct RinDlInventoryUnwindLeaseV1 {
    uint32_t abi_version;
    uint32_t flags;
    uint64_t token;
    uint64_t launch_generation;
    uint64_t reserved[1];
} RinDlInventoryUnwindLeaseV1;

#ifdef __cplusplus
static_assert(sizeof(RinDlInventoryUnwindLeaseV1) == 32u,
              "RinDlInventoryUnwindLeaseV1 ABI size");
#else
_Static_assert(sizeof(RinDlInventoryUnwindLeaseV1) == 32u,
               "RinDlInventoryUnwindLeaseV1 ABI size");
#endif

#endif /* RIN_SDK_RIN_DYNLINK_UNWIND_ABI_H */
