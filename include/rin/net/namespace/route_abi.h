/* SPDX-License-Identifier: MIT */
/* Public namespace-local route configuration ABI. */

#ifndef RIN_SDK_NETWORK_NAMESPACE_ROUTE_ABI_H
#define RIN_SDK_NETWORK_NAMESPACE_ROUTE_ABI_H

#include <stdint.h>

#define RIN_NETWORK_NAMESPACE_ROUTE_SYSCALL_ABI_VERSION UINT16_C(1)

typedef enum RinNetworkNamespaceRouteSyscallOperationV1 {
    RIN_NETWORK_NAMESPACE_ROUTE_OP_ADD = 1,
    RIN_NETWORK_NAMESPACE_ROUTE_OP_REMOVE = 2,
    RIN_NETWORK_NAMESPACE_ROUTE_OP_QUERY = 3,
    RIN_NETWORK_NAMESPACE_ROUTE_OP_LOOKUP = 4
} RinNetworkNamespaceRouteSyscallOperationV1;

typedef struct RinNetworkNamespaceRouteSyscallRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    uint32_t reserved_alignment;
    uint64_t namespace_handle;
    uint64_t route_handle;
    uint32_t address_family;
    uint32_t prefix_length;
    uint8_t destination[16];
    uint8_t gateway[16];
    uint64_t interface_id;
    uint32_t metric;
    uint32_t reserved0;
    uint64_t reserved[1];
} RinNetworkNamespaceRouteSyscallRequestV1;

typedef struct RinNetworkNamespaceRouteSyscallResponseV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    uint32_t reserved_alignment;
    uint64_t namespace_handle;
    uint64_t route_handle;
    uint32_t address_family;
    uint32_t prefix_length;
    uint8_t destination[16];
    uint8_t gateway[16];
    uint64_t interface_id;
    uint32_t metric;
    uint32_t reserved0;
    uint64_t reserved[1];
} RinNetworkNamespaceRouteSyscallResponseV1;

#if defined(__cplusplus)
static_assert(sizeof(RinNetworkNamespaceRouteSyscallRequestV1) == 96u,
              "route syscall request ABI drift");
static_assert(sizeof(RinNetworkNamespaceRouteSyscallResponseV1) == 96u,
              "route syscall response ABI drift");
#else
_Static_assert(sizeof(RinNetworkNamespaceRouteSyscallRequestV1) == 96u,
               "route syscall request ABI drift");
_Static_assert(sizeof(RinNetworkNamespaceRouteSyscallResponseV1) == 96u,
               "route syscall response ABI drift");
#endif

#endif /* RIN_SDK_NETWORK_NAMESPACE_ROUTE_ABI_H */
