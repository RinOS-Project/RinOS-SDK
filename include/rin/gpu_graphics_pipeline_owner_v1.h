/* SPDX-License-Identifier: MIT */
/* Typed lifecycle owner for process-owned graphics pipelines. */
#ifndef RIN_SDK_GPU_GRAPHICS_PIPELINE_OWNER_V1_H
#define RIN_SDK_GPU_GRAPHICS_PIPELINE_OWNER_V1_H

#include <rin/gpu.h>

#include <stddef.h>
#include <stdint.h>

#define RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_VERSION_V1 UINT32_C(1)

#define RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_PHASE_EMPTY UINT32_C(0)
#define RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_PHASE_ACTIVE UINT32_C(1)
#define RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_PHASE_CLEANUP_PENDING \
    UINT32_C(2)

#define RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_HAS_VERTEX_MODULE \
    UINT32_C(0x1)
#define RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_HAS_FRAGMENT_MODULE \
    UINT32_C(0x2)
#define RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_HAS_PIPELINE UINT32_C(0x4)
#define RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_ALL_OBJECTS \
    (RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_HAS_VERTEX_MODULE | \
     RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_HAS_FRAGMENT_MODULE | \
     RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_HAS_PIPELINE)

/* Tokens are supplied by the caller and must be nonzero and pairwise unique.
 * This owner contains no shader pointers: creation copies both RSH1 modules
 * through the SDK calls, and later destruction uses only their tokens. */
typedef struct RinGpuProcessGraphicsPipelineOwnerV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t phase;
    uint32_t owned_objects;
    uint64_t device_id;
    uint64_t device_generation;
    uint64_t pipeline_token;
    uint64_t vertex_shader_token;
    uint64_t fragment_shader_token;
    uint64_t reserved[2];
} RinGpuProcessGraphicsPipelineOwnerV1;

static inline void rin_gpu_process_graphics_pipeline_owner_v1_zero(
    void* memory, size_t size)
{
    unsigned char* bytes = (unsigned char*)memory;
    while (size-- != 0u) *bytes++ = 0u;
}

static inline int rin_gpu_process_graphics_pipeline_owner_v1_zero_words(
    const uint64_t* words, size_t count)
{
    if (!words) return 0;
    while (count-- != 0u)
        if (*words++ != 0u) return 0;
    return 1;
}

static inline int rin_gpu_process_graphics_pipeline_owner_v1_range_valid(
    const void* pointer, uint64_t size)
{
    uintptr_t begin;
    if (!pointer || size == 0u || size > (uint64_t)UINTPTR_MAX) return 0;
    begin = (uintptr_t)pointer;
    return begin <= UINTPTR_MAX - (uintptr_t)size;
}

static inline int rin_gpu_process_graphics_pipeline_owner_v1_overlap(
    const void* first, uint64_t first_size,
    const void* second, uint64_t second_size)
{
    uintptr_t first_begin;
    uintptr_t second_begin;
    if (!rin_gpu_process_graphics_pipeline_owner_v1_range_valid(
            first, first_size) ||
        !rin_gpu_process_graphics_pipeline_owner_v1_range_valid(
            second, second_size))
        return 1;
    first_begin = (uintptr_t)first;
    second_begin = (uintptr_t)second;
    return first_begin < second_begin + (uintptr_t)second_size &&
           second_begin < first_begin + (uintptr_t)first_size;
}

static inline int rin_gpu_process_graphics_pipeline_owner_v1_valid(
    const RinGpuProcessGraphicsPipelineOwnerV1* owner)
{
    if (!owner || owner->struct_size != sizeof(*owner) ||
        owner->version != RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_VERSION_V1 ||
        !rin_gpu_process_graphics_pipeline_owner_v1_zero_words(
            owner->reserved, 2u))
        return 0;
    if (owner->phase == RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_PHASE_EMPTY)
        return owner->owned_objects == 0u && owner->device_id == 0u &&
               owner->device_generation == 0u && owner->pipeline_token == 0u &&
               owner->vertex_shader_token == 0u &&
               owner->fragment_shader_token == 0u;
    if ((owner->phase !=
             RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_PHASE_ACTIVE &&
         owner->phase != RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_PHASE_CLEANUP_PENDING) ||
        owner->device_id == 0u || owner->device_generation == 0u ||
        owner->pipeline_token == 0u || owner->vertex_shader_token == 0u ||
        owner->fragment_shader_token == 0u ||
        owner->pipeline_token == owner->vertex_shader_token ||
        owner->pipeline_token == owner->fragment_shader_token ||
        owner->vertex_shader_token == owner->fragment_shader_token ||
        owner->owned_objects == 0u ||
        (owner->owned_objects &
         ~RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_ALL_OBJECTS) != 0u)
        return 0;
    if (owner->phase ==
        RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_PHASE_ACTIVE)
        return owner->owned_objects ==
               RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_ALL_OBJECTS;
    return 1;
}

