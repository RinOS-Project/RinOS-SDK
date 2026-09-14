/* SPDX-License-Identifier: MIT */
/* Fixed-width public ABI for per-process resource limits. */

#ifndef RIN_SDK_RESOURCE_LIMIT_ABI_H
#define RIN_SDK_RESOURCE_LIMIT_ABI_H

#include <stdint.h>

#define RIN_RESOURCE_LIMIT_ABI_VERSION UINT16_C(1)
#define RIN_RESOURCE_LIMIT_CALL_SIZE UINT32_C(64)
#define RIN_RESOURCE_LIMIT_CALL_FLAG_SET UINT32_C(1)

#define RIN_RESOURCE_LIMIT_CALL_GET UINT16_C(1)
#define RIN_RESOURCE_LIMIT_CALL_SET UINT16_C(2)
#define RIN_RESOURCE_LIMIT_CALL_PRLIMIT UINT16_C(3)

#define RIN_RESOURCE_LIMIT_CALL_NOFILE UINT32_C(7)
#define RIN_RESOURCE_LIMIT_CALL_MEMLOCK UINT32_C(8)
#define RIN_RESOURCE_LIMIT_CALL_AS UINT32_C(9)

typedef struct RinResourceLimitCallRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    uint32_t resource;
    uint32_t target_process_id; /* zero selects the current process. */
    uint32_t reserved0;
    uint64_t target_instance_cookie;
    uint64_t new_current;
    uint64_t new_maximum;
    uint64_t reserved[2];
} RinResourceLimitCallRequestV1;

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
_Static_assert(sizeof(RinResourceLimitCallResponseV1) ==
                   RIN_RESOURCE_LIMIT_CALL_SIZE,
               "resource-limit response ABI drift");
#endif

#endif /* RIN_SDK_RESOURCE_LIMIT_ABI_H */
