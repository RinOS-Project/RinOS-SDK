/* SPDX-License-Identifier: MIT */
/* Linux raw syscall numbers accepted by RinOS compatibility dispatch. */
#ifndef RIN_SDK_LINUX_SYSCALL_ABI_H
#define RIN_SDK_LINUX_SYSCALL_ABI_H

/* Linux assigns 452 to fchmodat2 on i386 and x86_64.  x86_64 compat32
 * processes use the i386 syscall table, where the number is also 452. */
#define RIN_LINUX_NR_FCHMODAT2 452u

/* Linux perf_event_open numbers accepted by RinOS x86 compatibility entry. */
#define RIN_LINUX_NR_PERF_EVENT_OPEN_X86_64 298u
#define RIN_LINUX_NR_PERF_EVENT_OPEN_I386 336u

/* Linux assigns 457/458 to the mount inspection syscalls on x86. */
#define RIN_LINUX_NR_STATMOUNT 457u
#define RIN_LINUX_NR_LISTMOUNT 458u

#endif /* RIN_SDK_LINUX_SYSCALL_ABI_H */
