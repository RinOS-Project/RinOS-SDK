/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_NPU_RUNTIME_HPP
#define RIN_DRIVER_NPU_RUNTIME_HPP

#include <stdint.h>

#include <rin/contract_abi.h>
#include <rin/driver/npu_model.hpp>
#include <rin/driver/npu_user_v1.h>

/* Header-only C++ facade for the NPU user-call ABI. It owns no allocations:
 * callers provide bounded arrays for enumeration and event polling, and keep
 * registered host memory alive until unregister succeeds. Every returned
 * token carries its device/context generation so stale handles are easy to
 * reject before entering the kernel. */
namespace rin {
namespace npu {

typedef RinDriverV3NpuUserDeviceInfoV1 DeviceInfo;

struct Context {
    uint64_t device_generation;
    uint64_t handle;
    uint64_t generation;
};

struct Buffer {
    uint64_t device_generation;
    uint64_t context_handle;
    uint64_t context_generation;
    uint64_t handle;
    uint64_t generation;
    RinDriverV3NpuBufferInfoV1 info;
};

struct Queue {
    uint64_t device_generation;
    uint64_t context_handle;
    uint64_t context_generation;
    uint64_t handle;
    uint64_t generation;
};

struct HostMemory {
    uint64_t device_generation;
    uint64_t token;
    uint64_t generation;
    uint64_t size_bytes;
    uint32_t access;
};

struct DeviceRequirements {
    uint32_t required_vendor_id;
    uint32_t preferred_vendor_id;
    uint32_t minimum_address_bits;
    uint32_t minimum_contexts;
    uint32_t minimum_queues_per_context;
    uint64_t required_capabilities;
    uint64_t required_data_types;
    uint64_t minimum_buffer_bytes;
};

class Runtime {
public:
    /* `count_out` receives the total count reported by the kernel even when
     * capacity is too small. The kernel returns BUFFER_TOO_SMALL in that
     * case and leaves the output array zeroed. */
    static RinResultCode enumerate(DeviceInfo* devices, uint32_t capacity,
                                   uint32_t* count_out)
    {
        if (count_out == 0 || (devices == 0) != (capacity == 0u) ||
            capacity > RIN_DRIVER_V3_NPU_USER_MAX_BUFFER / sizeof(DeviceInfo))
            return RIN_RESULT_INVALID_ARGUMENT;
        *count_out = 0u;
        RinDriverV3NpuUserCallV1 request = make_request(
            RIN_DRIVER_V3_NPU_USER_ENUMERATE, 0u, 0, 0u, 0u);
        RinDriverV3NpuUserCallResultV1 response = {};
        const uint32_t bytes = capacity * (uint32_t)sizeof(DeviceInfo);
        if (bytes != 0u) zero_bytes(devices, bytes);
        const RinResultCode status = invoke(
            &request, &response, devices, bytes);
        *count_out = response.count;
        if (status != RIN_RESULT_OK) return status;
        if (response.count > capacity || response.data_bytes !=
                response.count * (uint32_t)sizeof(DeviceInfo))
            return RIN_RESULT_CORRUPT_DATA;
        for (uint32_t index = 0u; index < response.count; ++index) {
            if (!device_info_valid(&devices[index])) {
                zero_bytes(devices, bytes);
                *count_out = 0u;
                return RIN_RESULT_CORRUPT_DATA;
            }
        }
        return RIN_RESULT_OK;
    }

