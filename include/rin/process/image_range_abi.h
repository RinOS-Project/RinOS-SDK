/* SPDX-License-Identifier: MIT */
/* Public self-only process image range ABI. */

#ifndef RIN_SDK_PROCESS_IMAGE_RANGE_ABI_H
#define RIN_SDK_PROCESS_IMAGE_RANGE_ABI_H

#include <stdint.h>

#define RIN_PROCESS_IMAGE_RANGE_ABI_VERSION UINT16_C(1)
#define RIN_PROCESS_IMAGE_RANGE_FLAG_AUTHENTICATED UINT16_C(0x0001)

/* This range is copied from the calling process's kernel-owned launch/exec
 * image allocation. It is not a mapping query and cannot inspect another
 * process. */
typedef struct RinProcessImageRangeV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t image_base;
    uint64_t image_size;
    uint64_t reserved;
} RinProcessImageRangeV1;

#if !defined(MIDL_PASS) && defined(__cplusplus)
static_assert(sizeof(RinProcessImageRangeV1) == 32u,
              "RinProcessImageRangeV1 ABI drift");
#elif !defined(MIDL_PASS)
_Static_assert(sizeof(RinProcessImageRangeV1) == 32u,
               "RinProcessImageRangeV1 ABI drift");
#endif

#endif /* RIN_SDK_PROCESS_IMAGE_RANGE_ABI_H */
