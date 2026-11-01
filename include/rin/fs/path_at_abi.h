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
/* Return the generation-bound identity of an open RinFS or namespace-bound
 * tmpfs file/directory descriptor. */
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
/* Creation owner tokens are supplied in buffer before publication. Only
 * OPEN|O_CREAT|O_EXCL, MKDIR, and SYMLINK accept this flag. */
#define RIN_PATH_AT_CREATE_OPERATION_TOKEN 0x10000u
/* Read the immutable 16-byte creation owner token from a pinned RinFS fd. */
#define RIN_PATH_AT_FGETOPERATIONTOKEN 29u
#define RIN_PATH_AT_OPERATION_TOKEN_SIZE 16u
/* Linux-compatible openat2 uses the existing path-at syscall envelope.  The
 * request's buffer points to a size-versioned RinOpenHowV1 record and its
 * buffer_size carries the caller-supplied structure size. */
#define RIN_PATH_AT_OPENAT2 30u

#define RIN_OPEN_HOW_V1_SIZE 24u
#define RIN_OPEN_HOW_MAX_SIZE 4096u
#define RIN_RESOLVE_NO_XDEV       UINT64_C(0x01)
#define RIN_RESOLVE_NO_MAGICLINKS UINT64_C(0x02)
#define RIN_RESOLVE_NO_SYMLINKS   UINT64_C(0x04)
#define RIN_RESOLVE_BENEATH       UINT64_C(0x08)
#define RIN_RESOLVE_IN_ROOT       UINT64_C(0x10)
#define RIN_RESOLVE_CACHED        UINT64_C(0x20)
#define RIN_RESOLVE_ALL (RIN_RESOLVE_NO_XDEV | RIN_RESOLVE_NO_MAGICLINKS | \
                         RIN_RESOLVE_NO_SYMLINKS | RIN_RESOLVE_BENEATH | \
                         RIN_RESOLVE_IN_ROOT | RIN_RESOLVE_CACHED)

typedef struct RinOpenHowV1 {
    uint64_t flags;
    uint64_t mode;
    uint64_t resolve;
} RinOpenHowV1;

#if defined(__cplusplus) && !defined(MIDL_PASS)
static_assert(sizeof(RinOpenHowV1) == RIN_OPEN_HOW_V1_SIZE,
              "RinOpenHowV1 ABI size");
#elif !defined(MIDL_PASS)
_Static_assert(sizeof(RinOpenHowV1) == RIN_OPEN_HOW_V1_SIZE,
               "RinOpenHowV1 ABI size");
#endif

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
#define RIN_PATH_AT_O_CLOFORK       0x20000u

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
