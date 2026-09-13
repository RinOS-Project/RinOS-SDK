// SPDX-License-Identifier: MIT
#ifndef RIN_SDK_SHM_ABI_H
#define RIN_SDK_SHM_ABI_H

#include "syscall_abi.h"

#define SYS_SHMGET_PRIVATE RIN_SYS_SHMGET
#define SYS_SHMAT_PRIVATE  RIN_SYS_SHMAT
#define SYS_SHMDT_PRIVATE  RIN_SYS_SHMDT

#define RIN_SHM_NAME_MAX 64

#define RIN_SHM_FLAG_CREAT            0x00000001u
#define RIN_SHM_FLAG_EXCL             0x00000002u
#define RIN_SHM_FLAG_UNLINK_ON_CLOSE  0x00000004u
/* Kernel-only handle admission for a POSIX semaphore page.  User syscalls
 * must reject this bit and the matching reserved namespace. */
#define RIN_SHM_FLAG_INTERNAL_POSIX_SEMAPHORE 0x40000000u

#define RIN_SHM_POSIX_SEMAPHORE_PREFIX ".rin.sem:"

#define RIN_SHM_PROT_READ   0x00000001u
#define RIN_SHM_PROT_WRITE  0x00000002u

#endif

