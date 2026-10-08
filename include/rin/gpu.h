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

#define RIN_GPU_HEAP_LOCAL UINT32_C(1)
#define RIN_GPU_HEAP_SYSTEM UINT32_C(2)

#define RIN_GPU_ALLOCATION_GPU_READ UINT32_C(0x00000001)
#define RIN_GPU_ALLOCATION_GPU_WRITE UINT32_C(0x00000002)
#define RIN_GPU_ALLOCATION_CPU_VISIBLE UINT32_C(0x00000004)
#define RIN_GPU_ALLOCATION_ZEROED UINT32_C(0x00000008)

enum RinGpuSdkOperationV1 {
    RIN_GPU_SDK_MEMORY_ALLOCATE = 1,
    RIN_GPU_SDK_MEMORY_QUERY = 2,
    RIN_GPU_SDK_MEMORY_DESTROY = 3
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
#endif

#if defined(__cplusplus)
static_assert(sizeof(RinGpuAllocationDescV1) == 64u,
              "RinGpuAllocationDescV1 ABI drift");
static_assert(sizeof(RinGpuAllocationInfoV1) == 96u,
              "RinGpuAllocationInfoV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinGpuAllocationDescV1) == 64u,
               "RinGpuAllocationDescV1 ABI drift");
_Static_assert(sizeof(RinGpuAllocationInfoV1) == 96u,
               "RinGpuAllocationInfoV1 ABI drift");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_GPU_H */
