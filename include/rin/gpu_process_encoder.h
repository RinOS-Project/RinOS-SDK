/* SPDX-License-Identifier: MIT */
/* Typed bounded encoder for canonical RinGPU process-submit records. */
#ifndef RIN_SDK_GPU_PROCESS_ENCODER_H
#define RIN_SDK_GPU_PROCESS_ENCODER_H

#include <rin/gpu.h>
#include <ringpu/ringpu.h>
#include <ringpu/backend_command_v1.h>

#include <stddef.h>
#include <stdint.h>
#include <float.h>
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

typedef struct RinGpuProcessEncoderGraphicsStateV1 {
    uint64_t color_target_cookie;
    uint64_t query_cookie;
    uint32_t mip_level;
    uint32_t array_layer;
    uint32_t render_pass_active;
    uint32_t query_type;
} RinGpuProcessEncoderGraphicsStateV1;

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

static inline int rin_gpu_process_encoder_v1_finite_float(float value)
{
    return value == value && value <= FLT_MAX && value >= -FLT_MAX;
}

static inline int rin_gpu_process_encoder_v1_render_pass_valid(
    const RinGpuBackendRenderPassBeginV1* pass)
{
    const RinGpuClearRegionV1* region;
    if (!pass || pass->color_target_cookie == 0u ||
        (pass->load_op != RIN_GPU_RENDER_LOAD &&
         pass->load_op != RIN_GPU_RENDER_CLEAR) ||
        pass->store_op != RIN_GPU_RENDER_STORE || pass->flags != 0u ||
        pass->reserved != 0u || pass->reserved1 != 0u ||
        (pass->color_write_mask & ~RIN_GPU_COLOR_WRITE_ALL) != 0u)
        return 0;
    region = &pass->clear_region;
    if (region->enabled > 1u || region->reserved != 0u)
        return 0;
    if (region->enabled == 0u &&
        (region->x != 0 || region->y != 0 || region->width != 0u ||
         region->height != 0u))
        return 0;
    if (region->enabled != 0u && (region->x < 0 || region->y < 0))
        return 0;
    if (pass->load_op == RIN_GPU_RENDER_LOAD)
        return pass->clear_red == 0.0f && pass->clear_green == 0.0f &&
               pass->clear_blue == 0.0f && pass->clear_alpha == 0.0f &&
               pass->color_write_mask == 0u;
    return rin_gpu_process_encoder_v1_finite_float(pass->clear_red) &&
           rin_gpu_process_encoder_v1_finite_float(pass->clear_green) &&
           rin_gpu_process_encoder_v1_finite_float(pass->clear_blue) &&
           rin_gpu_process_encoder_v1_finite_float(pass->clear_alpha);
}

static inline int rin_gpu_process_encoder_v1_draw_valid(
    const RinGpuBackendDrawV1* draw)
{
    return draw && draw->pipeline_cookie != 0u &&
           draw->color_target_cookie != 0u && draw->vertex_count != 0u &&
           draw->vertex_count <= RIN_GPU_MAX_DRAW_VERTICES &&
           draw->instance_count != 0u &&
           draw->instance_count <= RIN_GPU_MAX_DRAW_INSTANCES &&
           draw->first_vertex <= UINT32_MAX - draw->vertex_count &&
           draw->first_instance <= UINT32_MAX - draw->instance_count &&
           draw->reserved0 == 0u && draw->reserved1 == 0u;
}

/* Process-query types and RinGPU backend-query types currently share numeric
 * values, but translate explicitly at this ABI boundary so a future public
 * token schema cannot silently change the backend command meaning. */
static inline RinGpuResult rin_gpu_process_encoder_v1_query_type(
    uint32_t process_query_type, uint32_t* backend_query_type_out)
{
    if (!backend_query_type_out) return RIN_GPU_ERROR_INVALID_ARGUMENT;
    *backend_query_type_out = 0u;
    switch (process_query_type) {
    case RIN_GPU_PROCESS_QUERY_TIMESTAMP_V1:
        *backend_query_type_out = RIN_GPU_QUERY_TIMESTAMP;
        return RIN_GPU_OK;
    case RIN_GPU_PROCESS_QUERY_OCCLUSION_V1:
        *backend_query_type_out = RIN_GPU_QUERY_OCCLUSION;
        return RIN_GPU_OK;
    case RIN_GPU_PROCESS_QUERY_PIPELINE_STATISTICS_V1:
        *backend_query_type_out = RIN_GPU_QUERY_PIPELINE_STATISTICS;
        return RIN_GPU_OK;
    default:
        return RIN_GPU_ERROR_UNSUPPORTED;
    }
}

