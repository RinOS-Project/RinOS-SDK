/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_DIRENT_ABI_H
#define RIN_SDK_DIRENT_ABI_H

#include <stddef.h>
#include <stdint.h>

/* VFAT permits 255 UTF-16 code units. The largest UTF-8 encoding for those
 * units is 765 bytes; callers can pass the extended READDIR64 buffer below. */
#define RIN_DIRENT64_NAME_MAX 765u

#if !defined(MIDL_PASS)
#define RIN_DIRENT64_ALIGN __attribute__((aligned(8)))
#else
#define RIN_DIRENT64_ALIGN
#endif

/* IA-32/native64/compat32 共通の固定長 dirent64 wire record。 */
typedef struct RIN_DIRENT64_ALIGN RinDirent64Abi {
    uint64_t inode;
    int64_t offset;
    uint16_t record_length;
    uint8_t type;
    char name[256];
    uint8_t reserved[5];
} RinDirent64Abi;

/* Variable-length READDIR64 record storage. d_reclen identifies the bytes
 * actually written; records for names up to 255 bytes retain the 280-byte
 * legacy length. The 792-byte object is the maximum-capacity user buffer. */
typedef struct RIN_DIRENT64_ALIGN RinDirent64ExtendedAbi {
    uint64_t inode;
    int64_t offset;
    uint16_t record_length;
    uint8_t type;
    char name[RIN_DIRENT64_NAME_MAX + 1u];
    uint8_t reserved[7];
} RinDirent64ExtendedAbi;

#if defined(__cplusplus) && !defined(MIDL_PASS)
static_assert(sizeof(RinDirent64Abi) == 280u,
              "dirent64 syscall ABI drift");
static_assert(offsetof(RinDirent64Abi, name) == 19u,
              "dirent64 name offset drift");
static_assert(sizeof(RinDirent64ExtendedAbi) == 792u,
              "extended dirent64 buffer size drift");
static_assert(offsetof(RinDirent64ExtendedAbi, name) == 19u,
              "extended dirent64 name offset drift");
#elif !defined(MIDL_PASS)
_Static_assert(sizeof(RinDirent64Abi) == 280u,
               "dirent64 syscall ABI drift");
_Static_assert(offsetof(RinDirent64Abi, name) == 19u,
               "dirent64 name offset drift");
_Static_assert(sizeof(RinDirent64ExtendedAbi) == 792u,
               "extended dirent64 buffer size drift");
_Static_assert(offsetof(RinDirent64ExtendedAbi, name) == 19u,
               "extended dirent64 name offset drift");
#endif

#undef RIN_DIRENT64_ALIGN

#endif
