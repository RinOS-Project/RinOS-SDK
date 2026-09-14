/* SPDX-License-Identifier: MIT */
/* Public fixed-width process-group/session syscall ABI. */

#ifndef RIN_SDK_PROCESS_GROUP_ABI_H
#define RIN_SDK_PROCESS_GROUP_ABI_H

#include <stdint.h>

#define RIN_PROCESS_GROUP_SYSCALL_ABI_VERSION UINT16_C(1)

enum {
    RIN_PROCESS_GROUP_OP_GETPGRP = 1,
    RIN_PROCESS_GROUP_OP_GETSID = 2,
    RIN_PROCESS_GROUP_OP_SETPGID = 3,
    RIN_PROCESS_GROUP_OP_SETSID = 4,
    RIN_PROCESS_GROUP_OP_SNAPSHOT = 5
};

typedef struct RinProcessGroupSyscallRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t target_pid;
    uint32_t process_group_id;
    uint32_t flags;
    uint32_t reserved0;
    uint64_t reserved[5];
} RinProcessGroupSyscallRequestV1;

typedef struct RinProcessGroupSyscallResponseV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    int32_t result;
    uint32_t process_group_id;
    uint32_t session_id;
    uint32_t member_count;
    uint32_t reserved0;
    uint32_t reserved1;
    uint64_t reserved[4];
} RinProcessGroupSyscallResponseV1;

#if defined(__cplusplus)
static_assert(sizeof(RinProcessGroupSyscallRequestV1) == 64u,
              "RinProcessGroupSyscallRequestV1 ABI drift");
static_assert(sizeof(RinProcessGroupSyscallResponseV1) == 64u,
              "RinProcessGroupSyscallResponseV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinProcessGroupSyscallRequestV1) == 64u,
               "RinProcessGroupSyscallRequestV1 ABI drift");
_Static_assert(sizeof(RinProcessGroupSyscallResponseV1) == 64u,
               "RinProcessGroupSyscallResponseV1 ABI drift");
#endif

#endif /* RIN_SDK_PROCESS_GROUP_ABI_H */
