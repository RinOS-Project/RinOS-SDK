/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_BASE_H
#define RIN_SDK_BASE_H
#include "abi.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Process-local cleanup callback invoked before the kernel object close.
 * The legacy registration below is process-lifetime only. Dynamically
 * unloadable libraries must use the owned registration and unregister token
 * APIs so unregister can stop new callbacks and report in-flight callbacks. */
typedef void (*RinSdkObjectCloseHookV1)(RinObject object);
RIN_SDK_API RinResult rin_sdk_register_object_close_hook_v1(
    RinSdkObjectCloseHookV1 hook);
/* Each owned registration has an independent process-local token. Unregister
 * prevents callbacks not yet claimed by an object close. RIN_ERROR_BUSY means
 * one or more already-claimed callbacks remain in flight; retry the same
 * token, and unload the callback's module only after RIN_SUCCESS. A token is
 * single-use; later calls return RIN_ERROR_STALE_HANDLE. */
typedef uint64_t RinSdkObjectCloseHookTokenV1;
RIN_SDK_API RinResult rin_sdk_register_object_close_hook_owned_v1(
    RinSdkObjectCloseHookV1 hook,
    RinSdkObjectCloseHookTokenV1* token_out);
/* RIN_ERROR_BUSY means the hook is already inactive but one or more callbacks
 * are still running. Retry with the same token; unload is safe only after
 * RIN_SUCCESS. */
RIN_SDK_API RinResult rin_sdk_unregister_object_close_hook_v1(
    RinSdkObjectCloseHookTokenV1 token);

#ifndef RIN_SDK_OBJECT_INFO_ABI_DEFINED
#define RIN_SDK_OBJECT_INFO_ABI_DEFINED
typedef struct RinObjectInfoV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t type;
    uint32_t granted_rights;
    uint32_t flags;
    uint64_t object_id;
    uint64_t reserved[2];
} RinObjectInfoV1;
#endif /* RIN_SDK_OBJECT_INFO_ABI_DEFINED */

#if defined(__cplusplus)
static_assert(sizeof(RinObjectInfoV1) == 40u, "RinObjectInfoV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinObjectInfoV1) == 40u, "RinObjectInfoV1 ABI drift");
#endif

typedef struct RinProcessSpawnV1 {
    uint32_t struct_size;
    uint32_t version;
    RinStringV1 image;
    RinSliceV1 arguments;
    RinSliceV1 environment;
    uint64_t capability_mask;
    uint64_t flags;
    uint64_t reserved[4];
} RinProcessSpawnV1;

typedef struct RinMemoryMapV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t address_hint;
    uint64_t size;
    uint64_t offset;
    RinHandle source;
    uint32_t protection;
    uint32_t flags;
    uint64_t reserved[3];
} RinMemoryMapV1;

typedef struct RinThreadCreateV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t entry;
    uint64_t context;
    uint64_t stack_size;
    uint64_t affinity_mask;
    uint32_t flags;
    uint32_t priority;
    uint64_t reserved[3];
} RinThreadCreateV1;

typedef struct RinTimeV1 {
    uint32_t struct_size;
    uint32_t version;
    int64_t seconds;
    uint32_t nanoseconds;
    uint32_t clock_id;
    uint64_t monotonic_nanoseconds;
    uint64_t reserved[3];
} RinTimeV1;

RIN_SDK_API RinResult rin_object_close_v1(RinObject object);
RIN_SDK_API RinResult rin_object_query_v1(RinObject object, RinObjectInfoV1* info);
RIN_SDK_API RinResult rin_process_spawn_v1(const RinProcessSpawnV1* request, RinProcess* process);
RIN_SDK_API RinResult rin_process_wait_v1(RinProcess process, uint64_t timeout_ns, int32_t* exit_code);
RIN_SDK_API RinResult rin_memory_map_v1(const RinMemoryMapV1* request, uint64_t* address);
RIN_SDK_API RinResult rin_memory_unmap_v1(uint64_t address, uint64_t size);
RIN_SDK_API RinResult rin_time_get_v1(uint32_t clock_id, RinTimeV1* time);
RIN_SDK_API RinResult rin_random_fill_v1(RinSliceV1 buffer);
RIN_SDK_API RinResult rin_log_write_v1(uint32_t level, RinStringV1 message);
RIN_SDK_API RinResult rin_thread_create_v1(const RinThreadCreateV1* request, RinThread* thread);
RIN_SDK_API RinResult rin_thread_join_v1(RinThread thread, uint64_t timeout_ns, int32_t* exit_code);

#ifdef __cplusplus
}
#endif
#endif
