/* SPDX-License-Identifier: MIT */
/* Typed bounded encoder for canonical RinGPU process-submit records. */
#ifndef RIN_SDK_GPU_PROCESS_ENCODER_H
#define RIN_SDK_GPU_PROCESS_ENCODER_H

#include <rin/gpu.h>
#include <ringpu/ringpu.h>
#include <ringpu/backend_command_v1.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#define RIN_GPU_PROCESS_ENCODER_ALIGNOF(type) alignof(type)
#else
#define RIN_GPU_PROCESS_ENCODER_ALIGNOF(type) _Alignof(type)
#endif

#define RIN_GPU_PROCESS_COMMAND_ENCODER_VERSION_V1 UINT32_C(1)

/* Caller-owned command storage. Process resource cookies passed to buffer
 * commands must already be bound to authenticated allocation ranges; pipeline
 * and bind-group cookies must be process-owned SDK objects. The encoder never
 * allocates, retains, or translates resource handles. */
typedef struct RinGpuProcessCommandEncoderV1 {
    uint32_t struct_size;
    uint32_t version;
    RinGpuBackendCommandV1* commands;
    uint32_t command_capacity;
    uint32_t command_count;
    uint64_t reserved[2];
} RinGpuProcessCommandEncoderV1;

static inline int rin_gpu_process_encoder_v1_zero_words(
    const uint64_t* words, size_t count)
{
    if (!words) return 0;
    while (count-- != 0u)
        if (*words++ != 0u) return 0;
    return 1;
}

static inline int rin_gpu_process_encoder_v1_ranges_overlap(
    const void* left, size_t left_size, const void* right, size_t right_size)
{
    uintptr_t left_begin;
    uintptr_t right_begin;
    if (!left || !right || left_size == 0u || right_size == 0u) return 0;
    left_begin = (uintptr_t)left;
    right_begin = (uintptr_t)right;
    if (left_begin > UINTPTR_MAX - left_size ||
        right_begin > UINTPTR_MAX - right_size)
        return 1;
    return left_begin < right_begin + right_size &&
           right_begin < left_begin + left_size;
}

static inline RinGpuResult rin_gpu_process_encoder_v1_validate(
    const RinGpuProcessCommandEncoderV1* encoder)
{
    size_t storage_size;
    if (!encoder || encoder->struct_size != sizeof(*encoder) ||
        encoder->version != RIN_GPU_PROCESS_COMMAND_ENCODER_VERSION_V1 ||
        !encoder->commands || encoder->command_capacity == 0u ||
        encoder->command_capacity > RIN_GPU_PROCESS_MAX_COMMANDS_V1 ||
        encoder->command_count > encoder->command_capacity ||
        !rin_gpu_process_encoder_v1_zero_words(encoder->reserved, 2u) ||
        (uintptr_t)encoder->commands %
                RIN_GPU_PROCESS_ENCODER_ALIGNOF(RinGpuBackendCommandV1) != 0u)
        return RIN_GPU_ERROR_INVALID_ARGUMENT;
    storage_size = (size_t)encoder->command_capacity *
                   sizeof(encoder->commands[0]);
    if (rin_gpu_process_encoder_v1_ranges_overlap(
            encoder, sizeof(*encoder), encoder->commands, storage_size))
        return RIN_GPU_ERROR_INVALID_ARGUMENT;
    return RIN_GPU_OK;
}

static inline RinGpuResult rin_gpu_process_command_encoder_init_v1(
    RinGpuProcessCommandEncoderV1* encoder,
    RinGpuBackendCommandV1* command_storage, uint32_t command_capacity)
{
    if (!encoder || !command_storage || command_capacity == 0u ||
        command_capacity > RIN_GPU_PROCESS_MAX_COMMANDS_V1 ||
        (uintptr_t)command_storage %
                RIN_GPU_PROCESS_ENCODER_ALIGNOF(RinGpuBackendCommandV1) != 0u ||
        rin_gpu_process_encoder_v1_ranges_overlap(
            encoder, sizeof(*encoder), command_storage,
            (size_t)command_capacity * sizeof(command_storage[0])))
        return command_capacity > RIN_GPU_PROCESS_MAX_COMMANDS_V1
                   ? RIN_GPU_ERROR_LIMIT
                   : RIN_GPU_ERROR_INVALID_ARGUMENT;
    memset(encoder, 0, sizeof(*encoder));
    encoder->struct_size = sizeof(*encoder);
    encoder->version = RIN_GPU_PROCESS_COMMAND_ENCODER_VERSION_V1;
    encoder->commands = command_storage;
    encoder->command_capacity = command_capacity;
    return RIN_GPU_OK;
}

