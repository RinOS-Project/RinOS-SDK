/* SPDX-License-Identifier: MIT */
/* Transactional allocation, process binding, and image-token owner. */
#ifndef RIN_SDK_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_V1_H
#define RIN_SDK_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_V1_H

#include "gpu_process_image_owner_v1.h"

#include <stddef.h>
#include <stdint.h>

#define RIN_GPU_PROCESS_ALLOCATED_IMAGE_DESC_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_STATE_ACTIVE_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_STATE_CREATE_UNBIND_PENDING_V1 \
    UINT32_C(2)
#define RIN_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_STATE_UNBIND_PENDING_V1 \
    UINT32_C(3)
#define RIN_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_STATE_ALLOCATION_DESTROY_PENDING_V1 \
    UINT32_C(4)

/* The image owns the whole new allocation. The caller selects the byte size,
 * alignment, heap, and exact GPU access from the authoritative API-neutral
 * image descriptor; this helper never derives a storage layout from extent or
 * format. A provider that cannot import the requested image still returns its
 * real error through rin_gpu_process_image_create_v1. */
typedef struct RinGpuProcessAllocatedImageDescV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t device_id;
    uint64_t device_generation;
    RinGpuAllocationDescV1 allocation;
    uint32_t required_gpu_access;
    uint32_t reserved;
    RinGpuProcessImageViewV1 image;
} RinGpuProcessAllocatedImageDescV1;

/* Caller-owned lifecycle record. ACTIVE owns the image token, process-resource
 * binding, and allocation. Cleanup phases distinguish a failed image create
 * from destruction of a live image so retries never destroy an object that
 * was not created or unbind an allocation before its image token is gone. */
typedef struct RinGpuProcessAllocatedImageOwnerV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t state;
    uint32_t reserved0;
    RinGpuAllocationV1 allocation;
    RinGpuProcessAllocatedImageDescV1 descriptor;
    RinGpuProcessImageOwnerV1 image_owner;
    uint64_t reserved[2];
} RinGpuProcessAllocatedImageOwnerV1;

static inline void rin_gpu_process_allocated_image_owner_v1_zero(
    void* memory, size_t size)
{
    unsigned char* bytes = (unsigned char*)memory;
    while (size-- != 0u) *bytes++ = 0u;
}

static inline int rin_gpu_process_allocated_image_owner_v1_bytes_zero(
    const void* memory, size_t size)
{
    const unsigned char* bytes = (const unsigned char*)memory;
    if (!memory) return 0;
    while (size-- != 0u)
        if (*bytes++ != 0u) return 0;
    return 1;
}

static inline int rin_gpu_process_allocated_image_owner_v1_bytes_equal(
    const void* first, const void* second, size_t size)
{
    const unsigned char* left = (const unsigned char*)first;
    const unsigned char* right = (const unsigned char*)second;
    if (!first || !second) return 0;
    while (size-- != 0u)
        if (*left++ != *right++) return 0;
    return 1;
}

static inline int rin_gpu_process_allocated_image_owner_v1_overlap(
    const void* first, size_t first_size,
    const void* second, size_t second_size)
{
    uintptr_t first_begin;
    uintptr_t second_begin;
    if (!first || !second || first_size == 0u || second_size == 0u) return 0;
    first_begin = (uintptr_t)first;
    second_begin = (uintptr_t)second;
    if (first_begin > UINTPTR_MAX - first_size ||
        second_begin > UINTPTR_MAX - second_size)
        return 1;
    return first_begin < second_begin + second_size &&
           second_begin < first_begin + first_size;
}

static inline uint32_t rin_gpu_process_allocated_image_required_access_v1(
    uint32_t usage)
{
    uint32_t access = 0u;
    if ((usage & (RIN_GPU_PROCESS_IMAGE_USAGE_COPY_SOURCE_V1 |
                  RIN_GPU_PROCESS_IMAGE_USAGE_SAMPLED_V1 |
                  RIN_GPU_PROCESS_IMAGE_USAGE_PRESENT_V1)) != 0u)
        access |= RIN_GPU_ALLOCATION_GPU_READ;
    if ((usage & (RIN_GPU_PROCESS_IMAGE_USAGE_COPY_DESTINATION_V1 |
                  RIN_GPU_PROCESS_IMAGE_USAGE_STORAGE_V1 |
                  RIN_GPU_PROCESS_IMAGE_USAGE_COLOR_TARGET_V1 |
                  RIN_GPU_PROCESS_IMAGE_USAGE_DEPTH_STENCIL_V1)) != 0u)
        access |= RIN_GPU_ALLOCATION_GPU_WRITE;
    if ((usage & RIN_GPU_PROCESS_IMAGE_USAGE_STORAGE_V1) != 0u)
        access |= RIN_GPU_ALLOCATION_GPU_READ;
    return access;
}

