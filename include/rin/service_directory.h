/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_SERVICE_DIRECTORY_H
#define RIN_SDK_SERVICE_DIRECTORY_H

#include "abi.h"
#include "contract_abi.h"

#ifdef __cplusplus
extern "C" {
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
