/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_DRIVER_NPU_V1_H
#define RIN_DRIVER_NPU_V1_H

#include <stddef.h>
#include <stdint.h>

#include <rin/driver/module_api_v3.h>

/* NPU is an independent compute device class. Its command and buffer ABI is
 * separate from display/GPU drawing operations. All handles in this contract
 * are opaque and scoped to one kernel-created context. */
#define RIN_DRIVER_V3_NPU_ABI_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_NPU_OPS_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_NPU_MAX_SUBMIT_BUFFERS UINT32_C(64)
#define RIN_DRIVER_V3_NPU_MAX_EVENTS UINT32_C(128)

#define RIN_DRIVER_V3_NPU_DATA_TYPE_INT8 UINT64_C(0x0000000000000001)
#define RIN_DRIVER_V3_NPU_DATA_TYPE_UINT8 UINT64_C(0x0000000000000002)
#define RIN_DRIVER_V3_NPU_DATA_TYPE_INT16 UINT64_C(0x0000000000000004)
#define RIN_DRIVER_V3_NPU_DATA_TYPE_UINT16 UINT64_C(0x0000000000000008)
#define RIN_DRIVER_V3_NPU_DATA_TYPE_INT32 UINT64_C(0x0000000000000010)
#define RIN_DRIVER_V3_NPU_DATA_TYPE_UINT32 UINT64_C(0x0000000000000020)
#define RIN_DRIVER_V3_NPU_DATA_TYPE_FLOAT16 UINT64_C(0x0000000000000040)
#define RIN_DRIVER_V3_NPU_DATA_TYPE_BFLOAT16 UINT64_C(0x0000000000000080)
#define RIN_DRIVER_V3_NPU_DATA_TYPE_FLOAT32 UINT64_C(0x0000000000000100)
#define RIN_DRIVER_V3_NPU_DATA_TYPE_INT64 UINT64_C(0x0000000000000200)
#define RIN_DRIVER_V3_NPU_DATA_TYPE_UINT64 UINT64_C(0x0000000000000400)
#define RIN_DRIVER_V3_NPU_DATA_TYPE_FLOAT64 UINT64_C(0x0000000000000800)
#define RIN_DRIVER_V3_NPU_DATA_TYPE_MASK UINT64_C(0x0000000000000fff)

#define RIN_DRIVER_V3_NPU_CAP_DMA_ISOLATED UINT64_C(0x0000000000000001)
#define RIN_DRIVER_V3_NPU_CAP_HOST_IMPORT UINT64_C(0x0000000000000002)
#define RIN_DRIVER_V3_NPU_CAP_PREEMPT UINT64_C(0x0000000000000004)
#define RIN_DRIVER_V3_NPU_CAP_CANCEL UINT64_C(0x0000000000000008)
#define RIN_DRIVER_V3_NPU_CAP_FAULT_EVENTS UINT64_C(0x0000000000000010)
#define RIN_DRIVER_V3_NPU_CAP_RESET UINT64_C(0x0000000000000020)
#define RIN_DRIVER_V3_NPU_CAP_POWER_CONTROL UINT64_C(0x0000000000000040)
#define RIN_DRIVER_V3_NPU_CAPABILITY_MASK UINT64_C(0x000000000000007f)

#define RIN_DRIVER_V3_NPU_BUFFER_READ UINT32_C(0x00000001)
#define RIN_DRIVER_V3_NPU_BUFFER_WRITE UINT32_C(0x00000002)
#define RIN_DRIVER_V3_NPU_BUFFER_ACCESS_MASK UINT32_C(0x00000003)
#define RIN_DRIVER_V3_NPU_BUFFER_DEVICE_LOCAL UINT32_C(0x00000001)
#define RIN_DRIVER_V3_NPU_BUFFER_HOST_SHARED UINT32_C(0x00000002)
#define RIN_DRIVER_V3_NPU_BUFFER_MEMORY_MASK UINT32_C(0x00000003)

#define RIN_DRIVER_V3_NPU_QUEUE_LOW_LATENCY UINT32_C(0x00000001)
#define RIN_DRIVER_V3_NPU_QUEUE_PREEMPTIBLE UINT32_C(0x00000002)
#define RIN_DRIVER_V3_NPU_QUEUE_FLAG_MASK UINT32_C(0x00000003)

