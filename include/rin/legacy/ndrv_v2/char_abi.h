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

_Static_assert(sizeof(RinLegacyDriverV2CharPendingEntry) ==
                   RIN_LEGACY_DRIVER_V2_CHAR_ENTRY_SIZE,
               "legacy v2 CHAR2 entry ABI drift");

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

#endif /* RIN_API_RIN_LEGACY_DRIVER_V2_CHAR_ABI_H */
