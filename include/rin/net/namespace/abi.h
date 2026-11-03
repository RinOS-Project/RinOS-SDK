/* SPDX-License-Identifier: MIT */
/* Public network-namespace lifecycle ABI. */

#ifndef RIN_SDK_NETWORK_NAMESPACE_ABI_H
#define RIN_SDK_NETWORK_NAMESPACE_ABI_H

#include <stdint.h>

#define RIN_NETWORK_NAMESPACE_SYSCALL_ABI_VERSION UINT16_C(1)
#define RIN_NETWORK_NAMESPACE_SANDBOX_QUERY_DIGEST_BYTES UINT32_C(32)
#define RIN_NETWORK_NAMESPACE_MOVE_INTERFACE_VERSION_V2 UINT16_C(2)

typedef uint64_t RinNetworkNamespaceAbiHandleV1;

typedef enum RinNetworkNamespaceSyscallOperationV1 {
    RIN_NETWORK_NAMESPACE_OP_CREATE_ATTACH = 1,
    RIN_NETWORK_NAMESPACE_OP_ATTACH = 2,
    RIN_NETWORK_NAMESPACE_OP_DETACH = 3,
    RIN_NETWORK_NAMESPACE_OP_DESTROY = 4,
    RIN_NETWORK_NAMESPACE_OP_QUERY = 5,
    /* Legacy move has no replay-safe result contract and the kernel returns
     * ENOSYS. Use MOVE_INTERFACE_V2 for interface transfers. */
    RIN_NETWORK_NAMESPACE_OP_MOVE_INTERFACE = 6,
    /* Return the kernel-owned host/root namespace lease. */
    RIN_NETWORK_NAMESPACE_OP_QUERY_ROOT = 7,
    /* Resolve the namespace only when its kernel-owned Pod identity matches. */
    RIN_NETWORK_NAMESPACE_OP_QUERY_SANDBOX = 8,
    /* Idempotent, result-recoverable interface move. The operation-specific
     * V2 payloads below occupy request/response reserved words. */
    RIN_NETWORK_NAMESPACE_OP_MOVE_INTERFACE_V2 = 9
} RinNetworkNamespaceSyscallOperationV1;

typedef struct RinNetworkNamespaceMoveInterfaceRequestV2 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t operation_id;
    RinNetworkNamespaceAbiHandleV1 source_namespace_handle;
    uint64_t reserved[2];
} RinNetworkNamespaceMoveInterfaceRequestV2;

/* MOVE_INTERFACE_V2 returns this in response.reserved. Retry the original
 * request with the same operation_id after EFAULT; the kernel keeps the
 * captured result until a response copy succeeds. IDs must be nonzero and
 * unique for each outstanding move on an interface lease. */
typedef struct RinNetworkNamespaceMoveInterfaceResultV2 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t operation_id;
    RinNetworkNamespaceAbiHandleV1 source_namespace_handle;
    uint64_t source_namespace_id;
    uint64_t reserved;
} RinNetworkNamespaceMoveInterfaceResultV2;

/* Operation-specific payload copied into request.reserved for QUERY_SANDBOX.
 * A path, numeric namespace ID, or handle is never an identity selector. */
typedef struct RinNetworkNamespaceSandboxQueryV1 {
    uint64_t pod_generation;
    uint8_t pod_uid_digest[RIN_NETWORK_NAMESPACE_SANDBOX_QUERY_DIGEST_BYTES];
} RinNetworkNamespaceSandboxQueryV1;

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
    /* All zero except QUERY_SANDBOX and MOVE_INTERFACE_V2, where this is
     * exactly the operation-specific versioned payload. */
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
static_assert(sizeof(RinNetworkNamespaceSandboxQueryV1) == 40u,
              "RinNetworkNamespaceSandboxQueryV1 ABI drift");
static_assert(sizeof(RinNetworkNamespaceMoveInterfaceRequestV2) == 40u,
              "RinNetworkNamespaceMoveInterfaceRequestV2 ABI drift");
static_assert(sizeof(RinNetworkNamespaceMoveInterfaceResultV2) == 40u,
              "RinNetworkNamespaceMoveInterfaceResultV2 ABI drift");
#else
_Static_assert(sizeof(RinNetworkNamespaceSyscallRequestV1) == 96u,
               "RinNetworkNamespaceSyscallRequestV1 ABI drift");
_Static_assert(sizeof(RinNetworkNamespaceSyscallResponseV1) == 96u,
              "RinNetworkNamespaceSyscallResponseV1 ABI drift");
_Static_assert(sizeof(RinNetworkNamespaceSandboxQueryV1) == 40u,
              "RinNetworkNamespaceSandboxQueryV1 ABI drift");
_Static_assert(sizeof(RinNetworkNamespaceMoveInterfaceRequestV2) == 40u,
               "RinNetworkNamespaceMoveInterfaceRequestV2 ABI drift");
_Static_assert(sizeof(RinNetworkNamespaceMoveInterfaceResultV2) == 40u,
               "RinNetworkNamespaceMoveInterfaceResultV2 ABI drift");
#endif

#endif /* RIN_SDK_NETWORK_NAMESPACE_ABI_H */
