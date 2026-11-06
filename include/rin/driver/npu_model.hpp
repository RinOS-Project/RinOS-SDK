/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_NPU_MODEL_HPP
#define RIN_DRIVER_NPU_MODEL_HPP

#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include <rin/contract_abi.h>

/* A bounded, allocation-free portable model graph. Serialized models are
 * little-endian and contain only graph metadata; weights are ordinary graph
 * inputs so callers can keep them in separately registered memory. */
#define RIN_NPU_MODEL_ABI_VERSION UINT32_C(0x00010000)
#define RIN_NPU_MODEL_BLOB_MAGIC UINT32_C(0x4d4e4952)
#define RIN_NPU_MODEL_MAX_TENSORS UINT32_C(128)
#define RIN_NPU_MODEL_MAX_OPERATIONS UINT32_C(128)
#define RIN_NPU_MODEL_MAX_BACKENDS UINT32_C(8)
#define RIN_NPU_MODEL_MAX_RANK UINT32_C(8)
#define RIN_NPU_MODEL_MAX_TENSOR_ELEMENTS UINT64_C(16777216)
#define RIN_NPU_MODEL_MAX_COMPILER_SCRATCH_BYTES UINT32_C(16777216)
#define RIN_NPU_MODEL_BACKEND_ABI_VERSION UINT32_C(0x00010000)

