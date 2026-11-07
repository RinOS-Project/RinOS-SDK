/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_SERVICE_H
#define RIN_SDK_SERVICE_H

#include "abi.h"
#include "service_directory.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_SERVICE_DEPENDENCY_OPERATION_ACQUIRE 1u
#define RIN_SERVICE_DEPENDENCY_OPERATION_RELEASE 2u
#define RIN_SERVICE_DEPENDENCY_OPERATION_GET_STATE 3u

/* Lifecycle operations are real service-manager requests. The kernel checks
 * the caller's service-control capability; callers must propagate failures. */
RIN_SDK_API RinResult rin_service_start(int scope, const char* id);
RIN_SDK_API RinResult rin_service_stop(int scope, const char* id);
RIN_SDK_API RinResult rin_service_restart(int scope, const char* id);

/* Wait until a system service publishes a generation-tagged UNIX endpoint.
 * On success publication_generation_inout receives the generation to pass
 * back on the next wait. The optional cancellation callback is checked at
 * bounded intervals while the kernel wait sleeps for publication. */
typedef int (*RinServiceWaitCancellationFunctionV1)(void* context);
RIN_SDK_API RinResult rin_service_wait_published(
    const char* id, uint32_t timeout_ms,
    uint32_t* publication_generation_inout,
    RinServiceWaitCancellationFunctionV1 cancellation, void* context);

/* A dependency lease starts one explicitly allowlisted system service for
 * this signed application process and authenticated session. The lease is
 * reclaimed when the process exits; releasing the last dependency-only lease
 * stops the service. These calls do not grant general service-control access.
 * State values use RIN_MANAGEMENT_SERVICE_STATE_* from contract_abi.h. */
RIN_SDK_API RinResult rin_service_dependency_acquire(const char* id);
RIN_SDK_API RinResult rin_service_dependency_release(const char* id);
RIN_SDK_API RinResult rin_service_dependency_get_state(
    const char* id, uint32_t* state_out);

#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_SERVICE_H */
