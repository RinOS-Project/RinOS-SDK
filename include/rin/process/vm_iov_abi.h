/* SPDX-License-Identifier: MIT */
/* Cross-process vectored memory transfer request ABI. */

#ifndef RIN_SDK_PROCESS_VM_IOV_ABI_H
#define RIN_SDK_PROCESS_VM_IOV_ABI_H

#include <stdint.h>

#define RIN_PROCESS_VM_IOV_ABI_VERSION UINT16_C(1)
#define RIN_PROCESS_VM_IOV_MAX_COUNT UINT32_C(64)
#define RIN_PROCESS_VM_IOV_MAX_BYTES UINT64_C(16777216)

enum {
    RIN_PROCESS_VM_IOV_READ_REMOTE = 1,
    RIN_PROCESS_VM_IOV_WRITE_REMOTE = 2
};

typedef struct RinProcessVmIovec32V1 {
    uint32_t base;
    uint32_t length;
} RinProcessVmIovec32V1;

typedef struct RinProcessVmIovec64V1 {
    uint64_t base;
    uint64_t length;
} RinProcessVmIovec64V1;

typedef struct RinProcessVmIovRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t direction;
    uint32_t flags;
    int32_t target_pid;
    uint32_t local_iov_count;
    uint32_t remote_iov_count;
    uint32_t iovec_width;
    uint32_t reserved0;
    uint64_t local_iov_address;
    uint64_t remote_iov_address;
    uint64_t reserved[2];
} RinProcessVmIovRequestV1;

#if defined(__cplusplus)
static_assert(sizeof(RinProcessVmIovec32V1) == 8u,
              "RinProcessVmIovec32V1 ABI drift");
static_assert(sizeof(RinProcessVmIovec64V1) == 16u,
              "RinProcessVmIovec64V1 ABI drift");
static_assert(sizeof(RinProcessVmIovRequestV1) == 64u,
              "RinProcessVmIovRequestV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinProcessVmIovec32V1) == 8u,
               "RinProcessVmIovec32V1 ABI drift");
_Static_assert(sizeof(RinProcessVmIovec64V1) == 16u,
               "RinProcessVmIovec64V1 ABI drift");
_Static_assert(sizeof(RinProcessVmIovRequestV1) == 64u,
               "RinProcessVmIovRequestV1 ABI drift");
#endif

#endif /* RIN_SDK_PROCESS_VM_IOV_ABI_H */
