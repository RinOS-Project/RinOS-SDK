/* SPDX-License-Identifier: MIT */
/* RinOS seccomp filter and user-notification ABI. */
#ifndef RIN_SDK_SECCOMP_ABI_H
#define RIN_SDK_SECCOMP_ABI_H

#include <stdint.h>

#define RIN_SECCOMP_FILTER_FLAG_NEW_LISTENER UINT32_C(1)
#define RIN_SECCOMP_NOTIF_FLAG_CONTINUE UINT32_C(1)

/* Linux-compatible _IOW/_IOWR request numbers with fixed wire structure sizes. */
#define RIN_SECCOMP_IOCTL_NOTIF_RECV UINT32_C(0xc0502100)
#define RIN_SECCOMP_IOCTL_NOTIF_SEND UINT32_C(0xc0182101)
#define RIN_SECCOMP_IOCTL_NOTIF_ID_VALID UINT32_C(0x40082102)

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

#endif /* RIN_SDK_SECCOMP_ABI_H */
