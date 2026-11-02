/* SPDX-License-Identifier: MIT */
#ifndef RINOS_SDK_LINUX_SCHED_H
#define RINOS_SDK_LINUX_SCHED_H

#include <stdint.h>

/* clone3() size-versioned argument record (Linux UAPI layout). */
#define CLONE_ARGS_SIZE_VER0 64u
#define CLONE_ARGS_SIZE_VER1 80u
#define CLONE_ARGS_SIZE_VER2 88u

#ifndef CLONE_VM
#define CLONE_VM               0x00000100ULL
#endif
#ifndef CLONE_NEWTIME
#define CLONE_NEWTIME          0x00000080ULL
#endif
#ifndef CLONE_FS
#define CLONE_FS               0x00000200ULL
#endif
#ifndef CLONE_FILES
#define CLONE_FILES            0x00000400ULL
#endif
#ifndef CLONE_SIGHAND
#define CLONE_SIGHAND          0x00000800ULL
#endif
#ifndef CLONE_PIDFD
#define CLONE_PIDFD             0x00001000ULL
#endif
#ifndef CLONE_PTRACE
#define CLONE_PTRACE            0x00002000ULL
#endif
#ifndef CLONE_VFORK
#define CLONE_VFORK             0x00004000ULL
#endif
#ifndef CLONE_PARENT
#define CLONE_PARENT            0x00008000ULL
#endif
#ifndef CLONE_THREAD
#define CLONE_THREAD            0x00010000ULL
#endif
#ifndef CLONE_NEWNS
#define CLONE_NEWNS             0x00020000ULL
#endif
#ifndef CLONE_SYSVSEM
#define CLONE_SYSVSEM           0x00040000ULL
#endif
#ifndef CLONE_SETTLS
#define CLONE_SETTLS            0x00080000ULL
#endif
#ifndef CLONE_PARENT_SETTID
#define CLONE_PARENT_SETTID     0x00100000ULL
#endif
#ifndef CLONE_CHILD_CLEARTID
#define CLONE_CHILD_CLEARTID    0x00200000ULL
#endif
#ifndef CLONE_DETACHED
#define CLONE_DETACHED          0x00400000ULL
#endif
#ifndef CLONE_UNTRACED
#define CLONE_UNTRACED          0x00800000ULL
#endif
#ifndef CLONE_CHILD_SETTID
#define CLONE_CHILD_SETTID      0x01000000ULL
#endif
#ifndef CLONE_NEWCGROUP
#define CLONE_NEWCGROUP         0x02000000ULL
#endif
#ifndef CLONE_NEWUTS
#define CLONE_NEWUTS            0x04000000ULL
#endif
#ifndef CLONE_NEWIPC
#define CLONE_NEWIPC            0x08000000ULL
#endif
#ifndef CLONE_NEWUSER
#define CLONE_NEWUSER           0x10000000ULL
#endif
#ifndef CLONE_NEWPID
#define CLONE_NEWPID            0x20000000ULL
#endif
#ifndef CLONE_NEWNET
#define CLONE_NEWNET            0x40000000ULL
#endif
#ifndef CLONE_IO
#define CLONE_IO                0x80000000ULL
#endif
#ifndef CLONE_CLEAR_SIGHAND
#define CLONE_CLEAR_SIGHAND     0x100000000ULL
#endif
#ifndef CLONE_INTO_CGROUP
#define CLONE_INTO_CGROUP       0x200000000ULL
#endif

struct clone_args {
    uint64_t flags;
    uint64_t pidfd;
    uint64_t child_tid;
    uint64_t parent_tid;
    uint64_t exit_signal;
    uint64_t stack;
    uint64_t stack_size;
    uint64_t tls;
    uint64_t set_tid;
    uint64_t set_tid_size;
    uint64_t cgroup;
};

#endif /* RINOS_SDK_LINUX_SCHED_H */
