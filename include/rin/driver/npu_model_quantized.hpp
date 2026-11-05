/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_NPU_MODEL_QUANTIZED_HPP
#define RIN_DRIVER_NPU_MODEL_QUANTIZED_HPP

#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <rin/driver/npu_model.hpp>

/* Versioned affine Q8 graph contract. The v1 layout stays fixed; later
 * versions add per-axis parameters, operation-specific sidecars, v5 runtime
 * outputs for ONNX DynamicQuantizeLinear, v6 blocked Q8 parameters, v7
 * integer matmul, v8 integer convolution parameters, v9 pooling geometry, and
 * v10 GlobalMaxPool. */
#define RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V1 UINT32_C(0x00010000)
#define RIN_NPU_QUANTIZED_MODEL_ABI_VERSION UINT32_C(0x00020000)
#define RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V3 UINT32_C(0x00030000)
#define RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V4 UINT32_C(0x00040000)
#define RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V5 UINT32_C(0x00050000)
#define RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V6 UINT32_C(0x00060000)
#define RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7 UINT32_C(0x00070000)
#define RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8 UINT32_C(0x00080000)
#define RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9 UINT32_C(0x00090000)
#define RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10 UINT32_C(0x000a0000)
#define RIN_NPU_QUANTIZED_MODEL_BLOB_MAGIC UINT32_C(0x514e4952)
#define RIN_NPU_QUANTIZED_MODEL_MAX_BLOB_BYTES UINT32_C(16384)
#define RIN_NPU_QUANTIZED_MODEL_MAX_BACKENDS RIN_NPU_MODEL_MAX_BACKENDS