    /* Returns an index into the supplied enumeration, or BUFFER_TOO_SMALL
     * when no device satisfies the model's memory, type, address, and vendor
     * requirements. Preferred-vendor devices win; ties choose the largest
     * usable buffer, then the lowest stable PCI identity. */
    static RinResultCode select_device(const DeviceInfo* devices,
                                       uint32_t count,
                                       const DeviceRequirements* requirements,
                                       uint32_t* index_out)
    {
        if (requirements == 0 || index_out == 0 ||
            (devices == 0 && count != 0u) ||
            (requirements->required_capabilities &
             ~RIN_DRIVER_V3_NPU_CAPABILITY_MASK) != 0u ||
            (requirements->required_data_types &
             ~RIN_DRIVER_V3_NPU_DATA_TYPE_MASK) != 0u ||
            requirements->minimum_address_bits > 64u)
            return RIN_RESULT_INVALID_ARGUMENT;
        *index_out = UINT32_MAX;
        uint32_t selected = UINT32_MAX;
        uint64_t selected_bytes = 0u;
        int selected_preferred = 0;
        for (uint32_t index = 0u; index < count; ++index) {
            if (!device_info_valid(&devices[index]))
                return RIN_RESULT_CORRUPT_DATA;
            const RinDriverV3NpuIdentityV1& identity =
                devices[index].identity;
            const RinDriverV3NpuCapabilitiesV1& capabilities =
                devices[index].capabilities;
            if ((requirements->required_vendor_id != 0u &&
                 identity.vendor_id != requirements->required_vendor_id) ||
                (capabilities.flags & requirements->required_capabilities) !=
                    requirements->required_capabilities ||
                (capabilities.supported_data_types &
                 requirements->required_data_types) !=
                    requirements->required_data_types ||
                capabilities.address_bits <
                    requirements->minimum_address_bits ||
                capabilities.max_contexts < requirements->minimum_contexts ||
                capabilities.max_queues_per_context <
                    requirements->minimum_queues_per_context ||
                capabilities.max_buffer_bytes <
                    requirements->minimum_buffer_bytes)
                continue;
            const int preferred = requirements->preferred_vendor_id != 0u &&
                identity.vendor_id == requirements->preferred_vendor_id;
            if (selected == UINT32_MAX || preferred > selected_preferred ||
                (preferred == selected_preferred &&
                 capabilities.max_buffer_bytes > selected_bytes) ||
                (preferred == selected_preferred &&
                 capabilities.max_buffer_bytes == selected_bytes &&
                 identity.vendor_id < devices[selected].identity.vendor_id) ||
                (preferred == selected_preferred &&
                 capabilities.max_buffer_bytes == selected_bytes &&
                 identity.vendor_id == devices[selected].identity.vendor_id &&
                 identity.device_id < devices[selected].identity.device_id)) {
                selected = index;
                selected_bytes = capabilities.max_buffer_bytes;
                selected_preferred = preferred;
            }
        }
        if (selected == UINT32_MAX) return RIN_RESULT_NOT_FOUND;
        *index_out = selected;
        return RIN_RESULT_OK;
    }

    static RinResultCode query_device(uint64_t device_generation,
                                      DeviceInfo* device_out)
    {
        if (device_generation == 0u || device_out == 0)
            return RIN_RESULT_INVALID_ARGUMENT;
        zero_bytes(device_out, sizeof(*device_out));
        RinDriverV3NpuUserCallV1 request = make_request(
            RIN_DRIVER_V3_NPU_USER_QUERY_DEVICE, device_generation, 0, 0u, 0u);
        RinDriverV3NpuUserCallResultV1 response = {};
        const RinResultCode status = invoke(&request, &response, 0, 0u);
        if (status != RIN_RESULT_OK) return status;
        if (!device_info_valid(&response.value.device) ||
            response.value.device.identity.device_generation !=
                device_generation)
            return RIN_RESULT_CORRUPT_DATA;
        *device_out = response.value.device;
        return RIN_RESULT_OK;
    }

    static RinResultCode create_context(uint64_t device_generation,
        const RinDriverV3NpuContextConfigV1* config, Context* context_out)
    {
        if (device_generation == 0u || config == 0 || context_out == 0 ||
            !rin_driver_v3_npu_context_config_valid(config))
            return RIN_RESULT_INVALID_ARGUMENT;
        clear(context_out);
        RinDriverV3NpuContextConfigV1 payload = *config;
        RinDriverV3NpuUserCallV1 request = make_request(
            RIN_DRIVER_V3_NPU_USER_CONTEXT_CREATE, device_generation, 0, 0u, 0u);
        RinDriverV3NpuUserCallResultV1 response = {};
        const RinResultCode status = invoke(&request, &response, &payload,
                                             sizeof(payload));
        if (status != RIN_RESULT_OK) return status;
        if (response.handle == 0u || response.generation == 0u)
            return RIN_RESULT_CORRUPT_DATA;
        context_out->device_generation = device_generation;
        context_out->handle = response.handle;
        context_out->generation = response.generation;
        return RIN_RESULT_OK;
    }