#define RIN_DRIVER_V3_NPU_SUBMIT_PREEMPTIBLE UINT32_C(0x00000001)
#define RIN_DRIVER_V3_NPU_SUBMIT_FLAG_MASK UINT32_C(0x00000001)

#define RIN_DRIVER_V3_NPU_EVENT_FENCE_COMPLETE UINT32_C(1)
#define RIN_DRIVER_V3_NPU_EVENT_FAULT UINT32_C(2)
#define RIN_DRIVER_V3_NPU_EVENT_DEVICE_LOST UINT32_C(3)
#define RIN_DRIVER_V3_NPU_EVENT_QUEUE_OVERFLOW UINT32_C(4)
#define RIN_DRIVER_V3_NPU_EVENT_TYPE_MAX RIN_DRIVER_V3_NPU_EVENT_QUEUE_OVERFLOW

#define RIN_DRIVER_V3_NPU_FAULT_NONE UINT32_C(0)
#define RIN_DRIVER_V3_NPU_FAULT_DMA_PERMISSION UINT32_C(1)
#define RIN_DRIVER_V3_NPU_FAULT_PAGE_NOT_PRESENT UINT32_C(2)
#define RIN_DRIVER_V3_NPU_FAULT_COMMAND_INVALID UINT32_C(3)
#define RIN_DRIVER_V3_NPU_FAULT_DEVICE_HANG UINT32_C(4)
#define RIN_DRIVER_V3_NPU_FAULT_CODE_MAX RIN_DRIVER_V3_NPU_FAULT_DEVICE_HANG

#define RIN_DRIVER_V3_NPU_POWER_ON UINT32_C(1)
#define RIN_DRIVER_V3_NPU_POWER_LOW UINT32_C(2)
#define RIN_DRIVER_V3_NPU_POWER_OFF UINT32_C(3)
#define RIN_DRIVER_V3_NPU_POWER_STATE_MAX RIN_DRIVER_V3_NPU_POWER_OFF

/* Stable device identity returned by the NPU driver. device_generation is
 * supplied by the kernel and must match the active NDRV session generation. */
typedef struct RinDriverV3NpuIdentityV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    uint32_t vendor_id;
    uint32_t device_id;
    uint32_t subsystem_vendor_id;
    uint32_t subsystem_device_id;
    uint32_t family_id;
    uint32_t revision_id;
    uint32_t engine_count;
    uint32_t reserved0;
    uint64_t device_generation;
    uint64_t firmware_version;
    uint8_t hardware_uuid[16];
    uint64_t reserved[2];
} RinDriverV3NpuIdentityV1;

typedef struct RinDriverV3NpuCapabilitiesV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t flags;
    uint32_t max_contexts;
    uint32_t max_queues_per_context;
    uint32_t max_buffers_per_context;
    uint32_t max_inflight_submissions;
    uint32_t max_fault_events;
    uint32_t max_dma_segments;
    uint32_t max_command_bytes;
    uint32_t address_bits;
    uint64_t max_buffer_bytes;
    uint64_t supported_data_types;
    uint64_t reserved[2];
} RinDriverV3NpuCapabilitiesV1;

/* The kernel broker creates this record from the authenticated caller and
 * never copies it from user memory. process_token is an unforgeable broker
 * handle; process_generation prevents PID reuse and context_generation is
 * freshly minted for each context. Drivers associate all child objects and
 * fences with this owner and reject cross-context references. */
typedef struct RinDriverV3NpuContextOwnerV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t process_token;
    uint64_t process_generation;
    uint64_t context_generation;
    uint64_t reserved[2];
} RinDriverV3NpuContextOwnerV1;

typedef struct RinDriverV3NpuContextConfigV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t memory_budget_bytes;
    uint32_t max_queues;
    uint32_t priority;
    uint32_t flags;
    uint32_t reserved0;
    uint64_t reserved[2];
} RinDriverV3NpuContextConfigV1;

typedef struct RinDriverV3NpuBufferCreateV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t size_bytes;
    uint64_t alignment_bytes;
    uint32_t access;
    uint32_t memory_type;
    uint32_t dma_flags;
    uint32_t reserved0;
    /* A host-memory token is issued by the kernel memory broker for the
     * owner process; it is not a user virtual address or physical address. */
    uint64_t host_memory_token;
    uint64_t host_memory_generation;
    uint64_t reserved[2];
} RinDriverV3NpuBufferCreateV1;