static inline int rin_gpu_process_encoder_v1_backend_query_type_valid(
    uint32_t backend_query_type)
{
    return backend_query_type == RIN_GPU_QUERY_TIMESTAMP ||
           backend_query_type == RIN_GPU_QUERY_OCCLUSION ||
           backend_query_type == RIN_GPU_QUERY_PIPELINE_STATISTICS;
}

/* Re-validate caller-owned records before append/submit.  The compact encoder
 * only accepts its explicit buffer/compute subset outside a render pass and
 * one color target for a graphics pass. */
static inline RinGpuResult rin_gpu_process_encoder_v1_graphics_state(
    const RinGpuProcessCommandEncoderV1* encoder,
    RinGpuProcessEncoderGraphicsStateV1* state_out)
{
    RinGpuProcessEncoderGraphicsStateV1 state = {0};
    for (uint32_t index = 0u; index < encoder->command_count; ++index) {
        const RinGpuBackendCommandV1* command = &encoder->commands[index];
        if (command->reserved != 0u) return RIN_GPU_ERROR_INVALID_ARGUMENT;
        switch (command->type) {
        case RIN_GPU_BACKEND_COMMAND_COPY_BUFFER:
            if (state.render_pass_active != 0u)
                return RIN_GPU_ERROR_STATE;
            if (command->value.buffer_copy.destination_cookie == 0u ||
                command->value.buffer_copy.source_cookie == 0u ||
                command->value.buffer_copy.size_bytes == 0u ||
                command->value.buffer_copy.destination_offset >
                    UINT64_MAX - command->value.buffer_copy.size_bytes ||
                command->value.buffer_copy.source_offset >
                    UINT64_MAX - command->value.buffer_copy.size_bytes)
                return RIN_GPU_ERROR_INVALID_ARGUMENT;
            break;
        case RIN_GPU_BACKEND_COMMAND_CLEAR_BUFFER:
            if (state.render_pass_active != 0u)
                return RIN_GPU_ERROR_STATE;
            if (command->value.buffer_clear.destination_cookie == 0u ||
                command->value.buffer_clear.size_bytes == 0u ||
                command->value.buffer_clear.offset >
                    UINT64_MAX - command->value.buffer_clear.size_bytes ||
                command->value.buffer_clear.reserved != 0u)
                return RIN_GPU_ERROR_INVALID_ARGUMENT;
            break;
        case RIN_GPU_BACKEND_COMMAND_DISPATCH:
            if (state.render_pass_active != 0u)
                return RIN_GPU_ERROR_STATE;
            if (command->value.dispatch.pipeline_cookie == 0u ||
                command->value.dispatch.bind_group_cookie == 0u ||
                command->value.dispatch.group_count_x == 0u ||
                command->value.dispatch.group_count_y == 0u ||
                command->value.dispatch.group_count_z == 0u ||
                command->value.dispatch.reserved != 0u)
                return RIN_GPU_ERROR_INVALID_ARGUMENT;
            break;
        case RIN_GPU_BACKEND_COMMAND_BEGIN_RENDER_PASS:
            if (state.render_pass_active != 0u)
                return RIN_GPU_ERROR_STATE;
            if (!rin_gpu_process_encoder_v1_render_pass_valid(
                    &command->value.render_pass_begin))
                return RIN_GPU_ERROR_INVALID_ARGUMENT;
            state.color_target_cookie =
                command->value.render_pass_begin.color_target_cookie;
            state.mip_level = command->value.render_pass_begin.mip_level;
            state.array_layer =
                command->value.render_pass_begin.array_layer;
            state.render_pass_active = 1u;
            break;
        case RIN_GPU_BACKEND_COMMAND_DRAW:
            if (state.render_pass_active == 0u)
                return RIN_GPU_ERROR_STATE;
            if (!rin_gpu_process_encoder_v1_draw_valid(
                    &command->value.draw))
                return RIN_GPU_ERROR_INVALID_ARGUMENT;
            if (command->value.draw.color_target_cookie !=
                    state.color_target_cookie ||
                command->value.draw.mip_level != state.mip_level ||
                command->value.draw.array_layer != state.array_layer)
                return RIN_GPU_ERROR_STATE;
            break;
        case RIN_GPU_BACKEND_COMMAND_END_RENDER_PASS:
            if (state.render_pass_active == 0u)
                return RIN_GPU_ERROR_STATE;
            state.color_target_cookie = 0u;
            state.mip_level = 0u;
            state.array_layer = 0u;
            state.render_pass_active = 0u;
            break;
        case RIN_GPU_BACKEND_COMMAND_BEGIN_QUERY: {
            if (command->value.query.query_cookie == 0u ||
                command->value.query.reserved != 0u)
                return RIN_GPU_ERROR_INVALID_ARGUMENT;
            if (!rin_gpu_process_encoder_v1_backend_query_type_valid(
                    command->value.query.query_type))
                return RIN_GPU_ERROR_UNSUPPORTED;
            if (state.query_cookie != 0u) return RIN_GPU_ERROR_STATE;
            state.query_cookie = command->value.query.query_cookie;
            state.query_type = command->value.query.query_type;
            break;
        }
        case RIN_GPU_BACKEND_COMMAND_END_QUERY: {
            if (command->value.query.query_cookie == 0u ||
                command->value.query.reserved != 0u)
                return RIN_GPU_ERROR_INVALID_ARGUMENT;
            if (!rin_gpu_process_encoder_v1_backend_query_type_valid(
                    command->value.query.query_type))
                return RIN_GPU_ERROR_UNSUPPORTED;
            if (state.query_cookie != command->value.query.query_cookie ||
                state.query_type != command->value.query.query_type)
                return RIN_GPU_ERROR_STATE;
            state.query_cookie = 0u;
            state.query_type = 0u;
            break;
        }
        case RIN_GPU_BACKEND_COMMAND_RESET_QUERY: {
            if (command->value.query.query_cookie == 0u ||
                command->value.query.reserved != 0u)
                return RIN_GPU_ERROR_INVALID_ARGUMENT;
            if (!rin_gpu_process_encoder_v1_backend_query_type_valid(
                    command->value.query.query_type))
                return RIN_GPU_ERROR_UNSUPPORTED;
            if (state.query_cookie != 0u ||
                state.render_pass_active != 0u)
                return RIN_GPU_ERROR_STATE;
            break;
        }
        default:
            return RIN_GPU_ERROR_UNSUPPORTED;
        }
    }
    if (state_out) *state_out = state;
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
    RinGpuProcessEncoderGraphicsStateV1 state;
    RinGpuResult result = rin_gpu_process_encoder_v1_validate(encoder);
    if (result != RIN_GPU_OK) return result;
    result = rin_gpu_process_encoder_v1_graphics_state(encoder, &state);
    if (result != RIN_GPU_OK) return result;
    if (state.render_pass_active != 0u) return RIN_GPU_ERROR_STATE;
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
    RinGpuProcessEncoderGraphicsStateV1 state;
    RinGpuResult result = rin_gpu_process_encoder_v1_validate(encoder);
    if (result != RIN_GPU_OK) return result;
    result = rin_gpu_process_encoder_v1_graphics_state(encoder, &state);
    if (result != RIN_GPU_OK) return result;
    if (state.render_pass_active != 0u) return RIN_GPU_ERROR_STATE;
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

/* Validate a borrowed, live process-buffer owner and the requested access
 * range before translating it to the cookie-based canonical command. This
 * does not retain the owner: callers must keep every referenced buffer bound
 * through submit and completion. The authenticated kernel still resolves the
 * cookie and acquires its own allocation lease at submit time. */
static inline RinGpuResult rin_gpu_process_encoder_v1_buffer_range_status(
    const RinGpuProcessBufferV1* buffer, uint32_t access,
    uint64_t offset, uint64_t size_bytes)
{
    const uint32_t known_access = RIN_GPU_ALLOCATION_GPU_READ |
                                  RIN_GPU_ALLOCATION_GPU_WRITE;

    if (!buffer || buffer->struct_size != sizeof(*buffer) ||
        buffer->version != RIN_GPU_PROCESS_BUFFER_VERSION_V1 ||
        buffer->state != RIN_GPU_PROCESS_BUFFER_STATE_BOUND_V1 ||
        buffer->reserved0 != 0u ||
        buffer->desc.struct_size != sizeof(buffer->desc) ||
        buffer->desc.version != RIN_GPU_PROCESS_BUFFER_DESC_VERSION_V1 ||
        buffer->desc.device_id == 0u ||
        buffer->desc.device_generation == 0u ||
        buffer->desc.resource_cookie == 0u || buffer->desc.allocation == 0u ||
        buffer->desc.size_bytes == 0u || buffer->desc.reserved != 0u ||
        buffer->desc.required_gpu_access == 0u ||
        (buffer->desc.required_gpu_access & ~known_access) != 0u ||
        access == 0u || (access & ~known_access) != 0u ||
        (access & buffer->desc.required_gpu_access) != access ||
        buffer->desc.allocation_offset >
            UINT64_MAX - buffer->desc.size_bytes)
        return RIN_GPU_ERROR_INVALID_ARGUMENT;
    if (size_bytes == 0u || offset > buffer->desc.size_bytes ||
        size_bytes > buffer->desc.size_bytes - offset)
        return RIN_GPU_ERROR_BOUNDS;
    return RIN_GPU_OK;
}

static inline RinGpuResult
rin_gpu_process_encoder_v1_allocated_buffer_view(
    const RinGpuProcessAllocatedBufferV1* allocated,
    RinGpuProcessBufferV1* view)
{
    if (!allocated || !view ||
        allocated->struct_size != sizeof(*allocated) ||
        allocated->version != RIN_GPU_PROCESS_ALLOCATED_BUFFER_VERSION_V1 ||
        allocated->reserved0 != 0u ||
        allocated->desc.allocation_offset != 0u)
        return RIN_GPU_ERROR_INVALID_ARGUMENT;
    if (allocated->state != RIN_GPU_PROCESS_ALLOCATED_BUFFER_STATE_BOUND_V1)
        return RIN_GPU_ERROR_STATE;

    memset(view, 0, sizeof(*view));
    view->struct_size = sizeof(*view);
    view->version = RIN_GPU_PROCESS_BUFFER_VERSION_V1;
    view->state = RIN_GPU_PROCESS_BUFFER_STATE_BOUND_V1;
    view->desc = allocated->desc;
    return RIN_GPU_OK;
}

static inline RinGpuResult
rin_gpu_process_command_encoder_copy_process_buffers_v1(
    RinGpuProcessCommandEncoderV1* encoder,
    const RinGpuProcessBufferV1* destination, uint64_t destination_offset,
    const RinGpuProcessBufferV1* source, uint64_t source_offset,
    uint64_t size_bytes)
{
    RinGpuResult result = rin_gpu_process_encoder_v1_buffer_range_status(
        destination, RIN_GPU_ALLOCATION_GPU_WRITE, destination_offset,
        size_bytes);
    if (result != RIN_GPU_OK) return result;
    result = rin_gpu_process_encoder_v1_buffer_range_status(
        source, RIN_GPU_ALLOCATION_GPU_READ, source_offset, size_bytes);
    if (result != RIN_GPU_OK) return result;
    if (destination->desc.device_id != source->desc.device_id ||
        destination->desc.device_generation !=
            source->desc.device_generation)
        return RIN_GPU_ERROR_STATE;
    return rin_gpu_process_command_encoder_copy_buffer_v1(
        encoder, destination->desc.resource_cookie, destination_offset,
        source->desc.resource_cookie, source_offset, size_bytes);
}

static inline RinGpuResult
rin_gpu_process_command_encoder_clear_process_buffer_v1(
    RinGpuProcessCommandEncoderV1* encoder,
    const RinGpuProcessBufferV1* destination, uint64_t offset,
    uint64_t size_bytes, uint32_t pattern)
{
    RinGpuResult result = rin_gpu_process_encoder_v1_buffer_range_status(
        destination, RIN_GPU_ALLOCATION_GPU_WRITE, offset, size_bytes);
    if (result != RIN_GPU_OK) return result;
    return rin_gpu_process_command_encoder_clear_buffer_v1(
        encoder, destination->desc.resource_cookie, offset, size_bytes,
        pattern);
}

static inline RinGpuResult
rin_gpu_process_command_encoder_copy_process_allocated_buffers_v1(
    RinGpuProcessCommandEncoderV1* encoder,
    const RinGpuProcessAllocatedBufferV1* destination,
    uint64_t destination_offset,
    const RinGpuProcessAllocatedBufferV1* source, uint64_t source_offset,
    uint64_t size_bytes)
{
    RinGpuProcessBufferV1 destination_view;
    RinGpuProcessBufferV1 source_view;
    RinGpuResult result = rin_gpu_process_encoder_v1_allocated_buffer_view(
        destination, &destination_view);
    if (result != RIN_GPU_OK) return result;
    result = rin_gpu_process_encoder_v1_allocated_buffer_view(
        source, &source_view);
    if (result != RIN_GPU_OK) return result;
    return rin_gpu_process_command_encoder_copy_process_buffers_v1(
        encoder, &destination_view, destination_offset, &source_view,
        source_offset, size_bytes);
}

static inline RinGpuResult
rin_gpu_process_command_encoder_copy_process_buffer_to_allocated_buffer_v1(
    RinGpuProcessCommandEncoderV1* encoder,
    const RinGpuProcessAllocatedBufferV1* destination,
    uint64_t destination_offset, const RinGpuProcessBufferV1* source,
    uint64_t source_offset, uint64_t size_bytes)
{
    RinGpuProcessBufferV1 destination_view;
    RinGpuResult result = rin_gpu_process_encoder_v1_allocated_buffer_view(
        destination, &destination_view);
    if (result != RIN_GPU_OK) return result;
    return rin_gpu_process_command_encoder_copy_process_buffers_v1(
        encoder, &destination_view, destination_offset, source, source_offset,
        size_bytes);
}

static inline RinGpuResult
rin_gpu_process_command_encoder_copy_process_allocated_buffer_to_buffer_v1(
    RinGpuProcessCommandEncoderV1* encoder,
    const RinGpuProcessBufferV1* destination, uint64_t destination_offset,
    const RinGpuProcessAllocatedBufferV1* source, uint64_t source_offset,
    uint64_t size_bytes)
{
    RinGpuProcessBufferV1 source_view;
    RinGpuResult result = rin_gpu_process_encoder_v1_allocated_buffer_view(
        source, &source_view);
    if (result != RIN_GPU_OK) return result;
    return rin_gpu_process_command_encoder_copy_process_buffers_v1(
        encoder, destination, destination_offset, &source_view, source_offset,
        size_bytes);
}

static inline RinGpuResult
rin_gpu_process_command_encoder_clear_process_allocated_buffer_v1(
    RinGpuProcessCommandEncoderV1* encoder,
    const RinGpuProcessAllocatedBufferV1* destination, uint64_t offset,
    uint64_t size_bytes, uint32_t pattern)
{
    RinGpuProcessBufferV1 destination_view;
    RinGpuResult result = rin_gpu_process_encoder_v1_allocated_buffer_view(
        destination, &destination_view);
    if (result != RIN_GPU_OK) return result;
    return rin_gpu_process_command_encoder_clear_process_buffer_v1(
        encoder, &destination_view, offset, size_bytes, pattern);
}

static inline RinGpuResult rin_gpu_process_command_encoder_dispatch_v1(
    RinGpuProcessCommandEncoderV1* encoder, uint64_t pipeline_cookie,
    uint64_t bind_group_cookie, uint32_t group_count_x,
    uint32_t group_count_y, uint32_t group_count_z)
{
    RinGpuBackendCommandV1* command;
    RinGpuProcessEncoderGraphicsStateV1 state;
    RinGpuResult result = rin_gpu_process_encoder_v1_validate(encoder);
    if (result != RIN_GPU_OK) return result;
    result = rin_gpu_process_encoder_v1_graphics_state(encoder, &state);
    if (result != RIN_GPU_OK) return result;
    if (state.render_pass_active != 0u) return RIN_GPU_ERROR_STATE;
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

static inline RinGpuResult rin_gpu_process_command_encoder_begin_render_pass_v1(
    RinGpuProcessCommandEncoderV1* encoder,
    const RinGpuBackendRenderPassBeginV1* descriptor)
{
    RinGpuBackendCommandV1* command;
    RinGpuProcessEncoderGraphicsStateV1 state;
    RinGpuResult result = rin_gpu_process_encoder_v1_validate(encoder);
    if (result != RIN_GPU_OK) return result;
    result = rin_gpu_process_encoder_v1_graphics_state(encoder, &state);
    if (result != RIN_GPU_OK) return result;
    if (state.render_pass_active != 0u)
        return RIN_GPU_ERROR_STATE;
    if (!rin_gpu_process_encoder_v1_render_pass_valid(descriptor))
        return RIN_GPU_ERROR_INVALID_ARGUMENT;
    if (encoder->command_count == encoder->command_capacity)
        return RIN_GPU_ERROR_LIMIT;
    command = &encoder->commands[encoder->command_count];
    memset(command, 0, sizeof(*command));
    command->type = RIN_GPU_BACKEND_COMMAND_BEGIN_RENDER_PASS;
    command->value.render_pass_begin = *descriptor;
    ++encoder->command_count;
    return RIN_GPU_OK;
}

static inline RinGpuResult rin_gpu_process_command_encoder_draw_v1(
    RinGpuProcessCommandEncoderV1* encoder,
    const RinGpuBackendDrawV1* descriptor)
{
    RinGpuBackendCommandV1* command;
    RinGpuProcessEncoderGraphicsStateV1 state;
    RinGpuResult result = rin_gpu_process_encoder_v1_validate(encoder);
    if (result != RIN_GPU_OK) return result;
    result = rin_gpu_process_encoder_v1_graphics_state(encoder, &state);
    if (result != RIN_GPU_OK) return result;
    if (state.render_pass_active == 0u)
        return RIN_GPU_ERROR_STATE;
    if (!rin_gpu_process_encoder_v1_draw_valid(descriptor))
        return RIN_GPU_ERROR_INVALID_ARGUMENT;
    if (descriptor->color_target_cookie != state.color_target_cookie ||
        descriptor->mip_level != state.mip_level ||
        descriptor->array_layer != state.array_layer)
        return RIN_GPU_ERROR_STATE;
    if (encoder->command_count == encoder->command_capacity)
        return RIN_GPU_ERROR_LIMIT;
    command = &encoder->commands[encoder->command_count];
    memset(command, 0, sizeof(*command));
    command->type = RIN_GPU_BACKEND_COMMAND_DRAW;
    command->value.draw = *descriptor;
    ++encoder->command_count;
    return RIN_GPU_OK;
}

static inline RinGpuResult rin_gpu_process_command_encoder_end_render_pass_v1(
    RinGpuProcessCommandEncoderV1* encoder)
{
    RinGpuBackendCommandV1* command;
    RinGpuProcessEncoderGraphicsStateV1 state;
    RinGpuResult result = rin_gpu_process_encoder_v1_validate(encoder);
    if (result != RIN_GPU_OK) return result;
    result = rin_gpu_process_encoder_v1_graphics_state(encoder, &state);
    if (result != RIN_GPU_OK) return result;
    if (state.render_pass_active == 0u)
        return RIN_GPU_ERROR_STATE;
    if (encoder->command_count == encoder->command_capacity)
        return RIN_GPU_ERROR_LIMIT;
    command = &encoder->commands[encoder->command_count];
    memset(command, 0, sizeof(*command));
    command->type = RIN_GPU_BACKEND_COMMAND_END_RENDER_PASS;
    ++encoder->command_count;
    return RIN_GPU_OK;
}

/* Query cookies are authenticated process-query tokens, never backend
 * cookies. The kernel resolves them to this process' query object before the
 * canonical command reaches the physical driver. */
static inline RinGpuResult rin_gpu_process_command_encoder_query_v1(
    RinGpuProcessCommandEncoderV1* encoder, uint32_t command_type,
    uint64_t process_query_token, uint32_t process_query_type)
{
    RinGpuBackendCommandV1* command;
    RinGpuProcessEncoderGraphicsStateV1 state;
    uint32_t backend_query_type;
    RinGpuResult result = rin_gpu_process_encoder_v1_validate(encoder);
    if (result != RIN_GPU_OK) return result;
    if (process_query_token == 0u) return RIN_GPU_ERROR_INVALID_ARGUMENT;
    result = rin_gpu_process_encoder_v1_query_type(process_query_type,
                                                   &backend_query_type);
    if (result != RIN_GPU_OK) return result;
    result = rin_gpu_process_encoder_v1_graphics_state(encoder, &state);
    if (result != RIN_GPU_OK) return result;
    switch (command_type) {
    case RIN_GPU_BACKEND_COMMAND_BEGIN_QUERY:
        if (state.query_cookie != 0u) return RIN_GPU_ERROR_STATE;
        break;
    case RIN_GPU_BACKEND_COMMAND_END_QUERY:
        if (state.query_cookie != process_query_token ||
            state.query_type != backend_query_type)
            return RIN_GPU_ERROR_STATE;
        break;
    case RIN_GPU_BACKEND_COMMAND_RESET_QUERY:
        if (state.query_cookie != 0u || state.render_pass_active != 0u)
            return RIN_GPU_ERROR_STATE;
        break;
    default:
        return RIN_GPU_ERROR_INVALID_ARGUMENT;
    }
    if (encoder->command_count == encoder->command_capacity)
        return RIN_GPU_ERROR_LIMIT;
    command = &encoder->commands[encoder->command_count];
    memset(command, 0, sizeof(*command));
    command->type = command_type;
    command->value.query.query_cookie = process_query_token;
    command->value.query.query_type = backend_query_type;
    ++encoder->command_count;
    return RIN_GPU_OK;
}

static inline RinGpuResult rin_gpu_process_command_encoder_begin_query_v1(
    RinGpuProcessCommandEncoderV1* encoder, uint64_t process_query_token,
    uint32_t process_query_type)
{
    return rin_gpu_process_command_encoder_query_v1(
        encoder, RIN_GPU_BACKEND_COMMAND_BEGIN_QUERY, process_query_token,
        process_query_type);
}

static inline RinGpuResult rin_gpu_process_command_encoder_end_query_v1(
    RinGpuProcessCommandEncoderV1* encoder, uint64_t process_query_token,
    uint32_t process_query_type)
{
    return rin_gpu_process_command_encoder_query_v1(
        encoder, RIN_GPU_BACKEND_COMMAND_END_QUERY, process_query_token,
        process_query_type);
}

static inline RinGpuResult rin_gpu_process_command_encoder_reset_query_v1(
    RinGpuProcessCommandEncoderV1* encoder, uint64_t process_query_token,
    uint32_t process_query_type)
{
    return rin_gpu_process_command_encoder_query_v1(
        encoder, RIN_GPU_BACKEND_COMMAND_RESET_QUERY, process_query_token,
        process_query_type);
}

static inline RinGpuResult rin_gpu_process_command_encoder_make_submit_v2(
    const RinGpuProcessCommandEncoderV1* encoder,
    RinGpuProcessSubmitDescV2* descriptor_out)
{
    RinGpuProcessEncoderGraphicsStateV1 state;
    RinGpuResult result = rin_gpu_process_encoder_v1_validate(encoder);
    if (result != RIN_GPU_OK) return result;
    result = rin_gpu_process_encoder_v1_graphics_state(encoder, &state);
    if (result != RIN_GPU_OK) return result;
    if (state.render_pass_active != 0u || state.query_cookie != 0u)
        return RIN_GPU_ERROR_STATE;
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
