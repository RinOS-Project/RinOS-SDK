#ifndef RIN_DRIVER_NPU_MODEL_QUANTIZED_ONNX_HPP
#define RIN_DRIVER_NPU_MODEL_QUANTIZED_ONNX_HPP
#include <float.h>
#include <rin/driver/npu_model_onnx.hpp>
#include <rin/driver/npu_model_quantized.hpp>
#include <stdint.h>

#define RIN_NPU_QUANTIZED_ONNX_MAX_WEIGHT_BYTES UINT64_C(16777216)

/* Allocation-free QLinearMatMul importer for rank-two, per-tensor Q8 graphs.
 * Scale and zero-point values must be scalar initializers. Unsupported ONNX
 * layouts/operators are rejected; all caller storage outlives the model. */
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
  if (!out || h.onnx_data_type != 1u || h.rank != 0u)
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
static inline RinResultCode quantization(const GraphFields &g, Name sn, Name zn,
                                         float *scale, int32_t *zero,
                                         uint32_t *dtype) {
  TensorHeader sh = {}, zh = {};
  int si = find_initializer(g, sn, &sh), zi = find_initializer(g, zn, &zh);
  if (si < 0 || zi < 0)
    return si == -2 || zi == -2 ? RIN_RESULT_CORRUPT_DATA
                                : RIN_RESULT_NOT_SUPPORTED;
  RinResultCode rc = scalar_scale(g.initializers[si], sh, scale);
  if (rc != RIN_RESULT_OK)
    return rc;
  if (zh.rank || (zh.onnx_data_type != 2u && zh.onnx_data_type != 3u))
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
static inline RinResultCode ensure_data(Workspace *w, const GraphFields &g,
                                        uint32_t *values, uint32_t *constants,
                                        Name name, float scale, int32_t zero,
                                        uint32_t dtype, uint64_t *bytes,
                                        uint32_t *out) {
  int index = find_value(w, *values, name);
  if (index >= 0) {
    ValueBinding &v = w->values[index];
    if (!(v.flags & VALUE_AVAILABLE) || !(v.flags & VALUE_HAS_DESCRIPTION))
      return RIN_RESULT_CORRUPT_DATA;
    q::TensorDescription &d = w->tensors[index];
    if (d.data_type != dtype)
      return RIN_RESULT_NOT_SUPPORTED;
    if (v.flags & VALUE_QUANTIZATION_SET) {
      if (d.scale != scale || d.zero_point != zero)
        return RIN_RESULT_NOT_SUPPORTED;
    }
    d.scale = scale;
    d.zero_point = zero;
    v.flags |= VALUE_QUANTIZATION_SET;
    if (!q::tensor_elements(d))
      return RIN_RESULT_NOT_SUPPORTED;
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
          dtype)
    return RIN_RESULT_NOT_SUPPORTED;
  q::TensorDescription d = make_description(h, scale, zero);
  uint64_t count = q::tensor_elements(d);
  if (!count)
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
                                   VALUE_AVAILABLE | VALUE_QUANTIZATION_SET,
                               &idx);
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
  const void *ptrs[] = {w->tensors,       w->graph_inputs,  w->external_inputs,
                        w->graph_outputs, w->operations,    w->values,
                        w->constants,     w->weight_storage};
  const uint64_t sizes[] = {
      (uint64_t)w->tensor_capacity * sizeof(*w->tensors),
      (uint64_t)w->graph_input_capacity * sizeof(*w->graph_inputs),
      (uint64_t)w->external_input_capacity * sizeof(*w->external_inputs),
      (uint64_t)w->graph_output_capacity * sizeof(*w->graph_outputs),
      (uint64_t)w->operation_capacity * sizeof(*w->operations),
      (uint64_t)w->value_capacity * sizeof(*w->values),
      (uint64_t)w->constant_capacity * sizeof(*w->constants),
      w->weight_capacity_bytes};
  for (uint32_t i = 0; i < 8u; ++i) {
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
    float scales[2] = {};
    int32_t zeros[2] = {};
    uint32_t types[2] = {}, inputs[2] = {};
    for (uint32_t side = 0; side < 2u; ++side) {
      rc = quantization(g, node.inputs[si[side]], node.inputs[zi[side]],
                        &scales[side], &zeros[side], &types[side]);
      if (rc != RIN_RESULT_OK)
        return rc;
      rc = ensure_data(w, g, &values, &constants, node.inputs[di[side]],
                       scales[side], zeros[side], types[side], &weight_bytes,
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
    if (a.rank != 2u || b.rank != 2u || a.dimensions[1] != b.dimensions[0])
      return RIN_RESULT_NOT_SUPPORTED;
    TensorHeader oh = {};
    oh.onnx_data_type = yt == q::DATA_TYPE_INT8 ? 3u : 2u;
    oh.rank = 2u;
    oh.dimensions[0] = a.dimensions[0];
    oh.dimensions[1] = b.dimensions[1];
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
