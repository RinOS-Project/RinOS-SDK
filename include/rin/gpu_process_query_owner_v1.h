/* SPDX-License-Identifier: MIT */
/* Typed lifecycle owner for process-owned GPU queries. */
#ifndef RIN_SDK_GPU_PROCESS_QUERY_OWNER_V1_H
#define RIN_SDK_GPU_PROCESS_QUERY_OWNER_V1_H

#include <rin/gpu.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define RIN_GPU_PROCESS_QUERY_OWNER_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_QUERY_OWNER_PHASE_EMPTY UINT32_C(0)
#define RIN_GPU_PROCESS_QUERY_OWNER_PHASE_ACTIVE UINT32_C(1)

/* The owner records the authenticated process-local token identity and query
 * type. It never owns or fabricates backend result storage. */
typedef struct RinGpuProcessQueryOwnerV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t phase;
    uint32_t query_type;
    uint64_t device_id;
    uint64_t device_generation;
    uint64_t query_token;
    uint64_t reserved[2];
} RinGpuProcessQueryOwnerV1;

static inline void rin_gpu_process_query_owner_v1_zero(
    void* memory, size_t size)
{
    unsigned char* bytes = (unsigned char*)memory;
    while (size-- != 0u) *bytes++ = 0u;
}

static inline int rin_gpu_process_query_owner_v1_zero_words(
    const uint64_t* words, size_t count)
{
    if (!words) return 0;
    while (count-- != 0u)
        if (*words++ != 0u) return 0;
    return 1;
}

static inline int rin_gpu_process_query_owner_v1_range_valid(
    const void* pointer, size_t size)
{
    uintptr_t begin;
    if (!pointer || size == 0u) return 0;
    begin = (uintptr_t)pointer;
    return begin <= UINTPTR_MAX - size;
}

static inline int rin_gpu_process_query_owner_v1_overlap(
    const void* first, size_t first_size,
    const void* second, size_t second_size)
{
    uintptr_t first_begin;
    uintptr_t second_begin;
    if (!rin_gpu_process_query_owner_v1_range_valid(first, first_size) ||
        !rin_gpu_process_query_owner_v1_range_valid(second, second_size))
        return 1;
    first_begin = (uintptr_t)first;
    second_begin = (uintptr_t)second;
    return first_begin < second_begin + second_size &&
           second_begin < first_begin + first_size;
}

static inline int rin_gpu_process_query_owner_v1_type_valid(
    uint32_t query_type)
{
    return query_type == RIN_GPU_PROCESS_QUERY_TIMESTAMP_V1 ||
           query_type == RIN_GPU_PROCESS_QUERY_OCCLUSION_V1 ||
           query_type == RIN_GPU_PROCESS_QUERY_PIPELINE_STATISTICS_V1;
}

static inline int rin_gpu_process_query_owner_v1_valid(
    const RinGpuProcessQueryOwnerV1* owner)
{
    if (!owner || owner->struct_size != sizeof(*owner) ||
        owner->version != RIN_GPU_PROCESS_QUERY_OWNER_VERSION_V1 ||
        !rin_gpu_process_query_owner_v1_zero_words(owner->reserved, 2u))
        return 0;
    if (owner->phase == RIN_GPU_PROCESS_QUERY_OWNER_PHASE_EMPTY)
        return owner->query_type == 0u && owner->device_id == 0u &&
               owner->device_generation == 0u && owner->query_token == 0u;
    return owner->phase == RIN_GPU_PROCESS_QUERY_OWNER_PHASE_ACTIVE &&
           rin_gpu_process_query_owner_v1_type_valid(owner->query_type) &&
           owner->device_id != 0u && owner->device_generation != 0u &&
           owner->query_token != 0u;
}

static inline void rin_gpu_process_query_owner_v1_init(
    RinGpuProcessQueryOwnerV1* owner)
{
    if (!owner) return;
    rin_gpu_process_query_owner_v1_zero(owner, sizeof(*owner));
    owner->struct_size = sizeof(*owner);
    owner->version = RIN_GPU_PROCESS_QUERY_OWNER_VERSION_V1;
    owner->phase = RIN_GPU_PROCESS_QUERY_OWNER_PHASE_EMPTY;
}

static inline int rin_gpu_process_query_owner_v1_descriptor_valid(
    const RinGpuProcessQueryDescV1* descriptor)
{
    return descriptor && descriptor->struct_size == sizeof(*descriptor) &&
           descriptor->version == RIN_GPU_PROCESS_QUERY_DESC_VERSION_V1 &&
           rin_gpu_process_query_owner_v1_type_valid(
               descriptor->query_type) &&
           descriptor->flags == 0u && descriptor->reserved0 == 0u &&
           descriptor->reserved1 == 0u;
}

