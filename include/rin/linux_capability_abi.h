/* SPDX-License-Identifier: MIT */
#ifndef RIN_LINUX_CAPABILITY_ABI_H
#define RIN_LINUX_CAPABILITY_ABI_H

#include <stdint.h>

/* RinOS syscall wire format for the Linux capability compatibility calls.
 * This is distinct from Linux's syscall numbers and from RinCapabilitySet. */
#define RIN_LINUX_CAPABILITY_VERSION_1 UINT32_C(0x19980330)
#define RIN_LINUX_CAPABILITY_VERSION_2 UINT32_C(0x20071026)
#define RIN_LINUX_CAPABILITY_VERSION_3 UINT32_C(0x20080522)
#define RIN_LINUX_CAPABILITY_LAST 40u
#define RIN_LINUX_CAPABILITY_WORDS_V1 1u
#define RIN_LINUX_CAPABILITY_WORDS_V2 2u
#define RIN_LINUX_CAPABILITY_WORDS_V3 2u

typedef struct RinLinuxCapabilityHeaderV1 {
    uint32_t version;
    int32_t pid;
} RinLinuxCapabilityHeaderV1;

typedef struct RinLinuxCapabilityDataV1 {
    uint32_t effective;
    uint32_t permitted;
    uint32_t inheritable;
} RinLinuxCapabilityDataV1;

#endif /* RIN_LINUX_CAPABILITY_ABI_H */
