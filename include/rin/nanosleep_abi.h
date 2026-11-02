/* SPDX-License-Identifier: MIT */
#ifndef RIN_NANOSLEEP_ABI_H
#define RIN_NANOSLEEP_ABI_H

#include <stddef.h>
#include <stdint.h>

/* SYS_NANOSLEEP のslot 2で選択する固定幅request形式。 */
#define RIN_NANOSLEEP_ABI_SELECTOR_V1 UINT32_C(0x4e534c50)
#define RIN_NANOSLEEP_ABI_VERSION_V1 UINT32_C(1)
#define RIN_NANOSLEEP_ABI_SELECTOR_USERPTR_V2 UINT32_C(0x4e534c32)
#define RIN_NANOSLEEP_ABI_VERSION_USERPTR_V2 UINT32_C(2)
#define RIN_NANOSLEEP_USERPTR_FLAG_ABSOLUTE UINT32_C(1)
#define RIN_NANOSLEEP_CLOCK_REALTIME UINT32_C(0)
#define RIN_NANOSLEEP_CLOCK_MONOTONIC UINT32_C(1)

typedef struct RinNanosleepRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    uint32_t reserved0;
    int64_t duration_sec;
    int64_t duration_nsec;
    int64_t remaining_sec;
    int64_t remaining_nsec;
    uint32_t reserved[2];
} RinNanosleepRequestV1;

/* Pointer transaction for POSIX nanosleep/clock_nanosleep. The kernel copies
 * and validates the caller's native timespec, and pins the optional remainder
 * before waiting. Widths describe time_t and long in the caller's ABI. */
typedef struct RinNanosleepUserRequestV2 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    uint32_t clock_id;
    uint32_t time_width;
    uint32_t long_width;
    uint32_t reserved0;
    uint32_t reserved1;
    uint64_t request_address;
    uint64_t remaining_address;
    uint64_t reserved;
} RinNanosleepUserRequestV2;

#if defined(__cplusplus)
static_assert(sizeof(RinNanosleepRequestV1) == 56,
              "RinNanosleepRequestV1 ABI drift");
#else
_Static_assert(sizeof(RinNanosleepRequestV1) == 56,
               "RinNanosleepRequestV1 ABI drift");
#endif

#if defined(__cplusplus)
static_assert(sizeof(RinNanosleepUserRequestV2) == 56,
              "RinNanosleepUserRequestV2 ABI drift");
static_assert(offsetof(RinNanosleepUserRequestV2, struct_size) == 0,
              "RinNanosleepUserRequestV2.struct_size ABI drift");
static_assert(offsetof(RinNanosleepUserRequestV2, version) == 4,
              "RinNanosleepUserRequestV2.version ABI drift");
static_assert(offsetof(RinNanosleepUserRequestV2, flags) == 8,
              "RinNanosleepUserRequestV2.flags ABI drift");
static_assert(offsetof(RinNanosleepUserRequestV2, clock_id) == 12,
              "RinNanosleepUserRequestV2.clock_id ABI drift");
static_assert(offsetof(RinNanosleepUserRequestV2, time_width) == 16,
              "RinNanosleepUserRequestV2.time_width ABI drift");
static_assert(offsetof(RinNanosleepUserRequestV2, long_width) == 20,
              "RinNanosleepUserRequestV2.long_width ABI drift");
static_assert(offsetof(RinNanosleepUserRequestV2, reserved0) == 24,
              "RinNanosleepUserRequestV2.reserved0 ABI drift");
static_assert(offsetof(RinNanosleepUserRequestV2, reserved1) == 28,
              "RinNanosleepUserRequestV2.reserved1 ABI drift");
static_assert(offsetof(RinNanosleepUserRequestV2, request_address) == 32,
              "RinNanosleepUserRequestV2.request_address ABI drift");
static_assert(offsetof(RinNanosleepUserRequestV2, remaining_address) == 40,
              "RinNanosleepUserRequestV2.remaining_address ABI drift");
static_assert(offsetof(RinNanosleepUserRequestV2, reserved) == 48,
              "RinNanosleepUserRequestV2.reserved ABI drift");
#else
_Static_assert(sizeof(RinNanosleepUserRequestV2) == 56,
               "RinNanosleepUserRequestV2 ABI drift");
_Static_assert(offsetof(RinNanosleepUserRequestV2, struct_size) == 0,
               "RinNanosleepUserRequestV2.struct_size ABI drift");
_Static_assert(offsetof(RinNanosleepUserRequestV2, version) == 4,
               "RinNanosleepUserRequestV2.version ABI drift");
_Static_assert(offsetof(RinNanosleepUserRequestV2, flags) == 8,
               "RinNanosleepUserRequestV2.flags ABI drift");
_Static_assert(offsetof(RinNanosleepUserRequestV2, clock_id) == 12,
               "RinNanosleepUserRequestV2.clock_id ABI drift");
_Static_assert(offsetof(RinNanosleepUserRequestV2, time_width) == 16,
               "RinNanosleepUserRequestV2.time_width ABI drift");
