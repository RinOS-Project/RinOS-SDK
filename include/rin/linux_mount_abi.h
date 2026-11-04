/* SPDX-License-Identifier: MIT */
/* Linux-compatible statmount/listmount request and result ABI. */
#ifndef RIN_SDK_LINUX_MOUNT_ABI_H
#define RIN_SDK_LINUX_MOUNT_ABI_H

#include <stddef.h>
#include <stdint.h>

#define MNT_ID_REQ_SIZE_VER0 24u
#define MNT_ID_REQ_SIZE_VER1 32u

struct mnt_id_req {
    uint32_t size;
    union {
        uint32_t mnt_ns_fd;
        uint32_t mnt_fd;
    };
    uint64_t mnt_id;
    uint64_t param;
    uint64_t mnt_ns_id;
};

struct statmount {
    uint32_t size;
    uint32_t mnt_opts;
    uint64_t mask;
    uint32_t sb_dev_major;
    uint32_t sb_dev_minor;
    uint64_t sb_magic;
    uint32_t sb_flags;
    uint32_t fs_type;
    uint64_t mnt_id;
    uint64_t mnt_parent_id;
    uint32_t mnt_id_old;
    uint32_t mnt_parent_id_old;
    uint64_t mnt_attr;
    uint64_t mnt_propagation;
    uint64_t mnt_peer_group;
    uint64_t mnt_master;
    uint64_t propagate_from;
    uint32_t mnt_root;
    uint32_t mnt_point;
    uint64_t mnt_ns_id;
    uint32_t fs_subtype;
    uint32_t sb_source;
    uint32_t opt_num;
    uint32_t opt_array;
    uint32_t opt_sec_num;
    uint32_t opt_sec_array;
    uint64_t supported_mask;
    uint32_t mnt_uidmap_num;
    uint32_t mnt_uidmap;
    uint32_t mnt_gidmap_num;
    uint32_t mnt_gidmap;
    uint64_t __spare2[43];
    char str[];
};

#define STATMOUNT_SB_BASIC          UINT64_C(0x00000001)
#define STATMOUNT_MNT_BASIC         UINT64_C(0x00000002)
#define STATMOUNT_PROPAGATE_FROM    UINT64_C(0x00000004)
#define STATMOUNT_MNT_ROOT          UINT64_C(0x00000008)
#define STATMOUNT_MNT_POINT         UINT64_C(0x00000010)
#define STATMOUNT_FS_TYPE           UINT64_C(0x00000020)
#define STATMOUNT_MNT_NS_ID         UINT64_C(0x00000040)
#define STATMOUNT_MNT_OPTS          UINT64_C(0x00000080)
#define STATMOUNT_FS_SUBTYPE        UINT64_C(0x00000100)
#define STATMOUNT_SB_SOURCE         UINT64_C(0x00000200)
#define STATMOUNT_OPT_ARRAY         UINT64_C(0x00000400)
#define STATMOUNT_OPT_SEC_ARRAY     UINT64_C(0x00000800)
#define STATMOUNT_SUPPORTED_MASK    UINT64_C(0x00001000)
#define STATMOUNT_MNT_UIDMAP        UINT64_C(0x00002000)
#define STATMOUNT_MNT_GIDMAP        UINT64_C(0x00004000)

#define STATMOUNT_BY_FD             UINT32_C(0x00000001)
#define LSMT_ROOT                  UINT64_MAX
#define LISTMOUNT_REVERSE           UINT32_C(0x00000001)

/* RinFS is a RinOS-specific filesystem and has no Linux-assigned magic. */
#define RIN_LINUX_RINFS_SUPER_MAGIC UINT64_C(0x0052494e4653)

#if defined(__cplusplus)
static_assert(sizeof(struct mnt_id_req) == MNT_ID_REQ_SIZE_VER1,
              "mnt_id_req ABI drift");
static_assert(offsetof(struct statmount, str) == 512u,
              "statmount ABI drift");
#else
_Static_assert(sizeof(struct mnt_id_req) == MNT_ID_REQ_SIZE_VER1,
               "mnt_id_req ABI drift");
_Static_assert(offsetof(struct statmount, str) == 512u,
               "statmount ABI drift");
#endif

#endif /* RIN_SDK_LINUX_MOUNT_ABI_H */
