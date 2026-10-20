/* SPDX-License-Identifier: MIT */
/* Typed, retryable lifecycle owner for a process-owned GPU queue. */
#ifndef RIN_SDK_GPU_PROCESS_QUEUE_OWNER_V1_H
#define RIN_SDK_GPU_PROCESS_QUEUE_OWNER_V1_H

#include <rin/gpu.h>

#include <stddef.h>
#include <stdint.h>

#define RIN_GPU_PROCESS_QUEUE_OWNER_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_QUEUE_OWNER_PHASE_EMPTY UINT32_C(0)
#define RIN_GPU_PROCESS_QUEUE_OWNER_PHASE_ACTIVE UINT32_C(1)
#define RIN_GPU_PROCESS_QUEUE_OWNER_PHASE_CLEANUP_PENDING UINT32_C(2)

/* The queue token remains the authoritative process-local capability. The
 * device generation and queue index are retained for exact identity and
 * diagnostics; release is performed using only the token, as required by the
 * existing SDK ABI. */
typedef struct RinGpuProcessQueueOwnerV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t phase;
    uint32_t queue_id;
    uint64_t device_id;
    uint64_t device_generation;
    RinGpuProcessQueueV1 queue_token;
    uint64_t reserved;
} RinGpuProcessQueueOwnerV1;

static inline int rin_gpu_process_queue_owner_v1_valid(
    const RinGpuProcessQueueOwnerV1* owner)
{
    if (!owner || owner->struct_size != sizeof(*owner) ||
        owner->version != RIN_GPU_PROCESS_QUEUE_OWNER_VERSION_V1 ||
        owner->reserved != 0u)
        return 0;
    if (owner->phase == RIN_GPU_PROCESS_QUEUE_OWNER_PHASE_EMPTY)
        return owner->queue_id == 0u && owner->device_id == 0u &&
               owner->device_generation == 0u && owner->queue_token == 0u;
    return (owner->phase == RIN_GPU_PROCESS_QUEUE_OWNER_PHASE_ACTIVE ||
            owner->phase ==
                RIN_GPU_PROCESS_QUEUE_OWNER_PHASE_CLEANUP_PENDING) &&
           owner->queue_id < RIN_GPU_PROCESS_MAX_QUEUES_V1 &&
           owner->device_id != 0u && owner->device_generation != 0u &&
           owner->queue_token != 0u;
}

static inline void rin_gpu_process_queue_owner_v1_init(
    RinGpuProcessQueueOwnerV1* owner)
{
    if (!owner) return;
    unsigned char* bytes = (unsigned char*)owner;
    size_t size = sizeof(*owner);
    while (size-- != 0u) *bytes++ = 0u;
    owner->struct_size = sizeof(*owner);
    owner->version = RIN_GPU_PROCESS_QUEUE_OWNER_VERSION_V1;
    owner->phase = RIN_GPU_PROCESS_QUEUE_OWNER_PHASE_EMPTY;
}

/* Bind the exact device generation and admitted queue. A failed bind leaves
 * the output owner empty; the SDK wrapper rejects malformed zero-token
 * success responses rather than inventing a local token. */
static inline RinResult rin_gpu_process_queue_owner_v1_create(
    uint64_t device_id, uint64_t device_generation, uint32_t queue_id,
    RinGpuProcessQueueOwnerV1* owner_out)
{
    RinGpuProcessQueueOwnerV1 candidate;
    RinGpuProcessQueueV1 queue_token = 0u;
    RinResult result;

    if (!owner_out || !rin_gpu_process_queue_owner_v1_valid(owner_out) ||
        owner_out->phase != RIN_GPU_PROCESS_QUEUE_OWNER_PHASE_EMPTY ||
        device_id == 0u || device_generation == 0u ||
        queue_id >= RIN_GPU_PROCESS_MAX_QUEUES_V1)
        return RIN_ERROR_INVALID_ARGUMENT;

    result = rin_gpu_process_queue_bind_v1(
        device_id, device_generation, queue_id, &queue_token);
    if (result != RIN_SUCCESS) return result;
    if (queue_token == 0u) return RIN_ERROR_ABI_MISMATCH;

    rin_gpu_process_queue_owner_v1_init(&candidate);
    candidate.phase = RIN_GPU_PROCESS_QUEUE_OWNER_PHASE_ACTIVE;
    candidate.queue_id = queue_id;
    candidate.device_id = device_id;
    candidate.device_generation = device_generation;
    candidate.queue_token = queue_token;
    *owner_out = candidate;
    return RIN_SUCCESS;
}

/* Destruction marks intent before releasing. Any release error leaves the
 * exact token and identity cleanup-pending for a safe retry. */
static inline RinResult rin_gpu_process_queue_owner_v1_destroy(
    RinGpuProcessQueueOwnerV1* owner)
{
    RinResult result;
    if (!rin_gpu_process_queue_owner_v1_valid(owner) ||
        (owner->phase != RIN_GPU_PROCESS_QUEUE_OWNER_PHASE_ACTIVE &&
         owner->phase !=
             RIN_GPU_PROCESS_QUEUE_OWNER_PHASE_CLEANUP_PENDING))
        return RIN_ERROR_INVALID_ARGUMENT;
    owner->phase = RIN_GPU_PROCESS_QUEUE_OWNER_PHASE_CLEANUP_PENDING;
    result = rin_gpu_process_queue_release_v1(owner->queue_token);
    if (result == RIN_SUCCESS) rin_gpu_process_queue_owner_v1_init(owner);
    return result;
}

#if defined(__cplusplus)
static_assert(sizeof(RinGpuProcessQueueOwnerV1) == 48u,
              "RinGpuProcessQueueOwnerV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinGpuProcessQueueOwnerV1) == 48u,
               "RinGpuProcessQueueOwnerV1 ABI drift");
#endif

#endif /* RIN_SDK_GPU_PROCESS_QUEUE_OWNER_V1_H */
