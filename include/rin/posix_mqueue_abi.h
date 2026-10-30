/* SPDX-License-Identifier: MIT */
/* Stable SDK wire contract used by the POSIX message-queue adapter. */

#ifndef RIN_SDK_POSIX_MQUEUE_ABI_H
#define RIN_SDK_POSIX_MQUEUE_ABI_H

#include <stdint.h>

#define RIN_POSIX_MQ_ABI_VERSION UINT32_C(1)
#define RIN_POSIX_MQ_NAME_MAX UINT32_C(255)
#define RIN_POSIX_MQ_MAX_MESSAGES UINT32_C(10)
#define RIN_POSIX_MQ_MAX_MESSAGE_SIZE UINT32_C(8192)
#define RIN_POSIX_MQ_MAX_QUEUES UINT32_C(64)
#define RIN_POSIX_MQ_MAX_DESCRIPTORS UINT32_C(512)
#define RIN_POSIX_MQ_MAX_WAITERS UINT32_C(1024)
#define RIN_POSIX_MQ_PRIORITY_MAX UINT32_C(32)
#define RIN_POSIX_MQ_DEFAULT_MAX_MESSAGES UINT32_C(10)
#define RIN_POSIX_MQ_DEFAULT_MESSAGE_SIZE UINT32_C(8192)

/* Access mode occupies the low two bits. The remaining values are SDK ABI
 * flags, independent of the target libc's numeric fcntl constants. */
#define RIN_POSIX_MQ_OPEN_ACCESS_MASK UINT32_C(0x03)
#define RIN_POSIX_MQ_OPEN_READ UINT32_C(0x00)
#define RIN_POSIX_MQ_OPEN_WRITE UINT32_C(0x01)
#define RIN_POSIX_MQ_OPEN_READ_WRITE UINT32_C(0x02)
#define RIN_POSIX_MQ_OPEN_CREATE (UINT32_C(1) << 2u)
#define RIN_POSIX_MQ_OPEN_EXCLUSIVE (UINT32_C(1) << 3u)
#define RIN_POSIX_MQ_OPEN_NONBLOCK (UINT32_C(1) << 4u)
#define RIN_POSIX_MQ_OPEN_CLOEXEC (UINT32_C(1) << 5u)
#define RIN_POSIX_MQ_OPEN_FLAGS_ALL \
    (RIN_POSIX_MQ_OPEN_ACCESS_MASK | RIN_POSIX_MQ_OPEN_CREATE | \
     RIN_POSIX_MQ_OPEN_EXCLUSIVE | RIN_POSIX_MQ_OPEN_NONBLOCK | \
     RIN_POSIX_MQ_OPEN_CLOEXEC)

#define RIN_POSIX_MQ_ATTR_NONBLOCK UINT64_C(1)

#define RIN_POSIX_MQ_NOTIFY_NONE UINT32_C(0)
#define RIN_POSIX_MQ_NOTIFY_EVENT UINT32_C(1)
#define RIN_POSIX_MQ_NOTIFY_CANCEL UINT32_C(2)
#define RIN_POSIX_MQ_NOTIFY_SIGNAL UINT32_C(3)
#define RIN_POSIX_MQ_WAIT_TOKEN_INVALID UINT64_C(0)
typedef uint64_t RinPosixMqWaitTokenV1;

/* IPC_POSIX_MQ_RECEIVE の RinSdkArgsV1.value[4] は、任意の優先度出力先
 * (uint32_t*)を表す。0 は出力先なし。優先度のcopyoutが成功するまで
 * kernelはメッセージのdequeueを確定しない。 */
#define RIN_POSIX_MQ_RECEIVE_PRIORITY_OUTPUT_SLOT UINT32_C(4)

typedef struct RinPosixMqAttrV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t mq_flags;
    uint64_t mq_maxmsg;
    uint64_t mq_msgsize;
    uint64_t mq_curmsgs;
    uint64_t reserved[2];
} RinPosixMqAttrV1;

typedef struct RinPosixMqReceiveResultV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t priority;
    uint32_t reserved0;
    uint64_t length;
    uint64_t reserved[2];
} RinPosixMqReceiveResultV1;

#if defined(__cplusplus)
static_assert(sizeof(RinPosixMqAttrV1) == 56u,
              "RinPosixMqAttrV1 ABI drift");
static_assert(sizeof(RinPosixMqReceiveResultV1) == 40u,
              "RinPosixMqReceiveResultV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinPosixMqAttrV1) == 56u,
               "RinPosixMqAttrV1 ABI drift");
_Static_assert(sizeof(RinPosixMqReceiveResultV1) == 40u,
               "RinPosixMqReceiveResultV1 ABI drift");
#endif

#endif /* RIN_SDK_POSIX_MQUEUE_ABI_H */
