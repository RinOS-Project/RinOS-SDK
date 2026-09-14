/* SPDX-License-Identifier: MIT */
/* Public namespace-local multicast membership control ABI. */
#ifndef RIN_SDK_NETWORK_NAMESPACE_MULTICAST_ABI_H
#define RIN_SDK_NETWORK_NAMESPACE_MULTICAST_ABI_H

#include <stdint.h>

#define RIN_NETWORK_NAMESPACE_MULTICAST_SYSCALL_ABI_VERSION UINT16_C(1)

typedef enum RinNetworkNamespaceMulticastSyscallOperationV1 {
    RIN_NETWORK_NAMESPACE_MULTICAST_OP_JOIN = 1,
    RIN_NETWORK_NAMESPACE_MULTICAST_OP_LEAVE = 2,
    RIN_NETWORK_NAMESPACE_MULTICAST_OP_QUERY = 3
} RinNetworkNamespaceMulticastSyscallOperationV1;

typedef struct RinNetworkNamespaceMulticastSyscallRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    uint32_t reserved_alignment;
    uint64_t handle;
    uint64_t namespace_handle;
    uint64_t socket_lease_handle;
    uint64_t interface_id;
    uint32_t address_family;
    uint32_t uid;
    uint32_t process_id;
    uint32_t reserved0;
    uint64_t process_instance_cookie;
    uint8_t group[16];
    uint64_t reserved[1];
} RinNetworkNamespaceMulticastSyscallRequestV1;

typedef struct RinNetworkNamespaceMulticastSyscallResponseV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    uint32_t reserved_alignment;
    uint64_t handle;
    uint64_t namespace_handle;
    uint64_t socket_lease_handle;
    uint64_t interface_id;
    uint32_t address_family;
    uint32_t uid;
    uint32_t process_id;
    uint32_t reserved0;
    uint64_t process_instance_cookie;
    uint8_t group[16];
    uint64_t reserved[1];
} RinNetworkNamespaceMulticastSyscallResponseV1;

#if defined(__cplusplus)
static_assert(sizeof(RinNetworkNamespaceMulticastSyscallRequestV1) == 96u,
              "multicast syscall request ABI drift");
static_assert(sizeof(RinNetworkNamespaceMulticastSyscallResponseV1) == 96u,
              "multicast syscall response ABI drift");
#else
_Static_assert(sizeof(RinNetworkNamespaceMulticastSyscallRequestV1) == 96u,
               "multicast syscall request ABI drift");
_Static_assert(sizeof(RinNetworkNamespaceMulticastSyscallResponseV1) == 96u,
               "multicast syscall response ABI drift");
#endif

#endif /* RIN_SDK_NETWORK_NAMESPACE_MULTICAST_ABI_H */