namespace rin {
namespace npu {
namespace model {

enum DataType {
    DATA_TYPE_FLOAT32 = 1
};

enum OperationCode {
    OPERATION_ADD_FLOAT32 = 1,
    OPERATION_MULTIPLY_FLOAT32 = 2,
    OPERATION_RELU_FLOAT32 = 3,
    OPERATION_MATMUL_FLOAT32 = 4,
    OPERATION_SOFTMAX_FLOAT32 = 5
};

enum BackendKind {
    BACKEND_CPU = 1,
    BACKEND_GPU = 2,
    BACKEND_NPU = 3
};

struct TensorDescription {
    uint32_t data_type;
    uint32_t rank;
    uint32_t dimensions[RIN_NPU_MODEL_MAX_RANK];
};

struct Operation {
    uint32_t opcode;
    uint32_t input_count;
    uint32_t inputs[2];
    uint32_t output_tensor;
    uint32_t reserved;
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
};

struct TensorView {
    float* data;
    uint64_t element_count;
};

struct CpuPlan {
    uint32_t struct_size;
    uint32_t version;
    const Model* model;
    uint64_t model_hash;
};

/* Backend callbacks are synchronous. execute() may read graph inputs but must
 * leave those inputs unchanged, and must return only after the device can no
 * longer access any supplied tensor. recover() must quiesce an uncertain
 * submission before CPU fallback is allowed. compile() may use scratch only
 * during the call; persistent plan state belongs to the provider. */
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

static inline uint64_t tensor_elements(const TensorDescription& tensor)
{
    if (tensor.data_type != DATA_TYPE_FLOAT32 ||
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
    if (left.data_type != right.data_type || left.rank != right.rank)
        return false;
    for (uint32_t dimension = 0u; dimension < left.rank; ++dimension)
        if (left.dimensions[dimension] != right.dimensions[dimension])
            return false;
    return true;
}

static inline bool operation_shape_valid(const Model* model,
                                         const Operation& operation)
{
    const TensorDescription& output = model->tensors[operation.output_tensor];
    if (operation.opcode == OPERATION_ADD_FLOAT32 ||
        operation.opcode == OPERATION_MULTIPLY_FLOAT32) {
        return operation.input_count == 2u &&
            same_shape(output, model->tensors[operation.inputs[0]]) &&
            same_shape(output, model->tensors[operation.inputs[1]]);
    }
    if (operation.opcode == OPERATION_RELU_FLOAT32) {
        return operation.input_count == 1u &&
            same_shape(output, model->tensors[operation.inputs[0]]);
    }
    if (operation.opcode == OPERATION_SOFTMAX_FLOAT32) {
        return operation.input_count == 1u && output.rank != 0u &&
            same_shape(output, model->tensors[operation.inputs[0]]);
    }
    if (operation.opcode == OPERATION_MATMUL_FLOAT32) {
        if (operation.input_count != 2u || output.rank != 2u) return false;
        const TensorDescription& left =
            model->tensors[operation.inputs[0]];
        const TensorDescription& right =
            model->tensors[operation.inputs[1]];
        return left.data_type == DATA_TYPE_FLOAT32 && left.rank == 2u &&
            right.data_type == DATA_TYPE_FLOAT32 && right.rank == 2u &&
            left.dimensions[1] == right.dimensions[0] &&
            output.dimensions[0] == left.dimensions[0] &&
            output.dimensions[1] == right.dimensions[1];
    }
    return false;
}

static inline bool model_valid(const Model* model)
{
    if (!model || model->struct_size != sizeof(*model) ||
        model->version != RIN_NPU_MODEL_ABI_VERSION ||
        model->tensor_count == 0u ||
        model->tensor_count > RIN_NPU_MODEL_MAX_TENSORS ||
        model->input_count == 0u || model->input_count > model->tensor_count ||
        model->output_count == 0u ||
        model->output_count > model->tensor_count ||
        model->operation_count > RIN_NPU_MODEL_MAX_OPERATIONS ||
        !model->tensors || !model->inputs || !model->outputs ||
        (model->operation_count != 0u && !model->operations))
        return false;

    uint8_t available[RIN_NPU_MODEL_MAX_TENSORS] = {};
    for (uint32_t index = 0u; index < model->tensor_count; ++index)
        if (tensor_elements(model->tensors[index]) == 0u) return false;
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
            (operation.opcode == OPERATION_ADD_FLOAT32 &&
             operation.input_count != 2u) ||
            (operation.opcode == OPERATION_MULTIPLY_FLOAT32 &&
             operation.input_count != 2u) ||
            (operation.opcode == OPERATION_MATMUL_FLOAT32 &&
             operation.input_count != 2u) ||
            (operation.opcode == OPERATION_RELU_FLOAT32 &&
             operation.input_count != 1u) ||
            (operation.opcode == OPERATION_SOFTMAX_FLOAT32 &&
             operation.input_count != 1u) ||
            operation.input_count == 0u || operation.input_count > 2u)
            return false;
        for (uint32_t input = 0u; input < operation.input_count; ++input)
            if (operation.inputs[input] >= model->tensor_count ||
                available[operation.inputs[input]] == 0u)
                return false;
        if (operation.input_count == 1u && operation.inputs[1] != 0u)
            return false;
        if (!operation_shape_valid(model, operation)) return false;
        available[operation.output_tensor] = 1u;
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

static inline uint64_t hash_byte(uint64_t hash, uint8_t byte)
{
    return (hash ^ byte) * UINT64_C(1099511628211);
}

static inline uint64_t hash_u32(uint64_t hash, uint32_t value)
{
    for (uint32_t byte = 0u; byte < 4u; ++byte)
        hash = hash_byte(hash, (uint8_t)(value >> (byte * 8u)));
    return hash;
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
        hash = hash_u32(hash, tensor.data_type);
        hash = hash_u32(hash, tensor.rank);
        for (uint32_t dimension = 0u;
             dimension < RIN_NPU_MODEL_MAX_RANK; ++dimension)
            hash = hash_u32(hash, tensor.dimensions[dimension]);
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
    return hash;
}

static inline void write_u32_le(uint8_t* output, uint32_t value)
{
    output[0] = (uint8_t)value;
    output[1] = (uint8_t)(value >> 8u);
    output[2] = (uint8_t)(value >> 16u);
    output[3] = (uint8_t)(value >> 24u);
}

static inline uint32_t read_u32_le(const uint8_t* input)
{
    return (uint32_t)input[0] | ((uint32_t)input[1] << 8u) |
        ((uint32_t)input[2] << 16u) | ((uint32_t)input[3] << 24u);
}

static inline bool memory_ranges_overlap(const void* left, uint64_t left_bytes,
                                        const void* right,
                                        uint64_t right_bytes)
{
    if (!left || !right || left_bytes == 0u || right_bytes == 0u)
        return false;
    const uintptr_t left_start = (uintptr_t)left;
    const uintptr_t right_start = (uintptr_t)right;
    if (left_bytes > UINTPTR_MAX - left_start ||
        right_bytes > UINTPTR_MAX - right_start)
        return true;
    const uintptr_t left_end = left_start + (uintptr_t)left_bytes;
    const uintptr_t right_end = right_start + (uintptr_t)right_bytes;
    return left_start < right_end && right_start < left_end;
}

static inline uint64_t serialized_size(const Model* model)
{
    return UINT64_C(32) + (uint64_t)model->tensor_count * 40u +
        (uint64_t)model->input_count * 4u +
        (uint64_t)model->output_count * 4u +
        (uint64_t)model->operation_count * 24u;
}

static inline RinResultCode serialize(const Model* model, void* blob,
                                      uint32_t capacity,
                                      uint32_t* bytes_out)
{
    if (!bytes_out) return RIN_RESULT_INVALID_ARGUMENT;
    if (!model_valid(model)) return RIN_RESULT_INVALID_ARGUMENT;
    if (memory_ranges_overlap(bytes_out, sizeof(*bytes_out), model,
                              sizeof(*model)) ||
        memory_ranges_overlap(bytes_out, sizeof(*bytes_out), model->tensors,
            (uint64_t)model->tensor_count * sizeof(*model->tensors)) ||
        memory_ranges_overlap(bytes_out, sizeof(*bytes_out), model->inputs,
            (uint64_t)model->input_count * sizeof(*model->inputs)) ||
        memory_ranges_overlap(bytes_out, sizeof(*bytes_out), model->outputs,
            (uint64_t)model->output_count * sizeof(*model->outputs)) ||
        memory_ranges_overlap(bytes_out, sizeof(*bytes_out), model->operations,
            (uint64_t)model->operation_count * sizeof(*model->operations)))
        return RIN_RESULT_INVALID_ARGUMENT;
    *bytes_out = 0u;
    const uint64_t required = serialized_size(model);
    if (required > UINT32_MAX) return RIN_RESULT_LIMIT_EXCEEDED;
    if (blob && memory_ranges_overlap(bytes_out, sizeof(*bytes_out), blob,
                                      required))
        return RIN_RESULT_INVALID_ARGUMENT;
    *bytes_out = (uint32_t)required;
    if (!blob || capacity < required) return RIN_RESULT_BUFFER_TOO_SMALL;
    if (memory_ranges_overlap(blob, required, model, sizeof(*model)) ||
        memory_ranges_overlap(blob, required, model->tensors,
            (uint64_t)model->tensor_count * sizeof(*model->tensors)) ||
        memory_ranges_overlap(blob, required, model->inputs,
            (uint64_t)model->input_count * sizeof(*model->inputs)) ||
        memory_ranges_overlap(blob, required, model->outputs,
            (uint64_t)model->output_count * sizeof(*model->outputs)) ||
        memory_ranges_overlap(blob, required, model->operations,
            (uint64_t)model->operation_count * sizeof(*model->operations)))
        return RIN_RESULT_INVALID_ARGUMENT;
    uint8_t* output = static_cast<uint8_t*>(blob);
    write_u32_le(output, RIN_NPU_MODEL_BLOB_MAGIC);
    write_u32_le(output + 4u, RIN_NPU_MODEL_ABI_VERSION);
    write_u32_le(output + 8u, 32u);
    write_u32_le(output + 12u, (uint32_t)required);
    write_u32_le(output + 16u, model->tensor_count);
    write_u32_le(output + 20u, model->input_count);
    write_u32_le(output + 24u, model->output_count);
    write_u32_le(output + 28u, model->operation_count);
    uint32_t offset = 32u;
    for (uint32_t index = 0u; index < model->tensor_count; ++index) {
        const TensorDescription& tensor = model->tensors[index];
        write_u32_le(output + offset, tensor.data_type);
        write_u32_le(output + offset + 4u, tensor.rank);
        for (uint32_t dimension = 0u;
             dimension < RIN_NPU_MODEL_MAX_RANK; ++dimension)
            write_u32_le(output + offset + 8u + dimension * 4u,
                         tensor.dimensions[dimension]);
        offset += 40u;
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
    return RIN_RESULT_OK;
}

static inline RinResultCode deserialize(const void* blob, uint32_t blob_bytes,
    TensorDescription* tensor_storage, uint32_t tensor_capacity,
    uint32_t* input_storage, uint32_t input_capacity,
    uint32_t* output_storage, uint32_t output_capacity,
    Operation* operation_storage, uint32_t operation_capacity,
    Model* model_out)
{
    if (!model_out) return RIN_RESULT_INVALID_ARGUMENT;
    if (!blob || blob_bytes < 32u ||
        memory_ranges_overlap(model_out, sizeof(*model_out), blob, blob_bytes))
        return RIN_RESULT_INVALID_ARGUMENT;
    const uint8_t* input = static_cast<const uint8_t*>(blob);
    const uint32_t tensor_count = read_u32_le(input + 16u);
    const uint32_t input_count = read_u32_le(input + 20u);
    const uint32_t output_count = read_u32_le(input + 24u);
    const uint32_t operation_count = read_u32_le(input + 28u);
    if (read_u32_le(input) != RIN_NPU_MODEL_BLOB_MAGIC ||
        read_u32_le(input + 4u) != RIN_NPU_MODEL_ABI_VERSION ||
        read_u32_le(input + 8u) != 32u ||
        read_u32_le(input + 12u) != blob_bytes || tensor_count == 0u ||
        tensor_count > RIN_NPU_MODEL_MAX_TENSORS || input_count == 0u ||
        input_count > tensor_count || output_count == 0u ||
        output_count > tensor_count ||
        operation_count > RIN_NPU_MODEL_MAX_OPERATIONS)
        return RIN_RESULT_CORRUPT_DATA;
    const uint64_t expected = UINT64_C(32) +
        (uint64_t)tensor_count * 40u + (uint64_t)input_count * 4u +
        (uint64_t)output_count * 4u +
        (uint64_t)operation_count * 24u;
    if (expected != blob_bytes) return RIN_RESULT_CORRUPT_DATA;
    if (tensor_capacity < tensor_count || input_capacity < input_count ||
        output_capacity < output_count ||
        operation_capacity < operation_count || !tensor_storage ||
        !input_storage || !output_storage ||
        (operation_count != 0u && !operation_storage))
        return RIN_RESULT_BUFFER_TOO_SMALL;
    const uint64_t tensor_bytes =
        (uint64_t)tensor_count * sizeof(*tensor_storage);
    const uint64_t input_bytes =
        (uint64_t)input_count * sizeof(*input_storage);
    const uint64_t output_bytes =
        (uint64_t)output_count * sizeof(*output_storage);
    const uint64_t operation_bytes =
        (uint64_t)operation_count * sizeof(*operation_storage);
    if (memory_ranges_overlap(blob, blob_bytes, tensor_storage, tensor_bytes) ||
        memory_ranges_overlap(blob, blob_bytes, input_storage, input_bytes) ||
        memory_ranges_overlap(blob, blob_bytes, output_storage, output_bytes) ||
        memory_ranges_overlap(blob, blob_bytes, operation_storage,
                              operation_bytes) ||
        memory_ranges_overlap(tensor_storage, tensor_bytes, input_storage,
                              input_bytes) ||
        memory_ranges_overlap(tensor_storage, tensor_bytes, output_storage,
                              output_bytes) ||
        memory_ranges_overlap(tensor_storage, tensor_bytes, operation_storage,
                              operation_bytes) ||
        memory_ranges_overlap(input_storage, input_bytes, output_storage,
                              output_bytes) ||
        memory_ranges_overlap(input_storage, input_bytes, operation_storage,
                              operation_bytes) ||
        memory_ranges_overlap(output_storage, output_bytes, operation_storage,
                              operation_bytes) ||
        memory_ranges_overlap(model_out, sizeof(*model_out), tensor_storage,
                              tensor_bytes) ||
        memory_ranges_overlap(model_out, sizeof(*model_out), input_storage,
                              input_bytes) ||
        memory_ranges_overlap(model_out, sizeof(*model_out), output_storage,
                              output_bytes) ||
        memory_ranges_overlap(model_out, sizeof(*model_out), operation_storage,
                              operation_bytes))
        return RIN_RESULT_INVALID_ARGUMENT;

    *model_out = {};
    uint32_t offset = 32u;
    for (uint32_t index = 0u; index < tensor_count; ++index) {
        tensor_storage[index].data_type = read_u32_le(input + offset);
        tensor_storage[index].rank = read_u32_le(input + offset + 4u);
        for (uint32_t dimension = 0u;
             dimension < RIN_NPU_MODEL_MAX_RANK; ++dimension)
            tensor_storage[index].dimensions[dimension] =
                read_u32_le(input + offset + 8u + dimension * 4u);
        offset += 40u;
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
    Model candidate = {};
    candidate.struct_size = sizeof(candidate);
    candidate.version = RIN_NPU_MODEL_ABI_VERSION;
    candidate.tensor_count = tensor_count;
    candidate.input_count = input_count;
    candidate.output_count = output_count;
    candidate.operation_count = operation_count;
    candidate.tensors = tensor_storage;
    candidate.inputs = input_storage;
    candidate.outputs = output_storage;
    candidate.operations = operation_storage;
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
            elements > SIZE_MAX / sizeof(float))
            return RIN_RESULT_INVALID_ARGUMENT;
        const uintptr_t start = (uintptr_t)tensors[index].data;
        const uint64_t bytes = elements * sizeof(float);
        if ((start & (sizeof(float) - 1u)) != 0u ||
            bytes > UINTPTR_MAX - start)
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

static inline RinResultCode compile_cpu(const Model* model, CpuPlan* plan_out)
{
    if (!plan_out) return RIN_RESULT_INVALID_ARGUMENT;
    *plan_out = {};
    if (!model_valid(model)) return RIN_RESULT_INVALID_ARGUMENT;
    plan_out->struct_size = sizeof(*plan_out);
    plan_out->version = RIN_NPU_MODEL_ABI_VERSION;
    plan_out->model = model;
    plan_out->model_hash = model_hash(model);
    return RIN_RESULT_OK;
}

static inline RinResultCode evaluate_cpu(const CpuPlan* plan,
    TensorView* tensors, uint32_t tensor_count)
{
    if (!plan || plan->struct_size != sizeof(*plan) ||
        plan->version != RIN_NPU_MODEL_ABI_VERSION ||
        !model_valid(plan->model) ||
        model_hash(plan->model) != plan->model_hash)
        return RIN_RESULT_CORRUPT_DATA;
    const RinResultCode view_status =
        validate_views(plan->model, tensors, tensor_count);
    if (view_status != RIN_RESULT_OK) return view_status;
    const Model* model = plan->model;
    for (uint32_t index = 0u; index < model->operation_count; ++index) {
        const Operation& operation = model->operations[index];
        const uint32_t output_index = operation.output_tensor;
        float* output = tensors[output_index].data;
        if (operation.opcode == OPERATION_ADD_FLOAT32 ||
            operation.opcode == OPERATION_MULTIPLY_FLOAT32) {
            const float* left = tensors[operation.inputs[0]].data;
            const float* right = tensors[operation.inputs[1]].data;
            const uint64_t elements = tensors[output_index].element_count;
            for (uint64_t element = 0u; element < elements; ++element)
                output[element] = operation.opcode == OPERATION_ADD_FLOAT32
                    ? left[element] + right[element]
                    : left[element] * right[element];
        } else if (operation.opcode == OPERATION_RELU_FLOAT32) {
            const float* input = tensors[operation.inputs[0]].data;
            const uint64_t elements = tensors[output_index].element_count;
            for (uint64_t element = 0u; element < elements; ++element)
                output[element] = input[element] > 0.0f ? input[element] : 0.0f;
        } else if (operation.opcode == OPERATION_MATMUL_FLOAT32) {
            const TensorDescription& left_desc =
                model->tensors[operation.inputs[0]];
            const TensorDescription& right_desc =
                model->tensors[operation.inputs[1]];
            const float* left = tensors[operation.inputs[0]].data;
            const float* right = tensors[operation.inputs[1]].data;
            const uint32_t rows = left_desc.dimensions[0];
            const uint32_t inner = left_desc.dimensions[1];
            const uint32_t columns = right_desc.dimensions[1];
            for (uint32_t row = 0u; row < rows; ++row)
                for (uint32_t column = 0u; column < columns; ++column) {
                    float sum = 0.0f;
                    for (uint32_t element = 0u; element < inner; ++element)
                        sum += left[(uint64_t)row * inner + element] *
                            right[(uint64_t)element * columns + column];
                    output[(uint64_t)row * columns + column] = sum;
                }
        } else if (operation.opcode == OPERATION_SOFTMAX_FLOAT32) {
            const float* input = tensors[operation.inputs[0]].data;
            const TensorDescription& description =
                model->tensors[operation.inputs[0]];
            const uint32_t axis = description.dimensions[description.rank - 1u];
            const uint64_t rows = tensors[output_index].element_count / axis;
            for (uint64_t row = 0u; row < rows; ++row) {
                const uint64_t base = row * axis;
                float maximum = input[base];
                for (uint32_t element = 1u; element < axis; ++element)
                    if (input[base + element] > maximum)
                        maximum = input[base + element];
                if (maximum == INFINITY) {
                    uint32_t positive_infinities = 0u;
                    for (uint32_t element = 0u; element < axis; ++element)
                        if (input[base + element] == INFINITY)
                            ++positive_infinities;
                    const float probability =
                        1.0f / (float)positive_infinities;
                    for (uint32_t element = 0u; element < axis; ++element)
                        output[base + element] =
                            input[base + element] == INFINITY
                                ? probability : 0.0f;
                } else if (maximum == -INFINITY) {
                    const float probability = 1.0f / (float)axis;
                    for (uint32_t element = 0u; element < axis; ++element)
                        output[base + element] = probability;
                } else {
                    float sum = 0.0f;
                    for (uint32_t element = 0u; element < axis; ++element) {
                        const float value = expf(input[base + element] - maximum);
                        output[base + element] = value;
                        sum += value;
                    }
                    for (uint32_t element = 0u; element < axis; ++element)
                        output[base + element] /= sum;
                }
            }
        }
    }
    return RIN_RESULT_OK;
}

static inline bool backend_provider_valid(const BackendProvider* provider)
{
    return provider && provider->struct_size == sizeof(*provider) &&
        provider->version == RIN_NPU_MODEL_BACKEND_ABI_VERSION &&
        (provider->kind == BACKEND_GPU || provider->kind == BACKEND_NPU) &&
        provider->reserved == 0u && provider->supports && provider->compile &&
        provider->execute && provider->recover && provider->release;
}

static inline bool backend_error_fallback_eligible(RinResultCode status)
{
    return status == RIN_RESULT_DEVICE_LOST || status == RIN_RESULT_TIMED_OUT ||
        status == RIN_RESULT_IO || status == RIN_RESULT_NOT_SUPPORTED ||
        status == RIN_RESULT_BUSY || status == RIN_RESULT_WOULD_BLOCK ||
        status == RIN_RESULT_NO_MEMORY;
}

static inline RinResultCode compile(const Model* model,
    const BackendProvider* const* providers, uint32_t provider_count,
    void* scratch, uint32_t scratch_bytes, bool allow_cpu_fallback,
    Executable* executable_out)
{
    if (!executable_out || provider_count > RIN_NPU_MODEL_MAX_BACKENDS ||
        scratch_bytes > RIN_NPU_MODEL_MAX_COMPILER_SCRATCH_BYTES ||
        (provider_count != 0u && !providers) ||
        (scratch_bytes != 0u && !scratch))
        return RIN_RESULT_INVALID_ARGUMENT;
    *executable_out = {};
    if (!model_valid(model)) return RIN_RESULT_INVALID_ARGUMENT;
    const uint64_t hash = model_hash(model);
    for (uint32_t index = 0u; index < provider_count; ++index) {
        const BackendProvider* provider = providers[index];
        if (!backend_provider_valid(provider)) return RIN_RESULT_CORRUPT_DATA;
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
            executable_out->version = RIN_NPU_MODEL_ABI_VERSION;
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
    executable_out->version = RIN_NPU_MODEL_ABI_VERSION;
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
        executable->version != RIN_NPU_MODEL_ABI_VERSION ||
        !model_valid(executable->model) ||
        model_hash(executable->model) != executable->model_hash)
        return RIN_RESULT_CORRUPT_DATA;
    const RinResultCode view_status =
        validate_views(executable->model, tensors, tensor_count);
    if (view_status != RIN_RESULT_OK) return view_status;
    if (executable->backend_kind == BACKEND_CPU) {
        CpuPlan plan = {};
        plan.struct_size = sizeof(plan);
        plan.version = RIN_NPU_MODEL_ABI_VERSION;
        plan.model = executable->model;
        plan.model_hash = executable->model_hash;
        return evaluate_cpu(&plan, tensors, tensor_count);
    }
    if (!backend_provider_valid(executable->provider) ||
        executable->provider->kind != executable->backend_kind ||
        executable->provider_plan_handle == 0u)
        return RIN_RESULT_CORRUPT_DATA;
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
    plan.version = RIN_NPU_MODEL_ABI_VERSION;
    plan.model = executable->model;
    plan.model_hash = executable->model_hash;
    return evaluate_cpu(&plan, tensors, tensor_count);
}

static inline RinResultCode release(Executable* executable)
{
    if (!executable || executable->struct_size != sizeof(*executable) ||
        executable->version != RIN_NPU_MODEL_ABI_VERSION)
        return RIN_RESULT_INVALID_ARGUMENT;
    if (executable->provider) {
        if (!backend_provider_valid(executable->provider) ||
            executable->provider_plan_handle == 0u)
            return RIN_RESULT_CORRUPT_DATA;
        executable->provider->release(executable->provider->context,
                                      executable->provider_plan_handle);
    }
    *executable = {};
    return RIN_RESULT_OK;
}

} /* namespace model */
} /* namespace npu */
} /* namespace rin */

#endif /* RIN_DRIVER_NPU_MODEL_HPP */
