/* SPDX-License-Identifier: MIT */
/* Typed lifecycle owner for a process-owned compute pipeline and its shader. */
#ifndef RIN_SDK_GPU_COMPUTE_PIPELINE_OWNER_V1_H
#define RIN_SDK_GPU_COMPUTE_PIPELINE_OWNER_V1_H

#include <rin/gpu.h>

#include <stddef.h>
#include <stdint.h>

#define RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_PHASE_EMPTY UINT32_C(0)
#define RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_PHASE_ACTIVE UINT32_C(1)
#define RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_PHASE_CLEANUP_PENDING \
    UINT32_C(2)

#define RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_HAS_SHADER_MODULE UINT32_C(1)
#define RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_HAS_PIPELINE UINT32_C(2)
#define RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_ALL_OBJECTS \
    (RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_HAS_SHADER_MODULE | \
     RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_HAS_PIPELINE)

/* This record owns only opaque process tokens. The shader IR is synchronously
 * copied by the SDK operation and is never retained here. */
typedef struct RinGpuProcessComputePipelineOwnerV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t phase;
    uint32_t owned_objects;
    uint64_t device_id;
    uint64_t device_generation;
    uint64_t pipeline_token;
    uint64_t shader_module_token;
    uint64_t reserved[2];
} RinGpuProcessComputePipelineOwnerV1;

static inline void rin_gpu_process_compute_pipeline_owner_v1_zero(
    void* memory, size_t size)
{
    unsigned char* bytes = (unsigned char*)memory;
    while (size-- != 0u) *bytes++ = 0u;
}

static inline int rin_gpu_process_compute_pipeline_owner_v1_zero_words(
    const uint64_t* words, size_t count)
{
    if (!words) return 0;
    while (count-- != 0u)
        if (*words++ != 0u) return 0;
    return 1;
}

static inline int rin_gpu_process_compute_pipeline_owner_v1_range_valid(
    const void* pointer, uint64_t size)
{
    uintptr_t begin;
    if (!pointer || size == 0u || size > (uint64_t)UINTPTR_MAX) return 0;
    begin = (uintptr_t)pointer;
    return begin <= UINTPTR_MAX - (uintptr_t)size;
}

static inline int rin_gpu_process_compute_pipeline_owner_v1_overlap(
    const void* first, uint64_t first_size,
    const void* second, uint64_t second_size)
{
    uintptr_t first_begin;
    uintptr_t second_begin;
    if (!rin_gpu_process_compute_pipeline_owner_v1_range_valid(
            first, first_size) ||
        !rin_gpu_process_compute_pipeline_owner_v1_range_valid(
            second, second_size))
        return 1;
    first_begin = (uintptr_t)first;
    second_begin = (uintptr_t)second;
    return first_begin < second_begin + (uintptr_t)second_size &&
           second_begin < first_begin + (uintptr_t)first_size;
}

static inline int rin_gpu_process_compute_pipeline_owner_v1_valid(
    const RinGpuProcessComputePipelineOwnerV1* owner)
{
    if (!owner || owner->struct_size != sizeof(*owner) ||
        owner->version != RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_VERSION_V1 ||
        !rin_gpu_process_compute_pipeline_owner_v1_zero_words(
            owner->reserved, 2u))
        return 0;
    if (owner->phase == RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_PHASE_EMPTY)
        return owner->owned_objects == 0u && owner->device_id == 0u &&
               owner->device_generation == 0u && owner->pipeline_token == 0u &&
               owner->shader_module_token == 0u;
    if ((owner->phase !=
             RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_PHASE_ACTIVE &&
         owner->phase !=
             RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_PHASE_CLEANUP_PENDING) ||
        owner->device_id == 0u || owner->device_generation == 0u ||
        owner->owned_objects == 0u ||
        (owner->owned_objects &
         ~RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_ALL_OBJECTS) != 0u ||
        ((owner->owned_objects &
          RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_HAS_PIPELINE) != 0u &&
         (owner->owned_objects &
          RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_HAS_SHADER_MODULE) == 0u))
        return 0;
    if ((owner->owned_objects &
         RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_HAS_SHADER_MODULE) != 0u) {
        if (owner->shader_module_token == 0u) return 0;
    } else if (owner->shader_module_token != 0u) {
        return 0;
    }
    if ((owner->owned_objects &
         RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_HAS_PIPELINE) != 0u) {
        if (owner->pipeline_token == 0u ||
            owner->pipeline_token == owner->shader_module_token)
            return 0;
    } else if (owner->pipeline_token != 0u) {
        return 0;
    }
    if (owner->phase == RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_PHASE_ACTIVE)
        return owner->owned_objects ==
               RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_ALL_OBJECTS;
    return 1;
}

