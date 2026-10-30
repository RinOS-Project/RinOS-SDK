/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_SERVICE_H
#define RIN_SDK_SERVICE_H

#include "abi.h"
#include "service_directory.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Lifecycle operations are real service-manager requests. The kernel checks
 * the caller's service-control capability; callers must propagate failures. */
RIN_SDK_API RinResult rin_service_start(int scope, const char* id);
RIN_SDK_API RinResult rin_service_stop(int scope, const char* id);
RIN_SDK_API RinResult rin_service_restart(int scope, const char* id);

#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_SERVICE_H */
