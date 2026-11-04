/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_DRIVER_NPU_V1_H
#define RIN_DRIVER_NPU_V1_H

#include <stddef.h>
#include <stdint.h>

#include <rin/driver/module_api_v3.h>

/* NPU is an independent compute device class. Its command and buffer ABI is
 * separate from display/GPU drawing operations. All handles in this contract
 * are opaque and scoped to one kernel-created context. ABI 1.2 adds the
 * bounded AMD XDNA command and process-SVA host-import callbacks. */
#define RIN_DRIVER_V3_NPU_ABI_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_NPU_ABI_VERSION_1_1 UINT32_C(0x00010001)
#define RIN_DRIVER_V3_NPU_ABI_VERSION_1_2 UINT32_C(0x00010002)
#define RIN_DRIVER_V3_NPU_OPS_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_NPU_OPS_VERSION_1_1 UINT32_C(0x00010001)
#define RIN_DRIVER_V3_NPU_OPS_VERSION_1_2 UINT32_C(0x00010002)
#define RIN_DRIVER_V3_NPU_BUFFER_INFO_V1_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_NPU_BUFFER_INFO_DEVICE_ADDRESS_VERSION \
    UINT32_C(0x00010001)
#define RIN_DRIVER_V3_NPU_AMDXDNA_COMMAND_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_NPU_AMDXDNA_COMMAND_VERSION_1_1 UINT32_C(0x00010001)
#define RIN_DRIVER_V3_NPU_AMDXDNA_COMMAND_PARTIAL_ELF UINT32_C(2)
#define RIN_DRIVER_V3_NPU_AMDXDNA_COMMAND_DPU_CHAIN UINT32_C(3)
#define RIN_DRIVER_V3_NPU_AMDXDNA_MAX_COMMAND_ARGUMENTS UINT32_C(26)
#define RIN_DRIVER_V3_NPU_AMDXDNA_MAX_CHAIN_ENTRIES UINT32_C(16)
#define RIN_DRIVER_V3_NPU_AMDXDNA_MAX_CHAIN_COMMAND_BYTES UINT32_C(2072)
#define RIN_DRIVER_V3_NPU_AMDXDNA_HOST_BUFFER_CHUNK_BYTES UINT32_C(32768)
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
#define RIN_DRIVER_V3_NPU_CAP_DEVICE_ADDRESS UINT64_C(0x0000000000000080)
#define RIN_DRIVER_V3_NPU_CAP_PROCESS_PASID UINT64_C(0x0000000000000100)
#define RIN_DRIVER_V3_NPU_CAP_SVA_HOST_IMPORT UINT64_C(0x0000000000000200)
#define RIN_DRIVER_V3_NPU_CAPABILITY_MASK UINT64_C(0x00000000000003ff)
#define RIN_DRIVER_V3_NPU_CONTEXT_OWNER_HAS_PASID UINT32_C(0x00000001)

/* NPU ABI 1.1 uses capabilities.reserved[0] as an operation bitmap. The
 * structure size and every existing field offset stay unchanged. ABI 1.0
 * retains its original contract where every operation is mandatory. */
#define RIN_DRIVER_V3_NPU_OP_CONTEXTS UINT64_C(0x0000000000000001)
#define RIN_DRIVER_V3_NPU_OP_BUFFERS UINT64_C(0x0000000000000002)
#define RIN_DRIVER_V3_NPU_OP_QUEUES UINT64_C(0x0000000000000004)
#define RIN_DRIVER_V3_NPU_OP_SUBMIT UINT64_C(0x0000000000000008)
#define RIN_DRIVER_V3_NPU_OP_CANCEL UINT64_C(0x0000000000000010)
#define RIN_DRIVER_V3_NPU_OP_RESET UINT64_C(0x0000000000000020)
#define RIN_DRIVER_V3_NPU_OP_SET_POWER UINT64_C(0x0000000000000040)
#define RIN_DRIVER_V3_NPU_OP_MASK UINT64_C(0x000000000000007f)

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
#if defined(__cplusplus)
#define RIN_DRIVER_NPU_ALIGNAS(n) alignas(n)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define RIN_DRIVER_NPU_ALIGNAS(n) _Alignas(n)
#elif defined(__GNUC__) || defined(__clang__)
#define RIN_DRIVER_NPU_ALIGNAS(n) __attribute__((aligned(n)))
#else
#define RIN_DRIVER_NPU_ALIGNAS(n)
#endif

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
    /* The identity ABI places 64-bit generations on an 8-byte boundary on
     * IA-32 as well as x86_64. Do not make this depend on -malign-double. */
    RIN_DRIVER_NPU_ALIGNAS(8) uint64_t device_generation;
    uint64_t firmware_version;
    uint8_t hardware_uuid[16];
    uint64_t reserved[2];
} RinDriverV3NpuIdentityV1;

