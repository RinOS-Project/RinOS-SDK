/* SPDX-License-Identifier: MIT */
/* Process-owned GPU memory allocation syscall contract. */
#ifndef RIN_SDK_GPU_H
#define RIN_SDK_GPU_H

#include "abi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_GPU_ALLOCATION_DESC_VERSION_V1 UINT32_C(1)
#define RIN_GPU_ALLOCATION_INFO_VERSION_V1 UINT32_C(1)
#define RIN_GPU_ALLOCATION_STATE_ACTIVE_V1 UINT32_C(1)
#define RIN_GPU_ALLOCATION_MAX_LEASES_V1 UINT32_C(64)

#define RIN_GPU_HEAP_LOCAL UINT32_C(1)
#define RIN_GPU_HEAP_SYSTEM UINT32_C(2)

#define RIN_GPU_ALLOCATION_GPU_READ UINT32_C(0x00000001)
#define RIN_GPU_ALLOCATION_GPU_WRITE UINT32_C(0x00000002)
#define RIN_GPU_ALLOCATION_CPU_VISIBLE UINT32_C(0x00000004)
#define RIN_GPU_ALLOCATION_ZEROED UINT32_C(0x00000008)
#define RIN_GPU_SYNC_CPU_TO_DEVICE UINT32_C(1)
#define RIN_GPU_SYNC_DEVICE_TO_CPU UINT32_C(2)
#define RIN_GPU_MAP_CPU_READ UINT32_C(0x00000001)
#define RIN_GPU_MAP_CPU_WRITE UINT32_C(0x00000002)
#define RIN_GPU_MEMORY_MAX_TRANSFER_BYTES UINT64_C(1048576)
#define RIN_GPU_MEMORY_MAPPING_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_SUBMIT_DESC_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_SUBMIT_RECEIPT_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_QUEUE_TIMELINE_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_MAX_COMMANDS_V1 UINT32_C(1024)
#define RIN_GPU_PROCESS_MAX_RESOURCES_V1 UINT32_C(8)
#define RIN_GPU_PROCESS_MAX_QUEUES_V1 UINT32_C(8)

enum RinGpuSdkOperationV1 {
    RIN_GPU_SDK_MEMORY_ALLOCATE = 1,
    RIN_GPU_SDK_MEMORY_QUERY = 2,
    RIN_GPU_SDK_MEMORY_DESTROY = 3,
    RIN_GPU_SDK_MEMORY_SYNC = 4,
    RIN_GPU_SDK_MEMORY_UPLOAD = 5,
    RIN_GPU_SDK_MEMORY_READBACK = 6,
    RIN_GPU_SDK_MEMORY_MAP = 7,
    RIN_GPU_SDK_MEMORY_UNMAP = 8,
    /* Service-only broker dispatch; the kernel admits this operation only
     * for the exact signed ringpu-capability system service. */
    RIN_GPU_SDK_CAPABILITY_DISPATCH = 9,
    RIN_GPU_SDK_PROCESS_QUEUE_BIND = 10,
    RIN_GPU_SDK_PROCESS_QUEUE_RELEASE = 11,
    RIN_GPU_SDK_PROCESS_SUBMIT = 12,
    RIN_GPU_SDK_PROCESS_QUEUE_TIMELINE_QUERY = 13
};

typedef uint64_t RinGpuAllocationV1;
/* An authenticated process-local queue token, not a RingGPU implementation
 * handle or a physical queue register value. */
typedef uint64_t RinGpuProcessQueueV1;

typedef struct RinGpuProcessResourceV1 {
    uint64_t allocation;
    uint32_t required_gpu_access;
    uint32_t reserved;
} RinGpuProcessResourceV1;

/* `commands` is a byte slice of canonical public RinGpuBackendCommandV1
 * records (record size is checked by the kernel). The descriptor and both
 * slices are copied by the syscall before submit; their addresses are never
 * retained as command cookies. */