static inline int rin_gpu_process_allocated_image_desc_v1_valid(
    const RinGpuProcessAllocatedImageDescV1* descriptor)
{
    const uint32_t known_allocation_flags =
        RIN_GPU_ALLOCATION_GPU_READ | RIN_GPU_ALLOCATION_GPU_WRITE |
        RIN_GPU_ALLOCATION_CPU_VISIBLE | RIN_GPU_ALLOCATION_ZEROED;
    const uint32_t required_access = descriptor
        ? rin_gpu_process_allocated_image_required_access_v1(
              descriptor->image.usage)
        : 0u;
    if (!descriptor || descriptor->struct_size != sizeof(*descriptor) ||
        descriptor->version !=
            RIN_GPU_PROCESS_ALLOCATED_IMAGE_DESC_VERSION_V1 ||
        descriptor->device_id == 0u ||
        descriptor->device_generation == 0u ||
        descriptor->allocation.struct_size !=
            sizeof(descriptor->allocation) ||
        descriptor->allocation.version !=
            RIN_GPU_ALLOCATION_DESC_VERSION_V1 ||
        (descriptor->allocation.heap != RIN_GPU_HEAP_LOCAL &&
         descriptor->allocation.heap != RIN_GPU_HEAP_SYSTEM) ||
        (descriptor->allocation.flags & ~known_allocation_flags) != 0u ||
        descriptor->allocation.size_bytes == 0u ||
        (descriptor->allocation.alignment != 0u &&
         (descriptor->allocation.alignment &
          (descriptor->allocation.alignment - 1u)) != 0u) ||
        required_access == 0u ||
        descriptor->required_gpu_access != required_access ||
        (descriptor->allocation.flags & required_access) != required_access ||
        descriptor->reserved != 0u ||
        descriptor->image.allocation_offset != 0u ||
        descriptor->image.allocation_size_bytes !=
            descriptor->allocation.size_bytes ||
        !rin_gpu_process_image_create_desc_v1_valid(&descriptor->image))
        return 0;
    for (uint32_t index = 0u; index < 4u; ++index)
        if (descriptor->allocation.reserved[index] != 0u) return 0;
    return 1;
}

static inline void rin_gpu_process_allocated_image_owner_v1_init(
    RinGpuProcessAllocatedImageOwnerV1* owner)
{
    if (!owner) return;
    rin_gpu_process_allocated_image_owner_v1_zero(owner, sizeof(*owner));
}

static inline int rin_gpu_process_allocated_image_owner_v1_valid(
    const RinGpuProcessAllocatedImageOwnerV1* owner)
{
    if (!owner) return 0;
    if (owner->state == 0u)
        return rin_gpu_process_allocated_image_owner_v1_bytes_zero(
            owner, sizeof(*owner));
    if (owner->struct_size != sizeof(*owner) ||
        owner->version != RIN_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_VERSION_V1 ||
        owner->allocation == 0u || owner->reserved0 != 0u ||
        !rin_gpu_process_allocated_image_owner_v1_bytes_zero(
            owner->reserved, sizeof(owner->reserved)) ||
        !rin_gpu_process_allocated_image_desc_v1_valid(&owner->descriptor) ||
        !rin_gpu_process_image_owner_v1_valid(&owner->image_owner))
        return 0;
    if (owner->state == RIN_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_STATE_ACTIVE_V1)
        return owner->image_owner.phase ==
                   RIN_GPU_PROCESS_IMAGE_OWNER_PHASE_ACTIVE &&
               owner->image_owner.device_id == owner->descriptor.device_id &&
               owner->image_owner.device_generation ==
                   owner->descriptor.device_generation &&
               rin_gpu_process_allocated_image_owner_v1_bytes_equal(
                   &owner->image_owner.image, &owner->descriptor.image,
                   sizeof(owner->descriptor.image));
    if (owner->state ==
            RIN_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_STATE_CREATE_UNBIND_PENDING_V1 ||
        owner->state ==
            RIN_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_STATE_UNBIND_PENDING_V1 ||
        owner->state ==
            RIN_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_STATE_ALLOCATION_DESTROY_PENDING_V1)
        return owner->image_owner.phase ==
            RIN_GPU_PROCESS_IMAGE_OWNER_PHASE_EMPTY;
    return 0;
}

