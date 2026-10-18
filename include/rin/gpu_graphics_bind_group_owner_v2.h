/* SPDX-License-Identifier: MIT */
/* Typed lifecycle owner for V2 process-owned graphics bind groups. */
#ifndef RIN_SDK_GPU_GRAPHICS_BIND_GROUP_OWNER_V2_H
#define RIN_SDK_GPU_GRAPHICS_BIND_GROUP_OWNER_V2_H

#include <rin/gpu.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define RIN_GPU_PROCESS_GRAPHICS_BIND_GROUP_OWNER_VERSION_V2 UINT32_C(2)
#define RIN_GPU_PROCESS_GRAPHICS_BIND_GROUP_OWNER_PHASE_EMPTY_V2 UINT32_C(0)
#define RIN_GPU_PROCESS_GRAPHICS_BIND_GROUP_OWNER_PHASE_ACTIVE_V2 UINT32_C(1)

/* The owner records tokens only. V2 descriptors are copied by the SDK call
 * and are not retained; process-table dependencies own imported objects. */
typedef struct RinGpuProcessGraphicsBindGroupOwnerV2 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t phase;
    uint32_t binding_count;
    uint64_t device_id;
    uint64_t device_generation;
    uint64_t bind_group_token;
    uint64_t pipeline_token;
    uint64_t reserved[2];
} RinGpuProcessGraphicsBindGroupOwnerV2;

static inline int rin_gpu_process_graphics_bind_group_owner_v2_zero_words(
    const uint64_t* words, size_t count) {
    if (!words) return 0;
    while (count-- != 0u)
        if (*words++ != 0u) return 0;
    return 1;
}

static inline int rin_gpu_process_graphics_bind_group_owner_v2_valid(
    const RinGpuProcessGraphicsBindGroupOwnerV2* owner) {
    if (!owner || owner->struct_size != sizeof(*owner) ||
        owner->version != RIN_GPU_PROCESS_GRAPHICS_BIND_GROUP_OWNER_VERSION_V2 ||
        !rin_gpu_process_graphics_bind_group_owner_v2_zero_words(
            owner->reserved, 2u))
        return 0;
    if (owner->phase == RIN_GPU_PROCESS_GRAPHICS_BIND_GROUP_OWNER_PHASE_EMPTY_V2)
        return owner->binding_count == 0u && owner->device_id == 0u &&
               owner->device_generation == 0u &&
               owner->bind_group_token == 0u && owner->pipeline_token == 0u;
    return owner->phase == RIN_GPU_PROCESS_GRAPHICS_BIND_GROUP_OWNER_PHASE_ACTIVE_V2 &&
           owner->binding_count != 0u &&
           owner->binding_count <= RIN_GPU_PROCESS_GRAPHICS_BINDING_V2_MAX_COUNT &&
           owner->device_id != 0u && owner->device_generation != 0u &&
           owner->bind_group_token != 0u && owner->pipeline_token != 0u &&
           owner->bind_group_token != owner->pipeline_token;
}

static inline void rin_gpu_process_graphics_bind_group_owner_v2_init(
    RinGpuProcessGraphicsBindGroupOwnerV2* owner) {
    if (!owner) return;
    memset(owner, 0, sizeof(*owner));
    owner->struct_size = sizeof(*owner);
    owner->version = RIN_GPU_PROCESS_GRAPHICS_BIND_GROUP_OWNER_VERSION_V2;
    owner->phase = RIN_GPU_PROCESS_GRAPHICS_BIND_GROUP_OWNER_PHASE_EMPTY_V2;
}

static inline RinResult rin_gpu_process_graphics_bind_group_owner_v2_create(
    uint64_t device_id, uint64_t device_generation,
    uint64_t bind_group_token, uint64_t pipeline_token,
    const RinGpuProcessGraphicsBindingV2* bindings, uint32_t binding_count,
    RinGpuProcessGraphicsBindGroupOwnerV2* owner_out) {
    RinGpuProcessGraphicsBindGroupOwnerV2 candidate;
    RinGpuProcessGraphicsBindingV2 binding_copy[
        RIN_GPU_PROCESS_GRAPHICS_BINDING_V2_MAX_COUNT];
    size_t binding_bytes;
    RinResult result;

    if (!owner_out || !rin_gpu_process_graphics_bind_group_owner_v2_valid(
                          owner_out) ||
        owner_out->phase != RIN_GPU_PROCESS_GRAPHICS_BIND_GROUP_OWNER_PHASE_EMPTY_V2 ||
        device_id == 0u || device_generation == 0u || bind_group_token == 0u ||
        pipeline_token == 0u || bind_group_token == pipeline_token || !bindings ||
        binding_count == 0u ||
        binding_count > RIN_GPU_PROCESS_GRAPHICS_BINDING_V2_MAX_COUNT)
        return RIN_ERROR_INVALID_ARGUMENT;
    binding_bytes = sizeof(binding_copy[0]) * (size_t)binding_count;
    if ((uintptr_t)bindings > UINTPTR_MAX - binding_bytes ||
        (uintptr_t)owner_out > UINTPTR_MAX - sizeof(*owner_out) ||
        ((uintptr_t)owner_out < (uintptr_t)bindings + binding_bytes &&
         (uintptr_t)bindings < (uintptr_t)owner_out + sizeof(*owner_out)))
        return RIN_ERROR_INVALID_ARGUMENT;
    memcpy(binding_copy, bindings, binding_bytes);
    result = rin_gpu_process_graphics_bind_group_create_v2(
        device_id, device_generation, bind_group_token, pipeline_token,
        binding_copy, binding_count);
    if (result != RIN_SUCCESS) return result;
    rin_gpu_process_graphics_bind_group_owner_v2_init(&candidate);
    candidate.phase = RIN_GPU_PROCESS_GRAPHICS_BIND_GROUP_OWNER_PHASE_ACTIVE_V2;
    candidate.binding_count = binding_count;
    candidate.device_id = device_id;
    candidate.device_generation = device_generation;
    candidate.bind_group_token = bind_group_token;
    candidate.pipeline_token = pipeline_token;
    *owner_out = candidate;
    return RIN_SUCCESS;
}

static inline RinResult rin_gpu_process_graphics_bind_group_owner_v2_destroy(
    RinGpuProcessGraphicsBindGroupOwnerV2* owner) {
    RinResult result;
    if (!rin_gpu_process_graphics_bind_group_owner_v2_valid(owner) ||
        owner->phase != RIN_GPU_PROCESS_GRAPHICS_BIND_GROUP_OWNER_PHASE_ACTIVE_V2)
        return RIN_ERROR_INVALID_ARGUMENT;
    result = rin_gpu_process_object_destroy_v1(
        owner->device_id, owner->device_generation,
        owner->bind_group_token);
    if (result == RIN_SUCCESS)
        rin_gpu_process_graphics_bind_group_owner_v2_init(owner);
    return result;
}

#if defined(__cplusplus)
static_assert(sizeof(RinGpuProcessGraphicsBindGroupOwnerV2) == 64u,
              "RinGpuProcessGraphicsBindGroupOwnerV2 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinGpuProcessGraphicsBindGroupOwnerV2) == 64u,
               "RinGpuProcessGraphicsBindGroupOwnerV2 ABI drift");
#endif

#endif /* RIN_SDK_GPU_GRAPHICS_BIND_GROUP_OWNER_V2_H */
