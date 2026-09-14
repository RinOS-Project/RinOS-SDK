/* SPDX-License-Identifier: MIT */
/* Public network-namespace lifecycle ABI. */

#ifndef RIN_SDK_NETWORK_NAMESPACE_ABI_H
#define RIN_SDK_NETWORK_NAMESPACE_ABI_H

#include <stdint.h>

#define RIN_NETWORK_NAMESPACE_SYSCALL_ABI_VERSION UINT16_C(1)

typedef uint64_t RinNetworkNamespaceAbiHandleV1;

typedef enum RinNetworkNamespaceSyscallOperationV1 {
    RIN_NETWORK_NAMESPACE_OP_CREATE_ATTACH = 1,
    RIN_NETWORK_NAMESPACE_OP_ATTACH = 2,
    RIN_NETWORK_NAMESPACE_OP_DETACH = 3,
    RIN_NETWORK_NAMESPACE_OP_DESTROY = 4,
    RIN_NETWORK_NAMESPACE_OP_QUERY = 5,
    /* Move an existing virtual-interface lease into namespace_handle.  The
     * lease identity is carried in interface_handle; the owner keeps the
     * same generation and performs the transfer under one lock. */
    RIN_NETWORK_NAMESPACE_OP_MOVE_INTERFACE = 6
} RinNetworkNamespaceSyscallOperationV1;

typedef struct RinNetworkNamespaceSyscallRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    uint32_t target_uid;
    uint32_t target_process_id;
    uint32_t reserved_alignment;
    uint64_t target_process_instance_cookie;
    RinNetworkNamespaceAbiHandleV1 namespace_handle;
    uint32_t owner_uid;
    uint32_t reserved0;
    RinNetworkNamespaceAbiHandleV1 interface_handle;
    uint64_t reserved[5];
} RinNetworkNamespaceSyscallRequestV1;

typedef struct RinNetworkNamespaceSyscallResponseV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    uint32_t reserved_alignment;
    RinNetworkNamespaceAbiHandleV1 namespace_handle;
    uint64_t namespace_id;
    uint32_t owner_uid;
    uint32_t interface_count;
    uint32_t socket_lease_count;
    uint32_t process_attachment_count;
    RinNetworkNamespaceAbiHandleV1 interface_handle;
    uint64_t reserved[5];
} RinNetworkNamespaceSyscallResponseV1;

#if defined(__cplusplus)
static_assert(sizeof(RinNetworkNamespaceSyscallRequestV1) == 96u,
              "RinNetworkNamespaceSyscallRequestV1 ABI drift");
static_assert(sizeof(RinNetworkNamespaceSyscallResponseV1) == 96u,
              "RinNetworkNamespaceSyscallResponseV1 ABI drift");
#else
_Static_assert(sizeof(RinNetworkNamespaceSyscallRequestV1) == 96u,
               "RinNetworkNamespaceSyscallRequestV1 ABI drift");
_Static_assert(sizeof(RinNetworkNamespaceSyscallResponseV1) == 96u,
               "RinNetworkNamespaceSyscallResponseV1 ABI drift");
#endif

#endif /* RIN_SDK_NETWORK_NAMESPACE_ABI_H */