#undef RIN_DRIVER_NPU_ALIGNAS

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
 * fences with this owner and reject cross-context references. A driver that
 * advertises PROCESS_PASID receives the kernel-issued PASID here; it must
 * never allocate or accept a PASID from a caller. */
typedef struct RinDriverV3NpuContextOwnerV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t process_token;
    uint64_t process_generation;
    uint64_t context_generation;
    uint32_t process_pasid;
    uint32_t flags;
    uint64_t reserved[1];
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
    /* Version 1.1 returns a per-context VPU virtual address. This is not a
     * host pointer, physical address, or DMA IOVA. The address is safe to use
     * in device command buffers only while the owning context is live. */
    uint64_t device_address;
    uint64_t reserved;
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

/* Kernel-copied AMD XDNA partial-ELF command payload. Buffer references stay
 * opaque in the public submit call; instruction_resource_index selects one
 * of that submit's checked references, and the driver resolves it to the
 * kernel-issued SVA mapping before constructing the firmware request. */
#if defined(__GNUC__) || defined(__clang__)
#define RIN_DRIVER_NPU_PACKED __attribute__((packed))
#else
#define RIN_DRIVER_NPU_PACKED
#endif
typedef struct RIN_DRIVER_NPU_PACKED RinDriverV3NpuAmdXdnaCommandV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t type;
    uint32_t engine_id;
    uint32_t instruction_resource_index;
    uint32_t instruction_property_count;
    uint32_t argument_count;
    uint32_t reserved0;
    uint64_t instruction_offset_bytes;
    uint32_t instruction_size_bytes;
    uint32_t reserved1;
    uint32_t arguments[RIN_DRIVER_V3_NPU_AMDXDNA_MAX_COMMAND_ARGUMENTS];
} RinDriverV3NpuAmdXdnaCommandV1;

/* Chain command bytes are stored in the mapped host buffer. The header is
 * followed by entry_count variable-size slots. Each slot consists of the
 * fixed header below and argument_count little-endian uint32 arguments, in
 * the same layout consumed by Phoenix MERT's CHAIN_EXEC_DPU request. */
typedef struct RIN_DRIVER_NPU_PACKED RinDriverV3NpuAmdXdnaChainCommandV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t type;
    uint32_t engine_id;
    uint32_t entry_count;
    uint32_t reserved0;
} RinDriverV3NpuAmdXdnaChainCommandV1;

typedef struct RIN_DRIVER_NPU_PACKED
    RinDriverV3NpuAmdXdnaChainSlotHeaderV1 {
    uint64_t instruction_buffer_address;
    uint32_t instruction_size_bytes;
    uint32_t instruction_property_count;
    uint32_t engine_id;
    uint32_t argument_count;
} RinDriverV3NpuAmdXdnaChainSlotHeaderV1;
#undef RIN_DRIVER_NPU_PACKED

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
/* Returning OK from context_destroy is the process-lifetime quiesce point:
 * the driver must stop that context's queues, retire outstanding fences, and
 * guarantee that no further DMA can reference its buffers. On failure the
 * kernel retains all host-memory pins and retries teardown. */
typedef RinDriverV3Result (*RinDriverV3NpuContextDestroyFn)(
    uint64_t driver_context, uint64_t context_handle,
    uint64_t context_generation);
typedef RinDriverV3Result (*RinDriverV3NpuBufferCreateFn)(
    uint64_t driver_context, uint64_t context_handle,
    uint64_t context_generation,
    const RinDriverV3NpuBufferCreateV1* request,
    RinDriverV3NpuBufferInfoV1* buffer);
