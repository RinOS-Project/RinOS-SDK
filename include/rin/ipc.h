/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_IPC_H
#define RIN_SDK_IPC_H
#include "abi.h"
#include "ipc_peer_abi.h"
#include "ipc_limits.h"
#include "posix_mqueue_abi.h"

#define RIN_EVENT_CREATE_FLAG_CLOSE_ON_EXEC UINT32_C(1)
#define RIN_EVENT_CREATE_FLAGS_ALL RIN_EVENT_CREATE_FLAG_CLOSE_ON_EXEC

/* Kernel code imports IPC wire contracts while intentionally suppressing the
 * public convenience aliases in abi.h. Keep this header independently
 * includable without introducing aliases that collide with kernel types. */
#if defined(RIN_SDK_KERNEL_INTERNAL_ABI)
#define RIN_IPC_HANDLE_TYPE uint64_t
#define RIN_IPC_RESULT_TYPE int32_t
#else
#define RIN_IPC_HANDLE_TYPE RinHandle
#define RIN_IPC_RESULT_TYPE RinResult
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RinIpcMessageV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t message_id;
    RinSliceV1 bytes;
    RinSliceV1 handles;
    uint64_t flags;
    uint64_t reserved[3];
} RinIpcMessageV1;

/* Channel calls are non-blocking. Empty receive and queue-full send return
 * RIN_ERROR_WOULD_BLOCK; wait for RIN_WAIT_EVENT_READABLE/WRITABLE with
 * rin_wait_many_v1 or a wait set before retrying. Bulk payloads should be
 * shared-memory objects transferred in `handles`, not large inline messages.
 * A too-small receive buffer returns RIN_ERROR_BUFFER_TOO_SMALL without
 * consuming the queued message. Handle transfer preserves the source handle's
 * exact rights and requires RIN_OBJECT_RIGHT_DUPLICATE. */
#define RIN_CHANNEL_MAX_MESSAGE_BYTES UINT32_C(65536)
#define RIN_CHANNEL_MAX_TRANSFER_HANDLES UINT32_C(16)
#define RIN_CHANNEL_MAX_QUEUED_MESSAGES UINT32_C(32)
#define RIN_CHANNEL_MAX_QUEUED_BYTES UINT32_C(262144)

typedef struct RinWaitItemV1 {
    RIN_IPC_HANDLE_TYPE handle;
    uint32_t events;
    uint32_t observed;
    uint64_t user_tag;
} RinWaitItemV1;

/* Readiness bits are shared with the public RinRuntime EventLoop adapter.
 * The kernel owns only production/dispatch of these observations; the
 * userspace adapter and ordinary applications may consume the ABI. */
#define RIN_WAIT_EVENT_READABLE (UINT32_C(1) << 0u)
#define RIN_WAIT_EVENT_WRITABLE (UINT32_C(1) << 1u)
#define RIN_WAIT_EVENT_ERROR    (UINT32_C(1) << 2u)
#define RIN_WAIT_EVENT_HANGUP   (UINT32_C(1) << 3u)
#define RIN_WAIT_EVENT_ALL      (RIN_WAIT_EVENT_READABLE | \
                                 RIN_WAIT_EVENT_WRITABLE | \
                                 RIN_WAIT_EVENT_ERROR | \
                                 RIN_WAIT_EVENT_HANGUP)

typedef struct RinWaitResultV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t index;
    uint32_t events;
    uint64_t user_tag;
    uint64_t value;
    uint64_t reserved[3];
} RinWaitResultV1;

RIN_SDK_API RIN_IPC_RESULT_TYPE rin_channel_create_v1(RinChannel* first, RinChannel* second);
RIN_SDK_API RIN_IPC_RESULT_TYPE rin_channel_send_v1(RinChannel channel, const RinIpcMessageV1* message);
RIN_SDK_API RIN_IPC_RESULT_TYPE rin_channel_receive_v1(RinChannel channel, RinIpcMessageV1* message);
RIN_SDK_API RIN_IPC_RESULT_TYPE rin_channel_receive_with_peer_v1(
    RinChannel channel, RinIpcMessageV1* message,
    RinIpcPeerIdentityV1* peer_identity_out);