namespace rin {
namespace npu {
namespace model {
namespace quantized {

enum DataType {
    DATA_TYPE_INT8 = 2,
    DATA_TYPE_UINT8 = 3,
    DATA_TYPE_FLOAT32 = 1,
    DATA_TYPE_INT32 = 4
};

enum OperationCode {
    OPERATION_ADD_Q8 = 0x101,
    OPERATION_MULTIPLY_Q8 = 0x102,
    OPERATION_RELU_Q8 = 0x103,
    OPERATION_MATMUL_Q8 = 0x104,
    OPERATION_SOFTMAX_Q8 = 0x105,
    OPERATION_CONV_Q8 = 0x106,
    OPERATION_QUANTIZE_Q8 = 0x107,
    OPERATION_DEQUANTIZE_Q8 = 0x108,
    OPERATION_DYNAMIC_QUANTIZE_Q8 = 0x109,
    OPERATION_MATMUL_INTEGER = 0x10a,
    OPERATION_CONV_INTEGER = 0x10b,
    OPERATION_AVERAGE_POOL_FLOAT32 = 0x10c,
    OPERATION_GLOBAL_MAX_POOL_FLOAT32 = 0x10d
};

#define RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS \
    (RIN_NPU_MODEL_MAX_RANK - 2u)

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
    /* For OPERATION_DYNAMIC_QUANTIZE_Q8 this is the output tensor index for
     * the float32 scale; ordinary one-input operations keep it zero. */
    uint32_t inputs[2];
    uint32_t output_tensor;
    /* Reserved and zero for legacy operations. In model v5/v6, DynamicQuantize
     * stores its uint8 zero-point output index here; Dynamic DequantizeLinear
     * stores the matching runtime zero-point tensor index here. */
    uint32_t reserved;
};

/* ONNX MatMulInteger uses signed/unsigned Q8 inputs and an int32 output. Its
 * optional zero-point tensors are represented by their tensor indexes here;
 * UINT32_MAX means the ONNX default zero point of zero. */
struct IntegerMatMulParameters {
    uint32_t operation_index;
    uint32_t a_zero_point_tensor;
    uint32_t b_zero_point_tensor;
    uint32_t reserved;
};

/* ONNX ConvInteger zero points. Convolution geometry is held in the existing
 * ConvolutionParameters sidecar for the same operation index. */
struct IntegerConvolutionParameters {
    uint32_t operation_index;
    uint32_t input_zero_point_tensor;
    uint32_t weight_zero_point_tensor;
    uint32_t reserved;
};

/* ONNX AveragePool geometry resolved to explicit fixed-rank values. */
struct AveragePoolParameters {
    uint32_t operation_index;
    uint32_t spatial_rank;
    uint32_t ceil_mode;
    uint32_t count_include_pad;
    uint32_t reserved;
    uint32_t kernel_shape[RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS];
    uint32_t strides[RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS];
    uint32_t dilations[RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS];
    uint32_t pads_begin[RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS];
    uint32_t pads_end[RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS];
};

/* Optional per-axis affine parameters. In model v6 and later, `reserved` is
 * the block_size for ONNX blocked quantization; it remains zero for ordinary
 * per-axis parameters and every model version before v6. */
struct AxisQuantization {
    uint32_t tensor_index;
    uint32_t axis;
    uint32_t count;
    uint32_t reserved;
    const float* scales;
    const int32_t* zero_points;
};

/* Broadcast-shaped QLinearMatMul parameters retain quantization values that
 * vary by both batch and row/column. Dimensions are right-aligned to the
 * operand and must broadcast to its fixed shape. */
struct BatchQuantization {
    uint32_t tensor_index;
    uint32_t rank;
    uint32_t count;
    uint32_t reserved;
    uint32_t dimensions[RIN_NPU_MODEL_MAX_RANK];
    const float* scales;
    const int32_t* zero_points;
};

/* Resolved fixed-shape convolution geometry. Padding is stored explicitly,
 * including the result of ONNX SAME_UPPER/SAME_LOWER resolution. */
struct ConvolutionParameters {
    uint32_t operation_index;
    uint32_t spatial_rank;
    uint32_t group;
    uint32_t bias_count;
    uint32_t reserved;
    uint32_t strides[RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS];
    uint32_t dilations[RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS];
    uint32_t pads_begin[RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS];
    uint32_t pads_end[RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS];
    const int32_t* bias;
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
    /* Appended in ABI v3. */
    uint32_t convolution_count;
    uint32_t reserved2;
    const ConvolutionParameters* convolutions;
    /* Appended in ABI v4. */
    uint32_t batch_quantization_count;
    uint32_t reserved3;
    const BatchQuantization* batch_quantizations;
    /* Appended in ABI v7. */
    uint32_t integer_matmul_count;
    uint32_t reserved4;
    const IntegerMatMulParameters* integer_matmuls;
    /* Appended in ABI v8. */
    uint32_t integer_convolution_count;
    uint32_t reserved5;
    const IntegerConvolutionParameters* integer_convolutions;
    /* Appended in ABI v9. */
    uint32_t average_pool_count;
    uint32_t reserved6;
    const AveragePoolParameters* average_pools;
};

#define RIN_NPU_QUANTIZED_MODEL_V1_STRUCT_SIZE \
    ((uint32_t)offsetof(::rin::npu::model::quantized::Model, quantization_count))
#define RIN_NPU_QUANTIZED_MODEL_V2_STRUCT_SIZE \
    ((uint32_t)offsetof(::rin::npu::model::quantized::Model, convolution_count))
#define RIN_NPU_QUANTIZED_MODEL_V3_STRUCT_SIZE \
    ((uint32_t)offsetof(::rin::npu::model::quantized::Model, batch_quantization_count))
#define RIN_NPU_QUANTIZED_MODEL_V6_STRUCT_SIZE \
    ((uint32_t)offsetof(::rin::npu::model::quantized::Model, integer_matmul_count))
#define RIN_NPU_QUANTIZED_MODEL_V7_STRUCT_SIZE \
    ((uint32_t)offsetof(::rin::npu::model::quantized::Model, integer_convolution_count))
#define RIN_NPU_QUANTIZED_MODEL_V8_STRUCT_SIZE \
    ((uint32_t)offsetof(::rin::npu::model::quantized::Model, average_pool_count))

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
    const bool float_tensor = tensor.data_type == DATA_TYPE_FLOAT32;
    const bool int32_tensor = tensor.data_type == DATA_TYPE_INT32;
    if ((!float_tensor && !int32_tensor &&
         (!data_type_bounds(tensor.data_type, &minimum, &maximum) ||
          tensor.zero_point < minimum || tensor.zero_point > maximum ||
          !(tensor.scale > 0.0f) || !(tensor.scale <= FLT_MAX))) ||
        (float_tensor && (tensor.zero_point != 0 || tensor.scale != 1.0f)) ||
        (int32_tensor && (tensor.zero_point != 0 || tensor.scale != 1.0f)) ||
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

static inline uint64_t tensor_storage_bytes(const TensorDescription& tensor)
{
    const uint64_t elements = tensor_elements(tensor);
    const uint64_t element_bytes =
        tensor.data_type == DATA_TYPE_FLOAT32 ||
                tensor.data_type == DATA_TYPE_INT32
            ? sizeof(uint32_t) : 1u;
    return elements && elements <= UINT64_MAX / element_bytes
        ? elements * element_bytes : 0u;
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

static inline bool matmul_output_shape(const TensorDescription& left,
                                       const TensorDescription& right,
                                       uint32_t* rank_out,
                                       uint32_t* dimensions_out)
{
    if (rank_out) *rank_out = 0u;
    if (dimensions_out)
        for (uint32_t i = 0u; i < RIN_NPU_MODEL_MAX_RANK; ++i)
            dimensions_out[i] = 0u;
    if (!rank_out || !dimensions_out || left.rank < 2u || right.rank < 2u ||
        left.rank > RIN_NPU_MODEL_MAX_RANK ||
        right.rank > RIN_NPU_MODEL_MAX_RANK ||
        left.dimensions[left.rank - 1u] !=
            right.dimensions[right.rank - 2u])
        return false;
    const uint32_t left_batch_rank = left.rank - 2u;
    const uint32_t right_batch_rank = right.rank - 2u;
    const uint32_t batch_rank = left_batch_rank > right_batch_rank
        ? left_batch_rank : right_batch_rank;
    if (batch_rank > RIN_NPU_MODEL_MAX_RANK - 2u) return false;
    for (uint32_t dimension = 0u; dimension < batch_rank; ++dimension) {
        const uint32_t left_leading = batch_rank - left_batch_rank;
        const uint32_t right_leading = batch_rank - right_batch_rank;
        const uint32_t left_extent = dimension < left_leading
            ? 1u : left.dimensions[dimension - left_leading];
        const uint32_t right_extent = dimension < right_leading
            ? 1u : right.dimensions[dimension - right_leading];
        if (left_extent != right_extent && left_extent != 1u &&
            right_extent != 1u)
            return false;
        dimensions_out[dimension] = left_extent > right_extent
            ? left_extent : right_extent;
    }
    dimensions_out[batch_rank] = left.dimensions[left.rank - 2u];
    dimensions_out[batch_rank + 1u] = right.dimensions[right.rank - 1u];
    *rank_out = batch_rank + 2u;
    return true;
}

static inline bool convolution_output_shape(const TensorDescription& input,
    const TensorDescription& weights, const ConvolutionParameters& parameters,
    uint32_t* rank_out, uint32_t* dimensions_out)
{
    if (rank_out) *rank_out = 0u;
    if (dimensions_out)
        for (uint32_t i = 0u; i < RIN_NPU_MODEL_MAX_RANK; ++i)
            dimensions_out[i] = 0u;
    if (!rank_out || !dimensions_out || input.rank < 3u ||
        input.rank > RIN_NPU_MODEL_MAX_RANK || weights.rank != input.rank ||
        parameters.spatial_rank != input.rank - 2u || !parameters.group ||
        parameters.reserved != 0u ||
        (parameters.bias_count != 0u && !parameters.bias))
        return false;
    const uint32_t output_channels = weights.dimensions[0];
    const uint32_t input_channels = input.dimensions[1];
    if (input_channels % parameters.group != 0u ||
        output_channels % parameters.group != 0u ||
        weights.dimensions[1] != input_channels / parameters.group ||
        (parameters.bias_count != 0u &&
         parameters.bias_count != output_channels))
        return false;

    TensorDescription output = {};
    output.rank = input.rank;
    output.dimensions[0] = input.dimensions[0];
    output.dimensions[1] = output_channels;
    for (uint32_t axis = 0u; axis < parameters.spatial_rank; ++axis) {
        const uint32_t stride = parameters.strides[axis];
        const uint32_t dilation = parameters.dilations[axis];
        const uint32_t kernel = weights.dimensions[axis + 2u];
        if (!stride || !dilation || !kernel) return false;
        const uint64_t effective_kernel =
            (uint64_t)(kernel - 1u) * dilation + 1u;
        const uint64_t padded_input =
            (uint64_t)input.dimensions[axis + 2u] +
            parameters.pads_begin[axis] + parameters.pads_end[axis];
        if (padded_input < effective_kernel) return false;
        const uint64_t extent =
            (padded_input - effective_kernel) / stride + 1u;
        if (!extent || extent > UINT32_MAX) return false;
        output.dimensions[axis + 2u] = (uint32_t)extent;
    }
    uint64_t output_elements = 1u;
    for (uint32_t dimension = 0u; dimension < output.rank; ++dimension) {
        if (!output.dimensions[dimension] ||
            output_elements > RIN_NPU_MODEL_MAX_TENSOR_ELEMENTS /
                                  output.dimensions[dimension])
            return false;
        output_elements *= output.dimensions[dimension];
    }
    *rank_out = output.rank;
    for (uint32_t i = 0u; i < output.rank; ++i)
        dimensions_out[i] = output.dimensions[i];
    return true;
}

static inline bool average_pool_output_shape(const TensorDescription& input,
    const AveragePoolParameters& parameters, uint32_t* rank_out,
    uint32_t* dimensions_out)
{
    if (rank_out) *rank_out = 0u;
    if (dimensions_out)
        for (uint32_t index = 0u; index < RIN_NPU_MODEL_MAX_RANK; ++index)
            dimensions_out[index] = 0u;
    if (!rank_out || !dimensions_out || input.rank < 3u ||
        input.rank > RIN_NPU_MODEL_MAX_RANK ||
        parameters.spatial_rank != input.rank - 2u ||
        parameters.ceil_mode > 1u || parameters.count_include_pad > 1u ||
        parameters.reserved != 0u)
        return false;

    TensorDescription output = {};
    output.rank = input.rank;
    output.dimensions[0] = input.dimensions[0];
    output.dimensions[1] = input.dimensions[1];
    uint64_t kernel_elements = 1u;
    for (uint32_t axis = 0u; axis < parameters.spatial_rank; ++axis) {
        const uint32_t kernel = parameters.kernel_shape[axis];
        const uint32_t stride = parameters.strides[axis];
        const uint32_t dilation = parameters.dilations[axis];
        if (!kernel || !stride || !dilation ||
            kernel_elements > RIN_NPU_MODEL_MAX_TENSOR_ELEMENTS / kernel)
            return false;
        kernel_elements *= kernel;
        const uint64_t effective_kernel =
            (uint64_t)(kernel - 1u) * dilation + 1u;
        const uint64_t padded_extent =
            (uint64_t)input.dimensions[axis + 2u] +
            parameters.pads_begin[axis] + parameters.pads_end[axis];
        if (effective_kernel > INT64_MAX || padded_extent > INT64_MAX)
            return false;
        const int64_t numerator = (int64_t)padded_extent -
            (int64_t)effective_kernel;
        int64_t quotient = numerator / (int64_t)stride;
        const int64_t remainder = numerator % (int64_t)stride;
        if (parameters.ceil_mode) {
            if (numerator > 0 && remainder != 0) ++quotient;
        } else if (numerator < 0 && remainder != 0) {
            --quotient;
        }
        int64_t signed_extent = quotient + 1;
        if (parameters.ceil_mode && signed_extent > 0 &&
            (uint64_t)(signed_extent - 1) * stride >=
                (uint64_t)input.dimensions[axis + 2u] +
                    parameters.pads_begin[axis])
            --signed_extent;
        if (signed_extent <= 0 || signed_extent > UINT32_MAX) return false;
        output.dimensions[axis + 2u] = (uint32_t)signed_extent;
    }
    for (uint32_t axis = parameters.spatial_rank;
         axis < RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS; ++axis)
        if (parameters.kernel_shape[axis] || parameters.strides[axis] ||
            parameters.dilations[axis] || parameters.pads_begin[axis] ||
            parameters.pads_end[axis])
            return false;
    uint64_t elements = 1u;
    for (uint32_t dimension = 0u; dimension < output.rank; ++dimension) {
        if (!output.dimensions[dimension] ||
            elements > RIN_NPU_MODEL_MAX_TENSOR_ELEMENTS /
                           output.dimensions[dimension])
            return false;
        elements *= output.dimensions[dimension];
    }
    *rank_out = output.rank;
    for (uint32_t index = 0u; index < output.rank; ++index)
        dimensions_out[index] = output.dimensions[index];
    return true;
}

static inline bool global_max_pool_output_shape(const TensorDescription& input,
    uint32_t* rank_out, uint32_t* dimensions_out)
{
    if (rank_out) *rank_out = 0u;
    if (dimensions_out)
        for (uint32_t index = 0u; index < RIN_NPU_MODEL_MAX_RANK; ++index)
            dimensions_out[index] = 0u;
    if (!rank_out || !dimensions_out || input.rank < 3u ||
        input.rank > RIN_NPU_MODEL_MAX_RANK)
        return false;
    uint64_t spatial_elements = 1u;
    for (uint32_t dimension = 2u; dimension < input.rank; ++dimension) {
        const uint32_t extent = input.dimensions[dimension];
        if (!extent || spatial_elements >
                           RIN_NPU_MODEL_MAX_TENSOR_ELEMENTS / extent)
            return false;
        spatial_elements *= extent;
    }
    *rank_out = input.rank;
    dimensions_out[0] = input.dimensions[0];
    dimensions_out[1] = input.dimensions[1];
    for (uint32_t dimension = 2u; dimension < input.rank; ++dimension)
        dimensions_out[dimension] = 1u;
    return true;
}

static inline uint32_t model_convolution_count(const Model* model)
{
    return model && (model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V3 ||
                     model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V4 ||
                     model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V5 ||
                     model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V6 ||
                     model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7 ||
                     model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8 ||
                     model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9 ||
                     model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10)
        ? model->convolution_count : 0u;
}

static inline uint32_t model_batch_quantization_count(const Model* model)
{
    return model && (model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V4 ||
                     model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V5 ||
                     model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V6 ||
                     model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7 ||
                     model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8 ||
                     model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9 ||
                     model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10)
        ? model->batch_quantization_count : 0u;
}

static inline uint32_t model_integer_matmul_count(const Model* model)
{
    return model && (model->version ==
            RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7 ||
            model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8 ||
            model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9 ||
            model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10)
        ? model->integer_matmul_count : 0u;
}

static inline uint32_t model_integer_convolution_count(const Model* model)
{
    return model && (model->version ==
            RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8 ||
            model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9 ||
            model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10)
        ? model->integer_convolution_count : 0u;
}

static inline uint32_t model_average_pool_count(const Model* model)
{
    return model && (model->version ==
            RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9 ||
            model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10)
        ? model->average_pool_count : 0u;
}

static inline const AveragePoolParameters* average_pool_for_operation(
    const Model* model, uint32_t operation_index)
{
    const uint32_t count = model_average_pool_count(model);
    if (!model || (count && !model->average_pools)) return 0;
    for (uint32_t index = 0u; index < count; ++index)
        if (model->average_pools[index].operation_index == operation_index)
            return &model->average_pools[index];
    return 0;
}

static inline const IntegerConvolutionParameters*
integer_convolution_for_operation(const Model* model, uint32_t operation_index)
{
    const uint32_t count = model_integer_convolution_count(model);
    if (!model || (count && !model->integer_convolutions)) return 0;
    for (uint32_t index = 0u; index < count; ++index)
        if (model->integer_convolutions[index].operation_index == operation_index)
            return &model->integer_convolutions[index];
    return 0;
}

static inline const IntegerMatMulParameters* integer_matmul_for_operation(
    const Model* model, uint32_t operation_index)
{
    const uint32_t count = model_integer_matmul_count(model);
    if (!model || (count && !model->integer_matmuls)) return 0;
    for (uint32_t index = 0u; index < count; ++index)
        if (model->integer_matmuls[index].operation_index == operation_index)
            return &model->integer_matmuls[index];
    return 0;
}

static inline const BatchQuantization* batch_quantization_for_tensor(
    const Model* model, uint32_t tensor_index)
{
    const uint32_t count = model_batch_quantization_count(model);
    if (!model || (count && !model->batch_quantizations)) return 0;
    for (uint32_t index = 0u; index < count; ++index)
        if (model->batch_quantizations[index].tensor_index == tensor_index)
            return &model->batch_quantizations[index];
    return 0;
}

static inline const ConvolutionParameters* model_convolution_for_operation(
    const Model* model, uint32_t operation_index)
{
    const uint32_t count = model_convolution_count(model);
    if (!model || (count && !model->convolutions)) return 0;
    for (uint32_t index = 0u; index < count; ++index)
        if (model->convolutions[index].operation_index == operation_index)
            return &model->convolutions[index];
    return 0;
}

static inline bool integer_matmul_zero_point_shape_valid(
    const TensorDescription& zero_point, const TensorDescription& operand,
    bool for_a)
{
    if (zero_point.data_type != operand.data_type) return false;
    if (zero_point.rank == 0u) return true;
    if (operand.rank < 2u) return false;
    if (zero_point.rank == 1u && operand.rank == 2u) {
        const uint32_t expected = for_a
            ? operand.dimensions[operand.rank - 2u]
            : operand.dimensions[operand.rank - 1u];
        return zero_point.dimensions[0] == expected;
    }
    if (zero_point.rank != operand.rank) return false;
    for (uint32_t dimension = 0u; dimension < operand.rank; ++dimension) {
        uint32_t expected = operand.dimensions[dimension];
        if ((for_a && dimension == operand.rank - 1u) ||
            (!for_a && dimension == operand.rank - 2u))
            expected = 1u;
        const uint32_t actual = zero_point.dimensions[dimension];
        if (actual != 1u && actual != expected) return false;
    }
    if (for_a)
        return zero_point.dimensions[zero_point.rank - 2u] ==
                   operand.dimensions[operand.rank - 2u] &&
               zero_point.dimensions[zero_point.rank - 1u] == 1u;
    return zero_point.dimensions[zero_point.rank - 2u] == 1u &&
           zero_point.dimensions[zero_point.rank - 1u] ==
               operand.dimensions[operand.rank - 1u];
}

static inline bool integer_convolution_zero_point_shape_valid(
    const TensorDescription& zero_point, const TensorDescription& operand,
    bool for_weights)
{
    if (zero_point.data_type != operand.data_type) return false;
    if (zero_point.rank == 0u) return true;
    return for_weights && zero_point.rank == 1u && operand.rank >= 3u &&
        zero_point.dimensions[0] == operand.dimensions[0];
}

static inline bool operation_shape_valid(const Model* model,
                                         const Operation& operation,
                                         uint32_t operation_index)
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
    if (operation.opcode == OPERATION_QUANTIZE_Q8 ||
        operation.opcode == OPERATION_DYNAMIC_QUANTIZE_Q8)
        return operation.input_count == 1u &&
            same_shape(output, model->tensors[operation.inputs[0]]);
    if (operation.opcode == OPERATION_DEQUANTIZE_Q8)
        return (operation.input_count == 1u ||
                ((model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V5 ||
                  model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V6 ||
                  model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7 ||
                  model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8 ||
                  model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9 ||
                  model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10) &&
                 operation.input_count == 2u)) &&
            same_shape(output, model->tensors[operation.inputs[0]]);
    if (operation.opcode == OPERATION_MATMUL_Q8) {
        if (operation.input_count != 2u) return false;
        const TensorDescription& left = model->tensors[operation.inputs[0]];
        const TensorDescription& right = model->tensors[operation.inputs[1]];
        uint32_t expected_rank = 0u;
        uint32_t expected_dimensions[RIN_NPU_MODEL_MAX_RANK] = {};
        if (!matmul_output_shape(left, right, &expected_rank,
                                 expected_dimensions) ||
            output.rank != expected_rank)
            return false;
        for (uint32_t dimension = 0u; dimension < expected_rank; ++dimension)
            if (output.dimensions[dimension] != expected_dimensions[dimension])
                return false;
        return true;
    }
    if (operation.opcode == OPERATION_MATMUL_INTEGER) {
        if (operation.input_count != 2u ||
            output.data_type != DATA_TYPE_INT32)
            return false;
        const TensorDescription& left = model->tensors[operation.inputs[0]];
        const TensorDescription& right = model->tensors[operation.inputs[1]];
        uint32_t expected_rank = 0u;
        uint32_t expected_dimensions[RIN_NPU_MODEL_MAX_RANK] = {};
        if (!matmul_output_shape(left, right, &expected_rank,
                                 expected_dimensions) ||
            output.rank != expected_rank)
            return false;
        for (uint32_t dimension = 0u; dimension < expected_rank; ++dimension)
            if (output.dimensions[dimension] != expected_dimensions[dimension])
                return false;
        return true;
    }
    if (operation.opcode == OPERATION_CONV_Q8 ||
        operation.opcode == OPERATION_CONV_INTEGER) {
        if (operation.input_count != 2u) return false;
        uint32_t operation_index = 0u;
        while (operation_index < model->operation_count &&
               &model->operations[operation_index] != &operation)
            ++operation_index;
        const ConvolutionParameters* parameters =
            model_convolution_for_operation(model, operation_index);
        uint32_t expected_rank = 0u;
        uint32_t expected_dimensions[RIN_NPU_MODEL_MAX_RANK] = {};
        if (!parameters || !convolution_output_shape(
                model->tensors[operation.inputs[0]],
                model->tensors[operation.inputs[1]], *parameters,
                &expected_rank, expected_dimensions) ||
            output.rank != expected_rank)
            return false;
        for (uint32_t dimension = 0u; dimension < expected_rank; ++dimension)
            if (output.dimensions[dimension] != expected_dimensions[dimension])
                return false;
        return true;
    }
    if (operation.opcode == OPERATION_AVERAGE_POOL_FLOAT32) {
        if ((model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9 &&
             model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10) ||
            operation.input_count != 1u ||
            model->tensors[operation.inputs[0]].data_type != DATA_TYPE_FLOAT32 ||
            output.data_type != DATA_TYPE_FLOAT32)
            return false;
        const AveragePoolParameters* parameters =
            average_pool_for_operation(model, operation_index);
        uint32_t expected_rank = 0u;
        uint32_t expected_dimensions[RIN_NPU_MODEL_MAX_RANK] = {};
        if (!parameters || !average_pool_output_shape(
                model->tensors[operation.inputs[0]], *parameters,
                &expected_rank, expected_dimensions) ||
            output.rank != expected_rank)
            return false;
        for (uint32_t dimension = 0u; dimension < expected_rank; ++dimension)
            if (output.dimensions[dimension] != expected_dimensions[dimension])
                return false;
        return true;
    }
    if (operation.opcode == OPERATION_GLOBAL_MAX_POOL_FLOAT32) {
        if (model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10 ||
            operation.input_count != 1u ||
            model->tensors[operation.inputs[0]].data_type != DATA_TYPE_FLOAT32 ||
            output.data_type != DATA_TYPE_FLOAT32)
            return false;
        uint32_t expected_rank = 0u;
        uint32_t expected_dimensions[RIN_NPU_MODEL_MAX_RANK] = {};
        if (!global_max_pool_output_shape(model->tensors[operation.inputs[0]],
                &expected_rank, expected_dimensions) ||
            output.rank != expected_rank)
            return false;
        for (uint32_t dimension = 0u; dimension < expected_rank; ++dimension)
            if (output.dimensions[dimension] != expected_dimensions[dimension])
                return false;
        return true;
    }
    return false;
}

static inline uint32_t model_quantization_count(const Model* model)
{
    return model && (model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION ||
                     model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V3 ||
                     model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V4 ||
                     model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V5 ||
                     model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V6 ||
                      model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7 ||
                      model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8 ||
                      model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9 ||
                      model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10)
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
        ((model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9 ||
          model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10)
            ? model->struct_size != sizeof(*model) || model->reserved != 0u ||
                  model->reserved2 != 0u || model->reserved3 != 0u ||
                  model->reserved4 != 0u || model->reserved5 != 0u ||
                  model->reserved6 != 0u
            : model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8
            ? model->struct_size != RIN_NPU_QUANTIZED_MODEL_V8_STRUCT_SIZE ||
                  model->reserved != 0u || model->reserved2 != 0u ||
                  model->reserved3 != 0u || model->reserved4 != 0u ||
                  model->reserved5 != 0u
            : model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7
            ? model->struct_size != RIN_NPU_QUANTIZED_MODEL_V7_STRUCT_SIZE ||
                  model->reserved != 0u || model->reserved2 != 0u ||
                  model->reserved3 != 0u || model->reserved4 != 0u
            : (model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V4 ||
               model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V5 ||
               model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V6)
            ? model->struct_size != RIN_NPU_QUANTIZED_MODEL_V6_STRUCT_SIZE ||
                  model->reserved != 0u || model->reserved2 != 0u ||
                  model->reserved3 != 0u
            : model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V3
            ? model->struct_size != RIN_NPU_QUANTIZED_MODEL_V3_STRUCT_SIZE ||
                  model->reserved != 0u ||
                  model->reserved2 != 0u
            : model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION
                ? model->struct_size != RIN_NPU_QUANTIZED_MODEL_V2_STRUCT_SIZE ||
                      model->reserved != 0u
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
    const uint32_t convolution_count = model_convolution_count(model);
    const uint32_t batch_quantization_count =
        model_batch_quantization_count(model);
    const uint32_t integer_matmul_count =
        model_integer_matmul_count(model);
    const uint32_t integer_convolution_count =
        model_integer_convolution_count(model);
    const uint32_t average_pool_count = model_average_pool_count(model);
    if (quantization_count > model->tensor_count ||
        (quantization_count != 0u && !model->quantizations) ||
        convolution_count > model->operation_count ||
        (convolution_count != 0u && !model->convolutions) ||
        batch_quantization_count > model->tensor_count ||
        (batch_quantization_count != 0u && !model->batch_quantizations) ||
        integer_matmul_count > model->operation_count ||
        (integer_matmul_count != 0u && !model->integer_matmuls) ||
        integer_convolution_count > model->operation_count ||
        (integer_convolution_count != 0u && !model->integer_convolutions) ||
        average_pool_count > model->operation_count ||
        (average_pool_count != 0u && !model->average_pools) ||
        (model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7 &&
         model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8 &&
         model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9 &&
         model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10 &&
         integer_matmul_count != 0u))
        return false;
    if (model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9 &&
        model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10 &&
        average_pool_count != 0u)
        return false;

    if (model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V4 &&
        model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V5 &&
         model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V6 &&
          model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7 &&
          model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8 &&
           model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9 &&
           model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10) {
        for (uint32_t index = 0u; index < model->tensor_count; ++index)
            if (model->tensors[index].data_type == DATA_TYPE_FLOAT32)
                return false;
        for (uint32_t index = 0u; index < model->operation_count; ++index)
            if (model->operations[index].opcode == OPERATION_QUANTIZE_Q8 ||
                model->operations[index].opcode == OPERATION_DEQUANTIZE_Q8 ||
                model->operations[index].opcode ==
                    OPERATION_DYNAMIC_QUANTIZE_Q8)
                return false;
    }

    uint8_t available[RIN_NPU_MODEL_MAX_TENSORS] = {};
    for (uint32_t index = 0u; index < model->tensor_count; ++index)
        if (tensor_elements(model->tensors[index]) == 0u ||
            (model->tensors[index].data_type == DATA_TYPE_INT32 &&
             model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7 &&
             model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8 &&
             model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9 &&
             model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10))
            return false;
    for (uint32_t index = 0u; index < quantization_count; ++index) {
        const AxisQuantization& axis = model->quantizations[index];
        const bool blocked =
            (model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V6 ||
             model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7 ||
             model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8 ||
             model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9 ||
             model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10) &&
            axis.reserved != 0u;
        if ((axis.reserved != 0u && !blocked) ||
            axis.tensor_index >= model->tensor_count ||
            axis.axis >= model->tensors[axis.tensor_index].rank ||
            !axis.count || !axis.scales || !axis.zero_points ||
            model->tensors[axis.tensor_index].rank == 0u ||
            axis_quantization_for_tensor(model, axis.tensor_index) != &axis)
            return false;
        const TensorDescription& axis_tensor =
            model->tensors[axis.tensor_index];
        if (blocked) {
            if (axis_tensor.rank < 2u) return false;
            uint64_t expected_count = 1u;
            for (uint32_t dimension = 0u; dimension < axis_tensor.rank;
                 ++dimension) {
                uint32_t extent = axis_tensor.dimensions[dimension];
                if (dimension == axis.axis)
                    extent = extent / axis.reserved +
                        (extent % axis.reserved != 0u ? 1u : 0u);
                if (!extent || expected_count > UINT32_MAX / extent)
                    return false;
                expected_count *= extent;
            }
            if (expected_count != axis.count) return false;
        } else if (axis.count != axis_tensor.dimensions[axis.axis] ||
                   axis.count < 2u) {
            return false;
        }
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
    for (uint32_t index = 0u; index < batch_quantization_count; ++index) {
        const BatchQuantization& batch = model->batch_quantizations[index];
        if (batch.reserved != 0u || batch.tensor_index >= model->tensor_count ||
            batch.rank < 3u ||
            batch.rank != model->tensors[batch.tensor_index].rank ||
            batch.count < 2u || !batch.scales || !batch.zero_points ||
            batch_quantization_for_tensor(model, batch.tensor_index) != &batch ||
            axis_quantization_for_tensor(model, batch.tensor_index))
            return false;
        uint64_t parameter_count = 1u;
        const TensorDescription& tensor = model->tensors[batch.tensor_index];
        for (uint32_t dimension = 0u; dimension < batch.rank; ++dimension) {
            const uint32_t extent = batch.dimensions[dimension];
            if (!extent || (extent != 1u &&
                            extent != tensor.dimensions[dimension]) ||
                parameter_count > UINT32_MAX / extent)
                return false;
            parameter_count *= extent;
        }
        for (uint32_t dimension = batch.rank;
             dimension < RIN_NPU_MODEL_MAX_RANK; ++dimension)
            if (batch.dimensions[dimension] != 0u) return false;
        if (parameter_count != batch.count) return false;
        int32_t minimum = 0;
        int32_t maximum = 0;
        if (!data_type_bounds(tensor.data_type, &minimum, &maximum))
            return false;
        for (uint32_t parameter = 0u; parameter < batch.count; ++parameter)
            if (!(batch.scales[parameter] > 0.0f) ||
                !(batch.scales[parameter] <= FLT_MAX) ||
                batch.zero_points[parameter] < minimum ||
                batch.zero_points[parameter] > maximum)
                return false;
        for (uint32_t prior = 0u; prior < index; ++prior)
            if (model->batch_quantizations[prior].tensor_index ==
                    batch.tensor_index)
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
        const bool dynamic_quantize =
            operation.opcode == OPERATION_DYNAMIC_QUANTIZE_Q8;
        const bool dynamic_dequantize =
            operation.opcode == OPERATION_DEQUANTIZE_Q8 &&
            operation.input_count == 2u;
        if ((operation.reserved != 0u && !dynamic_quantize &&
             !dynamic_dequantize) ||
            operation.output_tensor >= model->tensor_count ||
            available[operation.output_tensor] != 0u ||
            operation.input_count == 0u || operation.input_count > 2u)
            return false;
        for (uint32_t input = 0u; input < operation.input_count; ++input)
            if (operation.inputs[input] >= model->tensor_count ||
                available[operation.inputs[input]] == 0u)
                return false;
        if (operation.input_count == 1u && operation.inputs[1] != 0u &&
            !dynamic_quantize)
            return false;
        if (dynamic_quantize &&
            ((model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V5 &&
              model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V6 &&
              (model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7 &&
               model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8 &&
                model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9 &&
                model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10)) ||
             operation.input_count != 1u ||
             operation.inputs[1] >= model->tensor_count ||
             operation.reserved >= model->tensor_count ||
             operation.inputs[1] == operation.reserved ||
             operation.inputs[1] == operation.output_tensor ||
             operation.reserved == operation.output_tensor ||
             available[operation.inputs[1]] != 0u ||
             available[operation.reserved] != 0u))
            return false;
        if (dynamic_dequantize &&
            ((model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V5 &&
              model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V6 &&
              (model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7 &&
               model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8 &&
                model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9 &&
                model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10)) ||
             operation.reserved >= model->tensor_count ||
             available[operation.reserved] == 0u ||
             operation.reserved == operation.inputs[0] ||
             operation.reserved == operation.inputs[1] ||
             operation.reserved == operation.output_tensor))
            return false;
        if (dynamic_dequantize) {
            bool matching_quantizer = false;
            for (uint32_t prior = 0u; prior < index; ++prior) {
                const Operation& candidate = model->operations[prior];
                if (candidate.opcode == OPERATION_DYNAMIC_QUANTIZE_Q8 &&
                    candidate.output_tensor == operation.inputs[0] &&
                    candidate.inputs[1] == operation.inputs[1] &&
                    candidate.reserved == operation.reserved) {
                    matching_quantizer = true;
                    break;
                }
            }
            if (!matching_quantizer) return false;
        }
        if (!operation_shape_valid(model, operation, index)) return false;
        const TensorDescription& operation_output =
            model->tensors[operation.output_tensor];
        const TensorDescription& operation_input =
            model->tensors[operation.inputs[0]];
        if (operation.opcode == OPERATION_QUANTIZE_Q8) {
            if (operation_input.data_type != DATA_TYPE_FLOAT32 ||
                (operation_output.data_type != DATA_TYPE_INT8 &&
                 operation_output.data_type != DATA_TYPE_UINT8))
                return false;
        } else if (dynamic_quantize) {
            const TensorDescription& scale_output =
                model->tensors[operation.inputs[1]];
            const TensorDescription& zero_output =
                model->tensors[operation.reserved];
            if (operation_input.data_type != DATA_TYPE_FLOAT32 ||
                operation_output.data_type != DATA_TYPE_UINT8 ||
                scale_output.data_type != DATA_TYPE_FLOAT32 ||
                scale_output.rank != 0u ||
                zero_output.data_type != DATA_TYPE_UINT8 ||
                zero_output.rank != 0u)
                return false;
        } else if (operation.opcode == OPERATION_DEQUANTIZE_Q8) {
            if ((operation_input.data_type != DATA_TYPE_INT8 &&
                 operation_input.data_type != DATA_TYPE_UINT8) ||
                operation_output.data_type != DATA_TYPE_FLOAT32 ||
                (dynamic_dequantize &&
                 (model->tensors[operation.inputs[1]].data_type !=
                      DATA_TYPE_FLOAT32 ||
                  model->tensors[operation.inputs[1]].rank != 0u ||
                  model->tensors[operation.reserved].data_type !=
                      DATA_TYPE_UINT8 ||
                  model->tensors[operation.reserved].rank != 0u)))
                return false;
        } else if (operation.opcode == OPERATION_MATMUL_INTEGER) {
            const IntegerMatMulParameters* parameters =
                integer_matmul_for_operation(model, index);
            if ((model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7 &&
                 model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8 &&
                 model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9 &&
                 model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10) ||
                operation_input.data_type == DATA_TYPE_FLOAT32 ||
                (operation_input.data_type != DATA_TYPE_INT8 &&
                 operation_input.data_type != DATA_TYPE_UINT8) ||
                (model->tensors[operation.inputs[1]].data_type !=
                     DATA_TYPE_INT8 &&
                 model->tensors[operation.inputs[1]].data_type !=
                     DATA_TYPE_UINT8) ||
                operation_output.data_type != DATA_TYPE_INT32 || !parameters ||
                parameters->reserved != 0u)
                return false;
            if (parameters->a_zero_point_tensor != UINT32_MAX) {
                const uint32_t zero_index = parameters->a_zero_point_tensor;
                if (zero_index >= model->tensor_count ||
                    available[zero_index] == 0u ||
                    zero_index == operation.inputs[0] ||
                    zero_index == operation.inputs[1] ||
                    zero_index == operation.output_tensor ||
                    !integer_matmul_zero_point_shape_valid(
                        model->tensors[zero_index], operation_input, true))
                    return false;
            }
            if (parameters->b_zero_point_tensor != UINT32_MAX) {
                const uint32_t zero_index = parameters->b_zero_point_tensor;
                if (zero_index >= model->tensor_count ||
                    available[zero_index] == 0u ||
                    zero_index == operation.inputs[0] ||
                    zero_index == operation.inputs[1] ||
                    zero_index == operation.output_tensor ||
                    !integer_matmul_zero_point_shape_valid(
                        model->tensors[zero_index],
                        model->tensors[operation.inputs[1]], false))
                    return false;
            }
        } else if (operation.opcode == OPERATION_CONV_INTEGER) {
            const IntegerConvolutionParameters* parameters =
                integer_convolution_for_operation(model, index);
            const ConvolutionParameters* geometry =
                model_convolution_for_operation(model, index);
            const TensorDescription& weights =
                model->tensors[operation.inputs[1]];
            if ((model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8 &&
                 model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9 &&
                 model->version != RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10) ||
                (operation_input.data_type != DATA_TYPE_INT8 &&
                 operation_input.data_type != DATA_TYPE_UINT8) ||
                (weights.data_type != DATA_TYPE_INT8 &&
                 weights.data_type != DATA_TYPE_UINT8) ||
                operation_output.data_type != DATA_TYPE_INT32 || !parameters ||
                parameters->reserved != 0u || !geometry ||
                geometry->bias_count != 0u)
                return false;
            if (parameters->input_zero_point_tensor != UINT32_MAX) {
                const uint32_t zero_index =
                    parameters->input_zero_point_tensor;
                if (zero_index >= model->tensor_count ||
                    available[zero_index] == 0u ||
                    zero_index == operation.inputs[0] ||
                    zero_index == operation.inputs[1] ||
                    zero_index == operation.output_tensor ||
                    !integer_convolution_zero_point_shape_valid(
                        model->tensors[zero_index], operation_input, false))
                    return false;
            }
            if (parameters->weight_zero_point_tensor != UINT32_MAX) {
                const uint32_t zero_index =
                    parameters->weight_zero_point_tensor;
                if (zero_index >= model->tensor_count ||
                    available[zero_index] == 0u ||
                    zero_index == operation.inputs[0] ||
                    zero_index == operation.inputs[1] ||
                    zero_index == operation.output_tensor ||
                    zero_index == parameters->input_zero_point_tensor ||
                    !integer_convolution_zero_point_shape_valid(
                        model->tensors[zero_index], weights, true))
                    return false;
            }
        } else if (operation.opcode == OPERATION_AVERAGE_POOL_FLOAT32 ||
                   operation.opcode == OPERATION_GLOBAL_MAX_POOL_FLOAT32) {
            if (operation_input.data_type != DATA_TYPE_FLOAT32 ||
                operation_output.data_type != DATA_TYPE_FLOAT32)
                return false;
        } else {
            if (operation_output.data_type == DATA_TYPE_FLOAT32)
                return false;
            for (uint32_t input = 0u; input < operation.input_count; ++input)
                if (model->tensors[operation.inputs[input]].data_type ==
                    DATA_TYPE_FLOAT32)
                    return false;
        }
        const AxisQuantization* left_axis =
            axis_quantization_for_tensor(model, operation.inputs[0]);
        const AxisQuantization* right_axis = operation.input_count == 2u
            ? axis_quantization_for_tensor(model, operation.inputs[1]) : 0;
        const BatchQuantization* left_batch =
            batch_quantization_for_tensor(model, operation.inputs[0]);
        const BatchQuantization* right_batch = operation.input_count == 2u
            ? batch_quantization_for_tensor(model, operation.inputs[1]) : 0;
        const bool left_axis_valid = !left_axis ||
            operation.opcode == OPERATION_MATMUL_INTEGER ||
            operation.opcode == OPERATION_CONV_INTEGER ||
            (operation.opcode == OPERATION_DEQUANTIZE_Q8 &&
             operation.inputs[0] == left_axis->tensor_index) ||
            (operation.opcode == OPERATION_MATMUL_Q8 &&
             model->tensors[operation.inputs[0]].rank >= 2u &&
             left_axis->axis ==
                 model->tensors[operation.inputs[0]].rank - 2u);
        const bool right_axis_valid = !right_axis ||
            operation.opcode == OPERATION_MATMUL_INTEGER ||
            (operation.opcode == OPERATION_MATMUL_Q8 &&
             model->tensors[operation.inputs[1]].rank >= 2u &&
             right_axis->axis ==
                 model->tensors[operation.inputs[1]].rank - 1u) ||
             ((operation.opcode == OPERATION_CONV_Q8 ||
               operation.opcode == OPERATION_CONV_INTEGER) &&
             right_axis->axis == 0u);
        const TensorDescription& left_tensor =
            model->tensors[operation.inputs[0]];
        const TensorDescription* right_tensor = operation.input_count == 2u
            ? &model->tensors[operation.inputs[1]] : 0;
        const bool left_batch_valid = !left_batch ||
            operation.opcode == OPERATION_MATMUL_INTEGER ||
            (operation.opcode == OPERATION_MATMUL_Q8 &&
             left_batch->dimensions[left_tensor.rank - 2u] ==
                 left_tensor.dimensions[left_tensor.rank - 2u] &&
             left_batch->dimensions[left_tensor.rank - 1u] == 1u);
        const bool right_batch_valid = !right_batch ||
            operation.opcode == OPERATION_MATMUL_INTEGER ||
            (operation.opcode == OPERATION_MATMUL_Q8 && right_tensor &&
             right_batch->dimensions[right_tensor->rank - 2u] == 1u &&
             right_batch->dimensions[right_tensor->rank - 1u] ==
                 right_tensor->dimensions[right_tensor->rank - 1u]);
        if (!left_axis_valid || !right_axis_valid || !left_batch_valid ||
            !right_batch_valid ||
            (axis_quantization_for_tensor(model, operation.output_tensor) &&
             operation.opcode != OPERATION_QUANTIZE_Q8) ||
            batch_quantization_for_tensor(model, operation.output_tensor))
            return false;
        available[operation.output_tensor] = 1u;
        if (dynamic_quantize) {
            available[operation.inputs[1]] = 1u;
            available[operation.reserved] = 1u;
        }
    }
    uint32_t integer_matmul_operations = 0u;
    for (uint32_t index = 0u; index < model->operation_count; ++index)
        if (model->operations[index].opcode == OPERATION_MATMUL_INTEGER)
            ++integer_matmul_operations;
    if (integer_matmul_operations != integer_matmul_count) return false;
    for (uint32_t index = 0u; index < integer_matmul_count; ++index) {
        const IntegerMatMulParameters& parameters =
            model->integer_matmuls[index];
        if (parameters.reserved != 0u ||
            parameters.operation_index >= model->operation_count ||
            model->operations[parameters.operation_index].opcode !=
                OPERATION_MATMUL_INTEGER ||
            integer_matmul_for_operation(model, parameters.operation_index) !=
                &parameters)
            return false;
        for (uint32_t prior = 0u; prior < index; ++prior)
            if (model->integer_matmuls[prior].operation_index ==
                parameters.operation_index)
                return false;
    }
    uint32_t integer_convolution_operations = 0u;
    for (uint32_t index = 0u; index < model->operation_count; ++index)
        if (model->operations[index].opcode == OPERATION_CONV_INTEGER)
            ++integer_convolution_operations;
    if (integer_convolution_operations != integer_convolution_count)
        return false;
    for (uint32_t index = 0u; index < integer_convolution_count; ++index) {
        const IntegerConvolutionParameters& parameters =
            model->integer_convolutions[index];
        if (parameters.reserved != 0u ||
            parameters.operation_index >= model->operation_count ||
            model->operations[parameters.operation_index].opcode !=
                OPERATION_CONV_INTEGER ||
            integer_convolution_for_operation(model, parameters.operation_index)
                != &parameters)
            return false;
        for (uint32_t prior = 0u; prior < index; ++prior)
            if (model->integer_convolutions[prior].operation_index ==
                parameters.operation_index)
                return false;
    }
    uint32_t average_pool_operations = 0u;
    for (uint32_t index = 0u; index < model->operation_count; ++index)
        if (model->operations[index].opcode == OPERATION_AVERAGE_POOL_FLOAT32)
            ++average_pool_operations;
    if (average_pool_operations != average_pool_count) return false;
    for (uint32_t index = 0u; index < average_pool_count; ++index) {
        const AveragePoolParameters& parameters = model->average_pools[index];
        if (parameters.reserved != 0u ||
            parameters.operation_index >= model->operation_count ||
            model->operations[parameters.operation_index].opcode !=
                OPERATION_AVERAGE_POOL_FLOAT32 ||
            average_pool_for_operation(model, parameters.operation_index) !=
                &parameters)
            return false;
        for (uint32_t prior = 0u; prior < index; ++prior)
            if (model->average_pools[prior].operation_index ==
                parameters.operation_index)
                return false;
    }
    for (uint32_t index = 0u; index < model->operation_count; ++index) {
        const Operation& dynamic = model->operations[index];
        if (dynamic.opcode != OPERATION_DYNAMIC_QUANTIZE_Q8) continue;
        for (uint32_t consumer_index = index + 1u;
             consumer_index < model->operation_count; ++consumer_index) {
            const Operation& consumer = model->operations[consumer_index];
            for (uint32_t input = 0u; input < consumer.input_count; ++input) {
                const bool matched_dequantize =
                    consumer.opcode == OPERATION_DEQUANTIZE_Q8 &&
                    consumer.input_count == 2u &&
                    consumer.inputs[0] == dynamic.output_tensor &&
                    consumer.inputs[1] == dynamic.inputs[1] &&
                    consumer.reserved == dynamic.reserved;
                if (consumer.inputs[input] == dynamic.output_tensor &&
                    (input != 0u || !matched_dequantize))
                    return false;
                if (consumer.inputs[input] == dynamic.inputs[1] &&
                    (input != 1u || !matched_dequantize))
                    return false;
                if (consumer.inputs[input] == dynamic.reserved)
                    return false;
            }
            if (consumer.reserved == dynamic.output_tensor ||
                (consumer.reserved == dynamic.reserved &&
                 !(consumer.opcode == OPERATION_DEQUANTIZE_Q8 &&
                   consumer.input_count == 2u &&
                   consumer.inputs[0] == dynamic.output_tensor &&
                   consumer.inputs[1] == dynamic.inputs[1])))
                return false;
        }
    }
    for (uint32_t quantization = 0u;
         quantization < quantization_count; ++quantization) {
        const AxisQuantization& axis = model->quantizations[quantization];
        uint32_t used = 0u;
        const bool blocked =
            (model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V6 ||
             model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7 ||
             model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8 ||
             model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9 ||
             model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10) &&
            axis.reserved != 0u;
        for (uint32_t operation = 0u;
             operation < model->operation_count; ++operation) {
            const Operation& candidate = model->operations[operation];
            if (!blocked && candidate.opcode == OPERATION_MATMUL_Q8 &&
                ((candidate.inputs[0] == axis.tensor_index &&
                  model->tensors[axis.tensor_index].rank >= 2u &&
                  axis.axis == model->tensors[axis.tensor_index].rank - 2u) ||
                 (candidate.inputs[1] == axis.tensor_index &&
                  model->tensors[axis.tensor_index].rank >= 2u &&
                  axis.axis == model->tensors[axis.tensor_index].rank - 1u)))
                used = 1u;
            if (!blocked && (candidate.opcode == OPERATION_CONV_Q8 ||
                             candidate.opcode == OPERATION_CONV_INTEGER) &&
                candidate.input_count == 2u &&
                candidate.inputs[1] == axis.tensor_index && axis.axis == 0u)
                used = 1u;
            if (candidate.opcode == OPERATION_QUANTIZE_Q8 &&
                candidate.output_tensor == axis.tensor_index)
                used = 1u;
            if (candidate.opcode == OPERATION_DEQUANTIZE_Q8 &&
                candidate.input_count == 1u &&
                candidate.inputs[0] == axis.tensor_index)
                used = 1u;
        }
        if (!used) return false;
    }
    for (uint32_t quantization = 0u;
         quantization < batch_quantization_count; ++quantization) {
        const BatchQuantization& batch =
            model->batch_quantizations[quantization];
        uint32_t used = 0u;
        for (uint32_t operation = 0u;
             operation < model->operation_count; ++operation) {
            const Operation& candidate = model->operations[operation];
            if (candidate.opcode != OPERATION_MATMUL_Q8 ||
                candidate.input_count != 2u)
                continue;
            const TensorDescription& left =
                model->tensors[candidate.inputs[0]];
            const TensorDescription& right =
                model->tensors[candidate.inputs[1]];
            if ((candidate.inputs[0] == batch.tensor_index && left.rank >= 3u &&
                 batch.dimensions[left.rank - 2u] ==
                     left.dimensions[left.rank - 2u] &&
                 batch.dimensions[left.rank - 1u] == 1u) ||
                (candidate.inputs[1] == batch.tensor_index && right.rank >= 3u &&
                 batch.dimensions[right.rank - 2u] == 1u &&
                 batch.dimensions[right.rank - 1u] ==
                     right.dimensions[right.rank - 1u]))
                used = 1u;
        }
        if (!used) return false;
    }
    for (uint32_t index = 0u; index < convolution_count; ++index) {
        const ConvolutionParameters& parameters = model->convolutions[index];
        if (parameters.operation_index >= model->operation_count ||
            (model->operations[parameters.operation_index].opcode !=
                 OPERATION_CONV_Q8 &&
             model->operations[parameters.operation_index].opcode !=
                 OPERATION_CONV_INTEGER) ||
            model_convolution_for_operation(model,
                parameters.operation_index) != &parameters)
            return false;
        for (uint32_t prior = 0u; prior < index; ++prior)
            if (model->convolutions[prior].operation_index ==
                parameters.operation_index)
                return false;
        for (uint32_t dimension = parameters.spatial_rank;
             dimension < RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS;
             ++dimension)
            if (parameters.strides[dimension] != 0u ||
                parameters.dilations[dimension] != 0u ||
                parameters.pads_begin[dimension] != 0u ||
                parameters.pads_end[dimension] != 0u)
                return false;
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
    const uint32_t convolution_count = model_convolution_count(model);
    if (convolution_count && memory_ranges_overlap(
            memory, memory_bytes, model->convolutions,
            (uint64_t)convolution_count * sizeof(*model->convolutions)))
        return true;
    for (uint32_t index = 0u; index < convolution_count; ++index) {
        const ConvolutionParameters& parameters = model->convolutions[index];
        if (parameters.bias_count && memory_ranges_overlap(
                memory, memory_bytes, parameters.bias,
                (uint64_t)parameters.bias_count * sizeof(*parameters.bias)))
            return true;
    }
    const uint32_t batch_count = model_batch_quantization_count(model);
    if (batch_count && memory_ranges_overlap(memory, memory_bytes,
            model->batch_quantizations,
            (uint64_t)batch_count * sizeof(*model->batch_quantizations)))
        return true;
    for (uint32_t index = 0u; index < batch_count; ++index) {
        const BatchQuantization& batch = model->batch_quantizations[index];
        if (memory_ranges_overlap(memory, memory_bytes, batch.scales,
                (uint64_t)batch.count * sizeof(*batch.scales)) ||
            memory_ranges_overlap(memory, memory_bytes, batch.zero_points,
                (uint64_t)batch.count * sizeof(*batch.zero_points)))
            return true;
    }
    const uint32_t integer_matmul_count =
        model_integer_matmul_count(model);
    if (integer_matmul_count && memory_ranges_overlap(memory, memory_bytes,
            model->integer_matmuls,
            (uint64_t)integer_matmul_count * sizeof(*model->integer_matmuls)))
        return true;
    const uint32_t integer_convolution_count =
        model_integer_convolution_count(model);
    if (integer_convolution_count && memory_ranges_overlap(memory, memory_bytes,
            model->integer_convolutions,
            (uint64_t)integer_convolution_count *
                sizeof(*model->integer_convolutions)))
        return true;
    const uint32_t average_pool_count = model_average_pool_count(model);
    if (average_pool_count && memory_ranges_overlap(memory, memory_bytes,
            model->average_pools,
            (uint64_t)average_pool_count * sizeof(*model->average_pools)))
        return true;
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
        if (model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V6 ||
            model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7 ||
            model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8 ||
            model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9 ||
            model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10)
            hash = hash_u32(hash, axis.reserved);
        for (uint32_t parameter = 0u; parameter < axis.count; ++parameter) {
            uint32_t scale_bits = 0u;
            memcpy(&scale_bits, &axis.scales[parameter], sizeof(scale_bits));
            hash = hash_u32(hash, scale_bits);
            hash = hash_u32(hash, (uint32_t)axis.zero_points[parameter]);
        }
    }
    const uint32_t convolution_count = model_convolution_count(model);
    hash = hash_u32(hash, convolution_count);
    for (uint32_t index = 0u; index < convolution_count; ++index) {
        const ConvolutionParameters& parameters = model->convolutions[index];
        hash = hash_u32(hash, parameters.operation_index);
        hash = hash_u32(hash, parameters.spatial_rank);
        hash = hash_u32(hash, parameters.group);
        hash = hash_u32(hash, parameters.bias_count);
        for (uint32_t dimension = 0u;
             dimension < RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS;
             ++dimension) {
            hash = hash_u32(hash, parameters.strides[dimension]);
            hash = hash_u32(hash, parameters.dilations[dimension]);
            hash = hash_u32(hash, parameters.pads_begin[dimension]);
            hash = hash_u32(hash, parameters.pads_end[dimension]);
        }
        for (uint32_t channel = 0u; channel < parameters.bias_count; ++channel)
            hash = hash_u32(hash, (uint32_t)parameters.bias[channel]);
    }
    const uint32_t batch_count = model_batch_quantization_count(model);
    if (batch_count != 0u) hash = hash_u32(hash, batch_count);
    for (uint32_t index = 0u; index < batch_count; ++index) {
        const BatchQuantization& batch = model->batch_quantizations[index];
        hash = hash_u32(hash, batch.tensor_index);
        hash = hash_u32(hash, batch.rank);
        hash = hash_u32(hash, batch.count);
        for (uint32_t dimension = 0u;
             dimension < RIN_NPU_MODEL_MAX_RANK; ++dimension)
            hash = hash_u32(hash, batch.dimensions[dimension]);
        for (uint32_t parameter = 0u; parameter < batch.count; ++parameter) {
            uint32_t scale_bits = 0u;
            memcpy(&scale_bits, &batch.scales[parameter], sizeof(scale_bits));
            hash = hash_u32(hash, scale_bits);
            hash = hash_u32(hash, (uint32_t)batch.zero_points[parameter]);
        }
    }
    const uint32_t integer_matmul_count =
        model_integer_matmul_count(model);
    if (integer_matmul_count != 0u)
        hash = hash_u32(hash, integer_matmul_count);
    for (uint32_t index = 0u; index < integer_matmul_count; ++index) {
        const IntegerMatMulParameters& parameters =
            model->integer_matmuls[index];
        hash = hash_u32(hash, parameters.operation_index);
        hash = hash_u32(hash, parameters.a_zero_point_tensor);
        hash = hash_u32(hash, parameters.b_zero_point_tensor);
    }
    const uint32_t integer_convolution_count =
        model_integer_convolution_count(model);
    if (integer_convolution_count != 0u)
        hash = hash_u32(hash, integer_convolution_count);
    for (uint32_t index = 0u; index < integer_convolution_count; ++index) {
        const IntegerConvolutionParameters& parameters =
            model->integer_convolutions[index];
        hash = hash_u32(hash, parameters.operation_index);
        hash = hash_u32(hash, parameters.input_zero_point_tensor);
        hash = hash_u32(hash, parameters.weight_zero_point_tensor);
    }
    const uint32_t average_pool_count = model_average_pool_count(model);
    if (average_pool_count != 0u)
        hash = hash_u32(hash, average_pool_count);
    for (uint32_t index = 0u; index < average_pool_count; ++index) {
        const AveragePoolParameters& parameters = model->average_pools[index];
        hash = hash_u32(hash, parameters.operation_index);
        hash = hash_u32(hash, parameters.spatial_rank);
        hash = hash_u32(hash, parameters.ceil_mode);
        hash = hash_u32(hash, parameters.count_include_pad);
        for (uint32_t axis = 0u;
             axis < RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS; ++axis) {
            hash = hash_u32(hash, parameters.kernel_shape[axis]);
            hash = hash_u32(hash, parameters.strides[axis]);
            hash = hash_u32(hash, parameters.dilations[axis]);
            hash = hash_u32(hash, parameters.pads_begin[axis]);
            hash = hash_u32(hash, parameters.pads_end[axis]);
        }
    }
    return hash;
}

static inline uint64_t serialized_size(const Model* model)
{
    const uint32_t quantization_count = model_quantization_count(model);
    const uint32_t convolution_count = model_convolution_count(model);
    const uint32_t batch_quantization_count =
        model_batch_quantization_count(model);
    const uint32_t integer_matmul_count =
        model_integer_matmul_count(model);
    const uint32_t integer_convolution_count =
        model_integer_convolution_count(model);
    const uint32_t average_pool_count = model_average_pool_count(model);
    const bool version4 = model->version ==
        RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V4 ||
        model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V5 ||
        model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V6 ||
        model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7 ||
        model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8 ||
        model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9 ||
        model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10;
    uint64_t size = ((model->version ==
                          RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10 ||
                      model->version ==
                          RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9)
                     ? UINT64_C(80)
                     : model->version ==
                         RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8
                     ? UINT64_C(72)
                     : model->version ==
                         RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7
                     ? UINT64_C(64)
        : version4 || batch_quantization_count != 0u
        ? UINT64_C(56)
        : convolution_count != 0u ? UINT64_C(48)
        : quantization_count != 0u ? UINT64_C(40) : UINT64_C(32)) +
        (uint64_t)model->tensor_count * 48u +
        (uint64_t)model->input_count * 4u +
        (uint64_t)model->output_count * 4u +
        (uint64_t)model->operation_count * 24u;
    for (uint32_t index = 0u; index < quantization_count; ++index) {
        const uint64_t parameters = model->quantizations[index].count;
        if (parameters > (UINT64_MAX - size - 16u) / 8u) return 0u;
        size += 16u + parameters * 8u;
    }
    for (uint32_t index = 0u; index < convolution_count; ++index) {
        const ConvolutionParameters& parameters = model->convolutions[index];
        const uint64_t record = 16u +
            (uint64_t)parameters.spatial_rank * 16u +
            (uint64_t)parameters.bias_count * 4u;
        if (record > UINT64_MAX - size) return 0u;
        size += record;
    }
    for (uint32_t index = 0u; index < batch_quantization_count; ++index) {
        const BatchQuantization& batch = model->batch_quantizations[index];
        const uint64_t record = 16u +
            (uint64_t)batch.rank * 4u + (uint64_t)batch.count * 8u;
        if (record > UINT64_MAX - size) return 0u;
        size += record;
    }
    if (integer_matmul_count > (UINT64_MAX - size) / 16u) return 0u;
    size += (uint64_t)integer_matmul_count * 16u;
    if (integer_convolution_count > (UINT64_MAX - size) / 16u) return 0u;
    size += (uint64_t)integer_convolution_count * 16u;
    for (uint32_t index = 0u; index < average_pool_count; ++index) {
        const uint64_t record = 20u +
            (uint64_t)model->average_pools[index].spatial_rank * 20u;
        if (record > UINT64_MAX - size) return 0u;
        size += record;
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
    const uint32_t convolution_count = model_convolution_count(model);
    const uint32_t batch_quantization_count =
        model_batch_quantization_count(model);
    const uint32_t integer_matmul_count =
        model_integer_matmul_count(model);
    const uint32_t integer_convolution_count =
        model_integer_convolution_count(model);
    const uint32_t average_pool_count = model_average_pool_count(model);
    const uint32_t serialized_version = model->version ==
            RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10
        ? RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10
        : model->version ==
            RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9
        ? RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9
        : model->version ==
            RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8
        ? RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8
        : model->version ==
            RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7
        ? RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7
        : model->version ==
            RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V6
        ? RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V6
        : model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V5
        ? RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V5
        : model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V4 ||
            batch_quantization_count != 0u
        ? RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V4
        : convolution_count != 0u
        ? RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V3
        : quantization_count != 0u
            ? RIN_NPU_QUANTIZED_MODEL_ABI_VERSION
            : RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V1;
    write_u32_le(output, RIN_NPU_QUANTIZED_MODEL_BLOB_MAGIC);
    write_u32_le(output + 4u, serialized_version);
    const uint32_t header_bytes = serialized_version ==
            RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10 || serialized_version ==
            RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9
        ? 80u
        : serialized_version ==
            RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8
        ? 72u
        : serialized_version ==
            RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7
        ? 64u
        : serialized_version ==
            RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V4 ||
            serialized_version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V5 ||
            serialized_version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V6
        ? 56u
        : convolution_count != 0u ? 48u
        : quantization_count != 0u ? 40u : 32u;
    write_u32_le(output + 8u, header_bytes);
    write_u32_le(output + 12u, (uint32_t)required);
    write_u32_le(output + 16u, model->tensor_count);
    write_u32_le(output + 20u, model->input_count);
    write_u32_le(output + 24u, model->output_count);
    write_u32_le(output + 28u, model->operation_count);
    if (header_bytes >= 40u) {
        write_u32_le(output + 32u, model->quantization_count);
        write_u32_le(output + 36u, 0u);
    }
    if (header_bytes >= 48u) {
        write_u32_le(output + 40u, convolution_count);
        write_u32_le(output + 44u, 0u);
    }
    if (header_bytes >= 56u) {
        write_u32_le(output + 48u, batch_quantization_count);
        write_u32_le(output + 52u, 0u);
    }
    if (header_bytes == 64u) {
        write_u32_le(output + 56u, integer_matmul_count);
        write_u32_le(output + 60u, 0u);
    }
    if (header_bytes == 72u) {
        write_u32_le(output + 56u, integer_matmul_count);
        write_u32_le(output + 60u, 0u);
        write_u32_le(output + 64u, integer_convolution_count);
        write_u32_le(output + 68u, 0u);
    }
    if (header_bytes == 80u) {
        write_u32_le(output + 56u, integer_matmul_count);
        write_u32_le(output + 60u, 0u);
        write_u32_le(output + 64u, integer_convolution_count);
        write_u32_le(output + 68u, 0u);
        write_u32_le(output + 72u, average_pool_count);
        write_u32_le(output + 76u, 0u);
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
        write_u32_le(output + offset + 12u,
            (model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V6 ||
             model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7 ||
             model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8 ||
             model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9 ||
             model->version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10)
                ? axis.reserved : 0u);
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
    for (uint32_t index = 0u; index < convolution_count; ++index) {
        const ConvolutionParameters& parameters = model->convolutions[index];
        write_u32_le(output + offset, parameters.operation_index);
        write_u32_le(output + offset + 4u, parameters.spatial_rank);
        write_u32_le(output + offset + 8u, parameters.group);
        write_u32_le(output + offset + 12u, parameters.bias_count);
        offset += 16u;
        for (uint32_t dimension = 0u; dimension < parameters.spatial_rank;
             ++dimension) {
            write_u32_le(output + offset, parameters.strides[dimension]);
            write_u32_le(output + offset + 4u,
                         parameters.dilations[dimension]);
            write_u32_le(output + offset + 8u,
                         parameters.pads_begin[dimension]);
            write_u32_le(output + offset + 12u,
                         parameters.pads_end[dimension]);
            offset += 16u;
        }
        for (uint32_t channel = 0u; channel < parameters.bias_count; ++channel) {
            write_u32_le(output + offset, (uint32_t)parameters.bias[channel]);
            offset += 4u;
        }
    }
    for (uint32_t index = 0u; index < batch_quantization_count; ++index) {
        const BatchQuantization& batch = model->batch_quantizations[index];
        write_u32_le(output + offset, batch.tensor_index);
        write_u32_le(output + offset + 4u, batch.rank);
        write_u32_le(output + offset + 8u, batch.count);
        write_u32_le(output + offset + 12u, 0u);
        offset += 16u;
        for (uint32_t dimension = 0u; dimension < batch.rank; ++dimension) {
            write_u32_le(output + offset, batch.dimensions[dimension]);
            offset += 4u;
        }
        for (uint32_t parameter = 0u; parameter < batch.count; ++parameter) {
            uint32_t scale_bits = 0u;
            memcpy(&scale_bits, &batch.scales[parameter], sizeof(scale_bits));
            write_u32_le(output + offset, scale_bits);
            offset += 4u;
        }
        for (uint32_t parameter = 0u; parameter < batch.count; ++parameter) {
            write_u32_le(output + offset,
                         (uint32_t)batch.zero_points[parameter]);
            offset += 4u;
        }
    }
    for (uint32_t index = 0u; index < integer_matmul_count; ++index) {
        const IntegerMatMulParameters& parameters =
            model->integer_matmuls[index];
        write_u32_le(output + offset, parameters.operation_index);
        write_u32_le(output + offset + 4u, parameters.a_zero_point_tensor);
        write_u32_le(output + offset + 8u, parameters.b_zero_point_tensor);
        write_u32_le(output + offset + 12u, 0u);
        offset += 16u;
    }
    for (uint32_t index = 0u; index < integer_convolution_count; ++index) {
        const IntegerConvolutionParameters& parameters =
            model->integer_convolutions[index];
        write_u32_le(output + offset, parameters.operation_index);
        write_u32_le(output + offset + 4u,
                     parameters.input_zero_point_tensor);
        write_u32_le(output + offset + 8u,
                     parameters.weight_zero_point_tensor);
        write_u32_le(output + offset + 12u, parameters.reserved);
        offset += 16u;
    }
    for (uint32_t index = 0u; index < average_pool_count; ++index) {
        const AveragePoolParameters& parameters = model->average_pools[index];
        write_u32_le(output + offset, parameters.operation_index);
        write_u32_le(output + offset + 4u, parameters.spatial_rank);
        write_u32_le(output + offset + 8u, parameters.ceil_mode);
        write_u32_le(output + offset + 12u, parameters.count_include_pad);
        write_u32_le(output + offset + 16u, 0u);
        offset += 20u;
        const uint32_t* arrays[] = {
            parameters.kernel_shape, parameters.strides,
            parameters.dilations, parameters.pads_begin, parameters.pads_end};
        for (uint32_t array = 0u; array < 5u; ++array)
            for (uint32_t axis = 0u; axis < parameters.spatial_rank; ++axis) {
                write_u32_le(output + offset, arrays[array][axis]);
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
    int32_t* zero_point_storage = 0, uint32_t parameter_capacity = 0u,
    ConvolutionParameters* convolution_storage = 0,
    uint32_t convolution_capacity = 0u, int32_t* convolution_bias_storage = 0,
    uint32_t convolution_bias_capacity = 0u,
    BatchQuantization* batch_quantization_storage = 0,
    uint32_t batch_quantization_capacity = 0u,
    IntegerMatMulParameters* integer_matmul_storage = 0,
    uint32_t integer_matmul_capacity = 0u,
    IntegerConvolutionParameters* integer_convolution_storage = 0,
    uint32_t integer_convolution_capacity = 0u,
    AveragePoolParameters* average_pool_storage = 0,
    uint32_t average_pool_capacity = 0u)
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
    const bool version3 = version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V3;
    const bool version4 = version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V4;
    const bool version5 = version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V5;
    const bool version6 = version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V6;
    const bool version7 = version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7;
    const bool version8 = version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8;
    const bool version9 = version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9;
    const bool version10 = version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10;
    if ((!version1 && !version2 && !version3 && !version4 && !version5 &&
         !version6 && !version7 && !version8 && !version9 && !version10) ||
        ((version2 || version3 || version4 || version5 || version6 ||
          version7 || version8 || version9 || version10) &&
         blob_bytes < 40u) ||
         ((version3 || version4 || version5 || version6 || version7 ||
            version8 || version9 || version10) &&
         blob_bytes < 48u) ||
         ((version4 || version5 || version6 || version7 || version8 ||
            version9 || version10) && blob_bytes < 56u) ||
         (version7 && blob_bytes < 64u) ||
         (version8 && blob_bytes < 72u) ||
         ((version9 || version10) && blob_bytes < 80u))
        return RIN_RESULT_CORRUPT_DATA;
    const uint32_t header_bytes = version1 ? 32u : version2 ? 40u
        : version3 ? 48u : (version9 || version10) ? 80u : version8 ? 72u
        : version7 ? 64u : 56u;
    if (read_u32_le(input) != RIN_NPU_QUANTIZED_MODEL_BLOB_MAGIC ||
        read_u32_le(input + 8u) != header_bytes ||
        read_u32_le(input + 12u) != blob_bytes || tensor_count == 0u ||
        tensor_count > RIN_NPU_MODEL_MAX_TENSORS || input_count == 0u ||
        input_count > tensor_count || output_count == 0u ||
        output_count > tensor_count ||
        operation_count > RIN_NPU_MODEL_MAX_OPERATIONS)
        return RIN_RESULT_CORRUPT_DATA;
    const uint32_t axis_count = version1 ? 0u : read_u32_le(input + 32u);
    const uint32_t convolution_count = version3 || version4 || version5 ||
        version6 || version7 || version8 || version9 || version10
        ? read_u32_le(input + 40u) : 0u;
    const uint32_t batch_quantization_count =
        version4 || version5 || version6 || version7 || version8 || version9 ||
            version10
        ? read_u32_le(input + 48u) : 0u;
    const uint32_t integer_matmul_count = version7
        || version8 || version9 || version10 ? read_u32_le(input + 56u) : 0u;
    const uint32_t integer_convolution_count = version8 || version9 || version10
        ? read_u32_le(input + 64u) : 0u;
    const uint32_t average_pool_count = version9 || version10
        ? read_u32_le(input + 72u) : 0u;
    if (((version2 || version3 || version4 || version5 || version6 ||
          version7 || version8 || version9 || version10) &&
         read_u32_le(input + 36u) != 0u) ||
        ((version3 || version4 || version5 || version6 || version7 ||
          version8 || version9 || version10) &&
         read_u32_le(input + 44u) != 0u) ||
        ((version4 || version5 || version6 || version7 || version8 ||
           version9 || version10) &&
         read_u32_le(input + 52u) != 0u) ||
         ((version7 || version8 || version9 || version10) &&
         read_u32_le(input + 60u) != 0u) ||
         ((version8 || version9 || version10) &&
          read_u32_le(input + 68u) != 0u) ||
         ((version9 || version10) && read_u32_le(input + 76u) != 0u) ||
        axis_count > tensor_count || convolution_count > operation_count ||
        batch_quantization_count > tensor_count ||
        integer_matmul_count > operation_count ||
        integer_convolution_count > operation_count ||
        average_pool_count > operation_count)
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
        const uint32_t block_size =
            read_u32_le(input + scan_offset + 12u);
        if ((block_size != 0u && !version6 && !version7 && !version8 &&
             !version9 && !version10) ||
            (!block_size && count < 2u) || !count ||
            count > (blob_bytes - scan_offset - 16u) / 8u ||
            parameter_count > UINT32_MAX - count)
            return RIN_RESULT_CORRUPT_DATA;
        parameter_count += count;
        scan_offset += 16u + count * 8u;
    }
    uint64_t convolution_bias_count = 0u;
    for (uint32_t index = 0u; index < convolution_count; ++index) {
        if (scan_offset > blob_bytes || blob_bytes - scan_offset < 16u)
            return RIN_RESULT_CORRUPT_DATA;
        const uint32_t spatial_rank = read_u32_le(input + scan_offset + 4u);
        const uint32_t bias_count = read_u32_le(input + scan_offset + 12u);
        if (spatial_rank == 0u ||
            spatial_rank > RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS ||
            bias_count > RIN_NPU_MODEL_MAX_TENSOR_ELEMENTS ||
            convolution_bias_count > UINT32_MAX - bias_count)
            return RIN_RESULT_CORRUPT_DATA;
        const uint64_t record_bytes = 16u +
            (uint64_t)spatial_rank * 16u + (uint64_t)bias_count * 4u;
        if (record_bytes > blob_bytes - scan_offset)
            return RIN_RESULT_CORRUPT_DATA;
        convolution_bias_count += bias_count;
        scan_offset += (uint32_t)record_bytes;
    }
    for (uint32_t index = 0u; index < batch_quantization_count; ++index) {
        if (scan_offset > blob_bytes || blob_bytes - scan_offset < 16u)
            return RIN_RESULT_CORRUPT_DATA;
        const uint32_t rank = read_u32_le(input + scan_offset + 4u);
        const uint32_t count = read_u32_le(input + scan_offset + 8u);
        if (read_u32_le(input + scan_offset + 12u) != 0u || rank < 3u ||
            rank > RIN_NPU_MODEL_MAX_RANK || count < 2u ||
            (uint64_t)rank * 4u + 16u > blob_bytes - scan_offset ||
            count > (blob_bytes - scan_offset - 16u - rank * 4u) / 8u ||
            parameter_count > UINT32_MAX - count)
            return RIN_RESULT_CORRUPT_DATA;
        parameter_count += count;
        scan_offset += 16u + rank * 4u + count * 8u;
    }
    if (integer_matmul_count > (blob_bytes - scan_offset) / 16u)
        return RIN_RESULT_CORRUPT_DATA;
    const uint32_t integer_matmul_offset = scan_offset;
    scan_offset += integer_matmul_count * 16u;
    if (integer_convolution_count > (blob_bytes - scan_offset) / 16u)
        return RIN_RESULT_CORRUPT_DATA;
    const uint32_t integer_convolution_offset = scan_offset;
    scan_offset += integer_convolution_count * 16u;
    const uint32_t average_pool_offset = scan_offset;
    for (uint32_t index = 0u; index < average_pool_count; ++index) {
        if (scan_offset > blob_bytes || blob_bytes - scan_offset < 20u)
            return RIN_RESULT_CORRUPT_DATA;
        const uint32_t spatial_rank = read_u32_le(input + scan_offset + 4u);
        if (spatial_rank == 0u ||
            spatial_rank > RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS ||
            read_u32_le(input + scan_offset + 8u) > 1u ||
            read_u32_le(input + scan_offset + 12u) > 1u ||
            read_u32_le(input + scan_offset + 16u) != 0u ||
            (uint64_t)spatial_rank * 20u > blob_bytes - scan_offset - 20u)
            return RIN_RESULT_CORRUPT_DATA;
        scan_offset += 20u + spatial_rank * 20u;
    }
    if (scan_offset != blob_bytes) return RIN_RESULT_CORRUPT_DATA;
    if (tensor_capacity < tensor_count || input_capacity < input_count ||
        output_capacity < output_count ||
        operation_capacity < operation_count || !tensor_storage ||
        !input_storage || !output_storage ||
        (operation_count != 0u && !operation_storage) ||
        quantization_capacity < axis_count ||
        parameter_capacity < parameter_count ||
        convolution_capacity < convolution_count ||
        convolution_bias_capacity < convolution_bias_count ||
        batch_quantization_capacity < batch_quantization_count ||
        integer_matmul_capacity < integer_matmul_count ||
        integer_convolution_capacity < integer_convolution_count ||
        average_pool_capacity < average_pool_count ||
        (axis_count && !quantization_storage) ||
        (parameter_count && (!scale_storage || !zero_point_storage)) ||
        (convolution_count && !convolution_storage) ||
        (convolution_bias_count && !convolution_bias_storage) ||
        (batch_quantization_count && !batch_quantization_storage) ||
        (integer_matmul_count && !integer_matmul_storage) ||
        (integer_convolution_count && !integer_convolution_storage) ||
        (average_pool_count && !average_pool_storage))
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
    const uint64_t convolution_bytes =
        (uint64_t)convolution_count * sizeof(*convolution_storage);
    const uint64_t convolution_bias_bytes =
        convolution_bias_count * sizeof(*convolution_bias_storage);
    const uint64_t batch_quantization_bytes = (uint64_t)batch_quantization_count *
        sizeof(*batch_quantization_storage);
    const uint64_t integer_matmul_bytes = (uint64_t)integer_matmul_count *
        sizeof(*integer_matmul_storage);
    const uint64_t integer_convolution_bytes =
        (uint64_t)integer_convolution_count *
            sizeof(*integer_convolution_storage);
    const uint64_t average_pool_bytes = (uint64_t)average_pool_count *
        sizeof(*average_pool_storage);
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
    const void* batch_prior_buffers[] = {
        blob, model_out, tensor_storage, input_storage, output_storage,
        operation_storage, quantization_storage, scale_storage,
        zero_point_storage, convolution_storage, convolution_bias_storage};
    const uint64_t batch_prior_sizes[] = {
        blob_bytes, sizeof(*model_out), tensor_bytes, input_bytes, output_bytes,
        operation_bytes, quantization_bytes, parameter_bytes, parameter_bytes,
        convolution_bytes, convolution_bias_bytes};
    for (uint32_t prior = 0u; prior < 11u; ++prior)
        if (memory_ranges_overlap(batch_quantization_storage,
                batch_quantization_bytes, batch_prior_buffers[prior],
                batch_prior_sizes[prior]))
            return RIN_RESULT_INVALID_ARGUMENT;
    if (memory_ranges_overlap(model_out, sizeof(*model_out),
            batch_quantization_storage, batch_quantization_bytes))
        return RIN_RESULT_INVALID_ARGUMENT;
    const void* integer_prior_buffers[] = {
        blob, model_out, tensor_storage, input_storage, output_storage,
        operation_storage, quantization_storage, scale_storage,
        zero_point_storage, convolution_storage, convolution_bias_storage,
        batch_quantization_storage};
    const uint64_t integer_prior_sizes[] = {
        blob_bytes, sizeof(*model_out), tensor_bytes, input_bytes, output_bytes,
        operation_bytes, quantization_bytes, parameter_bytes, parameter_bytes,
        convolution_bytes, convolution_bias_bytes, batch_quantization_bytes};
    for (uint32_t prior = 0u; prior < 12u; ++prior)
        if (memory_ranges_overlap(integer_matmul_storage, integer_matmul_bytes,
                integer_prior_buffers[prior], integer_prior_sizes[prior]))
            return RIN_RESULT_INVALID_ARGUMENT;
    if (memory_ranges_overlap(integer_convolution_storage,
            integer_convolution_bytes, model_out, sizeof(*model_out)) ||
        memory_ranges_overlap(integer_convolution_storage,
            integer_convolution_bytes, integer_matmul_storage,
            integer_matmul_bytes))
        return RIN_RESULT_INVALID_ARGUMENT;
    const void* integer_convolution_prior_buffers[] = {
        blob, tensor_storage, input_storage, output_storage, operation_storage,
        quantization_storage, scale_storage, zero_point_storage,
        convolution_storage, convolution_bias_storage,
        batch_quantization_storage};
    const uint64_t integer_convolution_prior_sizes[] = {
        blob_bytes, tensor_bytes, input_bytes, output_bytes, operation_bytes,
        quantization_bytes, parameter_bytes, parameter_bytes,
        convolution_bytes, convolution_bias_bytes, batch_quantization_bytes};
    for (uint32_t prior = 0u; prior < 11u; ++prior)
        if (memory_ranges_overlap(integer_convolution_storage,
                integer_convolution_bytes, integer_convolution_prior_buffers[prior],
                integer_convolution_prior_sizes[prior]))
            return RIN_RESULT_INVALID_ARGUMENT;
    const void* average_pool_prior_buffers[] = {
        blob, model_out, tensor_storage, input_storage, output_storage,
        operation_storage, quantization_storage, scale_storage,
        zero_point_storage, convolution_storage, convolution_bias_storage,
        batch_quantization_storage, integer_matmul_storage,
        integer_convolution_storage};
    const uint64_t average_pool_prior_sizes[] = {
        blob_bytes, sizeof(*model_out), tensor_bytes, input_bytes, output_bytes,
        operation_bytes, quantization_bytes, parameter_bytes, parameter_bytes,
        convolution_bytes, convolution_bias_bytes, batch_quantization_bytes,
        integer_matmul_bytes, integer_convolution_bytes};
    for (uint32_t prior = 0u; prior < 14u; ++prior)
        if (memory_ranges_overlap(average_pool_storage, average_pool_bytes,
                average_pool_prior_buffers[prior],
                average_pool_prior_sizes[prior]))
            return RIN_RESULT_INVALID_ARGUMENT;
    const void* prior_buffers[] = {
        blob, model_out, tensor_storage, input_storage, output_storage,
        operation_storage, quantization_storage, scale_storage,
        zero_point_storage};
    const uint64_t prior_sizes[] = {
        blob_bytes, sizeof(*model_out), tensor_bytes, input_bytes, output_bytes,
        operation_bytes, quantization_bytes, parameter_bytes, parameter_bytes};
    const void* appended_buffers[] = {
        convolution_storage, convolution_bias_storage};
    const uint64_t appended_sizes[] = {
        convolution_bytes, convolution_bias_bytes};
    for (uint32_t appended = 0u; appended < 2u; ++appended)
        for (uint32_t prior = 0u; prior < 9u; ++prior)
            if (memory_ranges_overlap(appended_buffers[appended],
                    appended_sizes[appended], prior_buffers[prior],
                    prior_sizes[prior]))
                return RIN_RESULT_INVALID_ARGUMENT;
    if (memory_ranges_overlap(convolution_storage, convolution_bytes,
            convolution_bias_storage, convolution_bias_bytes))
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
        axis.reserved = version6 || version7 || version8 || version9
            ? read_u32_le(input + offset + 12u) : 0u;
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
    uint32_t convolution_bias_offset = 0u;
    for (uint32_t index = 0u; index < convolution_count; ++index) {
        ConvolutionParameters& parameters = convolution_storage[index];
        parameters = {};
        parameters.operation_index = read_u32_le(input + offset);
        parameters.spatial_rank = read_u32_le(input + offset + 4u);
        parameters.group = read_u32_le(input + offset + 8u);
        parameters.bias_count = read_u32_le(input + offset + 12u);
        offset += 16u;
        for (uint32_t dimension = 0u; dimension < parameters.spatial_rank;
             ++dimension) {
            parameters.strides[dimension] = read_u32_le(input + offset);
            parameters.dilations[dimension] =
                read_u32_le(input + offset + 4u);
            parameters.pads_begin[dimension] =
                read_u32_le(input + offset + 8u);
            parameters.pads_end[dimension] =
                read_u32_le(input + offset + 12u);
            offset += 16u;
        }
        if (parameters.bias_count) {
            parameters.bias = convolution_bias_storage +
                convolution_bias_offset;
            for (uint32_t channel = 0u; channel < parameters.bias_count;
                 ++channel) {
                convolution_bias_storage[convolution_bias_offset + channel] =
                    (int32_t)read_u32_le(input + offset);
                offset += 4u;
            }
            convolution_bias_offset += parameters.bias_count;
        }
    }
    for (uint32_t index = 0u; index < batch_quantization_count; ++index) {
        BatchQuantization& batch = batch_quantization_storage[index];
        batch = {};
        batch.tensor_index = read_u32_le(input + offset);
        batch.rank = read_u32_le(input + offset + 4u);
        batch.count = read_u32_le(input + offset + 8u);
        offset += 16u;
        for (uint32_t dimension = 0u; dimension < batch.rank; ++dimension) {
            batch.dimensions[dimension] = read_u32_le(input + offset);
            offset += 4u;
        }
        batch.scales = scale_storage + parameter_offset;
        batch.zero_points = zero_point_storage + parameter_offset;
        for (uint32_t parameter = 0u; parameter < batch.count; ++parameter) {
            const uint32_t bits = read_u32_le(input + offset);
            memcpy(&scale_storage[parameter_offset + parameter], &bits,
                   sizeof(bits));
            offset += 4u;
        }
        for (uint32_t parameter = 0u; parameter < batch.count; ++parameter) {
            zero_point_storage[parameter_offset + parameter] =
                (int32_t)read_u32_le(input + offset);
            offset += 4u;
        }
        parameter_offset += batch.count;
    }
    for (uint32_t index = 0u; index < integer_matmul_count; ++index) {
        IntegerMatMulParameters& parameters = integer_matmul_storage[index];
        parameters.operation_index = read_u32_le(input + integer_matmul_offset +
                                                   (uint64_t)index * 16u);
        parameters.a_zero_point_tensor = read_u32_le(
            input + integer_matmul_offset + (uint64_t)index * 16u + 4u);
        parameters.b_zero_point_tensor = read_u32_le(
            input + integer_matmul_offset + (uint64_t)index * 16u + 8u);
        parameters.reserved = read_u32_le(
            input + integer_matmul_offset + (uint64_t)index * 16u + 12u);
    }
    for (uint32_t index = 0u; index < integer_convolution_count; ++index) {
        IntegerConvolutionParameters& parameters =
            integer_convolution_storage[index];
        parameters.operation_index = read_u32_le(
            input + integer_convolution_offset + (uint64_t)index * 16u);
        parameters.input_zero_point_tensor = read_u32_le(
            input + integer_convolution_offset + (uint64_t)index * 16u + 4u);
        parameters.weight_zero_point_tensor = read_u32_le(
            input + integer_convolution_offset + (uint64_t)index * 16u + 8u);
        parameters.reserved = read_u32_le(
            input + integer_convolution_offset + (uint64_t)index * 16u + 12u);
    }
    uint32_t pool_record_offset = average_pool_offset;
    for (uint32_t index = 0u; index < average_pool_count; ++index) {
        AveragePoolParameters& parameters = average_pool_storage[index];
        parameters = {};
        parameters.operation_index = read_u32_le(input + pool_record_offset);
        parameters.spatial_rank = read_u32_le(input + pool_record_offset + 4u);
        parameters.ceil_mode = read_u32_le(input + pool_record_offset + 8u);
        parameters.count_include_pad =
            read_u32_le(input + pool_record_offset + 12u);
        pool_record_offset += 20u;
        uint32_t* arrays[] = {
            parameters.kernel_shape, parameters.strides,
            parameters.dilations, parameters.pads_begin, parameters.pads_end};
        for (uint32_t array = 0u; array < 5u; ++array)
            for (uint32_t axis = 0u; axis < parameters.spatial_rank; ++axis) {
                arrays[array][axis] = read_u32_le(input + pool_record_offset);
                pool_record_offset += 4u;
            }
    }
    Model candidate = {};
    candidate.struct_size = version1
        ? RIN_NPU_QUANTIZED_MODEL_V1_STRUCT_SIZE
        : version2 ? RIN_NPU_QUANTIZED_MODEL_V2_STRUCT_SIZE
        : version3 ? RIN_NPU_QUANTIZED_MODEL_V3_STRUCT_SIZE
        : version4 || version5 || version6
            ? RIN_NPU_QUANTIZED_MODEL_V6_STRUCT_SIZE
        : version7 ? RIN_NPU_QUANTIZED_MODEL_V7_STRUCT_SIZE
        : version8 ? RIN_NPU_QUANTIZED_MODEL_V8_STRUCT_SIZE
            : sizeof(candidate);
    candidate.version = version;
    candidate.tensor_count = tensor_count;
    candidate.input_count = input_count;
    candidate.output_count = output_count;
    candidate.operation_count = operation_count;
    candidate.tensors = tensor_storage;
    candidate.inputs = input_storage;
    candidate.outputs = output_storage;
    candidate.operations = operation_storage;
    if (version2 || version3 || version4 || version5 || version6 || version7 ||
        version8 || version9 || version10) {
        candidate.quantization_count = axis_count;
        candidate.quantizations = quantization_storage;
    }
    if (version3 || version4 || version5 || version6 || version7 || version8 ||
        version9 || version10) {
        candidate.convolution_count = convolution_count;
        candidate.convolutions = convolution_storage;
    }
    if (version4 || version5 || version6 || version7 || version8 || version9 ||
        version10) {
        candidate.batch_quantization_count = batch_quantization_count;
        candidate.batch_quantizations = batch_quantization_storage;
    }
    if (version7 || version8 || version9 || version10) {
        candidate.integer_matmul_count = integer_matmul_count;
        candidate.integer_matmuls = integer_matmul_storage;
    }
    if (version8 || version9 || version10) {
        candidate.integer_convolution_count = integer_convolution_count;
        candidate.integer_convolutions = integer_convolution_storage;
    }
    if (version9 || version10) {
        candidate.average_pool_count = average_pool_count;
        candidate.average_pools = average_pool_storage;
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
        const TensorDescription& description = model->tensors[index];
        const uint64_t elements = tensor_elements(description);
        const uint64_t bytes = tensor_storage_bytes(description);
        if (!tensors[index].data || tensors[index].element_count != elements ||
            !bytes || bytes > UINTPTR_MAX)
            return RIN_RESULT_INVALID_ARGUMENT;
        const uintptr_t start = (uintptr_t)tensors[index].data;
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

static inline float load_float32(const void* data, uint64_t index)
{
    float value = 0.0f;
    memcpy(&value, static_cast<const uint8_t*>(data) + index * sizeof(value),
           sizeof(value));
    return value;
}

static inline void store_float32(void* data, uint64_t index, float value)
{
    memcpy(static_cast<uint8_t*>(data) + index * sizeof(value), &value,
           sizeof(value));
}

static inline void store_int32(void* data, uint64_t index, int32_t value)
{
    memcpy(static_cast<uint8_t*>(data) + index * sizeof(value), &value,
           sizeof(value));
}

static inline uint64_t integer_matmul_zero_point_offset(
    const TensorDescription& operand, const TensorDescription& zero_point,
    uint64_t element_index, bool for_a)
{
    if (zero_point.rank == 0u) return 0u;
    if (zero_point.rank == 1u) {
        const uint32_t inner = operand.dimensions[operand.rank - 1u];
        const uint32_t columns = operand.dimensions[operand.rank - 1u];
        return for_a ? element_index / inner : element_index % columns;
    }
    uint64_t coordinates[RIN_NPU_MODEL_MAX_RANK] = {};
    for (uint32_t reverse = operand.rank; reverse > 0u; --reverse) {
        const uint32_t dimension = reverse - 1u;
        const uint32_t extent = operand.dimensions[dimension];
        coordinates[dimension] = element_index % extent;
        element_index /= extent;
    }
    uint64_t offset = 0u;
    for (uint32_t dimension = 0u; dimension < zero_point.rank; ++dimension) {
        offset *= zero_point.dimensions[dimension];
        if (zero_point.dimensions[dimension] != 1u)
            offset += coordinates[dimension];
    }
    return offset;
}

static inline RinResultCode evaluate_matmul_integer(
    const Model* model, const Operation& operation, uint32_t operation_index,
    TensorView* tensors, bool write_output)
{
    const IntegerMatMulParameters* parameters =
        integer_matmul_for_operation(model, operation_index);
    if (!parameters) return RIN_RESULT_CORRUPT_DATA;
    const uint32_t left_index = operation.inputs[0];
    const uint32_t right_index = operation.inputs[1];
    const TensorDescription& left_description = model->tensors[left_index];
    const TensorDescription& right_description = model->tensors[right_index];
    const TensorView* left_view = &tensors[left_index];
    const TensorView* right_view = &tensors[right_index];
    const TensorDescription* left_zero_description = 0;
    const TensorDescription* right_zero_description = 0;
    const TensorView* left_zero_view = 0;
    const TensorView* right_zero_view = 0;
    if (parameters->a_zero_point_tensor != UINT32_MAX) {
        left_zero_description =
            &model->tensors[parameters->a_zero_point_tensor];
        left_zero_view = &tensors[parameters->a_zero_point_tensor];
    }
    if (parameters->b_zero_point_tensor != UINT32_MAX) {
        right_zero_description =
            &model->tensors[parameters->b_zero_point_tensor];
        right_zero_view = &tensors[parameters->b_zero_point_tensor];
    }
    const TensorDescription& output_description =
        model->tensors[operation.output_tensor];
    const uint32_t left_batch_rank = left_description.rank - 2u;
    const uint32_t right_batch_rank = right_description.rank - 2u;
    const uint32_t batch_rank = output_description.rank - 2u;
    const uint32_t left_leading = batch_rank - left_batch_rank;
    const uint32_t right_leading = batch_rank - right_batch_rank;
    const uint32_t rows =
        left_description.dimensions[left_description.rank - 2u];
    const uint32_t inner =
        left_description.dimensions[left_description.rank - 1u];
    const uint32_t columns =
        right_description.dimensions[right_description.rank - 1u];
    uint64_t batch_count = 1u;
    for (uint32_t dimension = 0u; dimension < batch_rank; ++dimension)
        batch_count *= output_description.dimensions[dimension];
    for (uint64_t batch = 0u; batch < batch_count; ++batch) {
        uint64_t remaining = batch;
        uint64_t left_batch = 0u;
        uint64_t right_batch = 0u;
        uint32_t batch_coordinates[RIN_NPU_MODEL_MAX_RANK] = {};
        for (uint32_t reverse = batch_rank; reverse > 0u; --reverse) {
            const uint32_t dimension = reverse - 1u;
            const uint32_t extent = output_description.dimensions[dimension];
            batch_coordinates[dimension] = (uint32_t)(remaining % extent);
            remaining /= extent;
        }
        for (uint32_t dimension = 0u; dimension < batch_rank; ++dimension) {
            const uint32_t coordinate = batch_coordinates[dimension];
            if (dimension >= left_leading) {
                const uint32_t source_dimension = dimension - left_leading;
                const uint32_t extent =
                    left_description.dimensions[source_dimension];
                left_batch = left_batch * extent +
                    (extent == 1u ? 0u : coordinate);
            }
            if (dimension >= right_leading) {
                const uint32_t source_dimension = dimension - right_leading;
                const uint32_t extent =
                    right_description.dimensions[source_dimension];
                right_batch = right_batch * extent +
                    (extent == 1u ? 0u : coordinate);
            }
        }
        const uint64_t left_batch_offset = left_batch * (uint64_t)rows * inner;
        const uint64_t right_batch_offset =
            right_batch * (uint64_t)inner * columns;
        for (uint32_t row = 0u; row < rows; ++row) {
            for (uint32_t column = 0u; column < columns; ++column) {
                int64_t accumulator = 0;
                for (uint32_t element = 0u; element < inner; ++element) {
                    const uint64_t left_offset = left_batch_offset +
                        (uint64_t)row * inner + element;
                    const uint64_t right_offset = right_batch_offset +
                        (uint64_t)element * columns + column;
                    int32_t left_zero = 0;
                    int32_t right_zero = 0;
                    if (left_zero_view) {
                        const uint64_t zero_offset =
                            integer_matmul_zero_point_offset(left_description,
                                *left_zero_description, left_offset, true);
                        left_zero = load_q8(*left_zero_description,
                            left_zero_view->data, zero_offset);
                    }
                    if (right_zero_view) {
                        const uint64_t zero_offset =
                            integer_matmul_zero_point_offset(right_description,
                                *right_zero_description, right_offset, false);
                        right_zero = load_q8(*right_zero_description,
                            right_zero_view->data, zero_offset);
                    }
                    const int64_t left_value =
                        load_q8(left_description, left_view->data, left_offset) -
                        left_zero;
                    const int64_t right_value =
                        load_q8(right_description, right_view->data,
                                right_offset) - right_zero;
                    accumulator += left_value * right_value;
                }
                if (accumulator < INT32_MIN || accumulator > INT32_MAX)
                    return RIN_RESULT_LIMIT_EXCEEDED;
                if (write_output) {
                    const uint64_t output_offset =
                        (batch * rows + row) * columns + column;
                    store_int32(tensors[operation.output_tensor].data,
                                output_offset, (int32_t)accumulator);
                }
            }
        }
    }
    return RIN_RESULT_OK;
}

static inline RinResultCode evaluate_convolution_integer(
    const Model* model, const Operation& operation, uint32_t operation_index,
    TensorView* tensors, bool write_output)
{
    const IntegerConvolutionParameters* integer_parameters =
        integer_convolution_for_operation(model, operation_index);
    const ConvolutionParameters* geometry =
        model_convolution_for_operation(model, operation_index);
    if (!integer_parameters || !geometry) return RIN_RESULT_CORRUPT_DATA;
    const uint32_t input_index = operation.inputs[0];
    const uint32_t weight_index = operation.inputs[1];
    const TensorDescription& input = model->tensors[input_index];
    const TensorDescription& weights = model->tensors[weight_index];
    const TensorDescription& output = model->tensors[operation.output_tensor];
    const TensorDescription* input_zero =
        integer_parameters->input_zero_point_tensor == UINT32_MAX ? 0 :
        &model->tensors[integer_parameters->input_zero_point_tensor];
    const TensorDescription* weight_zero =
        integer_parameters->weight_zero_point_tensor == UINT32_MAX ? 0 :
        &model->tensors[integer_parameters->weight_zero_point_tensor];
    const void* input_data = tensors[input_index].data;
    const void* weight_data = tensors[weight_index].data;
    const void* input_zero_data = input_zero
        ? tensors[integer_parameters->input_zero_point_tensor].data : 0;
    const void* weight_zero_data = weight_zero
        ? tensors[integer_parameters->weight_zero_point_tensor].data : 0;
    uint64_t input_spatial_elements = 1u;
    uint64_t output_spatial_elements = 1u;
    uint64_t kernel_spatial_elements = 1u;
    for (uint32_t axis = 0u; axis < geometry->spatial_rank; ++axis) {
        input_spatial_elements *= input.dimensions[axis + 2u];
        output_spatial_elements *= output.dimensions[axis + 2u];
        kernel_spatial_elements *= weights.dimensions[axis + 2u];
    }
    const uint32_t input_channels = input.dimensions[1];
    const uint32_t output_channels = weights.dimensions[0];
    const uint32_t channels_per_group = input_channels / geometry->group;
    const uint32_t outputs_per_group = output_channels / geometry->group;
    const uint64_t output_elements = tensors[operation.output_tensor].element_count;
    for (uint64_t output_element = 0u; output_element < output_elements;
         ++output_element) {
        const uint32_t output_channel = (uint32_t)(
            (output_element / output_spatial_elements) % output_channels);
        const uint64_t batch = output_element /
            ((uint64_t)output_channels * output_spatial_elements);
        uint64_t output_spatial = output_element % output_spatial_elements;
        uint32_t output_coordinates[RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS] = {};
        for (uint32_t reverse = geometry->spatial_rank; reverse > 0u; --reverse) {
            const uint32_t axis = reverse - 1u;
            const uint32_t extent = output.dimensions[axis + 2u];
            output_coordinates[axis] = (uint32_t)(output_spatial % extent);
            output_spatial /= extent;
        }
        const uint32_t group_index = output_channel / outputs_per_group;
        const uint32_t input_group_base = group_index * channels_per_group;
        const uint64_t weight_channel_base =
            (uint64_t)output_channel * channels_per_group *
            kernel_spatial_elements;
        const int32_t input_zero_point = input_zero
            ? load_q8(*input_zero, input_zero_data, 0u) : 0;
        const int32_t weight_zero_point = weight_zero
            ? load_q8(*weight_zero, weight_zero_data,
                weight_zero->rank == 0u ? 0u : output_channel) : 0;
        int64_t accumulator = 0;
        for (uint32_t input_channel = 0u;
             input_channel < channels_per_group; ++input_channel) {
            const uint64_t input_tensor_base =
                (batch * input_channels + input_group_base + input_channel) *
                input_spatial_elements;
            const uint64_t weight_input_base = weight_channel_base +
                (uint64_t)input_channel * kernel_spatial_elements;
            for (uint64_t kernel_element = 0u;
                 kernel_element < kernel_spatial_elements; ++kernel_element) {
                uint64_t remaining_kernel = kernel_element;
                uint32_t kernel_coordinates[
                    RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS] = {};
                for (uint32_t reverse = geometry->spatial_rank;
                     reverse > 0u; --reverse) {
                    const uint32_t axis = reverse - 1u;
                    const uint32_t extent = weights.dimensions[axis + 2u];
                    kernel_coordinates[axis] = (uint32_t)(remaining_kernel % extent);
                    remaining_kernel /= extent;
                }
                uint64_t input_spatial_offset = 0u;
                uint64_t input_stride = 1u;
                bool in_bounds = true;
                for (uint32_t reverse = geometry->spatial_rank;
                     reverse > 0u; --reverse) {
                    const uint32_t axis = reverse - 1u;
                    const int64_t coordinate =
                        (int64_t)output_coordinates[axis] * geometry->strides[axis] -
                        geometry->pads_begin[axis] +
                        (int64_t)kernel_coordinates[axis] * geometry->dilations[axis];
                    const uint32_t extent = input.dimensions[axis + 2u];
                    if (coordinate < 0 || (uint64_t)coordinate >= extent) {
                        in_bounds = false;
                        break;
                    }
                    input_spatial_offset += (uint64_t)coordinate * input_stride;
                    input_stride *= extent;
                }
                if (!in_bounds) continue;
                const int64_t input_value =
                    load_q8(input, input_data,
                        input_tensor_base + input_spatial_offset) -
                    input_zero_point;
                const int64_t weight_value = load_q8(weights, weight_data,
                    weight_input_base + kernel_element) - weight_zero_point;
                accumulator += input_value * weight_value;
            }
        }
        if (accumulator < INT32_MIN || accumulator > INT32_MAX)
            return RIN_RESULT_LIMIT_EXCEEDED;
        if (write_output)
            store_int32(tensors[operation.output_tensor].data, output_element,
                        (int32_t)accumulator);
    }
    return RIN_RESULT_OK;
}

static inline void model_quantization_at(const Model* model,
    uint32_t tensor_index, uint64_t element_index, float* scale_out,
    int32_t* zero_point_out)
{
    const TensorDescription& description = model->tensors[tensor_index];
    *scale_out = description.scale;
    *zero_point_out = description.zero_point;
    const BatchQuantization* batch =
        batch_quantization_for_tensor(model, tensor_index);
    if (batch) {
        uint64_t tensor_stride = 1u;
        uint64_t parameter_stride = 1u;
        uint64_t parameter_index = 0u;
        for (uint32_t reverse = 0u; reverse < description.rank; ++reverse) {
            const uint32_t dimension = description.rank - reverse - 1u;
            const uint32_t extent = description.dimensions[dimension];
            const uint32_t parameter_extent = batch->dimensions[dimension];
            const uint64_t coordinate =
                (element_index / tensor_stride) % extent;
            if (parameter_extent != 1u)
                parameter_index += coordinate * parameter_stride;
            tensor_stride *= extent;
            parameter_stride *= parameter_extent;
        }
        if (parameter_index < batch->count) {
            *scale_out = batch->scales[parameter_index];
            *zero_point_out = batch->zero_points[parameter_index];
        }
        return;
    }
    const AxisQuantization* axis =
        axis_quantization_for_tensor(model, tensor_index);
    if (!axis) return;
    uint64_t inner = 1u;
    for (uint32_t dimension = axis->axis + 1u;
         dimension < description.rank; ++dimension)
        inner *= description.dimensions[dimension];
    const uint64_t coordinate =
        (element_index / inner) % description.dimensions[axis->axis];
    if (axis->reserved != 0u) {
        const uint64_t axis_extent = description.dimensions[axis->axis];
        const uint64_t outer_index = element_index / (axis_extent * inner);
        const uint64_t inner_index = element_index % inner;
        const uint64_t blocked_axis_extent = axis_extent / axis->reserved +
            (axis_extent % axis->reserved != 0u ? 1u : 0u);
        const uint64_t parameter_index =
            (outer_index * blocked_axis_extent + coordinate / axis->reserved) *
                inner + inner_index;
        if (parameter_index < axis->count) {
            *scale_out = axis->scales[parameter_index];
            *zero_point_out = axis->zero_points[parameter_index];
        }
    } else {
        *scale_out = axis->scales[coordinate];
        *zero_point_out = axis->zero_points[coordinate];
    }
}

static inline bool runtime_quantization_at(const Model* model,
    const TensorView* tensors, uint32_t tensor_index, uint64_t element_index,
    float* scale_out, int32_t* zero_point_out)
{
    if (!model || !tensors || tensor_index >= model->tensor_count ||
        !scale_out || !zero_point_out)
        return false;
    for (uint32_t index = 0u; index < model->operation_count; ++index) {
        const Operation& operation = model->operations[index];
        if (operation.opcode != OPERATION_DYNAMIC_QUANTIZE_Q8 ||
            operation.output_tensor != tensor_index)
            continue;
        if (operation.inputs[1] >= model->tensor_count ||
            operation.reserved >= model->tensor_count)
            return false;
        const float scale = load_float32(
            tensors[operation.inputs[1]].data, 0u);
        const int32_t zero_point = (int32_t)static_cast<const uint8_t*>(
            tensors[operation.reserved].data)[0];
        if (!(scale > 0.0f) || !(scale <= FLT_MAX) || !isfinite(scale))
            return false;
        *scale_out = scale;
        *zero_point_out = zero_point;
        return true;
    }
    model_quantization_at(model, tensor_index, element_index, scale_out,
                          zero_point_out);
    return true;
}

static inline double dequantize_runtime_value(const Model* model,
    const TensorView* tensors, uint32_t tensor_index, uint64_t element_index)
{
    const TensorDescription& description = model->tensors[tensor_index];
    float scale = 0.0f;
    int32_t zero_point = 0;
    if (!runtime_quantization_at(model, tensors, tensor_index, element_index,
                                 &scale, &zero_point))
        return NAN;
    return (double)(load_q8(description, tensors[tensor_index].data,
                            element_index) - zero_point) * (double)scale;
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

static inline bool dynamic_quantize_linear(const void* input,
    uint64_t element_count, uint8_t* output, float* scale_out,
    uint8_t* zero_point_out)
{
    if (!input || !element_count || !output || !scale_out ||
        !zero_point_out)
        return false;
    float range_min = 0.0f;
    float range_max = 0.0f;
    for (uint64_t index = 0u; index < element_count; ++index) {
        const float value = load_float32(input, index);
        if (!isfinite(value)) return false;
        if (value < range_min) range_min = value;
        if (value > range_max) range_max = value;
    }
    const float range = range_max - range_min;
    if (!isfinite(range)) return false;
    const float scale = range == 0.0f ? 1.0f : range / 255.0f;
    if (!(scale > 0.0f) || !isfinite(scale))
        return false;
    const float zero_scaled_float = -range_min / scale;
    double zero_scaled = (double)zero_scaled_float;
    if (!isfinite(zero_scaled_float)) return false;
    if (zero_scaled < 0.0) zero_scaled = 0.0;
    if (zero_scaled > 255.0) zero_scaled = 255.0;
    int64_t zero_point = round_ties_to_even(zero_scaled);
    if (zero_point < 0) zero_point = 0;
    if (zero_point > 255) zero_point = 255;
    for (uint64_t index = 0u; index < element_count; ++index) {
        const float scaled_float = load_float32(input, index) / scale;
        const double scaled = (double)scaled_float;
        if (!isfinite(scaled_float)) return false;
        int64_t quantized;
        if (scaled <= -(double)zero_point - 1.0)
            quantized = 0;
        else if (scaled >= 256.0 - (double)zero_point)
            quantized = 255;
        else
            quantized = round_ties_to_even(scaled) + zero_point;
        if (quantized < 0) quantized = 0;
        if (quantized > 255) quantized = 255;
        output[index] = (uint8_t)quantized;
    }
    *scale_out = scale;
    *zero_point_out = (uint8_t)zero_point;
    return true;
}

static inline void store_quantized(const TensorDescription& description,
                                   void* data, uint64_t index,
                                   double real_value)
{
    int32_t minimum = 0;
    int32_t maximum = 0;
    data_type_bounds(description.data_type, &minimum, &maximum);
    const double scaled = real_value / (double)description.scale;
    int64_t quantized;
    const double minimum_scaled =
        (double)minimum - (double)description.zero_point;
    const double maximum_scaled =
        (double)maximum - (double)description.zero_point;
    if (scaled <= minimum_scaled - 1.0)
        quantized = minimum;
    else if (scaled >= maximum_scaled + 1.0)
        quantized = maximum;
    else {
        quantized = round_ties_to_even(scaled) + description.zero_point;
    }
    if (quantized < minimum) quantized = minimum;
    if (quantized > maximum) quantized = maximum;
    if (description.data_type == DATA_TYPE_INT8)
        static_cast<int8_t*>(data)[index] = (int8_t)quantized;
    else
        static_cast<uint8_t*>(data)[index] = (uint8_t)quantized;
}

static inline bool store_model_quantized(const Model* model,
    uint32_t tensor_index, void* data, uint64_t index, double real_value)
{
    const TensorDescription& description = model->tensors[tensor_index];
    float scale = 0.0f;
    int32_t zero_point = 0;
    model_quantization_at(model, tensor_index, index, &scale, &zero_point);
    int32_t minimum = 0;
    int32_t maximum = 0;
    if (!data_type_bounds(description.data_type, &minimum, &maximum) ||
        !(scale > 0.0f) || !(scale <= FLT_MAX) || !isfinite(real_value))
        return false;
    const double scaled = real_value / (double)scale;
    int64_t quantized;
    const double minimum_scaled = (double)minimum - (double)zero_point;
    const double maximum_scaled = (double)maximum - (double)zero_point;
    if (scaled <= minimum_scaled - 1.0)
        quantized = minimum;
    else if (scaled >= maximum_scaled + 1.0)
        quantized = maximum;
    else {
        if (!isfinite(scaled)) return false;
        quantized = round_ties_to_even(scaled) + zero_point;
    }
    if (quantized < minimum) quantized = minimum;
    if (quantized > maximum) quantized = maximum;
    if (description.data_type == DATA_TYPE_INT8)
        static_cast<int8_t*>(data)[index] = (int8_t)quantized;
    else
        static_cast<uint8_t*>(data)[index] = (uint8_t)quantized;
    return true;
}

static inline RinResultCode evaluate_average_pool_float32(
    const Model* model, const Operation& operation, uint32_t operation_index,
    TensorView* tensors)
{
    const AveragePoolParameters* parameters =
        average_pool_for_operation(model, operation_index);
    if (!parameters || operation.input_count != 1u)
        return RIN_RESULT_CORRUPT_DATA;
    const TensorDescription& input_description =
        model->tensors[operation.inputs[0]];
    const TensorDescription& output_description =
        model->tensors[operation.output_tensor];
    uint64_t input_spatial_elements = 1u;
    uint64_t output_spatial_elements = 1u;
    uint64_t kernel_elements = 1u;
    for (uint32_t axis = 0u; axis < parameters->spatial_rank; ++axis) {
        const uint32_t input_extent = input_description.dimensions[axis + 2u];
        const uint32_t output_extent = output_description.dimensions[axis + 2u];
        const uint32_t kernel_extent = parameters->kernel_shape[axis];
        if (input_spatial_elements > UINT64_MAX / input_extent ||
            output_spatial_elements > UINT64_MAX / output_extent ||
            kernel_elements > UINT64_MAX / kernel_extent)
            return RIN_RESULT_CORRUPT_DATA;
        input_spatial_elements *= input_extent;
        output_spatial_elements *= output_extent;
        kernel_elements *= kernel_extent;
    }
    const void* input = tensors[operation.inputs[0]].data;
    void* output = tensors[operation.output_tensor].data;
    const uint64_t output_elements =
        tensors[operation.output_tensor].element_count;
    for (uint64_t output_element = 0u;
         output_element < output_elements; ++output_element) {
        const uint64_t outer_index =
            output_element / output_spatial_elements;
        uint64_t output_spatial = output_element % output_spatial_elements;
        uint32_t output_coordinates[
            RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS] = {};
        for (uint32_t reverse = parameters->spatial_rank; reverse > 0u;
             --reverse) {
            const uint32_t axis = reverse - 1u;
            const uint32_t extent = output_description.dimensions[axis + 2u];
            output_coordinates[axis] = (uint32_t)(output_spatial % extent);
            output_spatial /= extent;
        }

        double sum = 0.0;
        uint64_t valid_samples = 0u;
        for (uint64_t kernel_index = 0u;
             kernel_index < kernel_elements; ++kernel_index) {
            uint64_t remaining = kernel_index;
            uint64_t input_spatial = 0u;
            bool in_bounds = true;
            uint32_t kernel_coordinates[
                RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS] = {};
            for (uint32_t reverse = parameters->spatial_rank; reverse > 0u;
                 --reverse) {
                const uint32_t axis = reverse - 1u;
                const uint32_t extent = parameters->kernel_shape[axis];
                kernel_coordinates[axis] = (uint32_t)(remaining % extent);
                remaining /= extent;
            }
            for (uint32_t axis = 0u; axis < parameters->spatial_rank; ++axis) {
                const int64_t coordinate =
                    (int64_t)output_coordinates[axis] *
                        (int64_t)parameters->strides[axis] -
                    (int64_t)parameters->pads_begin[axis] +
                    (int64_t)kernel_coordinates[axis] *
                        (int64_t)parameters->dilations[axis];
                const uint32_t extent = input_description.dimensions[axis + 2u];
                if (coordinate < 0 || coordinate >= (int64_t)extent) {
                    in_bounds = false;
                    break;
                }
                input_spatial = input_spatial * extent +
                    (uint64_t)coordinate;
            }
            if (!in_bounds) continue;
            const uint64_t input_element =
                outer_index * input_spatial_elements + input_spatial;
            sum += (double)load_float32(input, input_element);
            ++valid_samples;
        }
        const uint64_t divisor = parameters->count_include_pad
            ? kernel_elements : valid_samples;
        if (divisor == 0u) return RIN_RESULT_INVALID_ARGUMENT;
        store_float32(output, output_element, (float)(sum / (double)divisor));
    }
    return RIN_RESULT_OK;
}

static inline RinResultCode evaluate_global_max_pool_float32(
    const Model* model, const Operation& operation, TensorView* tensors)
{
    if (operation.input_count != 1u ||
        model->tensors[operation.inputs[0]].data_type != DATA_TYPE_FLOAT32 ||
        model->tensors[operation.output_tensor].data_type != DATA_TYPE_FLOAT32)
        return RIN_RESULT_CORRUPT_DATA;
    const TensorDescription& input_description =
        model->tensors[operation.inputs[0]];
    uint64_t spatial_elements = 1u;
    for (uint32_t dimension = 2u; dimension < input_description.rank;
         ++dimension) {
        const uint32_t extent = input_description.dimensions[dimension];
        if (!extent || spatial_elements > UINT64_MAX / extent)
            return RIN_RESULT_CORRUPT_DATA;
        spatial_elements *= extent;
    }
    const uint64_t output_elements =
        tensors[operation.output_tensor].element_count;
    const uint64_t input_elements =
        tensors[operation.inputs[0]].element_count;
    if (!spatial_elements || !output_elements ||
        output_elements > UINT64_MAX / spatial_elements ||
        output_elements * spatial_elements != input_elements)
        return RIN_RESULT_CORRUPT_DATA;

    const void* input = tensors[operation.inputs[0]].data;
    void* output = tensors[operation.output_tensor].data;
    for (uint64_t output_element = 0u; output_element < output_elements;
         ++output_element) {
        const uint64_t begin = output_element * spatial_elements;
        float maximum = load_float32(input, begin);
        for (uint64_t spatial = 1u; spatial < spatial_elements; ++spatial) {
            const float value = load_float32(input, begin + spatial);
            if (isnan(maximum) || isnan(value)) {
                maximum = NAN;
                break;
            }
            if (value > maximum) maximum = value;
        }
        store_float32(output, output_element, maximum);
    }
    return RIN_RESULT_OK;
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
                tensor_storage_bytes(plan->model->tensors[index]),
                plan, sizeof(*plan)))
            return RIN_RESULT_INVALID_ARGUMENT;
    const Model* model = plan->model;
    for (uint32_t index = 0u; index < model->operation_count; ++index) {
        const Operation& operation = model->operations[index];
        const uint32_t output_index = operation.output_tensor;
        const TensorDescription& output_description =
            model->tensors[output_index];
        void* output = tensors[output_index].data;
        if (operation.opcode == OPERATION_DYNAMIC_QUANTIZE_Q8) {
            const uint32_t input_index = operation.inputs[0];
            const uint64_t elements = tensors[input_index].element_count;
            float scale = 0.0f;
            uint8_t zero_point = 0u;
            if (!dynamic_quantize_linear(tensors[input_index].data, elements,
                    static_cast<uint8_t*>(output), &scale, &zero_point))
                return RIN_RESULT_INVALID_ARGUMENT;
            store_float32(tensors[operation.inputs[1]].data, 0u, scale);
            static_cast<uint8_t*>(tensors[operation.reserved].data)[0] =
                zero_point;
        } else if (operation.opcode == OPERATION_QUANTIZE_Q8) {
            const uint32_t input_index = operation.inputs[0];
            const uint64_t elements = tensors[output_index].element_count;
            for (uint64_t element = 0u; element < elements; ++element) {
                const float value = load_float32(
                    tensors[input_index].data, element);
                if (!store_model_quantized(model, output_index, output,
                                            element, value))
                    return RIN_RESULT_INVALID_ARGUMENT;
            }
        } else if (operation.opcode == OPERATION_DEQUANTIZE_Q8) {
            const uint32_t input_index = operation.inputs[0];
            const uint64_t elements = tensors[output_index].element_count;
            for (uint64_t element = 0u; element < elements; ++element) {
                const double value = dequantize_runtime_value(
                    model, tensors, input_index, element);
                if (!isfinite(value) || value > FLT_MAX || value < -FLT_MAX)
                    return RIN_RESULT_INVALID_ARGUMENT;
                store_float32(output, element, (float)value);
            }
        } else if (operation.opcode == OPERATION_ADD_Q8 ||
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
        } else if (operation.opcode == OPERATION_MATMUL_INTEGER) {
            RinResultCode result = evaluate_matmul_integer(
                model, operation, index, tensors, false);
            if (result != RIN_RESULT_OK) return result;
            result = evaluate_matmul_integer(model, operation, index, tensors,
                                             true);
            if (result != RIN_RESULT_OK) return result;
        } else if (operation.opcode == OPERATION_CONV_INTEGER) {
            RinResultCode result = evaluate_convolution_integer(
                model, operation, index, tensors, false);
            if (result != RIN_RESULT_OK) return result;
            result = evaluate_convolution_integer(model, operation, index,
                                                  tensors, true);
            if (result != RIN_RESULT_OK) return result;
        } else if (operation.opcode == OPERATION_AVERAGE_POOL_FLOAT32) {
            const RinResultCode result = evaluate_average_pool_float32(
                model, operation, index, tensors);
            if (result != RIN_RESULT_OK) return result;
        } else if (operation.opcode == OPERATION_GLOBAL_MAX_POOL_FLOAT32) {
            const RinResultCode result = evaluate_global_max_pool_float32(
                model, operation, tensors);
            if (result != RIN_RESULT_OK) return result;
        } else if (operation.opcode == OPERATION_MATMUL_Q8) {
            const uint32_t left_index = operation.inputs[0];
            const uint32_t right_index = operation.inputs[1];
            const TensorDescription& left_description =
                model->tensors[left_index];
            const TensorDescription& right_description =
                model->tensors[right_index];
            const void* left = tensors[left_index].data;
            const void* right = tensors[right_index].data;
            const uint32_t left_batch_rank = left_description.rank - 2u;
            const uint32_t right_batch_rank = right_description.rank - 2u;
            const uint32_t batch_rank = output_description.rank - 2u;
            const uint32_t left_leading = batch_rank - left_batch_rank;
            const uint32_t right_leading = batch_rank - right_batch_rank;
            const uint32_t rows =
                left_description.dimensions[left_description.rank - 2u];
            const uint32_t inner =
                left_description.dimensions[left_description.rank - 1u];
            const uint32_t columns =
                right_description.dimensions[right_description.rank - 1u];
            uint64_t batch_count = 1u;
            for (uint32_t dimension = 0u; dimension < batch_rank; ++dimension)
                batch_count *= output_description.dimensions[dimension];
            for (uint64_t batch = 0u; batch < batch_count; ++batch) {
                uint64_t remaining = batch;
                uint64_t left_batch = 0u;
                uint64_t right_batch = 0u;
                uint32_t batch_coordinates[RIN_NPU_MODEL_MAX_RANK] = {};
                for (uint32_t reverse = batch_rank; reverse > 0u; --reverse) {
                    const uint32_t dimension = reverse - 1u;
                    const uint32_t extent =
                        output_description.dimensions[dimension];
                    batch_coordinates[dimension] =
                        (uint32_t)(remaining % extent);
                    remaining /= extent;
                }
                for (uint32_t dimension = 0u; dimension < batch_rank;
                     ++dimension) {
                    const uint32_t coordinate = batch_coordinates[dimension];
                    if (dimension >= left_leading) {
                        const uint32_t source_dimension =
                            dimension - left_leading;
                        const uint32_t source_extent =
                            left_description.dimensions[source_dimension];
                        left_batch = left_batch * source_extent +
                            (source_extent == 1u ? 0u : coordinate);
                    }
                    if (dimension >= right_leading) {
                        const uint32_t source_dimension =
                            dimension - right_leading;
                        const uint32_t source_extent =
                            right_description.dimensions[source_dimension];
                        right_batch = right_batch * source_extent +
                            (source_extent == 1u ? 0u : coordinate);
                    }
                }
                const uint64_t left_batch_offset =
                    left_batch * (uint64_t)rows * inner;
                const uint64_t right_batch_offset =
                    right_batch * (uint64_t)inner * columns;
                for (uint32_t row = 0u; row < rows; ++row)
                    for (uint32_t column = 0u; column < columns; ++column) {
                        float left_scale = 0.0f;
                        int32_t left_zero_point = 0;
                        float right_scale = 0.0f;
                        int32_t right_zero_point = 0;
                        model_quantization_at(model, left_index,
                            left_batch_offset + (uint64_t)row * inner,
                            &left_scale, &left_zero_point);
                        model_quantization_at(model, right_index,
                            right_batch_offset + column, &right_scale,
                            &right_zero_point);
                        const double output_scale =
                            (double)left_scale * (double)right_scale;
                        int64_t accumulator = 0;
                        for (uint32_t element = 0u; element < inner; ++element) {
                            const uint64_t left_offset = left_batch_offset +
                                (uint64_t)row * inner + element;
                            const uint64_t right_offset = right_batch_offset +
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
                            (batch * rows + row) * columns + column;
                        store_quantized(output_description, output,
                                        output_offset, real_value);
                    }
            }
        } else if (operation.opcode == OPERATION_CONV_Q8) {
            const uint32_t input_index = operation.inputs[0];
            const uint32_t weight_index = operation.inputs[1];
            const TensorDescription& input_description =
                model->tensors[input_index];
            const TensorDescription& weight_description =
                model->tensors[weight_index];
            const ConvolutionParameters* parameters =
                model_convolution_for_operation(model, index);
            if (!parameters) return RIN_RESULT_CORRUPT_DATA;
            uint64_t input_spatial_elements = 1u;
            uint64_t output_spatial_elements = 1u;
            uint64_t kernel_spatial_elements = 1u;
            for (uint32_t axis = 0u; axis < parameters->spatial_rank; ++axis) {
                input_spatial_elements *=
                    input_description.dimensions[axis + 2u];
                output_spatial_elements *=
                    output_description.dimensions[axis + 2u];
                kernel_spatial_elements *=
                    weight_description.dimensions[axis + 2u];
            }
            const uint32_t input_channels = input_description.dimensions[1];
            const uint32_t output_channels = weight_description.dimensions[0];
            const uint32_t channels_per_group =
                input_channels / parameters->group;
            const uint32_t outputs_per_group =
                output_channels / parameters->group;
            const void* input_data = tensors[input_index].data;
            const void* weight_data = tensors[weight_index].data;
            const uint64_t output_elements =
                tensors[output_index].element_count;
            for (uint64_t output_element = 0u;
                 output_element < output_elements; ++output_element) {
                const uint32_t output_channel = (uint32_t)(
                    (output_element / output_spatial_elements) %
                    output_channels);
                const uint64_t batch = output_element /
                    ((uint64_t)output_channels * output_spatial_elements);
                uint64_t output_spatial =
                    output_element % output_spatial_elements;
                uint32_t output_coordinates[
                    RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS] = {};
                for (uint32_t reverse = parameters->spatial_rank;
                     reverse > 0u; --reverse) {
                    const uint32_t axis = reverse - 1u;
                    const uint32_t extent =
                        output_description.dimensions[axis + 2u];
                    output_coordinates[axis] =
                        (uint32_t)(output_spatial % extent);
                    output_spatial /= extent;
                }
                const uint32_t group_index =
                    output_channel / outputs_per_group;
                const uint32_t input_channel_group_base =
                    group_index * channels_per_group;
                const uint64_t weight_channel_base =
                    (uint64_t)output_channel * channels_per_group *
                    kernel_spatial_elements;
                float input_scale = 0.0f;
                int32_t input_zero_point = 0;
                float weight_scale = 0.0f;
                int32_t weight_zero_point = 0;
                model_quantization_at(model, input_index, 0u, &input_scale,
                                      &input_zero_point);
                model_quantization_at(model, weight_index,
                    weight_channel_base, &weight_scale, &weight_zero_point);
                int64_t accumulator = parameters->bias_count
                    ? parameters->bias[output_channel] : 0;
                for (uint32_t input_channel = 0u;
                     input_channel < channels_per_group; ++input_channel) {
                    const uint64_t input_tensor_offset =
                        (batch * input_channels + input_channel_group_base +
                         input_channel) * input_spatial_elements;
                    const uint64_t weight_input_base = weight_channel_base +
                        (uint64_t)input_channel * kernel_spatial_elements;
                    for (uint64_t kernel_element = 0u;
                         kernel_element < kernel_spatial_elements;
                         ++kernel_element) {
                        uint64_t remaining_kernel = kernel_element;
                        uint64_t input_spatial_offset = 0u;
                        uint64_t input_stride = 1u;
                        uint32_t kernel_coordinates[
                            RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS] = {};
                        for (uint32_t reverse = parameters->spatial_rank;
                             reverse > 0u; --reverse) {
                            const uint32_t axis = reverse - 1u;
                            const uint32_t kernel_extent =
                                weight_description.dimensions[axis + 2u];
                            kernel_coordinates[axis] = (uint32_t)(
                                remaining_kernel % kernel_extent);
                            remaining_kernel /= kernel_extent;
                        }
                        uint32_t in_bounds = 1u;
                        for (uint32_t axis = parameters->spatial_rank;
                             axis > 0u; --axis) {
                            const uint32_t dimension = axis - 1u;
                            const int64_t input_coordinate =
                                (int64_t)output_coordinates[dimension] *
                                    parameters->strides[dimension] -
                                parameters->pads_begin[dimension] +
                                (int64_t)kernel_coordinates[dimension] *
                                    parameters->dilations[dimension];
                            const uint32_t input_extent =
                                input_description.dimensions[dimension + 2u];
                            if (input_coordinate < 0 ||
                                (uint64_t)input_coordinate >= input_extent) {
                                in_bounds = 0u;
                                break;
                            }
                            input_spatial_offset +=
                                (uint64_t)input_coordinate * input_stride;
                            input_stride *= input_extent;
                        }
                        if (!in_bounds) continue;
                        const uint64_t input_offset = input_tensor_offset +
                            input_spatial_offset;
                        const uint64_t weight_offset =
                            weight_input_base + kernel_element;
                        const int64_t input_value =
                            load_q8(input_description, input_data,
                                    input_offset) - input_zero_point;
                        const int64_t weight_value =
                            load_q8(weight_description, weight_data,
                                    weight_offset) - weight_zero_point;
                        accumulator += input_value * weight_value;
                    }
                }
                const double real_value = (double)accumulator *
                    (double)input_scale * (double)weight_scale;
                store_quantized(output_description, output, output_element,
                                real_value);
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
                tensor_storage_bytes(
                    executable->model->tensors[index]), executable,
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
                tensor_storage_bytes(
                    executable->model->tensors[index]), executable->provider,
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
