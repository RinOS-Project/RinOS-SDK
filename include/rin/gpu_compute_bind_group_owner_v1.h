/* SPDX-License-Identifier: MIT */
/* Typed lifecycle owner for process-owned compute bind groups. */
#ifndef RIN_SDK_GPU_COMPUTE_BIND_GROUP_OWNER_V1_H
#define RIN_SDK_GPU_COMPUTE_BIND_GROUP_OWNER_V1_H

#include <rin/gpu.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define RIN_GPU_PROCESS_COMPUTE_BIND_GROUP_OWNER_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_COMPUTE_BIND_GROUP_OWNER_PHASE_EMPTY UINT32_C(0)
#define RIN_GPU_PROCESS_COMPUTE_BIND_GROUP_OWNER_PHASE_ACTIVE UINT32_C(1)

/* The owner retains only process-local tokens. Binding descriptors are
 * validated and copied synchronously by the SDK create operation. */
typedef struct RinGpuProcessComputeBindGroupOwnerV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t phase;
    uint32_t binding_count;
    uint64_t device_id;
    uint64_t device_generation;
    uint64_t bind_group_token;
    uint64_t pipeline_token;
    uint64_t reserved[2];
} RinGpuProcessComputeBindGroupOwnerV1;

static inline void rin_gpu_process_compute_bind_group_owner_v1_zero(
    void* memory, size_t size)
{
    unsigned char* bytes = (unsigned char*)memory;
    while (size-- != 0u) *bytes++ = 0u;
}

static inline int rin_gpu_process_compute_bind_group_owner_v1_zero_words(
    const uint64_t* words, size_t count)
{
    if (!words) return 0;
    while (count-- != 0u)
        if (*words++ != 0u) return 0;
    return 1;
}

static inline int rin_gpu_process_compute_bind_group_owner_v1_range_valid(
    const void* pointer, size_t size)
{
    uintptr_t begin;
    if (!pointer || size == 0u) return 0;
    begin = (uintptr_t)pointer;
    return begin <= UINTPTR_MAX - size;
}

static inline int rin_gpu_process_compute_bind_group_owner_v1_overlap(
    const void* first, size_t first_size,
    const void* second, size_t second_size)
{
    uintptr_t first_begin;
    uintptr_t second_begin;
    if (!rin_gpu_process_compute_bind_group_owner_v1_range_valid(
            first, first_size) ||
        !rin_gpu_process_compute_bind_group_owner_v1_range_valid(
            second, second_size))
        return 1;
    first_begin = (uintptr_t)first;
    second_begin = (uintptr_t)second;
    return first_begin < second_begin + second_size &&
           second_begin < first_begin + first_size;
}

static inline int rin_gpu_process_compute_bind_group_owner_v1_valid(
    const RinGpuProcessComputeBindGroupOwnerV1* owner)
{
    if (!owner || owner->struct_size != sizeof(*owner) ||
        owner->version !=
            RIN_GPU_PROCESS_COMPUTE_BIND_GROUP_OWNER_VERSION_V1 ||
        !rin_gpu_process_compute_bind_group_owner_v1_zero_words(
            owner->reserved, 2u))
        return 0;
    if (owner->phase ==
        RIN_GPU_PROCESS_COMPUTE_BIND_GROUP_OWNER_PHASE_EMPTY)
        return owner->binding_count == 0u && owner->device_id == 0u &&
               owner->device_generation == 0u &&
               owner->bind_group_token == 0u && owner->pipeline_token == 0u;
    return owner->phase ==
               RIN_GPU_PROCESS_COMPUTE_BIND_GROUP_OWNER_PHASE_ACTIVE &&
           owner->binding_count <=
               RIN_GPU_PROCESS_MAX_COMPUTE_BINDINGS_V1 &&
           owner->device_id != 0u && owner->device_generation != 0u &&
           owner->bind_group_token != 0u && owner->pipeline_token != 0u &&
           owner->bind_group_token != owner->pipeline_token;
}