    static RinResultCode destroy_context(Context* context)
    {
        if (!context_valid(context)) return RIN_RESULT_INVALID_ARGUMENT;
        RinDriverV3NpuUserCallV1 request = make_request(
            RIN_DRIVER_V3_NPU_USER_CONTEXT_DESTROY,
            context->device_generation, context, 0u, 0u);
        RinDriverV3NpuUserCallResultV1 response = {};
        const RinResultCode status = invoke(&request, &response, 0, 0u);
        if (status == RIN_RESULT_OK) clear(context);
        return status;
    }

    static RinResultCode create_buffer(const Context* context,
        const RinDriverV3NpuBufferCreateV1* create_info, Buffer* buffer_out)
    {
        if (!context_valid(context) || create_info == 0 || buffer_out == 0 ||
            !rin_driver_v3_npu_buffer_create_valid(create_info))
            return RIN_RESULT_INVALID_ARGUMENT;
        clear(buffer_out);
        RinDriverV3NpuBufferCreateV1 payload = *create_info;
        RinDriverV3NpuUserCallV1 request = make_request(
            RIN_DRIVER_V3_NPU_USER_BUFFER_CREATE,
            context->device_generation, context, 0u, 0u);
        RinDriverV3NpuUserCallResultV1 response = {};
        const RinResultCode status = invoke(&request, &response, &payload,
                                             sizeof(payload));
        if (status != RIN_RESULT_OK) return status;
        if (!rin_driver_v3_npu_buffer_info_valid(&response.value.buffer) ||
            response.handle != response.value.buffer.driver_handle ||
            response.generation != response.value.buffer.driver_generation)
            return RIN_RESULT_CORRUPT_DATA;
        buffer_out->device_generation = context->device_generation;
        buffer_out->context_handle = context->handle;
        buffer_out->context_generation = context->generation;
        buffer_out->handle = response.handle;
        buffer_out->generation = response.generation;
        buffer_out->info = response.value.buffer;
        return RIN_RESULT_OK;
    }

    static RinResultCode destroy_buffer(const Context* context,
                                        Buffer* buffer)
    {
        if (!context_valid(context) || !buffer_matches(context, buffer))
            return RIN_RESULT_INVALID_ARGUMENT;
        RinDriverV3NpuUserCallV1 request = make_request(
            RIN_DRIVER_V3_NPU_USER_BUFFER_DESTROY,
            context->device_generation, context, buffer->handle,
            buffer->generation);
        RinDriverV3NpuUserCallResultV1 response = {};
        const RinResultCode status = invoke(&request, &response, 0, 0u);
        if (status == RIN_RESULT_OK) clear(buffer);
        return status;
    }

    static RinResultCode make_buffer_reference(const Context* context,
        const Buffer* buffer, uint64_t offset_bytes, uint64_t length_bytes,
        uint32_t access, RinDriverV3NpuBufferRefV1* reference_out)
    {
        if (reference_out == 0) return RIN_RESULT_INVALID_ARGUMENT;
        zero_bytes(reference_out, sizeof(*reference_out));
        if (!context_valid(context) || !buffer_matches(context, buffer) ||
            !rin_driver_v3_npu_buffer_info_valid(&buffer->info) ||
            length_bytes == 0u || offset_bytes > UINT64_MAX - length_bytes ||
            offset_bytes + length_bytes > buffer->info.size_bytes ||
            access == 0u ||
            (access & ~RIN_DRIVER_V3_NPU_BUFFER_ACCESS_MASK) != 0u ||
            (access & ~buffer->info.access) != 0u)
            return RIN_RESULT_INVALID_ARGUMENT;
        reference_out->buffer_handle = buffer->handle;
        reference_out->buffer_generation = buffer->generation;
        reference_out->offset_bytes = offset_bytes;
        reference_out->length_bytes = length_bytes;
        reference_out->access = access;
        return RIN_RESULT_OK;
    }

