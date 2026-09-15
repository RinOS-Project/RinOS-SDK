#ifndef RIN_SDK_IPC_H
#define RIN_SDK_IPC_H
#include "abi.h"
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

typedef struct RinWaitItemV1 {
    RinHandle handle;
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

RIN_SDK_API RinResult rin_channel_create_v1(RinChannel* first, RinChannel* second);
RIN_SDK_API RinResult rin_channel_send_v1(RinChannel channel, const RinIpcMessageV1* message);
RIN_SDK_API RinResult rin_channel_receive_v1(RinChannel channel, RinIpcMessageV1* message);
RIN_SDK_API RinResult rin_event_create_v1(uint32_t flags, RinEvent* event);
RIN_SDK_API RinResult rin_event_signal_v1(RinEvent event, uint64_t value);
RIN_SDK_API RinResult rin_wait_many_v1(RinSliceV1 items, uint64_t timeout_ns, uint32_t* index);
RIN_SDK_API RinResult rin_shared_memory_create_v1(uint64_t size, uint32_t flags, RinSharedMemory* memory);
RIN_SDK_API RinResult rin_service_connect_v1(RinStringV1 name, RinChannel* channel);
RIN_SDK_API RinResult rin_wait_set_create_v1(uint32_t flags, RinWaitSet* wait_set);
RIN_SDK_API RinResult rin_wait_set_set_items_v1(RinWaitSet wait_set, RinSliceV1 items);
RIN_SDK_API RinResult rin_wait_set_wait_v1(RinWaitSet wait_set, uint64_t timeout_ns, RinWaitResultV1* result);
RIN_SDK_API RinResult rin_service_register_v1(RinStringV1 name, uint32_t flags, RinService* service);
RIN_SDK_API RinResult rin_service_accept_v1(RinService service, uint64_t timeout_ns, RinChannel* channel);

#ifdef __cplusplus
}
#endif
#endif
