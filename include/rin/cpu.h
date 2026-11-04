/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_CPU_H
#define RIN_SDK_CPU_H

#include <stdint.h>
#include "syscall_abi.h"
#include "syscall.h"

/* Read the current CPU and its NUMA node from one kernel snapshot. Either
 * output may be NULL. An unknown node is reported as UINT32_MAX. */
static inline long rin_getcpu_v1(uint32_t* cpu, uint32_t* node)
{
    return syscall(RIN_SYS_GETCPU_V1,
                   (uintptr_t)cpu, (uintptr_t)node);
}

#endif /* RIN_SDK_CPU_H */