    /* Registers host memory in the current process, then binds a host-shared
     * NPU buffer to that generation-bound pin. Destroy the buffer before
     * unregistering the memory; unregister returns BUSY while a device still
     * owns the mapping. */
    static RinResultCode create_host_buffer(const Context* context,
        const HostMemory* memory, uint64_t size_bytes,
        uint64_t alignment_bytes, uint32_t access, Buffer* buffer_out)
    {
        if (!context_valid(context) || memory == 0 || buffer_out == 0 ||
            memory->device_generation != context->device_generation ||
            memory->token == 0u || memory->generation == 0u ||
            size_bytes == 0u || size_bytes > memory->size_bytes ||
            access == 0u || (access & ~memory->access) != 0u ||
            alignment_bytes == 0u ||
            (alignment_bytes & (alignment_bytes - 1u)) != 0u)
            return RIN_RESULT_INVALID_ARGUMENT;
        RinDriverV3NpuBufferCreateV1 create_info = {};
        create_info.struct_size = sizeof(create_info);
        create_info.version = RIN_DRIVER_V3_NPU_ABI_VERSION;
        create_info.size_bytes = size_bytes;
        create_info.alignment_bytes = alignment_bytes;
        create_info.access = access;
        create_info.memory_type = RIN_DRIVER_V3_NPU_BUFFER_HOST_SHARED;
        if ((access & RIN_DRIVER_V3_NPU_BUFFER_READ) != 0u)
            create_info.dma_flags |= RIN_DRIVER_V3_DMA_READ;
        if ((access & RIN_DRIVER_V3_NPU_BUFFER_WRITE) != 0u)
            create_info.dma_flags |= RIN_DRIVER_V3_DMA_WRITE;
        create_info.host_memory_token = memory->token;
        create_info.host_memory_generation = memory->generation;
        return create_buffer(context, &create_info, buffer_out);
    }

    static RinResultCode create_queue(const Context* context,
        const RinDriverV3NpuQueueCreateV1* create_info, Queue* queue_out)
    {
        if (!context_valid(context) || create_info == 0 || queue_out == 0 ||
            !rin_driver_v3_npu_queue_create_valid(create_info))
            return RIN_RESULT_INVALID_ARGUMENT;
        clear(queue_out);
        RinDriverV3NpuQueueCreateV1 payload = *create_info;
        RinDriverV3NpuUserCallV1 request = make_request(
            RIN_DRIVER_V3_NPU_USER_QUEUE_CREATE,
            context->device_generation, context, 0u, 0u);
        RinDriverV3NpuUserCallResultV1 response = {};
        const RinResultCode status = invoke(&request, &response, &payload,
                                             sizeof(payload));
        if (status != RIN_RESULT_OK) return status;
        if (response.handle == 0u || response.generation == 0u)
            return RIN_RESULT_CORRUPT_DATA;
        queue_out->device_generation = context->device_generation;
        queue_out->context_handle = context->handle;
        queue_out->context_generation = context->generation;
        queue_out->handle = response.handle;
        queue_out->generation = response.generation;
        return RIN_RESULT_OK;
    }

    static RinResultCode destroy_queue(const Context* context, Queue* queue)
    {
        if (!context_valid(context) || !queue_matches(context, queue))
            return RIN_RESULT_INVALID_ARGUMENT;
        RinDriverV3NpuUserCallV1 request = make_request(
            RIN_DRIVER_V3_NPU_USER_QUEUE_DESTROY,
            context->device_generation, context, queue->handle,
            queue->generation);
        RinDriverV3NpuUserCallResultV1 response = {};
        const RinResultCode status = invoke(&request, &response, 0, 0u);
        if (status == RIN_RESULT_OK) clear(queue);
        return status;
    }

