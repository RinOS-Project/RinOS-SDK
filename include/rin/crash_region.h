/* SPDX-License-Identifier: MIT */

#ifndef RIN_API_RIN_CRASH_REGION_H
#define RIN_API_RIN_CRASH_REGION_H

#include <stdint.h>

#define RIN_CRASH_REGION_MAGIC UINT32_C(0x31444352) /* "RCD1" */
#define RIN_CRASH_REGION_VERSION UINT16_C(4)
#define RIN_CRASH_REGION_V3_VERSION UINT16_C(3)
#define RIN_CRASH_REGION_V2_VERSION UINT16_C(2)
#define RIN_CRASH_REGION_LEGACY_VERSION UINT16_C(1)
#define RIN_CRASH_REGION_STATE_ACTIVE UINT32_C(1)
#define RIN_CRASH_REGION_DESCRIPTOR_SIZE UINT32_C(4096)
#define RIN_CRASH_REGION_LOGICAL_BLOCK_SIZE UINT32_C(512)
#define RIN_CRASH_REGION_RIT1_LAST_LBA UINT64_C(49)
#define RIN_CRASH_REGION_LEGACY_FIRST_LBA UINT64_C(256)
#define RIN_CRASH_REGION_LEGACY_SECTOR_COUNT \
    (RIN_CRASH_REGION_DESCRIPTOR_SECTORS + \
     RIN_CRASH_REGION_LEGACY_SLOT_SECTORS * \
         RIN_CRASH_REGION_LEGACY_SLOT_COUNT)
#define RIN_CRASH_REGION_FIRST_LBA UINT64_C(1280)
#define RIN_CRASH_REGION_FIRST_PARTITION_LBA UINT64_C(2048)
#define RIN_CRASH_REGION_DESCRIPTOR_SECTORS UINT64_C(8)
#define RIN_CRASH_REGION_V2_SLOT_SECTORS UINT64_C(128)
#define RIN_CRASH_REGION_V2_SLOT_SIZE UINT32_C(65536)
#define RIN_CRASH_REGION_V2_SLOT_COUNT UINT32_C(5)
#define RIN_CRASH_REGION_V2_SECTOR_COUNT \
    (RIN_CRASH_REGION_DESCRIPTOR_SECTORS + \
     RIN_CRASH_REGION_V2_SLOT_SECTORS * RIN_CRASH_REGION_V2_SLOT_COUNT)
#define RIN_CRASH_REGION_V3_SLOT_COUNT UINT32_C(20)
#define RIN_CRASH_REGION_V3_SECTOR_COUNT \
    (RIN_CRASH_REGION_DESCRIPTOR_SECTORS + \
     RIN_CRASH_REGION_SLOT_SECTORS * RIN_CRASH_REGION_V3_SLOT_COUNT)
#define RIN_CRASH_REGION_SLOT_SECTORS UINT64_C(32)
#define RIN_CRASH_REGION_SLOT_SIZE UINT32_C(16384)
#define RIN_CRASH_REGION_SLOT_COUNT UINT32_C(23)
#define RIN_CRASH_REGION_MAX_SLOT_SIZE RIN_CRASH_REGION_V2_SLOT_SIZE
#define RIN_CRASH_REGION_LEGACY_SLOT_SECTORS \
    RIN_CRASH_REGION_V2_SLOT_SECTORS
#define RIN_CRASH_REGION_LEGACY_SLOT_SIZE RIN_CRASH_REGION_V2_SLOT_SIZE
#define RIN_CRASH_REGION_LEGACY_SLOT_COUNT UINT32_C(2)
#define RIN_CRASH_REGION_SLOT_A_LBA \
    (RIN_CRASH_REGION_FIRST_LBA + RIN_CRASH_REGION_DESCRIPTOR_SECTORS)
#define RIN_CRASH_REGION_SLOT_B_LBA \
    (RIN_CRASH_REGION_SLOT_A_LBA + RIN_CRASH_REGION_SLOT_SECTORS)
#define RIN_CRASH_REGION_SECTOR_COUNT \
    (RIN_CRASH_REGION_DESCRIPTOR_SECTORS + \
     RIN_CRASH_REGION_SLOT_SECTORS * RIN_CRASH_REGION_SLOT_COUNT)
#define RIN_CRASH_REGION_LEGACY_SLOT_A_LBA \
    (RIN_CRASH_REGION_LEGACY_FIRST_LBA + \
     RIN_CRASH_REGION_DESCRIPTOR_SECTORS)

#if RIN_CRASH_REGION_FIRST_LBA <= RIN_CRASH_REGION_RIT1_LAST_LBA
#error "RCD1 overlaps the RIT1 journal"
#endif
#if RIN_CRASH_REGION_FIRST_LBA + RIN_CRASH_REGION_SECTOR_COUNT > \
    RIN_CRASH_REGION_FIRST_PARTITION_LBA
