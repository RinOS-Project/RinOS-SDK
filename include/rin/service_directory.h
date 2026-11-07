/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_SERVICE_DIRECTORY_H
#define RIN_SDK_SERVICE_DIRECTORY_H

#include "abi.h"
#include "contract_abi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_SERVICE_PUBLICATION_WAIT_VERSION_V1 UINT32_C(1)

/* A publication wait is generation-bound to one system service slot. The
 * kernel updates result and publication_generation before waking a blocked
 * caller. timeout_ms and after_generation are inputs; output fields are
 * cleared on entry. */
typedef struct RinServicePublicationWaitV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t timeout_ms;
    uint32_t after_generation;
    uint32_t publication_generation;
    int32_t result;
    uint32_t reserved;
} RinServicePublicationWaitV1;

#if defined(__cplusplus)
static_assert(sizeof(RinServicePublicationWaitV1) == 28u,
              "RinServicePublicationWaitV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinServicePublicationWaitV1) == 28u,
               "RinServicePublicationWaitV1 ABI drift");
#endif

/* Resolve a kernel-published system service slot before trusting the
 * service identity returned by a connected local socket.  A non-zero return
 * is failure; slot_id is cleared on every failure path. */
RIN_SDK_API int rin_service_find_system_slot(const char* service_id,
                                              uint32_t* slot_id);

/* Rescan a service directory in the current session. User-scope rescans are
 * confined to the caller's authenticated home directory; system-scope scans
 * require the service-control capability and an administrator session. */
RIN_SDK_API int rin_service_rescan(uint32_t scope);

#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_SERVICE_DIRECTORY_H */
