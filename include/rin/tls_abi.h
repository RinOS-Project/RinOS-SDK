#ifndef RIN_SDK_TLS_ABI_H
#define RIN_SDK_TLS_ABI_H

#include <stdint.h>

/* SysV x86_64 TLS access contract shared by the kernel-created TCB/DTV and
 * the userspace runtime.  DTV slot zero is the publication generation and
 * slots one through the bounded maximum contain per-module block addresses. */
#define RIN_TLS_ABI_VERSION UINT32_C(1)
#define RIN_TLS_DTV_GENERATION_SLOT UINT32_C(0)
#define RIN_TLS_DTV_FIRST_MODULE_SLOT UINT32_C(1)
#define RIN_TLS_DTV_MAX_MODULES UINT32_C(64)
/* The current kernel publication contains only the static module block. */
#define RIN_TLS_DTV_PUBLISHED_MODULES UINT32_C(1)
#define RIN_TLS_TCB_DTV_OFFSET UINT32_C(8)

typedef struct RinTlsIndexV1 {
    uint64_t module;
    uint64_t offset;
} RinTlsIndexV1;

#endif /* RIN_SDK_TLS_ABI_H */
