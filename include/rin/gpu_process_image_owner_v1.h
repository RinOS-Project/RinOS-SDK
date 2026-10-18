/* SPDX-License-Identifier: MIT */
/* Typed lifecycle owner for process-owned images. */
#ifndef RIN_SDK_GPU_PROCESS_IMAGE_OWNER_V1_H
#define RIN_SDK_GPU_PROCESS_IMAGE_OWNER_V1_H

#include <rin/gpu.h>

#include <stddef.h>
#include <stdint.h>

#define RIN_GPU_PROCESS_IMAGE_OWNER_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_IMAGE_OWNER_PHASE_EMPTY UINT32_C(0)
#define RIN_GPU_PROCESS_IMAGE_OWNER_PHASE_ACTIVE UINT32_C(1)

typedef struct RinGpuProcessImageOwnerV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t phase;
    uint32_t reserved0;
    uint64_t device_id;
    uint64_t device_generation;
    RinGpuProcessImageViewV1 image;
    uint64_t reserved[2];
} RinGpuProcessImageOwnerV1;

static inline void rin_gpu_process_image_owner_v1_zero(
    void* memory, size_t size)
{
    unsigned char* bytes = (unsigned char*)memory;
    while (size-- != 0u) *bytes++ = 0u;
}

static inline int rin_gpu_process_image_owner_v1_overlap(
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

static inline int rin_gpu_process_image_owner_v1_valid(
    const RinGpuProcessImageOwnerV1* owner)
{
    if (!owner || owner->struct_size != sizeof(*owner) ||
        owner->version != RIN_GPU_PROCESS_IMAGE_OWNER_VERSION_V1 ||
        owner->reserved0 != 0u || owner->reserved[0] != 0u ||
        owner->reserved[1] != 0u)
        return 0;
    if (owner->phase == RIN_GPU_PROCESS_IMAGE_OWNER_PHASE_EMPTY)
        return owner->device_id == 0u && owner->device_generation == 0u &&
               rin_gpu_process_image_view_v1_zero(&owner->image);
    return owner->phase == RIN_GPU_PROCESS_IMAGE_OWNER_PHASE_ACTIVE &&
           owner->device_id != 0u && owner->device_generation != 0u &&
           rin_gpu_process_image_create_desc_v1_valid(&owner->image);
}

static inline void rin_gpu_process_image_owner_v1_init(
    RinGpuProcessImageOwnerV1* owner)
{
    if (!owner) return;
    rin_gpu_process_image_owner_v1_zero(owner, sizeof(*owner));
    owner->struct_size = sizeof(*owner);
    owner->version = RIN_GPU_PROCESS_IMAGE_OWNER_VERSION_V1;
    owner->phase = RIN_GPU_PROCESS_IMAGE_OWNER_PHASE_EMPTY;
}

/* A failed create leaves the initialized output empty. */
static inline RinResult rin_gpu_process_image_owner_v1_create(
    uint64_t device_id, uint64_t device_generation,
    const RinGpuProcessImageViewV1* image,
    RinGpuProcessImageOwnerV1* owner_out)
{
    RinGpuProcessImageViewV1 image_copy;
    RinResult result;
    if (!owner_out || !rin_gpu_process_image_owner_v1_valid(owner_out) ||
        owner_out->phase != RIN_GPU_PROCESS_IMAGE_OWNER_PHASE_EMPTY ||
        device_id == 0u || device_generation == 0u || !image ||
        rin_gpu_process_image_owner_v1_overlap(
            owner_out, sizeof(*owner_out), image, sizeof(*image)) ||
        !rin_gpu_process_image_create_desc_v1_valid(image))
        return RIN_ERROR_INVALID_ARGUMENT;
    image_copy = *image;
    result = rin_gpu_process_image_create_v1(
        device_id, device_generation, &image_copy);
    if (result != RIN_SUCCESS) return result;
    owner_out->device_id = device_id;
    owner_out->device_generation = device_generation;
    owner_out->image = image_copy;
    owner_out->phase = RIN_GPU_PROCESS_IMAGE_OWNER_PHASE_ACTIVE;
    return RIN_SUCCESS;
}

/* A failed destroy keeps the image token and descriptor for retry. */
static inline RinResult rin_gpu_process_image_owner_v1_destroy(
    RinGpuProcessImageOwnerV1* owner)
{
    RinResult result;
    if (!rin_gpu_process_image_owner_v1_valid(owner) ||
        owner->phase != RIN_GPU_PROCESS_IMAGE_OWNER_PHASE_ACTIVE)
        return RIN_ERROR_INVALID_ARGUMENT;
    result = rin_gpu_process_object_destroy_v1(
        owner->device_id, owner->device_generation, owner->image.image_token);
    if (result != RIN_SUCCESS) return result;
    rin_gpu_process_image_owner_v1_init(owner);
    return RIN_SUCCESS;
}

#if defined(__cplusplus)
static_assert(sizeof(RinGpuProcessImageOwnerV1) == 168u,
              "RinGpuProcessImageOwnerV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinGpuProcessImageOwnerV1) == 168u,
               "RinGpuProcessImageOwnerV1 ABI drift");
#endif

#endif /* RIN_SDK_GPU_PROCESS_IMAGE_OWNER_V1_H */
