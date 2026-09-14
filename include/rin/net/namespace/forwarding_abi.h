/* SPDX-License-Identifier: MIT */
/* Public namespace-local IPv4/IPv6 forwarding policy ABI. */

#ifndef RIN_SDK_NETWORK_NAMESPACE_FORWARDING_ABI_H
#define RIN_SDK_NETWORK_NAMESPACE_FORWARDING_ABI_H

#include <stdint.h>

#define RIN_NETWORK_NAMESPACE_FORWARDING_SYSCALL_ABI_VERSION UINT16_C(1)

typedef enum RinNetworkNamespaceForwardingSyscallOperationV1 {
    RIN_NETWORK_NAMESPACE_FORWARDING_OP_SET = 1,
    RIN_NETWORK_NAMESPACE_FORWARDING_OP_QUERY = 2
} RinNetworkNamespaceForwardingSyscallOperationV1;

typedef struct RinNetworkNamespaceForwardingSyscallRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    uint32_t reserved_alignment;
    uint64_t namespace_handle;
    uint32_t address_family;
    uint32_t enabled;
    uint64_t reserved[3];
} RinNetworkNamespaceForwardingSyscallRequestV1;

typedef struct RinNetworkNamespaceForwardingSyscallResponseV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    uint32_t reserved_alignment;
    uint64_t namespace_handle;
    uint32_t address_family;
    uint32_t enabled;
    uint64_t reserved[3];
} RinNetworkNamespaceForwardingSyscallResponseV1;

#if defined(__cplusplus)
static_assert(sizeof(RinNetworkNamespaceForwardingSyscallRequestV1) == 56u,
              "forwarding syscall request ABI drift");
static_assert(sizeof(RinNetworkNamespaceForwardingSyscallResponseV1) == 56u,
              "forwarding syscall response ABI drift");
#else
_Static_assert(sizeof(RinNetworkNamespaceForwardingSyscallRequestV1) == 56u,
               "forwarding syscall request ABI drift");
_Static_assert(sizeof(RinNetworkNamespaceForwardingSyscallResponseV1) == 56u,
               "forwarding syscall response ABI drift");
#endif

#endif /* RIN_SDK_NETWORK_NAMESPACE_FORWARDING_ABI_H */