static inline void rin_gpu_process_compute_pipeline_owner_v1_init(
    RinGpuProcessComputePipelineOwnerV1* owner)
{
    if (!owner) return;
    rin_gpu_process_compute_pipeline_owner_v1_zero(owner, sizeof(*owner));
    owner->struct_size = sizeof(*owner);
    owner->version = RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_VERSION_V1;
    owner->phase = RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_PHASE_EMPTY;
}

static inline RinResult rin_gpu_process_compute_pipeline_owner_v1_destroy(
    RinGpuProcessComputePipelineOwnerV1* owner);

/* Failed pipeline creation rolls back its shader module. If rollback itself
 * fails, owner_out records that live shader token as cleanup-pending. */
static inline RinResult rin_gpu_process_compute_pipeline_owner_v1_create(
    uint64_t device_id, uint64_t device_generation,
    uint64_t pipeline_token, uint64_t shader_module_token,
    const void* shader_ir, uint64_t shader_ir_size,
    RinGpuProcessComputePipelineOwnerV1* owner_out)
{
    RinGpuProcessComputePipelineOwnerV1 candidate;
    RinResult result;

    if (!owner_out ||
        !rin_gpu_process_compute_pipeline_owner_v1_valid(owner_out) ||
        owner_out->phase != RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_PHASE_EMPTY ||
        device_id == 0u || device_generation == 0u || pipeline_token == 0u ||
        shader_module_token == 0u || pipeline_token == shader_module_token ||
        !rin_gpu_process_compute_pipeline_owner_v1_range_valid(
            shader_ir, shader_ir_size) ||
        shader_ir_size > RIN_GPU_PROCESS_SHADER_MAX_BYTES_V1 ||
        rin_gpu_process_compute_pipeline_owner_v1_overlap(
            owner_out, sizeof(*owner_out), shader_ir, shader_ir_size))
        return RIN_ERROR_INVALID_ARGUMENT;

    rin_gpu_process_compute_pipeline_owner_v1_init(&candidate);
    result = rin_gpu_process_shader_module_create_v1(
        device_id, device_generation, shader_module_token, shader_ir,
        shader_ir_size);
    if (result != RIN_SUCCESS) return result;
    candidate.device_id = device_id;
    candidate.device_generation = device_generation;
    candidate.shader_module_token = shader_module_token;
    candidate.owned_objects =
        RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_HAS_SHADER_MODULE;
    candidate.phase =
        RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_PHASE_CLEANUP_PENDING;

    result = rin_gpu_process_compute_pipeline_create_v1(
        device_id, device_generation, pipeline_token, shader_module_token);
    if (result != RIN_SUCCESS) {
        const RinResult create_result = result;
        if (rin_gpu_process_compute_pipeline_owner_v1_destroy(&candidate) !=
            RIN_SUCCESS)
            *owner_out = candidate;
        return create_result;
    }

    candidate.pipeline_token = pipeline_token;
    candidate.owned_objects |=
        RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_HAS_PIPELINE;
    candidate.phase = RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_PHASE_ACTIVE;
    *owner_out = candidate;
    return RIN_SUCCESS;
}

/* Destroy the dependent pipeline before its shader. Partial cleanup is
 * retained as a retryable owner rather than losing either process token. */
static inline RinResult rin_gpu_process_compute_pipeline_owner_v1_destroy(
    RinGpuProcessComputePipelineOwnerV1* owner)
{
    RinResult result;
    if (!rin_gpu_process_compute_pipeline_owner_v1_valid(owner) ||
        owner->phase == RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_PHASE_EMPTY)
        return RIN_ERROR_INVALID_ARGUMENT;
    if ((owner->owned_objects &
         RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_HAS_PIPELINE) != 0u) {
        result = rin_gpu_process_object_destroy_v1(
            owner->device_id, owner->device_generation, owner->pipeline_token);
        if (result != RIN_SUCCESS) return result;
        owner->pipeline_token = 0u;
        owner->owned_objects &=
            ~RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_HAS_PIPELINE;
        owner->phase =
            RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_PHASE_CLEANUP_PENDING;
    }
    if ((owner->owned_objects &
         RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_HAS_SHADER_MODULE) != 0u) {
        result = rin_gpu_process_object_destroy_v1(
            owner->device_id, owner->device_generation,
            owner->shader_module_token);
        if (result != RIN_SUCCESS) return result;
        owner->shader_module_token = 0u;
        owner->owned_objects &=
            ~RIN_GPU_PROCESS_COMPUTE_PIPELINE_OWNER_HAS_SHADER_MODULE;
    }
    rin_gpu_process_compute_pipeline_owner_v1_init(owner);
    return RIN_SUCCESS;
}

#if defined(__cplusplus)
static_assert(sizeof(RinGpuProcessComputePipelineOwnerV1) == 64u,
              "RinGpuProcessComputePipelineOwnerV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinGpuProcessComputePipelineOwnerV1) == 64u,
               "RinGpuProcessComputePipelineOwnerV1 ABI drift");
#endif

#endif /* RIN_SDK_GPU_COMPUTE_PIPELINE_OWNER_V1_H */