static inline RinGpuResult rin_gpu_process_command_encoder_reset_v1(
    RinGpuProcessCommandEncoderV1* encoder)
{
    RinGpuResult result = rin_gpu_process_encoder_v1_validate(encoder);
    if (result != RIN_GPU_OK) return result;
    encoder->command_count = 0u;
    return RIN_GPU_OK;
}

static inline RinGpuResult rin_gpu_process_command_encoder_copy_buffer_v1(
    RinGpuProcessCommandEncoderV1* encoder, uint64_t destination_cookie,
    uint64_t destination_offset, uint64_t source_cookie,
    uint64_t source_offset, uint64_t size_bytes)
{
    RinGpuBackendCommandV1* command;
    RinGpuResult result = rin_gpu_process_encoder_v1_validate(encoder);
    if (result != RIN_GPU_OK) return result;
    if (destination_cookie == 0u || source_cookie == 0u || size_bytes == 0u)
        return RIN_GPU_ERROR_INVALID_ARGUMENT;
    if (destination_offset > UINT64_MAX - size_bytes ||
        source_offset > UINT64_MAX - size_bytes)
        return RIN_GPU_ERROR_BOUNDS;
    if (encoder->command_count == encoder->command_capacity)
        return RIN_GPU_ERROR_LIMIT;
    command = &encoder->commands[encoder->command_count];
    memset(command, 0, sizeof(*command));
    command->type = RIN_GPU_BACKEND_COMMAND_COPY_BUFFER;
    command->value.buffer_copy.destination_cookie = destination_cookie;
    command->value.buffer_copy.destination_offset = destination_offset;
    command->value.buffer_copy.source_cookie = source_cookie;
    command->value.buffer_copy.source_offset = source_offset;
    command->value.buffer_copy.size_bytes = size_bytes;
    ++encoder->command_count;
    return RIN_GPU_OK;
}

static inline RinGpuResult rin_gpu_process_command_encoder_clear_buffer_v1(
    RinGpuProcessCommandEncoderV1* encoder, uint64_t destination_cookie,
    uint64_t offset, uint64_t size_bytes, uint32_t pattern)
{
    RinGpuBackendCommandV1* command;
    RinGpuResult result = rin_gpu_process_encoder_v1_validate(encoder);
    if (result != RIN_GPU_OK) return result;
    if (destination_cookie == 0u || size_bytes == 0u)
        return RIN_GPU_ERROR_INVALID_ARGUMENT;
    if (offset > UINT64_MAX - size_bytes) return RIN_GPU_ERROR_BOUNDS;
    if (encoder->command_count == encoder->command_capacity)
        return RIN_GPU_ERROR_LIMIT;
    command = &encoder->commands[encoder->command_count];
    memset(command, 0, sizeof(*command));
    command->type = RIN_GPU_BACKEND_COMMAND_CLEAR_BUFFER;
    command->value.buffer_clear.destination_cookie = destination_cookie;
    command->value.buffer_clear.offset = offset;
    command->value.buffer_clear.size_bytes = size_bytes;
    command->value.buffer_clear.pattern = pattern;
    ++encoder->command_count;
    return RIN_GPU_OK;
}

