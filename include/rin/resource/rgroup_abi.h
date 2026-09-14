/* SPDX-License-Identifier: MIT */
/* Stable user/kernel records for the native RinOS resource-group ABI. */

#ifndef RIN_SDK_RIN_RGROUP_ABI_H
#define RIN_SDK_RIN_RGROUP_ABI_H

#include <stdint.h>

#define RIN_RGROUP_OWNER_VERSION UINT16_C(1)
#define RIN_RGROUP_ABI_VERSION UINT16_C(1)

/* Every physical allocation has one stable allocation ID. A shared mapping
 * is charged to its first owner; a COW replacement must use a new ID. */
#define RIN_RGROUP_MEMORY_KIND_ANONYMOUS UINT16_C(0x0001)
#define RIN_RGROUP_MEMORY_KIND_MAPPED UINT16_C(0x0002)
#define RIN_RGROUP_MEMORY_FLAG_SHARED UINT16_C(0x0004)
#define RIN_RGROUP_MEMORY_FLAG_COW UINT16_C(0x0008)
#define RIN_RGROUP_MEMORY_KIND_MASK UINT16_C(0x0003)
#define RIN_RGROUP_IO_FLAG_READ UINT16_C(0x0001)
#define RIN_RGROUP_IO_FLAG_WRITE UINT16_C(0x0002)
/* Control operations carry no data bytes but still consume one I/O policy
 * operation and remain pending until the device callback reaches a terminal
 * result.  Keeping these in the existing flags word preserves the fixed
 * request ABI while making flush/reset accounting explicit. */
#define RIN_RGROUP_IO_FLAG_FLUSH UINT16_C(0x0004)
#define RIN_RGROUP_IO_FLAG_RESET UINT16_C(0x0008)

typedef uint64_t RinRgroupHandle;

typedef enum RinRgroupStatus {
    RIN_RGROUP_OK = 0,
    RIN_RGROUP_INVALID_ARGUMENT = -1,
    RIN_RGROUP_DENIED = -2,
    RIN_RGROUP_STALE_HANDLE = -3,
    RIN_RGROUP_CAPACITY = -4,
    RIN_RGROUP_BUSY = -5,
    RIN_RGROUP_IDENTITY_MISMATCH = -6,
    RIN_RGROUP_NOT_FOUND = -7,
    RIN_RGROUP_TIME_REGRESSION = -8,
    RIN_RGROUP_THROTTLED = -9,
    RIN_RGROUP_OVERFLOW = -10,
    RIN_RGROUP_MEMORY_LIMIT = -11,
    RIN_RGROUP_UNDERFLOW = -12
} RinRgroupStatus;

typedef struct RinRgroupCreateRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t owner_uid;
    uint32_t max_processes;
    uint64_t reserved[6];
} RinRgroupCreateRequestV1;

typedef struct RinRgroupInfoV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t group_id;
    RinRgroupHandle handle;
    RinRgroupHandle parent_handle;
    uint32_t owner_uid;
    uint32_t max_processes;
    uint32_t member_count;
    uint32_t child_count;
    uint64_t reserved[4];
} RinRgroupInfoV1;

typedef struct RinRgroupCpuRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t relative_weight;
    uint32_t reserved0;
    uint64_t quota_ticks;
    uint64_t period_ticks;
    uint64_t reserved[4];
} RinRgroupCpuRequestV1;

typedef struct RinRgroupCpuInfoV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    RinRgroupHandle handle;
    uint32_t relative_weight;
    uint32_t reserved0;
    uint64_t quota_ticks;
    uint64_t period_ticks;
    uint64_t period_start_tick;
    uint64_t period_used_ticks;
    uint64_t usage_ticks;
    uint64_t throttled_ticks;
    uint64_t reserved[3];
} RinRgroupCpuInfoV1;

typedef struct RinRgroupMemoryRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t max_bytes;
    uint64_t high_bytes;
    uint64_t reserved[5];
} RinRgroupMemoryRequestV1;

typedef struct RinRgroupMemoryChargeRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t allocation_id;
    uint64_t bytes;
    uint64_t reserved[5];
} RinRgroupMemoryChargeRequestV1;

typedef struct RinRgroupMemoryInfoV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    RinRgroupHandle handle;
    uint64_t current_bytes;
    uint64_t peak_bytes;
    uint64_t max_bytes;
    uint64_t high_bytes;
    uint64_t reclaimed_bytes;
    uint64_t oom_events;
    uint64_t high_events;
    uint64_t reclaim_events;
    uint64_t reserved[2];
} RinRgroupMemoryInfoV1;

typedef struct RinRgroupThreadLimitRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t max_threads;
    uint32_t reserved0;
    uint64_t reserved[6];
} RinRgroupThreadLimitRequestV1;

typedef struct RinRgroupThreadRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t thread_id;
    uint32_t reserved0;
    uint64_t thread_instance_cookie;
    uint64_t reserved[5];
} RinRgroupThreadRequestV1;

typedef struct RinRgroupThreadInfoV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    RinRgroupHandle handle;
    uint32_t max_threads;
    uint32_t thread_count;
    uint64_t reserved[5];
} RinRgroupThreadInfoV1;

typedef struct RinRgroupIoChargeRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t device_id;
    uint64_t operation_id;
    uint64_t bytes;
    uint64_t reserved[4];
} RinRgroupIoChargeRequestV1;

/* Optional block-I/O policy.  A zero limit means unlimited for that
 * dimension; a nonzero limit requires a nonzero period_ticks.  The record is
 * intentionally the same fixed size as every other rgroup payload so a
 * future bandwidth/IOPS policy does not grow the public request ABI. */
typedef struct RinRgroupIoPolicyRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t bandwidth_bytes_per_period;
    uint64_t operations_per_period;
    uint64_t period_ticks;
    uint64_t reserved[4];
} RinRgroupIoPolicyRequestV1;

typedef struct RinRgroupIoInfoV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    RinRgroupHandle handle;
    uint64_t read_bytes;
    uint64_t write_bytes;
    uint64_t read_operations;
    uint64_t write_operations;
    uint64_t pending_operations;
    uint64_t flush_operations;
    uint64_t reset_operations;
    uint64_t reserved[3];
} RinRgroupIoInfoV1;

typedef struct RinRgroupIoPolicyInfoV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    RinRgroupHandle handle;
    uint64_t bandwidth_bytes_per_period;
    uint64_t operations_per_period;
    uint64_t period_ticks;
    uint64_t period_start_tick;
    uint64_t period_bytes;
    uint64_t period_operations;
    uint64_t throttled_operations;
    uint64_t reserved[3];
} RinRgroupIoPolicyInfoV1;

typedef enum RinRgroupAbiOperationV1 {
    RIN_RGROUP_ABI_CREATE = 1,
    RIN_RGROUP_ABI_QUERY = 2,
    RIN_RGROUP_ABI_DESTROY = 3,
    RIN_RGROUP_ABI_ATTACH = 4,
    RIN_RGROUP_ABI_DETACH = 5,
    RIN_RGROUP_ABI_CONFIGURE_CPU = 6,
    RIN_RGROUP_ABI_QUERY_CPU = 7,
    RIN_RGROUP_ABI_CONFIGURE_MEMORY = 8,
    RIN_RGROUP_ABI_QUERY_MEMORY = 9,
    RIN_RGROUP_ABI_CONFIGURE_THREAD_LIMIT = 10,
    RIN_RGROUP_ABI_QUERY_THREAD_LIMIT = 11,
    RIN_RGROUP_ABI_CONFIGURE_IO = 12,
    RIN_RGROUP_ABI_QUERY_IO = 13
} RinRgroupAbiOperationV1;

/* Caller authority is never user supplied. target_process_id and
 * target_instance_cookie are only a requested identity for ATTACH/DETACH;
 * the kernel resolves the live process again and rejects any mismatch. */
typedef struct RinRgroupAbiRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    uint32_t reserved0;
    RinRgroupHandle handle;
    RinRgroupHandle parent_handle;
    union {
        RinRgroupCreateRequestV1 create;
        RinRgroupCpuRequestV1 cpu;
        RinRgroupMemoryRequestV1 memory;
        RinRgroupThreadLimitRequestV1 thread_limit;
        RinRgroupIoPolicyRequestV1 io_policy;
        uint64_t raw[8];
    } payload;
    uint64_t target_process_id;
    uint64_t target_instance_cookie;
    uint64_t reserved[2];
} RinRgroupAbiRequestV1;

typedef struct RinRgroupAbiResponseV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    int32_t status;
    uint32_t flags;
    RinRgroupHandle handle;
    union {
        RinRgroupInfoV1 group;
        RinRgroupCpuInfoV1 cpu;
        RinRgroupMemoryInfoV1 memory;
        RinRgroupThreadInfoV1 thread_limit;
        RinRgroupIoPolicyInfoV1 io_policy;
        uint64_t raw[12];
    } payload;
    uint64_t reserved;
} RinRgroupAbiResponseV1;

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinRgroupCreateRequestV1) == 64u,
               "RinRgroupCreateRequestV1 ABI drift");
_Static_assert(sizeof(RinRgroupInfoV1) == 80u,
               "RinRgroupInfoV1 ABI drift");
_Static_assert(sizeof(RinRgroupCpuRequestV1) == 64u,
               "RinRgroupCpuRequestV1 ABI drift");
_Static_assert(sizeof(RinRgroupCpuInfoV1) == 96u,
               "RinRgroupCpuInfoV1 ABI drift");
_Static_assert(sizeof(RinRgroupMemoryRequestV1) == 64u,
               "RinRgroupMemoryRequestV1 ABI drift");
_Static_assert(sizeof(RinRgroupMemoryChargeRequestV1) == 64u,
               "RinRgroupMemoryChargeRequestV1 ABI drift");
_Static_assert(sizeof(RinRgroupMemoryInfoV1) == 96u,
               "RinRgroupMemoryInfoV1 ABI drift");
_Static_assert(sizeof(RinRgroupThreadLimitRequestV1) == 64u,
               "RinRgroupThreadLimitRequestV1 ABI drift");
_Static_assert(sizeof(RinRgroupThreadRequestV1) == 64u,
               "RinRgroupThreadRequestV1 ABI drift");
_Static_assert(sizeof(RinRgroupThreadInfoV1) == 64u,
               "RinRgroupThreadInfoV1 ABI drift");
_Static_assert(sizeof(RinRgroupIoChargeRequestV1) == 64u,
               "RinRgroupIoChargeRequestV1 ABI drift");
_Static_assert(sizeof(RinRgroupIoPolicyRequestV1) == 64u,
               "RinRgroupIoPolicyRequestV1 ABI drift");
_Static_assert(sizeof(RinRgroupIoInfoV1) == 96u,
               "RinRgroupIoInfoV1 ABI drift");
_Static_assert(sizeof(RinRgroupIoPolicyInfoV1) == 96u,
               "RinRgroupIoPolicyInfoV1 ABI drift");
_Static_assert(sizeof(RinRgroupAbiRequestV1) == 128u,
               "RinRgroupAbiRequestV1 ABI drift");
_Static_assert(sizeof(RinRgroupAbiResponseV1) == 128u,
               "RinRgroupAbiResponseV1 ABI drift");
#endif

#endif /* RIN_SDK_RIN_RGROUP_ABI_H */
