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
    RIN_GPU_SDK_CAPABILITY_DISPATCH = 9
};

typedef uint64_t RinGpuAllocationV1;

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
#endif

#if defined(__cplusplus)
static_assert(sizeof(RinGpuAllocationDescV1) == 64u,
              "RinGpuAllocationDescV1 ABI drift");
static_assert(sizeof(RinGpuAllocationInfoV1) == 96u,
              "RinGpuAllocationInfoV1 ABI drift");
static_assert(sizeof(RinGpuMemoryMappingV1) == 64u,
              "RinGpuMemoryMappingV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinGpuAllocationDescV1) == 64u,
               "RinGpuAllocationDescV1 ABI drift");
_Static_assert(sizeof(RinGpuAllocationInfoV1) == 96u,
               "RinGpuAllocationInfoV1 ABI drift");
_Static_assert(sizeof(RinGpuMemoryMappingV1) == 64u,
               "RinGpuMemoryMappingV1 ABI drift");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_GPU_H */
