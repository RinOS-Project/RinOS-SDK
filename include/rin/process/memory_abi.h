/* SPDX-License-Identifier: MIT */
/* Public self-only process memory telemetry ABI. */

#ifndef RIN_SDK_PROCESS_MEMORY_ABI_H
#define RIN_SDK_PROCESS_MEMORY_ABI_H

#include <stdint.h>

#define RIN_PROCESS_MEMORY_INFO_ABI_VERSION UINT16_C(1)
#define RIN_PROCESS_MEMORY_INFO_FLAG_RESIDENT_VALID UINT16_C(0x0001)
#define RIN_PROCESS_MEMORY_INFO_FLAG_PEAK_VALID     UINT16_C(0x0002)
#define RIN_PROCESS_MEMORY_INFO_FLAG_VIRTUAL_VALID  UINT16_C(0x0004)
#define RIN_PROCESS_MEMORY_INFO_FLAG_MASK \
    (RIN_PROCESS_MEMORY_INFO_FLAG_RESIDENT_VALID | \
     RIN_PROCESS_MEMORY_INFO_FLAG_PEAK_VALID | \
     RIN_PROCESS_MEMORY_INFO_FLAG_VIRTUAL_VALID)

/* This record describes only the calling process.  The kernel never accepts
 * a process id or handle here, so an unprivileged caller cannot turn the
 * telemetry primitive into a cross-process information oracle.  Resident
 * bytes are the current kernel page-accounting aggregate; virtual bytes are
 * the current mapped VMA aggregate. */
typedef struct RinProcessMemoryInfoV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t resident_bytes;
    uint64_t resident_peak_bytes;
    uint64_t virtual_bytes;
    uint64_t virtual_peak_bytes;
    uint64_t reserved;
} RinProcessMemoryInfoV1;

#if !defined(MIDL_PASS) && defined(__cplusplus)
static_assert(sizeof(RinProcessMemoryInfoV1) == 48u,
              "RinProcessMemoryInfoV1 ABI drift");
#elif !defined(MIDL_PASS)
_Static_assert(sizeof(RinProcessMemoryInfoV1) == 48u,
               "RinProcessMemoryInfoV1 ABI drift");
#endif

#endif /* RIN_SDK_PROCESS_MEMORY_ABI_H */
