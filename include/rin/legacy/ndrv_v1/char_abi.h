/* SPDX-License-Identifier: MIT */
#ifndef RIN_API_RIN_LEGACY_DRIVER_V1_CHAR_ABI_H
#define RIN_API_RIN_LEGACY_DRIVER_V1_CHAR_ABI_H

#include <stdint.h>

/* Canonical legacy SDK contract; retained only for the legacy namespace. */

/* Initialisation-time, signed writable-data character-device intent table.
 * The table deliberately carries only a 32-bit pointer to an ops table; the
 * kernel snapshots and range-checks that table before any callback can be
 * published.  It is not a character-device data-plane ABI. */
#define RIN_LEGACY_DRIVER_V1_CHAR_MAGIC UINT32_C(0x31524843) /* CHR1 */
#define RIN_LEGACY_DRIVER_V1_CHAR_MAX_ENTRIES UINT32_C(4)
#define RIN_LEGACY_DRIVER_V1_CHAR_NAME_SIZE UINT32_C(16)
#define RIN_LEGACY_DRIVER_V1_CHAR_OPS_COUNT UINT32_C(5)
#define RIN_LEGACY_DRIVER_V1_CHAR_OPS_SIZE \
    (RIN_LEGACY_DRIVER_V1_CHAR_OPS_COUNT * UINT32_C(4))
#define RIN_LEGACY_DRIVER_V1_CHAR_ENTRY_SIZE UINT32_C(24)
#define RIN_LEGACY_DRIVER_V1_CHAR_WIRE_SIZE \
    (UINT32_C(8) + RIN_LEGACY_DRIVER_V1_CHAR_MAX_ENTRIES * \
         RIN_LEGACY_DRIVER_V1_CHAR_ENTRY_SIZE)

enum {
    RIN_LEGACY_DRIVER_V1_CHAR_OPEN = 0,
    RIN_LEGACY_DRIVER_V1_CHAR_CLOSE = 1,
    RIN_LEGACY_DRIVER_V1_CHAR_READ = 2,
    RIN_LEGACY_DRIVER_V1_CHAR_WRITE = 3,
    RIN_LEGACY_DRIVER_V1_CHAR_IOCTL = 4
};

#endif /* RIN_API_RIN_LEGACY_DRIVER_V1_CHAR_ABI_H */
