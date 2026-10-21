/* SPDX-License-Identifier: MIT */
#ifndef RIN_API_RIN_LEGACY_DRIVER_V2_CHAR_ABI_H
#define RIN_API_RIN_LEGACY_DRIVER_V2_CHAR_ABI_H

#include <stdint.h>

/* Canonical legacy SDK contract; retained only for the legacy namespace. */

/* NDRV v2 native character-device registration mailbox.  The mailbox is
 * writable image data; the loader validates every pointer before publication
 * and never exposes the image's raw pointers to VFS consumers. */
#define RIN_LEGACY_DRIVER_V2_CHAR_MAGIC UINT32_C(0x32524843) /* CHR2 */
#define RIN_LEGACY_DRIVER_V2_CHAR_MAX_ENTRIES UINT32_C(4)
#define RIN_LEGACY_DRIVER_V2_CHAR_NAME_SIZE UINT32_C(16)
#define RIN_LEGACY_DRIVER_V2_CHAR_ENTRY_SIZE UINT32_C(40)
#define RIN_LEGACY_DRIVER_V2_CHAR_WIRE_SIZE \
    (UINT32_C(8) + RIN_LEGACY_DRIVER_V2_CHAR_MAX_ENTRIES * \
         RIN_LEGACY_DRIVER_V2_CHAR_ENTRY_SIZE)

typedef struct __attribute__((packed)) RinLegacyDriverV2CharPendingEntry {
    char name[RIN_LEGACY_DRIVER_V2_CHAR_NAME_SIZE];
    uint64_t device;
    uint64_t ops;
    uint8_t valid;
    uint8_t reserved[7];
} RinLegacyDriverV2CharPendingEntry;

#if defined(__cplusplus)
static_assert(sizeof(RinLegacyDriverV2CharPendingEntry) ==
                  RIN_LEGACY_DRIVER_V2_CHAR_ENTRY_SIZE,
              "legacy v2 CHAR2 entry ABI drift");
#else
_Static_assert(sizeof(RinLegacyDriverV2CharPendingEntry) ==
                   RIN_LEGACY_DRIVER_V2_CHAR_ENTRY_SIZE,
               "legacy v2 CHAR2 entry ABI drift");
#endif

/* Callbacks execute in ordinary process context after the VFS owner has
 * acquired its generation-bound callback lease.  `device` is the optional
 * opaque image-local value recorded in the mailbox. */
typedef struct RinLegacyDriverV2CharOps {
    int (*open)(void* device);
    int (*close)(void* device);
    int (*read)(void* device, void* buffer, uint32_t buffer_size);
    int (*write)(void* device, const void* buffer, uint32_t buffer_size);
    int (*ioctl)(void* device, uint32_t request, void* buffer,
                 uint32_t buffer_size);
} RinLegacyDriverV2CharOps;

/* Optional CHAR2 extension for drivers that need one Core owner per VFS
 * public open.  Every operation receives the kernel-generated public-open
 * cookie; it is never supplied by the driver image or userspace.  This table
 * is registered through CAR1.reserved and does not change the CHAR2 table. */
#define RIN_LEGACY_DRIVER_V2_CHAR_PER_OPEN_ABI_VERSION UINT32_C(1)
typedef struct RinLegacyDriverV2CharPerOpenOpsV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    int (*open)(void* device, uint64_t public_open_cookie);
    int (*close)(void* device, uint64_t public_open_cookie);
    int (*read)(void* device, uint64_t public_open_cookie,
                void* buffer, uint32_t buffer_size);
    int (*write)(void* device, uint64_t public_open_cookie,
                 const void* buffer, uint32_t buffer_size);
    int (*ioctl)(void* device, uint64_t public_open_cookie,
                 uint32_t request, void* buffer, uint32_t buffer_size);
} RinLegacyDriverV2CharPerOpenOpsV1;

#if defined(__cplusplus)
static_assert(sizeof(RinLegacyDriverV2CharPerOpenOpsV1) ==
                  8u + 5u * sizeof(void*),
              "legacy v2 per-open CHAR ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinLegacyDriverV2CharPerOpenOpsV1) ==
                   8u + 5u * sizeof(void*),
               "legacy v2 per-open CHAR ABI drift");
#endif

#endif /* RIN_API_RIN_LEGACY_DRIVER_V2_CHAR_ABI_H */
