/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_NPU_MODEL_QUANTIZED_HPP
#define RIN_DRIVER_NPU_MODEL_QUANTIZED_HPP

#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <rin/driver/npu_model.hpp>

/* Versioned per-tensor affine Q8 graph contract. This is separate from the
 * float32 v1 blob so existing model images and providers keep their layout. */
#define RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V1 UINT32_C(0x00010000)
#define RIN_NPU_QUANTIZED_MODEL_ABI_VERSION UINT32_C(0x00020000)
#define RIN_NPU_QUANTIZED_MODEL_BLOB_MAGIC UINT32_C(0x514e4952)
#define RIN_NPU_QUANTIZED_MODEL_MAX_BLOB_BYTES UINT32_C(16384)
#define RIN_NPU_QUANTIZED_MODEL_MAX_BACKENDS RIN_NPU_MODEL_MAX_BACKENDS

namespace rin {
namespace npu {
namespace model {
namespace quantized {

enum DataType {
    DATA_TYPE_INT8 = 2,
    DATA_TYPE_UINT8 = 3
};

enum OperationCode {
    OPERATION_ADD_Q8 = 0x101,
    OPERATION_MULTIPLY_Q8 = 0x102,
    OPERATION_RELU_Q8 = 0x103,
    OPERATION_MATMUL_Q8 = 0x104,
    OPERATION_SOFTMAX_Q8 = 0x105
};

struct TensorDescription {
    uint32_t data_type;
    uint32_t rank;
    uint32_t dimensions[RIN_NPU_MODEL_MAX_RANK];
    /* Real value = (stored value - zero_point) * scale. Scale must be finite
     * and positive; zero_point must fit the selected signed/unsigned Q8 type. */
    float scale;
    int32_t zero_point;
};

struct Operation {
    uint32_t opcode;
    uint32_t input_count;
    uint32_t inputs[2];
    uint32_t output_tensor;
    uint32_t reserved;
};

/* Optional per-axis affine parameters. This is a sidecar so the v1 tensor
 * description and its 48-byte serialized record remain unchanged. */
struct AxisQuantization {
    uint32_t tensor_index;
    uint32_t axis;
    uint32_t count;
    uint32_t reserved;
    const float* scales;
    const int32_t* zero_points;
};

struct Model {
    uint32_t struct_size;
    uint32_t version;
    uint32_t tensor_count;
    uint32_t input_count;
    uint32_t output_count;
    uint32_t operation_count;
    const TensorDescription* tensors;
    const uint32_t* inputs;
    const uint32_t* outputs;
    const Operation* operations;
    /* Appended in ABI v2. ABI v1 callers end at operations. */
    uint32_t quantization_count;
    uint32_t reserved;
    const AxisQuantization* quantizations;
};

#define RIN_NPU_QUANTIZED_MODEL_V1_STRUCT_SIZE \
    ((uint32_t)offsetof(Model, quantization_count))

struct TensorView {
    void* data;
    uint64_t element_count;
};

struct CpuPlan {
    uint32_t struct_size;
    uint32_t version;
    const Model* model;
    uint64_t model_hash;
};

/* Provider calls are synchronous. execute() must return only after the device
 * can no longer access any supplied tensor. recover() must quiesce an
 * uncertain submission before CPU fallback can run. */
struct BackendProvider {
    uint32_t struct_size;
    uint32_t version;
    uint32_t kind;
    uint32_t reserved;
    void* context;
    RinResultCode (*supports)(void* context, const Model* model,
                              uint32_t* supported_out);
    RinResultCode (*compile)(void* context, const Model* model,
                             void* scratch, uint32_t scratch_bytes,
                             uint64_t* plan_handle_out);
    RinResultCode (*execute)(void* context, uint64_t plan_handle,
                             TensorView* tensors, uint32_t tensor_count);
    RinResultCode (*recover)(void* context, uint64_t plan_handle);
    void (*release)(void* context, uint64_t plan_handle);
};

struct Executable {
    uint32_t struct_size;
    uint32_t version;
    uint32_t backend_kind;
    uint32_t allow_cpu_fallback;
    const Model* model;
    const BackendProvider* provider;
    uint64_t provider_plan_handle;
    uint64_t model_hash;
};

static_assert(sizeof(float) == 4u && FLT_RADIX == 2,
              "Q8 model scales require IEEE-754 binary32 storage");

static inline bool data_type_bounds(uint32_t data_type, int32_t* minimum,
                                    int32_t* maximum)
{
    if (minimum) *minimum = 0;
    if (maximum) *maximum = 0;
    if (!minimum || !maximum) return false;
    if (data_type == DATA_TYPE_INT8) {
        *minimum = -128;
        *maximum = 127;
        return true;
    }
    if (data_type == DATA_TYPE_UINT8) {
        *minimum = 0;
        *maximum = 255;
        return true;
    }
    return false;
}

static inline uint64_t tensor_elements(const TensorDescription& tensor)
{
    int32_t minimum = 0;
    int32_t maximum = 0;
    if (!data_type_bounds(tensor.data_type, &minimum, &maximum) ||
        tensor.zero_point < minimum || tensor.zero_point > maximum ||
        !(tensor.scale > 0.0f) || !(tensor.scale <= FLT_MAX) ||
        tensor.rank > RIN_NPU_MODEL_MAX_RANK)
        return 0u;
    uint64_t elements = 1u;
    for (uint32_t dimension = 0u; dimension < tensor.rank; ++dimension) {
        if (tensor.dimensions[dimension] == 0u ||
            elements > RIN_NPU_MODEL_MAX_TENSOR_ELEMENTS /
                           tensor.dimensions[dimension])
            return 0u;
        elements *= tensor.dimensions[dimension];
    }
    for (uint32_t dimension = tensor.rank;
         dimension < RIN_NPU_MODEL_MAX_RANK; ++dimension)
        if (tensor.dimensions[dimension] != 0u) return 0u;
    return elements;
}

static inline bool same_shape(const TensorDescription& left,
                              const TensorDescription& right)
{
    if (left.rank != right.rank) return false;
    for (uint32_t dimension = 0u; dimension < left.rank; ++dimension)
        if (left.dimensions[dimension] != right.dimensions[dimension])
            return false;
    return true;
}

static inline bool operation_shape_valid(const Model* model,
                                         const Operation& operation)
{
    const TensorDescription& output = model->tensors[operation.output_tensor];
    if (operation.opcode == OPERATION_ADD_Q8 ||
        operation.opcode == OPERATION_MULTIPLY_Q8) {
        return operation.input_count == 2u &&
            same_shape(output, model->tensors[operation.inputs[0]]) &&
            same_shape(output, model->tensors[operation.inputs[1]]);
    }
    if (operation.opcode == OPERATION_RELU_Q8) {
        return operation.input_count == 1u &&
            same_shape(output, model->tensors[operation.inputs[0]]);
    }
    if (operation.opcode == OPERATION_SOFTMAX_Q8) {
        return operation.input_count == 1u && output.rank != 0u &&
            same_shape(output, model->tensors[operation.inputs[0]]);
    }
    if (operation.opcode == OPERATION_MATMUL_Q8) {
        if (operation.input_count != 2u || output.rank != 2u) return false;
        const TensorDescription& left = model->tensors[operation.inputs[0]];
        const TensorDescription& right = model->tensors[operation.inputs[1]];
        return left.rank == 2u && right.rank == 2u &&
            left.dimensions[1] == right.dimensions[0] &&
            output.dimensions[0] == left.dimensions[0] &&
            output.dimensions[1] == right.dimensions[1];
    }
    return false;
}

static inline uint32_t model_quantization_count(const Model* model)
{
    return model && model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION
        ? model->quantization_count : 0u;
}

static inline const AxisQuantization* axis_quantization_for_tensor(
    const Model* model, uint32_t tensor_index)
{
    const uint32_t count = model_quantization_count(model);
    if (!model || (count && !model->quantizations)) return 0;
    for (uint32_t index = 0u; index < count; ++index)
        if (model->quantizations[index].tensor_index == tensor_index)
            return &model->quantizations[index];
    return 0;
}

static inline bool model_valid(const Model* model)
{
    if (!model ||
        (model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION
            ? model->struct_size != sizeof(*model) || model->reserved != 0u
            : model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V1
                ? model->struct_size != RIN_NPU_QUANTIZED_MODEL_V1_STRUCT_SIZE
                : true) ||
        model->tensor_count == 0u ||
        model->tensor_count > RIN_NPU_MODEL_MAX_TENSORS ||
        model->input_count == 0u || model->input_count > model->tensor_count ||
        model->output_count == 0u ||
        model->output_count > model->tensor_count ||
        model->operation_count > RIN_NPU_MODEL_MAX_OPERATIONS ||
        !model->tensors || !model->inputs || !model->outputs ||
        (model->operation_count != 0u && !model->operations))
        return false;

    const uint32_t quantization_count = model_quantization_count(model);
    if (quantization_count > model->tensor_count ||
        (quantization_count != 0u && !model->quantizations))
        return false;

    uint8_t available[RIN_NPU_MODEL_MAX_TENSORS] = {};
    for (uint32_t index = 0u; index < model->tensor_count; ++index)
        if (tensor_elements(model->tensors[index]) == 0u) return false;
    for (uint32_t index = 0u; index < quantization_count; ++index) {
        const AxisQuantization& axis = model->quantizations[index];
        if (axis.reserved != 0u || axis.tensor_index >= model->tensor_count ||
            axis.axis >= model->tensors[axis.tensor_index].rank ||
            axis.count != model->tensors[axis.tensor_index].dimensions[axis.axis] ||
            axis.count < 2u || !axis.scales || !axis.zero_points ||
            (axis.axis != 0u && axis.axis != 1u) ||
            model->tensors[axis.tensor_index].rank != 2u ||
            axis_quantization_for_tensor(model, axis.tensor_index) != &axis)
            return false;
        int32_t minimum = 0;
        int32_t maximum = 0;
        if (!data_type_bounds(model->tensors[axis.tensor_index].data_type,
                              &minimum, &maximum))
            return false;
        for (uint32_t parameter = 0u; parameter < axis.count; ++parameter)
            if (!(axis.scales[parameter] > 0.0f) ||
                !(axis.scales[parameter] <= FLT_MAX) ||
                axis.zero_points[parameter] < minimum ||
                axis.zero_points[parameter] > maximum)
                return false;
        for (uint32_t prior = 0u; prior < index; ++prior)
            if (model->quantizations[prior].tensor_index == axis.tensor_index)
                return false;
    }
    for (uint32_t index = 0u; index < model->input_count; ++index) {
        const uint32_t tensor = model->inputs[index];
        if (tensor >= model->tensor_count || available[tensor] != 0u)
            return false;
        available[tensor] = 1u;
    }
    for (uint32_t index = 0u; index < model->operation_count; ++index) {
        const Operation& operation = model->operations[index];
        if (operation.reserved != 0u ||
            operation.output_tensor >= model->tensor_count ||
            available[operation.output_tensor] != 0u ||
            operation.input_count == 0u || operation.input_count > 2u)
            return false;
        for (uint32_t input = 0u; input < operation.input_count; ++input)
            if (operation.inputs[input] >= model->tensor_count ||
                available[operation.inputs[input]] == 0u)
                return false;
        if (operation.input_count == 1u && operation.inputs[1] != 0u)
            return false;
        if (!operation_shape_valid(model, operation)) return false;
        const AxisQuantization* left_axis =
            axis_quantization_for_tensor(model, operation.inputs[0]);
        const AxisQuantization* right_axis = operation.input_count == 2u
            ? axis_quantization_for_tensor(model, operation.inputs[1]) : 0;
        if ((left_axis && (operation.opcode != OPERATION_MATMUL_Q8 ||
                           left_axis->axis != 0u)) ||
            (right_axis && (operation.opcode != OPERATION_MATMUL_Q8 ||
                            right_axis->axis != 1u)) ||
            axis_quantization_for_tensor(model, operation.output_tensor))
            return false;
        available[operation.output_tensor] = 1u;
    }
    for (uint32_t quantization = 0u;
         quantization < quantization_count; ++quantization) {
        const AxisQuantization& axis = model->quantizations[quantization];
        uint32_t used = 0u;
        for (uint32_t operation = 0u;
             operation < model->operation_count; ++operation) {
            const Operation& candidate = model->operations[operation];
            if (candidate.opcode == OPERATION_MATMUL_Q8 &&
                ((candidate.inputs[0] == axis.tensor_index && axis.axis == 0u) ||
                 (candidate.inputs[1] == axis.tensor_index && axis.axis == 1u)))
                used = 1u;
        }
        if (!used) return false;
    }
    for (uint32_t index = 0u; index < model->output_count; ++index) {
        const uint32_t tensor = model->outputs[index];
        if (tensor >= model->tensor_count || available[tensor] == 0u)
            return false;
        for (uint32_t prior = 0u; prior < index; ++prior)
            if (model->outputs[prior] == tensor) return false;
    }
    return true;
}

static inline bool model_storage_overlaps(const Model* model,
    const void* memory, uint64_t memory_bytes)
{
    if (!model || !memory || memory_bytes == 0u) return false;
    if (memory_ranges_overlap(memory, memory_bytes, model, model->struct_size) ||
        memory_ranges_overlap(memory, memory_bytes, model->tensors,
            (uint64_t)model->tensor_count * sizeof(*model->tensors)) ||
        memory_ranges_overlap(memory, memory_bytes, model->inputs,
            (uint64_t)model->input_count * sizeof(*model->inputs)) ||
        memory_ranges_overlap(memory, memory_bytes, model->outputs,
            (uint64_t)model->output_count * sizeof(*model->outputs)) ||
        memory_ranges_overlap(memory, memory_bytes, model->operations,
            (uint64_t)model->operation_count * sizeof(*model->operations)))
        return true;
    const uint32_t quantization_count = model_quantization_count(model);
    if (quantization_count && memory_ranges_overlap(
            memory, memory_bytes, model->quantizations,
            (uint64_t)quantization_count * sizeof(*model->quantizations)))
        return true;
    for (uint32_t index = 0u; index < quantization_count; ++index) {
        const AxisQuantization& axis = model->quantizations[index];
        if (memory_ranges_overlap(memory, memory_bytes, axis.scales,
                (uint64_t)axis.count * sizeof(*axis.scales)) ||
            memory_ranges_overlap(memory, memory_bytes, axis.zero_points,
                (uint64_t)axis.count * sizeof(*axis.zero_points)))
            return true;
    }
    return false;
}

static inline uint64_t model_hash(const Model* model)
{
    uint64_t hash = UINT64_C(14695981039346656037);
    hash = hash_u32(hash, model->tensor_count);
    hash = hash_u32(hash, model->input_count);
    hash = hash_u32(hash, model->output_count);
    hash = hash_u32(hash, model->operation_count);
    for (uint32_t index = 0u; index < model->tensor_count; ++index) {
        const TensorDescription& tensor = model->tensors[index];
        uint32_t scale_bits = 0u;
        memcpy(&scale_bits, &tensor.scale, sizeof(scale_bits));
        hash = hash_u32(hash, tensor.data_type);
        hash = hash_u32(hash, tensor.rank);
        for (uint32_t dimension = 0u;
             dimension < RIN_NPU_MODEL_MAX_RANK; ++dimension)
            hash = hash_u32(hash, tensor.dimensions[dimension]);
        hash = hash_u32(hash, scale_bits);
        hash = hash_u32(hash, (uint32_t)tensor.zero_point);
    }
    for (uint32_t index = 0u; index < model->input_count; ++index)
        hash = hash_u32(hash, model->inputs[index]);
    for (uint32_t index = 0u; index < model->output_count; ++index)
        hash = hash_u32(hash, model->outputs[index]);
    for (uint32_t index = 0u; index < model->operation_count; ++index) {
        const Operation& operation = model->operations[index];
        hash = hash_u32(hash, operation.opcode);
        hash = hash_u32(hash, operation.input_count);
        hash = hash_u32(hash, operation.inputs[0]);
        hash = hash_u32(hash, operation.inputs[1]);
        hash = hash_u32(hash, operation.output_tensor);
        hash = hash_u32(hash, operation.reserved);
    }
    const uint32_t quantization_count = model_quantization_count(model);
    hash = hash_u32(hash, quantization_count);
    for (uint32_t index = 0u; index < quantization_count; ++index) {
        const AxisQuantization& axis = model->quantizations[index];
        hash = hash_u32(hash, axis.tensor_index);
        hash = hash_u32(hash, axis.axis);
        hash = hash_u32(hash, axis.count);
        for (uint32_t parameter = 0u; parameter < axis.count; ++parameter) {
            uint32_t scale_bits = 0u;
            memcpy(&scale_bits, &axis.scales[parameter], sizeof(scale_bits));
            hash = hash_u32(hash, scale_bits);
            hash = hash_u32(hash, (uint32_t)axis.zero_points[parameter]);
        }
    }
    return hash;
}

static inline uint64_t serialized_size(const Model* model)
{
    const uint32_t quantization_count = model_quantization_count(model);
    uint64_t size = (quantization_count != 0u
                         ? UINT64_C(40) : UINT64_C(32)) +
        (uint64_t)model->tensor_count * 48u +
        (uint64_t)model->input_count * 4u +
        (uint64_t)model->output_count * 4u +
        (uint64_t)model->operation_count * 24u;
    for (uint32_t index = 0u; index < quantization_count; ++index) {
        const uint64_t parameters = model->quantizations[index].count;
        if (parameters > (UINT64_MAX - size - 16u) / 8u) return 0u;
        size += 16u + parameters * 8u;
    }
    return size;
}

static inline RinResultCode serialize(const Model* model, void* blob,
                                      uint32_t capacity,
                                      uint32_t* bytes_out)
{
    if (!bytes_out) return RIN_RESULT_INVALID_ARGUMENT;
    if (!model_valid(model)) return RIN_RESULT_INVALID_ARGUMENT;
    if (model_storage_overlaps(model, bytes_out, sizeof(*bytes_out)))
        return RIN_RESULT_INVALID_ARGUMENT;
    *bytes_out = 0u;
    const uint64_t required = serialized_size(model);
    if (required == 0u || required > RIN_NPU_QUANTIZED_MODEL_MAX_BLOB_BYTES ||
        required > UINT32_MAX)
        return RIN_RESULT_LIMIT_EXCEEDED;
    if (blob && memory_ranges_overlap(bytes_out, sizeof(*bytes_out), blob,
                                      required))
        return RIN_RESULT_INVALID_ARGUMENT;
    *bytes_out = (uint32_t)required;
    if (!blob || capacity < required) return RIN_RESULT_BUFFER_TOO_SMALL;
    if (model_storage_overlaps(model, blob, required))
        return RIN_RESULT_INVALID_ARGUMENT;
    uint8_t* output = static_cast<uint8_t*>(blob);
    const uint32_t quantization_count = model_quantization_count(model);
    const uint32_t serialized_version = quantization_count != 0u
        ? RIN_NPU_QUANTIZED_MODEL_ABI_VERSION
        : RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V1;
    write_u32_le(output, RIN_NPU_QUANTIZED_MODEL_BLOB_MAGIC);
    write_u32_le(output + 4u, serialized_version);
    const uint32_t header_bytes = quantization_count != 0u ? 40u : 32u;
    write_u32_le(output + 8u, header_bytes);
    write_u32_le(output + 12u, (uint32_t)required);
    write_u32_le(output + 16u, model->tensor_count);
    write_u32_le(output + 20u, model->input_count);
    write_u32_le(output + 24u, model->output_count);
    write_u32_le(output + 28u, model->operation_count);
    if (header_bytes == 40u) {
        write_u32_le(output + 32u, model->quantization_count);
        write_u32_le(output + 36u, 0u);
    }
    uint32_t offset = header_bytes;
    for (uint32_t index = 0u; index < model->tensor_count; ++index) {
        const TensorDescription& tensor = model->tensors[index];
        uint32_t scale_bits = 0u;
        memcpy(&scale_bits, &tensor.scale, sizeof(scale_bits));
        write_u32_le(output + offset, tensor.data_type);
        write_u32_le(output + offset + 4u, tensor.rank);
        for (uint32_t dimension = 0u;
             dimension < RIN_NPU_MODEL_MAX_RANK; ++dimension)
            write_u32_le(output + offset + 8u + dimension * 4u,
                         tensor.dimensions[dimension]);
        write_u32_le(output + offset + 40u, scale_bits);
        write_u32_le(output + offset + 44u,
                     (uint32_t)tensor.zero_point);
        offset += 48u;
    }
    for (uint32_t index = 0u; index < model->input_count; ++index) {
        write_u32_le(output + offset, model->inputs[index]);
        offset += 4u;
    }
    for (uint32_t index = 0u; index < model->output_count; ++index) {
        write_u32_le(output + offset, model->outputs[index]);
        offset += 4u;
    }
    for (uint32_t index = 0u; index < model->operation_count; ++index) {
        const Operation& operation = model->operations[index];
        write_u32_le(output + offset, operation.opcode);
        write_u32_le(output + offset + 4u, operation.input_count);
        write_u32_le(output + offset + 8u, operation.inputs[0]);
        write_u32_le(output + offset + 12u, operation.inputs[1]);
        write_u32_le(output + offset + 16u, operation.output_tensor);
        write_u32_le(output + offset + 20u, operation.reserved);
        offset += 24u;
    }
    for (uint32_t index = 0u; index < quantization_count; ++index) {
        const AxisQuantization& axis = model->quantizations[index];
        write_u32_le(output + offset, axis.tensor_index);
        write_u32_le(output + offset + 4u, axis.axis);
        write_u32_le(output + offset + 8u, axis.count);
        write_u32_le(output + offset + 12u, 0u);
        offset += 16u;
        for (uint32_t parameter = 0u; parameter < axis.count; ++parameter) {
            uint32_t scale_bits = 0u;
            memcpy(&scale_bits, &axis.scales[parameter], sizeof(scale_bits));
            write_u32_le(output + offset, scale_bits);
            offset += 4u;
        }
        for (uint32_t parameter = 0u; parameter < axis.count; ++parameter) {
            write_u32_le(output + offset,
                         (uint32_t)axis.zero_points[parameter]);
            offset += 4u;
        }
    }
    return RIN_RESULT_OK;
}

static inline RinResultCode deserialize(const void* blob, uint32_t blob_bytes,
    TensorDescription* tensor_storage, uint32_t tensor_capacity,
    uint32_t* input_storage, uint32_t input_capacity,
    uint32_t* output_storage, uint32_t output_capacity,
    Operation* operation_storage, uint32_t operation_capacity,
    Model* model_out, AxisQuantization* quantization_storage = 0,
    uint32_t quantization_capacity = 0u, float* scale_storage = 0,
    int32_t* zero_point_storage = 0, uint32_t parameter_capacity = 0u)
{
    if (!model_out) return RIN_RESULT_INVALID_ARGUMENT;
    if (!blob || blob_bytes < 32u ||
        blob_bytes > RIN_NPU_QUANTIZED_MODEL_MAX_BLOB_BYTES ||
        memory_ranges_overlap(model_out, sizeof(*model_out), blob, blob_bytes))
        return RIN_RESULT_INVALID_ARGUMENT;
    const uint8_t* input = static_cast<const uint8_t*>(blob);
    const uint32_t tensor_count = read_u32_le(input + 16u);
    const uint32_t input_count = read_u32_le(input + 20u);
    const uint32_t output_count = read_u32_le(input + 24u);
    const uint32_t operation_count = read_u32_le(input + 28u);
    const uint32_t version = read_u32_le(input + 4u);
    const bool version1 = version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V1;
    const bool version2 = version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION;
    if ((!version1 && !version2) || (version2 && blob_bytes < 40u))
        return RIN_RESULT_CORRUPT_DATA;
    if (read_u32_le(input) != RIN_NPU_QUANTIZED_MODEL_BLOB_MAGIC ||
        read_u32_le(input + 8u) != (version1 ? 32u : 40u) ||
        read_u32_le(input + 12u) != blob_bytes || tensor_count == 0u ||
        tensor_count > RIN_NPU_MODEL_MAX_TENSORS || input_count == 0u ||
        input_count > tensor_count || output_count == 0u ||
        output_count > tensor_count ||
        operation_count > RIN_NPU_MODEL_MAX_OPERATIONS)
        return RIN_RESULT_CORRUPT_DATA;
    const uint32_t header_bytes = version1 ? 32u : 40u;
    const uint32_t axis_count = version1 ? 0u : read_u32_le(input + 32u);
    if ((!version1 && read_u32_le(input + 36u) != 0u) ||
        axis_count > tensor_count)
        return RIN_RESULT_CORRUPT_DATA;
    uint64_t expected = (uint64_t)header_bytes +
        (uint64_t)tensor_count * 48u + (uint64_t)input_count * 4u +
        (uint64_t)output_count * 4u +
        (uint64_t)operation_count * 24u;
    if (expected > blob_bytes) return RIN_RESULT_CORRUPT_DATA;
    uint64_t parameter_count = 0u;
    uint32_t scan_offset = (uint32_t)expected;
    for (uint32_t index = 0u; index < axis_count; ++index) {
        if (scan_offset > blob_bytes || blob_bytes - scan_offset < 16u)
            return RIN_RESULT_CORRUPT_DATA;
        const uint32_t count = read_u32_le(input + scan_offset + 8u);
        if (read_u32_le(input + scan_offset + 12u) != 0u || count < 2u ||
            count > (blob_bytes - scan_offset - 16u) / 8u ||
            parameter_count > UINT32_MAX - count)
            return RIN_RESULT_CORRUPT_DATA;
        parameter_count += count;
        scan_offset += 16u + count * 8u;
    }
    if (scan_offset != blob_bytes) return RIN_RESULT_CORRUPT_DATA;
    if (tensor_capacity < tensor_count || input_capacity < input_count ||
        output_capacity < output_count ||
        operation_capacity < operation_count || !tensor_storage ||
        !input_storage || !output_storage ||
        (operation_count != 0u && !operation_storage) ||
        quantization_capacity < axis_count ||
        parameter_capacity < parameter_count ||
        (axis_count && !quantization_storage) ||
        (parameter_count && (!scale_storage || !zero_point_storage)))
        return RIN_RESULT_BUFFER_TOO_SMALL;
    const uint64_t tensor_bytes =
        (uint64_t)tensor_count * sizeof(*tensor_storage);
    const uint64_t input_bytes = (uint64_t)input_count * sizeof(*input_storage);
    const uint64_t output_bytes =
        (uint64_t)output_count * sizeof(*output_storage);
    const uint64_t operation_bytes =
        (uint64_t)operation_count * sizeof(*operation_storage);
    const uint64_t quantization_bytes =
        (uint64_t)axis_count * sizeof(*quantization_storage);
    const uint64_t parameter_bytes = parameter_count * sizeof(float);
    if (memory_ranges_overlap(blob, blob_bytes, tensor_storage, tensor_bytes) ||
        memory_ranges_overlap(blob, blob_bytes, input_storage, input_bytes) ||
        memory_ranges_overlap(blob, blob_bytes, output_storage, output_bytes) ||
        memory_ranges_overlap(blob, blob_bytes, operation_storage,
                              operation_bytes) ||
        memory_ranges_overlap(blob, blob_bytes, quantization_storage,
                              quantization_bytes) ||
        memory_ranges_overlap(blob, blob_bytes, scale_storage,
                              parameter_bytes) ||
        memory_ranges_overlap(blob, blob_bytes, zero_point_storage,
                              parameter_bytes) ||
        memory_ranges_overlap(tensor_storage, tensor_bytes, input_storage,
                              input_bytes) ||
        memory_ranges_overlap(tensor_storage, tensor_bytes, output_storage,
                              output_bytes) ||
        memory_ranges_overlap(tensor_storage, tensor_bytes, operation_storage,
                              operation_bytes) ||
        memory_ranges_overlap(tensor_storage, tensor_bytes,
                              quantization_storage, quantization_bytes) ||
        memory_ranges_overlap(tensor_storage, tensor_bytes, scale_storage,
                              parameter_bytes) ||
        memory_ranges_overlap(tensor_storage, tensor_bytes, zero_point_storage,
                              parameter_bytes) ||
        memory_ranges_overlap(input_storage, input_bytes, output_storage,
                              output_bytes) ||
        memory_ranges_overlap(input_storage, input_bytes, operation_storage,
                              operation_bytes) ||
        memory_ranges_overlap(input_storage, input_bytes,
                              quantization_storage, quantization_bytes) ||
        memory_ranges_overlap(input_storage, input_bytes, scale_storage,
                              parameter_bytes) ||
        memory_ranges_overlap(input_storage, input_bytes, zero_point_storage,
                              parameter_bytes) ||
        memory_ranges_overlap(output_storage, output_bytes, operation_storage,
                              operation_bytes) ||
        memory_ranges_overlap(output_storage, output_bytes,
                              quantization_storage, quantization_bytes) ||
        memory_ranges_overlap(output_storage, output_bytes, scale_storage,
                              parameter_bytes) ||
        memory_ranges_overlap(output_storage, output_bytes,
                              zero_point_storage, parameter_bytes) ||
        memory_ranges_overlap(model_out, sizeof(*model_out), tensor_storage,
                              tensor_bytes) ||
        memory_ranges_overlap(model_out, sizeof(*model_out), input_storage,
                              input_bytes) ||
        memory_ranges_overlap(model_out, sizeof(*model_out), output_storage,
                              output_bytes) ||
        memory_ranges_overlap(model_out, sizeof(*model_out), operation_storage,
                              operation_bytes) ||
        memory_ranges_overlap(model_out, sizeof(*model_out),
                              quantization_storage, quantization_bytes) ||
        memory_ranges_overlap(model_out, sizeof(*model_out), scale_storage,
                              parameter_bytes) ||
        memory_ranges_overlap(model_out, sizeof(*model_out),
                              zero_point_storage, parameter_bytes) ||
        memory_ranges_overlap(quantization_storage, quantization_bytes,
                              scale_storage, parameter_bytes) ||
        memory_ranges_overlap(quantization_storage, quantization_bytes,
                              zero_point_storage, parameter_bytes) ||
        memory_ranges_overlap(scale_storage, parameter_bytes,
                              zero_point_storage, parameter_bytes))
        return RIN_RESULT_INVALID_ARGUMENT;

    *model_out = {};
    uint32_t offset = header_bytes;
    for (uint32_t index = 0u; index < tensor_count; ++index) {
        tensor_storage[index].data_type = read_u32_le(input + offset);
        tensor_storage[index].rank = read_u32_le(input + offset + 4u);
        for (uint32_t dimension = 0u;
             dimension < RIN_NPU_MODEL_MAX_RANK; ++dimension)
            tensor_storage[index].dimensions[dimension] =
                read_u32_le(input + offset + 8u + dimension * 4u);
        const uint32_t scale_bits = read_u32_le(input + offset + 40u);
        memcpy(&tensor_storage[index].scale, &scale_bits, sizeof(scale_bits));
        const uint32_t zero_point_bits = read_u32_le(input + offset + 44u);
        memcpy(&tensor_storage[index].zero_point, &zero_point_bits,
               sizeof(zero_point_bits));
        offset += 48u;
    }
    for (uint32_t index = 0u; index < input_count; ++index) {
        input_storage[index] = read_u32_le(input + offset);
        offset += 4u;
    }
    for (uint32_t index = 0u; index < output_count; ++index) {
        output_storage[index] = read_u32_le(input + offset);
        offset += 4u;
    }
    for (uint32_t index = 0u; index < operation_count; ++index) {
        operation_storage[index].opcode = read_u32_le(input + offset);
        operation_storage[index].input_count =
            read_u32_le(input + offset + 4u);
        operation_storage[index].inputs[0] =
            read_u32_le(input + offset + 8u);
        operation_storage[index].inputs[1] =
            read_u32_le(input + offset + 12u);
        operation_storage[index].output_tensor =
            read_u32_le(input + offset + 16u);
        operation_storage[index].reserved =
            read_u32_le(input + offset + 20u);
        offset += 24u;
    }
    uint32_t parameter_offset = 0u;
    for (uint32_t index = 0u; index < axis_count; ++index) {
        AxisQuantization& axis = quantization_storage[index];
        axis.tensor_index = read_u32_le(input + offset);
        axis.axis = read_u32_le(input + offset + 4u);
        axis.count = read_u32_le(input + offset + 8u);
        axis.reserved = 0u;
        offset += 16u;
        axis.scales = scale_storage + parameter_offset;
        axis.zero_points = zero_point_storage + parameter_offset;
        for (uint32_t parameter = 0u; parameter < axis.count; ++parameter) {
            const uint32_t bits = read_u32_le(input + offset);
            memcpy(&scale_storage[parameter_offset + parameter], &bits,
                   sizeof(bits));
            offset += 4u;
        }
        for (uint32_t parameter = 0u; parameter < axis.count; ++parameter) {
            zero_point_storage[parameter_offset + parameter] =
                (int32_t)read_u32_le(input + offset);
            offset += 4u;
        }
        parameter_offset += axis.count;
    }
    Model candidate = {};
    candidate.struct_size = version1
        ? RIN_NPU_QUANTIZED_MODEL_V1_STRUCT_SIZE : sizeof(candidate);
    candidate.version = version;
    candidate.tensor_count = tensor_count;
    candidate.input_count = input_count;
    candidate.output_count = output_count;
    candidate.operation_count = operation_count;
    candidate.tensors = tensor_storage;
    candidate.inputs = input_storage;
    candidate.outputs = output_storage;
    candidate.operations = operation_storage;
    if (version2) {
        candidate.quantization_count = axis_count;
        candidate.quantizations = quantization_storage;
    }
    if (!model_valid(&candidate)) return RIN_RESULT_CORRUPT_DATA;
    *model_out = candidate;
    return RIN_RESULT_OK;
}

static inline RinResultCode validate_views(const Model* model,
                                           const TensorView* tensors,
                                           uint32_t tensor_count)
{
    if (!model_valid(model) || !tensors || tensor_count != model->tensor_count)
        return RIN_RESULT_INVALID_ARGUMENT;
    uintptr_t starts[RIN_NPU_MODEL_MAX_TENSORS] = {};
    uint64_t byte_counts[RIN_NPU_MODEL_MAX_TENSORS] = {};
    for (uint32_t index = 0u; index < tensor_count; ++index) {
        const uint64_t elements = tensor_elements(model->tensors[index]);
        if (!tensors[index].data || tensors[index].element_count != elements ||
            elements > UINTPTR_MAX)
            return RIN_RESULT_INVALID_ARGUMENT;
        const uintptr_t start = (uintptr_t)tensors[index].data;
        const uint64_t bytes = elements;
        if (bytes > UINTPTR_MAX - start) return RIN_RESULT_INVALID_ARGUMENT;
        if (memory_ranges_overlap(tensors[index].data, bytes, tensors,
                                  (uint64_t)tensor_count * sizeof(*tensors)) ||
            model_storage_overlaps(model, tensors[index].data, bytes))
            return RIN_RESULT_INVALID_ARGUMENT;
        starts[index] = start;
        byte_counts[index] = bytes;
        for (uint32_t prior = 0u; prior < index; ++prior) {
            const uintptr_t prior_end =
                starts[prior] + (uintptr_t)byte_counts[prior];
            const uintptr_t end = start + (uintptr_t)bytes;
            if (start < prior_end && starts[prior] < end)
                return RIN_RESULT_INVALID_ARGUMENT;
        }
    }
    return RIN_RESULT_OK;
}

static inline int32_t load_q8(const TensorDescription& description,
                              const void* data, uint64_t index)
{
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    if (description.data_type == DATA_TYPE_INT8)
        return static_cast<const int8_t*>(static_cast<const void*>(bytes))[index];
    return bytes[index];
}

static inline void model_quantization_at(const Model* model,
    uint32_t tensor_index, uint64_t element_index, float* scale_out,
    int32_t* zero_point_out)
{
    const TensorDescription& description = model->tensors[tensor_index];
    *scale_out = description.scale;
    *zero_point_out = description.zero_point;
    const AxisQuantization* axis =
        axis_quantization_for_tensor(model, tensor_index);
    if (!axis) return;
    uint64_t inner = 1u;
    for (uint32_t dimension = axis->axis + 1u;
         dimension < description.rank; ++dimension)
        inner *= description.dimensions[dimension];
    const uint64_t coordinate =
        (element_index / inner) % description.dimensions[axis->axis];
    *scale_out = axis->scales[coordinate];
    *zero_point_out = axis->zero_points[coordinate];
}

static inline double dequantize_model_value(const Model* model,
    uint32_t tensor_index, const void* data, uint64_t element_index)
{
    const TensorDescription& description = model->tensors[tensor_index];
    float scale = 0.0f;
    int32_t zero_point = 0;
    model_quantization_at(model, tensor_index, element_index, &scale,
                          &zero_point);
    return (double)(load_q8(description, data, element_index) - zero_point) *
           (double)scale;
}

static inline double dequantize_value(const TensorDescription& description,
                                      const void* data, uint64_t index)
{
    const int32_t value = load_q8(description, data, index);
    return (double)(value - description.zero_point) *
           (double)description.scale;
}

static inline int64_t round_ties_to_even(double value)
{
    const double lower = floor(value);
    const double fraction = value - lower;
    int64_t rounded = (int64_t)lower;
    if (fraction > 0.5 ||
        (fraction == 0.5 && (rounded % INT64_C(2)) != 0))
        ++rounded;
    return rounded;
}

static inline void store_quantized(const TensorDescription& description,
                                   void* data, uint64_t index,
                                   double real_value)
{
    int32_t minimum = 0;
    int32_t maximum = 0;
    data_type_bounds(description.data_type, &minimum, &maximum);
    const double scaled = real_value / (double)description.scale +
                          (double)description.zero_point;
    int64_t quantized;
    if (scaled <= (double)minimum - 1.0)
        quantized = minimum;
    else if (scaled >= (double)maximum + 1.0)
        quantized = maximum;
    else
        quantized = round_ties_to_even(scaled);
    if (quantized < minimum) quantized = minimum;
    if (quantized > maximum) quantized = maximum;
    if (description.data_type == DATA_TYPE_INT8)
        static_cast<int8_t*>(data)[index] = (int8_t)quantized;
    else
        static_cast<uint8_t*>(data)[index] = (uint8_t)quantized;
}

static inline RinResultCode compile_cpu(const Model* model,
                                        CpuPlan* plan_out)
{
    if (!plan_out) return RIN_RESULT_INVALID_ARGUMENT;
    if (!model_valid(model)) return RIN_RESULT_INVALID_ARGUMENT;
    if (model_storage_overlaps(model, plan_out, sizeof(*plan_out)))
        return RIN_RESULT_INVALID_ARGUMENT;
    *plan_out = {};
    plan_out->struct_size = sizeof(*plan_out);
    plan_out->version = RIN_NPU_QUANTIZED_MODEL_ABI_VERSION;
    plan_out->model = model;
    plan_out->model_hash = model_hash(model);
    return RIN_RESULT_OK;
}

static inline RinResultCode evaluate_cpu(const CpuPlan* plan,
    TensorView* tensors, uint32_t tensor_count)
{
    if (!plan || plan->struct_size != sizeof(*plan) ||
        plan->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION ||
        !model_valid(plan->model) || model_hash(plan->model) != plan->model_hash)
        return RIN_RESULT_CORRUPT_DATA;
    const RinResultCode view_status =
        validate_views(plan->model, tensors, tensor_count);
    if (view_status != RIN_RESULT_OK) return view_status;
    for (uint32_t index = 0u; index < tensor_count; ++index)
        if (memory_ranges_overlap(tensors[index].data,
                tensors[index].element_count, plan, sizeof(*plan)))
            return RIN_RESULT_INVALID_ARGUMENT;
    const Model* model = plan->model;
    for (uint32_t index = 0u; index < model->operation_count; ++index) {
        const Operation& operation = model->operations[index];
        const uint32_t output_index = operation.output_tensor;
        const TensorDescription& output_description =
            model->tensors[output_index];
        void* output = tensors[output_index].data;
        if (operation.opcode == OPERATION_ADD_Q8 ||
            operation.opcode == OPERATION_MULTIPLY_Q8) {
            const uint32_t left_index = operation.inputs[0];
            const uint32_t right_index = operation.inputs[1];
            const TensorDescription& left_description =
                model->tensors[left_index];
            const TensorDescription& right_description =
                model->tensors[right_index];
            const uint64_t elements = tensors[output_index].element_count;
            for (uint64_t element = 0u; element < elements; ++element) {
                const double left = dequantize_value(
                    left_description, tensors[left_index].data, element);
                const double right = dequantize_value(
                    right_description, tensors[right_index].data, element);
                const double value = operation.opcode == OPERATION_ADD_Q8
                    ? left + right : left * right;
                store_quantized(output_description, output, element, value);
            }
        } else if (operation.opcode == OPERATION_RELU_Q8) {
            const uint32_t input_index = operation.inputs[0];
            const TensorDescription& input_description =
                model->tensors[input_index];
            const uint64_t elements = tensors[output_index].element_count;
            for (uint64_t element = 0u; element < elements; ++element) {
                double value = dequantize_value(
                    input_description, tensors[input_index].data, element);
                if (value < 0.0) value = 0.0;
                store_quantized(output_description, output, element, value);
            }
        } else if (operation.opcode == OPERATION_MATMUL_Q8) {
            const uint32_t left_index = operation.inputs[0];
            const uint32_t right_index = operation.inputs[1];
            const TensorDescription& left_description =
                model->tensors[left_index];
            const TensorDescription& right_description =
                model->tensors[right_index];
            const void* left = tensors[left_index].data;
            const void* right = tensors[right_index].data;
            const uint32_t rows = left_description.dimensions[0];
            const uint32_t inner = left_description.dimensions[1];
            const uint32_t columns = right_description.dimensions[1];
            for (uint32_t row = 0u; row < rows; ++row)
                for (uint32_t column = 0u; column < columns; ++column) {
                    float left_scale = 0.0f;
                    int32_t left_zero_point = 0;
                    float right_scale = 0.0f;
                    int32_t right_zero_point = 0;
                    model_quantization_at(model, left_index,
                        (uint64_t)row * inner, &left_scale,
                        &left_zero_point);
                    model_quantization_at(model, right_index, column,
                        &right_scale, &right_zero_point);
                    const double output_scale =
                        (double)left_scale * (double)right_scale;
                    int64_t accumulator = 0;
                    for (uint32_t element = 0u; element < inner; ++element) {
                        const uint64_t left_offset =
                            (uint64_t)row * inner + element;
                        const uint64_t right_offset =
                            (uint64_t)element * columns + column;
                        const int64_t left_value =
                            load_q8(left_description, left, left_offset) -
                            left_zero_point;
                        const int64_t right_value =
                            load_q8(right_description, right, right_offset) -
                            right_zero_point;
                        accumulator += left_value * right_value;
                    }
                    const double real_value =
                        (double)accumulator * output_scale;
                    const uint64_t output_offset =
                        (uint64_t)row * columns + column;
                    store_quantized(output_description, output,
                                    output_offset, real_value);
                }
        } else if (operation.opcode == OPERATION_SOFTMAX_Q8) {
            const uint32_t input_index = operation.inputs[0];
            const TensorDescription& input_description =
                model->tensors[input_index];
            const uint32_t axis =
                input_description.dimensions[input_description.rank - 1u];
            const uint64_t rows = tensors[output_index].element_count / axis;
            for (uint64_t row = 0u; row < rows; ++row) {
                const uint64_t base = row * axis;
                double maximum = dequantize_value(
                    input_description, tensors[input_index].data, base);
                for (uint32_t element = 1u; element < axis; ++element) {
                    const double value = dequantize_value(
                        input_description, tensors[input_index].data,
                        base + element);
                    if (value > maximum) maximum = value;
                }
                double sum = 0.0;
                for (uint32_t element = 0u; element < axis; ++element) {
                    const double value = dequantize_value(
                        input_description, tensors[input_index].data,
                        base + element);
                    sum += exp(value - maximum);
                }
                for (uint32_t element = 0u; element < axis; ++element) {
                    const double value = dequantize_value(
                        input_description, tensors[input_index].data,
                        base + element);
                    const double exponential = exp(value - maximum);
                    store_quantized(output_description, output,
                                    base + element, exponential / sum);
                }
            }
        }
    }
    return RIN_RESULT_OK;
}

static inline bool backend_provider_valid(const BackendProvider* provider)
{
    return provider && provider->struct_size == sizeof(*provider) &&
        (provider->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V1 ||
         provider->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION) &&
        (provider->kind == BACKEND_GPU || provider->kind == BACKEND_NPU) &&
        provider->reserved == 0u && provider->supports && provider->compile &&
        provider->execute && provider->recover && provider->release;
}

static inline RinResultCode compile(const Model* model,
    const BackendProvider* const* providers, uint32_t provider_count,
    void* scratch, uint32_t scratch_bytes, bool allow_cpu_fallback,
    Executable* executable_out)
{
    if (!executable_out || provider_count > RIN_NPU_QUANTIZED_MODEL_MAX_BACKENDS ||
        scratch_bytes > RIN_NPU_MODEL_MAX_COMPILER_SCRATCH_BYTES ||
        (provider_count != 0u && !providers) ||
        (scratch_bytes != 0u && !scratch))
        return RIN_RESULT_INVALID_ARGUMENT;
    if (!model_valid(model)) return RIN_RESULT_INVALID_ARGUMENT;
    const uint64_t provider_array_bytes =
        (uint64_t)provider_count * sizeof(*providers);
    if (model_storage_overlaps(model, executable_out,
                               sizeof(*executable_out)) ||
        memory_ranges_overlap(executable_out, sizeof(*executable_out),
                              providers, provider_array_bytes) ||
        memory_ranges_overlap(executable_out, sizeof(*executable_out),
                              scratch, scratch_bytes) ||
        memory_ranges_overlap(scratch, scratch_bytes, providers,
                              provider_array_bytes) ||
        model_storage_overlaps(model, scratch, scratch_bytes))
        return RIN_RESULT_INVALID_ARGUMENT;
    for (uint32_t index = 0u; index < provider_count; ++index) {
        const BackendProvider* provider = providers[index];
        if (!backend_provider_valid(provider)) return RIN_RESULT_CORRUPT_DATA;
        if (provider->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V1 &&
            model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V1)
            continue;
        if (memory_ranges_overlap(executable_out, sizeof(*executable_out),
                                  provider, sizeof(*provider)) ||
            memory_ranges_overlap(scratch, scratch_bytes,
                                  provider, sizeof(*provider)))
            return RIN_RESULT_INVALID_ARGUMENT;
    }
    *executable_out = {};
    const uint64_t hash = model_hash(model);
    for (uint32_t index = 0u; index < provider_count; ++index) {
        const BackendProvider* provider = providers[index];
        uint32_t supported = 0u;
        RinResultCode status = provider->supports(
            provider->context, model, &supported);
        if (status == RIN_RESULT_NOT_SUPPORTED ||
            (status == RIN_RESULT_OK && supported == 0u))
            continue;
        if (status != RIN_RESULT_OK) {
            if (status == RIN_RESULT_INVALID_ARGUMENT ||
                status == RIN_RESULT_CORRUPT_DATA ||
                status == RIN_RESULT_ACCESS_DENIED ||
                status == RIN_RESULT_VERSION_MISMATCH ||
                status == RIN_RESULT_SIGNATURE_REJECTED)
                return status;
            continue;
        }
        if (supported != 1u) return RIN_RESULT_CORRUPT_DATA;
        uint64_t plan_handle = 0u;
        if (scratch_bytes != 0u) {
            uint8_t* work = static_cast<uint8_t*>(scratch);
            for (uint32_t byte = 0u; byte < scratch_bytes; ++byte)
                work[byte] = 0u;
        }
        status = provider->compile(provider->context, model, scratch,
                                   scratch_bytes, &plan_handle);
        if (status == RIN_RESULT_OK && plan_handle != 0u) {
            executable_out->struct_size = sizeof(*executable_out);
            executable_out->version = RIN_NPU_QUANTIZED_MODEL_ABI_VERSION;
            executable_out->backend_kind = provider->kind;
            executable_out->allow_cpu_fallback = allow_cpu_fallback ? 1u : 0u;
            executable_out->model = model;
            executable_out->provider = provider;
            executable_out->provider_plan_handle = plan_handle;
            executable_out->model_hash = hash;
            return RIN_RESULT_OK;
        }
        if (plan_handle != 0u)
            provider->release(provider->context, plan_handle);
        if (status == RIN_RESULT_OK) return RIN_RESULT_CORRUPT_DATA;
        if (status == RIN_RESULT_INVALID_ARGUMENT ||
            status == RIN_RESULT_CORRUPT_DATA ||
            status == RIN_RESULT_ACCESS_DENIED ||
            status == RIN_RESULT_VERSION_MISMATCH ||
            status == RIN_RESULT_SIGNATURE_REJECTED)
            return status;
    }
    if (!allow_cpu_fallback) return RIN_RESULT_NOT_SUPPORTED;
    executable_out->struct_size = sizeof(*executable_out);
    executable_out->version = RIN_NPU_QUANTIZED_MODEL_ABI_VERSION;
    executable_out->backend_kind = BACKEND_CPU;
    executable_out->allow_cpu_fallback = 1u;
    executable_out->model = model;
    executable_out->model_hash = hash;
    return RIN_RESULT_OK;
}

static inline RinResultCode execute(Executable* executable,
    TensorView* tensors, uint32_t tensor_count)
{
    if (!executable || executable->struct_size != sizeof(*executable) ||
        executable->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION ||
        executable->allow_cpu_fallback > 1u ||
        !model_valid(executable->model) ||
        model_hash(executable->model) != executable->model_hash)
        return RIN_RESULT_CORRUPT_DATA;
    const RinResultCode view_status =
        validate_views(executable->model, tensors, tensor_count);
    if (view_status != RIN_RESULT_OK) return view_status;
    for (uint32_t index = 0u; index < tensor_count; ++index)
        if (memory_ranges_overlap(tensors[index].data,
                tensors[index].element_count, executable,
                sizeof(*executable)))
            return RIN_RESULT_INVALID_ARGUMENT;
    if (executable->backend_kind == BACKEND_CPU) {
        if (executable->provider || executable->provider_plan_handle != 0u)
            return RIN_RESULT_CORRUPT_DATA;
        CpuPlan plan = {};
        plan.struct_size = sizeof(plan);
        plan.version = RIN_NPU_QUANTIZED_MODEL_ABI_VERSION;
        plan.model = executable->model;
        plan.model_hash = executable->model_hash;
        return evaluate_cpu(&plan, tensors, tensor_count);
    }
    if (!backend_provider_valid(executable->provider) ||
        executable->provider->kind != executable->backend_kind ||
        executable->provider_plan_handle == 0u)
        return RIN_RESULT_CORRUPT_DATA;
    for (uint32_t index = 0u; index < tensor_count; ++index)
        if (memory_ranges_overlap(tensors[index].data,
                tensors[index].element_count, executable->provider,
                sizeof(*executable->provider)))
            return RIN_RESULT_INVALID_ARGUMENT;
    const RinResultCode status = executable->provider->execute(
        executable->provider->context, executable->provider_plan_handle,
        tensors, tensor_count);
    if (status == RIN_RESULT_OK) return status;
    if (executable->allow_cpu_fallback == 0u ||
        !backend_error_fallback_eligible(status))
        return status;
    const RinResultCode recovery = executable->provider->recover(
        executable->provider->context, executable->provider_plan_handle);
    if (recovery != RIN_RESULT_OK) return recovery;
    executable->provider->release(executable->provider->context,
                                  executable->provider_plan_handle);
    executable->provider = 0;
    executable->provider_plan_handle = 0u;
    executable->backend_kind = BACKEND_CPU;
    CpuPlan plan = {};
    plan.struct_size = sizeof(plan);
    plan.version = RIN_NPU_QUANTIZED_MODEL_ABI_VERSION;
    plan.model = executable->model;
    plan.model_hash = executable->model_hash;
    return evaluate_cpu(&plan, tensors, tensor_count);
}

static inline RinResultCode release(Executable* executable)
{
    if (!executable || executable->struct_size != sizeof(*executable) ||
        executable->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION ||
        executable->allow_cpu_fallback > 1u)
        return RIN_RESULT_INVALID_ARGUMENT;
    if (executable->backend_kind == BACKEND_CPU) {
        if (executable->provider || executable->provider_plan_handle != 0u)
            return RIN_RESULT_CORRUPT_DATA;
    } else if (!backend_provider_valid(executable->provider) ||
               executable->provider->kind != executable->backend_kind ||
               executable->provider_plan_handle == 0u) {
        return RIN_RESULT_CORRUPT_DATA;
    }
    if (executable->provider) {
        executable->provider->release(executable->provider->context,
                                      executable->provider_plan_handle);
    }
    *executable = {};
    return RIN_RESULT_OK;
}

} /* namespace quantized */
} /* namespace model */
} /* namespace npu */
} /* namespace rin */

#endif /* RIN_DRIVER_NPU_MODEL_QUANTIZED_HPP */