static inline void rin_gpu_process_compute_bind_group_owner_v1_init(
    RinGpuProcessComputeBindGroupOwnerV1* owner)
{
    if (!owner) return;
    rin_gpu_process_compute_bind_group_owner_v1_zero(owner, sizeof(*owner));
    owner->struct_size = sizeof(*owner);
    owner->version = RIN_GPU_PROCESS_COMPUTE_BIND_GROUP_OWNER_VERSION_V1;
    owner->phase = RIN_GPU_PROCESS_COMPUTE_BIND_GROUP_OWNER_PHASE_EMPTY;
}

/* A resource-free pipeline may use the canonical empty binding set. Failed
 * creates leave the owner empty; failed destroys retain the token for retry. */
static inline RinResult rin_gpu_process_compute_bind_group_owner_v1_create(
    uint64_t device_id, uint64_t device_generation,
    uint64_t bind_group_token, uint64_t pipeline_token,
    const RinGpuProcessComputeBindingV1* bindings, uint32_t binding_count,
    RinGpuProcessComputeBindGroupOwnerV1* owner_out)
{
    RinGpuProcessComputeBindGroupOwnerV1 candidate;
    RinGpuProcessComputeBindingV1 binding_copy[
        RIN_GPU_PROCESS_MAX_COMPUTE_BINDINGS_V1];
    size_t binding_bytes = 0u;
    RinResult result;

    if (!owner_out || !rin_gpu_process_compute_bind_group_owner_v1_valid(
                          owner_out) ||
        owner_out->phase !=
            RIN_GPU_PROCESS_COMPUTE_BIND_GROUP_OWNER_PHASE_EMPTY ||
        device_id == 0u || device_generation == 0u || bind_group_token == 0u ||
        pipeline_token == 0u || bind_group_token == pipeline_token ||
        ((binding_count == 0u) != (bindings == NULL)) ||
        binding_count > RIN_GPU_PROCESS_MAX_COMPUTE_BINDINGS_V1)
        return RIN_ERROR_INVALID_ARGUMENT;
    if (binding_count != 0u) {
        binding_bytes = sizeof(binding_copy[0]) * (size_t)binding_count;
        if (!rin_gpu_process_compute_bind_group_owner_v1_range_valid(
                bindings, binding_bytes) ||
            rin_gpu_process_compute_bind_group_owner_v1_overlap(
                owner_out, sizeof(*owner_out), bindings, binding_bytes))
            return RIN_ERROR_INVALID_ARGUMENT;
        memcpy(binding_copy, bindings, binding_bytes);
    }
    rin_gpu_process_compute_bind_group_owner_v1_init(&candidate);
    result = rin_gpu_process_compute_bind_group_create_v1(
        device_id, device_generation, bind_group_token, pipeline_token,
        binding_count == 0u ? NULL : binding_copy, binding_count);
    if (result != RIN_SUCCESS) return result;
    candidate.phase = RIN_GPU_PROCESS_COMPUTE_BIND_GROUP_OWNER_PHASE_ACTIVE;
    candidate.binding_count = binding_count;
    candidate.device_id = device_id;
    candidate.device_generation = device_generation;
    candidate.bind_group_token = bind_group_token;
    candidate.pipeline_token = pipeline_token;
    *owner_out = candidate;
    return RIN_SUCCESS;
}

static inline RinResult rin_gpu_process_compute_bind_group_owner_v1_destroy(
    RinGpuProcessComputeBindGroupOwnerV1* owner)
{
    RinResult result;
    if (!rin_gpu_process_compute_bind_group_owner_v1_valid(owner) ||
        owner->phase !=
            RIN_GPU_PROCESS_COMPUTE_BIND_GROUP_OWNER_PHASE_ACTIVE)
        return RIN_ERROR_INVALID_ARGUMENT;
    result = rin_gpu_process_object_destroy_v1(
        owner->device_id, owner->device_generation,
        owner->bind_group_token);
    if (result == RIN_SUCCESS)
        rin_gpu_process_compute_bind_group_owner_v1_init(owner);
    return result;
}

#if defined(__cplusplus)
static_assert(sizeof(RinGpuProcessComputeBindGroupOwnerV1) == 64u,
              "RinGpuProcessComputeBindGroupOwnerV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinGpuProcessComputeBindGroupOwnerV1) == 64u,
               "RinGpuProcessComputeBindGroupOwnerV1 ABI drift");
#endif

#endif /* RIN_SDK_GPU_COMPUTE_BIND_GROUP_OWNER_V1_H */