    static RinResultCode submit(const Context* context, const Queue* queue,
        const RinDriverV3NpuSubmitV1* submission,
        const RinDriverV3NpuBufferRefV1* resources, uint32_t resource_count,
        RinDriverV3NpuFenceV1* fence_out)
    {
        if (!context_valid(context) || !queue_matches(context, queue) ||
            submission == 0 || fence_out == 0 ||
            (resource_count != 0u && resources == 0) ||
            resource_count > RIN_DRIVER_V3_NPU_MAX_SUBMIT_BUFFERS ||
            !rin_driver_v3_npu_submit_valid(submission, resources,
                                             resource_count))
            return RIN_RESULT_INVALID_ARGUMENT;
        fence_out->value = 0u;
        fence_out->generation = 0u;
        const uint32_t payload_bytes = (uint32_t)sizeof(*submission) +
            resource_count * (uint32_t)sizeof(*resources);
        uint8_t payload[sizeof(RinDriverV3NpuSubmitV1) +
            RIN_DRIVER_V3_NPU_MAX_SUBMIT_BUFFERS *
                sizeof(RinDriverV3NpuBufferRefV1)];
        zero_bytes(payload, sizeof(payload));
        copy_bytes(payload, submission, sizeof(*submission));
        if (resource_count != 0u)
            copy_bytes(payload + sizeof(*submission), resources,
                       resource_count * (uint32_t)sizeof(*resources));
        RinDriverV3NpuUserCallV1 request = make_request(
            RIN_DRIVER_V3_NPU_USER_SUBMIT, context->device_generation,
            context, queue->handle, queue->generation);
        RinDriverV3NpuUserCallResultV1 response = {};
        const RinResultCode status = invoke(&request, &response, payload,
                                             payload_bytes);
        zero_bytes(payload, sizeof(payload));
        if (status != RIN_RESULT_OK) return status;
        if (!rin_driver_v3_npu_fence_valid(&response.value.fence))
            return RIN_RESULT_CORRUPT_DATA;
        *fence_out = response.value.fence;
        return RIN_RESULT_OK;
    }

    static RinResultCode submit_buffer(const Context* context,
        const Queue* queue, const Buffer* command_buffer,
        uint64_t command_offset_bytes, uint64_t command_length_bytes,
        uint32_t flags, uint64_t user_tag,
        const RinDriverV3NpuBufferRefV1* resources,
        uint32_t resource_count, RinDriverV3NpuFenceV1* fence_out)
    {
        if (!context_valid(context) ||
            !queue_matches(context, queue) ||
            !buffer_matches(context, command_buffer) ||
            (command_buffer->info.access & RIN_DRIVER_V3_NPU_BUFFER_READ) == 0u ||
            command_length_bytes == 0u ||
            command_offset_bytes > UINT64_MAX - command_length_bytes ||
            command_offset_bytes + command_length_bytes >
                command_buffer->info.size_bytes ||
            (flags & ~RIN_DRIVER_V3_NPU_SUBMIT_FLAG_MASK) != 0u ||
            fence_out == 0)
            return RIN_RESULT_INVALID_ARGUMENT;
        RinDriverV3NpuSubmitV1 submission = {};
        submission.struct_size = sizeof(submission);
        submission.version = RIN_DRIVER_V3_NPU_ABI_VERSION;
        submission.flags = flags;
        submission.resource_count = resource_count;
        submission.command_buffer_handle = command_buffer->handle;
        submission.command_buffer_generation = command_buffer->generation;
        submission.command_offset_bytes = command_offset_bytes;
        submission.command_length_bytes = command_length_bytes;
        submission.user_tag = user_tag;
        return submit(context, queue, &submission, resources, resource_count,
                      fence_out);
    }

    static RinResultCode poll_events(const Context* context,
        RinDriverV3NpuEventV1* events, uint32_t capacity,
        uint32_t* count_out)
    {
        if (!context_valid(context) || count_out == 0 ||
            (events == 0) != (capacity == 0u) ||
            capacity > RIN_DRIVER_V3_NPU_MAX_EVENTS)
            return RIN_RESULT_INVALID_ARGUMENT;
        *count_out = 0u;
        const uint32_t bytes = capacity *
            (uint32_t)sizeof(RinDriverV3NpuEventV1);
        if (bytes != 0u) zero_bytes(events, bytes);
        RinDriverV3NpuUserCallV1 request = make_request(
            RIN_DRIVER_V3_NPU_USER_POLL_EVENTS,
            context->device_generation, context, 0u, 0u);
        RinDriverV3NpuUserCallResultV1 response = {};
        const RinResultCode status = invoke(&request, &response, events, bytes);
        if (status != RIN_RESULT_OK) return status;
        if (response.count > capacity || response.data_bytes != response.count *
                (uint32_t)sizeof(RinDriverV3NpuEventV1)) {
            if (bytes != 0u) zero_bytes(events, bytes);
            return RIN_RESULT_CORRUPT_DATA;
        }
        for (uint32_t index = 0u; index < response.count; ++index) {
            if (!rin_driver_v3_npu_event_valid(&events[index])) {
                if (bytes != 0u) zero_bytes(events, bytes);
                return RIN_RESULT_CORRUPT_DATA;
            }
        }
        *count_out = response.count;
        return RIN_RESULT_OK;
    }

