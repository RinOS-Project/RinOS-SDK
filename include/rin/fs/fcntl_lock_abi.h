/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_FCNTL_LOCK_ABI_H
#define RIN_SDK_FCNTL_LOCK_ABI_H

#include <stdint.h>

/* Linux-compatible wire layouts with no target-width long/pointer fields. */
#if !defined(MIDL_PASS)
#define RIN_FCNTL_LOCK_ALIGN __attribute__((aligned(8)))
#else
#define RIN_FCNTL_LOCK_ALIGN
#endif

typedef struct RIN_FCNTL_LOCK_ALIGN RinNativeFlockAbi {
    int16_t type;
    int16_t whence;
    int64_t start;
    int64_t length;
    int32_t pid;
    int32_t reserved;
} RinNativeFlockAbi;

typedef struct RinCompat32FlockAbi {
    int16_t type;
    int16_t whence;
    int32_t start;
    int32_t length;
    int32_t pid;
} RinCompat32FlockAbi;

/* FCNTL_LOCK64 preserves the fixed RinNativeFlockAbi input by default.
 * Callers that pass their native struct flock/flock64 use this selector so
 * the kernel can copy and translate the caller layout directly. */
#define RIN_FCNTL_LOCK_LAYOUT_FIXED_V1 0u
#define RIN_FCNTL_LOCK_LAYOUT_CALLER_OFF64_V1 1u

#ifndef MIDL_PASS
static inline uint32_t rin_fcntl_lock_caller_off64_size(int compat32)
{
    return compat32 ? 24u : 32u;
}

static inline uint16_t rin_fcntl_lock_user_load_u16(const uint8_t* bytes)
{
    return (uint16_t)bytes[0] | (uint16_t)((uint16_t)bytes[1] << 8);
}

static inline uint32_t rin_fcntl_lock_user_load_u32(const uint8_t* bytes)
{
    return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) |
           ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
}

static inline uint64_t rin_fcntl_lock_user_load_u64(const uint8_t* bytes)
{
    return (uint64_t)rin_fcntl_lock_user_load_u32(bytes) |
           ((uint64_t)rin_fcntl_lock_user_load_u32(bytes + 4u) << 32);
}

static inline int16_t rin_fcntl_lock_user_i16(uint16_t value)
{
    if ((value & UINT16_C(0x8000)) == 0u) return (int16_t)value;
    if (value == UINT16_C(0x8000)) return INT16_MIN;
    return (int16_t)-(int16_t)((~value + 1u) & UINT16_MAX);
}

static inline int32_t rin_fcntl_lock_user_i32(uint32_t value)
{
    if ((value & UINT32_C(0x80000000)) == 0u) return (int32_t)value;
    if (value == UINT32_C(0x80000000)) return INT32_MIN;
    return -(int32_t)((~value + 1u) & UINT32_MAX);
}

static inline int64_t rin_fcntl_lock_user_i64(uint64_t value)
{
    if ((value & (UINT64_C(1) << 63)) == 0u) return (int64_t)value;
    if (value == (UINT64_C(1) << 63)) return INT64_MIN;
    return -(int64_t)(~value + 1u);
}

static inline int rin_fcntl_lock_caller_off64_decode(
    const uint8_t* bytes, uint32_t byte_count, int compat32,
    RinNativeFlockAbi* value)
{
    const uint32_t start_offset = compat32 ? 4u : 8u;
    const uint32_t length_offset = start_offset + 8u;
    const uint32_t pid_offset = length_offset + 8u;
    if (bytes == 0 || value == 0 ||
        byte_count != rin_fcntl_lock_caller_off64_size(compat32))
        return 0;
    value->type = rin_fcntl_lock_user_i16(
        rin_fcntl_lock_user_load_u16(bytes));
    value->whence = rin_fcntl_lock_user_i16(
        rin_fcntl_lock_user_load_u16(bytes + 2u));
    value->start = rin_fcntl_lock_user_i64(
        rin_fcntl_lock_user_load_u64(bytes + start_offset));
    value->length = rin_fcntl_lock_user_i64(
        rin_fcntl_lock_user_load_u64(bytes + length_offset));
    value->pid = rin_fcntl_lock_user_i32(
        rin_fcntl_lock_user_load_u32(bytes + pid_offset));
    value->reserved = 0;
    return 1;
}

static inline void rin_fcntl_lock_user_store_u16(uint8_t* bytes,
                                                 uint16_t value)
{
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
}

static inline void rin_fcntl_lock_user_store_u64(uint8_t* bytes,
                                                 uint64_t value)
{
    uint32_t index;
    for (index = 0u; index < 8u; ++index)
        bytes[index] = (uint8_t)(value >> (index * 8u));
}

static inline void rin_fcntl_lock_caller_off64_encode(
    uint8_t* bytes, uint32_t byte_count, int compat32,
    const RinNativeFlockAbi* value)
{
    const uint32_t start_offset = compat32 ? 4u : 8u;
    const uint32_t length_offset = start_offset + 8u;
    const uint32_t pid_offset = length_offset + 8u;
    uint32_t index;
    if (bytes == 0 || value == 0 ||
        byte_count != rin_fcntl_lock_caller_off64_size(compat32))
        return;
    for (index = 0u; index < byte_count; ++index) bytes[index] = 0u;
    rin_fcntl_lock_user_store_u16(bytes, (uint16_t)value->type);
    rin_fcntl_lock_user_store_u16(bytes + 2u, (uint16_t)value->whence);
    rin_fcntl_lock_user_store_u64(bytes + start_offset,
                                  (uint64_t)value->start);
    rin_fcntl_lock_user_store_u64(bytes + length_offset,
                                  (uint64_t)value->length);
    {
        const uint32_t pid = (uint32_t)value->pid;
        bytes[pid_offset] = (uint8_t)pid;
        bytes[pid_offset + 1u] = (uint8_t)(pid >> 8);
        bytes[pid_offset + 2u] = (uint8_t)(pid >> 16);
        bytes[pid_offset + 3u] = (uint8_t)(pid >> 24);
    }
}
#endif /* !MIDL_PASS */

#if defined(__cplusplus) && !defined(MIDL_PASS)
static_assert(sizeof(RinNativeFlockAbi) == 32u,
              "native fcntl lock ABI drift");
static_assert(sizeof(RinCompat32FlockAbi) == 16u,
              "compat32 fcntl lock ABI drift");
#elif !defined(MIDL_PASS)
_Static_assert(sizeof(RinNativeFlockAbi) == 32u,
               "native fcntl lock ABI drift");
_Static_assert(sizeof(RinCompat32FlockAbi) == 16u,
               "compat32 fcntl lock ABI drift");
#endif

#undef RIN_FCNTL_LOCK_ALIGN

#endif
