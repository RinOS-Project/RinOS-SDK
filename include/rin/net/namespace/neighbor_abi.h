/* SPDX-License-Identifier: MIT */
/* Public namespace-local ARP/NDP neighbor control ABI. */
#ifndef RIN_SDK_NETWORK_NAMESPACE_NEIGHBOR_ABI_H
#define RIN_SDK_NETWORK_NAMESPACE_NEIGHBOR_ABI_H

#include <stdint.h>

#define RIN_NETWORK_NAMESPACE_NEIGHBOR_SYSCALL_ABI_VERSION UINT16_C(1)

typedef enum RinNetworkNamespaceNeighborSyscallOperationV1 {
    RIN_NETWORK_NAMESPACE_NEIGHBOR_OP_ADD = 1,
    RIN_NETWORK_NAMESPACE_NEIGHBOR_OP_REMOVE = 2,
    RIN_NETWORK_NAMESPACE_NEIGHBOR_OP_QUERY = 3,
    RIN_NETWORK_NAMESPACE_NEIGHBOR_OP_LOOKUP = 4,
    RIN_NETWORK_NAMESPACE_NEIGHBOR_OP_UPDATE = 5
} RinNetworkNamespaceNeighborSyscallOperationV1;

typedef struct RinNetworkNamespaceNeighborSyscallRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    uint32_t reserved_alignment;
    uint64_t namespace_handle;
    uint64_t neighbor_handle;
    uint32_t address_family;
    uint32_t state;
    uint8_t address[16];
    uint64_t interface_id;
    uint8_t link_address[6];
    uint16_t reserved0;
    uint64_t reserved[3];
} RinNetworkNamespaceNeighborSyscallRequestV1;

typedef struct RinNetworkNamespaceNeighborSyscallResponseV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    uint32_t reserved_alignment;
    uint64_t namespace_handle;
    uint64_t neighbor_handle;
    uint32_t address_family;
    uint32_t state;
    uint8_t address[16];
    uint64_t interface_id;
    uint8_t link_address[6];
    uint16_t reserved0;
    uint64_t reserved[3];
} RinNetworkNamespaceNeighborSyscallResponseV1;

#if defined(__cplusplus)
static_assert(sizeof(RinNetworkNamespaceNeighborSyscallRequestV1) == 96u,
              "neighbor syscall request ABI drift");
static_assert(sizeof(RinNetworkNamespaceNeighborSyscallResponseV1) == 96u,
              "neighbor syscall response ABI drift");
#else
_Static_assert(sizeof(RinNetworkNamespaceNeighborSyscallRequestV1) == 96u,
               "neighbor syscall request ABI drift");
_Static_assert(sizeof(RinNetworkNamespaceNeighborSyscallResponseV1) == 96u,
               "neighbor syscall response ABI drift");
#endif

#endif /* RIN_SDK_NETWORK_NAMESPACE_NEIGHBOR_ABI_H */