    static RinResultCode cancel(const Context* context,
        const RinDriverV3NpuFenceV1* fence)
    {
        if (!context_valid(context) ||
            !rin_driver_v3_npu_fence_valid(fence))
            return RIN_RESULT_INVALID_ARGUMENT;
        RinDriverV3NpuUserCallV1 request = make_request(
            RIN_DRIVER_V3_NPU_USER_CANCEL,
            context->device_generation, context, 0u, 0u);
        request.argument0 = fence->value;
        request.argument1 = fence->generation;
        RinDriverV3NpuUserCallResultV1 response = {};
        return invoke(&request, &response, 0, 0u);
    }

    static RinResultCode reset_context(const Context* context,
                                       uint32_t timeout_ms)
    {
        if (!context_valid(context) || timeout_ms == 0u)
            return RIN_RESULT_INVALID_ARGUMENT;
        RinDriverV3NpuResetV1 payload = {};
        payload.struct_size = sizeof(payload);
        payload.version = RIN_DRIVER_V3_NPU_ABI_VERSION;
        payload.timeout_ms = timeout_ms;
        payload.context_handle = context->handle;
        payload.context_generation = context->generation;
        RinDriverV3NpuUserCallV1 request = make_request(
            RIN_DRIVER_V3_NPU_USER_RESET_CONTEXT,
            context->device_generation, context, 0u, 0u);
        RinDriverV3NpuUserCallResultV1 response = {};
        return invoke(&request, &response, &payload, sizeof(payload));
    }

    static RinResultCode register_memory(uint64_t device_generation,
        void* address, uint64_t size_bytes, uint32_t access,
        HostMemory* memory_out)
    {
        const uint64_t user_address = (uint64_t)(uintptr_t)address;
        if (device_generation == 0u || address == 0 || size_bytes == 0u ||
            user_address > UINT64_MAX - size_bytes || memory_out == 0 ||
            access == 0u ||
            (access & ~RIN_DRIVER_V3_NPU_BUFFER_ACCESS_MASK) != 0u)
            return RIN_RESULT_INVALID_ARGUMENT;
        clear(memory_out);
        RinDriverV3NpuUserCallV1 request = make_request(
            RIN_DRIVER_V3_NPU_USER_REGISTER_MEMORY,
            device_generation, 0, 0u, 0u);
        request.argument0 = user_address;
        request.argument1 = size_bytes;
        request.access = access;
        RinDriverV3NpuUserCallResultV1 response = {};
        const RinResultCode status = invoke(&request, &response, 0, 0u);
        if (status != RIN_RESULT_OK) return status;
        if (response.handle == 0u || response.generation == 0u ||
            response.value.token_pair[0] != response.handle ||
            response.value.token_pair[1] != response.generation)
            return RIN_RESULT_CORRUPT_DATA;
        memory_out->device_generation = device_generation;
        memory_out->token = response.handle;
        memory_out->generation = response.generation;
        memory_out->size_bytes = size_bytes;
        memory_out->access = access;
        return RIN_RESULT_OK;
    }

    static RinResultCode unregister_memory(HostMemory* memory)
    {
        if (memory == 0 || memory->device_generation == 0u ||
            memory->token == 0u || memory->generation == 0u ||
            memory->size_bytes == 0u || memory->access == 0u)
            return RIN_RESULT_INVALID_ARGUMENT;
        RinDriverV3NpuUserCallV1 request = make_request(
            RIN_DRIVER_V3_NPU_USER_UNREGISTER_MEMORY,
            memory->device_generation, 0, 0u, 0u);
        request.argument0 = memory->token;
        request.argument1 = memory->generation;
        RinDriverV3NpuUserCallResultV1 response = {};
        const RinResultCode status = invoke(&request, &response, 0, 0u);
        if (status == RIN_RESULT_OK) clear(memory);
        return status;
    }

private:
    static void zero_bytes(void* destination, size_t bytes)
    {
        uint8_t* output = static_cast<uint8_t*>(destination);
        for (size_t index = 0u; index < bytes; ++index) output[index] = 0u;
    }

