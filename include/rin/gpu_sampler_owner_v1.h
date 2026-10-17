/* SPDX-License-Identifier: MIT */
/* Typed lifecycle owner for process-owned samplers. */
#ifndef RIN_SDK_GPU_SAMPLER_OWNER_V1_H
#define RIN_SDK_GPU_SAMPLER_OWNER_V1_H

#include <rin/gpu.h>

#include <stddef.h>
#include <stdint.h>

#define RIN_GPU_PROCESS_SAMPLER_OWNER_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_SAMPLER_OWNER_PHASE_EMPTY UINT32_C(0)
#define RIN_GPU_PROCESS_SAMPLER_OWNER_PHASE_ACTIVE UINT32_C(1)

typedef struct RinGpuProcessSamplerOwnerV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t phase;
    uint32_t reserved0;
    uint64_t device_id;
    uint64_t device_generation;
    uint64_t sampler_token;
    uint64_t reserved[2];
} RinGpuProcessSamplerOwnerV1;

static inline void rin_gpu_process_sampler_owner_v1_zero(
    void* memory, size_t size)
{
    unsigned char* bytes = (unsigned char*)memory;
    while (size-- != 0u) *bytes++ = 0u;
}

static inline int rin_gpu_process_sampler_owner_v1_zero_words(
    const uint64_t* words, size_t count)
{
    if (!words) return 0;
    while (count-- != 0u)
        if (*words++ != 0u) return 0;
    return 1;
}

static inline int rin_gpu_process_sampler_owner_v1_overlap(
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

static inline int rin_gpu_process_sampler_owner_v1_valid(
    const RinGpuProcessSamplerOwnerV1* owner)
{
    if (!owner || owner->struct_size != sizeof(*owner) ||
        owner->version != RIN_GPU_PROCESS_SAMPLER_OWNER_VERSION_V1 ||
        owner->reserved0 != 0u ||
        !rin_gpu_process_sampler_owner_v1_zero_words(owner->reserved, 2u))
        return 0;
    if (owner->phase == RIN_GPU_PROCESS_SAMPLER_OWNER_PHASE_EMPTY)
        return owner->device_id == 0u && owner->device_generation == 0u &&
               owner->sampler_token == 0u;
    return owner->phase == RIN_GPU_PROCESS_SAMPLER_OWNER_PHASE_ACTIVE &&
           owner->device_id != 0u && owner->device_generation != 0u &&
           owner->sampler_token != 0u;
}

static inline void rin_gpu_process_sampler_owner_v1_init(
    RinGpuProcessSamplerOwnerV1* owner)
{
    if (!owner) return;
    rin_gpu_process_sampler_owner_v1_zero(owner, sizeof(*owner));
    owner->struct_size = sizeof(*owner);
    owner->version = RIN_GPU_PROCESS_SAMPLER_OWNER_VERSION_V1;
    owner->phase = RIN_GPU_PROCESS_SAMPLER_OWNER_PHASE_EMPTY;
}

/* The SDK syscall copies the descriptor and establishes a real process-owned
 * sampler token before this owner becomes active. Failed calls leave the
 * initialized output empty and retryable. */
static inline RinResult rin_gpu_process_sampler_owner_v1_create(
    uint64_t device_id, uint64_t device_generation, uint64_t sampler_token,
    const RinGpuProcessSamplerDescV1* descriptor,
    RinGpuProcessSamplerOwnerV1* owner_out)
{
    RinGpuProcessSamplerDescV1 descriptor_copy;
    RinResult result;

    if (!owner_out || !rin_gpu_process_sampler_owner_v1_valid(owner_out) ||
        owner_out->phase != RIN_GPU_PROCESS_SAMPLER_OWNER_PHASE_EMPTY ||
        device_id == 0u || device_generation == 0u || sampler_token == 0u ||
        !descriptor ||
        rin_gpu_process_sampler_owner_v1_overlap(
            owner_out, sizeof(*owner_out), descriptor, sizeof(*descriptor)))
        return RIN_ERROR_INVALID_ARGUMENT;
    descriptor_copy = *descriptor;
    result = rin_gpu_process_sampler_create_v1(
        device_id, device_generation, sampler_token, &descriptor_copy);
    if (result != RIN_SUCCESS) return result;
    owner_out->device_id = device_id;
    owner_out->device_generation = device_generation;
    owner_out->sampler_token = sampler_token;
    owner_out->phase = RIN_GPU_PROCESS_SAMPLER_OWNER_PHASE_ACTIVE;
    return RIN_SUCCESS;
}

/* A failed revoke/destroy leaves the token live in this record for retry. */
static inline RinResult rin_gpu_process_sampler_owner_v1_destroy(
    RinGpuProcessSamplerOwnerV1* owner)
{
    RinResult result;
    if (!rin_gpu_process_sampler_owner_v1_valid(owner) ||
        owner->phase != RIN_GPU_PROCESS_SAMPLER_OWNER_PHASE_ACTIVE)
        return RIN_ERROR_INVALID_ARGUMENT;
    result = rin_gpu_process_object_destroy_v1(
        owner->device_id, owner->device_generation, owner->sampler_token);
    if (result != RIN_SUCCESS) return result;
    rin_gpu_process_sampler_owner_v1_init(owner);
    return RIN_SUCCESS;
}

#if defined(__cplusplus)
static_assert(sizeof(RinGpuProcessSamplerOwnerV1) == 56u,
              "RinGpuProcessSamplerOwnerV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinGpuProcessSamplerOwnerV1) == 56u,
               "RinGpuProcessSamplerOwnerV1 ABI drift");
#endif

#endif /* RIN_SDK_GPU_SAMPLER_OWNER_V1_H */