static inline void rin_gpu_process_allocated_image_owner_v1_capture(
    const RinGpuProcessAllocatedImageDescV1* descriptor,
    RinGpuAllocationV1 allocation,
    uint32_t state,
    RinGpuProcessAllocatedImageOwnerV1* owner_out)
{
    RinGpuProcessAllocatedImageOwnerV1 candidate;
    rin_gpu_process_allocated_image_owner_v1_init(&candidate);
    candidate.struct_size = sizeof(candidate);
    candidate.version = RIN_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_VERSION_V1;
    candidate.state = state;
    candidate.allocation = allocation;
    candidate.descriptor = *descriptor;
    rin_gpu_process_image_owner_v1_init(&candidate.image_owner);
    *owner_out = candidate;
}

/* Creation is transactional. If rollback cannot release the resource lease
 * or allocation, owner_out receives the exact retry phase instead of losing
 * the live handle. */
static inline RinResult rin_gpu_process_allocated_image_create_v1(
    const RinGpuProcessAllocatedImageDescV1* descriptor,
    RinGpuProcessAllocatedImageOwnerV1* owner_out)
{
    RinGpuProcessAllocatedImageDescV1 descriptor_copy;
    RinGpuProcessAllocatedImageOwnerV1 candidate;
    RinGpuProcessResourceBindingV1 binding;
    RinGpuAllocationV1 allocation = 0u;
    RinResult result;
    RinResult cleanup_result;

    if (!descriptor || !owner_out ||
        rin_gpu_process_allocated_image_owner_v1_overlap(
            owner_out, sizeof(*owner_out), descriptor, sizeof(*descriptor)))
        return RIN_ERROR_INVALID_ARGUMENT;
    descriptor_copy = *descriptor;
    if (!rin_gpu_process_allocated_image_desc_v1_valid(&descriptor_copy))
        return RIN_ERROR_INVALID_ARGUMENT;
    if (!rin_gpu_process_allocated_image_owner_v1_bytes_zero(
            owner_out, sizeof(*owner_out)))
        return rin_gpu_process_allocated_image_owner_v1_valid(owner_out)
                   ? RIN_ERROR_BUSY
                   : RIN_ERROR_INVALID_ARGUMENT;

    result = rin_gpu_memory_allocate_v1(
        descriptor_copy.device_id, descriptor_copy.device_generation,
        &descriptor_copy.allocation, &allocation);
    if (result != RIN_SUCCESS) return result;
    if (allocation == 0u) return RIN_ERROR_ABI_MISMATCH;

    rin_gpu_process_allocated_image_owner_v1_capture(
        &descriptor_copy, allocation,
        RIN_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_STATE_ALLOCATION_DESTROY_PENDING_V1,
        &candidate);
    rin_gpu_process_allocated_image_owner_v1_zero(&binding, sizeof(binding));
    binding.struct_size = sizeof(binding);
    binding.version = RIN_GPU_PROCESS_RESOURCE_BINDING_VERSION_V1;
    binding.resource_cookie = descriptor_copy.image.resource_cookie;
    binding.allocation = allocation;
    binding.allocation_offset = 0u;
    binding.size_bytes = descriptor_copy.allocation.size_bytes;
    binding.required_gpu_access = descriptor_copy.required_gpu_access;
    result = rin_gpu_process_resource_bind_v1(
        descriptor_copy.device_id, descriptor_copy.device_generation,
        &binding);
    if (result != RIN_SUCCESS) {
        cleanup_result = rin_gpu_memory_destroy_v1(
            descriptor_copy.device_id, descriptor_copy.device_generation,
            allocation);
        if (cleanup_result != RIN_SUCCESS) {
            *owner_out = candidate;
            return cleanup_result;
        }
        return result;
    }

    rin_gpu_process_image_owner_v1_init(&candidate.image_owner);
    result = rin_gpu_process_image_owner_v1_create(
        descriptor_copy.device_id, descriptor_copy.device_generation,
        &descriptor_copy.image, &candidate.image_owner);
    if (result != RIN_SUCCESS) {
        candidate.state =
            RIN_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_STATE_CREATE_UNBIND_PENDING_V1;
        cleanup_result = rin_gpu_process_resource_unbind_v1(
            descriptor_copy.device_id, descriptor_copy.device_generation,
            descriptor_copy.image.resource_cookie);
        if (cleanup_result != RIN_SUCCESS) {
            *owner_out = candidate;
            return cleanup_result;
        }
        candidate.state =
            RIN_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_STATE_ALLOCATION_DESTROY_PENDING_V1;
        cleanup_result = rin_gpu_memory_destroy_v1(
            descriptor_copy.device_id, descriptor_copy.device_generation,
            allocation);
        if (cleanup_result != RIN_SUCCESS) {
            *owner_out = candidate;
            return cleanup_result;
        }
        return result;
    }

    candidate.state = RIN_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_STATE_ACTIVE_V1;
    *owner_out = candidate;
    return RIN_SUCCESS;
}