/* Initialize before create. Empty owners are deliberately not accepted by
 * destroy, so accidental cleanup of an uninitialized record is observable. */
static inline void rin_gpu_process_graphics_pipeline_owner_v1_init(
    RinGpuProcessGraphicsPipelineOwnerV1* owner)
{
    if (!owner) return;
    rin_gpu_process_graphics_pipeline_owner_v1_zero(owner, sizeof(*owner));
    owner->struct_size = sizeof(*owner);
    owner->version = RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_VERSION_V1;
    owner->phase = RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_PHASE_EMPTY;
}

static inline RinResult rin_gpu_process_graphics_pipeline_owner_v1_destroy(
    RinGpuProcessGraphicsPipelineOwnerV1* owner);

/* On a failed create, owner_out remains empty if rollback succeeds. If any
 * rollback destroy fails, owner_out contains only the still-live tokens in
 * CLEANUP_PENDING state; callers must retry destroy before discarding it. */
static inline RinResult rin_gpu_process_graphics_pipeline_owner_v1_create(
    uint64_t device_id, uint64_t device_generation,
    uint64_t pipeline_token, uint64_t vertex_shader_token,
    uint64_t fragment_shader_token,
    const void* vertex_shader_ir, uint64_t vertex_shader_ir_size,
    const void* fragment_shader_ir, uint64_t fragment_shader_ir_size,
    const RinGpuProcessGraphicsPipelineDescV1* descriptor,
    RinGpuProcessGraphicsPipelineOwnerV1* owner_out)
{
    RinGpuProcessGraphicsPipelineOwnerV1 candidate;
    RinGpuProcessGraphicsPipelineDescV1 descriptor_copy;
    RinResult result;

    if (!owner_out ||
        !rin_gpu_process_graphics_pipeline_owner_v1_valid(owner_out) ||
        owner_out->phase !=
            RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_PHASE_EMPTY ||
        device_id == 0u || device_generation == 0u || pipeline_token == 0u ||
        vertex_shader_token == 0u || fragment_shader_token == 0u ||
        pipeline_token == vertex_shader_token ||
        pipeline_token == fragment_shader_token ||
        vertex_shader_token == fragment_shader_token ||
        !vertex_shader_ir || vertex_shader_ir_size == 0u ||
        vertex_shader_ir_size > RIN_GPU_PROCESS_SHADER_MAX_BYTES_V1 ||
        !fragment_shader_ir || fragment_shader_ir_size == 0u ||
        fragment_shader_ir_size > RIN_GPU_PROCESS_SHADER_MAX_BYTES_V1 ||
        !descriptor ||
        rin_gpu_process_graphics_pipeline_owner_v1_overlap(
            owner_out, sizeof(*owner_out), descriptor, sizeof(*descriptor)) ||
        rin_gpu_process_graphics_pipeline_owner_v1_overlap(
            owner_out, sizeof(*owner_out), vertex_shader_ir,
            vertex_shader_ir_size) ||
        rin_gpu_process_graphics_pipeline_owner_v1_overlap(
            owner_out, sizeof(*owner_out), fragment_shader_ir,
            fragment_shader_ir_size))
        return RIN_ERROR_INVALID_ARGUMENT;

    descriptor_copy = *descriptor;
    rin_gpu_process_graphics_pipeline_owner_v1_init(&candidate);
    candidate.device_id = device_id;
    candidate.device_generation = device_generation;
    candidate.pipeline_token = pipeline_token;
    candidate.vertex_shader_token = vertex_shader_token;
    candidate.fragment_shader_token = fragment_shader_token;

    result = rin_gpu_process_shader_module_create_v1(
        device_id, device_generation, vertex_shader_token, vertex_shader_ir,
        vertex_shader_ir_size);
    if (result != RIN_SUCCESS) return result;
    candidate.owned_objects |=
        RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_HAS_VERTEX_MODULE;

    result = rin_gpu_process_shader_module_create_v1(
        device_id, device_generation, fragment_shader_token, fragment_shader_ir,
        fragment_shader_ir_size);
    if (result != RIN_SUCCESS) goto rollback;
    candidate.owned_objects |=
        RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_HAS_FRAGMENT_MODULE;

    result = rin_gpu_process_graphics_pipeline_create_v1(
        device_id, device_generation, pipeline_token, vertex_shader_token,
        fragment_shader_token, &descriptor_copy);
    if (result != RIN_SUCCESS) goto rollback;
    candidate.owned_objects |=
        RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_HAS_PIPELINE;
    candidate.phase = RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_PHASE_ACTIVE;
    *owner_out = candidate;
    return RIN_SUCCESS;

rollback:
    candidate.phase =
        RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_PHASE_CLEANUP_PENDING;
    if (rin_gpu_process_graphics_pipeline_owner_v1_destroy(&candidate) !=
        RIN_SUCCESS)
        *owner_out = candidate;
    return result;
}

