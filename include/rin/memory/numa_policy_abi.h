/* SPDX-License-Identifier: MIT */
/* Process-local NUMA memory-placement policy shared by libc and kernel. */

#ifndef RIN_SDK_NUMA_POLICY_ABI_H
#define RIN_SDK_NUMA_POLICY_ABI_H

#include <stdint.h>

#define RIN_NUMA_POLICY_ABI_VERSION UINT16_C(1)

#define RIN_NUMA_POLICY_OPERATION_GET UINT32_C(1)
#define RIN_NUMA_POLICY_OPERATION_SET UINT32_C(2)
#define RIN_NUMA_POLICY_OPERATION_GET_THREAD UINT32_C(3)
#define RIN_NUMA_POLICY_OPERATION_SET_THREAD UINT32_C(4)
#define RIN_NUMA_POLICY_OPERATION_GET_RANGE UINT32_C(5)
#define RIN_NUMA_POLICY_OPERATION_SET_RANGE UINT32_C(6)
#define RIN_NUMA_POLICY_OPERATION_MOVE_PAGES UINT32_C(7)
#define RIN_NUMA_POLICY_OPERATION_GET_ALLOWED_NODES UINT32_C(8)
#define RIN_NUMA_POLICY_OPERATION_SET_ALLOWED_NODES UINT32_C(9)
#define RIN_NUMA_POLICY_OPERATION_MIGRATE_PAGES UINT32_C(10)

#define RIN_NUMA_MOVE_PAGES_FLAG_NONE UINT16_C(0)
#define RIN_NUMA_MOVE_PAGES_FLAG_MOVE UINT16_C(2)
#define RIN_NUMA_MOVE_PAGES_FLAG_MOVE_ALL UINT16_C(4)
#define RIN_NUMA_MOVE_PAGES_FLAG_SOFT_OFFLINE UINT16_C(8)
#define RIN_NUMA_MOVE_PAGES_MAX_COUNT UINT32_C(256)

#define RIN_NUMA_RANGE_FLAG_STRICT UINT16_C(1)
#define RIN_NUMA_RANGE_FLAG_MOVE UINT16_C(2)
#define RIN_NUMA_RANGE_FLAG_MOVE_ALL UINT16_C(4)
#define RIN_NUMA_RANGE_MAX_ENFORCE_PAGES UINT32_C(262144)

enum {
    RIN_NUMA_POLICY_DEFAULT = 0,
    RIN_NUMA_POLICY_PREFERRED = 1,
    RIN_NUMA_POLICY_BIND = 2,
    RIN_NUMA_POLICY_INTERLEAVE = 3,
    RIN_NUMA_POLICY_LOCAL = 4
};

#define RIN_NUMA_POLICY_FLAG_NONE UINT32_C(0)
#define RIN_NUMA_POLICY_PREFERRED_NODE_ANY UINT16_MAX
#define RIN_NUMA_POLICY_INHERIT UINT32_MAX

typedef struct RinNumaMemoryPolicyV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t policy;
    uint16_t preferred_node;
    uint16_t reserved0;
    uint32_t flags;
    /* v1 preserved this word for forward compatibility.  It is now the
     * 64-node mask used by BIND and INTERLEAVE, or a one-node mask for
     * PREFERRED.  Zero retains the original preferred_node representation. */
    uint64_t node_mask;
} RinNumaMemoryPolicyV1;

typedef struct RinNumaMemoryPolicyRangeV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t address;
    uint64_t length;
    RinNumaMemoryPolicyV1 policy;
} RinNumaMemoryPolicyRangeV1;

/* User-page placement query/migration request.  The arrays are owned by the
 * caller and contain count 64-bit addresses, signed 32-bit node IDs, and
 * signed 32-bit per-page results respectively.  A null nodes pointer queries
 * placement without migrating pages. */
typedef struct RinNumaMovePagesV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    int32_t process_id; /* zero or current process; other tasks require rights */
    uint32_t flags;
    uint32_t count;
    uint32_t reserved1;
    uint64_t addresses;
    uint64_t nodes;
    uint64_t status;
} RinNumaMovePagesV1;

/* Process-wide page migration request. Fixed-width masks keep the wire ABI
 * identical for IA-32, x86_64, and compat32 callers. */
typedef struct RinNumaMigratePagesV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    int32_t process_id; /* zero means the caller */
    uint32_t maxnode;
    uint32_t node_word_bits;
    uint32_t reserved1;
    uint64_t old_nodes; /* user pointer to unsigned-long node mask */
    uint64_t new_nodes; /* user pointer to unsigned-long node mask */
} RinNumaMigratePagesV1;


/* Process cpuset memory-node allowance.  A zero mask resets the allowance to
 * all online nodes; nonzero masks must contain only online memory nodes. */
typedef struct RinNumaNodeMaskV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint64_t node_mask;
} RinNumaNodeMaskV1;

#if defined(__cplusplus)
static_assert(sizeof(RinNumaMemoryPolicyV1) == 24u,
              "RinNumaMemoryPolicyV1 ABI drift");
static_assert(sizeof(RinNumaMemoryPolicyRangeV1) == 48u,
              "RinNumaMemoryPolicyRangeV1 ABI drift");
static_assert(sizeof(RinNumaMovePagesV1) == 48u,
              "RinNumaMovePagesV1 ABI drift");
static_assert(sizeof(RinNumaMigratePagesV1) == 40u,
              "RinNumaMigratePagesV1 ABI drift");
static_assert(sizeof(RinNumaNodeMaskV1) == 16u,
              "RinNumaNodeMaskV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinNumaMemoryPolicyV1) == 24u,
               "RinNumaMemoryPolicyV1 ABI drift");
_Static_assert(sizeof(RinNumaMemoryPolicyRangeV1) == 48u,
               "RinNumaMemoryPolicyRangeV1 ABI drift");
_Static_assert(sizeof(RinNumaMovePagesV1) == 48u,
               "RinNumaMovePagesV1 ABI drift");
_Static_assert(sizeof(RinNumaMigratePagesV1) == 40u,
               "RinNumaMigratePagesV1 ABI drift");
_Static_assert(sizeof(RinNumaNodeMaskV1) == 16u,
               "RinNumaNodeMaskV1 ABI drift");
#endif

#endif /* RIN_SDK_NUMA_POLICY_ABI_H */