/* Creation is transactional: the owner remains EMPTY when SDK creation
 * fails. The descriptor is snapshotted before the SDK call. */
static inline RinResult rin_gpu_process_query_owner_v1_create(
    uint64_t device_id, uint64_t device_generation, uint64_t query_token,
    const RinGpuProcessQueryDescV1* descriptor,
    RinGpuProcessQueryOwnerV1* owner_out)
{
    RinGpuProcessQueryDescV1 descriptor_copy;
    RinGpuProcessQueryOwnerV1 candidate;
    RinResult result;

    if (!owner_out || !rin_gpu_process_query_owner_v1_valid(owner_out) ||
        owner_out->phase != RIN_GPU_PROCESS_QUERY_OWNER_PHASE_EMPTY ||
        device_id == 0u || device_generation == 0u || query_token == 0u ||
        !rin_gpu_process_query_owner_v1_range_valid(
            descriptor, sizeof(*descriptor)) ||
        rin_gpu_process_query_owner_v1_overlap(
            owner_out, sizeof(*owner_out), descriptor, sizeof(*descriptor)))
        return RIN_ERROR_INVALID_ARGUMENT;
    descriptor_copy = *descriptor;
    if (!rin_gpu_process_query_owner_v1_descriptor_valid(&descriptor_copy))
        return RIN_ERROR_INVALID_ARGUMENT;
    rin_gpu_process_query_owner_v1_init(&candidate);
    result = rin_gpu_process_query_create_v1(
        device_id, device_generation, query_token, &descriptor_copy);
    if (result != RIN_SUCCESS) return result;
    candidate.phase = RIN_GPU_PROCESS_QUERY_OWNER_PHASE_ACTIVE;
    candidate.query_type = descriptor_copy.query_type;
    candidate.device_id = device_id;
    candidate.device_generation = device_generation;
    candidate.query_token = query_token;
    *owner_out = candidate;
    return RIN_SUCCESS;
}

/* Result retrieval preserves the SDK's versioned output contract and checks
 * that the returned query type matches the type bound at creation. */
static inline RinResult rin_gpu_process_query_owner_v1_result(
    const RinGpuProcessQueryOwnerV1* owner,
    RinGpuProcessQueryResultV1* result_out)
{
    RinResult result;
    if (!rin_gpu_process_query_owner_v1_valid(owner) ||
        owner->phase != RIN_GPU_PROCESS_QUERY_OWNER_PHASE_ACTIVE ||
        !rin_gpu_process_query_owner_v1_range_valid(
            result_out, sizeof(*result_out)) ||
        rin_gpu_process_query_owner_v1_overlap(
            owner, sizeof(*owner), result_out, sizeof(*result_out)))
        return RIN_ERROR_INVALID_ARGUMENT;
    if (result_out->struct_size != sizeof(*result_out) ||
        result_out->version != RIN_GPU_PROCESS_QUERY_RESULT_VERSION_V1)
        return RIN_ERROR_ABI_MISMATCH;
    result = rin_gpu_process_query_result_v1(
        owner->device_id, owner->device_generation, owner->query_token,
        result_out);
    if (result == RIN_SUCCESS && result_out->query_type != owner->query_type) {
        rin_gpu_process_query_owner_v1_zero(result_out, sizeof(*result_out));
        result_out->struct_size = sizeof(*result_out);
        result_out->version = RIN_GPU_PROCESS_QUERY_RESULT_VERSION_V1;
        return RIN_ERROR_ABI_MISMATCH;
    }
    return result;
}

/* Destroy failures retain the exact token so callers can retry safely. */
static inline RinResult rin_gpu_process_query_owner_v1_destroy(
    RinGpuProcessQueryOwnerV1* owner)
{
    RinResult result;
    if (!rin_gpu_process_query_owner_v1_valid(owner) ||
        owner->phase != RIN_GPU_PROCESS_QUERY_OWNER_PHASE_ACTIVE)
        return RIN_ERROR_INVALID_ARGUMENT;
    result = rin_gpu_process_object_destroy_v1(
        owner->device_id, owner->device_generation, owner->query_token);
    if (result == RIN_SUCCESS)
        rin_gpu_process_query_owner_v1_init(owner);
    return result;
}

#if defined(__cplusplus)
static_assert(sizeof(RinGpuProcessQueryOwnerV1) == 56u,
              "RinGpuProcessQueryOwnerV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinGpuProcessQueryOwnerV1) == 56u,
               "RinGpuProcessQueryOwnerV1 ABI drift");
#endif

#endif /* RIN_SDK_GPU_PROCESS_QUERY_OWNER_V1_H */