/* SVA host import receives the registration base from the kernel memory
 * broker, never from a user-supplied buffer-create request. */
typedef RinDriverV3Result (*RinDriverV3NpuBufferCreateSvaFn)(
    uint64_t driver_context, uint64_t context_handle,
    uint64_t context_generation,
    const RinDriverV3NpuBufferCreateV1* request,
    uint64_t host_address, RinDriverV3NpuBufferInfoV1* buffer);
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
/* Version 1.2 operation for backends that translate a bounded, kernel-copied
 * command payload before sending it to device firmware. */
typedef RinDriverV3Result (*RinDriverV3NpuSubmitCommandFn)(
    uint64_t driver_context, uint64_t context_handle,
    uint64_t context_generation, uint64_t queue_handle,
    uint64_t queue_generation, const RinDriverV3NpuSubmitV1* request,
    const void* command_bytes, uint32_t command_length,
    const RinDriverV3NpuBufferRefV1* resources, uint32_t resource_count,
    RinDriverV3NpuFenceV1* fence);
typedef RinDriverV3Result (*RinDriverV3NpuPollEventsFn)(
    uint64_t driver_context, uint64_t context_handle,
    uint64_t context_generation, RinDriverV3NpuEventV1* events,
    uint32_t capacity, uint32_t* event_count);
typedef RinDriverV3Result (*RinDriverV3NpuCancelFn)(
    uint64_t driver_context, uint64_t context_handle,
    uint64_t context_generation, RinDriverV3NpuFenceV1 fence);
/* A successful context-scoped reset must quiesce that context's submissions
 * before returning; the kernel retires its outstanding fence tokens then. */
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
    RinDriverV3NpuBufferCreateSvaFn buffer_create_sva;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t buffer_create_sva_padding;
#endif
    RinDriverV3NpuSubmitCommandFn submit_command;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t submit_command_padding;
#endif
    uint64_t reserved[2];
} RinDriverV3NpuOps;

