/* SPDX-License-Identifier: MIT */
/* Process-local NUMA memory-placement policy shared by libc and kernel. */

#ifndef RIN_SDK_NUMA_POLICY_ABI_H
#define RIN_SDK_NUMA_POLICY_ABI_H

#include <stdint.h>

#define RIN_NUMA_POLICY_ABI_VERSION UINT16_C(1)

#define RIN_NUMA_POLICY_OPERATION_GET UINT32_C(1)
#define RIN_NUMA_POLICY_OPERATION_SET UINT32_C(2)

enum {
    RIN_NUMA_POLICY_DEFAULT = 0,
    RIN_NUMA_POLICY_PREFERRED = 1
};

#define RIN_NUMA_POLICY_FLAG_NONE UINT32_C(0)
#define RIN_NUMA_POLICY_PREFERRED_NODE_ANY UINT16_MAX

typedef struct RinNumaMemoryPolicyV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t policy;
    uint16_t preferred_node;
    uint16_t reserved0;
    uint32_t flags;
    uint64_t reserved1;
} RinNumaMemoryPolicyV1;

#if defined(__cplusplus)
static_assert(sizeof(RinNumaMemoryPolicyV1) == 24u,
              "RinNumaMemoryPolicyV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinNumaMemoryPolicyV1) == 24u,
               "RinNumaMemoryPolicyV1 ABI drift");
#endif

#endif /* RIN_SDK_NUMA_POLICY_ABI_H */