/* Image destroy precedes unbind, which precedes allocation destruction. A
 * failure preserves the current phase and can be retried with this function. */
static inline RinResult rin_gpu_process_allocated_image_destroy_v1(
    RinGpuProcessAllocatedImageOwnerV1* owner)
{
    RinGpuProcessAllocatedImageOwnerV1 candidate;
    RinResult result;
    if (!rin_gpu_process_allocated_image_owner_v1_valid(owner) ||
        owner->state == 0u)
        return RIN_ERROR_INVALID_ARGUMENT;
    candidate = *owner;
    if (candidate.state ==
        RIN_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_STATE_ACTIVE_V1) {
        result = rin_gpu_process_image_owner_v1_destroy(
            &candidate.image_owner);
        if (result != RIN_SUCCESS) return result;
        candidate.state =
            RIN_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_STATE_UNBIND_PENDING_V1;
        *owner = candidate;
    }
    if (candidate.state ==
            RIN_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_STATE_CREATE_UNBIND_PENDING_V1 ||
        candidate.state ==
            RIN_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_STATE_UNBIND_PENDING_V1) {
        result = rin_gpu_process_resource_unbind_v1(
            candidate.descriptor.device_id,
            candidate.descriptor.device_generation,
            candidate.descriptor.image.resource_cookie);
        if (result != RIN_SUCCESS) {
            *owner = candidate;
            return result;
        }
        candidate.state =
            RIN_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_STATE_ALLOCATION_DESTROY_PENDING_V1;
        *owner = candidate;
    }
    result = rin_gpu_memory_destroy_v1(
        candidate.descriptor.device_id,
        candidate.descriptor.device_generation, candidate.allocation);
    if (result != RIN_SUCCESS) {
        candidate.state =
            RIN_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_STATE_ALLOCATION_DESTROY_PENDING_V1;
        *owner = candidate;
        return result;
    }
    rin_gpu_process_allocated_image_owner_v1_init(owner);
    return RIN_SUCCESS;
}

#if defined(__cplusplus)
static_assert(sizeof(RinGpuProcessAllocatedImageDescV1) == 216u,
              "RinGpuProcessAllocatedImageDescV1 ABI drift");
static_assert(sizeof(RinGpuProcessAllocatedImageOwnerV1) == 424u,
              "RinGpuProcessAllocatedImageOwnerV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinGpuProcessAllocatedImageDescV1) == 216u,
               "RinGpuProcessAllocatedImageDescV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessAllocatedImageOwnerV1) == 424u,
               "RinGpuProcessAllocatedImageOwnerV1 ABI drift");
#endif

#endif /* RIN_SDK_GPU_PROCESS_ALLOCATED_IMAGE_OWNER_V1_H */
