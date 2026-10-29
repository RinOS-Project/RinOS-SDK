/* SPDX-License-Identifier: MIT */
/* RinOS ptrace request and event ABI. */
#ifndef RIN_SDK_PTRACE_ABI_H
#define RIN_SDK_PTRACE_ABI_H

#include <stdint.h>

#define RIN_PTRACE_TRACEME          0u
#define RIN_PTRACE_CONT             7u
#define RIN_PTRACE_PEEKDATA         2u
#define RIN_PTRACE_POKEDATA         5u
#define RIN_PTRACE_ATTACH           16u
#define RIN_PTRACE_DETACH           17u
#define RIN_PTRACE_GETREGSET        0x4204u
#define RIN_PTRACE_SETREGSET        0x4205u
#define RIN_PTRACE_SEIZE            0x4206u
#define RIN_PTRACE_INTERRUPT        0x4207u
#define RIN_PTRACE_READ_MEMORY_V1   0x4210u
#define RIN_PTRACE_WRITE_MEMORY_V1  0x4211u
#define RIN_PTRACE_NT_PRSTATUS      1u
#define RIN_PTRACE_SETOPTIONS       0x4200u
#define RIN_PTRACE_GETEVENTMSG      0x4201u
#define RIN_PTRACE_GETSIGINFO       0x4202u

#define RIN_PTRACE_ABI_VERSION_1    1u
#define RIN_PTRACE_ARCH_I386        1u
#define RIN_PTRACE_ARCH_X86_64      2u
#define RIN_PTRACE_MAX_MEMORY_BYTES (64u * 1024u)

/* Syscall convention: ATTACH/SEIZE take option bits in data; GETREGSET and
 * SETREGSET take NT_PRSTATUS in address and an architecture-sized iovec in
 * data, whose base points to RinPtraceRegSetV1. Initialize its version,
 * struct_size and thread_id before GETREGSET; zero thread_id selects any
 * stopped thread. READ_MEMORY_V1/WRITE_MEMORY_V1 take a RinPtraceMemoryV1
 * pointer in data. PEEKDATA/POKEDATA operate on one tracee register-width word.
 */

/* Versioned general register record. Registers are 64-bit slots on the wire;
 * IA-32 and compat32 producers must zero-extend their 32-bit values. gpr is
 * ordered RAX, RBX, RCX, RDX, RSI, RDI, RBP, RSP, R8..R15. GETREGSET uses a
 * versioned record inside an iovec and takes the desired thread_id from its
 * input header (zero selects the oldest stopped thread). */
typedef struct RinPtraceRegistersV1 {
    uint32_t architecture;
    uint32_t register_width;
    uint32_t thread_id;
    uint32_t reserved;
    uint64_t gpr[16];
    uint64_t instruction_pointer;
    uint64_t stack_pointer;
    uint64_t flags;
    uint64_t code_segment;
    uint64_t stack_segment;
    uint64_t fs_base;
    uint64_t gs_base;
} RinPtraceRegistersV1;

typedef struct RinPtraceRegSetV1 {
    uint32_t version;
    uint32_t struct_size;
    uint32_t thread_id;
    uint32_t reserved;
    RinPtraceRegistersV1 registers;
} RinPtraceRegSetV1;

/* The caller buffer and tracee address are separate virtual address spaces.
 * transferred is always the number of bytes committed, including a partial
 * prefix when the next page is inaccessible. */
typedef struct RinPtraceMemoryV1 {
    uint32_t version;
    uint32_t struct_size;
    uint32_t thread_id;
    uint32_t reserved;
    uint64_t tracee_address;
    uint64_t caller_buffer;
    uint64_t length;
    uint64_t transferred;
    uint64_t reserved2;
} RinPtraceMemoryV1;

/* The v1 signal namespace has valid numbers through the realtime signal 62. */
#define RIN_PTRACE_SIGNAL_MAX       62u

/* Stable signal payload used by RIN_PTRACE_GETSIGINFO.  Keep this fixed-size
 * and pointer-free so it is identical for IA-32 and x86_64 callers. */
typedef struct RinPtraceSigInfoV1 {
    int32_t signal_number;
    int32_t error_number;
    int32_t signal_code;
    int32_t reserved0;
    union {
        uint64_t fault_address;
        struct {
            int32_t sender_pid;
            uint32_t sender_uid;
        } sender;
    } fields;
    uint64_t signal_value;
    uint8_t reserved[96];
} RinPtraceSigInfoV1;

#define RIN_PTRACE_O_TRACESECCOMP   0x00000080u
#define RIN_PTRACE_EVENT_SECCOMP    7u

#endif /* RIN_SDK_PTRACE_ABI_H */
