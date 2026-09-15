/* SPDX-License-Identifier: MIT */
#ifndef RIN_FUTIMENS_ABI_H
#define RIN_FUTIMENS_ABI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_FUTIMENS_CALL_VERSION 1u
#define RIN_FUTIMENS_FLAG_TIMES_NULL 0x0001u
#define RIN_FUTIMENS_NSEC_NOW INT32_C(1073741823)
#define RIN_FUTIMENS_NSEC_OMIT INT32_C(1073741822)
#define RIN_UTIMENSAT_FLAG_SYMLINK_NOFOLLOW UINT32_C(0x100)

typedef struct RinFutimensCallV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    int64_t access_seconds;
    int32_t access_nanoseconds;
    uint32_t reserved0;
    int64_t write_seconds;
    int32_t write_nanoseconds;
    uint32_t reserved1;
} RinFutimensCallV1;

#if !defined(MIDL_PASS) && defined(__cplusplus)
static_assert(sizeof(RinFutimensCallV1) == 40u,
              "RinFutimensCallV1 ABI size mismatch");
#elif !defined(MIDL_PASS)
_Static_assert(sizeof(RinFutimensCallV1) == 40u,
               "RinFutimensCallV1 ABI size mismatch");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RIN_FUTIMENS_ABI_H */