typedef struct RinGpuProcessSubmitDescV1 {
    uint32_t struct_size;
    uint32_t version;
    RinSliceV1 commands;
    uint32_t command_count;
    uint32_t command_record_size;
    RinSliceV1 resources;
    uint32_t resource_count;
    uint32_t reserved;
} RinGpuProcessSubmitDescV1;

typedef struct RinGpuProcessSubmitReceiptV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t queue_id;
    uint32_t reserved0;
    uint64_t sequence;
    uint64_t completion_value;
    uint64_t device_epoch;
    uint64_t iommu_map_generation;
    uint64_t reserved[2];
} RinGpuProcessSubmitReceiptV1;

/* Nonblocking snapshot of the real, monotonically completed backend timeline
 * for one authenticated process queue. The caller supplies the device and
 * IOMMU generations from its submit receipt; a reset/rebind is reported as
 * stale instead of making an old timeline appear complete. */
typedef struct RinGpuProcessQueueTimelineV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t queue_id;
    uint32_t reserved0;
    uint64_t completed_value;
    uint64_t device_epoch;
    uint64_t iommu_map_generation;
} RinGpuProcessQueueTimelineV1;

/* Device identity comes from RinDeviceInfoV1. The generation is mandatory so
 * an allocation cannot silently bind to a replacement device with a reused
 * device ID. GPU virtual addresses in the query record remain GPUVA only;
 * they are not physical, IOVA, or display-fetch addresses. */
typedef struct RinGpuAllocationDescV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t heap;
    uint32_t flags;
    uint64_t size_bytes;
    uint64_t alignment;
    uint64_t reserved[4];
} RinGpuAllocationDescV1;

/* A successful query echoes the requested allocation handle, reports a
 * supported heap/access shape and active allocation, and returns aligned
 * address geometry with allocation_size_bytes >= requested_size_bytes,
 * nonzero device/IOMMU generations, and a bounded lease count. The reserved
 * field must be zero; the SDK rejects responses outside this contract. */
typedef struct RinGpuAllocationInfoV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t heap;
    uint32_t flags;
    RinGpuAllocationV1 allocation;
    uint64_t gpu_virtual_address;
    uint64_t heap_offset;
    uint64_t requested_size_bytes;
    uint64_t allocation_size_bytes;
    uint64_t alignment;
    uint64_t iommu_map_generation;
    uint64_t device_epoch;
    uint32_t lease_count;
    uint32_t state;
    uint64_t reserved;
} RinGpuAllocationInfoV1;

typedef struct RinGpuMemoryMappingV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t access;
    uint32_t reserved0;
    RinGpuAllocationV1 allocation;
    uint64_t address;
    uint64_t size_bytes;
    uint64_t mapping;
    uint64_t device_generation;
    uint64_t reserved;
} RinGpuMemoryMappingV1;

#if !defined(RIN_SDK_KERNEL_INTERNAL_ABI)
RIN_SDK_API RinResult rin_gpu_memory_allocate_v1(
    uint64_t device_id, uint64_t device_generation,
    const RinGpuAllocationDescV1* descriptor,
    RinGpuAllocationV1* allocation_out);
RIN_SDK_API RinResult rin_gpu_memory_query_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuAllocationV1 allocation, RinGpuAllocationInfoV1* info_out);
RIN_SDK_API RinResult rin_gpu_memory_destroy_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuAllocationV1 allocation);
/* Transfers are bounded to 1 MiB per call. Upload/readback use kernel-copied
 * buffers rather than mapping a physical backing into the process. A failed
 * multi-chunk transfer may have completed a prefix; retry the full range when
 * the reported result permits it. */
RIN_SDK_API RinResult rin_gpu_memory_sync_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuAllocationV1 allocation, uint32_t action, uint64_t offset,
    uint64_t length);
RIN_SDK_API RinResult rin_gpu_memory_upload_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuAllocationV1 allocation, uint64_t offset,
    const void* source, uint64_t length);
