/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_NPU_MODEL_ONNX_HPP
#define RIN_DRIVER_NPU_MODEL_ONNX_HPP

#include <float.h>
#include <stddef.h>
#include <stdint.h>

#include <rin/driver/npu_model.hpp>

#define RIN_NPU_ONNX_MAX_MODEL_BYTES UINT32_C(67108864)
#define RIN_NPU_ONNX_MAX_VALUES RIN_NPU_MODEL_MAX_TENSORS
#define RIN_NPU_ONNX_MAX_TOTAL_WEIGHT_ELEMENTS UINT64_C(16777216)
#define RIN_NPU_ONNX_MAX_NAME_BYTES UINT32_C(127)

namespace rin {
namespace npu {
namespace model {
namespace onnx {

struct ConstantBinding {
    uint32_t tensor_index;
    uint32_t reserved;
    uint64_t element_offset;
    uint64_t element_count;
};

/* The importer accepts concrete float32 tensors and the listed portable ops;
 * dynamic/external/sparse weights and unsupported operators are rejected.
 * All storage belongs to the caller and must remain alive as long as the
 * returned model or constant bindings are used. Import does not allocate. */
struct Workspace {
    TensorDescription* tensors;
    uint32_t tensor_capacity;
    uint32_t* graph_inputs;
    uint32_t graph_input_capacity;
    uint32_t* external_inputs;
    uint32_t external_input_capacity;
    uint32_t* graph_outputs;
    uint32_t graph_output_capacity;
    Operation* operations;
    uint32_t operation_capacity;
    struct ValueBinding* values;
    uint32_t value_capacity;
    ConstantBinding* constants;
    uint32_t constant_capacity;
    float* weight_storage;
    uint64_t weight_capacity_elements;
};

struct Result {
    Model graph;
    uint32_t external_input_count;
    const uint32_t* external_input_indices;
    uint32_t constant_count;
    const ConstantBinding* constants;
    float* constant_values;
    uint64_t constant_value_count;
};

struct ValueBinding {
    uint32_t tensor_index;
    uint32_t name_bytes;
    uint32_t flags;
    uint32_t reserved;
    char name[RIN_NPU_ONNX_MAX_NAME_BYTES + 1u];
};

enum ValueFlags {
    VALUE_HAS_DESCRIPTION = 1u << 0,
    VALUE_EXTERNAL_INPUT = 1u << 1,
    VALUE_INITIALIZER = 1u << 2,
    VALUE_AVAILABLE = 1u << 3,
    VALUE_PRODUCED = 1u << 4
};

struct Slice {
    const uint8_t* bytes;
    uint32_t size;
};

struct Reader {
    const uint8_t* bytes;
    uint32_t size;
    uint32_t offset;
};

struct Field {
    uint32_t number;
    uint32_t wire_type;
    uint64_t integer;
    Slice bytes;
};

struct Name {
    const uint8_t* bytes;
    uint32_t size;
};

struct GraphFields {
    Slice nodes[RIN_NPU_MODEL_MAX_OPERATIONS];
    Slice initializers[RIN_NPU_MODEL_MAX_TENSORS];
    Slice inputs[RIN_NPU_MODEL_MAX_TENSORS];
    Slice outputs[RIN_NPU_MODEL_MAX_TENSORS];
    Slice value_info[RIN_NPU_MODEL_MAX_TENSORS];
    uint32_t node_count;
    uint32_t initializer_count;
    uint32_t input_count;
    uint32_t output_count;
    uint32_t value_info_count;
};

struct TensorHeader {
    TensorDescription description;
    Name name;
};

struct Node {
    Name inputs[2];
    Name output;
    uint32_t input_count;
    uint32_t opcode;
    int64_t softmax_axis;
    uint32_t has_softmax_axis;
};

static inline bool read_varint(Reader* reader, uint64_t* value_out)
{
    if (!reader || !value_out) return false;
    uint64_t value = 0u;
    for (uint32_t index = 0u; index < 10u; ++index) {
        if (reader->offset >= reader->size) return false;
        const uint8_t byte = reader->bytes[reader->offset++];
        if (index == 9u && (byte & 0xfeu) != 0u) return false;
        value |= (uint64_t)(byte & 0x7fu) << (index * 7u);
        if ((byte & 0x80u) == 0u) {
            *value_out = value;
            return true;
        }
    }
    return false;
}

/* Returns 1 for a field, 0 at end, and -1 for malformed protobuf. */
static inline int next_field(Reader* reader, Field* field_out)
{
    if (!reader || !field_out) return -1;
    if (reader->offset == reader->size) return 0;
    uint64_t key = 0u;
    if (!read_varint(reader, &key) || key == 0u ||
        (key >> 3u) > UINT32_MAX)
        return -1;
    Field field = {};
    field.number = (uint32_t)(key >> 3u);
    field.wire_type = (uint32_t)(key & 7u);
    if (field.number == 0u) return -1;
    if (field.wire_type == 0u) {
        if (!read_varint(reader, &field.integer)) return -1;
    } else if (field.wire_type == 1u || field.wire_type == 5u) {
        const uint32_t fixed_bytes = field.wire_type == 1u ? 8u : 4u;
        if (fixed_bytes > reader->size - reader->offset) return -1;
        field.bytes.bytes = reader->bytes + reader->offset;
        field.bytes.size = fixed_bytes;
        reader->offset += fixed_bytes;
    } else if (field.wire_type == 2u) {
        uint64_t length = 0u;
        if (!read_varint(reader, &length) ||
            length > reader->size - reader->offset || length > UINT32_MAX)
            return -1;
        field.bytes.bytes = reader->bytes + reader->offset;
        field.bytes.size = (uint32_t)length;
        reader->offset += (uint32_t)length;
    } else {
        return -1;
    }
    *field_out = field;
    return 1;
}

static inline Reader make_reader(Slice bytes)
{
    Reader reader = {bytes.bytes, bytes.size, 0u};
    return reader;
}

static inline bool field_name(const Field& field, Name* name_out)
{
    if (!name_out || field.wire_type != 2u || field.bytes.size == 0u ||
        field.bytes.size > RIN_NPU_ONNX_MAX_NAME_BYTES)
        return false;
    name_out->bytes = field.bytes.bytes;
    name_out->size = field.bytes.size;
    return true;
}

static inline bool names_equal(Name left, Name right)
{
    if (left.size != right.size) return false;
    for (uint32_t index = 0u; index < left.size; ++index)
        if (left.bytes[index] != right.bytes[index]) return false;
    return true;
}

static inline bool name_is(Name name, const char* text, uint32_t text_size)
{
    if (!text || name.size != text_size) return false;
    for (uint32_t index = 0u; index < text_size; ++index)
        if (name.bytes[index] != (uint8_t)text[index]) return false;
    return true;
}

static inline RinResultCode parse_shape(Slice shape_bytes,
                                        TensorDescription* description_out)
{
    if (!description_out) return RIN_RESULT_INVALID_ARGUMENT;
    Reader reader = make_reader(shape_bytes);
    TensorDescription description = {};
    description.data_type = DATA_TYPE_FLOAT32;
    uint32_t rank = 0u;
    Field field;
    int next = 0;
    while ((next = next_field(&reader, &field)) > 0) {
        if (field.number != 1u) continue;
        if (field.wire_type != 2u) return RIN_RESULT_CORRUPT_DATA;
        Reader dimension_reader = make_reader(field.bytes);
        uint64_t dimension_value = 0u;
        uint32_t has_value = 0u;
        uint32_t has_symbol = 0u;
        Field dimension_field;
        int dimension_next = 0;
        while ((dimension_next = next_field(
                    &dimension_reader, &dimension_field)) > 0) {
            if (dimension_field.number == 1u) {
                if (dimension_field.wire_type != 0u || has_value != 0u)
                    return RIN_RESULT_CORRUPT_DATA;
                dimension_value = dimension_field.integer;
                has_value = 1u;
            } else if (dimension_field.number == 2u) {
                if (dimension_field.wire_type != 2u || has_symbol != 0u)
                    return RIN_RESULT_CORRUPT_DATA;
                has_symbol = 1u;
            }
        }
        if (dimension_next < 0) return RIN_RESULT_CORRUPT_DATA;
        if (has_symbol != 0u || has_value == 0u)
            return RIN_RESULT_NOT_SUPPORTED;
        if (rank >= RIN_NPU_MODEL_MAX_RANK || dimension_value == 0u ||
            dimension_value > UINT32_MAX)
            return RIN_RESULT_NOT_SUPPORTED;
        description.dimensions[rank++] = (uint32_t)dimension_value;
    }
    if (next < 0) return RIN_RESULT_CORRUPT_DATA;
    description.rank = rank;
    if (tensor_elements(description) == 0u)
        return RIN_RESULT_NOT_SUPPORTED;
    *description_out = description;
    return RIN_RESULT_OK;
}

static inline RinResultCode parse_type(Slice type_bytes,
                                       TensorDescription* description_out)
{
    Reader reader = make_reader(type_bytes);
    Slice tensor_type = {};
    uint32_t has_tensor_type = 0u;
    Field field;
    int next = 0;
    while ((next = next_field(&reader, &field)) > 0) {
        if (field.number == 1u) {
            if (field.wire_type != 2u || has_tensor_type != 0u)
                return RIN_RESULT_CORRUPT_DATA;
            tensor_type = field.bytes;
            has_tensor_type = 1u;
        } else if (field.number == 4u || field.number == 5u ||
                   field.number == 7u || field.number == 8u ||
                   field.number == 9u) {
            return RIN_RESULT_NOT_SUPPORTED;
        }
    }
    if (next < 0) return RIN_RESULT_CORRUPT_DATA;
    if (has_tensor_type == 0u) return RIN_RESULT_NOT_SUPPORTED;
    Reader tensor_reader = make_reader(tensor_type);
    uint32_t data_type = 0u;
    uint32_t has_data_type = 0u;
    uint32_t has_shape = 0u;
    Slice shape = {};
    Field tensor_field;
    int tensor_next = 0;
    while ((tensor_next = next_field(&tensor_reader, &tensor_field)) > 0) {
        if (tensor_field.number == 1u) {
            if (tensor_field.wire_type != 0u || has_data_type != 0u)
                return RIN_RESULT_CORRUPT_DATA;
            data_type = (uint32_t)tensor_field.integer;
            has_data_type = 1u;
        } else if (tensor_field.number == 2u) {
            if (tensor_field.wire_type != 2u || has_shape != 0u)
                return RIN_RESULT_CORRUPT_DATA;
            shape = tensor_field.bytes;
            has_shape = 1u;
        }
    }
    if (tensor_next < 0) return RIN_RESULT_CORRUPT_DATA;
    if (has_data_type == 0u || data_type != 1u || has_shape == 0u)
        return RIN_RESULT_NOT_SUPPORTED;
    return parse_shape(shape, description_out);
}

static inline RinResultCode parse_value_info(Slice bytes, Name* name_out,
                                              TensorDescription* desc_out)
{
    if (!name_out || !desc_out) return RIN_RESULT_INVALID_ARGUMENT;
    Reader reader = make_reader(bytes);
    Name name = {};
    Slice type = {};
    uint32_t has_name = 0u;
    uint32_t has_type = 0u;
    Field field;
    int next = 0;
    while ((next = next_field(&reader, &field)) > 0) {
        if (field.number == 1u) {
            if (has_name != 0u || !field_name(field, &name))
                return RIN_RESULT_CORRUPT_DATA;
            has_name = 1u;
        } else if (field.number == 2u) {
            if (field.wire_type != 2u || has_type != 0u)
                return RIN_RESULT_CORRUPT_DATA;
            type = field.bytes;
            has_type = 1u;
        }
    }
    if (next < 0) return RIN_RESULT_CORRUPT_DATA;
    if (has_name == 0u || has_type == 0u) return RIN_RESULT_CORRUPT_DATA;
    RinResultCode status = parse_type(type, desc_out);
    if (status != RIN_RESULT_OK) return status;
    *name_out = name;
    return RIN_RESULT_OK;
}

static inline RinResultCode parse_tensor_header(Slice bytes,
                                                 TensorHeader* header_out)
{
    if (!header_out) return RIN_RESULT_INVALID_ARGUMENT;
    Reader reader = make_reader(bytes);
    TensorHeader header = {};
    header.description.data_type = DATA_TYPE_FLOAT32;
    uint32_t has_type = 0u;
    uint32_t has_name = 0u;
    uint32_t rank = 0u;
    Field field;
    int next = 0;
    while ((next = next_field(&reader, &field)) > 0) {
        if (field.number == 1u) {
            if (field.wire_type == 0u) {
                if (rank >= RIN_NPU_MODEL_MAX_RANK || field.integer == 0u ||
                    field.integer > UINT32_MAX)
                    return RIN_RESULT_NOT_SUPPORTED;
                header.description.dimensions[rank++] =
                    (uint32_t)field.integer;
            } else if (field.wire_type == 2u) {
                Reader packed = make_reader(field.bytes);
                uint64_t dimension = 0u;
                while (packed.offset < packed.size) {
                    if (!read_varint(&packed, &dimension))
                        return RIN_RESULT_CORRUPT_DATA;
                    if (rank >= RIN_NPU_MODEL_MAX_RANK || dimension == 0u ||
                        dimension > UINT32_MAX)
                        return RIN_RESULT_NOT_SUPPORTED;
                    header.description.dimensions[rank++] =
                        (uint32_t)dimension;
                }
            } else {
                return RIN_RESULT_CORRUPT_DATA;
            }
        } else if (field.number == 2u) {
            if (field.wire_type != 0u || has_type != 0u)
                return RIN_RESULT_CORRUPT_DATA;
            has_type = 1u;
            if (field.integer != 1u) return RIN_RESULT_NOT_SUPPORTED;
        } else if (field.number == 3u || field.number == 13u) {
            return RIN_RESULT_NOT_SUPPORTED;
        } else if (field.number == 8u) {
            if (has_name != 0u || !field_name(field, &header.name))
                return RIN_RESULT_CORRUPT_DATA;
            has_name = 1u;
        } else if (field.number == 14u) {
            if (field.wire_type != 0u) return RIN_RESULT_CORRUPT_DATA;
            if (field.integer != 0u) return RIN_RESULT_NOT_SUPPORTED;
        } else if (field.number == 5u || field.number == 6u ||
                   field.number == 7u || field.number == 10u ||
                   field.number == 11u) {
            return RIN_RESULT_NOT_SUPPORTED;
        }
    }
    if (next < 0) return RIN_RESULT_CORRUPT_DATA;
    if (has_type == 0u || has_name == 0u) return RIN_RESULT_CORRUPT_DATA;
    header.description.rank = rank;
    if (tensor_elements(header.description) == 0u)
        return RIN_RESULT_NOT_SUPPORTED;
    *header_out = header;
    return RIN_RESULT_OK;
}

static inline float decode_float32_le(const uint8_t* bytes)
{
    union FloatBits {
        uint32_t bits;
        float value;
    } decoded;
    decoded.bits = read_u32_le(bytes);
    return decoded.value;
}

static inline RinResultCode parse_tensor_values(Slice bytes,
    float* destination, uint64_t element_count)
{
    if ((element_count != 0u && !destination) || sizeof(float) != 4u ||
        FLT_RADIX != 2 || FLT_MANT_DIG != 24)
        return RIN_RESULT_NOT_SUPPORTED;
    Reader reader = make_reader(bytes);
    uint32_t data_mode = 0u;
    uint32_t raw_seen = 0u;
    uint64_t values_written = 0u;
    Field field;
    int next = 0;
    while ((next = next_field(&reader, &field)) > 0) {
        if (field.number == 9u) {
            if (field.wire_type != 2u || raw_seen != 0u || data_mode != 0u)
                return RIN_RESULT_CORRUPT_DATA;
            if (field.bytes.size != element_count * sizeof(float))
                return RIN_RESULT_CORRUPT_DATA;
            data_mode = 1u;
            raw_seen = 1u;
            for (uint64_t index = 0u; index < element_count; ++index)
                destination[index] = decode_float32_le(
                    field.bytes.bytes + index * sizeof(float));
            values_written = element_count;
        } else if (field.number == 4u) {
            if (data_mode == 1u) return RIN_RESULT_CORRUPT_DATA;
            data_mode = 2u;
            if (field.wire_type == 2u) {
                if ((field.bytes.size & 3u) != 0u)
                    return RIN_RESULT_CORRUPT_DATA;
                for (uint32_t offset = 0u; offset < field.bytes.size;
                     offset += 4u) {
                    if (values_written >= element_count)
                        return RIN_RESULT_CORRUPT_DATA;
                    destination[values_written++] =
                        decode_float32_le(field.bytes.bytes + offset);
                }
            } else if (field.wire_type == 5u) {
                if (values_written >= element_count)
                    return RIN_RESULT_CORRUPT_DATA;
                destination[values_written++] =
                    decode_float32_le(field.bytes.bytes);
            } else {
                return RIN_RESULT_CORRUPT_DATA;
            }
        }
    }
    if (next < 0) return RIN_RESULT_CORRUPT_DATA;
    return data_mode != 0u && values_written == element_count
        ? RIN_RESULT_OK : RIN_RESULT_CORRUPT_DATA;
}

static inline bool slice_is(Slice slice, const char* text, uint32_t length)
{
    if (!slice.bytes || slice.size != length) return false;
    for (uint32_t index = 0u; index < length; ++index)
        if (slice.bytes[index] != (uint8_t)text[index]) return false;
    return true;
}

static inline RinResultCode parse_node(Slice bytes, uint32_t opset_version,
                                       Node* node_out)
{
    if (!node_out) return RIN_RESULT_INVALID_ARGUMENT;
    Reader reader = make_reader(bytes);
    Name inputs[2] = {};
    Name output = {};
    Slice op_type = {};
    Slice domain = {};
    Slice attributes[4] = {};
    uint32_t input_count = 0u;
    uint32_t output_count = 0u;
    uint32_t has_op_type = 0u;
    uint32_t has_domain = 0u;
    uint32_t attribute_count = 0u;
    Field field;
    int next = 0;
    while ((next = next_field(&reader, &field)) > 0) {
        if (field.number == 1u) {
            if (input_count >= 2u || !field_name(
                    field, &inputs[input_count]))
                return RIN_RESULT_NOT_SUPPORTED;
            ++input_count;
        } else if (field.number == 2u) {
            if (output_count != 0u || !field_name(field, &output))
                return RIN_RESULT_CORRUPT_DATA;
            ++output_count;
        } else if (field.number == 4u) {
            if (field.wire_type != 2u || has_op_type != 0u)
                return RIN_RESULT_CORRUPT_DATA;
            op_type = field.bytes;
            has_op_type = 1u;
        } else if (field.number == 5u) {
            if (field.wire_type != 2u || attribute_count >= 4u)
                return RIN_RESULT_NOT_SUPPORTED;
            attributes[attribute_count++] = field.bytes;
        } else if (field.number == 7u) {
            if (field.wire_type != 2u || has_domain != 0u)
                return RIN_RESULT_CORRUPT_DATA;
            domain = field.bytes;
            has_domain = 1u;
        }
    }
    if (next < 0) return RIN_RESULT_CORRUPT_DATA;
    if (has_op_type == 0u || output_count != 1u)
        return RIN_RESULT_CORRUPT_DATA;
    if (has_domain != 0u && domain.size != 0u &&
        !slice_is(domain, "ai.onnx", 7u))
        return RIN_RESULT_NOT_SUPPORTED;

    Node node = {};
    node.input_count = input_count;
    node.output = output;
    node.softmax_axis = opset_version >= 13u ? -1 : 1;
    if (slice_is(op_type, "Add", 3u))
        node.opcode = OPERATION_ADD_FLOAT32;
    else if (slice_is(op_type, "Mul", 3u))
        node.opcode = OPERATION_MULTIPLY_FLOAT32;
    else if (slice_is(op_type, "Relu", 4u))
        node.opcode = OPERATION_RELU_FLOAT32;
    else if (slice_is(op_type, "MatMul", 6u)) {
        if (opset_version < 9u) return RIN_RESULT_NOT_SUPPORTED;
        node.opcode = OPERATION_MATMUL_FLOAT32;
    } else if (slice_is(op_type, "Softmax", 7u)) {
        node.opcode = OPERATION_SOFTMAX_FLOAT32;
    } else {
        return RIN_RESULT_NOT_SUPPORTED;
    }
    const uint32_t required_inputs =
        node.opcode == OPERATION_ADD_FLOAT32 ||
        node.opcode == OPERATION_MULTIPLY_FLOAT32 ||
        node.opcode == OPERATION_MATMUL_FLOAT32 ? 2u : 1u;
    if (input_count != required_inputs) return RIN_RESULT_NOT_SUPPORTED;
    for (uint32_t index = 0u; index < input_count; ++index)
        node.inputs[index] = inputs[index];

    if (node.opcode != OPERATION_SOFTMAX_FLOAT32 && attribute_count != 0u)
        return RIN_RESULT_NOT_SUPPORTED;
    if (attribute_count != 0u) {
        if (attribute_count != 1u) return RIN_RESULT_NOT_SUPPORTED;
        Reader attribute_reader = make_reader(attributes[0]);
        Name attribute_name = {};
        uint64_t axis_value = 0u;
        uint32_t has_axis = 0u;
        uint32_t has_attribute_name = 0u;
        uint32_t attribute_type = 0u;
        uint32_t has_attribute_type = 0u;
        Field attribute_field;
        int attribute_next = 0;
        while ((attribute_next = next_field(
                    &attribute_reader, &attribute_field)) > 0) {
            if (attribute_field.number == 1u) {
                if (has_attribute_name != 0u ||
                    !field_name(attribute_field, &attribute_name))
                    return RIN_RESULT_CORRUPT_DATA;
                has_attribute_name = 1u;
            } else if (attribute_field.number == 3u) {
                if (attribute_field.wire_type != 0u || has_axis != 0u)
                    return RIN_RESULT_CORRUPT_DATA;
                axis_value = attribute_field.integer;
                has_axis = 1u;
            } else if (attribute_field.number == 20u) {
                if (attribute_field.wire_type != 0u ||
                    has_attribute_type != 0u ||
                    attribute_field.integer > UINT32_MAX)
                    return RIN_RESULT_CORRUPT_DATA;
                attribute_type = (uint32_t)attribute_field.integer;
                has_attribute_type = 1u;
            } else if (attribute_field.number == 2u ||
                       attribute_field.number == 4u ||
                       attribute_field.number == 5u ||
                       attribute_field.number == 6u ||
                       attribute_field.number == 7u ||
                       attribute_field.number == 8u ||
                       attribute_field.number == 9u ||
                       attribute_field.number == 10u ||
                       attribute_field.number == 11u ||
                       attribute_field.number == 12u ||
                       attribute_field.number == 13u ||
                       attribute_field.number == 14u ||
                       attribute_field.number == 15u ||
                       attribute_field.number == 16u ||
                       attribute_field.number == 17u ||
                       attribute_field.number == 18u ||
                       attribute_field.number == 19u ||
                       attribute_field.number == 21u ||
                       attribute_field.number == 22u ||
                       attribute_field.number == 23u) {
                return RIN_RESULT_NOT_SUPPORTED;
            }
        }
        if (attribute_next < 0) return RIN_RESULT_CORRUPT_DATA;
        if (!name_is(attribute_name, "axis", 4u) || has_axis == 0u ||
            (has_attribute_type != 0u && attribute_type != 2u))
            return RIN_RESULT_NOT_SUPPORTED;
        node.softmax_axis = axis_value <= INT64_MAX
            ? (int64_t)axis_value
            : -1 - (int64_t)(~axis_value);
        node.has_softmax_axis = 1u;
    }
    *node_out = node;
    return RIN_RESULT_OK;
}

static inline int find_value(const Workspace* workspace, uint32_t value_count,
                             Name name)
{
    for (uint32_t index = 0u; index < value_count; ++index) {
        const ValueBinding& binding = workspace->values[index];
        if (binding.name_bytes == name.size) {
            uint32_t equal = 1u;
            for (uint32_t byte = 0u; byte < name.size; ++byte)
                if ((uint8_t)binding.name[byte] != name.bytes[byte]) {
                    equal = 0u;
                    break;
                }
            if (equal != 0u) return (int)index;
        }
    }
    return -1;
}

static inline RinResultCode add_value(Workspace* workspace,
    uint32_t* value_count_io, Name name,
    const TensorDescription* description, uint32_t flags,
    uint32_t* index_out)
{
    if (!workspace || !value_count_io || !index_out || name.size == 0u ||
        name.size > RIN_NPU_ONNX_MAX_NAME_BYTES)
        return RIN_RESULT_INVALID_ARGUMENT;
    int index = find_value(workspace, *value_count_io, name);
    if (index >= 0) {
        ValueBinding& binding = workspace->values[index];
        if ((binding.flags & flags &
             (VALUE_EXTERNAL_INPUT | VALUE_INITIALIZER)) != 0u)
            return RIN_RESULT_CORRUPT_DATA;
        if (description) {
            if ((binding.flags & VALUE_HAS_DESCRIPTION) != 0u) {
                if (!same_shape(workspace->tensors[index], *description))
                    return RIN_RESULT_CORRUPT_DATA;
            } else {
                workspace->tensors[index] = *description;
                binding.flags |= VALUE_HAS_DESCRIPTION;
            }
        }
        binding.flags |= flags;
        *index_out = (uint32_t)index;
        return RIN_RESULT_OK;
    }
    if (*value_count_io >= workspace->value_capacity ||
        *value_count_io >= RIN_NPU_ONNX_MAX_VALUES ||
        *value_count_io >= workspace->tensor_capacity)
        return RIN_RESULT_BUFFER_TOO_SMALL;
    const uint32_t new_index = *value_count_io;
    ValueBinding binding = {};
    binding.tensor_index = new_index;
    binding.name_bytes = name.size;
    binding.flags = flags;
    for (uint32_t byte = 0u; byte < name.size; ++byte)
        binding.name[byte] = (char)name.bytes[byte];
    workspace->values[new_index] = binding;
    workspace->tensors[new_index] = description ? *description
                                                : TensorDescription{};
    ++*value_count_io;
    *index_out = new_index;
    return RIN_RESULT_OK;
}

static inline RinResultCode parse_graph(Slice bytes, GraphFields* graph_out)
{
    if (!graph_out) return RIN_RESULT_INVALID_ARGUMENT;
    GraphFields graph = {};
    Reader reader = make_reader(bytes);
    Field field;
    int next = 0;
    while ((next = next_field(&reader, &field)) > 0) {
        Slice* list = 0;
        uint32_t* count = 0;
        uint32_t capacity = 0u;
        if (field.number == 1u) {
            list = graph.nodes;
            count = &graph.node_count;
            capacity = RIN_NPU_MODEL_MAX_OPERATIONS;
        } else if (field.number == 5u) {
            list = graph.initializers;
            count = &graph.initializer_count;
            capacity = RIN_NPU_MODEL_MAX_TENSORS;
        } else if (field.number == 11u) {
            list = graph.inputs;
            count = &graph.input_count;
            capacity = RIN_NPU_MODEL_MAX_TENSORS;
        } else if (field.number == 12u) {
            list = graph.outputs;
            count = &graph.output_count;
            capacity = RIN_NPU_MODEL_MAX_TENSORS;
        } else if (field.number == 13u) {
            list = graph.value_info;
            count = &graph.value_info_count;
            capacity = RIN_NPU_MODEL_MAX_TENSORS;
        } else if (field.number == 15u) {
            return RIN_RESULT_NOT_SUPPORTED;
        }
        if (!list) continue;
        if (field.wire_type != 2u) return RIN_RESULT_CORRUPT_DATA;
        if (*count >= capacity) return RIN_RESULT_LIMIT_EXCEEDED;
        list[(*count)++] = field.bytes;
    }
    if (next < 0) return RIN_RESULT_CORRUPT_DATA;
    if (graph.output_count == 0u) return RIN_RESULT_CORRUPT_DATA;
    *graph_out = graph;
    return RIN_RESULT_OK;
}

static inline RinResultCode parse_opset(Slice bytes, uint32_t* version_out,
                                        uint32_t* standard_out)
{
    if (!version_out || !standard_out) return RIN_RESULT_INVALID_ARGUMENT;
    Reader reader = make_reader(bytes);
    Name domain = {};
    uint32_t has_domain = 0u;
    uint32_t has_version = 0u;
    uint64_t version = 0u;
    Field field;
    int next = 0;
    while ((next = next_field(&reader, &field)) > 0) {
        if (field.number == 1u) {
            if (field.wire_type != 2u || has_domain != 0u)
                return RIN_RESULT_CORRUPT_DATA;
            has_domain = 1u;
            domain.bytes = field.bytes.bytes;
            domain.size = field.bytes.size;
        } else if (field.number == 2u) {
            if (field.wire_type != 0u || has_version != 0u)
                return RIN_RESULT_CORRUPT_DATA;
            has_version = 1u;
            version = field.integer;
        }
    }
    if (next < 0) return RIN_RESULT_CORRUPT_DATA;
    if (has_version == 0u || version == 0u || version > UINT32_MAX)
        return RIN_RESULT_CORRUPT_DATA;
    const uint32_t standard = has_domain == 0u || domain.size == 0u ||
        name_is(domain, "ai.onnx", 7u);
    if (standard != 0u) {
        *version_out = (uint32_t)version;
        *standard_out = 1u;
    } else {
        *standard_out = 0u;
    }
    return RIN_RESULT_OK;
}

static inline RinResultCode parse_model(Slice bytes, Slice* graph_out,
                                        uint32_t* opset_out)
{
    if (!graph_out || !opset_out) return RIN_RESULT_INVALID_ARGUMENT;
    *graph_out = {};
    *opset_out = 0u;
    uint32_t has_graph = 0u;
    uint32_t standard_opset_count = 0u;
    Reader reader = make_reader(bytes);
    Field field;
    int next = 0;
    while ((next = next_field(&reader, &field)) > 0) {
        if (field.number == 7u) {
            if (field.wire_type != 2u || has_graph != 0u)
                return RIN_RESULT_CORRUPT_DATA;
            *graph_out = field.bytes;
            has_graph = 1u;
        } else if (field.number == 8u) {
            if (field.wire_type != 2u) return RIN_RESULT_CORRUPT_DATA;
            uint32_t version = 0u;
            uint32_t standard = 0u;
            RinResultCode status = parse_opset(
                field.bytes, &version, &standard);
            if (status != RIN_RESULT_OK) return status;
            if (standard != 0u) {
                if (standard_opset_count != 0u)
                    return RIN_RESULT_CORRUPT_DATA;
                *opset_out = version;
                ++standard_opset_count;
            }
        }
    }
    if (next < 0) return RIN_RESULT_CORRUPT_DATA;
    if (has_graph == 0u || standard_opset_count != 1u)
        return RIN_RESULT_CORRUPT_DATA;
    return RIN_RESULT_OK;
}

static inline RinResultCode node_output_description(
    const Node& node, const Workspace* workspace, const uint32_t* indices,
    uint32_t opset_version, TensorDescription* description_out)
{
    const TensorDescription& left = workspace->tensors[indices[0]];
    if (node.opcode == OPERATION_ADD_FLOAT32 ||
        node.opcode == OPERATION_MULTIPLY_FLOAT32) {
        const TensorDescription& right = workspace->tensors[indices[1]];
        if (opset_version >= 7u) {
            if (!broadcast_shape(left, right, description_out))
                return RIN_RESULT_NOT_SUPPORTED;
        } else {
            if (!same_shape(left, right)) return RIN_RESULT_NOT_SUPPORTED;
            *description_out = left;
        }
        return RIN_RESULT_OK;
    }
    if (node.opcode == OPERATION_RELU_FLOAT32) {
        *description_out = left;
        return RIN_RESULT_OK;
    }
    if (node.opcode == OPERATION_MATMUL_FLOAT32) {
        const TensorDescription& right = workspace->tensors[indices[1]];
        if (left.rank != 2u || right.rank != 2u ||
            left.dimensions[1] != right.dimensions[0])
            return RIN_RESULT_NOT_SUPPORTED;
        TensorDescription output = {};
        output.data_type = DATA_TYPE_FLOAT32;
        output.rank = 2u;
        output.dimensions[0] = left.dimensions[0];
        output.dimensions[1] = right.dimensions[1];
        if (tensor_elements(output) == 0u) return RIN_RESULT_NOT_SUPPORTED;
        *description_out = output;
        return RIN_RESULT_OK;
    }
    if (node.opcode == OPERATION_SOFTMAX_FLOAT32) {
        if (left.rank == 0u) return RIN_RESULT_NOT_SUPPORTED;
        int64_t axis = node.softmax_axis;
        if (axis < 0) axis += left.rank;
        if (axis < 0 || axis >= (int64_t)left.rank ||
            (uint32_t)axis != left.rank - 1u)
            return RIN_RESULT_NOT_SUPPORTED;
        (void)opset_version;
        *description_out = left;
        return RIN_RESULT_OK;
    }
    return RIN_RESULT_NOT_SUPPORTED;
}

static inline RinResultCode validate_workspace(
    const Workspace* workspace, uint32_t model_bytes,
    const void* model_blob, const Result* result_out)
{
    if (!workspace || !model_blob || !result_out ||
        workspace->tensor_capacity > RIN_NPU_MODEL_MAX_TENSORS ||
        workspace->value_capacity > RIN_NPU_ONNX_MAX_VALUES ||
        workspace->graph_input_capacity > RIN_NPU_MODEL_MAX_TENSORS ||
        workspace->external_input_capacity > RIN_NPU_MODEL_MAX_TENSORS ||
        workspace->graph_output_capacity > RIN_NPU_MODEL_MAX_TENSORS ||
        workspace->operation_capacity > RIN_NPU_MODEL_MAX_OPERATIONS ||
        workspace->constant_capacity > RIN_NPU_MODEL_MAX_TENSORS ||
        workspace->weight_capacity_elements >
            RIN_NPU_ONNX_MAX_TOTAL_WEIGHT_ELEMENTS)
        return RIN_RESULT_INVALID_ARGUMENT;
    if ((workspace->tensor_capacity != 0u && !workspace->tensors) ||
        (workspace->value_capacity != 0u && !workspace->values) ||
        (workspace->graph_input_capacity != 0u && !workspace->graph_inputs) ||
        (workspace->external_input_capacity != 0u &&
         !workspace->external_inputs) ||
        (workspace->graph_output_capacity != 0u &&
         !workspace->graph_outputs) ||
        (workspace->operation_capacity != 0u && !workspace->operations) ||
        (workspace->constant_capacity != 0u && !workspace->constants) ||
        (workspace->weight_capacity_elements != 0u &&
         !workspace->weight_storage))
        return RIN_RESULT_INVALID_ARGUMENT;

    const uint64_t sizes[8] = {
        (uint64_t)workspace->tensor_capacity * sizeof(*workspace->tensors),
        (uint64_t)workspace->graph_input_capacity *
            sizeof(*workspace->graph_inputs),
        (uint64_t)workspace->external_input_capacity *
            sizeof(*workspace->external_inputs),
        (uint64_t)workspace->graph_output_capacity *
            sizeof(*workspace->graph_outputs),
        (uint64_t)workspace->operation_capacity * sizeof(*workspace->operations),
        (uint64_t)workspace->value_capacity * sizeof(*workspace->values),
        (uint64_t)workspace->constant_capacity * sizeof(*workspace->constants),
        workspace->weight_capacity_elements * sizeof(float)
    };
    const void* addresses[8] = {
        workspace->tensors, workspace->graph_inputs,
        workspace->external_inputs, workspace->graph_outputs,
        workspace->operations, workspace->values,
        workspace->constants, workspace->weight_storage
    };
    for (uint32_t index = 0u; index < 8u; ++index) {
        if (memory_ranges_overlap(addresses[index], sizes[index], model_blob,
                                  model_bytes) ||
            memory_ranges_overlap(addresses[index], sizes[index], result_out,
                                  sizeof(*result_out)) ||
            memory_ranges_overlap(addresses[index], sizes[index], workspace,
                                  sizeof(*workspace)))
            return RIN_RESULT_INVALID_ARGUMENT;
        for (uint32_t other = index + 1u; other < 8u; ++other)
            if (memory_ranges_overlap(addresses[index], sizes[index],
                                      addresses[other], sizes[other]))
                return RIN_RESULT_INVALID_ARGUMENT;
    }
    if (memory_ranges_overlap(model_blob, model_bytes, result_out,
                              sizeof(*result_out)) ||
        memory_ranges_overlap(workspace, sizeof(*workspace), result_out,
                              sizeof(*result_out)))
        return RIN_RESULT_INVALID_ARGUMENT;
    return RIN_RESULT_OK;
}

static inline RinResultCode import_model(const void* model_blob,
    uint32_t model_bytes, Workspace* workspace, Result* result_out)
{
    if (!model_blob || model_bytes == 0u ||
        model_bytes > RIN_NPU_ONNX_MAX_MODEL_BYTES ||
        !workspace || !result_out)
        return RIN_RESULT_INVALID_ARGUMENT;
    RinResultCode status = validate_workspace(
        workspace, model_bytes, model_blob, result_out);
    if (status != RIN_RESULT_OK) return status;
    *result_out = {};

    Slice graph_slice = {};
    uint32_t opset_version = 0u;
    Slice model_slice = {static_cast<const uint8_t*>(model_blob), model_bytes};
    status = parse_model(model_slice, &graph_slice, &opset_version);
    if (status != RIN_RESULT_OK) return status;
    GraphFields graph = {};
    status = parse_graph(graph_slice, &graph);
    if (status != RIN_RESULT_OK) return status;
    if (graph.node_count > workspace->operation_capacity ||
        graph.output_count > workspace->graph_output_capacity ||
        graph.initializer_count > workspace->constant_capacity)
        return RIN_RESULT_BUFFER_TOO_SMALL;

    uint32_t value_count = 0u;
    uint32_t external_input_count = 0u;
    uint32_t constant_count = 0u;
    uint32_t graph_input_count = 0u;
    uint64_t weight_element_count = 0u;

    for (uint32_t index = 0u; index < graph.initializer_count; ++index) {
        TensorHeader header = {};
        status = parse_tensor_header(graph.initializers[index], &header);
        if (status != RIN_RESULT_OK) return status;
        uint32_t tensor_index = 0u;
        status = add_value(workspace, &value_count, header.name,
                           &header.description,
                           VALUE_INITIALIZER | VALUE_AVAILABLE,
                           &tensor_index);
        if (status != RIN_RESULT_OK) return status;
        const uint64_t elements = tensor_elements(header.description);
        if (elements == 0u ||
            elements > RIN_NPU_ONNX_MAX_TOTAL_WEIGHT_ELEMENTS -
                           weight_element_count ||
            elements > workspace->weight_capacity_elements -
                           weight_element_count)
            return RIN_RESULT_BUFFER_TOO_SMALL;
        status = parse_tensor_values(
            graph.initializers[index],
            workspace->weight_storage + weight_element_count, elements);
        if (status != RIN_RESULT_OK) return status;
        ConstantBinding binding = {};
        binding.tensor_index = tensor_index;
        binding.element_offset = weight_element_count;
        binding.element_count = elements;
        workspace->constants[constant_count++] = binding;
        weight_element_count += elements;
    }

    for (uint32_t index = 0u; index < graph.input_count; ++index) {
        Name name = {};
        TensorDescription description = {};
        status = parse_value_info(
            graph.inputs[index], &name, &description);
        if (status != RIN_RESULT_OK) return status;
        int existing = find_value(workspace, value_count, name);
        if (existing >= 0 &&
            (workspace->values[existing].flags & VALUE_INITIALIZER) != 0u) {
            if (!same_shape(workspace->tensors[existing], description))
                return RIN_RESULT_CORRUPT_DATA;
            continue;
        }
        uint32_t tensor_index = 0u;
        status = add_value(workspace, &value_count, name, &description,
                           VALUE_EXTERNAL_INPUT | VALUE_AVAILABLE,
                           &tensor_index);
        if (status != RIN_RESULT_OK) return status;
        if (external_input_count >= workspace->external_input_capacity)
            return RIN_RESULT_BUFFER_TOO_SMALL;
        workspace->external_inputs[external_input_count++] = tensor_index;
    }

    for (uint32_t index = 0u; index < graph.value_info_count; ++index) {
        Name name = {};
        TensorDescription description = {};
        status = parse_value_info(
            graph.value_info[index], &name, &description);
        if (status != RIN_RESULT_OK) return status;
        uint32_t tensor_index = 0u;
        status = add_value(workspace, &value_count, name, &description,
                           VALUE_HAS_DESCRIPTION, &tensor_index);
        if (status != RIN_RESULT_OK) return status;
    }
    for (uint32_t index = 0u; index < graph.output_count; ++index) {
        Name name = {};
        TensorDescription description = {};
        status = parse_value_info(
            graph.outputs[index], &name, &description);
        if (status != RIN_RESULT_OK) return status;
        uint32_t tensor_index = 0u;
        status = add_value(workspace, &value_count, name, &description,
                           VALUE_HAS_DESCRIPTION, &tensor_index);
        if (status != RIN_RESULT_OK) return status;
    }

    for (uint32_t index = 0u; index < graph.node_count; ++index) {
        Node node = {};
        status = parse_node(graph.nodes[index], opset_version, &node);
        if (status != RIN_RESULT_OK) return status;
        uint32_t input_indices[2] = {0u, 0u};
        for (uint32_t input = 0u; input < node.input_count; ++input) {
            const int value_index = find_value(
                workspace, value_count, node.inputs[input]);
            if (value_index < 0 ||
                (workspace->values[value_index].flags &
                 VALUE_AVAILABLE) == 0u)
                return RIN_RESULT_CORRUPT_DATA;
            input_indices[input] = (uint32_t)value_index;
        }
        TensorDescription output_description = {};
        status = node_output_description(
            node, workspace, input_indices, opset_version,
            &output_description);
        if (status != RIN_RESULT_OK) return status;
        uint32_t output_index = 0u;
        const int existing = find_value(workspace, value_count, node.output);
        if (existing >= 0 &&
            (workspace->values[existing].flags & VALUE_AVAILABLE) != 0u)
            return RIN_RESULT_CORRUPT_DATA;
        status = add_value(workspace, &value_count, node.output,
                           &output_description,
                           VALUE_PRODUCED | VALUE_AVAILABLE,
                           &output_index);
        if (status != RIN_RESULT_OK) return status;
        if (index >= workspace->operation_capacity)
            return RIN_RESULT_BUFFER_TOO_SMALL;
        Operation operation = {};
        operation.opcode = node.opcode;
        operation.input_count = node.input_count;
        operation.inputs[0] = input_indices[0];
        operation.inputs[1] = input_indices[1];
        operation.output_tensor = output_index;
        workspace->operations[index] = operation;
    }

    for (uint32_t index = 0u; index < external_input_count; ++index) {
        if (graph_input_count >= workspace->graph_input_capacity)
            return RIN_RESULT_BUFFER_TOO_SMALL;
        workspace->graph_inputs[graph_input_count++] =
            workspace->external_inputs[index];
    }
    for (uint32_t index = 0u; index < constant_count; ++index) {
        if (graph_input_count >= workspace->graph_input_capacity)
            return RIN_RESULT_BUFFER_TOO_SMALL;
        workspace->graph_inputs[graph_input_count++] =
            workspace->constants[index].tensor_index;
    }
    if (graph_input_count == 0u) return RIN_RESULT_NOT_SUPPORTED;

    for (uint32_t index = 0u; index < graph.output_count; ++index) {
        Name name = {};
        TensorDescription description = {};
        status = parse_value_info(
            graph.outputs[index], &name, &description);
        if (status != RIN_RESULT_OK) return status;
        const int value_index = find_value(workspace, value_count, name);
        if (value_index < 0 ||
            (workspace->values[value_index].flags & VALUE_AVAILABLE) == 0u ||
            !same_shape(workspace->tensors[value_index], description))
            return RIN_RESULT_CORRUPT_DATA;
        workspace->graph_outputs[index] = (uint32_t)value_index;
    }

    Model candidate = {};
    candidate.struct_size = sizeof(candidate);
    candidate.version = RIN_NPU_MODEL_ABI_VERSION;
    candidate.tensor_count = value_count;
    candidate.input_count = graph_input_count;
    candidate.output_count = graph.output_count;
    candidate.operation_count = graph.node_count;
    candidate.tensors = workspace->tensors;
    candidate.inputs = workspace->graph_inputs;
    candidate.outputs = workspace->graph_outputs;
    candidate.operations = graph.node_count != 0u
        ? workspace->operations : 0;
    if (!model_valid(&candidate)) return RIN_RESULT_CORRUPT_DATA;

    Result result = {};
    result.graph = candidate;
    result.external_input_count = external_input_count;
    result.external_input_indices = workspace->external_inputs;
    result.constant_count = constant_count;
    result.constants = workspace->constants;
    result.constant_values = workspace->weight_storage;
    result.constant_value_count = weight_element_count;
    *result_out = result;
    return RIN_RESULT_OK;
}

/* Bind imported initializer storage into a caller's TensorView array. This
 * validates the complete binding set before changing any view. */
static inline RinResultCode bind_constants(const Result* imported,
    TensorView* tensors, uint32_t tensor_count)
{
    if (!imported || !tensors || tensor_count == 0u ||
        tensor_count != imported->graph.tensor_count ||
        tensor_count > RIN_NPU_MODEL_MAX_TENSORS ||
        imported->constant_count > RIN_NPU_MODEL_MAX_TENSORS ||
        imported->constant_value_count >
            RIN_NPU_ONNX_MAX_TOTAL_WEIGHT_ELEMENTS ||
        (imported->constant_count != 0u &&
         (!imported->constants || !imported->constant_values)) ||
        !model_valid(&imported->graph))
        return RIN_RESULT_INVALID_ARGUMENT;
    const uint64_t view_bytes = (uint64_t)tensor_count * sizeof(*tensors);
    if (memory_ranges_overlap(tensors, view_bytes, imported,
                              sizeof(*imported)) ||
        memory_ranges_overlap(tensors, view_bytes, imported->graph.tensors,
            (uint64_t)imported->graph.tensor_count *
                sizeof(*imported->graph.tensors)) ||
        memory_ranges_overlap(tensors, view_bytes, imported->graph.inputs,
            (uint64_t)imported->graph.input_count *
                sizeof(*imported->graph.inputs)) ||
        memory_ranges_overlap(tensors, view_bytes, imported->graph.outputs,
            (uint64_t)imported->graph.output_count *
                sizeof(*imported->graph.outputs)) ||
        memory_ranges_overlap(tensors, view_bytes, imported->graph.operations,
            (uint64_t)imported->graph.operation_count *
                sizeof(*imported->graph.operations)) ||
        memory_ranges_overlap(tensors, view_bytes, imported->constants,
            (uint64_t)imported->constant_count *
                sizeof(*imported->constants)) ||
        memory_ranges_overlap(tensors, view_bytes,
            imported->external_input_indices,
            (uint64_t)imported->external_input_count *
                sizeof(*imported->external_input_indices)) ||
        memory_ranges_overlap(tensors, view_bytes, imported->constant_values,
            imported->constant_value_count * sizeof(float)))
        return RIN_RESULT_INVALID_ARGUMENT;

    for (uint32_t index = 0u; index < imported->constant_count; ++index) {
        const ConstantBinding& binding = imported->constants[index];
        if (binding.reserved != 0u ||
            binding.tensor_index >= tensor_count ||
            binding.element_count != tensor_elements(
                imported->graph.tensors[binding.tensor_index]) ||
            binding.element_offset > imported->constant_value_count ||
            binding.element_count > imported->constant_value_count -
                                        binding.element_offset)
            return RIN_RESULT_CORRUPT_DATA;
        for (uint32_t prior = 0u; prior < index; ++prior) {
            const ConstantBinding& previous = imported->constants[prior];
            if (previous.tensor_index == binding.tensor_index ||
                (binding.element_offset < previous.element_offset +
                     previous.element_count &&
                 previous.element_offset < binding.element_offset +
                     binding.element_count))
                return RIN_RESULT_CORRUPT_DATA;
        }
    }
    for (uint32_t index = 0u; index < imported->constant_count; ++index) {
        const ConstantBinding& binding = imported->constants[index];
        tensors[binding.tensor_index].data =
            imported->constant_values + binding.element_offset;
        tensors[binding.tensor_index].element_count = binding.element_count;
    }
    return RIN_RESULT_OK;
}

} /* namespace onnx */
} /* namespace model */
} /* namespace npu */
} /* namespace rin */

#endif /* RIN_DRIVER_NPU_MODEL_ONNX_HPP */
