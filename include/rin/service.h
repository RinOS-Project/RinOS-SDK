/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_SERVICE_H
#define RIN_SDK_SERVICE_H

#include "abi.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Service-manager scopes are part of the public SDK contract. */
#define RIN_SERVICE_SCOPE_SYSTEM 1
#define RIN_SERVICE_SCOPE_USER 2

/* Start is a real service-manager operation.  The SDK implementation may
 * reject callers without the required capability; it must never be replaced
 * by a success-returning compatibility stub. */
RIN_SDK_API RinResult rin_service_start(int scope, const char* id);

/* Resolve a kernel-published system service slot before trusting the
 * service identity returned by a connected local socket.  A non-zero return
 * is failure; slot_id is cleared on every failure path. */
RIN_SDK_API int rin_service_find_system_slot(const char* service_id,
                                             uint32_t* slot_id);

#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_SERVICE_H */
