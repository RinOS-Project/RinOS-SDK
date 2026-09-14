/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_RIN_DYNLINK_INVENTORY_ABI_H
#define RIN_SDK_RIN_DYNLINK_INVENTORY_ABI_H

#include <stdint.h>

#define RIN_DLINVENTORY_V1_ABI_VERSION 1u
#define RIN_DLINVENTORY_V1_NAME_CAPACITY 64u
#define RIN_DLINVENTORY_V1_MAX_LAUNCH_RECORDS 65u
/* A native process may retain the immutable launch catalog plus one entry
 * for each library in every attached runtime graph.  Keep the legacy launch
 * bound above for RLL32 callers, but allow the native aggregate snapshot to
 * cover all runtime catalog slots (8 * 64 libraries + the main image). */
#define RIN_DLINVENTORY_V1_MAX_RECORDS 513u
#define RIN_DLINVENTORY_V1_FLAG_MAIN_IMAGE 0x00000001u
/* Legacy RLL32 has no ELF main image.  This flag makes an inventory a
 * complete snapshot of live RLL images without manufacturing a main image
 * record at index zero. */
#define RIN_DLINVENTORY_V1_FLAG_LEGACY_RLL 0x00000002u

/* A single immutable member of the process launch graph.  RIN v3 images are
 * not ELF objects, so program_headers/program_header_count and TLS metadata
 * are zero.  The ABI makes that absence explicit instead of manufacturing
 * loader addresses for dl_iterate_phdr(). */
typedef struct RinDlInventoryRecordV1 {
    uint64_t launch_generation;
    uint64_t load_address;
    uint64_t mapped_size;
    uint64_t program_headers;
    uint64_t tls_data;
    uint64_t add_count;
    uint64_t sub_count;
    uint32_t abi_version;
    uint32_t record_count;
    uint32_t record_index;
    uint32_t flags;
    uint32_t program_header_count;
    uint32_t tls_module_id;
    uint32_t name_length;
    uint32_t reserved0;
    char name[RIN_DLINVENTORY_V1_NAME_CAPACITY];
} RinDlInventoryRecordV1;

#ifdef __cplusplus
static_assert(sizeof(RinDlInventoryRecordV1) == 152u,
              "RinDlInventoryRecordV1 ABI size");
#else
_Static_assert(sizeof(RinDlInventoryRecordV1) == 152u,
               "RinDlInventoryRecordV1 ABI size");
#endif

#endif /* RIN_SDK_RIN_DYNLINK_INVENTORY_ABI_H */
