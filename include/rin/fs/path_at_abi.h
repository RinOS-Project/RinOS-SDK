/* SPDX-License-Identifier: MIT */
/* Stable request ABI for descriptor-relative pathname operations. */

#ifndef RIN_SDK_RIN_PATH_AT_ABI_H
#define RIN_SDK_RIN_PATH_AT_ABI_H

#include <stdint.h>

#define RIN_PATH_AT_CALL_VERSION 1u
#define RIN_PATH_AT_PATH_MAX 1023u

/* Operations are deliberately independent from the host Linux syscall
 * numbers.  The only syscall argument is a pointer to RinPathAtCallV1. */
#define RIN_PATH_AT_OPEN      1u
#define RIN_PATH_AT_ACCESS    2u
#define RIN_PATH_AT_CHOWN     3u
#define RIN_PATH_AT_LINK      4u
#define RIN_PATH_AT_MKDIR     5u
#define RIN_PATH_AT_RENAME    6u
#define RIN_PATH_AT_SYMLINK   7u
#define RIN_PATH_AT_UNLINK    8u
#define RIN_PATH_AT_READLINK  9u
#define RIN_PATH_AT_STAT      10u
#define RIN_PATH_AT_CHMOD     11u
#define RIN_PATH_AT_UTIMENS   12u
/* These two operations intentionally require SYMLINK_NOFOLLOW.  They are
 * the object-metadata counterpart for a copied link: ordinary file metadata
 * remains descriptor-based so pathname lookup cannot become ambient access. */
#define RIN_PATH_AT_COPY_XATTRS     13u
#define RIN_PATH_AT_COPY_FILE_FLAGS 14u

/* Operation-specific flag values match the public POSIX constants.  In
 * particular 0x200 is EACCESS for access and REMOVEDIR for unlink. */
#define RIN_PATH_AT_SYMLINK_NOFOLLOW 0x0100u
#define RIN_PATH_AT_EACCESS          0x0200u
#define RIN_PATH_AT_REMOVEDIR        0x0200u
#define RIN_PATH_AT_SYMLINK_FOLLOW   0x0400u
#define RIN_PATH_AT_NO_AUTOMOUNT      0x0800u
#define RIN_PATH_AT_EMPTY_PATH       0x1000u

typedef struct RinPathAtCallV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    int32_t dirfd;
    int32_t secondary_dirfd;
    uint32_t open_flags;
    uint32_t mode;
    uint32_t owner;
    uint32_t group;
    uint32_t reserved0;
    uint64_t path1;
    uint64_t path2;
    uint64_t buffer;
    uint64_t buffer_size;
} RinPathAtCallV1;

#if defined(__cplusplus)
static_assert(sizeof(RinPathAtCallV1) == 72u,
              "RinPathAtCallV1 ABI size");
#else
_Static_assert(sizeof(RinPathAtCallV1) == 72u,
               "RinPathAtCallV1 ABI size");
#endif

#endif /* RIN_SDK_RIN_PATH_AT_ABI_H */