typedef struct RinDriverV3NpuBufferInfoV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t driver_handle;
    uint64_t driver_generation;
    uint64_t size_bytes;
    uint32_t access;
    uint32_t memory_type;
    uint64_t reserved[2];
} RinDriverV3NpuBufferInfoV1;

typedef struct RinDriverV3NpuQueueCreateV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t depth;
    uint32_t priority;
    uint32_t flags;
    uint32_t engine_id;
    uint64_t reserved[2];
} RinDriverV3NpuQueueCreateV1;

typedef struct RinDriverV3NpuBufferRefV1 {
    uint64_t buffer_handle;
    uint64_t buffer_generation;
    uint64_t offset_bytes;
    uint64_t length_bytes;
    uint32_t access;
    uint32_t reserved;
} RinDriverV3NpuBufferRefV1;

typedef struct RinDriverV3NpuSubmitV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    uint32_t resource_count;
    uint64_t command_buffer_handle;
    uint64_t command_buffer_generation;
    uint64_t command_offset_bytes;
    uint64_t command_length_bytes;
    uint64_t user_tag;
    uint64_t reserved[2];
} RinDriverV3NpuSubmitV1;

typedef struct RinDriverV3NpuFenceV1 {
    uint64_t value;
    uint64_t generation;
} RinDriverV3NpuFenceV1;

/* Faults name an opaque buffer and an offset within it. Raw IOVA/physical
 * addresses are never exposed across the NPU process boundary. */
typedef struct RinDriverV3NpuEventV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t type;
    uint32_t flags;
    int32_t result;
    uint32_t reserved0;
    RinDriverV3NpuFenceV1 fence;
    uint64_t user_tag;
    uint64_t buffer_handle;
    uint64_t buffer_generation;
    uint64_t buffer_offset_bytes;
    uint64_t fault_length_bytes;
    uint32_t fault_code;
    uint32_t reserved1;
    uint64_t reserved[1];
} RinDriverV3NpuEventV1;

typedef struct RinDriverV3NpuResetV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    uint32_t timeout_ms;
    uint64_t context_handle;
    uint64_t context_generation;
    uint64_t reserved[2];
} RinDriverV3NpuResetV1;

typedef struct RinDriverV3NpuPowerV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t state;
    uint32_t flags;
    uint64_t reserved[2];
} RinDriverV3NpuPowerV1;

typedef RinDriverV3Result (*RinDriverV3NpuGetIdentityFn)(
    uint64_t driver_context, RinDriverV3NpuIdentityV1* identity);
typedef RinDriverV3Result (*RinDriverV3NpuGetCapabilitiesFn)(
    uint64_t driver_context, RinDriverV3NpuCapabilitiesV1* capabilities);
typedef RinDriverV3Result (*RinDriverV3NpuContextCreateFn)(
    uint64_t driver_context, const RinDriverV3NpuContextOwnerV1* owner,
    const RinDriverV3NpuContextConfigV1* config,
    uint64_t* context_handle, uint64_t* context_generation);
typedef RinDriverV3Result (*RinDriverV3NpuContextDestroyFn)(
    uint64_t driver_context, uint64_t context_handle,
    uint64_t context_generation);
typedef RinDriverV3Result (*RinDriverV3NpuBufferCreateFn)(
    uint64_t driver_context, uint64_t context_handle,
    uint64_t context_generation,
    const RinDriverV3NpuBufferCreateV1* request,
    RinDriverV3NpuBufferInfoV1* buffer);
typedef RinDriverV3Result (*RinDriverV3NpuBufferDestroyFn)(
    uint64_t driver_context, uint64_t context_handle,
    uint64_t context_generation, uint64_t buffer_handle,
    uint64_t buffer_generation);
typedef RinDriverV3Result (*RinDriverV3NpuQueueCreateFn)(
    uint64_t driver_context, uint64_t context_handle,
    uint64_t context_generation,
    const RinDriverV3NpuQueueCreateV1* request,
    uint64_t* queue_handle, uint64_t* queue_generation);
typedef RinDriverV3Result (*RinDriverV3NpuQueueDestroyFn)(
    uint64_t driver_context, uint64_t context_handle,
    uint64_t context_generation, uint64_t queue_handle,
    uint64_t queue_generation);
typedef RinDriverV3Result (*RinDriverV3NpuSubmitFn)(
    uint64_t driver_context, uint64_t context_handle,
    uint64_t context_generation, uint64_t queue_handle,
    uint64_t queue_generation, const RinDriverV3NpuSubmitV1* request,
    const RinDriverV3NpuBufferRefV1* resources, uint32_t resource_count,
    RinDriverV3NpuFenceV1* fence);
