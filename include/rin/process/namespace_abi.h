/* SPDX-License-Identifier: MIT */
/* Public process-namespace inspection ABI. */
#ifndef RIN_SDK_PROCESS_NAMESPACE_ABI_H
#define RIN_SDK_PROCESS_NAMESPACE_ABI_H

#include <stdint.h>

#define RIN_PROCESS_NAMESPACE_INSPECT_ABI_VERSION UINT16_C(1)

typedef struct RinProcessNamespaceInspectRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t local_pid;
    uint32_t reserved0;
    uint64_t reserved[4];
} RinProcessNamespaceInspectRequestV1;

typedef struct RinProcessNamespaceInspectResponseV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t namespace_handle;
    uint32_t local_pid;
    uint32_t process_state;
    int32_t exit_code;
    int32_t management_priority;
    uint32_t management_revision;
    uint32_t management_suspended;
    uint32_t is_zombie;
    uint32_t target_uid;
    uint32_t target_process_id;
    uint32_t reserved_alignment;
    uint64_t target_process_instance_cookie;
    uint64_t management_cpu_ticks;
    uint64_t cpu_time_ns;
    uint64_t reserved[4];
} RinProcessNamespaceInspectResponseV1;

#if !defined(MIDL_PASS) && defined(__cplusplus)
static_assert(sizeof(RinProcessNamespaceInspectRequestV1) == 48u,
              "RinProcessNamespaceInspectRequestV1 ABI drift");
static_assert(sizeof(RinProcessNamespaceInspectResponseV1) == 112u,
              "RinProcessNamespaceInspectResponseV1 ABI drift");
#elif !defined(MIDL_PASS)
_Static_assert(sizeof(RinProcessNamespaceInspectRequestV1) == 48u,
               "RinProcessNamespaceInspectRequestV1 ABI drift");
_Static_assert(sizeof(RinProcessNamespaceInspectResponseV1) == 112u,
               "RinProcessNamespaceInspectResponseV1 ABI drift");
#endif

#endif