static inline RinGpuResult rin_gpu_process_command_encoder_dispatch_v1(
    RinGpuProcessCommandEncoderV1* encoder, uint64_t pipeline_cookie,
    uint64_t bind_group_cookie, uint32_t group_count_x,
    uint32_t group_count_y, uint32_t group_count_z)
{
    RinGpuBackendCommandV1* command;
    RinGpuResult result = rin_gpu_process_encoder_v1_validate(encoder);
    if (result != RIN_GPU_OK) return result;
    if (pipeline_cookie == 0u || bind_group_cookie == 0u ||
        group_count_x == 0u || group_count_y == 0u || group_count_z == 0u)
        return RIN_GPU_ERROR_INVALID_ARGUMENT;
    if (encoder->command_count == encoder->command_capacity)
        return RIN_GPU_ERROR_LIMIT;
    command = &encoder->commands[encoder->command_count];
    memset(command, 0, sizeof(*command));
    command->type = RIN_GPU_BACKEND_COMMAND_DISPATCH;
    command->value.dispatch.pipeline_cookie = pipeline_cookie;
    command->value.dispatch.bind_group_cookie = bind_group_cookie;
    command->value.dispatch.group_count_x = group_count_x;
    command->value.dispatch.group_count_y = group_count_y;
    command->value.dispatch.group_count_z = group_count_z;
    ++encoder->command_count;
    return RIN_GPU_OK;
}

static inline RinGpuResult rin_gpu_process_command_encoder_make_submit_v2(
    const RinGpuProcessCommandEncoderV1* encoder,
    RinGpuProcessSubmitDescV2* descriptor_out)
{
    RinGpuResult result = rin_gpu_process_encoder_v1_validate(encoder);
    if (result != RIN_GPU_OK) return result;
    if (!descriptor_out || encoder->command_count == 0u ||
        rin_gpu_process_encoder_v1_ranges_overlap(
            encoder, sizeof(*encoder), descriptor_out, sizeof(*descriptor_out)) ||
        rin_gpu_process_encoder_v1_ranges_overlap(
            encoder->commands,
            (size_t)encoder->command_capacity * sizeof(encoder->commands[0]),
            descriptor_out, sizeof(*descriptor_out)))
        return RIN_GPU_ERROR_INVALID_ARGUMENT;
    memset(descriptor_out, 0, sizeof(*descriptor_out));
    descriptor_out->struct_size = sizeof(*descriptor_out);
    descriptor_out->version = RIN_GPU_PROCESS_SUBMIT_DESC_VERSION_V2;
    descriptor_out->commands.address = (uint64_t)(uintptr_t)encoder->commands;
    descriptor_out->commands.size =
        (uint64_t)encoder->command_count *
        RIN_GPU_PROCESS_COMMAND_RECORD_SIZE_V1;
    descriptor_out->command_count = encoder->command_count;
    descriptor_out->command_record_size =
        RIN_GPU_PROCESS_COMMAND_RECORD_SIZE_V1;
    return RIN_GPU_OK;
}

static inline RinResult rin_gpu_process_command_encoder_submit_v2(
    RinGpuProcessQueueV1 queue,
    const RinGpuProcessCommandEncoderV1* encoder,
    RinGpuProcessSubmitReceiptV1* receipt_out)
{
    RinGpuProcessSubmitDescV2 descriptor;
    RinGpuResult result;
    if (!encoder || !receipt_out ||
        rin_gpu_process_encoder_v1_validate(encoder) != RIN_GPU_OK ||
        rin_gpu_process_encoder_v1_ranges_overlap(
            encoder, sizeof(*encoder), receipt_out, sizeof(*receipt_out)) ||
        rin_gpu_process_encoder_v1_ranges_overlap(
            encoder->commands,
            (size_t)encoder->command_capacity * sizeof(encoder->commands[0]),
            receipt_out, sizeof(*receipt_out)))
        return RIN_ERROR_INVALID_ARGUMENT;
    result = rin_gpu_process_command_encoder_make_submit_v2(
        encoder, &descriptor);
    if (result == RIN_GPU_ERROR_LIMIT) return RIN_ERROR_LIMIT_EXCEEDED;
    if (result != RIN_GPU_OK) return RIN_ERROR_INVALID_ARGUMENT;
    return rin_gpu_process_submit_v2(queue, &descriptor, receipt_out);
}

#ifdef __cplusplus
}
#endif

#undef RIN_GPU_PROCESS_ENCODER_ALIGNOF

#endif /* RIN_SDK_GPU_PROCESS_ENCODER_H */