typedef RinDriverV3Result (*RinDriverV3NpuPollEventsFn)(
    uint64_t driver_context, uint64_t context_handle,
    uint64_t context_generation, RinDriverV3NpuEventV1* events,
    uint32_t capacity, uint32_t* event_count);
typedef RinDriverV3Result (*RinDriverV3NpuCancelFn)(
    uint64_t driver_context, uint64_t context_handle,
    uint64_t context_generation, RinDriverV3NpuFenceV1 fence);
typedef RinDriverV3Result (*RinDriverV3NpuResetFn)(
    uint64_t driver_context, const RinDriverV3NpuResetV1* request);
typedef RinDriverV3Result (*RinDriverV3NpuSetPowerFn)(
    uint64_t driver_context, const RinDriverV3NpuPowerV1* request);

typedef struct RinDriverV3NpuOps {
    uint32_t struct_size;
    uint32_t version;
    RinDriverV3NpuGetIdentityFn get_identity;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t get_identity_padding;
#endif
    RinDriverV3NpuGetCapabilitiesFn get_capabilities;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t get_capabilities_padding;
#endif
    RinDriverV3NpuContextCreateFn context_create;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t context_create_padding;
#endif
    RinDriverV3NpuContextDestroyFn context_destroy;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t context_destroy_padding;
#endif
    RinDriverV3NpuBufferCreateFn buffer_create;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t buffer_create_padding;
#endif
    RinDriverV3NpuBufferDestroyFn buffer_destroy;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t buffer_destroy_padding;
#endif
    RinDriverV3NpuQueueCreateFn queue_create;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t queue_create_padding;
#endif
    RinDriverV3NpuQueueDestroyFn queue_destroy;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t queue_destroy_padding;
#endif
    RinDriverV3NpuSubmitFn submit;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t submit_padding;
#endif
    RinDriverV3NpuPollEventsFn poll_events;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t poll_events_padding;
#endif
    RinDriverV3NpuCancelFn cancel;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t cancel_padding;
#endif
    RinDriverV3NpuResetFn reset;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t reset_padding;
#endif
    RinDriverV3NpuSetPowerFn set_power;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t set_power_padding;
#endif
    RinDriverV3DeviceRemoveFn remove;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t remove_padding;
#endif
    uint64_t reserved[4];
} RinDriverV3NpuOps;

static inline int rin_driver_v3_npu_identity_valid(
    const RinDriverV3NpuIdentityV1* identity)
{
    if (!identity || identity->struct_size != sizeof(*identity) ||
        identity->version != RIN_DRIVER_V3_NPU_ABI_VERSION ||
        identity->flags != 0u || identity->vendor_id == 0u ||
        identity->device_id == 0u ||
        identity->engine_count == 0u || identity->device_generation == 0u ||
        identity->reserved0 != 0u || identity->reserved[0] != 0u ||
        identity->reserved[1] != 0u)
        return 0;
    return 1;
}

static inline int rin_driver_v3_npu_capabilities_valid(
    const RinDriverV3NpuCapabilitiesV1* capabilities)
{
    return capabilities && capabilities->struct_size == sizeof(*capabilities) &&
           capabilities->version == RIN_DRIVER_V3_NPU_ABI_VERSION &&
           (capabilities->flags & ~RIN_DRIVER_V3_NPU_CAPABILITY_MASK) == 0u &&
           capabilities->max_contexts != 0u &&
           capabilities->max_queues_per_context != 0u &&
           capabilities->max_buffers_per_context != 0u &&
           capabilities->max_inflight_submissions != 0u &&
           capabilities->max_fault_events <= RIN_DRIVER_V3_NPU_MAX_EVENTS &&
           capabilities->max_dma_segments != 0u &&
           capabilities->max_command_bytes != 0u &&
           capabilities->address_bits >= 32u &&
           capabilities->address_bits <= 64u &&
           capabilities->max_buffer_bytes != 0u &&
           capabilities->supported_data_types != 0u &&
           (capabilities->supported_data_types &
            ~RIN_DRIVER_V3_NPU_DATA_TYPE_MASK) == 0u &&
           (((capabilities->flags & RIN_DRIVER_V3_NPU_CAP_FAULT_EVENTS) !=
             0u) == (capabilities->max_fault_events != 0u)) &&
           capabilities->reserved[0] == 0u &&
           capabilities->reserved[1] == 0u;
}