RIN_SDK_API RinResult rin_gpu_memory_readback_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuAllocationV1 allocation, uint64_t offset,
    void* destination, uint64_t length);
/* Map CPU-visible allocation backing into this process. The mapping is
 * process-instance and device-generation bound; release it with unmap before
 * destroying the allocation. Process exit also revokes outstanding maps.
 * If a successful kernel response contains a malformed mapping record, the
 * wrapper attempts immediate unmap. When cleanup fails, map returns that
 * cleanup error and mapping_out retains only its allocation, device
 * generation, and mapping token (address and size remain zero), so the caller
 * can retry unmap without using an unvalidated CPU address. */
RIN_SDK_API RinResult rin_gpu_memory_map_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuAllocationV1 allocation, uint32_t access,
    RinGpuMemoryMappingV1* mapping_out);
RIN_SDK_API RinResult rin_gpu_memory_unmap_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuAllocationV1 allocation, uint64_t mapping);
/* Bind a process-owned token to an admitted backend queue index. The queue
 * index is validated against the exact device generation and later checked
 * against the engine actually selected by command preparation. */
RIN_SDK_API RinResult rin_gpu_process_queue_bind_v1(
    uint64_t device_id, uint64_t device_generation, uint32_t queue_id,
    RinGpuProcessQueueV1* queue_out);
RIN_SDK_API RinResult rin_gpu_process_queue_release_v1(
    RinGpuProcessQueueV1 queue);
/* Submit canonical API-independent command records. Resources are exact
 * process-owned allocation handles with explicit GPU access; the kernel
 * copies and validates all records before the physical driver receives them.
 * Accepted work remains owned by PID+process-instance until real completion. */
RIN_SDK_API RinResult rin_gpu_process_submit_v1(
    RinGpuProcessQueueV1 queue, const RinGpuProcessSubmitDescV1* descriptor,
    RinGpuProcessSubmitReceiptV1* receipt_out);
RIN_SDK_API RinResult rin_gpu_process_queue_query_timeline_v1(
    RinGpuProcessQueueV1 queue, uint64_t expected_device_epoch,
    uint64_t expected_iommu_map_generation,
    RinGpuProcessQueueTimelineV1* timeline_out);
#endif

#if defined(__cplusplus)
static_assert(sizeof(RinGpuAllocationDescV1) == 64u,
              "RinGpuAllocationDescV1 ABI drift");
static_assert(sizeof(RinGpuAllocationInfoV1) == 96u,
              "RinGpuAllocationInfoV1 ABI drift");
static_assert(sizeof(RinGpuMemoryMappingV1) == 64u,
              "RinGpuMemoryMappingV1 ABI drift");
static_assert(sizeof(RinGpuProcessResourceV1) == 16u,
              "RinGpuProcessResourceV1 ABI drift");
static_assert(sizeof(RinGpuProcessSubmitDescV1) == 56u,
              "RinGpuProcessSubmitDescV1 ABI drift");
static_assert(sizeof(RinGpuProcessSubmitReceiptV1) == 64u,
              "RinGpuProcessSubmitReceiptV1 ABI drift");
static_assert(sizeof(RinGpuProcessQueueTimelineV1) == 40u,
              "RinGpuProcessQueueTimelineV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinGpuAllocationDescV1) == 64u,
               "RinGpuAllocationDescV1 ABI drift");
_Static_assert(sizeof(RinGpuAllocationInfoV1) == 96u,
               "RinGpuAllocationInfoV1 ABI drift");
_Static_assert(sizeof(RinGpuMemoryMappingV1) == 64u,
               "RinGpuMemoryMappingV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessResourceV1) == 16u,
               "RinGpuProcessResourceV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessSubmitDescV1) == 56u,
               "RinGpuProcessSubmitDescV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessSubmitReceiptV1) == 64u,
               "RinGpuProcessSubmitReceiptV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessQueueTimelineV1) == 40u,
               "RinGpuProcessQueueTimelineV1 ABI drift");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_GPU_H */
