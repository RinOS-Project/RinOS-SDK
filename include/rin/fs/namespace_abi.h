/* SPDX-License-Identifier: MIT */
/* Public filesystem-namespace lifecycle ABI. */

#ifndef RIN_SDK_FILESYSTEM_NAMESPACE_ABI_H
#define RIN_SDK_FILESYSTEM_NAMESPACE_ABI_H

#include <stdint.h>

#define RIN_FILESYSTEM_NAMESPACE_SYSCALL_ABI_VERSION UINT16_C(1)

typedef uint64_t RinFilesystemNamespaceAbiHandleV1;

typedef struct RinFilesystemNamespaceAbiObjectIdentityV1 {
    uint64_t mount_id;
    uint64_t namespace_id;
    uint64_t object_id;
    uint64_t generation;
    uint32_t node_type;
    uint32_t node_flags;
    uint64_t reserved[2];
} RinFilesystemNamespaceAbiObjectIdentityV1;

typedef enum RinFilesystemNamespaceSyscallOperationV1 {
    RIN_FILESYSTEM_NAMESPACE_OP_CREATE_ATTACH = 1,
    RIN_FILESYSTEM_NAMESPACE_OP_CLONE_ATTACH = 2,
    RIN_FILESYSTEM_NAMESPACE_OP_ATTACH = 3,
    RIN_FILESYSTEM_NAMESPACE_OP_DETACH = 4,
    RIN_FILESYSTEM_NAMESPACE_OP_DESTROY = 5,
    RIN_FILESYSTEM_NAMESPACE_OP_QUERY = 6
} RinFilesystemNamespaceSyscallOperationV1;

/* The ABI carries only stable object identities.  It never carries a path,
 * FD, raw scheduler index, or caller capability bit. */
typedef struct RinFilesystemNamespaceSyscallRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    uint32_t target_uid;
    uint32_t target_process_id;
    uint32_t reserved_alignment;
    uint64_t target_process_instance_cookie;
    RinFilesystemNamespaceAbiHandleV1 namespace_handle;
    uint32_t owner_uid;
    uint32_t reserved0;
    RinFilesystemNamespaceAbiObjectIdentityV1 root;
    uint64_t reserved[3];
} RinFilesystemNamespaceSyscallRequestV1;

typedef struct RinFilesystemNamespaceSyscallResponseV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    uint32_t reserved_alignment;
    RinFilesystemNamespaceAbiHandleV1 namespace_handle;
    uint64_t namespace_id;
    uint32_t owner_uid;
    uint32_t mount_count;
    uint32_t attachment_count;
    uint32_t reserved0;
    RinFilesystemNamespaceAbiObjectIdentityV1 root;
    uint64_t reserved[3];
} RinFilesystemNamespaceSyscallResponseV1;

#if defined(__cplusplus)
static_assert(sizeof(RinFilesystemNamespaceSyscallRequestV1) == 128u,
              "RinFilesystemNamespaceSyscallRequestV1 ABI drift");
static_assert(sizeof(RinFilesystemNamespaceSyscallResponseV1) == 128u,
              "RinFilesystemNamespaceSyscallResponseV1 ABI drift");
static_assert(sizeof(RinFilesystemNamespaceAbiObjectIdentityV1) == 56u,
              "RinFilesystemNamespaceAbiObjectIdentityV1 ABI drift");
#else
_Static_assert(sizeof(RinFilesystemNamespaceSyscallRequestV1) == 128u,
               "RinFilesystemNamespaceSyscallRequestV1 ABI drift");
_Static_assert(sizeof(RinFilesystemNamespaceSyscallResponseV1) == 128u,
               "RinFilesystemNamespaceSyscallResponseV1 ABI drift");
_Static_assert(sizeof(RinFilesystemNamespaceAbiObjectIdentityV1) == 56u,
               "RinFilesystemNamespaceAbiObjectIdentityV1 ABI drift");
#endif

#endif /* RIN_SDK_FILESYSTEM_NAMESPACE_ABI_H */
