/* SPDX-License-Identifier: MIT */
/* Fixed-width public ABI for per-process resource limits. */

#ifndef RIN_SDK_RESOURCE_LIMIT_ABI_H
#define RIN_SDK_RESOURCE_LIMIT_ABI_H

#include <stdint.h>

#define RIN_RESOURCE_LIMIT_ABI_LEGACY_VERSION UINT16_C(1)
#define RIN_RESOURCE_LIMIT_ABI_VERSION UINT16_C(2)
#define RIN_RESOURCE_LIMIT_CALL_SIZE UINT32_C(64)
#define RIN_RESOURCE_LIMIT_CALL_FLAG_SET UINT32_C(1)
#define RIN_RESOURCE_LIMIT_CALL_FLAG_LIMIT64 UINT32_C(2)

#define RIN_RESOURCE_LIMIT_CALL_GET UINT16_C(1)
#define RIN_RESOURCE_LIMIT_CALL_SET UINT16_C(2)
#define RIN_RESOURCE_LIMIT_CALL_PRLIMIT UINT16_C(3)
/* Return the current process' effective child-count admission ceiling. */
#define RIN_RESOURCE_LIMIT_CALL_GET_EFFECTIVE_CHILD_MAX UINT16_C(4)

#define RIN_RESOURCE_LIMIT_CALL_CPU UINT32_C(0)
#define RIN_RESOURCE_LIMIT_CALL_FSIZE UINT32_C(1)
#define RIN_RESOURCE_LIMIT_CALL_DATA UINT32_C(2)
#define RIN_RESOURCE_LIMIT_CALL_STACK UINT32_C(3)
#define RIN_RESOURCE_LIMIT_CALL_CORE UINT32_C(4)
#define RIN_RESOURCE_LIMIT_CALL_RSS UINT32_C(5)
#define RIN_RESOURCE_LIMIT_CALL_NPROC UINT32_C(6)
#define RIN_RESOURCE_LIMIT_CALL_NOFILE UINT32_C(7)
#define RIN_RESOURCE_LIMIT_CALL_MEMLOCK UINT32_C(8)
#define RIN_RESOURCE_LIMIT_CALL_AS UINT32_C(9)
#define RIN_RESOURCE_LIMIT_CALL_LOCKS UINT32_C(10)
#define RIN_RESOURCE_LIMIT_CALL_SIGPENDING UINT32_C(11)
#define RIN_RESOURCE_LIMIT_CALL_MSGQUEUE UINT32_C(12)
#define RIN_RESOURCE_LIMIT_CALL_NICE UINT32_C(13)
#define RIN_RESOURCE_LIMIT_CALL_RTPRIO UINT32_C(14)
#define RIN_RESOURCE_LIMIT_CALL_RTTIME UINT32_C(15)
#define RIN_RESOURCE_LIMIT_CALL_COUNT UINT32_C(16)

typedef struct RinResourceLimitCallRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    uint32_t resource;
    uint32_t target_process_id; /* zero selects the current process. */
    uint32_t reserved0;
    /* Zero resolves the process ID to its current generation while holding
     * a scheduler process lease for the complete operation. */
    uint64_t target_instance_cookie;
    uint64_t new_current;
    uint64_t new_maximum;
    /* V2 passes caller rlimit pointers as values for kernel copyin/copyout.
     * V1 callers leave these two words zero. */
    uint64_t new_limit_address;
    uint64_t old_limit_address;
} RinResourceLimitCallRequestV1;

/* Source-compatible name for the current request layout. V1 request
 * instances remain accepted when their former reserved words are zero. */
typedef RinResourceLimitCallRequestV1 RinResourceLimitCallRequestV2;

typedef struct RinResourceLimitCallResponseV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    int32_t status;
    uint32_t resource;
    uint32_t target_process_id;
    uint32_t reserved0;
    uint64_t target_instance_cookie;
    uint64_t current;
    uint64_t maximum;
    uint64_t reserved[2];
} RinResourceLimitCallResponseV1;

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinResourceLimitCallRequestV1) ==
                   RIN_RESOURCE_LIMIT_CALL_SIZE,
               "resource-limit request ABI drift");
_Static_assert(sizeof(RinResourceLimitCallRequestV2) ==
                   RIN_RESOURCE_LIMIT_CALL_SIZE,
               "resource-limit v2 request ABI drift");
_Static_assert(sizeof(RinResourceLimitCallResponseV1) ==
                   RIN_RESOURCE_LIMIT_CALL_SIZE,
               "resource-limit response ABI drift");
#endif

#endif /* RIN_SDK_RESOURCE_LIMIT_ABI_H */
