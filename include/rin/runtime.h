/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_RUNTIME_H
#define RIN_SDK_RUNTIME_H

#include "abi.h"
#include "audio/abi.h"
#include "ipc/shm_abi.h"
#include "net/dns_abi.h"

#ifdef __cplusplus
extern "C" {
#endif

/* User-space service runtime operations.  These are backed by the RinOS
 * service/runtime implementation; callers must propagate a non-zero result
 * and must not replace a missing runtime with a success path. */
RIN_SDK_API int rin_service_should_stop(void);
RIN_SDK_API int rin_service_health(uint32_t status);

RIN_SDK_API uint64_t rin_monotonic_ms(void);
RIN_SDK_API unsigned long rin_time(void);
RIN_SDK_API int rin_process_spawn(const char* path,
                                  const char* const* argv,
                                  const char* const* envp);

RIN_SDK_API int rin_dns_getaddrinfo_dual_begin(
    const char* hostname, const char* service, int socktype, int protocol,
    uint32_t flags);
RIN_SDK_API int rin_dns_getaddrinfo_dual_poll(
    uint32_t handle, RinDnsGetAddrInfoDualResult* out);
RIN_SDK_API int rin_dns_resolve_cancel(uint32_t handle);

RIN_SDK_API int rin_shm_get(const char* name, uint32_t size, uint32_t flags);
RIN_SDK_API void* rin_shm_at(int handle, void* address_hint, uint32_t prot);
RIN_SDK_API int rin_shm_dt(int handle, void* address);

/* Transitional libc-backed filesystem entry points used by freestanding
 * services while their callers migrate to the structured file ABI. */
RIN_SDK_API uint32_t fs_open(const char* path, int mode);
RIN_SDK_API int fs_close(uint32_t handle);
RIN_SDK_API int fs_read(uint32_t handle, void* buffer, unsigned long size);
RIN_SDK_API uint64_t fs_size(uint32_t handle);

#define RIN_RUNTIME_PATH_TLS_TRUST_BUNDLE "/system/trust/roots.rinca"

RIN_SDK_API void rin_log(const char* message);
RIN_SDK_API void rin_logf(const char* format, ...);
RIN_SDK_API void rin_sleep(unsigned int milliseconds);

RIN_SDK_API int rin_audio_open(void);
RIN_SDK_API int rin_audio_write(int handle, const void* samples,
                                unsigned long size);
RIN_SDK_API int rin_audio_control(int handle, uint32_t command,
                                  uintptr_t argument);
RIN_SDK_API int rin_audio_get_status(int handle, RinAudioStatusV1* status);
RIN_SDK_API int rin_audio_wait_period(int handle, uint32_t timeout_ms);
RIN_SDK_API int rin_audio_close(int handle);

#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_RUNTIME_H */
