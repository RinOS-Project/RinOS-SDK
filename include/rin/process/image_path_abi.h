/* SPDX-License-Identifier: MIT */
/* Product-owned current executable image path ABI. */

#ifndef RIN_SDK_PROCESS_IMAGE_PATH_ABI_H
#define RIN_SDK_PROCESS_IMAGE_PATH_ABI_H

#include <stdint.h>

#define RIN_PROCESS_IMAGE_PATH_ABI_VERSION UINT16_C(1)
#define RIN_PROCESS_IMAGE_PATH_MAX UINT32_C(96)
#define RIN_PROCESS_IMAGE_PATH_FLAG_AUTHENTICATED UINT16_C(0x0001)

/* The path is copied from the kernel launch/exec boundary.  It is never
 * reconstructed from a PID, process name, argv[0], or a host /proc view. */
typedef struct RinProcessImagePathResponseV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t length;
    uint32_t reserved0;
    char path[RIN_PROCESS_IMAGE_PATH_MAX];
    uint64_t reserved[2];
} RinProcessImagePathResponseV1;

#if defined(__cplusplus)
static_assert(sizeof(RinProcessImagePathResponseV1) == 128u,
              "RinProcessImagePathResponseV1 ABI drift");
#else
_Static_assert(sizeof(RinProcessImagePathResponseV1) == 128u,
               "RinProcessImagePathResponseV1 ABI drift");
#endif

#endif /* RIN_SDK_PROCESS_IMAGE_PATH_ABI_H */