static inline int rin_driver_v3_npu_context_owner_valid(
    const RinDriverV3NpuContextOwnerV1* owner)
{
    return owner && owner->struct_size == sizeof(*owner) &&
           owner->version == RIN_DRIVER_V3_NPU_ABI_VERSION &&
           owner->process_token != 0u && owner->process_generation != 0u &&
           owner->context_generation != 0u && owner->reserved[0] == 0u &&
           owner->reserved[1] == 0u;
}

static inline int rin_driver_v3_npu_context_config_valid(
    const RinDriverV3NpuContextConfigV1* config)
{
    return config && config->struct_size == sizeof(*config) &&
           config->version == RIN_DRIVER_V3_NPU_ABI_VERSION &&
           config->memory_budget_bytes != 0u && config->max_queues != 0u &&
           config->priority <= 7u && config->flags == 0u &&
           config->reserved0 == 0u && config->reserved[0] == 0u &&
           config->reserved[1] == 0u;
}

static inline int rin_driver_v3_npu_buffer_create_valid(
    const RinDriverV3NpuBufferCreateV1* request)
{
    if (!request || request->struct_size != sizeof(*request) ||
        request->version != RIN_DRIVER_V3_NPU_ABI_VERSION ||
        request->size_bytes == 0u || request->alignment_bytes == 0u ||
        (request->alignment_bytes & (request->alignment_bytes - 1u)) != 0u ||
        request->access == 0u ||
        (request->access & ~RIN_DRIVER_V3_NPU_BUFFER_ACCESS_MASK) != 0u ||
        (request->memory_type != RIN_DRIVER_V3_NPU_BUFFER_DEVICE_LOCAL &&
         request->memory_type != RIN_DRIVER_V3_NPU_BUFFER_HOST_SHARED) ||
        (request->dma_flags & ~RIN_DRIVER_V3_DMA_FLAG_MASK) != 0u ||
        request->reserved0 != 0u || request->reserved[0] != 0u ||
        request->reserved[1] != 0u)
        return 0;
    if (request->memory_type == RIN_DRIVER_V3_NPU_BUFFER_HOST_SHARED)
        return request->host_memory_token != 0u &&
               request->host_memory_generation != 0u;
    return request->host_memory_token == 0u &&
           request->host_memory_generation == 0u;
}

static inline int rin_driver_v3_npu_buffer_info_valid(
    const RinDriverV3NpuBufferInfoV1* buffer)
{
    return buffer && buffer->struct_size == sizeof(*buffer) &&
           buffer->version == RIN_DRIVER_V3_NPU_ABI_VERSION &&
           buffer->driver_handle != 0u && buffer->driver_generation != 0u &&
           buffer->size_bytes != 0u && buffer->access != 0u &&
           (buffer->access & ~RIN_DRIVER_V3_NPU_BUFFER_ACCESS_MASK) == 0u &&
           (buffer->memory_type == RIN_DRIVER_V3_NPU_BUFFER_DEVICE_LOCAL ||
            buffer->memory_type == RIN_DRIVER_V3_NPU_BUFFER_HOST_SHARED) &&
           buffer->reserved[0] == 0u && buffer->reserved[1] == 0u;
}

static inline int rin_driver_v3_npu_queue_create_valid(
    const RinDriverV3NpuQueueCreateV1* request)
{
    return request && request->struct_size == sizeof(*request) &&
           request->version == RIN_DRIVER_V3_NPU_ABI_VERSION &&
           request->depth != 0u && request->priority <= 7u &&
           (request->flags & ~RIN_DRIVER_V3_NPU_QUEUE_FLAG_MASK) == 0u &&
           request->reserved[0] == 0u && request->reserved[1] == 0u;
}

static inline int rin_driver_v3_npu_buffer_ref_valid(
    const RinDriverV3NpuBufferRefV1* reference)
{
    return reference && reference->buffer_handle != 0u &&
           reference->buffer_generation != 0u &&
           reference->length_bytes != 0u && reference->access != 0u &&
           (reference->access & ~RIN_DRIVER_V3_NPU_BUFFER_ACCESS_MASK) == 0u &&
           reference->reserved == 0u &&
           reference->offset_bytes <= UINT64_MAX - reference->length_bytes;
}