_Static_assert(offsetof(RinNanosleepUserRequestV2, long_width) == 20,
               "RinNanosleepUserRequestV2.long_width ABI drift");
_Static_assert(offsetof(RinNanosleepUserRequestV2, reserved0) == 24,
               "RinNanosleepUserRequestV2.reserved0 ABI drift");
_Static_assert(offsetof(RinNanosleepUserRequestV2, reserved1) == 28,
               "RinNanosleepUserRequestV2.reserved1 ABI drift");
_Static_assert(offsetof(RinNanosleepUserRequestV2, request_address) == 32,
               "RinNanosleepUserRequestV2.request_address ABI drift");
_Static_assert(offsetof(RinNanosleepUserRequestV2, remaining_address) == 40,
               "RinNanosleepUserRequestV2.remaining_address ABI drift");
_Static_assert(offsetof(RinNanosleepUserRequestV2, reserved) == 48,
               "RinNanosleepUserRequestV2.reserved ABI drift");
#endif

static inline uint32_t rin_nanosleep_timespec_user_size(uint32_t time_width,
                                                         uint32_t long_width)
{
    const uint32_t nsec_offset = time_width == 4u && long_width == 8u
        ? 8u : time_width;
    if ((time_width != 4u && time_width != 8u) ||
        (long_width != 4u && long_width != 8u))
        return 0u;
    return nsec_offset + long_width;
}

static inline uint64_t rin_nanosleep_user_load_le(const uint8_t* bytes,
                                                   uint32_t width)
{
    uint64_t value = 0u;
    uint32_t index;
    for (index = 0u; index < width; ++index)
        value |= (uint64_t)bytes[index] << (index * 8u);
    return value;
}

static inline int rin_nanosleep_user_decode_signed(
    const uint8_t* bytes, uint32_t width, int64_t* value_out)
{
    uint64_t value;
    if (bytes == NULL || value_out == NULL ||
        (width != 4u && width != 8u))
        return 0;
    value = rin_nanosleep_user_load_le(bytes, width);
    if (width == 4u) {
        const uint32_t value32 = (uint32_t)value;
        *value_out = (value32 & UINT32_C(0x80000000)) != 0u
            ? -(int64_t)((~value32 + 1u) & UINT32_MAX)
            : (int64_t)value32;
        return 1;
    }
    if ((value & (UINT64_C(1) << 63)) != 0u) {
        if (value == (UINT64_C(1) << 63)) {
            *value_out = INT64_MIN;
        } else {
            *value_out = -(int64_t)(~value + 1u);
        }
    } else {
        *value_out = (int64_t)value;
    }
    return 1;
}

static inline int rin_nanosleep_user_encode_signed(uint8_t* bytes,
                                                    uint32_t width,
                                                    int64_t value)
{
    uint64_t encoded;
    uint32_t index;
    if (bytes == NULL || (width != 4u && width != 8u) || value < 0 ||
        (width == 4u && value > INT32_MAX))
        return 0;
    encoded = (uint64_t)value;
    for (index = 0u; index < width; ++index)
        bytes[index] = (uint8_t)(encoded >> (index * 8u));
    return 1;
}

static inline int rin_nanosleep_user_decode_timespec(
    const uint8_t* bytes, uint32_t time_width, uint32_t long_width,
    int64_t* seconds_out, int64_t* nanoseconds_out)
{
    const uint32_t nsec_offset = time_width == 4u && long_width == 8u
        ? 8u : time_width;
    if (rin_nanosleep_timespec_user_size(time_width, long_width) == 0u ||
        !rin_nanosleep_user_decode_signed(bytes, time_width, seconds_out) ||
        !rin_nanosleep_user_decode_signed(bytes + nsec_offset, long_width,
                                          nanoseconds_out))
        return 0;
    return 1;
}

static inline int rin_nanosleep_user_encode_timespec(
    uint8_t* bytes, uint32_t time_width, uint32_t long_width,
    int64_t seconds, int64_t nanoseconds)
{
    const uint32_t nsec_offset = time_width == 4u && long_width == 8u
        ? 8u : time_width;
    if (rin_nanosleep_timespec_user_size(time_width, long_width) == 0u ||
        !rin_nanosleep_user_encode_signed(bytes, time_width, seconds) ||
        !rin_nanosleep_user_encode_signed(bytes + nsec_offset, long_width,
                                          nanoseconds))
        return 0;
    return 1;
}

/* slot 2にRIN_NANOSLEEP_ABI_SELECTOR_V1を渡す場合、slot 1は
 * RinNanosleepRequestV1であり、durationは単調時計基準の相対時間となる。
 * 成功時のremaining欄は0、
 * EINTR時は残時間を非負の秒・ナノ秒で返す。起床精度はscheduler timerの
 * 分解能に従う。USERPTR_V2ではslot 1をRinNanosleepUserRequestV2として扱う。
 * 要求timespecは全体をkernel copyinし、残時間出力はwait前にpinする。絶対期限は
 * realtime／monotonic clockをrequestから選ぶ。その他のselector値では従来の
 * architecture別timespec入力を維持し、slot 2を無視する。 */

#endif /* RIN_NANOSLEEP_ABI_H */