/* Destroys pipeline before its shader modules. A failed destroy retains that
 * token and all dependencies it may still need, allowing a safe retry. */
static inline RinResult rin_gpu_process_graphics_pipeline_owner_v1_destroy(
    RinGpuProcessGraphicsPipelineOwnerV1* owner)
{
    RinResult first_error = RIN_SUCCESS;
    RinResult result;

    if (!rin_gpu_process_graphics_pipeline_owner_v1_valid(owner) ||
        owner->phase == RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_PHASE_EMPTY)
        return RIN_ERROR_INVALID_ARGUMENT;

    if ((owner->owned_objects &
         RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_HAS_PIPELINE) != 0u) {
        result = rin_gpu_process_object_destroy_v1(
            owner->device_id, owner->device_generation,
            owner->pipeline_token);
        if (result != RIN_SUCCESS) return result;
        owner->owned_objects &=
            ~RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_HAS_PIPELINE;
    }

    if ((owner->owned_objects &
         RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_HAS_FRAGMENT_MODULE) != 0u) {
        result = rin_gpu_process_object_destroy_v1(
            owner->device_id, owner->device_generation,
            owner->fragment_shader_token);
        if (result == RIN_SUCCESS)
            owner->owned_objects &=
                ~RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_HAS_FRAGMENT_MODULE;
        else
            first_error = result;
    }
    if ((owner->owned_objects &
         RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_HAS_VERTEX_MODULE) != 0u) {
        result = rin_gpu_process_object_destroy_v1(
            owner->device_id, owner->device_generation,
            owner->vertex_shader_token);
        if (result == RIN_SUCCESS)
            owner->owned_objects &=
                ~RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_HAS_VERTEX_MODULE;
        else if (first_error == RIN_SUCCESS)
            first_error = result;
    }

    if (owner->owned_objects == 0u) {
        rin_gpu_process_graphics_pipeline_owner_v1_init(owner);
        return RIN_SUCCESS;
    }
    owner->phase =
        RIN_GPU_PROCESS_GRAPHICS_PIPELINE_OWNER_PHASE_CLEANUP_PENDING;
    return first_error == RIN_SUCCESS ? RIN_ERROR_BUSY : first_error;
}

#if defined(__cplusplus)
static_assert(sizeof(RinGpuProcessGraphicsPipelineOwnerV1) == 72u,
              "RinGpuProcessGraphicsPipelineOwnerV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinGpuProcessGraphicsPipelineOwnerV1) == 72u,
               "RinGpuProcessGraphicsPipelineOwnerV1 ABI drift");
#endif

#endif /* RIN_SDK_GPU_GRAPHICS_PIPELINE_OWNER_V1_H */
