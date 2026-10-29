/* SPDX-License-Identifier: MIT */
#ifndef RIN_SYSTEM_INFO_SNAPSHOT_V1_H
#define RIN_SYSTEM_INFO_SNAPSHOT_V1_H

#include <stdint.h>

#define RIN_SYSTEM_INFO_SNAPSHOT_VERSION_V1 UINT32_C(1)
#define RIN_SYSTEM_INFO_SNAPSHOT_LAYOUT_RIN_V1 UINT32_C(1)
#define RIN_SYSTEM_INFO_SNAPSHOT_LAYOUT_LINUX_SYSINFO UINT32_C(2)

#define RIN_SYSTEM_INFO_SNAPSHOT_UPTIME_VALID UINT32_C(0x00000001)
#define RIN_SYSTEM_INFO_SNAPSHOT_MEMORY_VALID UINT32_C(0x00000002)
#define RIN_SYSTEM_INFO_SNAPSHOT_PROCESS_COUNT_VALID UINT32_C(0x00000004)
#define RIN_SYSTEM_INFO_SNAPSHOT_VALID_MASK UINT32_C(0x00000007)
#define RIN_SYSTEM_INFO_SNAPSHOT_PAGE_SIZE UINT32_C(4096)

/* Fixed-width RinOS snapshot, identical in IA-32, x86_64 native and compat32. */
typedef struct RinSystemInfoSnapshotV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t uptime_seconds;
    uint64_t total_ram_bytes;
    uint64_t available_ram_bytes;
    uint32_t process_count;
    uint32_t valid_metrics;
    uint64_t reserved;
} RinSystemInfoSnapshotV1;

/* Linux sysinfo wire layouts used by the layout selector above.  Unsupported
 * Linux metrics (loads, shared/buffer/swap/high memory) are always zero. */
typedef struct RinLinuxSysinfo32V1 {
    int32_t uptime;
    uint32_t loads[3];
    uint32_t totalram;
    uint32_t freeram;
    uint32_t sharedram;
    uint32_t bufferram;
    uint32_t totalswap;
    uint32_t freeswap;
    uint16_t procs;
    uint16_t pad;
    uint32_t totalhigh;
    uint32_t freehigh;
    uint32_t mem_unit;
    uint8_t reserved[8];
} RinLinuxSysinfo32V1;

typedef struct RinLinuxSysinfo64V1 {
    int64_t uptime;
    uint64_t loads[3];
    uint64_t totalram;
    uint64_t freeram;
    uint64_t sharedram;
    uint64_t bufferram;
    uint64_t totalswap;
    uint64_t freeswap;
    uint16_t procs;
    uint16_t pad;
    uint64_t totalhigh;
    uint64_t freehigh;
    uint32_t mem_unit;
    uint32_t reserved_tail;
} RinLinuxSysinfo64V1;

#if defined(__cplusplus)
static_assert(sizeof(RinSystemInfoSnapshotV1) == 48,
              "RinSystemInfoSnapshotV1 ABI drift");
static_assert(sizeof(RinLinuxSysinfo32V1) == 64,
              "RinLinuxSysinfo32V1 ABI drift");
static_assert(sizeof(RinLinuxSysinfo64V1) == 112,
              "RinLinuxSysinfo64V1 ABI drift");
#else
_Static_assert(sizeof(RinSystemInfoSnapshotV1) == 48,
               "RinSystemInfoSnapshotV1 ABI drift");
_Static_assert(sizeof(RinLinuxSysinfo32V1) == 64,
               "RinLinuxSysinfo32V1 ABI drift");
_Static_assert(sizeof(RinLinuxSysinfo64V1) == 112,
               "RinLinuxSysinfo64V1 ABI drift");
#endif

#endif /* RIN_SYSTEM_INFO_SNAPSHOT_V1_H */