#error "RCD1 overlaps the canonical first GPT partition"
#endif
#if RIN_CRASH_REGION_LEGACY_FIRST_LBA + \
        RIN_CRASH_REGION_LEGACY_SECTOR_COUNT > UINT64_C(640)
#error "legacy RCD1 overlaps the canonical System A descriptor copy"
#endif

typedef enum RinCrashRegionResult {
    RIN_CRASH_REGION_OK = 0,
    RIN_CRASH_REGION_INVALID = -1,
    RIN_CRASH_REGION_CORRUPT = -2,
    RIN_CRASH_REGION_MISMATCH = -3,
    RIN_CRASH_REGION_RANGE = -4
} RinCrashRegionResult;

typedef struct RinCrashRegionBinding {
    uint64_t block_count;
    uint32_t logical_block_size;
    uint32_t reserved;
    uint8_t disk_guid[16];
    uint8_t install_nonce[16];
} RinCrashRegionBinding;

typedef struct RinCrashRegionDescriptorV1 {
    uint32_t magic;
    uint16_t version;
    uint16_t header_size;
    uint32_t total_size;
    uint32_t state;
    uint64_t block_count;
    uint64_t region_first_lba;
    uint64_t region_sector_count;
    uint32_t logical_block_size;
    uint32_t slot_size;
    uint32_t slot_count;
    uint32_t reserved0;
    uint8_t disk_guid[16];
    uint8_t install_nonce[16];
    uint32_t crc32;
    uint32_t reserved1;
} RinCrashRegionDescriptorV1;

typedef struct RinCrashRegionPartitionRange {
    uint64_t first_lba;
    uint64_t last_lba;
} RinCrashRegionPartitionRange;

typedef struct RinCrashRegionPanicLayout {
    uint64_t byte_offset;
    uint64_t byte_size;
    uint32_t slot_size;
    uint32_t slot_count;
    uint64_t sequence_floor;
    uint32_t legacy_history_visible;
    uint32_t reserved;
} RinCrashRegionPanicLayout;

int rin_crash_region_binding_valid(const RinCrashRegionBinding* binding);
int rin_crash_region_binding_matches(const RinCrashRegionBinding* expected,
                                     const RinCrashRegionBinding* observed);
RinCrashRegionResult rin_crash_region_descriptor_encode(
    const RinCrashRegionBinding* binding,
    uint8_t output[RIN_CRASH_REGION_DESCRIPTOR_SIZE]);
RinCrashRegionResult rin_crash_region_descriptor_decode(
    const uint8_t input[RIN_CRASH_REGION_DESCRIPTOR_SIZE],
    RinCrashRegionBinding* binding);
/* Also returns the exact bounded slot layout advertised by the descriptor.
 * V1's two-slot, v2's five-slot, and v3's twenty-slot layouts remain readable;
 * newly encoded descriptors use the twenty-three-slot v4 layout. */
RinCrashRegionResult rin_crash_region_descriptor_decode_layout(
    const uint8_t input[RIN_CRASH_REGION_DESCRIPTOR_SIZE],
    RinCrashRegionBinding* binding, RinCrashRegionPanicLayout* layout);
RinCrashRegionResult rin_crash_region_validate_partition_ranges(
    const RinCrashRegionBinding* binding,
    const RinCrashRegionPartitionRange* ranges, uint32_t range_count);
RinCrashRegionResult rin_crash_region_panic_layout(
    const RinCrashRegionBinding* binding, RinCrashRegionPanicLayout* layout);
RinCrashRegionResult rin_crash_region_panic_layout_for_slots(
    const RinCrashRegionBinding* binding, uint64_t region_first_lba,
    uint32_t slot_size, uint32_t slot_count,
    RinCrashRegionPanicLayout* layout);

#if defined(__cplusplus)
static_assert(sizeof(RinCrashRegionBinding) == 48u,
              "RinCrashRegionBinding ABI drift");
static_assert(sizeof(RinCrashRegionDescriptorV1) == 96u,
              "RinCrashRegionDescriptorV1 ABI drift");
static_assert(sizeof(RinCrashRegionPanicLayout) == 40u,
              "RinCrashRegionPanicLayout ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinCrashRegionBinding) == 48u,
               "RinCrashRegionBinding ABI drift");
_Static_assert(sizeof(RinCrashRegionDescriptorV1) == 96u,
               "RinCrashRegionDescriptorV1 ABI drift");
_Static_assert(sizeof(RinCrashRegionPanicLayout) == 40u,
               "RinCrashRegionPanicLayout ABI drift");
#endif

#endif /* RIN_API_RIN_CRASH_REGION_H */
