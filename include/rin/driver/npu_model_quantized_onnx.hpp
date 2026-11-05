#ifndef RIN_DRIVER_NPU_MODEL_QUANTIZED_ONNX_HPP
#define RIN_DRIVER_NPU_MODEL_QUANTIZED_ONNX_HPP
#include <float.h>
#include <rin/driver/npu_model_onnx.hpp>
#include <rin/driver/npu_model_quantized.hpp>
#include <stdint.h>

#define RIN_NPU_QUANTIZED_ONNX_MAX_WEIGHT_BYTES UINT64_C(16777216)

/* Allocation-free QLinearMatMul importer for fixed-shape, batched Q8 graphs.
 * Unsupported ONNX layouts/operators are rejected; all caller storage
 * outlives the model. */
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
  Name inputs[8];
  Name output;
  uint32_t input_count;
};
struct QuantizationValues {
  float *scales;
  int32_t *zero_points;
  float scalar_scale;
  int32_t scalar_zero_point;
  uint32_t count;
  uint32_t data_type;
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
  if (!htype || !hshape || (type != 2u && type != 3u))
    return RIN_RESULT_NOT_SUPPORTED;
  q::TensorDescription d = {};
  d.data_type = type == 3u ? q::DATA_TYPE_INT8 : q::DATA_TYPE_UINT8;
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
  d.data_type = h.onnx_data_type == 3u ? q::DATA_TYPE_INT8 : q::DATA_TYPE_UINT8;
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
  if ((count && !out) || h.onnx_data_type != 1u || h.rank > 1u ||
      count != (h.rank ? h.dimensions[0] : 1u))
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

static inline RinResultCode zero_point_values(Slice tensor,
                                              const TensorHeader &h,
                                              int32_t *out,
                                              uint32_t count) {
  if ((count && !out) || count != (h.rank ? h.dimensions[0] : 1u) ||
      h.rank > 1u || (h.onnx_data_type != 2u && h.onnx_data_type != 3u))
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
  if (scale_header.onnx_data_type != 1u || scale_header.rank > 1u ||
      zero_header.rank != scale_header.rank ||
      (zero_header.onnx_data_type != 2u && zero_header.onnx_data_type != 3u) ||
      (scale_header.rank &&
       zero_header.dimensions[0] != scale_header.dimensions[0]))
    return RIN_RESULT_NOT_SUPPORTED;
  const uint32_t count = scale_header.rank ? scale_header.dimensions[0] : 1u;
  if (!count || count > RIN_NPU_MODEL_MAX_TENSOR_ELEMENTS)
    return RIN_RESULT_BUFFER_TOO_SMALL;
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
  if (scales) *parameter_count += count;
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
static inline RinResultCode parse_node(Slice bytes, QNode *out) {
  if (!out)
    return RIN_RESULT_INVALID_ARGUMENT;
  QNode node = {};
  Slice op = {}, domain = {};
  uint32_t hop = 0, hd = 0, outputs = 0;
  Reader r = make_reader(bytes);
  Field f;
  int n = 0;
  while ((n = next_field(&r, &f)) > 0) {
    if (f.number == 1u) {
      if (node.input_count >= 8u ||
          !field_name(f, &node.inputs[node.input_count]))
        return RIN_RESULT_NOT_SUPPORTED;
      ++node.input_count;
    } else if (f.number == 2u) {
      if (outputs || !field_name(f, &node.output))
        return RIN_RESULT_CORRUPT_DATA;
      ++outputs;
    } else if (f.number == 4u) {
      if (f.wire_type != 2u || hop)
        return RIN_RESULT_CORRUPT_DATA;
      op = f.bytes;
      hop = 1;
    } else if (f.number == 5u)
      return RIN_RESULT_NOT_SUPPORTED;
    else if (f.number == 7u) {
      if (f.wire_type != 2u || hd)
        return RIN_RESULT_CORRUPT_DATA;
      domain = f.bytes;
      hd = 1;
    }
  }
  if (n < 0)
    return RIN_RESULT_CORRUPT_DATA;
  if (!hop || outputs != 1u || node.input_count != 8u ||
      !slice_is(op, "QLinearMatMul", 13u) ||
      (hd && domain.size && !slice_is(domain, "ai.onnx", 7u)))
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
    Workspace *w, uint32_t *quantizations, uint32_t tensor_index,
    uint32_t axis, const QuantizationValues &parameters) {
  ValueBinding &value = w->values[tensor_index];
  q::TensorDescription &description = w->tensors[tensor_index];
  if (description.data_type != parameters.data_type || !parameters.count ||
      (!parameters.scales && parameters.count != 1u) ||
      (!parameters.zero_points && parameters.count != 1u))
    return RIN_RESULT_NOT_SUPPORTED;
  if (parameters.count > 1u &&
      (description.rank < 2u || axis >= description.rank ||
       parameters.count != description.dimensions[axis]))
    return RIN_RESULT_NOT_SUPPORTED;
  if (value.flags & VALUE_QUANTIZATION_SET) {
    if (description.scale != quantization_scale(parameters, 0u) ||
        description.zero_point != quantization_zero_point(parameters, 0u))
      return RIN_RESULT_NOT_SUPPORTED;
    if (parameters.count == 1u)
      return value.reserved == 0u ? RIN_RESULT_OK : RIN_RESULT_NOT_SUPPORTED;
    if (value.reserved == 0u || value.reserved > *quantizations)
      return RIN_RESULT_NOT_SUPPORTED;
    const q::AxisQuantization &existing =
        w->quantizations[value.reserved - 1u];
    if (existing.tensor_index != tensor_index || existing.axis != axis ||
        existing.count != parameters.count)
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
  if (parameters.count > 1u) {
    if (*quantizations >= w->quantization_capacity || !w->quantizations)
      return RIN_RESULT_BUFFER_TOO_SMALL;
    q::AxisQuantization quantization = {};
    quantization.tensor_index = tensor_index;
    quantization.axis = axis;
    quantization.count = parameters.count;
    quantization.scales = parameters.scales;
    quantization.zero_points = parameters.zero_points;
    w->quantizations[*quantizations] = quantization;
    value.reserved = *quantizations + 1u;
    ++*quantizations;
  }
  return RIN_RESULT_OK;
}
static inline RinResultCode ensure_data(Workspace *w, const GraphFields &g,
                                        uint32_t *values, uint32_t *constants,
                                        uint32_t *quantizations, Name name,
                                        const QuantizationValues &parameters,
                                        uint32_t axis_from_end, uint64_t *bytes,
                                        uint32_t *out) {
  int index = find_value(w, *values, name);
  if (index >= 0) {
    ValueBinding &v = w->values[index];
    if (!(v.flags & VALUE_AVAILABLE) || !(v.flags & VALUE_HAS_DESCRIPTION))
      return RIN_RESULT_CORRUPT_DATA;
    if (w->tensors[index].rank < 2u || !axis_from_end ||
        axis_from_end > w->tensors[index].rank)
      return RIN_RESULT_NOT_SUPPORTED;
    const uint32_t axis = w->tensors[index].rank - axis_from_end;
    RinResultCode rc = apply_quantization(
        w, quantizations, (uint32_t)index, axis, parameters);
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
  if (!count || d.rank < 2u || !axis_from_end || axis_from_end > d.rank)
    return RIN_RESULT_NOT_SUPPORTED;
  const uint32_t axis = d.rank - axis_from_end;
  if (parameters.count > 1u &&
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
  rc = apply_quantization(w, quantizations, idx, axis, parameters);
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
       (!w->quantization_scales || !w->quantization_zero_points)))
    return RIN_RESULT_INVALID_ARGUMENT;
  const void *ptrs[] = {w->tensors, w->graph_inputs, w->external_inputs,
                        w->graph_outputs, w->operations, w->values,
                        w->constants, w->weight_storage, w->quantizations,
                        w->quantization_scales, w->quantization_zero_points};
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
          sizeof(*w->quantization_zero_points)};
  for (uint32_t i = 0; i < 11u; ++i) {
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
    int old = find_value(w, values, node.output);
    if (old >= 0 && (w->values[old].flags & VALUE_AVAILABLE))
      return RIN_RESULT_CORRUPT_DATA;
    uint32_t oi = 0;
    rc = add_value(w, &values, node.output, &od,
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
  q::Model model = {};
  model.struct_size = sizeof(model);
  model.version = RIN_NPU_QUANTIZED_MODEL_ABI_VERSION;
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
    if (b.reserved || b.tensor_index >= tensor_count ||
        b.byte_offset > imported->constant_value_bytes ||
        b.element_count > imported->constant_value_bytes - b.byte_offset ||
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
