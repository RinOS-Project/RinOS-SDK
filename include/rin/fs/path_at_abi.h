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
/* Create a regular, character, block, or FIFO inode. For this operation only,
 * RinPathAtCallV1.reserved0 carries the 8:8 POSIX device number. */
#define RIN_PATH_AT_MKNOD           15u
/* Secure directory owners need the created inode pinned before another
 * thread can replace its name, and need removal tied to that exact inode. */
#define RIN_PATH_AT_MKDIR_OPEN      16u
#define RIN_PATH_AT_RMDIR_MATCH     17u
/* Descriptor-bound xattr access uses EMPTY_PATH, path1="", and path2 as
 * the attribute name. GET with buffer_size=0 queries value length; GET with
 * a short nonzero buffer returns ERANGE. SET and REMOVE return 0. */
#define RIN_PATH_AT_FGETXATTR       18u
#define RIN_PATH_AT_FSETXATTR       19u
#define RIN_PATH_AT_FREMOVEXATTR    20u
#define RIN_PATH_AT_XATTR_VALUE_MAX 1024u
/* Return the generation-bound identity of an open RinFS descriptor. */
#define RIN_PATH_AT_FGETIDENTITY    21u
#define RIN_PATH_AT_FILE_IDENTITY_V1_VERSION 1u
/* Return the NUL-separated xattr names for an open RinFS descriptor. */
#define RIN_PATH_AT_FLISTXATTR      22u
#define RIN_PATH_AT_XATTR_LIST_MAX  16384u
/* Path-bound xattr operations use path1 as the object and path2 as the name
 * (empty for LIST). SYMLINK_NOFOLLOW targets the final link itself. SET keeps
 * XATTR_CREATE/REPLACE in mode so path resolution flags remain independent. */
#define RIN_PATH_AT_GETXATTR        23u
#define RIN_PATH_AT_LISTXATTR       24u
#define RIN_PATH_AT_SETXATTR        25u
#define RIN_PATH_AT_REMOVEXATTR     26u
/* Remove a relative file or symlink only when it still names the object pinned
 * by secondary_dirfd. dirfd is the parent directory descriptor. */
#define RIN_PATH_AT_UNLINK_MATCH    27u
/* Descriptor-relative stat that rejects ambient path lookup and `..` escape.
 * buffer_size selects the caller's legacy, time64, or 64-bit stat record. */
#define RIN_PATH_AT_STAT_BENEATH    28u

/* FSETXATTR accepts these flags in addition to EMPTY_PATH. */
#define RIN_PATH_AT_XATTR_CREATE    0x0001u
#define RIN_PATH_AT_XATTR_REPLACE   0x0002u

typedef struct RinPathAtFileIdentityV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t mount_id;
    uint64_t object_id;
    uint64_t generation;
} RinPathAtFileIdentityV1;

#if defined(__cplusplus) && !defined(MIDL_PASS)
static_assert(sizeof(RinPathAtFileIdentityV1) == 32u,
              "RinPathAtFileIdentityV1 ABI size");
#elif !defined(MIDL_PASS)
_Static_assert(sizeof(RinPathAtFileIdentityV1) == 32u,
               "RinPathAtFileIdentityV1 ABI size");
#endif

#define RIN_PATH_AT_O_CLOEXEC       0x0200u

/* Operation-specific flag values match the public POSIX constants.  In
 * particular 0x200 is EACCESS for access and REMOVEDIR for unlink. */
#define RIN_PATH_AT_SYMLINK_NOFOLLOW 0x0100u
#define RIN_PATH_AT_EACCESS          0x0200u
#define RIN_PATH_AT_REMOVEDIR        0x0200u
#define RIN_PATH_AT_SYMLINK_FOLLOW   0x0400u
#define RIN_PATH_AT_NO_AUTOMOUNT      0x0800u
#define RIN_PATH_AT_EMPTY_PATH       0x1000u
#define RIN_PATH_AT_RENAME_NOREPLACE 0x0001u
/* Require RENAME to match the device and inode inspected by the caller. */
#define RIN_PATH_AT_RENAME_EXPECT_SOURCE_IDENTITY 0x0002u
#define RIN_PATH_AT_EXPECTED_SOURCE_IDENTITY_VERSION 2u

typedef struct RinPathAtExpectedSourceIdentityV2 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint64_t device;
    uint64_t inode;
    uint64_t generation;
} RinPathAtExpectedSourceIdentityV2;
/* Source-compatible name for clients that use inode-only expected rename.
 * A zero generation keeps the older inode match; new callers should supply it. */
typedef RinPathAtExpectedSourceIdentityV2 RinPathAtExpectedSourceIdentityV1;

#if defined(__cplusplus) && !defined(MIDL_PASS)
static_assert(sizeof(RinPathAtExpectedSourceIdentityV2) == 32u,
              "expected source identity ABI size");
#elif !defined(MIDL_PASS)
_Static_assert(sizeof(RinPathAtExpectedSourceIdentityV2) == 32u,
               "expected source identity ABI size");
#endif

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
    /* Zero for all operations except MKNOD, where this is the 8:8 device. */
    uint32_t reserved0;
    uint64_t path1;
    uint64_t path2;
    /* For RENAME_EXPECT_SOURCE_IDENTITY, this references the expected source
     * identity record. FGETIDENTITY writes RinPathAtFileIdentityV1 here. */
    uint64_t buffer;
    uint64_t buffer_size;
} RinPathAtCallV1;

#if defined(__cplusplus) && !defined(MIDL_PASS)
static_assert(sizeof(RinPathAtCallV1) == 72u,
              "RinPathAtCallV1 ABI size");
#elif !defined(MIDL_PASS)
_Static_assert(sizeof(RinPathAtCallV1) == 72u,
               "RinPathAtCallV1 ABI size");
#endif

#endif /* RIN_SDK_RIN_PATH_AT_ABI_H */
