/* SPDX-License-Identifier: MIT */
#ifndef RIN_API_RIN_LEGACY_DRIVER_V1_BLOCK_ABI_H
#define RIN_API_RIN_LEGACY_DRIVER_V1_BLOCK_ABI_H

#include <stdint.h>

/* Canonical legacy SDK contract; retained only for the legacy namespace. */

/* Initialisation-time, signed writable-data registration table.  A legacy v1
 * driver can enqueue entries only while drv_init executes; the 64-bit loader
 * validates and snapshots it before publishing any RinBlock backend. */
#define RIN_LEGACY_DRIVER_V1_BLOCK_MAGIC UINT32_C(0x314B4C42) /* BLK1 */
#define RIN_LEGACY_DRIVER_V1_BLOCK_MAX_ENTRIES UINT32_C(4)
#define RIN_LEGACY_DRIVER_V1_BLOCK_NAME_SIZE UINT32_C(16)
#define RIN_LEGACY_DRIVER_V1_BLOCK_ENTRY_SIZE UINT32_C(24)
#define RIN_LEGACY_DRIVER_V1_BLOCK_OPS_COUNT UINT32_C(7)
#define RIN_LEGACY_DRIVER_V1_BLOCK_OPS_SIZE \
    (RIN_LEGACY_DRIVER_V1_BLOCK_OPS_COUNT * UINT32_C(4))
#define RIN_LEGACY_DRIVER_V1_BLOCK_WIRE_SIZE \
    (UINT32_C(8) + RIN_LEGACY_DRIVER_V1_BLOCK_MAX_ENTRIES * \
         RIN_LEGACY_DRIVER_V1_BLOCK_ENTRY_SIZE)

#endif /* RIN_API_RIN_LEGACY_DRIVER_V1_BLOCK_ABI_H */