static inline int rin_driver_v3_npu_submit_valid(
    const RinDriverV3NpuSubmitV1* request,
    const RinDriverV3NpuBufferRefV1* resources, uint32_t resource_count)
{
    if (!request || request->struct_size != sizeof(*request) ||
        request->version != RIN_DRIVER_V3_NPU_ABI_VERSION ||
        (request->flags & ~RIN_DRIVER_V3_NPU_SUBMIT_FLAG_MASK) != 0u ||
        request->resource_count != resource_count ||
        resource_count > RIN_DRIVER_V3_NPU_MAX_SUBMIT_BUFFERS ||
        (resource_count != 0u && !resources) ||
        request->command_buffer_handle == 0u ||
        request->command_buffer_generation == 0u ||
        request->command_length_bytes == 0u ||
        request->command_offset_bytes > UINT64_MAX - request->command_length_bytes ||
        request->reserved[0] != 0u || request->reserved[1] != 0u)
        return 0;
    for (uint32_t index = 0u; index < resource_count; ++index)
        if (!rin_driver_v3_npu_buffer_ref_valid(&resources[index])) return 0;
    return 1;
}

static inline int rin_driver_v3_npu_fence_valid(
    const RinDriverV3NpuFenceV1* fence)
{
    return fence && fence->value != 0u && fence->generation != 0u;
}

static inline int rin_driver_v3_npu_event_valid(
    const RinDriverV3NpuEventV1* event)
{
    if (!event || event->struct_size != sizeof(*event) ||
        event->version != RIN_DRIVER_V3_NPU_ABI_VERSION ||
        event->type < RIN_DRIVER_V3_NPU_EVENT_FENCE_COMPLETE ||
        event->type > RIN_DRIVER_V3_NPU_EVENT_TYPE_MAX || event->flags != 0u ||
        event->reserved0 != 0u || event->reserved1 != 0u ||
        event->reserved[0] != 0u)
        return 0;
    if (event->type == RIN_DRIVER_V3_NPU_EVENT_FENCE_COMPLETE)
        return rin_driver_v3_npu_fence_valid(&event->fence) &&
               event->fault_code == RIN_DRIVER_V3_NPU_FAULT_NONE &&
               event->buffer_handle == 0u &&
               event->buffer_generation == 0u &&
               event->buffer_offset_bytes == 0u &&
               event->fault_length_bytes == 0u;
    if (event->type == RIN_DRIVER_V3_NPU_EVENT_FAULT)
        return event->fault_code != RIN_DRIVER_V3_NPU_FAULT_NONE &&
               event->fault_code <= RIN_DRIVER_V3_NPU_FAULT_CODE_MAX &&
               event->buffer_handle != 0u && event->buffer_generation != 0u &&
               event->fault_length_bytes != 0u &&
               event->buffer_offset_bytes <= UINT64_MAX - event->fault_length_bytes;
    return event->type == RIN_DRIVER_V3_NPU_EVENT_DEVICE_LOST ||
           event->type == RIN_DRIVER_V3_NPU_EVENT_QUEUE_OVERFLOW;
}

static inline int rin_driver_v3_npu_reset_valid(
    const RinDriverV3NpuResetV1* request)
{
    return request && request->struct_size == sizeof(*request) &&
           request->version == RIN_DRIVER_V3_NPU_ABI_VERSION &&
           request->flags == 0u && request->timeout_ms != 0u &&
           request->reserved[0] == 0u && request->reserved[1] == 0u &&
           ((request->context_handle == 0u &&
             request->context_generation == 0u) ||
            (request->context_handle != 0u &&
             request->context_generation != 0u));
}

static inline int rin_driver_v3_npu_power_valid(
    const RinDriverV3NpuPowerV1* request)
{
    return request && request->struct_size == sizeof(*request) &&
           request->version == RIN_DRIVER_V3_NPU_ABI_VERSION &&
           request->state >= RIN_DRIVER_V3_NPU_POWER_ON &&
           request->state <= RIN_DRIVER_V3_NPU_POWER_STATE_MAX &&
           request->flags == 0u && request->reserved[0] == 0u &&
           request->reserved[1] == 0u;
}

