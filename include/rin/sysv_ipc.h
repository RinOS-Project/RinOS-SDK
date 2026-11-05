/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_SYSV_IPC_H
#define RIN_SDK_SYSV_IPC_H

#include <stdint.h>

/* System V IPC is exposed through the stable RinOS syscall ABI. These
 * operation numbers intentionally match the traditional SysV command values. */
#define RIN_SYSV_IPC_COMMAND_MESSAGE UINT32_C(1)
#define RIN_SYSV_IPC_COMMAND_SEMAPHORE UINT32_C(2)
#define RIN_SYSV_IPC_COMMAND_SHARED_MEMORY UINT32_C(3)

#define RIN_SYSV_MSGGET UINT32_C(1)
#define RIN_SYSV_MSGSND UINT32_C(2)
#define RIN_SYSV_MSGRCV UINT32_C(3)
#define RIN_SYSV_MSGCTL UINT32_C(4)
#define RIN_SYSV_SEMGET UINT32_C(1)
#define RIN_SYSV_SEMOP UINT32_C(2)
#define RIN_SYSV_SEMTIMEDOP UINT32_C(3)
#define RIN_SYSV_SEMCTL UINT32_C(4)
#define RIN_SYSV_SHMGET UINT32_C(1)
#define RIN_SYSV_SHMAT UINT32_C(2)
#define RIN_SYSV_SHMDT UINT32_C(3)
#define RIN_SYSV_SHMCTL UINT32_C(4)

#define RIN_SYSV_IPC_PRIVATE INT32_C(0)
#define RIN_SYSV_IPC_CREAT UINT32_C(01000)
#define RIN_SYSV_IPC_EXCL UINT32_C(02000)
#define RIN_SYSV_IPC_NOWAIT UINT32_C(04000)
#define RIN_SYSV_IPC_RMID UINT32_C(0)
#define RIN_SYSV_IPC_SET UINT32_C(1)
#define RIN_SYSV_IPC_STAT UINT32_C(2)

#define RIN_SYSV_SHM_RDONLY UINT32_C(010000)
#define RIN_SYSV_SHM_RND UINT32_C(020000)
#define RIN_SYSV_SHM_REMAP UINT32_C(040000)
#define RIN_SYSV_SHM_EXEC UINT32_C(0100000)

#define RIN_SYSV_MSG_NOERROR UINT32_C(010000)
#define RIN_SYSV_MSG_EXCEPT UINT32_C(020000)

#define RIN_SYSV_SEM_UNDO UINT32_C(010000)
#define RIN_SYSV_GETNCNT UINT32_C(3)
#define RIN_SYSV_GETPID UINT32_C(11)
#define RIN_SYSV_GETVAL UINT32_C(12)
#define RIN_SYSV_GETALL UINT32_C(13)
#define RIN_SYSV_GETZCNT UINT32_C(14)
#define RIN_SYSV_SETVAL UINT32_C(16)
#define RIN_SYSV_SETALL UINT32_C(17)

#define RIN_SYSV_MSG_MAX_QUEUES 64u
#define RIN_SYSV_MSG_MAX_BYTES 65536u
#define RIN_SYSV_MSG_MAX_COUNT 256u
#define RIN_SYSV_SEM_MAX_SETS 64u
#define RIN_SYSV_SEM_MAX_PER_SET 256u
#define RIN_SYSV_SEM_MAX_OPS 64u
#define RIN_SYSV_SHM_MAX_SEGMENTS 64u
#define RIN_SYSV_SHM_MAX_BYTES UINT64_C(268435456)

typedef struct RinSysvIpcPerm {
    int32_t key;
    uint32_t uid;
    uint32_t gid;
    uint32_t cuid;
    uint32_t cgid;
    uint32_t mode;
    uint32_t sequence;
    uint32_t reserved0;
    uint64_t reserved[2];
} RinSysvIpcPerm;

typedef struct RinSysvMsqidDs {
    RinSysvIpcPerm perm;
    uint64_t send_time;
    uint64_t receive_time;
    uint64_t change_time;
    uint64_t current_bytes;
    uint64_t message_count;
    uint64_t byte_limit;
    int32_t last_sender_pid;
    int32_t last_receiver_pid;
    uint64_t reserved[2];
} RinSysvMsqidDs;

typedef struct RinSysvSemidDs {
    RinSysvIpcPerm perm;
    uint64_t operation_time;
    uint64_t change_time;
    uint64_t semaphore_count;
    uint64_t reserved[2];
} RinSysvSemidDs;

typedef struct RinSysvShmidDs {
    RinSysvIpcPerm perm;
    uint64_t size;
    uint64_t attach_time;
    uint64_t detach_time;
    uint64_t change_time;
    uint64_t attach_count;
    int32_t creator_pid;
    int32_t last_pid;
    uint64_t reserved[2];
} RinSysvShmidDs;

typedef struct RinSysvSembuf {
    uint16_t sem_num;
    int16_t sem_op;
    int16_t sem_flg;
} RinSysvSembuf;

typedef struct RinSysvTimespec {
    int64_t seconds;
    int64_t nanoseconds;
} RinSysvTimespec;

#endif
