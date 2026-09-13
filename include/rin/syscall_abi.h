/* SPDX-License-Identifier: MIT */
/* Stable syscall numbers used by the public RinCRT bootstrap. */

#ifndef RIN_SDK_SYSCALL_ABI_H
#define RIN_SDK_SYSCALL_ABI_H

#define RIN_SYSCALL_SCHEMA_VERSION 2u
#define RIN_SYSCALL_ABI_MAJOR 1u
#define RIN_SYSCALL_ABI_MINOR 0u

/* process */
#define RIN_SYS_EXIT 0u
#define RIN_SYS_FORK 1u
#define RIN_SYS_EXEC 2u
#define RIN_SYS_WAIT 3u

/* file */
#define RIN_SYS_OPEN 20u
#define RIN_SYS_CLOSE 21u
#define RIN_SYS_READ 22u
#define RIN_SYS_WRITE 23u
#define RIN_SYS_SEEK 24u
#define RIN_SYS_UNLINK 30u
#define RIN_SYS_RENAME 31u

/* memory */
#define RIN_SYS_MMAP 50u
#define RIN_SYS_MUNMAP 51u
#define RIN_SYS_MPROTECT 52u

#endif /* RIN_SDK_SYSCALL_ABI_H */
