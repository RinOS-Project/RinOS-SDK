#ifndef RIN_DRIVER_NPU_MODEL_QUANTIZED_ONNX_HPP
#define RIN_DRIVER_NPU_MODEL_QUANTIZED_ONNX_HPP
#include <float.h>
#include <rin/driver/npu_model_onnx.hpp>
#include <rin/driver/npu_model_quantized.hpp>
#include <stdint.h>

#define RIN_NPU_QUANTIZED_ONNX_MAX_WEIGHT_BYTES UINT64_C(16777216)

/* Allocation-free ONNX importer for fixed-shape Q8 operators, affine
 * QuantizeLinear/DequantizeLinear graphs, and float32 pooling. Unsupported
 * cases are rejected; all caller storage outlives the model. */
namespace rin {
namespace npu {
namespace model {
namespace onnx {
namespace quantized {
namespace q = ::rin::npu::model::quantized;

struct ConstantBinding {
  uint32_t tensor_index;
  uint32_t reserved;
  uint64_t byte_offset;
  uint64_t element_count;
};
struct ValueBinding {
  uint32_t tensor_index;
  uint32_t name_bytes;
  uint32_t flags;
  uint32_t reserved;
  char name[RIN_NPU_ONNX_MAX_NAME_BYTES + 1u];
};
enum ValueFlags {
  VALUE_HAS_DESCRIPTION = 1u,
  VALUE_EXTERNAL_INPUT = 2u,
  VALUE_INITIALIZER = 4u,
  VALUE_AVAILABLE = 8u,
  VALUE_PRODUCED = 16u,
  VALUE_QUANTIZATION_SET = 32u
};
struct Workspace {
  q::TensorDescription *tensors;
  uint32_t tensor_capacity;
  uint32_t *graph_inputs;
  uint32_t graph_input_capacity;
  uint32_t *external_inputs;
  uint32_t external_input_capacity;
  uint32_t *graph_outputs;
  uint32_t graph_output_capacity;
  q::Operation *operations;
  uint32_t operation_capacity;
  ValueBinding *values;
  uint32_t value_capacity;
  ConstantBinding *constants;
  uint32_t constant_capacity;
  uint8_t *weight_storage;
  uint64_t weight_capacity_bytes;
  q::AxisQuantization *quantizations;
  uint32_t quantization_capacity;
  float *quantization_scales;
  int32_t *quantization_zero_points;
  uint32_t quantization_parameter_capacity;
  q::ConvolutionParameters *convolutions;
  uint32_t convolution_capacity;
  int32_t *convolution_bias_storage;
  uint32_t convolution_bias_capacity;
  q::BatchQuantization *batch_quantizations;
  uint32_t batch_quantization_capacity;
  q::IntegerMatMulParameters *integer_matmuls;
  uint32_t integer_matmul_capacity;
  q::IntegerConvolutionParameters *integer_convolutions;
  uint32_t integer_convolution_capacity;
  q::AveragePoolParameters *average_pools;
  uint32_t average_pool_capacity;
};
struct Result {
  q::Model graph;
  uint32_t external_input_count;
  const uint32_t *external_input_indices;
  uint32_t constant_count;
  const ConstantBinding *constants;
  uint8_t *constant_values;
  uint64_t constant_value_bytes;
};
struct TensorHeader {
  Name name;
  uint32_t onnx_data_type;
  uint32_t rank;
  uint32_t dimensions[RIN_NPU_MODEL_MAX_RANK];
};
struct QNode {
  Name inputs[9];
  Name outputs[3];
  uint32_t input_count;
  uint32_t output_count;
  uint32_t opcode;
  int32_t axis;
  uint32_t has_axis;
  uint32_t block_size;
  uint32_t has_block_size;
  uint32_t group;
  uint32_t has_group;
  uint32_t auto_pad;
  uint32_t has_auto_pad;
  uint32_t strides[RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS];
  uint32_t stride_count;
  uint32_t has_strides;
  uint32_t dilations[RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS];
  uint32_t dilation_count;
  uint32_t has_dilations;
  uint32_t pads[RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS * 2u];
  uint32_t pad_count;
  uint32_t has_pads;
  uint32_t kernel_shape[RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS];
  uint32_t kernel_shape_count;
  uint32_t has_kernel_shape;
  uint32_t ceil_mode;
  uint32_t has_ceil_mode;
  uint32_t count_include_pad;
  uint32_t has_count_include_pad;
  uint32_t storage_order;
  uint32_t has_storage_order;
  uint32_t global_average_pool;
};
struct QuantizationValues {
  float *scales;
  int32_t *zero_points;
  float scalar_scale;
  int32_t scalar_zero_point;
  uint32_t count;
  uint32_t data_type;
  uint32_t rank;
  uint32_t dimensions[RIN_NPU_MODEL_MAX_RANK];
};

static inline float quantization_scale(const QuantizationValues &values,
                                       uint32_t index) {
  return values.scales ? values.scales[index] : values.scalar_scale;
}
static inline int32_t quantization_zero_point(const QuantizationValues &values,
                                              uint32_t index) {
  return values.zero_points ? values.zero_points[index]
                            : values.scalar_zero_point;
}

static inline RinResultCode tensor_header(Slice bytes, TensorHeader *out) {
  if (!out)
    return RIN_RESULT_INVALID_ARGUMENT;
  TensorHeader h = {};
  uint32_t ht = 0, hn = 0, has_data_location = 0;
  Reader r = make_reader(bytes);
  Field f;
  int n = 0;
  while ((n = next_field(&r, &f)) > 0) {
    if (f.number == 1u) {
      if (f.wire_type == 0u) {
        if (h.rank >= RIN_NPU_MODEL_MAX_RANK || !f.integer ||
            f.integer > UINT32_MAX)
          return RIN_RESULT_NOT_SUPPORTED;
        h.dimensions[h.rank++] = (uint32_t)f.integer;
      } else if (f.wire_type == 2u) {
        Reader p = make_reader(f.bytes);
        while (p.offset < p.size) {
          uint64_t d = 0;
          if (!read_varint(&p, &d))
            return RIN_RESULT_CORRUPT_DATA;
          if (h.rank >= RIN_NPU_MODEL_MAX_RANK || !d || d > UINT32_MAX)
            return RIN_RESULT_NOT_SUPPORTED;
          h.dimensions[h.rank++] = (uint32_t)d;
        }
      } else
        return RIN_RESULT_CORRUPT_DATA;
    } else if (f.number == 2u) {
      if (f.wire_type != 0u || ht || f.integer > UINT32_MAX)
        return RIN_RESULT_CORRUPT_DATA;
      h.onnx_data_type = (uint32_t)f.integer;
      ht = 1;
    } else if (f.number == 8u) {
      if (hn || !field_name(f, &h.name))
        return RIN_RESULT_CORRUPT_DATA;
      hn = 1;
    } else if (f.number == 3u || f.number == 13u)
      return RIN_RESULT_NOT_SUPPORTED;
    else if (f.number == 14u) {
      if (f.wire_type != 0u || has_data_location || f.integer != 0u)
        return RIN_RESULT_NOT_SUPPORTED;
      has_data_location = 1u;
    }
  }
  if (n < 0)
    return RIN_RESULT_CORRUPT_DATA;
  if (!ht || !hn)
    return RIN_RESULT_CORRUPT_DATA;
  *out = h;
  return RIN_RESULT_OK;
}
static inline int find_initializer(const GraphFields &g, Name name,
                                   TensorHeader *out) {
  int found = -1;
  for (uint32_t i = 0; i < g.initializer_count; ++i) {
    TensorHeader h = {};
    if (tensor_header(g.initializers[i], &h) != RIN_RESULT_OK)
      return -2;
    if (names_equal(h.name, name)) {
      if (found >= 0)
        return -2;
      found = (int)i;
      if (out)
        *out = h;
    }
  }
  return found;
}
static inline bool onnx_shape_equal(const q::TensorDescription &a,
                                    const q::TensorDescription &b) {
  if (a.data_type != b.data_type || a.rank != b.rank)
    return false;
  for (uint32_t i = 0; i < a.rank; ++i)
    if (a.dimensions[i] != b.dimensions[i])
      return false;
  return true;
}
static inline RinResultCode parse_shape(Slice bytes,
                                        q::TensorDescription *out) {
  if (!out)
    return RIN_RESULT_INVALID_ARGUMENT;
  q::TensorDescription d = *out;
  d.rank = 0u;
  for (uint32_t i = 0u; i < RIN_NPU_MODEL_MAX_RANK; ++i)
    d.dimensions[i] = 0u;
  uint32_t rank = 0;
  Reader r = make_reader(bytes);
  Field f;
  int n = 0;
  while ((n = next_field(&r, &f)) > 0) {
    if (f.number != 1u)
      continue;
    if (f.wire_type != 2u)
      return RIN_RESULT_CORRUPT_DATA;
    Reader dr = make_reader(f.bytes);
    Field df;
    int dn = 0;
    uint64_t dim = 0;
    uint32_t hv = 0, hs = 0;
    while ((dn = next_field(&dr, &df)) > 0) {
      if (df.number == 1u) {
        if (df.wire_type != 0u || hv)
          return RIN_RESULT_CORRUPT_DATA;
        dim = df.integer;
        hv = 1;
      } else if (df.number == 2u) {
        if (df.wire_type != 2u || hs)
          return RIN_RESULT_CORRUPT_DATA;
        hs = 1;
      }
    }
    if (dn < 0)
      return RIN_RESULT_CORRUPT_DATA;
    if (!hv || hs || !dim || dim > UINT32_MAX || rank >= RIN_NPU_MODEL_MAX_RANK)
      return RIN_RESULT_NOT_SUPPORTED;
    d.dimensions[rank++] = (uint32_t)dim;
  }
  if (n < 0)
    return RIN_RESULT_CORRUPT_DATA;
  d.rank = rank;
  *out = d;
  return RIN_RESULT_OK;
}
static inline RinResultCode value_info(Slice bytes, Name *name_out,
                                       q::TensorDescription *desc_out) {
  if (!name_out || !desc_out)
    return RIN_RESULT_INVALID_ARGUMENT;
  Name name = {};
  Slice tp = {};
  uint32_t hn = 0, ht = 0;
  Reader r = make_reader(bytes);
  Field f;
  int n = 0;
  while ((n = next_field(&r, &f)) > 0) {
    if (f.number == 1u) {
      if (hn || !field_name(f, &name))
        return RIN_RESULT_CORRUPT_DATA;
      hn = 1;
    } else if (f.number == 2u) {
      if (f.wire_type != 2u || ht)
        return RIN_RESULT_CORRUPT_DATA;
      tp = f.bytes;
      ht = 1;
    }
  }
  if (n < 0)
    return RIN_RESULT_CORRUPT_DATA;
  if (!hn || !ht)
    return RIN_RESULT_CORRUPT_DATA;
  Slice tt = {};
  uint32_t hten = 0;
  Reader tr = make_reader(tp);
  Field tf;
  int tn = 0;
  while ((tn = next_field(&tr, &tf)) > 0) {
    if (tf.number == 1u) {
      if (tf.wire_type != 2u || hten)
        return RIN_RESULT_CORRUPT_DATA;
      tt = tf.bytes;
      hten = 1;
    } else if (tf.number == 6u) {
      if (tf.wire_type != 2u)
        return RIN_RESULT_CORRUPT_DATA;
    } else if ((tf.number >= 2u && tf.number <= 5u) || tf.number == 7u ||
               tf.number == 8u || tf.number == 9u)
      return RIN_RESULT_NOT_SUPPORTED;
  }
  if (tn < 0)
    return RIN_RESULT_CORRUPT_DATA;
  if (!hten)
    return RIN_RESULT_NOT_SUPPORTED;
  uint32_t type = 0, htype = 0, hshape = 0;
  Slice shape = {};
  Reader er = make_reader(tt);
  Field ef;
  int en = 0;
  while ((en = next_field(&er, &ef)) > 0) {
    if (ef.number == 1u) {
      if (ef.wire_type != 0u || htype || ef.integer > UINT32_MAX)
        return RIN_RESULT_CORRUPT_DATA;
      type = (uint32_t)ef.integer;
      htype = 1;
    } else if (ef.number == 2u) {
      if (ef.wire_type != 2u || hshape)
        return RIN_RESULT_CORRUPT_DATA;
      shape = ef.bytes;
      hshape = 1;
    }
  }
  if (en < 0)
    return RIN_RESULT_CORRUPT_DATA;
  if (!htype || !hshape ||
      (type != 1u && type != 2u && type != 3u && type != 6u && type != 7u))
    return RIN_RESULT_NOT_SUPPORTED;
  q::TensorDescription d = {};
  d.data_type = type == 1u ? q::DATA_TYPE_FLOAT32
      : type == 3u ? q::DATA_TYPE_INT8
      : type == 6u ? q::DATA_TYPE_INT32
      : type == 7u ? q::DATA_TYPE_INT64 : q::DATA_TYPE_UINT8;
  d.scale = 1.0f;
  RinResultCode rc = parse_shape(shape, &d);
  if (rc != RIN_RESULT_OK)
    return rc;
  if (!q::tensor_elements(d))
    return RIN_RESULT_NOT_SUPPORTED;
  *name_out = name;
  *desc_out = d;
  return RIN_RESULT_OK;
}
static inline q::TensorDescription make_description(const TensorHeader &h,
                                                    float scale, int32_t zero) {
  q::TensorDescription d = {};
  d.data_type = h.onnx_data_type == 6u ? q::DATA_TYPE_INT32
      : h.onnx_data_type == 3u ? q::DATA_TYPE_INT8 : q::DATA_TYPE_UINT8;
  d.rank = h.rank;
  d.scale = scale;
  d.zero_point = zero;
  for (uint32_t i = 0; i < h.rank; ++i)
    d.dimensions[i] = h.dimensions[i];
  return d;
}
static inline RinResultCode q_payload(Slice tensor, const TensorHeader &h,
                                      uint8_t *dst, uint64_t count) {
  if ((count && !dst) || count > RIN_NPU_MODEL_MAX_TENSOR_ELEMENTS ||
      (h.onnx_data_type != 2u && h.onnx_data_type != 3u))
    return RIN_RESULT_INVALID_ARGUMENT;
  Reader r = make_reader(tensor);
  uint32_t mode = 0;
  uint64_t written = 0;
  Field f;
  int n = 0;
  while ((n = next_field(&r, &f)) > 0) {
    if (f.number == 9u) {
      if (f.wire_type != 2u || mode || f.bytes.size != count)
        return RIN_RESULT_CORRUPT_DATA;
      for (uint64_t i = 0; i < count; ++i)
        dst[i] = f.bytes.bytes[i];
      written = count;
      mode = 1;
    } else if (f.number == 5u) {
      if (mode == 1u || (f.wire_type != 0u && f.wire_type != 2u))
        return RIN_RESULT_CORRUPT_DATA;
      mode = 2;
      Reader p = f.wire_type == 2u ? make_reader(f.bytes) : Reader{};
      uint64_t v = f.integer;
      do {
        if (f.wire_type == 2u && !read_varint(&p, &v))
          return RIN_RESULT_CORRUPT_DATA;
        if (written >= count)
          return RIN_RESULT_CORRUPT_DATA;
        int64_t s = 0;
        if (v <= INT32_MAX)
          s = (int64_t)v;
        else if (v >= UINT64_MAX - (uint64_t)INT32_MAX)
          s = -(int64_t)(~v + 1u);
        else
          return RIN_RESULT_CORRUPT_DATA;
        if (h.onnx_data_type == 2u) {
          if (s < 0 || s > UINT8_MAX)
            return RIN_RESULT_CORRUPT_DATA;
        } else if (s < INT8_MIN || s > INT8_MAX)
          return RIN_RESULT_CORRUPT_DATA;
        dst[written++] = (uint8_t)s;
      } while (f.wire_type == 2u && p.offset < p.size);
    } else if (f.number == 3u || f.number == 4u || f.number == 6u ||
               f.number == 7u || f.number == 10u || f.number == 11u ||
               f.number == 12u || f.number == 13u) {
      return RIN_RESULT_NOT_SUPPORTED;
    }
  }
  if (n < 0)
    return RIN_RESULT_CORRUPT_DATA;
  return mode && written == count ? RIN_RESULT_OK : RIN_RESULT_CORRUPT_DATA;
}

static inline RinResultCode q_bias_payload(Slice tensor,
                                           const TensorHeader &header,
                                           int32_t *destination,
                                           uint32_t count) {
  if (header.onnx_data_type != 6u || header.rank != 1u ||
      header.dimensions[0] != count || !count || !destination ||
      count > RIN_NPU_MODEL_MAX_TENSOR_ELEMENTS)
    return RIN_RESULT_NOT_SUPPORTED;
  uint32_t mode = 0u;
  uint32_t written = 0u;
  Reader reader = make_reader(tensor);
  Field field;
  int next = 0;
  while ((next = next_field(&reader, &field)) > 0) {
    if (field.number == 9u) {
      if (field.wire_type != 2u || mode ||
          field.bytes.size != (uint64_t)count * 4u)
        return RIN_RESULT_CORRUPT_DATA;
      for (uint32_t index = 0u; index < count; ++index)
        destination[index] = (int32_t)read_u32_le(
            field.bytes.bytes + (uint64_t)index * 4u);
      written = count;
      mode = 1u;
    } else if (field.number == 7u) {
      if (mode == 1u ||
          (field.wire_type != 0u && field.wire_type != 2u))
        return RIN_RESULT_CORRUPT_DATA;
      mode = 2u;
      Reader packed = field.wire_type == 2u ? make_reader(field.bytes)
                                            : Reader{};
      uint64_t encoded = field.integer;
      do {
        if (field.wire_type == 2u && !read_varint(&packed, &encoded))
          return RIN_RESULT_CORRUPT_DATA;
        if (written >= count ||
            (encoded > INT32_MAX &&
             encoded < UINT64_MAX - (uint64_t)INT32_MAX))
          return RIN_RESULT_CORRUPT_DATA;
        const int64_t value = encoded <= INT32_MAX
            ? (int64_t)encoded : -(int64_t)(~encoded + 1u);
        if (value < INT32_MIN || value > INT32_MAX)
          return RIN_RESULT_CORRUPT_DATA;
        destination[written++] = (int32_t)value;
      } while (field.wire_type == 2u && packed.offset < packed.size);
    } else if (field.number == 3u || field.number == 4u ||
               field.number == 5u || field.number == 6u ||
               field.number == 10u || field.number == 11u ||
               field.number == 12u || field.number == 13u) {
      return RIN_RESULT_NOT_SUPPORTED;
    }
  }
  if (next < 0)
    return RIN_RESULT_CORRUPT_DATA;
  return mode && written == count ? RIN_RESULT_OK
                                  : RIN_RESULT_CORRUPT_DATA;
}
static inline float decode_float32(const uint8_t *p) {
  union U {
    uint32_t u;
    float f;
  } v;
  v.u = read_u32_le(p);
  return v.f;
}
static inline RinResultCode scalar_scale(Slice bytes, const TensorHeader &h,
                                         float *out) {
  if (!out || h.onnx_data_type != 1u || h.rank > 1u ||
      (h.rank == 1u && h.dimensions[0] != 1u))
    return RIN_RESULT_NOT_SUPPORTED;
  Reader r = make_reader(bytes);
  uint32_t mode = 0;
  float scale = 0;
  Field f;
  int n = 0;
  while ((n = next_field(&r, &f)) > 0) {
    if (f.number == 9u) {
      if (f.wire_type != 2u || f.bytes.size != 4u || mode)
        return RIN_RESULT_CORRUPT_DATA;
      scale = decode_float32(f.bytes.bytes);
      mode = 1;
    } else if (f.number == 4u) {
      if (mode ||
          (f.wire_type != 5u && !(f.wire_type == 2u && f.bytes.size == 4u)))
        return RIN_RESULT_CORRUPT_DATA;
      scale = decode_float32(f.bytes.bytes);
      mode = 2;
    } else if (f.number == 3u || f.number == 5u || f.number == 6u ||
               f.number == 7u || f.number == 10u || f.number == 11u ||
               f.number == 12u || f.number == 13u) {
      return RIN_RESULT_NOT_SUPPORTED;
    }
  }
  if (n < 0)
    return RIN_RESULT_CORRUPT_DATA;
  if (!mode || !(scale > 0.0f) || !(scale <= FLT_MAX))
    return RIN_RESULT_NOT_SUPPORTED;
  *out = scale;
  return RIN_RESULT_OK;
}
static inline RinResultCode scale_values(Slice bytes, const TensorHeader &h,
                                         float *out, uint32_t count) {
  uint64_t element_count = 1u;
  for (uint32_t dimension = 0u; dimension < h.rank; ++dimension) {
    if (!h.dimensions[dimension] ||
        element_count > UINT32_MAX / h.dimensions[dimension])
      return RIN_RESULT_NOT_SUPPORTED;
    element_count *= h.dimensions[dimension];
  }
  if ((count && !out) || h.onnx_data_type != 1u ||
      count != element_count)
    return RIN_RESULT_NOT_SUPPORTED;
  Reader r = make_reader(bytes);
  uint32_t mode = 0u;
  uint32_t written = 0u;
  Field f;
  int n = 0;
  while ((n = next_field(&r, &f)) > 0) {
    if (f.number == 9u) {
      if (mode || f.wire_type != 2u ||
          f.bytes.size != (uint64_t)count * sizeof(float))
        return RIN_RESULT_CORRUPT_DATA;
      mode = 1u;
      for (uint32_t index = 0u; index < count; ++index) {
        out[index] = decode_float32(f.bytes.bytes + (uint64_t)index * 4u);
        if (!(out[index] > 0.0f) || !(out[index] <= FLT_MAX))
          return RIN_RESULT_NOT_SUPPORTED;
      }
      written = count;
    } else if (f.number == 4u) {
      if (mode == 1u ||
          (f.wire_type != 5u && f.wire_type != 2u) ||
          (f.wire_type == 5u && f.bytes.size != 4u) ||
          (f.wire_type == 2u && (f.bytes.size == 0u ||
                                  (f.bytes.size & 3u) != 0u)))
        return RIN_RESULT_CORRUPT_DATA;
      mode = 2u;
      const uint32_t values = f.wire_type == 5u ? 1u
          : (uint32_t)(f.bytes.size / 4u);
      if (values > count - written) return RIN_RESULT_CORRUPT_DATA;
      for (uint32_t index = 0u; index < values; ++index) {
        out[written] = decode_float32(f.bytes.bytes + (uint64_t)index * 4u);
        if (!(out[written] > 0.0f) || !(out[written] <= FLT_MAX))
          return RIN_RESULT_NOT_SUPPORTED;
        ++written;
      }
    } else if (f.number == 3u || f.number == 5u || f.number == 6u ||
               f.number == 7u || f.number == 10u || f.number == 11u ||
               f.number == 12u || f.number == 13u) {
      return RIN_RESULT_NOT_SUPPORTED;
    }
  }
  if (n < 0) return RIN_RESULT_CORRUPT_DATA;
  return mode && written == count ? RIN_RESULT_OK
                                  : RIN_RESULT_CORRUPT_DATA;
}

static inline RinResultCode float_payload(Slice tensor,
                                          const TensorHeader &header,
                                          void *destination,
                                          uint32_t count) {
  if (header.onnx_data_type != 1u || !destination || !count ||
      count > RIN_NPU_MODEL_MAX_TENSOR_ELEMENTS)
    return RIN_RESULT_NOT_SUPPORTED;
  uint64_t expected_count = 1u;
  for (uint32_t dimension = 0u; dimension < header.rank; ++dimension) {
    if (!header.dimensions[dimension] ||
        expected_count > UINT32_MAX / header.dimensions[dimension])
      return RIN_RESULT_NOT_SUPPORTED;
    expected_count *= header.dimensions[dimension];
  }
  if (expected_count != count)
    return RIN_RESULT_CORRUPT_DATA;
  uint32_t mode = 0u;
  uint32_t written = 0u;
  Reader reader = make_reader(tensor);
  Field field;
  int next = 0;
  while ((next = next_field(&reader, &field)) > 0) {
    if (field.number == 9u) {
      if (mode || field.wire_type != 2u ||
          field.bytes.size != (uint64_t)count * sizeof(float))
        return RIN_RESULT_CORRUPT_DATA;
      mode = 1u;
      for (uint32_t index = 0u; index < count; ++index) {
        const float value = decode_float32(
            field.bytes.bytes + (uint64_t)index * sizeof(float));
        memcpy(static_cast<uint8_t *>(destination) +
                   (uint64_t)index * sizeof(value),
               &value, sizeof(value));
      }
      written = count;
    } else if (field.number == 4u) {
      if (mode == 1u ||
          (field.wire_type != 5u && field.wire_type != 2u) ||
          (field.wire_type == 5u && field.bytes.size != sizeof(float)) ||
          (field.wire_type == 2u &&
           (field.bytes.size == 0u ||
            field.bytes.size % sizeof(float) != 0u)))
        return RIN_RESULT_CORRUPT_DATA;
      mode = 2u;
      const uint32_t values = field.wire_type == 5u ? 1u
          : (uint32_t)(field.bytes.size / sizeof(float));
      if (values > count - written)
        return RIN_RESULT_CORRUPT_DATA;
      for (uint32_t index = 0u; index < values; ++index) {
        const uint8_t *encoded = field.wire_type == 5u
            ? field.bytes.bytes
            : field.bytes.bytes + (uint64_t)index * sizeof(float);
        const float value = decode_float32(encoded);
        memcpy(static_cast<uint8_t *>(destination) +
                   (uint64_t)written * sizeof(value),
               &value, sizeof(value));
        ++written;
      }
    } else if (field.number == 3u || field.number == 5u ||
               field.number == 6u || field.number == 7u ||
               field.number == 10u || field.number == 11u ||
               field.number == 12u || field.number == 13u) {
      return RIN_RESULT_NOT_SUPPORTED;
    }
  }
  if (next < 0)
    return RIN_RESULT_CORRUPT_DATA;
  return mode && written == count ? RIN_RESULT_OK
                                  : RIN_RESULT_CORRUPT_DATA;
}

static inline RinResultCode zero_point_values(Slice tensor,
                                              const TensorHeader &h,
                                              int32_t *out,
                                              uint32_t count) {
  uint64_t element_count = 1u;
  for (uint32_t dimension = 0u; dimension < h.rank; ++dimension) {
    if (!h.dimensions[dimension] ||
        element_count > UINT32_MAX / h.dimensions[dimension])
      return RIN_RESULT_NOT_SUPPORTED;
    element_count *= h.dimensions[dimension];
  }
  if ((count && !out) || count != element_count ||
      (h.onnx_data_type != 2u && h.onnx_data_type != 3u))
    return RIN_RESULT_NOT_SUPPORTED;
  Reader r = make_reader(tensor);
  uint32_t mode = 0u;
  uint32_t written = 0u;
  Field f;
  int n = 0;
  while ((n = next_field(&r, &f)) > 0) {
    if (f.number == 9u) {
      if (mode || f.wire_type != 2u || f.bytes.size != count)
        return RIN_RESULT_CORRUPT_DATA;
      mode = 1u;
      for (uint32_t index = 0u; index < count; ++index) {
        const uint8_t encoded = f.bytes.bytes[index];
        out[index] = h.onnx_data_type == 3u
            ? (int32_t)(int8_t)encoded : (int32_t)encoded;
      }
      written = count;
    } else if (f.number == 5u) {
      if (mode == 1u || (f.wire_type != 0u && f.wire_type != 2u))
        return RIN_RESULT_CORRUPT_DATA;
      mode = 2u;
      Reader packed = f.wire_type == 2u ? make_reader(f.bytes) : Reader{};
      do {
        uint64_t encoded = f.integer;
        if (f.wire_type == 2u && !read_varint(&packed, &encoded))
          return RIN_RESULT_CORRUPT_DATA;
        if (written >= count) return RIN_RESULT_CORRUPT_DATA;
        int64_t value = 0;
        if (encoded <= INT32_MAX)
          value = (int64_t)encoded;
        else if (encoded >= UINT64_MAX - (uint64_t)INT32_MAX)
          value = -(int64_t)(~encoded + 1u);
        else
          return RIN_RESULT_CORRUPT_DATA;
        if (h.onnx_data_type == 3u) {
          if (value < INT8_MIN || value > INT8_MAX)
            return RIN_RESULT_CORRUPT_DATA;
        } else if (value < 0 || value > UINT8_MAX)
          return RIN_RESULT_CORRUPT_DATA;
        out[written++] = (int32_t)value;
      } while (f.wire_type == 2u && packed.offset < packed.size);
    } else if (f.number == 3u || f.number == 4u || f.number == 6u ||
               f.number == 7u || f.number == 10u || f.number == 11u ||
               f.number == 12u || f.number == 13u) {
      return RIN_RESULT_NOT_SUPPORTED;
    }
  }
  if (n < 0) return RIN_RESULT_CORRUPT_DATA;
  return mode && written == count ? RIN_RESULT_OK
                                  : RIN_RESULT_CORRUPT_DATA;
}

static inline RinResultCode quantization_values(
    const GraphFields &g, Name scale_name, Name zero_name, Workspace *workspace,
    uint32_t *parameter_count, QuantizationValues *out) {
  if (!workspace || !parameter_count || !out)
    return RIN_RESULT_INVALID_ARGUMENT;
  TensorHeader scale_header = {}, zero_header = {};
  const int scale_index = find_initializer(g, scale_name, &scale_header);
  const int zero_index = find_initializer(g, zero_name, &zero_header);
  if (scale_index < 0 || zero_index < 0)
    return scale_index == -2 || zero_index == -2
        ? RIN_RESULT_CORRUPT_DATA : RIN_RESULT_NOT_SUPPORTED;
  if (scale_header.onnx_data_type != 1u ||
      zero_header.rank != scale_header.rank ||
      (zero_header.onnx_data_type != 2u && zero_header.onnx_data_type != 3u) ||
      scale_header.rank > RIN_NPU_MODEL_MAX_RANK)
    return RIN_RESULT_NOT_SUPPORTED;
  uint64_t total_parameters = 1u;
  for (uint32_t dimension = 0u; dimension < scale_header.rank; ++dimension) {
    if (!scale_header.dimensions[dimension] ||
        zero_header.dimensions[dimension] !=
            scale_header.dimensions[dimension] ||
        total_parameters > RIN_NPU_MODEL_MAX_TENSOR_ELEMENTS /
                              scale_header.dimensions[dimension])
      return RIN_RESULT_NOT_SUPPORTED;
    total_parameters *= scale_header.dimensions[dimension];
  }
  if (!total_parameters || total_parameters > UINT32_MAX)
    return RIN_RESULT_BUFFER_TOO_SMALL;
  const uint32_t count = (uint32_t)total_parameters;
  float *scales = 0;
  int32_t *zero_points = 0;
  float scalar_scale_value = 0.0f;
  int32_t scalar_zero_value = 0;
  const bool storage_available = workspace->quantization_scales &&
      workspace->quantization_zero_points &&
      *parameter_count <= workspace->quantization_parameter_capacity &&
      count <= workspace->quantization_parameter_capacity - *parameter_count;
  if (storage_available) {
    scales = workspace->quantization_scales + *parameter_count;
    zero_points = workspace->quantization_zero_points + *parameter_count;
  } else if (count != 1u) {
    return RIN_RESULT_BUFFER_TOO_SMALL;
  }
  RinResultCode rc = RIN_RESULT_OK;
  if (scales) {
    rc = scale_values(g.initializers[scale_index], scale_header, scales, count);
    if (rc != RIN_RESULT_OK) return rc;
    rc = zero_point_values(g.initializers[zero_index], zero_header,
                           zero_points, count);
    if (rc != RIN_RESULT_OK) return rc;
  } else {
    rc = scale_values(g.initializers[scale_index], scale_header,
                      &scalar_scale_value, 1u);
    if (rc != RIN_RESULT_OK) return rc;
    rc = zero_point_values(g.initializers[zero_index], zero_header,
                           &scalar_zero_value, 1u);
    if (rc != RIN_RESULT_OK) return rc;
  }
  QuantizationValues values = {};
  values.scales = scales;
  values.zero_points = zero_points;
  values.scalar_scale = scalar_scale_value;
  values.scalar_zero_point = scalar_zero_value;
  values.count = count;
  values.data_type = zero_header.onnx_data_type == 3u
      ? q::DATA_TYPE_INT8 : q::DATA_TYPE_UINT8;
  values.rank = scale_header.rank;
  for (uint32_t dimension = 0u; dimension < scale_header.rank; ++dimension)
    values.dimensions[dimension] = scale_header.dimensions[dimension];
  if (scales) *parameter_count += count;
  *out = values;
  return RIN_RESULT_OK;
}
static inline RinResultCode quantization_values_optional_zero(
    const GraphFields &g, Name scale_name, Name zero_name,
    Workspace *workspace, uint32_t *parameter_count,
    QuantizationValues *out) {
  if (zero_name.size)
    return quantization_values(g, scale_name, zero_name, workspace,
                               parameter_count, out);
  if (!workspace || !parameter_count || !out)
    return RIN_RESULT_INVALID_ARGUMENT;
  TensorHeader scale_header = {};
  const int scale_index = find_initializer(g, scale_name, &scale_header);
  if (scale_index < 0)
    return scale_index == -2 ? RIN_RESULT_CORRUPT_DATA
                             : RIN_RESULT_NOT_SUPPORTED;
  if (scale_header.onnx_data_type != 1u ||
      scale_header.rank > RIN_NPU_MODEL_MAX_RANK)
    return RIN_RESULT_NOT_SUPPORTED;
  uint64_t total = 1u;
  for (uint32_t dimension = 0u; dimension < scale_header.rank; ++dimension) {
    if (!scale_header.dimensions[dimension] ||
        total > RIN_NPU_MODEL_MAX_TENSOR_ELEMENTS /
                    scale_header.dimensions[dimension])
      return RIN_RESULT_NOT_SUPPORTED;
    total *= scale_header.dimensions[dimension];
  }
  if (!total || total > UINT32_MAX)
    return RIN_RESULT_BUFFER_TOO_SMALL;
  const uint32_t count = (uint32_t)total;
  float *scales = 0;
  int32_t *zero_points = 0;
  float scalar_scale_value = 0.0f;
  int32_t scalar_zero_value = 0;
  const bool storage_available = workspace->quantization_scales &&
      workspace->quantization_zero_points &&
      *parameter_count <= workspace->quantization_parameter_capacity &&
      count <= workspace->quantization_parameter_capacity - *parameter_count;
  if (storage_available) {
    scales = workspace->quantization_scales + *parameter_count;
    zero_points = workspace->quantization_zero_points + *parameter_count;
    RinResultCode rc = scale_values(g.initializers[scale_index],
                                    scale_header, scales, count);
    if (rc != RIN_RESULT_OK)
      return rc;
    for (uint32_t index = 0u; index < count; ++index)
      zero_points[index] = 0;
  } else {
    if (count != 1u)
      return RIN_RESULT_BUFFER_TOO_SMALL;
    RinResultCode rc = scalar_scale(g.initializers[scale_index],
                                    scale_header, &scalar_scale_value);
    if (rc != RIN_RESULT_OK)
      return rc;
  }
  QuantizationValues values = {};
  values.scales = scales;
  values.zero_points = zero_points;
  values.scalar_scale = scalar_scale_value;
  values.scalar_zero_point = scalar_zero_value;
  values.count = count;
  values.data_type = q::DATA_TYPE_UINT8;
  values.rank = scale_header.rank;
  for (uint32_t dimension = 0u; dimension < scale_header.rank; ++dimension)
    values.dimensions[dimension] = scale_header.dimensions[dimension];
  if (scales)
    *parameter_count += count;
  *out = values;
  return RIN_RESULT_OK;
}
static inline RinResultCode quantization(const GraphFields &g, Name sn, Name zn,
                                         float *scale, int32_t *zero,
                                         uint32_t *dtype) {
  TensorHeader sh = {}, zh = {};
  int si = find_initializer(g, sn, &sh), zi = find_initializer(g, zn, &zh);
  if (si < 0 || zi < 0)
    return si == -2 || zi == -2 ? RIN_RESULT_CORRUPT_DATA
                                : RIN_RESULT_NOT_SUPPORTED;
  if (sh.rank != zh.rank ||
      (sh.rank == 1u && sh.dimensions[0] != zh.dimensions[0]))
    return RIN_RESULT_NOT_SUPPORTED;
  RinResultCode rc = scalar_scale(g.initializers[si], sh, scale);
  if (rc != RIN_RESULT_OK)
    return rc;
  if (zh.rank > 1u || (zh.rank == 1u && zh.dimensions[0] != 1u) ||
      (zh.onnx_data_type != 2u && zh.onnx_data_type != 3u))
    return RIN_RESULT_NOT_SUPPORTED;
  uint8_t encoded = 0;
  rc = q_payload(g.initializers[zi], zh, &encoded, 1u);
  if (rc != RIN_RESULT_OK)
    return rc;
  *zero = zh.onnx_data_type == 3u ? (int32_t)(int8_t)encoded : (int32_t)encoded;
  *dtype = zh.onnx_data_type == 3u ? q::DATA_TYPE_INT8 : q::DATA_TYPE_UINT8;
  return RIN_RESULT_OK;
}
struct QNodeAttribute {
  Name name;
  Slice string_value;
  uint32_t type;
  uint32_t has_type;
  uint32_t integer_value;
  uint32_t has_integer;
  int32_t signed_integer_value;
  uint32_t values[RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS * 2u];
  uint32_t value_count;
};

static inline RinResultCode parse_qconv_attribute(Slice bytes, QNode *node) {
  if (!node)
    return RIN_RESULT_INVALID_ARGUMENT;
  QNodeAttribute attribute = {};
  uint32_t has_name = 0u;
  uint32_t has_string = 0u;
  Reader reader = make_reader(bytes);
  Field field;
  int next = 0;
  while ((next = next_field(&reader, &field)) > 0) {
    if (field.number == 1u) {
      if (has_name || !field_name(field, &attribute.name))
        return RIN_RESULT_CORRUPT_DATA;
      has_name = 1u;
    } else if (field.number == 3u) {
      if (attribute.has_integer || field.wire_type != 0u)
        return RIN_RESULT_NOT_SUPPORTED;
      const int64_t signed_value = field.integer <= INT64_MAX
          ? (int64_t)field.integer
          : -1 - (int64_t)(~field.integer);
      if (signed_value < INT32_MIN || signed_value > INT32_MAX)
        return RIN_RESULT_NOT_SUPPORTED;
      attribute.signed_integer_value = (int32_t)signed_value;
      attribute.integer_value = (uint32_t)signed_value;
      attribute.has_integer = 1u;
    } else if (field.number == 4u) {
      if (has_string || field.wire_type != 2u)
        return RIN_RESULT_CORRUPT_DATA;
      attribute.string_value = field.bytes;
      has_string = 1u;
    } else if (field.number == 8u) {
      if (field.wire_type == 0u) {
        if (field.integer > UINT32_MAX ||
            attribute.value_count >=
                RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS * 2u)
          return RIN_RESULT_NOT_SUPPORTED;
        attribute.values[attribute.value_count++] =
            (uint32_t)field.integer;
      } else if (field.wire_type == 2u) {
        Reader packed = make_reader(field.bytes);
        while (packed.offset < packed.size) {
          uint64_t value = 0u;
          if (!read_varint(&packed, &value) || value > UINT32_MAX ||
              attribute.value_count >=
                  RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS * 2u)
            return RIN_RESULT_NOT_SUPPORTED;
          attribute.values[attribute.value_count++] = (uint32_t)value;
        }
      } else {
        return RIN_RESULT_CORRUPT_DATA;
      }
    } else if (field.number == 20u) {
      if (attribute.has_type || field.wire_type != 0u ||
          field.integer > UINT32_MAX)
        return RIN_RESULT_CORRUPT_DATA;
      attribute.type = (uint32_t)field.integer;
      attribute.has_type = 1u;
    }
  }
  if (next < 0 || !has_name || !attribute.has_type)
    return RIN_RESULT_CORRUPT_DATA;
  if (name_is(attribute.name, "group", 5u)) {
    if (node->has_group || attribute.type != 2u ||
        !attribute.has_integer || attribute.signed_integer_value <= 0 ||
        has_string || attribute.value_count)
      return RIN_RESULT_NOT_SUPPORTED;
    node->group = attribute.integer_value;
    node->has_group = 1u;
    return RIN_RESULT_OK;
  }
  if (name_is(attribute.name, "auto_pad", 8u)) {
    if (node->has_auto_pad || attribute.type != 3u || !has_string ||
        attribute.has_integer || attribute.value_count)
      return RIN_RESULT_NOT_SUPPORTED;
    if (slice_is(attribute.string_value, "NOTSET", 6u))
      node->auto_pad = 0u;
    else if (slice_is(attribute.string_value, "VALID", 5u))
      node->auto_pad = 1u;
    else if (slice_is(attribute.string_value, "SAME_UPPER", 10u))
      node->auto_pad = 2u;
    else if (slice_is(attribute.string_value, "SAME_LOWER", 10u))
      node->auto_pad = 3u;
    else
      return RIN_RESULT_NOT_SUPPORTED;
    node->has_auto_pad = 1u;
    return RIN_RESULT_OK;
  }
  if (name_is(attribute.name, "axis", 4u)) {
    if (node->has_axis || attribute.type != 2u || !attribute.has_integer ||
        has_string || attribute.value_count)
      return RIN_RESULT_NOT_SUPPORTED;
    node->axis = attribute.signed_integer_value;
    node->has_axis = 1u;
    return RIN_RESULT_OK;
  }
  if (name_is(attribute.name, "block_size", 10u)) {
    if (node->has_block_size || attribute.type != 2u ||
        !attribute.has_integer || attribute.signed_integer_value < 0 ||
        (uint64_t)attribute.signed_integer_value > UINT32_MAX ||
        has_string || attribute.value_count)
      return RIN_RESULT_NOT_SUPPORTED;
    node->block_size = (uint32_t)attribute.signed_integer_value;
    node->has_block_size = 1u;
    return RIN_RESULT_OK;
  }
  if (name_is(attribute.name, "ceil_mode", 9u) ||
      name_is(attribute.name, "count_include_pad", 17u)) {
    uint32_t *value = name_is(attribute.name, "ceil_mode", 9u)
        ? &node->ceil_mode : &node->count_include_pad;
    uint32_t *present = name_is(attribute.name, "ceil_mode", 9u)
        ? &node->has_ceil_mode : &node->has_count_include_pad;
    if (*present || attribute.type != 2u || !attribute.has_integer ||
        (attribute.signed_integer_value != 0 &&
         attribute.signed_integer_value != 1) || has_string ||
        attribute.value_count)
      return RIN_RESULT_NOT_SUPPORTED;
    *value = (uint32_t)attribute.signed_integer_value;
    *present = 1u;
    return RIN_RESULT_OK;
  }
  if (name_is(attribute.name, "storage_order", 13u)) {
    if (node->has_storage_order || attribute.type != 2u ||
        !attribute.has_integer ||
        (attribute.signed_integer_value != 0 &&
         attribute.signed_integer_value != 1) || has_string ||
        attribute.value_count)
      return RIN_RESULT_NOT_SUPPORTED;
    node->storage_order = (uint32_t)attribute.signed_integer_value;
    node->has_storage_order = 1u;
    return RIN_RESULT_OK;
  }
  uint32_t *values = 0;
  uint32_t *value_count = 0;
  uint32_t *has_values = 0;
  uint32_t capacity = 0u;
  if (name_is(attribute.name, "strides", 7u)) {
    values = node->strides;
    value_count = &node->stride_count;
    has_values = &node->has_strides;
    capacity = RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS;
  } else if (name_is(attribute.name, "dilations", 9u)) {
    values = node->dilations;
    value_count = &node->dilation_count;
    has_values = &node->has_dilations;
    capacity = RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS;
  } else if (name_is(attribute.name, "pads", 4u)) {
    values = node->pads;
    value_count = &node->pad_count;
    has_values = &node->has_pads;
    capacity = RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS * 2u;
  } else if (name_is(attribute.name, "kernel_shape", 12u)) {
    values = node->kernel_shape;
    value_count = &node->kernel_shape_count;
    has_values = &node->has_kernel_shape;
    capacity = RIN_NPU_QUANTIZED_MODEL_MAX_SPATIAL_DIMS;
  } else {
    return RIN_RESULT_NOT_SUPPORTED;
  }
  if (*has_values || attribute.type != 7u || attribute.has_integer ||
      has_string || attribute.value_count == 0u ||
      attribute.value_count > capacity)
    return RIN_RESULT_NOT_SUPPORTED;
  for (uint32_t index = 0u; index < attribute.value_count; ++index) {
    if (attribute.values[index] == 0u &&
        !name_is(attribute.name, "pads", 4u))
      return RIN_RESULT_NOT_SUPPORTED;
    values[index] = attribute.values[index];
  }
  *value_count = attribute.value_count;
  *has_values = 1u;
  return RIN_RESULT_OK;
}

static inline RinResultCode parse_node(Slice bytes, QNode *out) {
  if (!out)
    return RIN_RESULT_INVALID_ARGUMENT;
  QNode node = {};
  Slice op = {}, domain = {};
  uint32_t hop = 0, hd = 0;
  Reader r = make_reader(bytes);
  Field f;
  int n = 0;
  while ((n = next_field(&r, &f)) > 0) {
    if (f.number == 1u) {
      if (node.input_count >= 9u || f.wire_type != 2u)
        return RIN_RESULT_NOT_SUPPORTED;
      if (f.bytes.size == 0u) {
        node.inputs[node.input_count] = {};
      } else if (!field_name(f, &node.inputs[node.input_count])) {
        return RIN_RESULT_NOT_SUPPORTED;
      }
      ++node.input_count;
    } else if (f.number == 2u) {
      if (node.output_count >= 3u || f.wire_type != 2u)
        return RIN_RESULT_CORRUPT_DATA;
      if (f.bytes.size == 0u) {
        node.outputs[node.output_count] = {};
      } else if (!field_name(f, &node.outputs[node.output_count])) {
        return RIN_RESULT_CORRUPT_DATA;
      }
      ++node.output_count;
    } else if (f.number == 4u) {
      if (f.wire_type != 2u || hop)
        return RIN_RESULT_CORRUPT_DATA;
      op = f.bytes;
      hop = 1;
    } else if (f.number == 5u) {
      if (f.wire_type != 2u)
        return RIN_RESULT_CORRUPT_DATA;
      RinResultCode rc = parse_qconv_attribute(f.bytes, &node);
      if (rc != RIN_RESULT_OK)
        return rc;
    } else if (f.number == 7u) {
      if (f.wire_type != 2u || hd)
        return RIN_RESULT_CORRUPT_DATA;
      domain = f.bytes;
      hd = 1;
    }
  }
  if (n < 0 || !hop || node.output_count == 0u ||
      (hd && domain.size && !slice_is(domain, "ai.onnx", 7u)))
    return RIN_RESULT_CORRUPT_DATA;
  if (node.outputs[0].size == 0u) return RIN_RESULT_CORRUPT_DATA;
  if (slice_is(op, "QLinearMatMul", 13u)) {
    if (node.output_count != 1u || node.input_count != 8u ||
        node.has_group || node.has_auto_pad ||
        node.has_strides || node.has_dilations || node.has_pads ||
        node.has_kernel_shape || node.has_axis || node.has_block_size ||
        node.has_ceil_mode || node.has_count_include_pad)
      return RIN_RESULT_NOT_SUPPORTED;
    for (uint32_t index = 0u; index < 8u; ++index)
      if (node.inputs[index].size == 0u)
        return RIN_RESULT_NOT_SUPPORTED;
    node.opcode = q::OPERATION_MATMUL_Q8;
  } else if (slice_is(op, "MatMulInteger", 13u)) {
    if (node.output_count != 1u || node.input_count < 2u ||
        node.input_count > 4u || node.inputs[0].size == 0u ||
        node.inputs[1].size == 0u || node.has_group || node.has_auto_pad ||
        node.has_strides || node.has_dilations || node.has_pads ||
        node.has_kernel_shape || node.has_axis || node.has_block_size ||
        node.has_ceil_mode || node.has_count_include_pad)
      return RIN_RESULT_NOT_SUPPORTED;
    node.opcode = q::OPERATION_MATMUL_INTEGER;
  } else if (slice_is(op, "ConvInteger", 11u)) {
    if (node.output_count != 1u || node.input_count < 2u ||
        node.input_count > 4u || node.inputs[0].size == 0u ||
        node.inputs[1].size == 0u || node.has_axis || node.has_block_size ||
        node.has_ceil_mode || node.has_count_include_pad)
      return RIN_RESULT_NOT_SUPPORTED;
    node.opcode = q::OPERATION_CONV_INTEGER;
  } else if (slice_is(op, "QLinearConv", 11u)) {
    if (node.output_count != 1u ||
        (node.input_count != 8u && node.input_count != 9u) ||
        node.has_axis || node.has_block_size || node.has_ceil_mode ||
        node.has_count_include_pad)
      return RIN_RESULT_NOT_SUPPORTED;
    for (uint32_t index = 0u; index < 8u; ++index)
      if (node.inputs[index].size == 0u)
        return RIN_RESULT_NOT_SUPPORTED;
    node.opcode = q::OPERATION_CONV_Q8;
  } else if (slice_is(op, "MaxPool", 7u)) {
    if ((node.output_count != 1u && node.output_count != 2u) ||
        node.input_count != 1u ||
        node.inputs[0].size == 0u || !node.has_kernel_shape ||
        node.has_group || node.has_axis || node.has_block_size ||
        node.has_count_include_pad)
      return RIN_RESULT_NOT_SUPPORTED;
    node.opcode = q::OPERATION_MAX_POOL_Q8;
  } else if (slice_is(op, "AveragePool", 11u)) {
    if (node.output_count != 1u || node.input_count != 1u ||
        node.inputs[0].size == 0u || !node.has_kernel_shape ||
        node.has_group || node.has_axis || node.has_block_size)
      return RIN_RESULT_NOT_SUPPORTED;
    node.opcode = q::OPERATION_AVERAGE_POOL_FLOAT32;
  } else if (slice_is(op, "GlobalAveragePool", 17u)) {
    if (node.output_count != 1u || node.input_count != 1u ||
        node.inputs[0].size == 0u || node.has_group || node.has_auto_pad ||
        node.has_strides || node.has_dilations || node.has_pads ||
        node.has_kernel_shape || node.has_axis || node.has_block_size ||
        node.has_ceil_mode || node.has_count_include_pad)
      return RIN_RESULT_NOT_SUPPORTED;
    node.opcode = q::OPERATION_AVERAGE_POOL_FLOAT32;
    node.global_average_pool = 1u;
  } else if (slice_is(op, "GlobalMaxPool", 13u)) {
    if (node.output_count != 1u || node.input_count != 1u ||
        node.inputs[0].size == 0u || node.has_group || node.has_auto_pad ||
        node.has_strides || node.has_dilations || node.has_pads ||
        node.has_kernel_shape || node.has_axis || node.has_block_size ||
        node.has_ceil_mode || node.has_count_include_pad)
      return RIN_RESULT_NOT_SUPPORTED;
    node.opcode = q::OPERATION_GLOBAL_MAX_POOL_FLOAT32;
  } else if (slice_is(op, "Identity", 8u)) {
    if (node.output_count != 1u || node.input_count != 1u ||
        node.inputs[0].size == 0u || node.has_axis || node.has_block_size ||
        node.has_group || node.has_auto_pad || node.has_strides ||
        node.has_dilations || node.has_pads || node.has_kernel_shape ||
        node.has_ceil_mode || node.has_count_include_pad)
      return RIN_RESULT_NOT_SUPPORTED;
    node.opcode = q::OPERATION_IDENTITY_FLOAT32;
  } else if (slice_is(op, "Flatten", 7u)) {
    if (node.output_count != 1u || node.input_count != 1u ||
        node.inputs[0].size == 0u || node.has_block_size || node.has_group ||
        node.has_auto_pad || node.has_strides || node.has_dilations ||
        node.has_pads || node.has_kernel_shape || node.has_ceil_mode ||
        node.has_count_include_pad)
      return RIN_RESULT_NOT_SUPPORTED;
    node.opcode = q::OPERATION_FLATTEN_FLOAT32;
  } else if (slice_is(op, "QuantizeLinear", 14u) ||
             slice_is(op, "DequantizeLinear", 16u)) {
    if (node.output_count != 1u || node.input_count != 3u ||
        node.has_group || node.has_auto_pad ||
        node.has_strides || node.has_dilations || node.has_pads ||
        node.has_kernel_shape || node.inputs[0].size == 0u ||
        node.inputs[1].size == 0u || node.has_ceil_mode ||
        node.has_count_include_pad)
      return RIN_RESULT_NOT_SUPPORTED;
    node.opcode = slice_is(op, "QuantizeLinear", 14u)
        ? q::OPERATION_QUANTIZE_Q8 : q::OPERATION_DEQUANTIZE_Q8;
  } else if (slice_is(op, "DynamicQuantizeLinear", 21u)) {
    if (node.input_count != 1u || node.output_count != 3u ||
        node.inputs[0].size == 0u || node.has_group || node.has_auto_pad ||
        node.has_strides || node.has_dilations || node.has_pads ||
        node.has_kernel_shape || node.has_axis || node.has_block_size ||
        node.has_ceil_mode || node.has_count_include_pad)
      return RIN_RESULT_NOT_SUPPORTED;
    node.opcode = q::OPERATION_DYNAMIC_QUANTIZE_Q8;
  } else {
    return RIN_RESULT_NOT_SUPPORTED;
  }
  if (node.has_storage_order && node.opcode != q::OPERATION_MAX_POOL_Q8)
    return RIN_RESULT_NOT_SUPPORTED;
  if (node.output_count > 1u && node.outputs[1].size == 0u &&
      node.opcode != q::OPERATION_MAX_POOL_Q8)
    return RIN_RESULT_NOT_SUPPORTED;
  *out = node;
  return RIN_RESULT_OK;
}
static inline int find_value(const Workspace *w, uint32_t count, Name name) {
  for (uint32_t i = 0; i < count; ++i) {
    const ValueBinding &v = w->values[i];
    if (v.name_bytes != name.size)
      continue;
    uint32_t eq = 1;
    for (uint32_t b = 0; b < name.size; ++b)
      if ((uint8_t)v.name[b] != name.bytes[b]) {
        eq = 0;
        break;
      }
    if (eq)
      return (int)i;
  }
  return -1;
}

static inline bool dynamic_quantization_values(const Workspace *workspace,
    uint32_t value_count, uint32_t operation_count, Name scale_name,
    Name zero_point_name, uint32_t *quantized_tensor_out,
    uint32_t *scale_tensor_out, uint32_t *zero_point_tensor_out) {
  if (!workspace || !quantized_tensor_out || !scale_tensor_out ||
      !zero_point_tensor_out)
    return false;
  const int scale_index = find_value(workspace, value_count, scale_name);
  const int zero_index = find_value(workspace, value_count, zero_point_name);
  if (scale_index < 0 || zero_index < 0)
    return false;
  for (uint32_t index = 0u; index < operation_count; ++index) {
    const q::Operation &operation = workspace->operations[index];
    if (operation.opcode == q::OPERATION_DYNAMIC_QUANTIZE_Q8 &&
        operation.inputs[1] == (uint32_t)scale_index &&
        operation.reserved == (uint32_t)zero_index) {
      *quantized_tensor_out = operation.output_tensor;
      *scale_tensor_out = (uint32_t)scale_index;
      *zero_point_tensor_out = (uint32_t)zero_index;
      return true;
    }
  }
  return false;
}

static inline RinResultCode add_value(Workspace *w, uint32_t *count, Name name,
                                      const q::TensorDescription *d,
                                      uint32_t flags, uint32_t *out) {
  int found = find_value(w, *count, name);
  if (found >= 0) {
    ValueBinding &v = w->values[found];
    if (v.flags & flags &
        (VALUE_EXTERNAL_INPUT | VALUE_INITIALIZER | VALUE_PRODUCED))
      return RIN_RESULT_CORRUPT_DATA;
    if (d) {
      if ((v.flags & VALUE_HAS_DESCRIPTION) &&
          !onnx_shape_equal(w->tensors[found], *d))
        return RIN_RESULT_CORRUPT_DATA;
      w->tensors[found] = *d;
      v.flags |= VALUE_HAS_DESCRIPTION;
    }
    v.flags |= flags;
    *out = (uint32_t)found;
    return RIN_RESULT_OK;
  }
  if (!name.size || name.size > RIN_NPU_ONNX_MAX_NAME_BYTES)
    return RIN_RESULT_INVALID_ARGUMENT;
  if (*count >= w->value_capacity || *count >= w->tensor_capacity ||
      *count >= RIN_NPU_MODEL_MAX_TENSORS)
    return RIN_RESULT_BUFFER_TOO_SMALL;
  ValueBinding v = {};
  v.tensor_index = *count;
  v.name_bytes = name.size;
  v.flags = flags;
  for (uint32_t i = 0; i < name.size; ++i)
    v.name[i] = (char)name.bytes[i];
  w->values[*count] = v;
  w->tensors[*count] = d ? *d : q::TensorDescription{};
  *out = *count;
  ++*count;
  return RIN_RESULT_OK;
}
static inline RinResultCode apply_quantization(
    Workspace *w, uint32_t *quantizations, uint32_t *batch_quantizations,
    uint32_t tensor_index,
    uint32_t axis, const QuantizationValues &parameters,
    uint32_t block_size = 0u) {
  ValueBinding &value = w->values[tensor_index];
  q::TensorDescription &description = w->tensors[tensor_index];
  if (description.data_type != parameters.data_type || !parameters.count ||
      (!parameters.scales && parameters.count != 1u) ||
      (!parameters.zero_points && parameters.count != 1u))
    return RIN_RESULT_NOT_SUPPORTED;
  const bool blocked = block_size != 0u;
  if (blocked) {
    if (description.rank < 2u || parameters.rank != description.rank ||
        parameters.rank == 0u || axis >= description.rank ||
        !parameters.scales || !parameters.zero_points)
      return RIN_RESULT_NOT_SUPPORTED;
    uint64_t expected_count = 1u;
    for (uint32_t dimension = 0u; dimension < description.rank; ++dimension) {
      uint32_t expected_extent = description.dimensions[dimension];
      if (dimension == axis) {
        expected_extent = expected_extent / block_size +
            (expected_extent % block_size != 0u ? 1u : 0u);
      }
      if (parameters.dimensions[dimension] != expected_extent ||
          !expected_extent || expected_count > UINT32_MAX / expected_extent)
        return RIN_RESULT_NOT_SUPPORTED;
      expected_count *= expected_extent;
    }
    if (expected_count != parameters.count)
      return RIN_RESULT_NOT_SUPPORTED;
  } else if (parameters.rank > description.rank ||
      (parameters.rank > 1u && parameters.count > 1u &&
       (parameters.rank != description.rank || parameters.rank < 3u ||
        axis >= description.rank)))
    return RIN_RESULT_NOT_SUPPORTED;
  const bool batch_parameters = !blocked && parameters.count > 1u &&
      parameters.rank > 1u;
  if (!blocked && parameters.count > 1u && !batch_parameters &&
      (parameters.rank != 1u || description.rank == 0u ||
       axis >= description.rank ||
       parameters.count != description.dimensions[axis]))
    return RIN_RESULT_NOT_SUPPORTED;
  if (batch_parameters) {
    for (uint32_t dimension = 0u; dimension < parameters.rank; ++dimension)
      if (!parameters.dimensions[dimension] ||
          (parameters.dimensions[dimension] != 1u &&
           parameters.dimensions[dimension] != description.dimensions[dimension]))
        return RIN_RESULT_NOT_SUPPORTED;
    const bool rows = axis == description.rank - 2u &&
        parameters.dimensions[description.rank - 2u] ==
            description.dimensions[description.rank - 2u] &&
        parameters.dimensions[description.rank - 1u] == 1u;
    const bool columns = axis == description.rank - 1u &&
        parameters.dimensions[description.rank - 2u] == 1u &&
        parameters.dimensions[description.rank - 1u] ==
            description.dimensions[description.rank - 1u];
    if (!rows && !columns) return RIN_RESULT_NOT_SUPPORTED;
  }
  if (value.flags & VALUE_QUANTIZATION_SET) {
    if (description.scale != quantization_scale(parameters, 0u) ||
        description.zero_point != quantization_zero_point(parameters, 0u))
      return RIN_RESULT_NOT_SUPPORTED;
    if (parameters.count == 1u && !blocked)
      return value.reserved == 0u ? RIN_RESULT_OK : RIN_RESULT_NOT_SUPPORTED;
    if (batch_parameters) {
      const q::BatchQuantization *existing = 0;
      for (uint32_t index = 0u; index < *batch_quantizations; ++index)
        if (w->batch_quantizations[index].tensor_index == tensor_index) {
          existing = &w->batch_quantizations[index];
          break;
        }
      if (!existing || existing->rank != parameters.rank ||
          existing->count != parameters.count)
        return RIN_RESULT_NOT_SUPPORTED;
      for (uint32_t dimension = 0u; dimension < parameters.rank; ++dimension)
        if (existing->dimensions[dimension] != parameters.dimensions[dimension])
          return RIN_RESULT_NOT_SUPPORTED;
      for (uint32_t index = 0u; index < parameters.count; ++index)
        if (existing->scales[index] != quantization_scale(parameters, index) ||
            existing->zero_points[index] !=
                quantization_zero_point(parameters, index))
          return RIN_RESULT_NOT_SUPPORTED;
      return RIN_RESULT_OK;
    }
    if (value.reserved == 0u || value.reserved > *quantizations)
      return RIN_RESULT_NOT_SUPPORTED;
    const q::AxisQuantization &existing =
        w->quantizations[value.reserved - 1u];
    if (existing.tensor_index != tensor_index || existing.axis != axis ||
        existing.count != parameters.count ||
        existing.reserved != block_size)
      return RIN_RESULT_NOT_SUPPORTED;
    for (uint32_t index = 0u; index < parameters.count; ++index)
      if (existing.scales[index] != quantization_scale(parameters, index) ||
          existing.zero_points[index] !=
              quantization_zero_point(parameters, index))
        return RIN_RESULT_NOT_SUPPORTED;
    return RIN_RESULT_OK;
  }
  description.scale = quantization_scale(parameters, 0u);
  description.zero_point = quantization_zero_point(parameters, 0u);
  if (!q::tensor_elements(description)) return RIN_RESULT_NOT_SUPPORTED;
  value.flags |= VALUE_QUANTIZATION_SET;
  value.reserved = 0u;
  if (parameters.count > 1u || blocked) {
    if (batch_parameters) {
      if (*batch_quantizations >= w->batch_quantization_capacity ||
          !w->batch_quantizations)
        return RIN_RESULT_BUFFER_TOO_SMALL;
      q::BatchQuantization quantization = {};
      quantization.tensor_index = tensor_index;
      quantization.rank = parameters.rank;
      quantization.count = parameters.count;
      for (uint32_t dimension = 0u; dimension < parameters.rank; ++dimension)
        quantization.dimensions[dimension] = parameters.dimensions[dimension];
      quantization.scales = parameters.scales;
      quantization.zero_points = parameters.zero_points;
      w->batch_quantizations[(*batch_quantizations)++] = quantization;
    } else {
      if (*quantizations >= w->quantization_capacity || !w->quantizations)
        return RIN_RESULT_BUFFER_TOO_SMALL;
      q::AxisQuantization quantization = {};
      quantization.tensor_index = tensor_index;
      quantization.axis = axis;
      quantization.count = parameters.count;
      quantization.reserved = block_size;
      quantization.scales = parameters.scales;
      quantization.zero_points = parameters.zero_points;
      w->quantizations[*quantizations] = quantization;
      value.reserved = *quantizations + 1u;
      ++*quantizations;
    }
  }
  return RIN_RESULT_OK;
}
static inline RinResultCode ensure_data(Workspace *w, const GraphFields &g,
                                        uint32_t *values, uint32_t *constants,
                                        uint32_t *quantizations,
                                        uint32_t *batch_quantizations, Name name,
                                        const QuantizationValues &parameters,
                                        uint32_t axis_from_end, uint64_t *bytes,
                                        uint32_t *out,
                                        uint32_t block_size = 0u) {
  int index = find_value(w, *values, name);
  if (index >= 0) {
    ValueBinding &v = w->values[index];
    if (!(v.flags & VALUE_AVAILABLE) || !(v.flags & VALUE_HAS_DESCRIPTION))
      return RIN_RESULT_CORRUPT_DATA;
    if ((axis_from_end != UINT32_MAX &&
         (!axis_from_end || axis_from_end > w->tensors[index].rank)))
      return RIN_RESULT_NOT_SUPPORTED;
    const uint32_t axis = axis_from_end == UINT32_MAX
        ? 0u : w->tensors[index].rank - axis_from_end;
    RinResultCode rc = apply_quantization(
        w, quantizations, batch_quantizations, (uint32_t)index, axis,
        parameters, block_size);
    if (rc != RIN_RESULT_OK) return rc;
    *out = (uint32_t)index;
    return RIN_RESULT_OK;
  }
  TensorHeader h = {};
  int init = find_initializer(g, name, &h);
  if (init == -2)
    return RIN_RESULT_CORRUPT_DATA;
  if (init < 0)
    return RIN_RESULT_NOT_SUPPORTED;
  if ((h.onnx_data_type != 2u && h.onnx_data_type != 3u) ||
      (h.onnx_data_type == 3u ? q::DATA_TYPE_INT8 : q::DATA_TYPE_UINT8) !=
          parameters.data_type)
    return RIN_RESULT_NOT_SUPPORTED;
  q::TensorDescription d =
      make_description(h, quantization_scale(parameters, 0u),
                       quantization_zero_point(parameters, 0u));
  uint64_t count = q::tensor_elements(d);
  if (!count ||
      (axis_from_end != UINT32_MAX &&
       (!axis_from_end || axis_from_end > d.rank)))
    return RIN_RESULT_NOT_SUPPORTED;
  const uint32_t axis = axis_from_end == UINT32_MAX
      ? 0u : d.rank - axis_from_end;
  if (parameters.count > 1u && parameters.rank == 1u &&
      (axis >= d.rank || parameters.count != d.dimensions[axis]))
    return RIN_RESULT_NOT_SUPPORTED;
  if (*bytes > w->weight_capacity_bytes ||
      *bytes > RIN_NPU_QUANTIZED_ONNX_MAX_WEIGHT_BYTES ||
      count > w->weight_capacity_bytes - *bytes ||
      count > RIN_NPU_QUANTIZED_ONNX_MAX_WEIGHT_BYTES - *bytes ||
      *constants >= w->constant_capacity)
    return RIN_RESULT_BUFFER_TOO_SMALL;
  uint32_t idx = 0;
  RinResultCode rc = add_value(w, values, name, &d,
                               VALUE_HAS_DESCRIPTION | VALUE_INITIALIZER |
                                   VALUE_AVAILABLE,
                               &idx);
  if (rc != RIN_RESULT_OK)
    return rc;
  rc = apply_quantization(w, quantizations, batch_quantizations, idx, axis,
                          parameters, block_size);
  if (rc != RIN_RESULT_OK)
    return rc;
  rc = q_payload(g.initializers[init], h, w->weight_storage + *bytes, count);
  if (rc != RIN_RESULT_OK)
    return rc;
  ConstantBinding binding = {};
  binding.tensor_index = idx;
  binding.byte_offset = *bytes;
  binding.element_count = count;
  w->constants[(*constants)++] = binding;
  *bytes += count;
  *out = idx;
  return RIN_RESULT_OK;
}

/* Materialize an ONNX int8/uint8 value without assigning affine scale or
 * zero-point metadata. MatMulInteger and ConvInteger consume these bytes
 * directly and use their separate optional zero-point tensor inputs. */
static inline RinResultCode ensure_raw_q8_data(
    Workspace *workspace, const GraphFields &graph, uint32_t *value_count,
    uint32_t *constant_count, Name name, uint64_t *weight_bytes,
    uint32_t *tensor_index_out)
{
  if (!workspace || !value_count || !constant_count || !weight_bytes ||
      !tensor_index_out || !name.size)
    return RIN_RESULT_INVALID_ARGUMENT;
  const int existing = find_value(workspace, *value_count, name);
  if (existing >= 0) {
    const ValueBinding &value = workspace->values[existing];
    if ((value.flags & VALUE_AVAILABLE) &&
        (value.flags & VALUE_HAS_DESCRIPTION)) {
      const uint32_t type = workspace->tensors[existing].data_type;
      if (type != q::DATA_TYPE_INT8 && type != q::DATA_TYPE_UINT8)
        return RIN_RESULT_NOT_SUPPORTED;
      *tensor_index_out = (uint32_t)existing;
      return RIN_RESULT_OK;
    }
  }
  TensorHeader header = {};
  const int initializer = find_initializer(graph, name, &header);
  if (initializer == -2) return RIN_RESULT_CORRUPT_DATA;
  if (initializer < 0 ||
      (header.onnx_data_type != 2u && header.onnx_data_type != 3u))
    return RIN_RESULT_NOT_SUPPORTED;
  const q::TensorDescription description = make_description(header, 1.0f, 0);
  const uint64_t count = q::tensor_elements(description);
  if (!count || *weight_bytes > workspace->weight_capacity_bytes ||
      *weight_bytes > RIN_NPU_QUANTIZED_ONNX_MAX_WEIGHT_BYTES ||
      count > workspace->weight_capacity_bytes - *weight_bytes ||
      count > RIN_NPU_QUANTIZED_ONNX_MAX_WEIGHT_BYTES - *weight_bytes ||
      *constant_count >= workspace->constant_capacity ||
      !workspace->weight_storage || !workspace->constants)
    return RIN_RESULT_BUFFER_TOO_SMALL;
  uint32_t tensor_index = 0u;
  RinResultCode result = add_value(workspace, value_count, name, &description,
      VALUE_HAS_DESCRIPTION | VALUE_INITIALIZER | VALUE_AVAILABLE,
      &tensor_index);
  if (result != RIN_RESULT_OK) return result;
  result = q_payload(graph.initializers[initializer], header,
      workspace->weight_storage + *weight_bytes, count);
  if (result != RIN_RESULT_OK) return result;
  ConstantBinding constant = {};
  constant.tensor_index = tensor_index;
  constant.byte_offset = *weight_bytes;
  constant.element_count = count;
  workspace->constants[(*constant_count)++] = constant;
  *weight_bytes += count;
  *tensor_index_out = tensor_index;
  return RIN_RESULT_OK;
}

static inline RinResultCode ensure_float_data(
    Workspace *w, const GraphFields &g, uint32_t *values,
    uint32_t *constants, Name name, uint64_t *bytes, uint32_t *out) {
  const int existing = find_value(w, *values, name);
  if (existing >= 0) {
    const ValueBinding &value = w->values[existing];
    if (value.flags & VALUE_AVAILABLE) {
      if (!(value.flags & VALUE_HAS_DESCRIPTION) ||
          w->tensors[existing].data_type != q::DATA_TYPE_FLOAT32)
        return RIN_RESULT_NOT_SUPPORTED;
      *out = (uint32_t)existing;
      return RIN_RESULT_OK;
    }
  }
  TensorHeader header = {};
  const int initializer = find_initializer(g, name, &header);
  if (initializer == -2)
    return RIN_RESULT_CORRUPT_DATA;
  if (initializer < 0 || header.onnx_data_type != 1u)
    return RIN_RESULT_NOT_SUPPORTED;
  q::TensorDescription description = {};
  description.data_type = q::DATA_TYPE_FLOAT32;
  description.scale = 1.0f;
  description.rank = header.rank;
  for (uint32_t dimension = 0u; dimension < header.rank; ++dimension)
    description.dimensions[dimension] = header.dimensions[dimension];
  const uint64_t count = q::tensor_elements(description);
  const uint64_t storage_bytes = q::tensor_storage_bytes(description);
  if (!count || !storage_bytes || *bytes > w->weight_capacity_bytes ||
      *bytes > RIN_NPU_QUANTIZED_ONNX_MAX_WEIGHT_BYTES ||
      storage_bytes > w->weight_capacity_bytes - *bytes ||
      storage_bytes > RIN_NPU_QUANTIZED_ONNX_MAX_WEIGHT_BYTES - *bytes ||
      *constants >= w->constant_capacity)
    return RIN_RESULT_BUFFER_TOO_SMALL;
  uint32_t tensor_index = 0u;
  RinResultCode rc = add_value(w, values, name, &description,
      VALUE_HAS_DESCRIPTION | VALUE_INITIALIZER | VALUE_AVAILABLE,
      &tensor_index);
  if (rc != RIN_RESULT_OK)
    return rc;
  rc = float_payload(g.initializers[initializer], header,
      w->weight_storage + *bytes, (uint32_t)count);
  if (rc != RIN_RESULT_OK)
    return rc;
  ConstantBinding binding = {};
  binding.tensor_index = tensor_index;
  binding.byte_offset = *bytes;
  binding.element_count = count;
  w->constants[(*constants)++] = binding;
  *bytes += storage_bytes;
  *out = tensor_index;
  return RIN_RESULT_OK;
}
static inline bool normalize_quantization_axis(int32_t axis, uint32_t rank,
                                               uint32_t *out) {
  if (!out)
    return false;
  if (rank == 0u) {
    if (axis != 0)
      return false;
    *out = 0u;
    return true;
  }
  int64_t normalized = axis;
  if (normalized < 0)
    normalized += rank;
  if (normalized < 0 || (uint64_t)normalized >= rank)
    return false;
  *out = (uint32_t)normalized;
  return true;
}
static inline RinResultCode value_name(Slice bytes, Name *out) {
  if (!out)
    return RIN_RESULT_INVALID_ARGUMENT;
  Reader r = make_reader(bytes);
  Field f;
  int n = 0;
  uint32_t seen = 0;
  while ((n = next_field(&r, &f)) > 0)
    if (f.number == 1u) {
      if (seen || !field_name(f, out))
        return RIN_RESULT_CORRUPT_DATA;
      seen = 1;
    }
  if (n < 0)
    return RIN_RESULT_CORRUPT_DATA;
  return seen ? RIN_RESULT_OK : RIN_RESULT_CORRUPT_DATA;
}

static inline bool graph_has_tensor(const uint32_t *indices, uint32_t count,
                                   uint32_t tensor_index) {
  if (!indices)
    return false;
  for (uint32_t index = 0u; index < count; ++index)
    if (indices[index] == tensor_index)
      return true;
  return false;
}

static inline void remap_operation_sidecars_after_remove(
    Workspace *w, uint32_t removed, uint32_t convolution_count,
    uint32_t integer_matmul_count, uint32_t integer_convolution_count,
    uint32_t average_pool_count) {
  for (uint32_t index = 0u; index < convolution_count; ++index)
    if (w->convolutions[index].operation_index > removed)
      --w->convolutions[index].operation_index;
  for (uint32_t index = 0u; index < integer_matmul_count; ++index)
    if (w->integer_matmuls[index].operation_index > removed)
      --w->integer_matmuls[index].operation_index;
  for (uint32_t index = 0u; index < integer_convolution_count; ++index)
    if (w->integer_convolutions[index].operation_index > removed)
      --w->integer_convolutions[index].operation_index;
  for (uint32_t index = 0u; index < average_pool_count; ++index)
    if (w->average_pools[index].operation_index > removed)
      --w->average_pools[index].operation_index;
}

static inline bool remap_tensor_after_remove(uint32_t tensor_index,
    uint32_t first_removed, uint32_t second_removed, uint32_t *out) {
  if (!out || tensor_index == first_removed || tensor_index == second_removed)
    return false;
  *out = tensor_index - (tensor_index > first_removed ? 1u : 0u) -
         (tensor_index > second_removed ? 1u : 0u);
  return true;
}

static inline void remove_operation_at(Workspace *w,
    uint32_t *operation_count, uint32_t removed,
    uint32_t convolution_count, uint32_t integer_matmul_count,
    uint32_t integer_convolution_count, uint32_t average_pool_count) {
  for (uint32_t index = removed; index + 1u < *operation_count; ++index)
    w->operations[index] = w->operations[index + 1u];
  --*operation_count;
  remap_operation_sidecars_after_remove(w, removed, convolution_count,
      integer_matmul_count, integer_convolution_count, average_pool_count);
}

static inline bool tensor_removal_has_external_reference(
    const Workspace *w, uint32_t tensor_a, uint32_t tensor_b,
    uint32_t graph_input_count, const uint32_t *graph_inputs,
    uint32_t graph_output_count, const uint32_t *graph_outputs,
    uint32_t constant_count, const ConstantBinding *constants,
    uint32_t quantization_count, uint32_t batch_quantization_count,
    uint32_t integer_matmul_count, uint32_t integer_convolution_count,
    uint32_t removed_dequantize, uint32_t removed_pool,
    uint32_t removed_quantize, uint32_t operation_count) {
  if (graph_has_tensor(graph_inputs, graph_input_count, tensor_a) ||
      graph_has_tensor(graph_inputs, graph_input_count, tensor_b) ||
      graph_has_tensor(graph_outputs, graph_output_count, tensor_a) ||
      graph_has_tensor(graph_outputs, graph_output_count, tensor_b))
    return true;
  for (uint32_t index = 0u; index < constant_count; ++index)
    if (constants[index].tensor_index == tensor_a ||
        constants[index].tensor_index == tensor_b)
      return true;
  for (uint32_t index = 0u; index < quantization_count; ++index)
    if (w->quantizations[index].tensor_index == tensor_a ||
        w->quantizations[index].tensor_index == tensor_b)
      return true;
  for (uint32_t index = 0u; index < batch_quantization_count; ++index)
    if (w->batch_quantizations[index].tensor_index == tensor_a ||
        w->batch_quantizations[index].tensor_index == tensor_b)
      return true;
  for (uint32_t index = 0u; index < integer_matmul_count; ++index)
    if (w->integer_matmuls[index].a_zero_point_tensor == tensor_a ||
        w->integer_matmuls[index].a_zero_point_tensor == tensor_b ||
        w->integer_matmuls[index].b_zero_point_tensor == tensor_a ||
        w->integer_matmuls[index].b_zero_point_tensor == tensor_b)
      return true;
  for (uint32_t index = 0u; index < integer_convolution_count; ++index)
    if (w->integer_convolutions[index].input_zero_point_tensor == tensor_a ||
        w->integer_convolutions[index].input_zero_point_tensor == tensor_b ||
        w->integer_convolutions[index].weight_zero_point_tensor == tensor_a ||
        w->integer_convolutions[index].weight_zero_point_tensor == tensor_b)
      return true;
  for (uint32_t index = 0u; index < operation_count; ++index) {
    if (index == removed_dequantize || index == removed_pool ||
        index == removed_quantize)
      continue;
    const q::Operation &operation = w->operations[index];
    if (operation.output_tensor == tensor_a ||
        operation.output_tensor == tensor_b)
      return true;
    for (uint32_t input = 0u; input < operation.input_count; ++input)
      if (operation.inputs[input] == tensor_a ||
          operation.inputs[input] == tensor_b)
        return true;
    if ((operation.opcode == q::OPERATION_DYNAMIC_QUANTIZE_Q8 &&
         (operation.inputs[1] == tensor_a ||
          operation.inputs[1] == tensor_b ||
          operation.reserved == tensor_a ||
          operation.reserved == tensor_b)) ||
        (operation.opcode == q::OPERATION_DEQUANTIZE_Q8 &&
         operation.input_count == 2u &&
         (operation.reserved == tensor_a ||
          operation.reserved == tensor_b)) ||
        (operation.opcode == q::OPERATION_MAX_POOL_Q8 &&
         operation.reserved != 0u &&
         (operation.reserved - 1u == tensor_a ||
          operation.reserved - 1u == tensor_b)))
      return true;
  }
  return false;
}

static inline bool compact_removed_average_pool_tensors(
    Workspace *w, uint32_t *tensor_count, uint32_t operation_count,
    uint32_t graph_input_count,
    uint32_t *graph_inputs, uint32_t external_input_count,
    uint32_t *external_inputs, uint32_t graph_output_count,
    uint32_t *graph_outputs, uint32_t constant_count,
    ConstantBinding *constants, uint32_t quantization_count,
    uint32_t batch_quantization_count, uint32_t integer_matmul_count,
    uint32_t integer_convolution_count, uint32_t first_removed,
    uint32_t second_removed) {
  const uint32_t old_count = *tensor_count;
  if (first_removed >= old_count || second_removed >= old_count ||
      first_removed == second_removed)
    return false;
  uint32_t compacted = 0u;
  for (uint32_t index = 0u; index < old_count; ++index) {
    if (index == first_removed || index == second_removed) continue;
    if (compacted != index) {
      w->tensors[compacted] = w->tensors[index];
      w->values[compacted] = w->values[index];
    }
    w->values[compacted].tensor_index = compacted;
    ++compacted;
  }
  for (uint32_t index = 0u; index < graph_input_count; ++index)
    if (!remap_tensor_after_remove(graph_inputs[index], first_removed,
            second_removed, &graph_inputs[index]))
      return false;
  for (uint32_t index = 0u; index < external_input_count; ++index)
    if (!remap_tensor_after_remove(external_inputs[index], first_removed,
            second_removed, &external_inputs[index]))
      return false;
  for (uint32_t index = 0u; index < graph_output_count; ++index)
    if (!remap_tensor_after_remove(graph_outputs[index], first_removed,
            second_removed, &graph_outputs[index]))
      return false;
  for (uint32_t index = 0u; index < constant_count; ++index)
    if (!remap_tensor_after_remove(constants[index].tensor_index,
            first_removed, second_removed, &constants[index].tensor_index))
      return false;
  for (uint32_t index = 0u; index < quantization_count; ++index)
    if (!remap_tensor_after_remove(w->quantizations[index].tensor_index,
            first_removed, second_removed,
            &w->quantizations[index].tensor_index))
      return false;
  for (uint32_t index = 0u; index < batch_quantization_count; ++index)
    if (!remap_tensor_after_remove(w->batch_quantizations[index].tensor_index,
            first_removed, second_removed,
            &w->batch_quantizations[index].tensor_index))
      return false;
  for (uint32_t index = 0u; index < integer_matmul_count; ++index) {
    q::IntegerMatMulParameters &parameters = w->integer_matmuls[index];
    if ((parameters.a_zero_point_tensor != UINT32_MAX &&
         !remap_tensor_after_remove(parameters.a_zero_point_tensor,
             first_removed, second_removed, &parameters.a_zero_point_tensor)) ||
        (parameters.b_zero_point_tensor != UINT32_MAX &&
         !remap_tensor_after_remove(parameters.b_zero_point_tensor,
             first_removed, second_removed, &parameters.b_zero_point_tensor)))
      return false;
  }
  for (uint32_t index = 0u; index < integer_convolution_count; ++index) {
    q::IntegerConvolutionParameters &parameters =
        w->integer_convolutions[index];
    if ((parameters.input_zero_point_tensor != UINT32_MAX &&
         !remap_tensor_after_remove(parameters.input_zero_point_tensor,
             first_removed, second_removed,
             &parameters.input_zero_point_tensor)) ||
        (parameters.weight_zero_point_tensor != UINT32_MAX &&
         !remap_tensor_after_remove(parameters.weight_zero_point_tensor,
             first_removed, second_removed,
             &parameters.weight_zero_point_tensor)))
      return false;
  }
  for (uint32_t index = 0u; index < operation_count; ++index) {
    q::Operation &operation = w->operations[index];
    for (uint32_t input = 0u; input < operation.input_count; ++input)
      if (!remap_tensor_after_remove(operation.inputs[input], first_removed,
              second_removed, &operation.inputs[input]))
        return false;
    if (!remap_tensor_after_remove(operation.output_tensor, first_removed,
            second_removed, &operation.output_tensor))
      return false;
    if (operation.opcode == q::OPERATION_DYNAMIC_QUANTIZE_Q8) {
      if (!remap_tensor_after_remove(operation.inputs[1], first_removed,
              second_removed, &operation.inputs[1]) ||
          !remap_tensor_after_remove(operation.reserved, first_removed,
              second_removed, &operation.reserved))
        return false;
    } else if (operation.opcode == q::OPERATION_DEQUANTIZE_Q8 &&
               operation.input_count == 2u) {
      if (!remap_tensor_after_remove(operation.reserved, first_removed,
              second_removed, &operation.reserved))
        return false;
    } else if (operation.opcode == q::OPERATION_MAX_POOL_Q8 &&
               operation.reserved != 0u) {
      uint32_t indices_tensor = 0u;
      if (!remap_tensor_after_remove(operation.reserved - 1u,
              first_removed, second_removed, &indices_tensor))
        return false;
      operation.reserved = indices_tensor + 1u;
    }
  }
  *tensor_count = old_count - 2u;
  return true;
}

static inline bool fuse_qdq_average_pool(Workspace *w,
    uint32_t *operation_count, uint32_t *tensor_count,
    uint32_t quantization_count,
    uint32_t batch_quantization_count, uint32_t convolution_count,
    uint32_t integer_matmul_count, uint32_t integer_convolution_count,
    uint32_t average_pool_count, uint32_t graph_input_count,
    uint32_t *graph_inputs, uint32_t external_input_count,
    uint32_t *external_inputs, uint32_t graph_output_count,
    uint32_t *graph_outputs, uint32_t constant_count,
    ConstantBinding *constants) {
  for (uint32_t pool_index = 0u; pool_index < *operation_count;
       ++pool_index) {
    q::Operation &pool = w->operations[pool_index];
    if (pool.opcode != q::OPERATION_AVERAGE_POOL_FLOAT32 ||
        pool.input_count != 1u ||
        graph_has_tensor(graph_outputs, graph_output_count,
                         pool.output_tensor))
      continue;

    uint32_t dequantize_index = UINT32_MAX;
    uint32_t quantize_index = UINT32_MAX;
    uint32_t dequantize_producers = 0u;
    uint32_t quantize_consumers = 0u;
    for (uint32_t candidate = 0u; candidate < *operation_count; ++candidate) {
      const q::Operation &operation = w->operations[candidate];
      if (operation.opcode == q::OPERATION_DEQUANTIZE_Q8 &&
          operation.output_tensor == pool.inputs[0]) {
        dequantize_index = candidate;
        ++dequantize_producers;
      }
      if (operation.opcode == q::OPERATION_QUANTIZE_Q8 &&
          operation.input_count == 1u &&
          operation.inputs[0] == pool.output_tensor) {
        quantize_index = candidate;
        ++quantize_consumers;
      }
    }
    if (dequantize_producers != 1u || quantize_consumers != 1u ||
        dequantize_index >= pool_index || quantize_index <= pool_index ||
        quantize_index >= *operation_count ||
        graph_has_tensor(graph_outputs, graph_output_count,
                         pool.inputs[0]) ||
        graph_has_tensor(graph_outputs, graph_output_count,
                         pool.output_tensor))
      continue;

    const q::Operation dequantize = w->operations[dequantize_index];
    const q::Operation quantize = w->operations[quantize_index];
    if (dequantize.input_count != 1u || quantize.reserved != 0u ||
        dequantize.inputs[0] >= *tensor_count ||
        quantize.output_tensor >= *tensor_count ||
        (w->tensors[dequantize.inputs[0]].data_type != q::DATA_TYPE_INT8 &&
         w->tensors[dequantize.inputs[0]].data_type != q::DATA_TYPE_UINT8) ||
        (w->tensors[quantize.output_tensor].data_type != q::DATA_TYPE_INT8 &&
         w->tensors[quantize.output_tensor].data_type != q::DATA_TYPE_UINT8) ||
        !q::same_shape(w->tensors[dequantize.inputs[0]],
                       w->tensors[pool.inputs[0]]) ||
        !q::same_shape(w->tensors[pool.output_tensor],
                       w->tensors[quantize.output_tensor]))
      continue;

    uint32_t dequantize_output_uses = 0u;
    uint32_t pool_output_uses = 0u;
    for (uint32_t candidate = 0u; candidate < *operation_count; ++candidate) {
      const q::Operation &operation = w->operations[candidate];
      for (uint32_t input = 0u; input < operation.input_count; ++input) {
        if (operation.inputs[input] == dequantize.output_tensor)
          ++dequantize_output_uses;
        if (operation.inputs[input] == pool.output_tensor)
          ++pool_output_uses;
      }
    }
    if (dequantize_output_uses != 1u || pool_output_uses != 1u)
      continue;

    bool spatial_quantization = false;
    for (uint32_t index = 0u; index < quantization_count; ++index) {
      const q::AxisQuantization &axis = w->quantizations[index];
      if ((axis.tensor_index == dequantize.inputs[0] ||
           axis.tensor_index == quantize.output_tensor) && axis.axis >= 2u)
        spatial_quantization = true;
    }
    for (uint32_t index = 0u; index < batch_quantization_count; ++index) {
      const q::BatchQuantization &batch = w->batch_quantizations[index];
      if (batch.tensor_index != dequantize.inputs[0] &&
          batch.tensor_index != quantize.output_tensor)
        continue;
      for (uint32_t axis = 2u; axis < batch.rank; ++axis)
        if (batch.dimensions[axis] != 1u)
          spatial_quantization = true;
    }
    if (spatial_quantization)
      continue;
    for (uint32_t candidate = 0u; candidate < *operation_count; ++candidate)
      if (candidate != dequantize_index &&
          w->operations[candidate].opcode ==
              q::OPERATION_DYNAMIC_QUANTIZE_Q8 &&
          w->operations[candidate].output_tensor == dequantize.inputs[0])
        spatial_quantization = true;
    if (spatial_quantization)
      continue;

    const uint32_t dequantized_tensor = dequantize.output_tensor;
    const uint32_t pooled_tensor = pool.output_tensor;
    const uint32_t output_tensor = quantize.output_tensor;
    if (dequantized_tensor >= *tensor_count || pooled_tensor >= *tensor_count ||
        output_tensor >= *tensor_count ||
        tensor_removal_has_external_reference(w, dequantized_tensor,
            pooled_tensor, graph_input_count, graph_inputs,
            graph_output_count, graph_outputs, constant_count, constants,
            quantization_count, batch_quantization_count,
            integer_matmul_count, integer_convolution_count,
            dequantize_index, pool_index, quantize_index, *operation_count))
      continue;

    q::Operation fused_pool = pool;
    fused_pool.opcode = q::OPERATION_AVERAGE_POOL_Q8;
    fused_pool.inputs[0] = dequantize.inputs[0];
    fused_pool.output_tensor = output_tensor;
    remove_operation_at(w, operation_count, quantize_index,
        convolution_count, integer_matmul_count, integer_convolution_count,
        average_pool_count);
    remove_operation_at(w, operation_count, dequantize_index,
        convolution_count, integer_matmul_count, integer_convolution_count,
        average_pool_count);
    --pool_index;
    w->operations[pool_index] = fused_pool;
    if (!compact_removed_average_pool_tensors(w, tensor_count,
            *operation_count,
            graph_input_count, graph_inputs, external_input_count,
            external_inputs, graph_output_count, graph_outputs, constant_count,
            constants, quantization_count, batch_quantization_count,
            integer_matmul_count, integer_convolution_count,
            dequantized_tensor, pooled_tensor))
      return false;
  }
  return true;
}

static inline RinResultCode import_model(const void *blob, uint32_t blob_bytes,
                                         Workspace *w, Result *out) {
  if (!out)
    return RIN_RESULT_INVALID_ARGUMENT;
  *out = {};
  if (!blob || !blob_bytes || blob_bytes > RIN_NPU_ONNX_MAX_MODEL_BYTES || !w ||
      !w->tensors || !w->graph_inputs || !w->external_inputs ||
      !w->graph_outputs || !w->operations || !w->values || !w->constants ||
      !w->weight_storage)
    return RIN_RESULT_INVALID_ARGUMENT;
  if ((w->quantization_capacity && !w->quantizations) ||
      (w->quantization_parameter_capacity &&
       (!w->quantization_scales || !w->quantization_zero_points)) ||
      (w->convolution_capacity && !w->convolutions) ||
      (w->convolution_bias_capacity && !w->convolution_bias_storage) ||
      (w->batch_quantization_capacity && !w->batch_quantizations) ||
      (w->integer_matmul_capacity && !w->integer_matmuls) ||
      (w->integer_convolution_capacity && !w->integer_convolutions) ||
      (w->average_pool_capacity && !w->average_pools))
    return RIN_RESULT_INVALID_ARGUMENT;
  const void *ptrs[] = {w->tensors, w->graph_inputs, w->external_inputs,
                        w->graph_outputs, w->operations, w->values,
                        w->constants, w->weight_storage, w->quantizations,
                        w->quantization_scales, w->quantization_zero_points,
                        w->convolutions, w->convolution_bias_storage,
                        w->batch_quantizations, w->integer_matmuls,
                        w->integer_convolutions, w->average_pools};
  const uint64_t sizes[] = {
      (uint64_t)w->tensor_capacity * sizeof(*w->tensors),
      (uint64_t)w->graph_input_capacity * sizeof(*w->graph_inputs),
      (uint64_t)w->external_input_capacity * sizeof(*w->external_inputs),
      (uint64_t)w->graph_output_capacity * sizeof(*w->graph_outputs),
      (uint64_t)w->operation_capacity * sizeof(*w->operations),
      (uint64_t)w->value_capacity * sizeof(*w->values),
      (uint64_t)w->constant_capacity * sizeof(*w->constants),
      w->weight_capacity_bytes,
      (uint64_t)w->quantization_capacity * sizeof(*w->quantizations),
      (uint64_t)w->quantization_parameter_capacity *
          sizeof(*w->quantization_scales),
      (uint64_t)w->quantization_parameter_capacity *
          sizeof(*w->quantization_zero_points),
      (uint64_t)w->convolution_capacity * sizeof(*w->convolutions),
      (uint64_t)w->convolution_bias_capacity *
          sizeof(*w->convolution_bias_storage),
      (uint64_t)w->batch_quantization_capacity *
          sizeof(*w->batch_quantizations),
       (uint64_t)w->integer_matmul_capacity * sizeof(*w->integer_matmuls),
       (uint64_t)w->integer_convolution_capacity *
           sizeof(*w->integer_convolutions),
       (uint64_t)w->average_pool_capacity * sizeof(*w->average_pools)};
  for (uint32_t i = 0; i < 17u; ++i) {
    if (sizes[i] &&
        (memory_ranges_overlap(ptrs[i], sizes[i], blob, blob_bytes) ||
         memory_ranges_overlap(ptrs[i], sizes[i], out, sizeof(*out)) ||
         memory_ranges_overlap(ptrs[i], sizes[i], w, sizeof(*w))))
      return RIN_RESULT_INVALID_ARGUMENT;
    for (uint32_t j = 0; j < i; ++j)
      if (memory_ranges_overlap(ptrs[i], sizes[i], ptrs[j], sizes[j]))
        return RIN_RESULT_INVALID_ARGUMENT;
  }
  if (memory_ranges_overlap(blob, blob_bytes, out, sizeof(*out)) ||
      memory_ranges_overlap(blob, blob_bytes, w, sizeof(*w)) ||
      memory_ranges_overlap(out, sizeof(*out), w, sizeof(*w)))
    return RIN_RESULT_INVALID_ARGUMENT;
  Slice graph_bytes = {};
  uint32_t opset = 0;
  RinResultCode rc = parse_model(
      {static_cast<const uint8_t *>(blob), blob_bytes}, &graph_bytes, &opset);
  if (rc != RIN_RESULT_OK)
    return rc;
  if (opset < 10u)
    return RIN_RESULT_NOT_SUPPORTED;
  GraphFields g = {};
  rc = parse_graph(graph_bytes, &g);
  if (rc != RIN_RESULT_OK)
    return rc;
  if (g.output_count > w->graph_output_capacity)
    return RIN_RESULT_BUFFER_TOO_SMALL;
  uint32_t values = 0, externals = 0, constants = 0, operations = 0;
  uint32_t quantizations = 0u, quantization_parameters = 0u;
  uint32_t batch_quantizations = 0u;
  uint32_t integer_matmuls = 0u;
  uint32_t integer_convolutions = 0u;
  uint32_t average_pools = 0u;
  uint32_t has_blocked_quantization = 0u;
  uint32_t convolutions = 0u, convolution_bias_count = 0u;
  uint64_t weight_bytes = 0;
  for (uint32_t i = 0; i < g.input_count; ++i) {
    Name name = {};
    q::TensorDescription d = {};
    rc = value_name(g.inputs[i], &name);
    if (rc != RIN_RESULT_OK)
      return rc;
    int init = find_initializer(g, name, 0);
    if (init == -2)
      return RIN_RESULT_CORRUPT_DATA;
    if (init >= 0)
      continue;
    rc = value_info(g.inputs[i], &name, &d);
    if (rc != RIN_RESULT_OK)
      return rc;
    if (externals >= w->external_input_capacity)
      return RIN_RESULT_BUFFER_TOO_SMALL;
    uint32_t idx = 0;
    rc = add_value(
        w, &values, name, &d,
        VALUE_HAS_DESCRIPTION | VALUE_EXTERNAL_INPUT | VALUE_AVAILABLE, &idx);
    if (rc != RIN_RESULT_OK)
      return rc;
    w->external_inputs[externals++] = idx;
  }
  for (uint32_t i = 0; i < g.value_info_count; ++i) {
    Name name = {};
    q::TensorDescription d = {};
    rc = value_info(g.value_info[i], &name, &d);
    if (rc != RIN_RESULT_OK)
      return rc;
    uint32_t ignored = 0;
    rc = add_value(w, &values, name, &d, VALUE_HAS_DESCRIPTION, &ignored);
    if (rc != RIN_RESULT_OK)
      return rc;
  }
  for (uint32_t i = 0; i < g.output_count; ++i) {
    Name name = {};
    q::TensorDescription d = {};
    rc = value_info(g.outputs[i], &name, &d);
    if (rc != RIN_RESULT_OK)
      return rc;
    uint32_t ignored = 0;
    rc = add_value(w, &values, name, &d, VALUE_HAS_DESCRIPTION, &ignored);
    if (rc != RIN_RESULT_OK)
      return rc;
  }

  for (uint32_t ni = 0; ni < g.node_count; ++ni) {
    QNode node = {};
    rc = parse_node(g.nodes[ni], &node);
    if (rc != RIN_RESULT_OK)
      return rc;
    if (node.opcode == q::OPERATION_MATMUL_INTEGER) {
      if (integer_matmuls >= w->integer_matmul_capacity ||
          !w->integer_matmuls)
        return RIN_RESULT_BUFFER_TOO_SMALL;
      uint32_t inputs[2] = {};
      for (uint32_t side = 0u; side < 2u; ++side) {
        rc = ensure_raw_q8_data(w, g, &values, &constants, node.inputs[side],
                                &weight_bytes, &inputs[side]);
        if (rc != RIN_RESULT_OK)
          return rc;
      }
      uint32_t zero_points[2] = {UINT32_MAX, UINT32_MAX};
      for (uint32_t side = 0u; side < 2u; ++side) {
        const uint32_t input_slot = side + 2u;
        if (input_slot >= node.input_count ||
            node.inputs[input_slot].size == 0u)
          continue;
        rc = ensure_raw_q8_data(w, g, &values, &constants,
            node.inputs[input_slot], &weight_bytes, &zero_points[side]);
        if (rc != RIN_RESULT_OK)
          return rc;
        const q::TensorDescription &operand = w->tensors[inputs[side]];
        if (!q::integer_matmul_zero_point_shape_valid(
                w->tensors[zero_points[side]], operand, side == 0u))
          return RIN_RESULT_NOT_SUPPORTED;
      }
      uint32_t output_rank = 0u;
      uint32_t output_dimensions[RIN_NPU_MODEL_MAX_RANK] = {};
      if (!q::matmul_output_shape(w->tensors[inputs[0]],
              w->tensors[inputs[1]], &output_rank, output_dimensions))
        return RIN_RESULT_NOT_SUPPORTED;
      q::TensorDescription output_description = {};
      output_description.data_type = q::DATA_TYPE_INT32;
      output_description.rank = output_rank;
      output_description.scale = 1.0f;
      for (uint32_t dimension = 0u; dimension < output_rank; ++dimension)
        output_description.dimensions[dimension] = output_dimensions[dimension];
      const int previous = find_value(w, values, node.outputs[0]);
      if (previous >= 0 && (w->values[previous].flags & VALUE_AVAILABLE))
        return RIN_RESULT_CORRUPT_DATA;
      uint32_t output_index = 0u;
      rc = add_value(w, &values, node.outputs[0], &output_description,
          VALUE_HAS_DESCRIPTION | VALUE_PRODUCED | VALUE_AVAILABLE,
          &output_index);
      if (rc != RIN_RESULT_OK)
        return rc;
      if (operations >= w->operation_capacity)
        return RIN_RESULT_BUFFER_TOO_SMALL;
      q::Operation operation = {};
      operation.opcode = q::OPERATION_MATMUL_INTEGER;
      operation.input_count = 2u;
      operation.inputs[0] = inputs[0];
      operation.inputs[1] = inputs[1];
      operation.output_tensor = output_index;
      w->operations[operations] = operation;
      q::IntegerMatMulParameters parameters = {};
      parameters.operation_index = operations;
      parameters.a_zero_point_tensor = zero_points[0];
      parameters.b_zero_point_tensor = zero_points[1];
      w->integer_matmuls[integer_matmuls++] = parameters;
      ++operations;
      continue;
    }
    if (node.opcode == q::OPERATION_CONV_INTEGER) {
      if (integer_convolutions >= w->integer_convolution_capacity ||
          convolutions >= w->convolution_capacity ||
          !w->integer_convolutions || !w->convolutions)
        return RIN_RESULT_BUFFER_TOO_SMALL;
      uint32_t inputs[2] = {};
      for (uint32_t side = 0u; side < 2u; ++side) {
        rc = ensure_raw_q8_data(w, g, &values, &constants, node.inputs[side],
                                &weight_bytes, &inputs[side]);
        if (rc != RIN_RESULT_OK)
          return rc;
      }
      uint32_t zero_points[2] = {UINT32_MAX, UINT32_MAX};
      for (uint32_t side = 0u; side < 2u; ++side) {
        const uint32_t slot = side + 2u;
        if (slot >= node.input_count || node.inputs[slot].size == 0u)
          continue;
        rc = ensure_raw_q8_data(w, g, &values, &constants, node.inputs[slot],
                                &weight_bytes, &zero_points[side]);
        if (rc != RIN_RESULT_OK)
          return rc;
        if (!q::integer_convolution_zero_point_shape_valid(
                w->tensors[zero_points[side]], w->tensors[inputs[side]],
                side == 1u))
          return RIN_RESULT_NOT_SUPPORTED;
      }
      const q::TensorDescription &input_description = w->tensors[inputs[0]];
      const q::TensorDescription &weight_description = w->tensors[inputs[1]];
      if (input_description.rank < 3u ||
          input_description.rank != weight_description.rank)
        return RIN_RESULT_NOT_SUPPORTED;
      const uint32_t spatial_rank = input_description.rank - 2u;
      if ((node.has_strides && node.stride_count != spatial_rank) ||
          (node.has_dilations && node.dilation_count != spatial_rank) ||
          (node.has_kernel_shape && node.kernel_shape_count != spatial_rank) ||
          (node.has_pads && node.pad_count != spatial_rank * 2u) ||
          (node.has_auto_pad && node.has_pads))
        return RIN_RESULT_NOT_SUPPORTED;
      q::ConvolutionParameters geometry = {};
      geometry.operation_index = operations;
      geometry.spatial_rank = spatial_rank;
      geometry.group = node.has_group ? node.group : 1u;
      for (uint32_t axis = 0u; axis < spatial_rank; ++axis) {
        const uint32_t input_extent = input_description.dimensions[axis + 2u];
        const uint32_t kernel_extent = weight_description.dimensions[axis + 2u];
        const uint32_t stride = node.has_strides ? node.strides[axis] : 1u;
        const uint32_t dilation = node.has_dilations
            ? node.dilations[axis] : 1u;
        if (!stride || !dilation || !kernel_extent ||
            (node.has_kernel_shape &&
             node.kernel_shape[axis] != kernel_extent))
          return RIN_RESULT_NOT_SUPPORTED;
        geometry.strides[axis] = stride;
        geometry.dilations[axis] = dilation;
        if (node.auto_pad == 0u) {
          if (node.has_pads) {
            geometry.pads_begin[axis] = node.pads[axis];
            geometry.pads_end[axis] = node.pads[spatial_rank + axis];
          }
        } else if (node.auto_pad == 1u) {
          /* VALID has no padding. */
        } else {
          const uint64_t output_extent =
              ((uint64_t)input_extent + stride - 1u) / stride;
          const uint64_t effective_kernel =
              (uint64_t)(kernel_extent - 1u) * dilation + 1u;
          const uint64_t covered_extent =
              (output_extent - 1u) * stride + effective_kernel;
          const uint64_t total_padding = covered_extent > input_extent
              ? covered_extent - input_extent : 0u;
          const uint64_t padding_begin = node.auto_pad == 2u
              ? total_padding / 2u
              : total_padding / 2u + total_padding % 2u;
          const uint64_t padding_end = total_padding - padding_begin;
          if (padding_begin > UINT32_MAX || padding_end > UINT32_MAX)
            return RIN_RESULT_NOT_SUPPORTED;
          geometry.pads_begin[axis] = (uint32_t)padding_begin;
          geometry.pads_end[axis] = (uint32_t)padding_end;
        }
      }
      uint32_t output_rank = 0u;
      uint32_t output_dimensions[RIN_NPU_MODEL_MAX_RANK] = {};
      if (!q::convolution_output_shape(input_description, weight_description,
              geometry, &output_rank, output_dimensions))
        return RIN_RESULT_NOT_SUPPORTED;
      q::TensorDescription output_description = {};
      output_description.data_type = q::DATA_TYPE_INT32;
      output_description.rank = output_rank;
      output_description.scale = 1.0f;
      for (uint32_t dimension = 0u; dimension < output_rank; ++dimension)
        output_description.dimensions[dimension] = output_dimensions[dimension];
      const int previous = find_value(w, values, node.outputs[0]);
      if (previous >= 0 && (w->values[previous].flags & VALUE_AVAILABLE))
        return RIN_RESULT_CORRUPT_DATA;
      uint32_t output_index = 0u;
      rc = add_value(w, &values, node.outputs[0], &output_description,
          VALUE_HAS_DESCRIPTION | VALUE_PRODUCED | VALUE_AVAILABLE,
          &output_index);
      if (rc != RIN_RESULT_OK)
        return rc;
      if (operations >= w->operation_capacity)
        return RIN_RESULT_BUFFER_TOO_SMALL;
      q::Operation operation = {};
      operation.opcode = q::OPERATION_CONV_INTEGER;
      operation.input_count = 2u;
      operation.inputs[0] = inputs[0];
      operation.inputs[1] = inputs[1];
      operation.output_tensor = output_index;
      w->operations[operations] = operation;
      w->convolutions[convolutions++] = geometry;
      q::IntegerConvolutionParameters parameters = {};
      parameters.operation_index = operations;
      parameters.input_zero_point_tensor = zero_points[0];
      parameters.weight_zero_point_tensor = zero_points[1];
      w->integer_convolutions[integer_convolutions++] = parameters;
      ++operations;
      continue;
    }
    if (node.opcode == q::OPERATION_MAX_POOL_Q8) {
      if (opset < 12u || average_pools >= w->average_pool_capacity ||
          !w->average_pools)
        return opset < 12u ? RIN_RESULT_NOT_SUPPORTED
                           : RIN_RESULT_BUFFER_TOO_SMALL;
      uint32_t input_index = 0u;
      const int source_index = find_value(w, values, node.inputs[0]);
      if (source_index < 0 ||
          !(w->values[source_index].flags & VALUE_AVAILABLE)) {
        rc = ensure_raw_q8_data(w, g, &values, &constants, node.inputs[0],
                                &weight_bytes, &input_index);
        if (rc != RIN_RESULT_OK)
          return rc;
      } else {
        input_index = (uint32_t)source_index;
      }
      const q::TensorDescription &input_description = w->tensors[input_index];
      if (!(w->values[input_index].flags & VALUE_HAS_DESCRIPTION) ||
          (input_description.data_type != q::DATA_TYPE_INT8 &&
           input_description.data_type != q::DATA_TYPE_UINT8) ||
          input_description.rank < 3u ||
          input_description.rank > RIN_NPU_MODEL_MAX_RANK ||
          !(input_description.scale > 0.0f) ||
          !(input_description.scale <= FLT_MAX))
        return RIN_RESULT_NOT_SUPPORTED;
      const q::AxisQuantization *input_axis_quantization = 0;
      for (uint32_t index = 0u; index < quantizations; ++index)
        if (w->quantizations[index].tensor_index == input_index) {
          input_axis_quantization = &w->quantizations[index];
          break;
        }
      if (input_axis_quantization && input_axis_quantization->axis >= 2u)
        return RIN_RESULT_NOT_SUPPORTED;
      const q::BatchQuantization *input_batch_quantization = 0;
      for (uint32_t index = 0u; index < batch_quantizations; ++index)
        if (w->batch_quantizations[index].tensor_index == input_index) {
          input_batch_quantization = &w->batch_quantizations[index];
          break;
        }
      if (input_batch_quantization) {
        if (input_batch_quantization->rank != input_description.rank)
          return RIN_RESULT_NOT_SUPPORTED;
        for (uint32_t axis = 2u; axis < input_batch_quantization->rank; ++axis)
          if (input_batch_quantization->dimensions[axis] != 1u)
            return RIN_RESULT_NOT_SUPPORTED;
      }
      if ((input_axis_quantization &&
           (quantizations >= w->quantization_capacity || !w->quantizations)) ||
          (input_batch_quantization &&
           (batch_quantizations >= w->batch_quantization_capacity ||
            !w->batch_quantizations)))
        return RIN_RESULT_BUFFER_TOO_SMALL;

      const uint32_t spatial_rank = input_description.rank - 2u;
      if (node.kernel_shape_count != spatial_rank ||
          (node.has_strides && node.stride_count != spatial_rank) ||
          (node.has_dilations && node.dilation_count != spatial_rank) ||
          (node.has_pads && node.pad_count != spatial_rank * 2u) ||
          (node.has_auto_pad && node.has_pads))
        return RIN_RESULT_NOT_SUPPORTED;
      q::AveragePoolParameters parameters = {};
      parameters.operation_index = operations;
      parameters.spatial_rank = spatial_rank;
      parameters.ceil_mode = node.has_ceil_mode ? node.ceil_mode : 0u;
      parameters.reserved = node.has_storage_order ? node.storage_order : 0u;
      for (uint32_t axis = 0u; axis < spatial_rank; ++axis) {
        const uint32_t input_extent = input_description.dimensions[axis + 2u];
        const uint32_t kernel = node.kernel_shape[axis];
        const uint32_t stride = node.has_strides ? node.strides[axis] : 1u;
        const uint32_t dilation = node.has_dilations
            ? node.dilations[axis] : 1u;
        if (!kernel || !stride || !dilation)
          return RIN_RESULT_NOT_SUPPORTED;
        parameters.kernel_shape[axis] = kernel;
        parameters.strides[axis] = stride;
        parameters.dilations[axis] = dilation;
        if (node.auto_pad == 0u) {
          if (node.has_pads) {
            parameters.pads_begin[axis] = node.pads[axis];
            parameters.pads_end[axis] = node.pads[spatial_rank + axis];
          }
        } else if (node.auto_pad == 1u) {
          /* VALID has no padding. */
        } else {
          const uint64_t output_extent =
              ((uint64_t)input_extent + stride - 1u) / stride;
          const uint64_t effective_kernel =
              (uint64_t)(kernel - 1u) * dilation + 1u;
          const uint64_t covered_extent =
              (output_extent - 1u) * stride + effective_kernel;
          const uint64_t total_padding = covered_extent > input_extent
              ? covered_extent - input_extent : 0u;
          const uint64_t padding_begin = node.auto_pad == 2u
              ? total_padding / 2u
              : total_padding / 2u + total_padding % 2u;
          const uint64_t padding_end = total_padding - padding_begin;
          if (padding_begin > UINT32_MAX || padding_end > UINT32_MAX)
            return RIN_RESULT_NOT_SUPPORTED;
          parameters.pads_begin[axis] = (uint32_t)padding_begin;
          parameters.pads_end[axis] = (uint32_t)padding_end;
        }
      }
      uint32_t output_rank = 0u;
      uint32_t output_dimensions[RIN_NPU_MODEL_MAX_RANK] = {};
      if (!q::average_pool_output_shape(input_description, parameters,
              &output_rank, output_dimensions))
        return RIN_RESULT_NOT_SUPPORTED;
      q::TensorDescription output_description = input_description;
      output_description.rank = output_rank;
      for (uint32_t axis = 0u; axis < output_rank; ++axis)
        output_description.dimensions[axis] = output_dimensions[axis];
      for (uint32_t axis = output_rank; axis < RIN_NPU_MODEL_MAX_RANK; ++axis)
        output_description.dimensions[axis] = 0u;
      const int previous = find_value(w, values, node.outputs[0]);
      if (previous >= 0 && (w->values[previous].flags & VALUE_AVAILABLE))
        return RIN_RESULT_CORRUPT_DATA;
      uint32_t output_index = 0u;
      rc = add_value(w, &values, node.outputs[0], &output_description,
          VALUE_HAS_DESCRIPTION | VALUE_PRODUCED | VALUE_AVAILABLE,
          &output_index);
      if (rc != RIN_RESULT_OK)
        return rc;
      uint32_t indices_index = 0u;
      const bool has_indices_output =
          node.output_count == 2u && node.outputs[1].size != 0u;
      if (has_indices_output) {
        q::TensorDescription indices_description = {};
        indices_description.data_type = q::DATA_TYPE_INT64;
        indices_description.rank = output_rank;
        indices_description.scale = 1.0f;
        for (uint32_t axis = 0u; axis < output_rank; ++axis)
          indices_description.dimensions[axis] = output_dimensions[axis];
        const int previous_indices = find_value(w, values, node.outputs[1]);
        if (previous_indices >= 0 &&
            (w->values[previous_indices].flags & VALUE_AVAILABLE))
          return RIN_RESULT_CORRUPT_DATA;
        rc = add_value(w, &values, node.outputs[1], &indices_description,
            VALUE_HAS_DESCRIPTION | VALUE_PRODUCED | VALUE_AVAILABLE,
            &indices_index);
        if (rc != RIN_RESULT_OK)
          return rc;
      }
      if (input_axis_quantization) {
        q::AxisQuantization output_quantization = *input_axis_quantization;
        output_quantization.tensor_index = output_index;
        w->quantizations[quantizations++] = output_quantization;
      }
      if (input_batch_quantization) {
        q::BatchQuantization output_quantization = *input_batch_quantization;
        output_quantization.tensor_index = output_index;
        w->batch_quantizations[batch_quantizations++] = output_quantization;
      }
      if (operations >= w->operation_capacity)
        return RIN_RESULT_BUFFER_TOO_SMALL;
      q::Operation operation = {};
      operation.opcode = q::OPERATION_MAX_POOL_Q8;
      operation.input_count = 1u;
      operation.inputs[0] = input_index;
      operation.output_tensor = output_index;
      operation.reserved = has_indices_output
          ? indices_index + 1u : 0u;
      w->operations[operations] = operation;
      w->average_pools[average_pools++] = parameters;
      ++operations;
      continue;
    }
    if (node.opcode == q::OPERATION_AVERAGE_POOL_FLOAT32) {
      if (average_pools >= w->average_pool_capacity || !w->average_pools)
        return RIN_RESULT_BUFFER_TOO_SMALL;
      if (node.has_dilations && opset < 19u)
        return RIN_RESULT_NOT_SUPPORTED;
      uint32_t input_index = 0u;
      const int source_index = find_value(w, values, node.inputs[0]);
      if (source_index < 0 ||
          !(w->values[source_index].flags & VALUE_AVAILABLE)) {
        rc = ensure_float_data(w, g, &values, &constants, node.inputs[0],
                               &weight_bytes, &input_index);
        if (rc != RIN_RESULT_OK)
          return rc;
      } else {
        input_index = (uint32_t)source_index;
      }
      const q::TensorDescription &input_description = w->tensors[input_index];
      if (!(w->values[input_index].flags & VALUE_HAS_DESCRIPTION) ||
          input_description.data_type != q::DATA_TYPE_FLOAT32 ||
          input_description.rank < 3u ||
          input_description.rank > RIN_NPU_MODEL_MAX_RANK)
        return RIN_RESULT_NOT_SUPPORTED;
      const uint32_t spatial_rank = input_description.rank - 2u;
      if ((!node.global_average_pool &&
           node.kernel_shape_count != spatial_rank) ||
          (node.has_strides && node.stride_count != spatial_rank) ||
          (node.has_dilations && node.dilation_count != spatial_rank) ||
          (node.has_pads && node.pad_count != spatial_rank * 2u) ||
          (node.has_auto_pad && node.has_pads))
        return RIN_RESULT_NOT_SUPPORTED;
      q::AveragePoolParameters parameters = {};
      parameters.operation_index = operations;
      parameters.spatial_rank = spatial_rank;
      parameters.ceil_mode = node.has_ceil_mode ? node.ceil_mode : 0u;
      parameters.count_include_pad = node.has_count_include_pad
          ? node.count_include_pad : 0u;
      for (uint32_t axis = 0u; axis < spatial_rank; ++axis) {
        const uint32_t input_extent = input_description.dimensions[axis + 2u];
        const uint32_t kernel = node.global_average_pool
            ? input_extent : node.kernel_shape[axis];
        const uint32_t stride = node.has_strides ? node.strides[axis] : 1u;
        const uint32_t dilation = node.has_dilations
            ? node.dilations[axis] : 1u;
        if (!kernel || !stride || !dilation)
          return RIN_RESULT_NOT_SUPPORTED;
        parameters.kernel_shape[axis] = kernel;
        parameters.strides[axis] = stride;
        parameters.dilations[axis] = dilation;
        if (node.auto_pad == 0u) {
          if (node.has_pads) {
            parameters.pads_begin[axis] = node.pads[axis];
            parameters.pads_end[axis] = node.pads[spatial_rank + axis];
          }
        } else if (node.auto_pad == 1u) {
          /* VALID has no padding. */
        } else {
          const uint64_t output_extent =
              ((uint64_t)input_extent + stride - 1u) / stride;
          const uint64_t effective_kernel =
              (uint64_t)(kernel - 1u) * dilation + 1u;
          const uint64_t covered_extent =
              (output_extent - 1u) * stride + effective_kernel;
          const uint64_t total_padding = covered_extent > input_extent
              ? covered_extent - input_extent : 0u;
          const uint64_t padding_begin = node.auto_pad == 2u
              ? total_padding / 2u
              : total_padding / 2u + total_padding % 2u;
          const uint64_t padding_end = total_padding - padding_begin;
          if (padding_begin > UINT32_MAX || padding_end > UINT32_MAX)
            return RIN_RESULT_NOT_SUPPORTED;
          parameters.pads_begin[axis] = (uint32_t)padding_begin;
          parameters.pads_end[axis] = (uint32_t)padding_end;
        }
      }
      uint32_t output_rank = 0u;
      uint32_t output_dimensions[RIN_NPU_MODEL_MAX_RANK] = {};
      if (!q::average_pool_output_shape(input_description, parameters,
              &output_rank, output_dimensions))
        return RIN_RESULT_NOT_SUPPORTED;
      q::TensorDescription output_description = {};
      output_description.data_type = q::DATA_TYPE_FLOAT32;
      output_description.rank = output_rank;
      output_description.scale = 1.0f;
      for (uint32_t axis = 0u; axis < output_rank; ++axis)
        output_description.dimensions[axis] = output_dimensions[axis];
      const int previous = find_value(w, values, node.outputs[0]);
      if (previous >= 0 && (w->values[previous].flags & VALUE_AVAILABLE))
        return RIN_RESULT_CORRUPT_DATA;
      uint32_t output_index = 0u;
      rc = add_value(w, &values, node.outputs[0], &output_description,
          VALUE_HAS_DESCRIPTION | VALUE_PRODUCED | VALUE_AVAILABLE,
          &output_index);
      if (rc != RIN_RESULT_OK)
        return rc;
      if (operations >= w->operation_capacity)
        return RIN_RESULT_BUFFER_TOO_SMALL;
      q::Operation operation = {};
      operation.opcode = q::OPERATION_AVERAGE_POOL_FLOAT32;
      operation.input_count = 1u;
      operation.inputs[0] = input_index;
      operation.output_tensor = output_index;
      w->operations[operations] = operation;
      w->average_pools[average_pools++] = parameters;
      ++operations;
      continue;
    }
    if (node.opcode == q::OPERATION_IDENTITY_FLOAT32 ||
        node.opcode == q::OPERATION_FLATTEN_FLOAT32) {
      uint32_t input_index = 0u;
      const int source_index = find_value(w, values, node.inputs[0]);
      if (source_index < 0 ||
          !(w->values[source_index].flags & VALUE_AVAILABLE)) {
        rc = ensure_float_data(w, g, &values, &constants, node.inputs[0],
                               &weight_bytes, &input_index);
        if (rc != RIN_RESULT_OK)
          return rc;
      } else {
        input_index = (uint32_t)source_index;
      }
      const q::TensorDescription &input_description = w->tensors[input_index];
      if (!(w->values[input_index].flags & VALUE_HAS_DESCRIPTION) ||
          input_description.data_type != q::DATA_TYPE_FLOAT32)
        return RIN_RESULT_NOT_SUPPORTED;
      q::TensorDescription output_description = input_description;
      uint32_t flatten_axis = 0u;
      if (node.opcode == q::OPERATION_FLATTEN_FLOAT32) {
        const int64_t axis = node.has_axis ? node.axis : 1;
        const int64_t rank = input_description.rank;
        const int64_t normalized_axis = axis < 0 ? axis + rank : axis;
        uint32_t output_rank = 0u;
        uint32_t output_dimensions[RIN_NPU_MODEL_MAX_RANK] = {};
        if (normalized_axis < 0 || normalized_axis > rank ||
            normalized_axis > UINT32_MAX ||
            !q::flatten_output_shape(input_description,
                (uint32_t)normalized_axis, &output_rank, output_dimensions))
          return RIN_RESULT_NOT_SUPPORTED;
        flatten_axis = (uint32_t)normalized_axis;
        output_description = {};
        output_description.data_type = q::DATA_TYPE_FLOAT32;
        output_description.scale = 1.0f;
        output_description.rank = output_rank;
        for (uint32_t dimension = 0u; dimension < output_rank; ++dimension)
          output_description.dimensions[dimension] = output_dimensions[dimension];
      }
      const int previous = find_value(w, values, node.outputs[0]);
      if (previous >= 0 && (w->values[previous].flags & VALUE_AVAILABLE))
        return RIN_RESULT_CORRUPT_DATA;
      uint32_t output_index = 0u;
      rc = add_value(w, &values, node.outputs[0], &output_description,
          VALUE_HAS_DESCRIPTION | VALUE_PRODUCED | VALUE_AVAILABLE,
          &output_index);
      if (rc != RIN_RESULT_OK)
        return rc;
      if (operations >= w->operation_capacity)
        return RIN_RESULT_BUFFER_TOO_SMALL;
      q::Operation operation = {};
      operation.opcode = node.opcode;
      operation.input_count = 1u;
      operation.inputs[0] = input_index;
      operation.output_tensor = output_index;
      operation.reserved = flatten_axis;
      w->operations[operations++] = operation;
      continue;
    }
    if (node.opcode == q::OPERATION_GLOBAL_MAX_POOL_FLOAT32) {
      uint32_t input_index = 0u;
      const int source_index = find_value(w, values, node.inputs[0]);
      if (source_index < 0 ||
          !(w->values[source_index].flags & VALUE_AVAILABLE)) {
        rc = ensure_float_data(w, g, &values, &constants, node.inputs[0],
                               &weight_bytes, &input_index);
        if (rc != RIN_RESULT_OK)
          return rc;
      } else {
        input_index = (uint32_t)source_index;
      }
      const q::TensorDescription &input_description = w->tensors[input_index];
      if (!(w->values[input_index].flags & VALUE_HAS_DESCRIPTION) ||
          input_description.data_type != q::DATA_TYPE_FLOAT32)
        return RIN_RESULT_NOT_SUPPORTED;
      uint32_t output_rank = 0u;
      uint32_t output_dimensions[RIN_NPU_MODEL_MAX_RANK] = {};
      if (!q::global_max_pool_output_shape(input_description, &output_rank,
                                            output_dimensions))
        return RIN_RESULT_NOT_SUPPORTED;
      q::TensorDescription output_description = {};
      output_description.data_type = q::DATA_TYPE_FLOAT32;
      output_description.rank = output_rank;
      output_description.scale = 1.0f;
      for (uint32_t dimension = 0u; dimension < output_rank; ++dimension)
        output_description.dimensions[dimension] = output_dimensions[dimension];
      const int previous = find_value(w, values, node.outputs[0]);
      if (previous >= 0 && (w->values[previous].flags & VALUE_AVAILABLE))
        return RIN_RESULT_CORRUPT_DATA;
      uint32_t output_index = 0u;
      rc = add_value(w, &values, node.outputs[0], &output_description,
          VALUE_HAS_DESCRIPTION | VALUE_PRODUCED | VALUE_AVAILABLE,
          &output_index);
      if (rc != RIN_RESULT_OK)
        return rc;
      if (operations >= w->operation_capacity)
        return RIN_RESULT_BUFFER_TOO_SMALL;
      q::Operation operation = {};
      operation.opcode = q::OPERATION_GLOBAL_MAX_POOL_FLOAT32;
      operation.input_count = 1u;
      operation.inputs[0] = input_index;
      operation.output_tensor = output_index;
      w->operations[operations++] = operation;
      continue;
    }
    if (node.opcode == q::OPERATION_DYNAMIC_QUANTIZE_Q8) {
      if (opset < 11u)
        return RIN_RESULT_NOT_SUPPORTED;
      uint32_t input_index = 0u;
      int source_index = find_value(w, values, node.inputs[0]);
      if (source_index < 0 ||
          !(w->values[source_index].flags & VALUE_AVAILABLE)) {
        rc = ensure_float_data(w, g, &values, &constants, node.inputs[0],
                               &weight_bytes, &input_index);
        if (rc != RIN_RESULT_OK)
          return rc;
      } else {
        input_index = (uint32_t)source_index;
      }
      if (!(w->values[input_index].flags & VALUE_HAS_DESCRIPTION) ||
          w->tensors[input_index].data_type != q::DATA_TYPE_FLOAT32)
        return RIN_RESULT_NOT_SUPPORTED;
      q::TensorDescription output_descriptions[3] = {};
      output_descriptions[0] = w->tensors[input_index];
      output_descriptions[0].data_type = q::DATA_TYPE_UINT8;
      output_descriptions[0].scale = 1.0f;
      output_descriptions[0].zero_point = 0;
      output_descriptions[1].data_type = q::DATA_TYPE_FLOAT32;
      output_descriptions[1].scale = 1.0f;
      output_descriptions[2].data_type = q::DATA_TYPE_UINT8;
      output_descriptions[2].scale = 1.0f;
      uint32_t output_indices[3] = {};
      for (uint32_t output = 0u; output < 3u; ++output) {
        const int previous = find_value(w, values, node.outputs[output]);
        if (previous >= 0 &&
            (w->values[previous].flags & VALUE_AVAILABLE))
          return RIN_RESULT_CORRUPT_DATA;
        rc = add_value(w, &values, node.outputs[output],
            &output_descriptions[output],
            VALUE_HAS_DESCRIPTION | VALUE_PRODUCED | VALUE_AVAILABLE,
            &output_indices[output]);
        if (rc != RIN_RESULT_OK)
          return rc;
      }
      if (operations >= w->operation_capacity)
        return RIN_RESULT_BUFFER_TOO_SMALL;
      q::Operation operation = {};
      operation.opcode = q::OPERATION_DYNAMIC_QUANTIZE_Q8;
      operation.input_count = 1u;
      operation.inputs[0] = input_index;
      operation.inputs[1] = output_indices[1];
      operation.output_tensor = output_indices[0];
      operation.reserved = output_indices[2];
      w->operations[operations++] = operation;
      continue;
    }
    if (node.opcode == q::OPERATION_DEQUANTIZE_Q8) {
      uint32_t quantized_index = 0u;
      uint32_t scale_index = 0u;
      uint32_t zero_point_index = 0u;
      if (dynamic_quantization_values(w, values, operations, node.inputs[1],
              node.inputs[2], &quantized_index, &scale_index,
              &zero_point_index)) {
        if (opset < 11u)
          return RIN_RESULT_NOT_SUPPORTED;
        const int source = find_value(w, values, node.inputs[0]);
        if (source < 0 || !(w->values[source].flags & VALUE_AVAILABLE) ||
            (uint32_t)source != quantized_index ||
            w->tensors[source].data_type != q::DATA_TYPE_UINT8)
          return RIN_RESULT_NOT_SUPPORTED;
        q::TensorDescription output_description = w->tensors[source];
        output_description.data_type = q::DATA_TYPE_FLOAT32;
        output_description.scale = 1.0f;
        output_description.zero_point = 0;
        const int previous = find_value(w, values, node.outputs[0]);
        if (previous >= 0 &&
            (w->values[previous].flags & VALUE_AVAILABLE))
          return RIN_RESULT_CORRUPT_DATA;
        uint32_t output_index = 0u;
        rc = add_value(w, &values, node.outputs[0], &output_description,
            VALUE_HAS_DESCRIPTION | VALUE_PRODUCED | VALUE_AVAILABLE,
            &output_index);
        if (rc != RIN_RESULT_OK)
          return rc;
        if (operations >= w->operation_capacity)
          return RIN_RESULT_BUFFER_TOO_SMALL;
        q::Operation operation = {};
        operation.opcode = q::OPERATION_DEQUANTIZE_Q8;
        operation.input_count = 2u;
        operation.inputs[0] = quantized_index;
        operation.inputs[1] = scale_index;
        operation.output_tensor = output_index;
        operation.reserved = zero_point_index;
        w->operations[operations++] = operation;
        continue;
      }
    }
    if (node.opcode == q::OPERATION_QUANTIZE_Q8 ||
        node.opcode == q::OPERATION_DEQUANTIZE_Q8) {
      if (node.has_block_size && opset < 21u)
        return RIN_RESULT_NOT_SUPPORTED;
      const uint32_t block_size = node.block_size;
      const bool blocked = block_size != 0u;
      if (blocked) has_blocked_quantization = 1u;
      if (node.has_axis && opset < 13u)
        return RIN_RESULT_NOT_SUPPORTED;
      QuantizationValues parameters = {};
      rc = quantization_values_optional_zero(g, node.inputs[1],
          node.inputs[2], w, &quantization_parameters, &parameters);
      if (rc != RIN_RESULT_OK)
        return rc;
      if (!blocked && (parameters.rank > 1u ||
          (parameters.count > 1u && parameters.rank != 1u)))
        return RIN_RESULT_NOT_SUPPORTED;
      uint32_t input_index = 0u;
      uint32_t output_index = 0u;
      uint32_t quantization_axis = 0u;
      const int32_t requested_axis = node.has_axis
          ? node.axis : (parameters.count > 1u && opset >= 13u ? 1 : 0);
      if (node.opcode == q::OPERATION_QUANTIZE_Q8) {
        int source_index = find_value(w, values, node.inputs[0]);
        if (source_index < 0 ||
            !(w->values[source_index].flags & VALUE_AVAILABLE)) {
          rc = ensure_float_data(w, g, &values, &constants,
              node.inputs[0], &weight_bytes, &input_index);
          if (rc != RIN_RESULT_OK)
            return rc;
        } else {
          input_index = (uint32_t)source_index;
        }
        if (!(w->values[input_index].flags & VALUE_HAS_DESCRIPTION) ||
            w->tensors[input_index].data_type != q::DATA_TYPE_FLOAT32)
          return RIN_RESULT_NOT_SUPPORTED;
        const q::TensorDescription &input_description =
            w->tensors[input_index];
        if (blocked && parameters.rank != input_description.rank)
          return RIN_RESULT_NOT_SUPPORTED;
        if (!normalize_quantization_axis(requested_axis,
                input_description.rank, &quantization_axis))
          return RIN_RESULT_NOT_SUPPORTED;
        TensorHeader output_header = {};
        output_header.onnx_data_type =
            parameters.data_type == q::DATA_TYPE_INT8 ? 3u : 2u;
        output_header.rank = input_description.rank;
        for (uint32_t dimension = 0u;
             dimension < input_description.rank; ++dimension)
          output_header.dimensions[dimension] =
              input_description.dimensions[dimension];
        const q::TensorDescription output_description = make_description(
            output_header, quantization_scale(parameters, 0u),
            quantization_zero_point(parameters, 0u));
        const int previous_output = find_value(w, values, node.outputs[0]);
        if (previous_output >= 0 &&
            (w->values[previous_output].flags & VALUE_AVAILABLE))
          return RIN_RESULT_CORRUPT_DATA;
        rc = add_value(w, &values, node.outputs[0], &output_description,
            VALUE_HAS_DESCRIPTION | VALUE_PRODUCED | VALUE_AVAILABLE,
            &output_index);
        if (rc != RIN_RESULT_OK)
          return rc;
        rc = apply_quantization(w, &quantizations, &batch_quantizations,
            output_index, quantization_axis, parameters, block_size);
        if (rc != RIN_RESULT_OK)
          return rc;
      } else {
        int source_index = find_value(w, values, node.inputs[0]);
        uint32_t input_rank = 0u;
        if (source_index >= 0 &&
            (w->values[source_index].flags & VALUE_HAS_DESCRIPTION)) {
          input_rank = w->tensors[source_index].rank;
        } else {
          TensorHeader input_header = {};
          const int initializer = find_initializer(g, node.inputs[0],
                                                   &input_header);
          if (initializer < 0)
            return initializer == -2 ? RIN_RESULT_CORRUPT_DATA
                                     : RIN_RESULT_NOT_SUPPORTED;
          input_rank = input_header.rank;
        }
        if (!normalize_quantization_axis(requested_axis, input_rank,
                                        &quantization_axis))
          return RIN_RESULT_NOT_SUPPORTED;
        if (blocked && parameters.rank != input_rank)
          return RIN_RESULT_NOT_SUPPORTED;
        const uint32_t axis_from_end = input_rank
            ? input_rank - quantization_axis : UINT32_MAX;
        rc = ensure_data(w, g, &values, &constants, &quantizations,
            &batch_quantizations, node.inputs[0], parameters, axis_from_end,
            &weight_bytes, &input_index, block_size);
        if (rc != RIN_RESULT_OK)
          return rc;
        if (w->tensors[input_index].data_type != q::DATA_TYPE_INT8 &&
            w->tensors[input_index].data_type != q::DATA_TYPE_UINT8)
          return RIN_RESULT_NOT_SUPPORTED;
        q::TensorDescription output_description = w->tensors[input_index];
        output_description.data_type = q::DATA_TYPE_FLOAT32;
        output_description.scale = 1.0f;
        output_description.zero_point = 0;
        const int previous_output = find_value(w, values, node.outputs[0]);
        if (previous_output >= 0 &&
            (w->values[previous_output].flags & VALUE_AVAILABLE))
          return RIN_RESULT_CORRUPT_DATA;
        rc = add_value(w, &values, node.outputs[0], &output_description,
            VALUE_HAS_DESCRIPTION | VALUE_PRODUCED | VALUE_AVAILABLE,
            &output_index);
        if (rc != RIN_RESULT_OK)
          return rc;
      }
      if (operations >= w->operation_capacity)
        return RIN_RESULT_BUFFER_TOO_SMALL;
      q::Operation operation = {};
      operation.opcode = node.opcode;
      operation.input_count = 1u;
      operation.inputs[0] = input_index;
      operation.output_tensor = output_index;
      w->operations[operations++] = operation;
      continue;
    }
    if (node.opcode == q::OPERATION_MATMUL_Q8) {
      const uint32_t di[2] = {0u, 3u}, si[2] = {1u, 4u}, zi[2] = {2u, 5u};
      QuantizationValues parameters[2] = {};
      uint32_t inputs[2] = {};
      for (uint32_t side = 0; side < 2u; ++side) {
        rc = quantization_values(g, node.inputs[si[side]], node.inputs[zi[side]],
                                 w, &quantization_parameters,
                                 &parameters[side]);
        if (rc != RIN_RESULT_OK)
          return rc;
        /* A is quantized by rows and B by columns; both axes are relative to
         * the trailing matrix dimensions and repeat across broadcast batches. */
        const uint32_t axis_from_end = side == 0u ? 2u : 1u;
        rc = ensure_data(w, g, &values, &constants, &quantizations,
                         &batch_quantizations,
                         node.inputs[di[side]],
                         parameters[side], axis_from_end, &weight_bytes,
                         &inputs[side]);
        if (rc != RIN_RESULT_OK)
          return rc;
      }
      float ys = 0;
      int32_t yz = 0;
      uint32_t yt = 0;
      rc = quantization(g, node.inputs[6], node.inputs[7], &ys, &yz, &yt);
      if (rc != RIN_RESULT_OK)
        return rc;
      const q::TensorDescription &a = w->tensors[inputs[0]];
      const q::TensorDescription &b = w->tensors[inputs[1]];
      uint32_t output_rank = 0u;
      uint32_t output_dimensions[RIN_NPU_MODEL_MAX_RANK] = {};
      if (!q::matmul_output_shape(a, b, &output_rank, output_dimensions))
        return RIN_RESULT_NOT_SUPPORTED;
      TensorHeader oh = {};
      oh.onnx_data_type = yt == q::DATA_TYPE_INT8 ? 3u : 2u;
      oh.rank = output_rank;
      for (uint32_t dimension = 0u; dimension < output_rank; ++dimension)
        oh.dimensions[dimension] = output_dimensions[dimension];
      q::TensorDescription od = make_description(oh, ys, yz);
      int old = find_value(w, values, node.outputs[0]);
      if (old >= 0 && (w->values[old].flags & VALUE_AVAILABLE))
        return RIN_RESULT_CORRUPT_DATA;
      uint32_t oi = 0;
      rc = add_value(w, &values, node.outputs[0], &od,
                     VALUE_HAS_DESCRIPTION | VALUE_QUANTIZATION_SET |
                         VALUE_PRODUCED | VALUE_AVAILABLE,
                     &oi);
      if (rc != RIN_RESULT_OK)
        return rc;
      if (operations >= w->operation_capacity)
        return RIN_RESULT_BUFFER_TOO_SMALL;
      q::Operation op = {};
      op.opcode = q::OPERATION_MATMUL_Q8;
      op.input_count = 2u;
      op.inputs[0] = inputs[0];
      op.inputs[1] = inputs[1];
      op.output_tensor = oi;
      w->operations[operations++] = op;
      continue;
    }

    if (convolutions >= w->convolution_capacity || !w->convolutions)
      return RIN_RESULT_BUFFER_TOO_SMALL;
    const uint32_t data_indices[2] = {0u, 3u};
    const uint32_t scale_indices[2] = {1u, 4u};
    const uint32_t zero_indices[2] = {2u, 5u};
    QuantizationValues conv_quantization[2] = {};
    uint32_t conv_inputs[2] = {};
    for (uint32_t side = 0u; side < 2u; ++side) {
      rc = quantization_values(g, node.inputs[scale_indices[side]],
                               node.inputs[zero_indices[side]], w,
                               &quantization_parameters,
                               &conv_quantization[side]);
      if (rc != RIN_RESULT_OK)
        return rc;
      if (side == 0u && conv_quantization[side].count != 1u)
        return RIN_RESULT_NOT_SUPPORTED;
      rc = ensure_data(w, g, &values, &constants, &quantizations,
                       &batch_quantizations,
                       node.inputs[data_indices[side]],
                       conv_quantization[side], UINT32_MAX, &weight_bytes,
                       &conv_inputs[side]);
      if (rc != RIN_RESULT_OK)
        return rc;
    }
    const q::TensorDescription &input_description =
        w->tensors[conv_inputs[0]];
    const q::TensorDescription &weight_description =
        w->tensors[conv_inputs[1]];
    if (input_description.rank < 3u ||
        input_description.rank != weight_description.rank)
      return RIN_RESULT_NOT_SUPPORTED;
    const uint32_t spatial_rank = input_description.rank - 2u;
    if ((node.has_strides && node.stride_count != spatial_rank) ||
        (node.has_dilations && node.dilation_count != spatial_rank) ||
        (node.has_kernel_shape &&
         node.kernel_shape_count != spatial_rank) ||
        (node.has_pads && node.pad_count != spatial_rank * 2u) ||
        (node.auto_pad != 0u && node.has_pads))
      return RIN_RESULT_NOT_SUPPORTED;
    q::ConvolutionParameters convolution = {};
    convolution.operation_index = operations;
    convolution.spatial_rank = spatial_rank;
    convolution.group = node.has_group ? node.group : 1u;
    for (uint32_t axis = 0u; axis < spatial_rank; ++axis) {
      const uint32_t input_extent = input_description.dimensions[axis + 2u];
      const uint32_t kernel_extent = weight_description.dimensions[axis + 2u];
      const uint32_t stride = node.has_strides ? node.strides[axis] : 1u;
      const uint32_t dilation = node.has_dilations
          ? node.dilations[axis] : 1u;
      if (!stride || !dilation || !kernel_extent ||
          (node.has_kernel_shape &&
           node.kernel_shape[axis] != kernel_extent))
        return RIN_RESULT_NOT_SUPPORTED;
      convolution.strides[axis] = stride;
      convolution.dilations[axis] = dilation;
      if (node.auto_pad == 0u) {
        if (node.has_pads) {
          convolution.pads_begin[axis] = node.pads[axis];
          convolution.pads_end[axis] = node.pads[spatial_rank + axis];
        }
      } else if (node.auto_pad == 1u) {
        /* VALID uses zero padding. */
      } else {
        const uint64_t output_extent =
            ((uint64_t)input_extent + stride - 1u) / stride;
        const uint64_t effective_kernel =
            (uint64_t)(kernel_extent - 1u) * dilation + 1u;
        const uint64_t covered_extent =
            (output_extent - 1u) * stride + effective_kernel;
        const uint64_t total_padding = covered_extent > input_extent
            ? covered_extent - input_extent : 0u;
        const uint32_t begin = (uint32_t)(node.auto_pad == 2u
            ? total_padding / 2u : (total_padding + 1u) / 2u);
        convolution.pads_begin[axis] = begin;
        convolution.pads_end[axis] =
            (uint32_t)(total_padding - begin);
      }
    }
    if (node.input_count == 9u && node.inputs[8].size != 0u) {
      TensorHeader bias_header = {};
      const int bias_index = find_initializer(g, node.inputs[8], &bias_header);
      if (bias_index == -2)
        return RIN_RESULT_CORRUPT_DATA;
      if (bias_index < 0 || bias_header.onnx_data_type != 6u ||
          bias_header.rank != 1u ||
          bias_header.dimensions[0] != weight_description.dimensions[0])
        return RIN_RESULT_NOT_SUPPORTED;
      const uint32_t bias_count = bias_header.dimensions[0];
      if (convolution_bias_count > w->convolution_bias_capacity ||
          bias_count > w->convolution_bias_capacity - convolution_bias_count ||
          !w->convolution_bias_storage)
        return RIN_RESULT_BUFFER_TOO_SMALL;
      rc = q_bias_payload(g.initializers[bias_index], bias_header,
          w->convolution_bias_storage + convolution_bias_count, bias_count);
      if (rc != RIN_RESULT_OK)
        return rc;
      convolution.bias =
          w->convolution_bias_storage + convolution_bias_count;
      convolution.bias_count = bias_count;
      convolution_bias_count += bias_count;
    }
    uint32_t output_rank = 0u;
    uint32_t output_dimensions[RIN_NPU_MODEL_MAX_RANK] = {};
    if (!q::convolution_output_shape(input_description, weight_description,
            convolution, &output_rank, output_dimensions))
      return RIN_RESULT_NOT_SUPPORTED;
    float output_scale = 0.0f;
    int32_t output_zero = 0;
    uint32_t output_type = 0u;
    rc = quantization(g, node.inputs[6], node.inputs[7], &output_scale,
                      &output_zero, &output_type);
    if (rc != RIN_RESULT_OK)
      return rc;
    TensorHeader output_header = {};
    output_header.onnx_data_type =
        output_type == q::DATA_TYPE_INT8 ? 3u : 2u;
    output_header.rank = output_rank;
    for (uint32_t axis = 0u; axis < output_rank; ++axis)
      output_header.dimensions[axis] = output_dimensions[axis];
    const q::TensorDescription output_description =
        make_description(output_header, output_scale, output_zero);
    int existing_output = find_value(w, values, node.outputs[0]);
    if (existing_output >= 0 &&
        (w->values[existing_output].flags & VALUE_AVAILABLE))
      return RIN_RESULT_CORRUPT_DATA;
    uint32_t output_index = 0u;
    rc = add_value(w, &values, node.outputs[0], &output_description,
        VALUE_HAS_DESCRIPTION | VALUE_QUANTIZATION_SET | VALUE_PRODUCED |
            VALUE_AVAILABLE, &output_index);
    if (rc != RIN_RESULT_OK)
      return rc;
    if (operations >= w->operation_capacity)
      return RIN_RESULT_BUFFER_TOO_SMALL;
    q::Operation convolution_operation = {};
    convolution_operation.opcode = q::OPERATION_CONV_Q8;
    convolution_operation.input_count = 2u;
    convolution_operation.inputs[0] = conv_inputs[0];
    convolution_operation.inputs[1] = conv_inputs[1];
    convolution_operation.output_tensor = output_index;
    w->operations[operations] = convolution_operation;
    w->convolutions[convolutions++] = convolution;
    ++operations;
  }
  if (!externals)
    return RIN_RESULT_NOT_SUPPORTED;
  if (externals > w->graph_input_capacity ||
      constants > w->graph_input_capacity - externals)
    return RIN_RESULT_BUFFER_TOO_SMALL;
  const uint32_t graph_input_count = externals + constants;
  for (uint32_t i = 0; i < externals; ++i)
    w->graph_inputs[i] = w->external_inputs[i];
  for (uint32_t i = 0; i < constants; ++i)
    w->graph_inputs[externals + i] = w->constants[i].tensor_index;
  for (uint32_t i = 0; i < g.output_count; ++i) {
    Name name = {};
    q::TensorDescription expected = {};
    rc = value_info(g.outputs[i], &name, &expected);
    if (rc != RIN_RESULT_OK)
      return rc;
    int idx = find_value(w, values, name);
    if (idx < 0 || !(w->values[idx].flags & VALUE_AVAILABLE) ||
        !onnx_shape_equal(w->tensors[idx], expected))
      return RIN_RESULT_CORRUPT_DATA;
    w->graph_outputs[i] = (uint32_t)idx;
  }
  if (!fuse_qdq_average_pool(w, &operations, &values, quantizations,
      batch_quantizations, convolutions, integer_matmuls,
      integer_convolutions, average_pools, graph_input_count,
      w->graph_inputs, externals, w->external_inputs, g.output_count,
      w->graph_outputs, constants, w->constants))
    return RIN_RESULT_CORRUPT_DATA;
  q::Model model = {};
  bool has_quantize_dequantize = false;
  bool has_dynamic_quantize = false;
  bool has_q8_average_pool = false;
  bool has_v10_operation = false;
  for (uint32_t operation = 0u; operation < operations; ++operation)
    if (w->operations[operation].opcode == q::OPERATION_QUANTIZE_Q8 ||
        w->operations[operation].opcode == q::OPERATION_DEQUANTIZE_Q8)
      has_quantize_dequantize = true;
    else if (w->operations[operation].opcode ==
             q::OPERATION_DYNAMIC_QUANTIZE_Q8)
      has_dynamic_quantize = true;
    else if (w->operations[operation].opcode ==
             q::OPERATION_AVERAGE_POOL_Q8)
      has_q8_average_pool = true;
    else if (w->operations[operation].opcode ==
                 q::OPERATION_GLOBAL_MAX_POOL_FLOAT32 ||
             w->operations[operation].opcode == q::OPERATION_MAX_POOL_Q8)
      has_v10_operation = true;
    else if (w->operations[operation].opcode ==
                 q::OPERATION_IDENTITY_FLOAT32 ||
             w->operations[operation].opcode == q::OPERATION_FLATTEN_FLOAT32)
      has_v10_operation = true;
  model.version = has_q8_average_pool
      ? RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V11
      : has_v10_operation
      ? RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10
      : average_pools
      ? RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9
      : integer_convolutions
      ? RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8
      : integer_matmuls
      ? RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7
      : has_blocked_quantization
      ? RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V6
      : has_dynamic_quantize
      ? RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V5
      : batch_quantizations || has_quantize_dequantize
      ? RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V4
      : convolutions
      ? RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V3
      : quantizations ? RIN_NPU_QUANTIZED_MODEL_ABI_VERSION
                      : RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V1;
  model.struct_size = model.version ==
          RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V11 || model.version ==
          RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V10 || model.version ==
          RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V9
      ? sizeof(model)
      : model.version ==
          RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V8
      ? RIN_NPU_QUANTIZED_MODEL_V8_STRUCT_SIZE
      : model.version ==
          RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V7
      ? RIN_NPU_QUANTIZED_MODEL_V7_STRUCT_SIZE
      : (model.version ==
          RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V4 ||
          model.version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V5 ||
          model.version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V6)
      ? RIN_NPU_QUANTIZED_MODEL_V6_STRUCT_SIZE
      : model.version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION_V3
          ? RIN_NPU_QUANTIZED_MODEL_V3_STRUCT_SIZE
      : model.version == RIN_NPU_QUANTIZED_MODEL_ABI_VERSION
          ? RIN_NPU_QUANTIZED_MODEL_V2_STRUCT_SIZE
          : RIN_NPU_QUANTIZED_MODEL_V1_STRUCT_SIZE;
  model.tensor_count = values;
  model.input_count = graph_input_count;
  model.output_count = g.output_count;
  model.operation_count = operations;
  model.tensors = w->tensors;
  model.inputs = w->graph_inputs;
  model.outputs = w->graph_outputs;
  model.operations = operations ? w->operations : 0;
  model.quantization_count = quantizations;
  model.quantizations = quantizations ? w->quantizations : 0;
  model.convolution_count = convolutions;
  model.convolutions = convolutions ? w->convolutions : 0;
  model.batch_quantization_count = batch_quantizations;
  model.batch_quantizations = batch_quantizations
      ? w->batch_quantizations : 0;
  model.integer_matmul_count = integer_matmuls;
  model.integer_matmuls = integer_matmuls ? w->integer_matmuls : 0;
  model.integer_convolution_count = integer_convolutions;
  model.integer_convolutions = integer_convolutions
      ? w->integer_convolutions : 0;
  model.average_pool_count = average_pools;
  model.average_pools = average_pools ? w->average_pools : 0;
  if (!q::model_valid(&model))
    return RIN_RESULT_CORRUPT_DATA;
  Result result = {};
  result.graph = model;
  result.external_input_count = externals;
  result.external_input_indices = w->external_inputs;
  result.constant_count = constants;
  result.constants = w->constants;
  result.constant_values = w->weight_storage;
  result.constant_value_bytes = weight_bytes;
  *out = result;
  return RIN_RESULT_OK;
}
static inline RinResultCode bind_constants(const Result *imported,
                                           q::TensorView *tensors,
                                           uint32_t tensor_count) {
  if (!imported || !tensors || !q::model_valid(&imported->graph) ||
      tensor_count != imported->graph.tensor_count ||
      imported->constant_count > RIN_NPU_MODEL_MAX_TENSORS ||
      (imported->constant_count &&
       (!imported->constants || !imported->constant_values)))
    return RIN_RESULT_INVALID_ARGUMENT;
  uint64_t vb = (uint64_t)tensor_count * sizeof(*tensors);
  if (memory_ranges_overlap(tensors, vb, imported, sizeof(*imported)) ||
      q::model_storage_overlaps(&imported->graph, tensors, vb) ||
      memory_ranges_overlap(tensors, vb, imported->graph.tensors,
                            (uint64_t)tensor_count *
                                sizeof(*imported->graph.tensors)) ||
      memory_ranges_overlap(tensors, vb, imported->graph.inputs,
                            (uint64_t)imported->graph.input_count *
                                sizeof(*imported->graph.inputs)) ||
      memory_ranges_overlap(tensors, vb, imported->graph.outputs,
                            (uint64_t)imported->graph.output_count *
                                sizeof(*imported->graph.outputs)) ||
      memory_ranges_overlap(tensors, vb, imported->graph.operations,
                            (uint64_t)imported->graph.operation_count *
                                sizeof(*imported->graph.operations)) ||
      memory_ranges_overlap(tensors, vb, imported->constants,
                            (uint64_t)imported->constant_count *
                                sizeof(*imported->constants)) ||
      memory_ranges_overlap(tensors, vb, imported->constant_values,
                            imported->constant_value_bytes))
    return RIN_RESULT_INVALID_ARGUMENT;
  for (uint32_t i = 0; i < imported->constant_count; ++i) {
    const ConstantBinding &b = imported->constants[i];
    const uint64_t storage_bytes = b.tensor_index < tensor_count
        ? q::tensor_storage_bytes(imported->graph.tensors[b.tensor_index])
        : 0u;
    if (b.reserved || b.tensor_index >= tensor_count ||
        b.byte_offset > imported->constant_value_bytes ||
        !storage_bytes ||
        storage_bytes > imported->constant_value_bytes - b.byte_offset ||
        b.element_count !=
            q::tensor_elements(imported->graph.tensors[b.tensor_index]))
      return RIN_RESULT_CORRUPT_DATA;
    for (uint32_t j = 0; j < i; ++j)
      if (imported->constants[j].tensor_index == b.tensor_index)
        return RIN_RESULT_CORRUPT_DATA;
  }
  for (uint32_t i = 0; i < imported->constant_count; ++i) {
    const ConstantBinding &b = imported->constants[i];
    tensors[b.tensor_index].data = imported->constant_values + b.byte_offset;
    tensors[b.tensor_index].element_count = b.element_count;
  }
  return RIN_RESULT_OK;
}
} // namespace quantized
} // namespace onnx
} // namespace model
} // namespace npu
} // namespace rin
#endif
