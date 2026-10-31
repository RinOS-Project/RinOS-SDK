/* SPDX-License-Identifier: MIT */
/* Versioned Linux-compatible filesystem-context and mount-FD call ABI. */

#ifndef RIN_SDK_FILESYSTEM_FD_MOUNT_ABI_H
#define RIN_SDK_FILESYSTEM_FD_MOUNT_ABI_H

#include <stdint.h>
#include <rin/syscall_abi.h>

#define RIN_FILESYSTEM_FD_MOUNT_ABI_VERSION UINT16_C(1)
typedef enum RinFilesystemFdMountOperationV1 {
    RIN_FILESYSTEM_FD_MOUNT_OP_OPEN_TREE = 0x101,
    RIN_FILESYSTEM_FD_MOUNT_OP_MOVE_MOUNT = 0x102,
    RIN_FILESYSTEM_FD_MOUNT_OP_MOUNT_SETATTR = 0x103,
    RIN_FILESYSTEM_FD_MOUNT_OP_FSOPEN = 0x104,
    RIN_FILESYSTEM_FD_MOUNT_OP_FSCONFIG = 0x105,
    RIN_FILESYSTEM_FD_MOUNT_OP_FSMOUNT = 0x106,
    RIN_FILESYSTEM_FD_MOUNT_OP_FSPICK = 0x107
} RinFilesystemFdMountOperationV1;

/* args[] is interpreted by operation; every pointer is a zero-extended
 * 64-bit user address.  Signed dirfds are kept in fixed-width fields so the
 * request has the same layout for native IA-32, native x86_64 and compat32. */
typedef struct RinFilesystemFdMountRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    uint32_t reserved0;
    int32_t fd;
    int32_t dirfd;
    int32_t from_dirfd;
    int32_t to_dirfd;
    uint64_t args[8];
    uint64_t reserved[4];
} RinFilesystemFdMountRequestV1;

typedef struct RinFilesystemFdMountResponseV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    uint32_t reserved0;
    int64_t result;
    uint64_t namespace_id;
    uint64_t namespace_generation;
    uint64_t mount_generation;
    uint64_t reserved[10];
} RinFilesystemFdMountResponseV1;

#if defined(__cplusplus)
static_assert(sizeof(RinFilesystemFdMountRequestV1) == 128u,
              "RinFilesystemFdMountRequestV1 ABI drift");
static_assert(sizeof(RinFilesystemFdMountResponseV1) == 128u,
              "RinFilesystemFdMountResponseV1 ABI drift");
#else
_Static_assert(sizeof(RinFilesystemFdMountRequestV1) == 128u,
               "RinFilesystemFdMountRequestV1 ABI drift");
_Static_assert(sizeof(RinFilesystemFdMountResponseV1) == 128u,
               "RinFilesystemFdMountResponseV1 ABI drift");
#endif

#endif /* RIN_SDK_FILESYSTEM_FD_MOUNT_ABI_H */