static inline int rin_driver_v3_npu_identity_valid(
    const RinDriverV3NpuIdentityV1* identity)
{
    if (!identity || identity->struct_size != sizeof(*identity) ||
        (identity->version != RIN_DRIVER_V3_NPU_ABI_VERSION &&
         identity->version != RIN_DRIVER_V3_NPU_ABI_VERSION_1_1 &&
         identity->version != RIN_DRIVER_V3_NPU_ABI_VERSION_1_2) ||
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
    if (!capabilities ||
        capabilities->struct_size != sizeof(*capabilities) ||
        (capabilities->version != RIN_DRIVER_V3_NPU_ABI_VERSION &&
         capabilities->version != RIN_DRIVER_V3_NPU_ABI_VERSION_1_1 &&
         capabilities->version != RIN_DRIVER_V3_NPU_ABI_VERSION_1_2) ||
        (capabilities->flags & ~RIN_DRIVER_V3_NPU_CAPABILITY_MASK) != 0u ||
        capabilities->max_fault_events > RIN_DRIVER_V3_NPU_MAX_EVENTS ||
        capabilities->address_bits < 32u ||
        capabilities->address_bits > 64u ||
        (capabilities->supported_data_types &
         ~RIN_DRIVER_V3_NPU_DATA_TYPE_MASK) != 0u ||
        (((capabilities->flags & RIN_DRIVER_V3_NPU_CAP_FAULT_EVENTS) !=
          0u) != (capabilities->max_fault_events != 0u)))
        return 0;
    if (capabilities->version == RIN_DRIVER_V3_NPU_ABI_VERSION)
        return capabilities->max_contexts != 0u &&
               capabilities->max_queues_per_context != 0u &&
               capabilities->max_buffers_per_context != 0u &&
               capabilities->max_inflight_submissions != 0u &&
               capabilities->max_dma_segments != 0u &&
               capabilities->max_command_bytes != 0u &&
               capabilities->max_buffer_bytes != 0u &&
               capabilities->supported_data_types != 0u &&
               capabilities->reserved[0] == 0u &&
               capabilities->reserved[1] == 0u;
    if ((capabilities->flags &
         RIN_DRIVER_V3_NPU_CAP_SVA_HOST_IMPORT) != 0u &&
        (capabilities->version !=
             RIN_DRIVER_V3_NPU_ABI_VERSION_1_2 ||
         (capabilities->flags &
             (RIN_DRIVER_V3_NPU_CAP_HOST_IMPORT |
              RIN_DRIVER_V3_NPU_CAP_PROCESS_PASID |
              RIN_DRIVER_V3_NPU_CAP_DEVICE_ADDRESS)) !=
             (RIN_DRIVER_V3_NPU_CAP_HOST_IMPORT |
              RIN_DRIVER_V3_NPU_CAP_PROCESS_PASID |
              RIN_DRIVER_V3_NPU_CAP_DEVICE_ADDRESS)))
        return 0;
    if ((capabilities->reserved[0] &
         ~RIN_DRIVER_V3_NPU_OP_MASK) != 0u ||
        capabilities->reserved[1] != 0u)
        return 0;
    {
        const uint64_t operations = capabilities->reserved[0];
        return ((operations & RIN_DRIVER_V3_NPU_OP_CONTEXTS) != 0u) ==
                   (capabilities->max_contexts != 0u) &&
               ((operations & RIN_DRIVER_V3_NPU_OP_QUEUES) != 0u) ==
                   (capabilities->max_queues_per_context != 0u) &&
               ((operations & RIN_DRIVER_V3_NPU_OP_BUFFERS) != 0u) ==
                   (capabilities->max_buffers_per_context != 0u) &&
               ((operations & RIN_DRIVER_V3_NPU_OP_SUBMIT) != 0u) ==
                   (capabilities->max_inflight_submissions != 0u) &&
               ((operations & RIN_DRIVER_V3_NPU_OP_BUFFERS) != 0u) ==
                   (capabilities->max_dma_segments != 0u) &&
               ((operations & RIN_DRIVER_V3_NPU_OP_SUBMIT) != 0u) ==
                   (capabilities->max_command_bytes != 0u) &&
               ((operations & RIN_DRIVER_V3_NPU_OP_BUFFERS) != 0u) ==
                   (capabilities->max_buffer_bytes != 0u) &&
               ((operations & RIN_DRIVER_V3_NPU_OP_SUBMIT) != 0u) ==
                   (capabilities->supported_data_types != 0u) &&
               (((capabilities->flags & RIN_DRIVER_V3_NPU_CAP_CANCEL) !=
                 0u) ==
                ((operations & RIN_DRIVER_V3_NPU_OP_CANCEL) != 0u)) &&
               (((capabilities->flags & RIN_DRIVER_V3_NPU_CAP_RESET) !=
                 0u) ==
                ((operations & RIN_DRIVER_V3_NPU_OP_RESET) != 0u)) &&
               (((capabilities->flags &
                  RIN_DRIVER_V3_NPU_CAP_POWER_CONTROL) != 0u) ==
                ((operations & RIN_DRIVER_V3_NPU_OP_SET_POWER) != 0u)) &&
               (((operations & RIN_DRIVER_V3_NPU_OP_BUFFERS) == 0u) ||
                ((operations & RIN_DRIVER_V3_NPU_OP_CONTEXTS) != 0u)) &&
               (((operations & RIN_DRIVER_V3_NPU_OP_QUEUES) == 0u) ||
                ((operations & RIN_DRIVER_V3_NPU_OP_CONTEXTS) != 0u)) &&
               (((operations & RIN_DRIVER_V3_NPU_OP_CONTEXTS) == 0u) ||
                ((operations & RIN_DRIVER_V3_NPU_OP_QUEUES) != 0u)) &&
               (((operations & RIN_DRIVER_V3_NPU_OP_SUBMIT) == 0u) ||
                ((operations & (RIN_DRIVER_V3_NPU_OP_CONTEXTS |
                                RIN_DRIVER_V3_NPU_OP_BUFFERS |
                                RIN_DRIVER_V3_NPU_OP_QUEUES)) ==
                 (RIN_DRIVER_V3_NPU_OP_CONTEXTS |
                  RIN_DRIVER_V3_NPU_OP_BUFFERS |
                  RIN_DRIVER_V3_NPU_OP_QUEUES))) &&
               (((capabilities->flags &
                  RIN_DRIVER_V3_NPU_CAP_PROCESS_PASID) == 0u) ||
                ((operations & RIN_DRIVER_V3_NPU_OP_CONTEXTS) != 0u)) &&
               (((capabilities->flags & RIN_DRIVER_V3_NPU_CAP_CANCEL) ==
                 0u) ||
                ((operations & RIN_DRIVER_V3_NPU_OP_SUBMIT) != 0u)) &&
               (((capabilities->flags & RIN_DRIVER_V3_NPU_CAP_RESET) ==
                 0u) ||
                ((operations & RIN_DRIVER_V3_NPU_OP_CONTEXTS) != 0u)) &&
               (((capabilities->flags &
                  (RIN_DRIVER_V3_NPU_CAP_HOST_IMPORT |
                   RIN_DRIVER_V3_NPU_CAP_DEVICE_ADDRESS)) == 0u) ||
                ((operations & RIN_DRIVER_V3_NPU_OP_BUFFERS) != 0u));
    }
}

static inline int rin_driver_v3_npu_context_owner_valid(
    const RinDriverV3NpuContextOwnerV1* owner)
{
    if (!owner || owner->struct_size != sizeof(*owner) ||
        owner->version != RIN_DRIVER_V3_NPU_ABI_VERSION ||
        owner->process_token == 0u || owner->process_generation == 0u ||
        owner->context_generation == 0u ||
        (owner->flags & ~RIN_DRIVER_V3_NPU_CONTEXT_OWNER_HAS_PASID) != 0u ||
        owner->reserved[0] != 0u)
        return 0;
    if ((owner->flags & RIN_DRIVER_V3_NPU_CONTEXT_OWNER_HAS_PASID) != 0u)
        return owner->process_pasid != 0u && owner->process_pasid <= 0xffffu;
    return owner->process_pasid == 0u;
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
    if (!buffer || buffer->struct_size != sizeof(*buffer) ||
        (buffer->version != RIN_DRIVER_V3_NPU_BUFFER_INFO_V1_VERSION &&
         buffer->version !=
             RIN_DRIVER_V3_NPU_BUFFER_INFO_DEVICE_ADDRESS_VERSION) ||
        buffer->driver_handle == 0u || buffer->driver_generation == 0u ||
        buffer->size_bytes == 0u || buffer->access == 0u ||
        (buffer->access & ~RIN_DRIVER_V3_NPU_BUFFER_ACCESS_MASK) != 0u ||
        (buffer->memory_type != RIN_DRIVER_V3_NPU_BUFFER_DEVICE_LOCAL &&
         buffer->memory_type != RIN_DRIVER_V3_NPU_BUFFER_HOST_SHARED) ||
        buffer->reserved != 0u)
        return 0;
    if (buffer->version == RIN_DRIVER_V3_NPU_BUFFER_INFO_V1_VERSION)
        return buffer->device_address == 0u;
    return buffer->device_address != 0u &&
           (buffer->device_address & UINT64_C(0xfff)) == 0u &&
           buffer->device_address <= UINT64_MAX - buffer->size_bytes;
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

static inline int rin_driver_v3_npu_amdxdna_command_valid(
    const RinDriverV3NpuAmdXdnaCommandV1* command,
    uint32_t command_length)
{
    if (!command || command_length < sizeof(uint32_t) * 3u)
        return 0;
    const uint8_t* bytes = (const uint8_t*)command;
    const uint32_t type = (uint32_t)bytes[8] |
        ((uint32_t)bytes[9] << 8u) | ((uint32_t)bytes[10] << 16u) |
        ((uint32_t)bytes[11] << 24u);
    if (type == RIN_DRIVER_V3_NPU_AMDXDNA_COMMAND_PARTIAL_ELF) {
        return command_length == sizeof(*command) &&
            command->struct_size == sizeof(*command) &&
            command->version == RIN_DRIVER_V3_NPU_AMDXDNA_COMMAND_VERSION &&
            command->engine_id < 32u && command->argument_count <=
                RIN_DRIVER_V3_NPU_AMDXDNA_MAX_COMMAND_ARGUMENTS &&
            command->instruction_size_bytes != 0u &&
            command->instruction_property_count <= command->argument_count &&
            command->instruction_offset_bytes <= UINT64_MAX -
                command->instruction_size_bytes &&
            command->reserved0 == 0u && command->reserved1 == 0u;
    }
    if (type == RIN_DRIVER_V3_NPU_AMDXDNA_COMMAND_DPU_CHAIN) {
        if (command_length < sizeof(RinDriverV3NpuAmdXdnaChainCommandV1) ||
            command_length >
                RIN_DRIVER_V3_NPU_AMDXDNA_MAX_CHAIN_COMMAND_BYTES)
            return 0;
        const RinDriverV3NpuAmdXdnaChainCommandV1* chain =
            (const RinDriverV3NpuAmdXdnaChainCommandV1*)command;
        if (chain->struct_size != command_length ||
            chain->version !=
                RIN_DRIVER_V3_NPU_AMDXDNA_COMMAND_VERSION_1_1 ||
            chain->engine_id >= 32u || chain->entry_count == 0u ||
            chain->entry_count >
                RIN_DRIVER_V3_NPU_AMDXDNA_MAX_CHAIN_ENTRIES ||
            chain->reserved0 != 0u)
            return 0;
        uint32_t offset = sizeof(*chain);
        for (uint32_t index = 0u; index < chain->entry_count; ++index) {
            if (offset > command_length ||
                sizeof(RinDriverV3NpuAmdXdnaChainSlotHeaderV1) >
                    command_length - offset)
                return 0;
            const RinDriverV3NpuAmdXdnaChainSlotHeaderV1* slot =
                (const RinDriverV3NpuAmdXdnaChainSlotHeaderV1*)(
                    bytes + offset);
            if (slot->instruction_buffer_address == 0u ||
                slot->instruction_size_bytes == 0u ||
                slot->instruction_buffer_address > UINT64_MAX -
                    slot->instruction_size_bytes ||
                slot->instruction_property_count > slot->argument_count ||
                slot->argument_count >
                    RIN_DRIVER_V3_NPU_AMDXDNA_MAX_COMMAND_ARGUMENTS ||
                slot->engine_id != chain->engine_id)
                return 0;
            const uint32_t argument_bytes = slot->argument_count *
                sizeof(uint32_t);
            offset += sizeof(*slot);
            if (argument_bytes > command_length - offset) return 0;
            offset += argument_bytes;
        }
        return offset == command_length;
    }
    return 0;
}

static inline int rin_driver_v3_npu_ops_valid(
    const RinDriverV3NpuOps* ops);

static inline int rin_driver_v3_npu_ops_match_capabilities(
    const RinDriverV3NpuOps* ops,
    const RinDriverV3NpuCapabilitiesV1* capabilities)
{
    if (!rin_driver_v3_npu_ops_valid(ops) ||
        !rin_driver_v3_npu_capabilities_valid(capabilities))
        return 0;
    if (capabilities->version == RIN_DRIVER_V3_NPU_ABI_VERSION)
        return ops->version == RIN_DRIVER_V3_NPU_OPS_VERSION;
    if ((ops->version != RIN_DRIVER_V3_NPU_OPS_VERSION_1_1 &&
         ops->version != RIN_DRIVER_V3_NPU_OPS_VERSION_1_2) ||
        ops->version != capabilities->version ||
        (ops->version & UINT32_C(0xffff0000)) !=
            (capabilities->version & UINT32_C(0xffff0000)))
        return 0;
    {
        const uint64_t declared = capabilities->reserved[0];
        const uint64_t present =
            (ops->context_create && ops->context_destroy
                 ? RIN_DRIVER_V3_NPU_OP_CONTEXTS : 0u) |
            ((ops->buffer_create || ops->buffer_create_sva) &&
             ops->buffer_destroy
                 ? RIN_DRIVER_V3_NPU_OP_BUFFERS : 0u) |
            (ops->queue_create && ops->queue_destroy
                 ? RIN_DRIVER_V3_NPU_OP_QUEUES : 0u) |
            ((ops->submit || ops->submit_command) && ops->poll_events
                 ? RIN_DRIVER_V3_NPU_OP_SUBMIT : 0u) |
            (ops->cancel ? RIN_DRIVER_V3_NPU_OP_CANCEL : 0u) |
            (ops->reset ? RIN_DRIVER_V3_NPU_OP_RESET : 0u) |
            (ops->set_power ? RIN_DRIVER_V3_NPU_OP_SET_POWER : 0u);
        if (declared != present ||
            ((ops->context_create == NULL) != (ops->context_destroy == NULL)) ||
            (((ops->buffer_create == NULL) &&
              (ops->buffer_create_sva == NULL)) !=
             (ops->buffer_destroy == NULL)) ||
            (ops->buffer_create != NULL &&
             ops->buffer_create_sva != NULL) ||
            ((ops->queue_create == NULL) != (ops->queue_destroy == NULL)) ||
            (((ops->submit == NULL) &&
              (ops->submit_command == NULL)) !=
             (ops->poll_events == NULL)) ||
            (ops->submit != NULL && ops->submit_command != NULL))
            return 0;
        if (ops->version == RIN_DRIVER_V3_NPU_OPS_VERSION_1_2) {
            const int sva =
                (capabilities->flags &
                 RIN_DRIVER_V3_NPU_CAP_SVA_HOST_IMPORT) != 0u;
            if ((ops->buffer_create_sva != NULL) != sva ||
                (ops->submit_command != NULL) != sva)
                return 0;
        } else if (ops->buffer_create_sva != NULL ||
                   ops->submit_command != NULL) {
            return 0;
        }
        return 1;
    }
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
        (ops->version != RIN_DRIVER_V3_NPU_OPS_VERSION &&
         ops->version != RIN_DRIVER_V3_NPU_OPS_VERSION_1_1 &&
         ops->version != RIN_DRIVER_V3_NPU_OPS_VERSION_1_2) ||
        !ops->get_identity || !ops->get_capabilities || !ops->remove ||
        ops->reserved[0] != 0u || ops->reserved[1] != 0u ||
        (ops->version != RIN_DRIVER_V3_NPU_OPS_VERSION_1_2 &&
         (ops->buffer_create_sva != NULL ||
          ops->submit_command != NULL)))
        return 0;
    if (ops->version == RIN_DRIVER_V3_NPU_OPS_VERSION &&
        (!ops->context_create || !ops->context_destroy ||
         !ops->buffer_create || !ops->buffer_destroy || !ops->queue_create ||
         !ops->queue_destroy || !ops->submit || !ops->poll_events ||
         !ops->cancel || !ops->reset || !ops->set_power))
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
        ops->remove_padding != 0u ||
        ops->buffer_create_sva_padding != 0u ||
        ops->submit_command_padding != 0u)
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
static_assert(sizeof(RinDriverV3NpuAmdXdnaCommandV1) == 152u,
              "AMD XDNA NPU command ABI drift");
static_assert(sizeof(RinDriverV3NpuAmdXdnaChainCommandV1) == 24u,
              "AMD XDNA chain command header ABI drift");
static_assert(sizeof(RinDriverV3NpuAmdXdnaChainSlotHeaderV1) == 24u,
              "AMD XDNA chain slot header ABI drift");
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
_Static_assert(sizeof(RinDriverV3NpuAmdXdnaCommandV1) == 152u,
               "AMD XDNA NPU command ABI drift");
_Static_assert(sizeof(RinDriverV3NpuAmdXdnaChainCommandV1) == 24u,
               "AMD XDNA chain command header ABI drift");
_Static_assert(sizeof(RinDriverV3NpuAmdXdnaChainSlotHeaderV1) == 24u,
               "AMD XDNA chain slot header ABI drift");
_Static_assert(sizeof(RinDriverV3NpuEventV1) == 96u,
               "NDRV v3 NPU event ABI drift");
_Static_assert(sizeof(RinDriverV3NpuOps) == 152u,
               "NDRV v3 NPU operations ABI drift");
#endif

#endif /* RIN_DRIVER_NPU_V1_H */