RIN_SDK_API RIN_IPC_RESULT_TYPE rin_event_create_v1(uint32_t flags, RinEvent* event);
RIN_SDK_API RIN_IPC_RESULT_TYPE rin_event_signal_v1(RinEvent event, uint64_t value);
/* 最後に通知された値を返す。未通知の場合はRIN_ERROR_WOULD_BLOCK。 */
RIN_SDK_API RIN_IPC_RESULT_TYPE rin_event_query_v1(RinEvent event, uint64_t* value_out);
RIN_SDK_API RIN_IPC_RESULT_TYPE rin_wait_many_v1(RinSliceV1 items, uint64_t timeout_ns, uint32_t* index);
RIN_SDK_API RIN_IPC_RESULT_TYPE rin_shared_memory_create_v1(uint64_t size, uint32_t flags, RinSharedMemory* memory);
RIN_SDK_API RIN_IPC_RESULT_TYPE rin_service_connect_v1(RinStringV1 name, RinChannel* channel);
RIN_SDK_API RIN_IPC_RESULT_TYPE rin_wait_set_create_v1(uint32_t flags, RinWaitSet* wait_set);
RIN_SDK_API RIN_IPC_RESULT_TYPE rin_wait_set_set_items_v1(RinWaitSet wait_set, RinSliceV1 items);
RIN_SDK_API RIN_IPC_RESULT_TYPE rin_wait_set_wait_v1(RinWaitSet wait_set, uint64_t timeout_ns, RinWaitResultV1* result);
RIN_SDK_API RIN_IPC_RESULT_TYPE rin_service_register_v1(RinStringV1 name, uint32_t flags, RinService* service);
RIN_SDK_API RIN_IPC_RESULT_TYPE rin_service_accept_v1(RinService service, uint64_t timeout_ns, RinChannel* channel);
RIN_SDK_API RIN_IPC_RESULT_TYPE rin_posix_mq_open_v1(
    RinStringV1 name, uint32_t flags, uint32_t mode,
    const RinPosixMqAttrV1* attributes, RinObjectHandle* queue_out);
RIN_SDK_API RIN_IPC_RESULT_TYPE rin_posix_mq_unlink_v1(RinStringV1 name);
RIN_SDK_API RIN_IPC_RESULT_TYPE rin_posix_mq_send_v1(
    RinObjectHandle queue, RinSliceV1 message, uint32_t priority);
RIN_SDK_API RIN_IPC_RESULT_TYPE rin_posix_mq_receive_v1(
    RinObjectHandle queue, RinSliceV1 buffer,
    RinPosixMqReceiveResultV1* result_out);
RIN_SDK_API RIN_IPC_RESULT_TYPE rin_posix_mq_getattr_v1(
    RinObjectHandle queue, RinPosixMqAttrV1* attributes_out);
RIN_SDK_API RIN_IPC_RESULT_TYPE rin_posix_mq_setattr_v1(
    RinObjectHandle queue, const RinPosixMqAttrV1* attributes,
    RinPosixMqAttrV1* old_attributes_out);
RIN_SDK_API RIN_IPC_RESULT_TYPE rin_posix_mq_notify_v1(
    RinObjectHandle queue, RinEvent event, uint32_t action);
/* 受信待ちの開始・終了を記録する。tokenが0なら登録前に受信可能。 */
RIN_SDK_API RIN_IPC_RESULT_TYPE rin_posix_mq_wait_receive_begin_v1(
    RinObjectHandle queue, RinPosixMqWaitTokenV1* token_out);
RIN_SDK_API RIN_IPC_RESULT_TYPE rin_posix_mq_wait_receive_end_v1(
    RinPosixMqWaitTokenV1 token);

#ifdef __cplusplus
}
#endif
#undef RIN_IPC_HANDLE_TYPE
#undef RIN_IPC_RESULT_TYPE
#endif
