/* SPDX-License-Identifier: MIT */
/* RinOS ptrace request and event ABI. */
#ifndef RIN_SDK_PTRACE_ABI_H
#define RIN_SDK_PTRACE_ABI_H

#define RIN_PTRACE_TRACEME          0u
#define RIN_PTRACE_CONT             7u
#define RIN_PTRACE_SETOPTIONS       0x4200u
#define RIN_PTRACE_GETEVENTMSG      0x4201u

#define RIN_PTRACE_O_TRACESECCOMP   0x00000080u
#define RIN_PTRACE_EVENT_SECCOMP    7u

#endif /* RIN_SDK_PTRACE_ABI_H */