static inline int rin_driver_v3_npu_ops_valid(
    const RinDriverV3NpuOps* ops)
{
    if (!ops || ops->struct_size != sizeof(*ops) ||
        ops->version != RIN_DRIVER_V3_NPU_OPS_VERSION ||
        !ops->get_identity || !ops->get_capabilities || !ops->context_create ||
        !ops->context_destroy || !ops->buffer_create || !ops->buffer_destroy ||
        !ops->queue_create || !ops->queue_destroy || !ops->submit ||
        !ops->poll_events || !ops->cancel || !ops->reset || !ops->set_power ||
        !ops->remove || ops->reserved[0] != 0u || ops->reserved[1] != 0u ||
        ops->reserved[2] != 0u || ops->reserved[3] != 0u)
        return 0;
#if UINTPTR_MAX == UINT32_MAX
    if (ops->get_identity_padding != 0u ||
        ops->get_capabilities_padding != 0u ||
        ops->context_create_padding != 0u ||
        ops->context_destroy_padding != 0u ||
        ops->buffer_create_padding != 0u ||
        ops->buffer_destroy_padding != 0u ||
        ops->queue_create_padding != 0u ||
        ops->queue_destroy_padding != 0u || ops->submit_padding != 0u ||
        ops->poll_events_padding != 0u || ops->cancel_padding != 0u ||
        ops->reset_padding != 0u || ops->set_power_padding != 0u ||
        ops->remove_padding != 0u)
        return 0;
#endif
    return 1;
}

#if defined(__cplusplus)
static_assert(sizeof(RinDriverV3NpuIdentityV1) == 96u,
              "NDRV v3 NPU identity ABI drift");
static_assert(sizeof(RinDriverV3NpuCapabilitiesV1) == 80u,
              "NDRV v3 NPU capabilities ABI drift");
static_assert(sizeof(RinDriverV3NpuContextOwnerV1) == 48u,
              "NDRV v3 NPU context owner ABI drift");
static_assert(sizeof(RinDriverV3NpuContextConfigV1) == 48u,
              "NDRV v3 NPU context config ABI drift");
static_assert(sizeof(RinDriverV3NpuBufferCreateV1) == 72u,
              "NDRV v3 NPU buffer create ABI drift");
static_assert(sizeof(RinDriverV3NpuBufferInfoV1) == 56u,
              "NDRV v3 NPU buffer info ABI drift");
static_assert(sizeof(RinDriverV3NpuQueueCreateV1) == 40u,
              "NDRV v3 NPU queue create ABI drift");
static_assert(sizeof(RinDriverV3NpuBufferRefV1) == 40u,
              "NDRV v3 NPU buffer reference ABI drift");
static_assert(sizeof(RinDriverV3NpuSubmitV1) == 72u,
              "NDRV v3 NPU submission ABI drift");
static_assert(sizeof(RinDriverV3NpuEventV1) == 96u,
              "NDRV v3 NPU event ABI drift");
static_assert(sizeof(RinDriverV3NpuOps) == 152u,
              "NDRV v3 NPU operations ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinDriverV3NpuIdentityV1) == 96u,
               "NDRV v3 NPU identity ABI drift");
_Static_assert(sizeof(RinDriverV3NpuCapabilitiesV1) == 80u,
               "NDRV v3 NPU capabilities ABI drift");
_Static_assert(sizeof(RinDriverV3NpuContextOwnerV1) == 48u,
               "NDRV v3 NPU context owner ABI drift");
_Static_assert(sizeof(RinDriverV3NpuContextConfigV1) == 48u,
               "NDRV v3 NPU context config ABI drift");
_Static_assert(sizeof(RinDriverV3NpuBufferCreateV1) == 72u,
               "NDRV v3 NPU buffer create ABI drift");
_Static_assert(sizeof(RinDriverV3NpuBufferInfoV1) == 56u,
               "NDRV v3 NPU buffer info ABI drift");
_Static_assert(sizeof(RinDriverV3NpuQueueCreateV1) == 40u,
               "NDRV v3 NPU queue create ABI drift");
_Static_assert(sizeof(RinDriverV3NpuBufferRefV1) == 40u,
               "NDRV v3 NPU buffer reference ABI drift");
_Static_assert(sizeof(RinDriverV3NpuSubmitV1) == 72u,
               "NDRV v3 NPU submission ABI drift");
_Static_assert(sizeof(RinDriverV3NpuEventV1) == 96u,
               "NDRV v3 NPU event ABI drift");
_Static_assert(sizeof(RinDriverV3NpuOps) == 152u,
               "NDRV v3 NPU operations ABI drift");
#endif

#endif /* RIN_DRIVER_NPU_V1_H */
