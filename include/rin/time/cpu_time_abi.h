/* SPDX-License-Identifier: MIT */
/* Fixed-width CPU-time ABI shared by libc and both target kernels. */
#ifndef RIN_SDK_CPU_TIME_ABI_H
#define RIN_SDK_CPU_TIME_ABI_H

/* A freestanding libcxx translation unit may already own the public
 * `_STDINT_H` definitions. Re-importing the compiler's hosted x86_64
 * typedefs there changes uint64_t from Rin's unsigned-long-long ABI and
 * causes a conflicting redeclaration. Kernel/hosted users still include the
 * normal stdint owner when no compatible definition is present. */
#if !defined(_STDINT_H)
#include <stdint.h>
#endif

#include "abi.h"
#include "../syscall_abi.h"

#define RIN_CPU_TIME_SCOPE_PROCESS 1u
#define RIN_CPU_TIME_SCOPE_THREAD  2u

#ifndef MIDL_PASS
static inline int rin_cpu_time_scope_is_valid(uint16_t scope)
{
    return scope == RIN_CPU_TIME_SCOPE_PROCESS ||
           scope == RIN_CPU_TIME_SCOPE_THREAD;
}

static inline int rin_cpu_time_v1_request_is_valid(const RinCpuTimeV1* value)
{
    return value && value->struct_size == sizeof(*value) &&
           value->version == RIN_CPU_TIME_ABI_VERSION &&
           rin_cpu_time_scope_is_valid(value->scope) &&
           value->reserved0 == 0u && value->reserved1 == 0u;
}
#endif /* !MIDL_PASS */

#endif /* RIN_SDK_CPU_TIME_ABI_H */
