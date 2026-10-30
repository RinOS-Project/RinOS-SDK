/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_TLS_ABI_H
#define RIN_SDK_TLS_ABI_H

#include <stdint.h>

/* RinOS native user-TLS contract shared by the kernel-created TCB/DTV and
 * the userspace runtime.  This is deliberately not an ELF TLS ABI.  V1
 * exposed only a static block in slot one.  V2 keeps that layout and adds
 * bounded dynamic slots, an immutable per-slot size, and a seqlock-style
 * publication generation. */
#define RIN_TLS_ABI_VERSION_V1 UINT32_C(1)
#define RIN_TLS_ABI_VERSION_V2 UINT32_C(2)
#define RIN_TLS_ABI_VERSION RIN_TLS_ABI_VERSION_V2
#define RIN_TLS_DTV_GENERATION_SLOT UINT32_C(0)
#define RIN_TLS_DTV_FIRST_MODULE_SLOT UINT32_C(1)
#define RIN_TLS_DTV_MAX_MODULES UINT32_C(64)
#define RIN_TLS_DTV_LAST_MODULE_SLOT \
    (RIN_TLS_DTV_FIRST_MODULE_SLOT + RIN_TLS_DTV_MAX_MODULES - UINT32_C(1))
#define RIN_TLS_DTV_SLOT_COUNT (RIN_TLS_DTV_LAST_MODULE_SLOT + UINT32_C(1))
#define RIN_TLS_DTV_FIRST_DYNAMIC_SLOT UINT32_C(2)
/* RinTlsIndexV2 contains a slot number but no slot generation. Dynamic DTV
 * indices are therefore never recycled within a process lifetime: unloading
 * a module does not return its index to this budget. */
#define RIN_TLS_DTV_MAX_DYNAMIC_MODULES_LIFETIME \
    (RIN_TLS_DTV_LAST_MODULE_SLOT - RIN_TLS_DTV_FIRST_DYNAMIC_SLOT + \
     UINT32_C(1))
#define RIN_TLS_DTV_PUBLISHED_MODULES UINT32_C(1) /* V1 compatibility */
#define RIN_TLS_TCB_DTV_OFFSET UINT32_C(8)
#define RIN_TLS_TCB_ABI_VERSION_OFFSET UINT32_C(0x30)
#define RIN_TLS_TCB_DTV_CAPACITY_OFFSET UINT32_C(0x38)
#define RIN_TLS_DTV_ENTRY_FLAG_PUBLISHED UINT64_C(1)

typedef struct RinTlsIndexV1 {
    uint64_t module;
    uint64_t offset;
} RinTlsIndexV1;

/* V2 intentionally uses the same index representation.  Keeping a named
 * type makes the ABI version visible at call sites without changing V1
 * objects or the static TLSOFF32S contract. */
typedef struct RinTlsIndexV2 {
    uint64_t module;
    uint64_t offset;
} RinTlsIndexV2;

typedef struct RinTlsDtvEntryV2 {
    uint64_t size;
    uint64_t flags;
} RinTlsDtvEntryV2;

/* `slots[0]` is the publication generation; `slots[1..64]` are block
 * addresses.  Entries are indexed by the same public DTV slot number. */
typedef struct RinTlsDtvV2 {
    uint64_t slots[RIN_TLS_DTV_SLOT_COUNT];
    RinTlsDtvEntryV2 entries[RIN_TLS_DTV_SLOT_COUNT];
} RinTlsDtvV2;

#endif /* RIN_SDK_TLS_ABI_H */