    static void copy_bytes(void* destination, const void* source, size_t bytes)
    {
        uint8_t* output = static_cast<uint8_t*>(destination);
        const uint8_t* input = static_cast<const uint8_t*>(source);
        for (size_t index = 0u; index < bytes; ++index) output[index] = input[index];
    }

    static void clear(Context* context)
    {
        if (context != 0) zero_bytes(context, sizeof(*context));
    }

    static void clear(Buffer* buffer)
    {
        if (buffer != 0) zero_bytes(buffer, sizeof(*buffer));
    }

    static void clear(Queue* queue)
    {
        if (queue != 0) zero_bytes(queue, sizeof(*queue));
    }

    static void clear(HostMemory* memory)
    {
        if (memory != 0) zero_bytes(memory, sizeof(*memory));
    }

    static bool context_valid(const Context* context)
    {
        return context != 0 && context->device_generation != 0u &&
               context->handle != 0u && context->generation != 0u;
    }

    static bool buffer_matches(const Context* context, const Buffer* buffer)
    {
        return buffer != 0 && buffer->device_generation ==
                   context->device_generation && buffer->context_handle ==
                   context->handle && buffer->context_generation ==
                   context->generation && buffer->handle != 0u &&
               buffer->generation != 0u;
    }

    static bool queue_matches(const Context* context, const Queue* queue)
    {
        return queue != 0 && queue->device_generation ==
                   context->device_generation && queue->context_handle ==
                   context->handle && queue->context_generation ==
                   context->generation && queue->handle != 0u &&
               queue->generation != 0u;
    }

    static bool device_info_valid(const DeviceInfo* device)
    {
        return device != 0 && device->struct_size == sizeof(*device) &&
            device->version == RIN_DRIVER_V3_NPU_USER_ABI_VERSION &&
            rin_driver_v3_npu_identity_valid(&device->identity) &&
            rin_driver_v3_npu_capabilities_valid(&device->capabilities);
    }

    static RinDriverV3NpuUserCallV1 make_request(
        uint32_t operation, uint64_t device_generation,
        const Context* context, uint64_t object_handle,
        uint64_t object_generation)
    {
        RinDriverV3NpuUserCallV1 request = {};
        request.struct_size = sizeof(request);
        request.version = RIN_DRIVER_V3_NPU_USER_ABI_VERSION;
        request.operation = operation;
        request.device_generation = device_generation;
        if (context != 0) {
            request.context_handle = context->handle;
            request.context_generation = context->generation;
        }
        request.object_handle = object_handle;
        request.object_generation = object_generation;
        return request;
    }

    static RinResultCode invoke(const RinDriverV3NpuUserCallV1* request,
        RinDriverV3NpuUserCallResultV1* response, void* data,
        uint32_t data_bytes)
    {
        if (request == 0 || response == 0) return RIN_RESULT_INVALID_ARGUMENT;
        zero_bytes(response, sizeof(*response));
        response->struct_size = sizeof(*response);
        response->version = RIN_DRIVER_V3_NPU_USER_ABI_VERSION;
        const long raw = rin_driver_v3_npu_user_call_v1(
            request, response, data, data_bytes);
        if (raw < INT32_MIN || raw > INT32_MAX)
            return RIN_RESULT_CORRUPT_DATA;
        const RinResultCode status = (RinResultCode)raw;
        if (response->struct_size != sizeof(*response) ||
            response->version != RIN_DRIVER_V3_NPU_USER_ABI_VERSION ||
            response->flags != 0u || response->reserved[0] != 0u ||
            response->reserved[1] != 0u || response->result != status)
            return RIN_RESULT_CORRUPT_DATA;
        return status;
    }
};

} /* namespace npu */
} /* namespace rin */

#endif /* RIN_DRIVER_NPU_RUNTIME_HPP */
