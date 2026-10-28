/* SPDX-License-Identifier: MIT */
/* RinOS seccomp filter and user-notification ABI. */
#ifndef RIN_SDK_SECCOMP_ABI_H
#define RIN_SDK_SECCOMP_ABI_H

#include <stdint.h>

#define RIN_SECCOMP_SET_MODE_STRICT UINT32_C(0)
#define RIN_SECCOMP_SET_MODE_FILTER UINT32_C(1)
#define RIN_SECCOMP_GET_ACTION_AVAIL UINT32_C(2)
#define RIN_SECCOMP_GET_NOTIF_SIZES UINT32_C(3)
#define RIN_SECCOMP_FILTER_FLAG_NEW_LISTENER UINT32_C(8)
#define RIN_SECCOMP_RET_KILL_THREAD UINT32_C(0x00000000)
#define RIN_SECCOMP_RET_KILL_PROCESS UINT32_C(0x80000000)
#define RIN_SECCOMP_RET_TRAP UINT32_C(0x00030000)
#define RIN_SECCOMP_RET_ERRNO UINT32_C(0x00050000)
#define RIN_SECCOMP_RET_USER_NOTIF UINT32_C(0x7fc00000)
#define RIN_SECCOMP_RET_TRACE UINT32_C(0x7ff00000)
#define RIN_SECCOMP_RET_LOG UINT32_C(0x7ffc0000)
#define RIN_SECCOMP_RET_ALLOW UINT32_C(0x7fff0000)
#define RIN_SECCOMP_NOTIF_FLAG_CONTINUE UINT32_C(1)

/* Linux-compatible _IOW/_IOWR request numbers with fixed wire structure sizes. */
#define RIN_SECCOMP_IOCTL_NOTIF_RECV UINT32_C(0xc0502100)
#define RIN_SECCOMP_IOCTL_NOTIF_SEND UINT32_C(0xc0182101)
#define RIN_SECCOMP_IOCTL_NOTIF_ID_VALID UINT32_C(0x40082102)

/* NOTIF_RECV blocks for a queued request unless O_NONBLOCK is set; blocked
 * receive is interrupted with -EINTR and poll(POLLIN) reports queued work.
 * Notification creation fails with -ESRCH when the tracee has no TID
 * representable in the listener creator's PID namespace. */

typedef struct RinSeccompNotifData {
    int32_t syscall_number;
    uint32_t architecture;
    uint64_t instruction_pointer;
    uint64_t arguments[6];
} RinSeccompNotifData;

typedef struct RinSeccompNotif {
    uint64_t id;
    uint32_t pid;
    uint32_t flags;
    RinSeccompNotifData data;
} RinSeccompNotif;

typedef struct RinSeccompNotifResp {
    uint64_t id;
    int64_t value;
    int32_t error;
    uint32_t flags;
} RinSeccompNotifResp;

typedef struct RinSeccompNotifSizes {
    uint16_t notification_size;
    uint16_t response_size;
    uint16_t data_size;
} RinSeccompNotifSizes;

#endif /* RIN_SDK_SECCOMP_ABI_H */
