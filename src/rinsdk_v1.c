#include <rin/sdk.h>
#include <rin/contract_abi.h>
#include <rin/service_directory.h>

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_GUI)
#include <sys/syscall.h>
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_MEDIA)
#include "rinmedia_sdk_client_v1.h"
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_BASE)
#include <limits.h>
#include <stddef.h>
#include <sys/syscall.h>
#define RIN_SDK_OBJECT_CLOSE_HOOK_CAPACITY_V1 8u
#define RIN_SDK_OBJECT_CLOSE_HOOK_SLOT_BITS_V1 8u
#define RIN_SDK_OBJECT_CLOSE_HOOK_SLOT_MASK_V1 UINT64_C(0xff)
#define RIN_SDK_OBJECT_CLOSE_HOOK_GENERATION_MAX_V1 \
    (UINT64_MAX >> RIN_SDK_OBJECT_CLOSE_HOOK_SLOT_BITS_V1)
typedef struct RinSdkObjectCloseHookEntryV1 {
    RinSdkObjectCloseHookV1 hook;
    uint64_t generation;
    uint64_t in_flight;
    uint8_t active;
    uint8_t process_lifetime;
} RinSdkObjectCloseHookEntryV1;
static RinSdkObjectCloseHookEntryV1
    g_object_close_hooks[RIN_SDK_OBJECT_CLOSE_HOOK_CAPACITY_V1];
static volatile uint32_t g_object_close_hooks_lock;
static uint64_t g_object_close_hook_next_generation = 1u;
static uint8_t g_object_close_hook_generation_exhausted;
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_CORE) || \
    defined(RINSDK_BUILD_IPC) || defined(RINSDK_BUILD_NET)
static void rin_sdk_copy_bytes(void* destination, const void* source,
                               uint64_t size) {
    uint8_t* out = (uint8_t*)destination;
    const uint8_t* in = (const uint8_t*)source;
    while (size-- > 0u) *out++ = *in++;
}
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_BASE) || \
    defined(RINSDK_BUILD_IPC) || defined(RINSDK_BUILD_FS) || \
    defined(RINSDK_BUILD_NET) || defined(RINSDK_BUILD_GUI) || \
    defined(RINSDK_BUILD_MEDIA) || defined(RINSDK_BUILD_CONFIG) || \
    defined(RINSDK_BUILD_PKG) || defined(RINSDK_BUILD_DEVICE) || \
    defined(RINSDK_BUILD_GPU)
static void rin_sdk_zero_bytes(void* destination, uint64_t size) {
    uint8_t* out = (uint8_t*)destination;
    while (size-- > 0u) *out++ = 0u;
}
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_CORE)
static RinSdkBackendV1 g_backend;
static RinSdkTargetBackendV1 g_target_backend;
static volatile uint32_t g_backend_state;
#endif

enum {
    BASE_OBJECT_CLOSE = 1, BASE_OBJECT_QUERY, BASE_PROCESS_SPAWN,
    BASE_PROCESS_WAIT, BASE_MEMORY_MAP, BASE_MEMORY_UNMAP, BASE_TIME_GET,
    BASE_RANDOM_FILL, BASE_LOG_WRITE, BASE_THREAD_CREATE, BASE_THREAD_JOIN,
    IPC_CHANNEL_CREATE = 1, IPC_CHANNEL_SEND, IPC_CHANNEL_RECEIVE,
    IPC_EVENT_CREATE, IPC_EVENT_SIGNAL, IPC_WAIT_MANY,
    IPC_SHARED_MEMORY_CREATE, IPC_SERVICE_CONNECT, IPC_WAIT_SET_CREATE,
    IPC_WAIT_SET_SET_ITEMS, IPC_WAIT_SET_WAIT, IPC_SERVICE_REGISTER,
    IPC_SERVICE_ACCEPT, IPC_CHANNEL_RECEIVE_WITH_PEER = 14,
    IPC_POSIX_MQ_OPEN = 15, IPC_POSIX_MQ_UNLINK, IPC_POSIX_MQ_SEND,
    IPC_POSIX_MQ_RECEIVE, IPC_POSIX_MQ_GETATTR, IPC_POSIX_MQ_SETATTR,
    IPC_POSIX_MQ_NOTIFY, IPC_POSIX_MQ_WAIT_RECEIVE_BEGIN,
    IPC_POSIX_MQ_WAIT_RECEIVE_END, IPC_EVENT_QUERY = 24,
    FS_FILE_OPEN = 1, FS_FILE_READ, FS_FILE_WRITE, FS_FILE_FLUSH,
    FS_DIRECTORY_NEXT, FS_PATH_NORMALIZE, FS_FILE_WATCH,
    FS_FILE_READ_ASYNC, FS_FILE_WRITE_ASYNC, FS_FILE_IO_RESULT,
    FS_FILE_IO_CANCEL, FS_DIRECTORY_OPEN, FS_DIRECTORY_REWIND,
    FS_FILE_WATCH_NEXT,
    FS_FILE_CLOSE = RIN_FS_OPERATION_FILE_CLOSE,
    FS_DIRECTORY_CLOSE = RIN_FS_OPERATION_DIRECTORY_CLOSE,
    FS_FILE_SEEK = RIN_FS_OPERATION_FILE_SEEK,
    FS_FILE_TRUNCATE = RIN_FS_OPERATION_FILE_TRUNCATE,
    FS_DIRECTORY_SYNC = RIN_FS_OPERATION_DIRECTORY_SYNC,
    FS_PATH_STAT = RIN_FS_OPERATION_PATH_STAT,
    FS_PATH_MKDIR = RIN_FS_OPERATION_PATH_MKDIR,
    FS_PATH_UNLINK = RIN_FS_OPERATION_PATH_UNLINK,
    FS_PATH_RENAME = RIN_FS_OPERATION_PATH_RENAME,
    FS_PORTAL_OPEN = RIN_FS_OPERATION_PORTAL_OPEN,
    FS_DIRECTORY_NEXT_BATCH = RIN_FS_OPERATION_DIRECTORY_NEXT_BATCH,
    FS_FILE_STAT = RIN_FS_OPERATION_FILE_STAT,
    NET_SOCKET_CREATE = 1, NET_SOCKET_CONNECT, NET_SOCKET_SEND,
    NET_SOCKET_RECEIVE, NET_DNS_RESOLVE, NET_TLS_CONNECT, NET_HTTP_EXECUTE,
    NET_TLS_SEND, NET_TLS_RECEIVE, NET_TLS_SHUTDOWN,
    NET_SOCKET_BIND, NET_SOCKET_LISTEN, NET_SOCKET_ACCEPT,
    NET_SOCKET_SHUTDOWN,
    GUI_WINDOW_CREATE = 1, GUI_WINDOW_NEXT_EVENT, GUI_SURFACE_CREATE,
    GUI_WINDOW_PRESENT, GUI_CLIPBOARD_SET, GUI_CLIPBOARD_GET,
    GUI_DIALOG_SHOW, GUI_NOTIFICATION_SHOW,
    MEDIA_OPEN = 1, MEDIA_IMAGE_DECODE, MEDIA_AUDIO_DECODE, MEDIA_VIDEO_DECODE,
    MEDIA_IMAGE_INFO, MEDIA_AUDIO_INFO,
    CONFIG_OPEN = 1, CONFIG_GET, CONFIG_SET, CONFIG_COMMIT, CONFIG_REMOVE,
    PKG_OPEN = 1, PKG_QUERY, PKG_INSTALL, PKG_REMOVE, PKG_ENUMERATE,
    DEVICE_ENUMERATE = 1, DEVICE_OPEN, DEVICE_CONTROL, DEVICE_QUERY
};

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_BASE) || \
    defined(RINSDK_BUILD_IPC) || defined(RINSDK_BUILD_FS) || \
    defined(RINSDK_BUILD_NET) || defined(RINSDK_BUILD_GUI) || \
    defined(RINSDK_BUILD_MEDIA) || defined(RINSDK_BUILD_PKG) || \
    defined(RINSDK_BUILD_DEVICE) || defined(RINSDK_BUILD_GPU)
static int versioned(const void* value, uint32_t minimum_size) {
    const RinVersionedV1* header = (const RinVersionedV1*)value;
    return header && header->struct_size >= minimum_size &&
           header->version == RIN_SDK_STRUCT_VERSION_1;
}

/* A backend receives a fixed response_size for v1 operations.  A successful
 * callback must therefore publish the exact response shape that the wrapper
 * initialized; accepting a larger or wrong version header would expose an
 * untrusted/ABI-incompatible response as success. */
static int versioned_exact(const void* value, uint32_t expected_size) {
    const RinVersionedV1* header = (const RinVersionedV1*)value;
    return header && header->struct_size == expected_size &&
           header->version == RIN_SDK_STRUCT_VERSION_1;
}

static void reset_versioned_output(void* value, uint32_t size) {
    RinVersionedV1* header = (RinVersionedV1*)value;
    if (!header) return;
    rin_sdk_zero_bytes(value, size);
    header->struct_size = size;
    header->version = RIN_SDK_STRUCT_VERSION_1;
}

static RinResult finalize_versioned_output(RinResult result, void* value,
                                           uint32_t size) {
    if (result == RIN_SUCCESS && !versioned_exact(value, size))
        result = RIN_ERROR_ABI_MISMATCH;
    if (result != RIN_SUCCESS) reset_versioned_output(value, size);
    return result;
}
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_CORE)
static RinResult rin_sdk_target_backend_invoke(
    uint64_t ignored_context, uint32_t library_id, uint32_t operation,
    const void* request, uint32_t request_size,
    void* response, uint32_t response_size) {
    RinSdkTargetInvokeV1 invoke;

    (void)ignored_context;
    invoke = (RinSdkTargetInvokeV1)(uintptr_t)g_target_backend.invoke;
    if (!invoke) return RIN_ERROR_ABI_MISMATCH;
    return invoke(
        g_target_backend.context, g_target_backend.syscall_number,
        (uint64_t)(uintptr_t)request, (uint64_t)request_size,
        (uint64_t)library_id, (uint64_t)operation,
        (uint64_t)(uintptr_t)response, (uint64_t)response_size);
}

RinResult rin_sdk_bind_backend_v1(const RinSdkBackendV1* backend) {
    if (!backend || backend->struct_size < sizeof(RinSdkBackendV1) ||
        backend->version != RIN_SDK_STRUCT_VERSION_1 || !backend->invoke ||
        backend->invoke > (uint64_t)UINTPTR_MAX) {
        return RIN_ERROR_ABI_MISMATCH;
    }
    if (!__sync_bool_compare_and_swap(&g_backend_state, 0u, 1u)) {
        return RIN_ERROR_BUSY;
    }
    rin_sdk_copy_bytes(&g_backend, backend, sizeof(g_backend));
    __atomic_store_n(&g_backend_state, 2u, __ATOMIC_RELEASE);
    return RIN_SUCCESS;
}

RinResult rin_sdk_bind_target_backend_v1(
    const RinSdkTargetBackendV1* backend) {
    if (!backend || backend->struct_size < sizeof(RinSdkTargetBackendV1) ||
        backend->version != RIN_SDK_STRUCT_VERSION_1 ||
        backend->syscall_number == 0u || !backend->invoke ||
        backend->invoke > (uint64_t)UINTPTR_MAX) {
        return RIN_ERROR_ABI_MISMATCH;
    }
    if (!__sync_bool_compare_and_swap(&g_backend_state, 0u, 1u)) {
        return RIN_ERROR_BUSY;
    }
    rin_sdk_copy_bytes(&g_target_backend, backend, sizeof(g_target_backend));
    g_backend.struct_size = sizeof(g_backend);
    g_backend.version = RIN_SDK_STRUCT_VERSION_1;
    g_backend.context = 0u;
    g_backend.invoke = (uint64_t)(uintptr_t)&rin_sdk_target_backend_invoke;
    __atomic_store_n(&g_backend_state, 2u, __ATOMIC_RELEASE);
    return RIN_SUCCESS;
}

RinResult rin_sdk_invoke_v1(uint32_t library_id, uint32_t operation,
                            const void* request, uint32_t request_size,
                            void* response, uint32_t response_size) {
    RinSdkInvokeV1 invoke;
    if (__atomic_load_n(&g_backend_state, __ATOMIC_ACQUIRE) != 2u) {
        return RIN_ERROR_NOT_SUPPORTED;
    }
    if (library_id < RIN_SDK_LIBRARY_BASE || library_id > RIN_SDK_LIBRARY_GPU ||
        operation == 0u || (request_size && !request) || (response_size && !response)) {
        return RIN_ERROR_INVALID_ARGUMENT;
    }
    invoke = (RinSdkInvokeV1)(uintptr_t)g_backend.invoke;
    return invoke(g_backend.context, library_id, operation,
                  request, request_size, response, response_size);
}
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_BASE) || \
    defined(RINSDK_BUILD_IPC) || defined(RINSDK_BUILD_FS) || \
    defined(RINSDK_BUILD_NET) || defined(RINSDK_BUILD_GUI) || \
    defined(RINSDK_BUILD_MEDIA) || defined(RINSDK_BUILD_CONFIG) || \
    defined(RINSDK_BUILD_PKG) || defined(RINSDK_BUILD_DEVICE) || \
    defined(RINSDK_BUILD_GPU)
static RinResult rin_sdk_invoke_scalar_output_v1(
    uint32_t library_id, uint32_t operation, const void* request,
    uint32_t request_size, void* response, uint32_t response_size) {
    RinResult result;
    if (response && response_size) rin_sdk_zero_bytes(response, response_size);
    result = rin_sdk_invoke_v1(library_id, operation, request, request_size,
                               response, response_size);
    if (result != RIN_SUCCESS && response && response_size)
        rin_sdk_zero_bytes(response, response_size);
    return result;
}

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_BASE) || \
    defined(RINSDK_BUILD_IPC) || defined(RINSDK_BUILD_NET) || \
    defined(RINSDK_BUILD_FS) || defined(RINSDK_BUILD_GUI) || \
    defined(RINSDK_BUILD_MEDIA) || defined(RINSDK_BUILD_CONFIG) || \
    defined(RINSDK_BUILD_PKG) || defined(RINSDK_BUILD_DEVICE)
static RinResult rin_sdk_invoke_handle_output_v1(
    uint32_t library_id, uint32_t operation, const void* request,
    uint32_t request_size, RinHandle* response) {
    if (!response) return RIN_ERROR_INVALID_ARGUMENT;
    RinResult result = rin_sdk_invoke_scalar_output_v1(
        library_id, operation, request, request_size, response,
        sizeof(*response));
    if (result == RIN_SUCCESS && response && *response == RIN_HANDLE_INVALID)
        result = RIN_ERROR_ABI_MISMATCH;
    if (result != RIN_SUCCESS && response) *response = RIN_HANDLE_INVALID;
    return result;
}
#endif
#endif

#define SIMPLE_CALL(lib, op, out, a, b, c, d, e, f) do { \
    RinSdkArgsV1 request; \
    rin_sdk_zero_bytes(&request, sizeof(request)); \
    request.struct_size = sizeof(request); \
    request.version = RIN_SDK_STRUCT_VERSION_1; \
    request.value[0] = (a); request.value[1] = (b); request.value[2] = (c); \
    request.value[3] = (d); request.value[4] = (e); request.value[5] = (f); \
    return rin_sdk_invoke_v1((lib), (op), &request, sizeof(request), \
                             (out), (out) ? sizeof(*(out)) : 0u); \
} while (0)

#define VERSIONED_OUTPUT_SIMPLE_CALL(lib, op, out, a, b, c, d, e, f) do { \
    RinSdkArgsV1 request; \
    RinResult call_result; \
    if (!versioned((out), sizeof(*(out)))) return RIN_ERROR_ABI_MISMATCH; \
    rin_sdk_zero_bytes((out), sizeof(*(out))); \
    ((RinVersionedV1 *)(out))->struct_size = sizeof(*(out)); \
    ((RinVersionedV1 *)(out))->version = RIN_SDK_STRUCT_VERSION_1; \
    rin_sdk_zero_bytes(&request, sizeof(request)); \
    request.struct_size = sizeof(request); \
    request.version = RIN_SDK_STRUCT_VERSION_1; \
    request.value[0] = (a); request.value[1] = (b); request.value[2] = (c); \
    request.value[3] = (d); request.value[4] = (e); request.value[5] = (f); \
    call_result = rin_sdk_invoke_v1((lib), (op), &request, sizeof(request), \
                                    (out), sizeof(*(out))); \
    return finalize_versioned_output(call_result, (out), sizeof(*(out))); \
} while (0)

#define SCALAR_OUTPUT_SIMPLE_CALL(lib, op, out, a, b, c, d, e, f) do { \
    RinSdkArgsV1 request; \
    RinResult call_result; \
    if ((out) != NULL) *(out) = 0; \
    rin_sdk_zero_bytes(&request, sizeof(request)); \
    request.struct_size = sizeof(request); \
    request.version = RIN_SDK_STRUCT_VERSION_1; \
    request.value[0] = (a); request.value[1] = (b); request.value[2] = (c); \
    request.value[3] = (d); request.value[4] = (e); request.value[5] = (f); \
    call_result = rin_sdk_invoke_v1((lib), (op), &request, sizeof(request), \
                                    (out), (out) ? sizeof(*(out)) : 0u); \
    if (call_result != RIN_SUCCESS && (out) != NULL) *(out) = 0; \
    return call_result; \
} while (0)

#define HANDLE_OUTPUT_SIMPLE_CALL(lib, op, out, a, b, c, d, e, f) do { \
    RinSdkArgsV1 request; \
    rin_sdk_zero_bytes(&request, sizeof(request)); \
    request.struct_size = sizeof(request); \
    request.version = RIN_SDK_STRUCT_VERSION_1; \
    request.value[0] = (a); request.value[1] = (b); request.value[2] = (c); \
    request.value[3] = (d); request.value[4] = (e); request.value[5] = (f); \
    return rin_sdk_invoke_handle_output_v1( \
        (lib), (op), &request, sizeof(request), (out)); \
} while (0)

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_BASE)
#if !defined(RINSDK_NO_RUNTIME_SERVICE_ADAPTERS)
enum {
    RINSDK_SERVICE_ID_CAPACITY = 64u,
    RINSDK_SERVICE_MAX_VISIBLE = 1024u
};

typedef struct RinSdkServiceInfoV1 {
    uint32_t slot_id;
    char id[RINSDK_SERVICE_ID_CAPACITY];
    char name[64];
    char exec_path[256];
    char control_app[256];
    uint32_t pid;
    uint32_t owner_uid;
    uint32_t restart_count;
    int32_t last_exit_code;
    uint32_t last_start_tick;
    uint32_t last_exit_tick;
    uint32_t last_health_tick;
    uint32_t health_interval_ms;
    uint32_t health_timeout_ms;
    uint16_t scope;
    uint16_t state;
    uint16_t last_exit_reason;
    uint16_t flags;
} RinSdkServiceInfoV1;

_Static_assert(sizeof(RinSdkServiceInfoV1) == 688u,
               "public service syscall record layout changed");

static size_t rin_sdk_service_id_size(const char* id) {
    size_t index;
    if (!id) return 0u;
    for (index = 0u; index < RINSDK_SERVICE_ID_CAPACITY; ++index) {
        if (id[index] == '\0') return index == 0u ? 0u : index;
    }
    return 0u;
}

static int rin_sdk_service_id_record_valid(
    const char id[RINSDK_SERVICE_ID_CAPACITY]) {
    size_t index;
    if (!id) return 0;
    for (index = 0u; index < RINSDK_SERVICE_ID_CAPACITY; ++index)
        if (id[index] == '\0') break;
    if (index == RINSDK_SERVICE_ID_CAPACITY) return 0;
    for (++index; index < RINSDK_SERVICE_ID_CAPACITY; ++index)
        if (id[index] != '\0') return 0;
    return 1;
}

static int rin_sdk_service_id_matches(
    const char id[RINSDK_SERVICE_ID_CAPACITY], const char* expected) {
    size_t index;
    size_t expected_size = rin_sdk_service_id_size(expected);
    if (!rin_sdk_service_id_record_valid(id) || expected_size == 0u)
        return 0;
    for (index = 0u; index < expected_size; ++index)
        if (id[index] != expected[index]) return 0;
    return id[expected_size] == '\0';
}

static int rin_sdk_service_result_valid(intptr_t result) {
    switch (result) {
    case RIN_SUCCESS:
    case RIN_ERROR_INVALID_ARGUMENT:
    case RIN_ERROR_NOT_SUPPORTED:
    case RIN_ERROR_NOT_FOUND:
    case RIN_ERROR_ACCESS_DENIED:
    case RIN_ERROR_NO_MEMORY:
    case RIN_ERROR_BUSY:
    case RIN_ERROR_TIMEOUT:
    case RIN_ERROR_CANCELLED:
    case RIN_ERROR_IO:
    case RIN_ERROR_WOULD_BLOCK:
    case RIN_ERROR_ABI_MISMATCH:
    case RIN_ERROR_STALE_HANDLE:
    case RIN_ERROR_INTEGRITY:
    case RIN_ERROR_BUFFER_TOO_SMALL:
    case RIN_ERROR_TABLE_FULL:
    case RIN_ERROR_LIMIT_EXCEEDED:
    case RIN_ERROR_WRONG_TYPE:
        return 1;
    default:
        return 0;
    }
}

static RinResult rin_sdk_service_status_result(intptr_t result) {
    if (result == 0) return RIN_SUCCESS;
    return rin_sdk_service_result_valid(result)
        ? (RinResult)result : RIN_ERROR_IO;
}

static int rin_sdk_service_info_get(int index, RinSdkServiceInfoV1* info) {
    if (!info || index < 0) return -1;
    return _syscall3((uintptr_t)RIN_SYS_SERVICE_GET_INFO,
                     (uintptr_t)RIN_SERVICE_SCOPE_SYSTEM,
                     (uintptr_t)(unsigned int)index,
                     (uintptr_t)info) == 0 ? 0 : -1;
}

RinResult rin_service_start(int scope, const char* id) {
    if ((scope != RIN_SERVICE_SCOPE_SYSTEM &&
         scope != RIN_SERVICE_SCOPE_USER) ||
        rin_sdk_service_id_size(id) == 0u)
        return RIN_ERROR_INVALID_ARGUMENT;
    return rin_sdk_service_status_result(_syscall2(
        (uintptr_t)RIN_SYS_SERVICE_START,
        (uintptr_t)(unsigned int)scope, (uintptr_t)id));
}

int rin_service_should_stop(void) {
    const intptr_t result = _syscall0((uintptr_t)RIN_SYS_SERVICE_SHOULD_STOP);
    if (result == 0 || result == 1) return (int)result;
    return (int)rin_sdk_service_status_result(result);
}

int rin_service_health(uint32_t status) {
    return (int)rin_sdk_service_status_result(_syscall1(
        (uintptr_t)RIN_SYS_SERVICE_HEALTH, (uintptr_t)status));
}

int rin_service_wait_stop(uint32_t timeout_ms) {
    const intptr_t result = _syscall1(
        (uintptr_t)RIN_SYS_SERVICE_WAIT_STOP, (uintptr_t)timeout_ms);
    if (result == 0 || result == 1) return (int)result;
    return (int)rin_sdk_service_status_result(result);
}

int rin_service_rescan(uint32_t scope) {
    if (scope != RIN_SERVICE_SCOPE_SYSTEM &&
        scope != RIN_SERVICE_SCOPE_USER)
        return -1;
    return (int)rin_sdk_service_status_result(_syscall1(
        (uintptr_t)RIN_SYS_SERVICE_RESCAN, (uintptr_t)scope));
}

int rin_service_find_system_slot(const char* service_id, uint32_t* slot_id) {
    RinSdkServiceInfoV1 info;
    intptr_t raw_count;
    int count;
    uint32_t found_slot = 0u;
    int index;

    if (!slot_id) return -1;
    *slot_id = 0u;
    if (rin_sdk_service_id_size(service_id) == 0u) return -1;

    raw_count = _syscall1((uintptr_t)RIN_SYS_SERVICE_COUNT,
                          (uintptr_t)RIN_SERVICE_SCOPE_SYSTEM);
    if (raw_count < 0 || raw_count > INT_MAX) return -1;
    count = (int)raw_count;
    if (count <= 0 || (unsigned int)count > RINSDK_SERVICE_MAX_VISIBLE)
        return -1;

    for (index = 0; index < count; ++index) {
        rin_sdk_zero_bytes(&info, sizeof(info));
        if (rin_sdk_service_info_get(index, &info) != 0) return -1;
        if (!rin_sdk_service_id_matches(info.id, service_id)) continue;

        if (found_slot != 0u || info.slot_id == 0u || info.pid == 0u ||
            info.owner_uid != 0u ||
            info.scope != RIN_SERVICE_SCOPE_SYSTEM ||
            info.state != RIN_MANAGEMENT_SERVICE_STATE_RUNNING)
            return -1;
        found_slot = info.slot_id;
    }

    if (found_slot == 0u) return -1;
    *slot_id = found_slot;
    return 0;
}
#endif /* !RINSDK_NO_RUNTIME_SERVICE_ADAPTERS */

RinResult rin_sdk_register_object_close_hook_v1(
    RinSdkObjectCloseHookV1 hook) {
    if (hook == NULL) return RIN_ERROR_INVALID_ARGUMENT;
    while (__sync_lock_test_and_set(&g_object_close_hooks_lock, 1u) != 0u) {
    }
    for (uint32_t index = 0u;
         index < RIN_SDK_OBJECT_CLOSE_HOOK_CAPACITY_V1; ++index) {
        RinSdkObjectCloseHookEntryV1* entry = &g_object_close_hooks[index];
        if (entry->hook == hook && entry->active != 0u) {
            /* Preserve the legacy process-lifetime promise if a caller uses
             * the old API for a hook that was also registered with a token. */
            entry->process_lifetime = 1u;
            __sync_lock_release(&g_object_close_hooks_lock);
            return RIN_SUCCESS;
        }
    }
    for (uint32_t index = 0u;
         index < RIN_SDK_OBJECT_CLOSE_HOOK_CAPACITY_V1; ++index) {
        RinSdkObjectCloseHookEntryV1* entry = &g_object_close_hooks[index];
        if (entry->hook == NULL) {
            entry->hook = hook;
            entry->generation = 0u;
            entry->in_flight = 0u;
            entry->active = 1u;
            entry->process_lifetime = 1u;
            __sync_lock_release(&g_object_close_hooks_lock);
            return RIN_SUCCESS;
        }
    }
    __sync_lock_release(&g_object_close_hooks_lock);
    return RIN_ERROR_LIMIT_EXCEEDED;
}

RinResult rin_sdk_register_object_close_hook_owned_v1(
    RinSdkObjectCloseHookV1 hook,
    RinSdkObjectCloseHookTokenV1* token_out) {
    if (hook == NULL || token_out == NULL)
        return RIN_ERROR_INVALID_ARGUMENT;
    while (__sync_lock_test_and_set(&g_object_close_hooks_lock, 1u) != 0u) {
    }
    if (g_object_close_hook_generation_exhausted != 0u) {
        __sync_lock_release(&g_object_close_hooks_lock);
        return RIN_ERROR_LIMIT_EXCEEDED;
    }
    for (uint32_t index = 0u;
         index < RIN_SDK_OBJECT_CLOSE_HOOK_CAPACITY_V1; ++index) {
        RinSdkObjectCloseHookEntryV1* entry = &g_object_close_hooks[index];
        if (entry->hook != NULL) continue;
        const uint64_t generation = g_object_close_hook_next_generation;
        const RinSdkObjectCloseHookTokenV1 token =
            (generation << RIN_SDK_OBJECT_CLOSE_HOOK_SLOT_BITS_V1) |
            ((uint64_t)index + 1u);
        if (generation == RIN_SDK_OBJECT_CLOSE_HOOK_GENERATION_MAX_V1) {
            g_object_close_hook_generation_exhausted = 1u;
        } else {
            g_object_close_hook_next_generation = generation + 1u;
        }
        entry->hook = hook;
        entry->generation = generation;
        entry->in_flight = 0u;
        entry->process_lifetime = 0u;
        entry->active = 1u;
        __sync_lock_release(&g_object_close_hooks_lock);
        *token_out = token;
        return RIN_SUCCESS;
    }
    __sync_lock_release(&g_object_close_hooks_lock);
    return RIN_ERROR_LIMIT_EXCEEDED;
}

RinResult rin_sdk_unregister_object_close_hook_v1(
    RinSdkObjectCloseHookTokenV1 token) {
    const uint64_t encoded_slot =
        token & RIN_SDK_OBJECT_CLOSE_HOOK_SLOT_MASK_V1;
    const uint64_t generation =
        token >> RIN_SDK_OBJECT_CLOSE_HOOK_SLOT_BITS_V1;
    if (generation == 0u || encoded_slot == 0u ||
        encoded_slot > RIN_SDK_OBJECT_CLOSE_HOOK_CAPACITY_V1) {
        return RIN_ERROR_INVALID_ARGUMENT;
    }
    const uint32_t index = (uint32_t)(encoded_slot - 1u);
    while (__sync_lock_test_and_set(&g_object_close_hooks_lock, 1u) != 0u) {
    }
    RinSdkObjectCloseHookEntryV1* entry = &g_object_close_hooks[index];
    if (entry->hook == NULL || entry->generation != generation) {
        __sync_lock_release(&g_object_close_hooks_lock);
        return RIN_ERROR_STALE_HANDLE;
    }
    if (entry->process_lifetime != 0u) {
        __sync_lock_release(&g_object_close_hooks_lock);
        return RIN_ERROR_ACCESS_DENIED;
    }
    entry->active = 0u;
    if (entry->in_flight != 0u) {
        __sync_lock_release(&g_object_close_hooks_lock);
        return RIN_ERROR_BUSY;
    }
    entry->hook = NULL;
    entry->generation = 0u;
    entry->process_lifetime = 0u;
    __sync_lock_release(&g_object_close_hooks_lock);
    return RIN_SUCCESS;
}

static void rin_sdk_notify_object_close_v1(RinObject object) {
    struct RinSdkObjectCloseHookCallV1 {
        RinSdkObjectCloseHookV1 hook;
        uint64_t generation;
        uint32_t slot;
    } calls[RIN_SDK_OBJECT_CLOSE_HOOK_CAPACITY_V1];
    uint32_t call_count = 0u;
    while (__sync_lock_test_and_set(&g_object_close_hooks_lock, 1u) != 0u) {
    }
    for (uint32_t index = 0u;
         index < RIN_SDK_OBJECT_CLOSE_HOOK_CAPACITY_V1; ++index) {
        RinSdkObjectCloseHookEntryV1* entry = &g_object_close_hooks[index];
        if (entry->hook == NULL || entry->active == 0u ||
            entry->in_flight == UINT64_MAX) {
            continue;
        }
        ++entry->in_flight;
        calls[call_count].hook = entry->hook;
        calls[call_count].generation = entry->generation;
        calls[call_count].slot = index;
        ++call_count;
    }
    __sync_lock_release(&g_object_close_hooks_lock);

    for (uint32_t index = 0u; index < call_count; ++index) {
        calls[index].hook(object);
        while (__sync_lock_test_and_set(&g_object_close_hooks_lock, 1u) != 0u) {
        }
        RinSdkObjectCloseHookEntryV1* entry =
            &g_object_close_hooks[calls[index].slot];
        if (entry->hook == calls[index].hook &&
            entry->generation == calls[index].generation &&
            entry->in_flight != 0u) {
            --entry->in_flight;
        }
        __sync_lock_release(&g_object_close_hooks_lock);
    }
}

RinResult rin_object_close_v1(RinObject object) {
    RinSdkArgsV1 request;
    RinResult result;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = object;
    rin_sdk_notify_object_close_v1(object);
    result = rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_BASE, BASE_OBJECT_CLOSE, &request, sizeof(request),
        NULL, 0u);
    return result;
}
RinResult rin_object_query_v1(RinObject object, RinObjectInfoV1* info) {
    RinSdkArgsV1 request;
    RinResult query_result;
    if (!info || info->struct_size < sizeof(*info) ||
        info->version != (uint16_t)RIN_SDK_STRUCT_VERSION_1) {
        return RIN_ERROR_ABI_MISMATCH;
    }
    rin_sdk_zero_bytes(info, sizeof(*info));
    info->struct_size = sizeof(*info);
    info->version = (uint16_t)RIN_SDK_STRUCT_VERSION_1;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = object;
    query_result = rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_BASE, BASE_OBJECT_QUERY, &request, sizeof(request),
        info, sizeof(*info));
    if (query_result == RIN_SUCCESS &&
        (info->struct_size != sizeof(*info) ||
         info->version != (uint16_t)RIN_SDK_STRUCT_VERSION_1))
        query_result = RIN_ERROR_ABI_MISMATCH;
    if (query_result != RIN_SUCCESS) {
        rin_sdk_zero_bytes(info, sizeof(*info));
        info->struct_size = sizeof(*info);
        info->version = (uint16_t)RIN_SDK_STRUCT_VERSION_1;
    }
    return query_result;
}
RinResult rin_process_spawn_v1(const RinProcessSpawnV1* request, RinProcess* process) {
    if (!versioned(request, sizeof(*request)) || !process) return RIN_ERROR_INVALID_ARGUMENT;
    return rin_sdk_invoke_handle_output_v1(
        RIN_SDK_LIBRARY_BASE, BASE_PROCESS_SPAWN, request, sizeof(*request),
        process);
}
RinResult rin_process_wait_v1(RinProcess process, uint64_t timeout_ns, int32_t* exit_code) {
    SCALAR_OUTPUT_SIMPLE_CALL(
        RIN_SDK_LIBRARY_BASE, BASE_PROCESS_WAIT, exit_code,
        process,timeout_ns,0,0,0,0);
}
RinResult rin_memory_map_v1(const RinMemoryMapV1* request, uint64_t* address) {
    if (!versioned(request, sizeof(*request)) || !address) return RIN_ERROR_INVALID_ARGUMENT;
    return rin_sdk_invoke_scalar_output_v1(
        RIN_SDK_LIBRARY_BASE, BASE_MEMORY_MAP, request, sizeof(*request),
        address, sizeof(*address));
}
RinResult rin_memory_unmap_v1(uint64_t address, uint64_t size) {
    SIMPLE_CALL(RIN_SDK_LIBRARY_BASE, BASE_MEMORY_UNMAP, (RinResult*)0, address,size,0,0,0,0);
}
RinResult rin_time_get_v1(uint32_t clock_id, RinTimeV1* time) {
    VERSIONED_OUTPUT_SIMPLE_CALL(
        RIN_SDK_LIBRARY_BASE, BASE_TIME_GET, time, clock_id,0,0,0,0,0);
}
RinResult rin_random_fill_v1(RinSliceV1 buffer) {
    SIMPLE_CALL(RIN_SDK_LIBRARY_BASE, BASE_RANDOM_FILL, (RinResult*)0, buffer.address,buffer.size,0,0,0,0);
}
RinResult rin_log_write_v1(uint32_t level, RinStringV1 message) {
    SIMPLE_CALL(RIN_SDK_LIBRARY_BASE, BASE_LOG_WRITE, (RinResult*)0, level,message.address,message.size,0,0,0);
}
RinResult rin_thread_create_v1(const RinThreadCreateV1* request, RinThread* thread) {
    if (!versioned(request, sizeof(*request)) || !thread) return RIN_ERROR_INVALID_ARGUMENT;
    return rin_sdk_invoke_handle_output_v1(
        RIN_SDK_LIBRARY_BASE, BASE_THREAD_CREATE, request, sizeof(*request),
        thread);
}
RinResult rin_thread_join_v1(RinThread thread, uint64_t timeout_ns, int32_t* exit_code) {
    SCALAR_OUTPUT_SIMPLE_CALL(
        RIN_SDK_LIBRARY_BASE, BASE_THREAD_JOIN, exit_code,
        thread,timeout_ns,0,0,0,0);
}
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_IPC)
RinResult rin_channel_create_v1(RinChannel* first, RinChannel* second) {
    RinChannel pair[2] = {0, 0};
    RinSdkArgsV1 request;
    RinResult result;
    if (!first || !second) return RIN_ERROR_INVALID_ARGUMENT;
    *first = RIN_HANDLE_INVALID;
    *second = RIN_HANDLE_INVALID;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    result = rin_sdk_invoke_scalar_output_v1(
        RIN_SDK_LIBRARY_IPC, IPC_CHANNEL_CREATE, &request, sizeof(request),
        pair, sizeof(pair));
    if (result == RIN_SUCCESS &&
        (pair[0] == RIN_HANDLE_INVALID || pair[1] == RIN_HANDLE_INVALID)) {
        pair[0] = RIN_HANDLE_INVALID;
        pair[1] = RIN_HANDLE_INVALID;
        result = RIN_ERROR_ABI_MISMATCH;
    }
    if (result == RIN_SUCCESS) { *first = pair[0]; *second = pair[1]; }
    return result;
}
RinResult rin_channel_send_v1(RinChannel channel, const RinIpcMessageV1* message) {
    if (!versioned(message, sizeof(*message))) return RIN_ERROR_ABI_MISMATCH;
    SIMPLE_CALL(RIN_SDK_LIBRARY_IPC, IPC_CHANNEL_SEND, (RinResult*)0,
                channel, (uint64_t)(uintptr_t)message, 0u, 0u, 0u, 0u);
}
RinResult rin_channel_receive_v1(RinChannel channel, RinIpcMessageV1* message) {
    RinSliceV1 bytes;
    RinSliceV1 handles;
    RinSdkArgsV1 request;
    RinResult receive_result;
    if (!versioned(message, sizeof(*message))) return RIN_ERROR_ABI_MISMATCH;
    bytes = message->bytes;
    handles = message->handles;
    rin_sdk_zero_bytes(message, sizeof(*message));
    message->struct_size = sizeof(*message);
    message->version = RIN_SDK_STRUCT_VERSION_1;
    message->bytes = bytes;
    message->handles = handles;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = channel;
    receive_result = rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_IPC, IPC_CHANNEL_RECEIVE, &request, sizeof(request),
        message, sizeof(*message));
    if (receive_result == RIN_SUCCESS &&
        !versioned_exact(message, sizeof(*message)))
        receive_result = RIN_ERROR_ABI_MISMATCH;
    if (receive_result != RIN_SUCCESS) {
        rin_sdk_zero_bytes(message, sizeof(*message));
        message->struct_size = sizeof(*message);
        message->version = RIN_SDK_STRUCT_VERSION_1;
        message->bytes = bytes;
        message->handles = handles;
    }
    return receive_result;
}
RinResult rin_channel_receive_with_peer_v1(
    RinChannel channel, RinIpcMessageV1* message,
    RinIpcPeerIdentityV1* peer_identity_out) {
    RinSliceV1 bytes;
    RinSliceV1 handles;
    RinSdkArgsV1 request;
    RinResult receive_result;
    if (!versioned(message, sizeof(*message)) ||
        !versioned(peer_identity_out, sizeof(*peer_identity_out)))
        return RIN_ERROR_ABI_MISMATCH;
    bytes = message->bytes;
    handles = message->handles;
    rin_sdk_zero_bytes(message, sizeof(*message));
    message->struct_size = sizeof(*message);
    message->version = RIN_SDK_STRUCT_VERSION_1;
    message->bytes = bytes;
    message->handles = handles;
    rin_sdk_zero_bytes(peer_identity_out, sizeof(*peer_identity_out));
    peer_identity_out->struct_size = sizeof(*peer_identity_out);
    peer_identity_out->version = RIN_SDK_STRUCT_VERSION_1;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = channel;
    request.value[1] = (uint64_t)(uintptr_t)peer_identity_out;
    receive_result = rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_IPC, IPC_CHANNEL_RECEIVE_WITH_PEER, &request,
        sizeof(request), message, sizeof(*message));
    if (receive_result == RIN_SUCCESS &&
        (!versioned_exact(message, sizeof(*message)) ||
         !versioned_exact(peer_identity_out, sizeof(*peer_identity_out)) ||
         peer_identity_out->process_id == 0u ||
         peer_identity_out->process_instance_cookie == 0u ||
         peer_identity_out->reserved != 0u))
        receive_result = RIN_ERROR_ABI_MISMATCH;
    if (receive_result != RIN_SUCCESS) {
        rin_sdk_zero_bytes(message, sizeof(*message));
        message->struct_size = sizeof(*message);
        message->version = RIN_SDK_STRUCT_VERSION_1;
        message->bytes = bytes;
        message->handles = handles;
        rin_sdk_zero_bytes(peer_identity_out, sizeof(*peer_identity_out));
        peer_identity_out->struct_size = sizeof(*peer_identity_out);
        peer_identity_out->version = RIN_SDK_STRUCT_VERSION_1;
    }
    return receive_result;
}
RinResult rin_event_create_v1(uint32_t flags, RinEvent* event) {
    HANDLE_OUTPUT_SIMPLE_CALL(
        RIN_SDK_LIBRARY_IPC, IPC_EVENT_CREATE, event, flags,0,0,0,0,0);
}
RinResult rin_event_signal_v1(RinEvent event, uint64_t value) {
    SIMPLE_CALL(RIN_SDK_LIBRARY_IPC, IPC_EVENT_SIGNAL, (RinResult*)0, event,value,0,0,0,0);
}
RinResult rin_event_query_v1(RinEvent event, uint64_t* value_out) {
    RinSdkArgsV1 request;
    if (!value_out) return RIN_ERROR_INVALID_ARGUMENT;
    *value_out = 0u;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = event;
    return rin_sdk_invoke_v1(RIN_SDK_LIBRARY_IPC, IPC_EVENT_QUERY,
                             &request, sizeof(request), value_out,
                             sizeof(*value_out));
}
RinResult rin_wait_many_v1(RinSliceV1 items, uint64_t timeout_ns, uint32_t* index) {
    SCALAR_OUTPUT_SIMPLE_CALL(
        RIN_SDK_LIBRARY_IPC, IPC_WAIT_MANY, index,
        items.address,items.size,timeout_ns,0,0,0);
}
RinResult rin_shared_memory_create_v1(uint64_t size, uint32_t flags, RinSharedMemory* memory) {
    HANDLE_OUTPUT_SIMPLE_CALL(
        RIN_SDK_LIBRARY_IPC, IPC_SHARED_MEMORY_CREATE, memory,
        size,flags,0,0,0,0);
}
RinResult rin_service_connect_v1(RinStringV1 name, RinChannel* channel) {
    HANDLE_OUTPUT_SIMPLE_CALL(
        RIN_SDK_LIBRARY_IPC, IPC_SERVICE_CONNECT, channel,
        name.address,name.size,0,0,0,0);
}
RinResult rin_wait_set_create_v1(uint32_t flags, RinWaitSet* wait_set) {
    HANDLE_OUTPUT_SIMPLE_CALL(
        RIN_SDK_LIBRARY_IPC, IPC_WAIT_SET_CREATE, wait_set,
        flags,0,0,0,0,0);
}
RinResult rin_wait_set_set_items_v1(RinWaitSet wait_set, RinSliceV1 items) {
    SIMPLE_CALL(RIN_SDK_LIBRARY_IPC, IPC_WAIT_SET_SET_ITEMS, (RinResult*)0, wait_set,items.address,items.size,0,0,0);
}
RinResult rin_wait_set_wait_v1(RinWaitSet wait_set, uint64_t timeout_ns, RinWaitResultV1* result) {
    RinSdkArgsV1 request;
    RinResult wait_result;
    if (!versioned(result, sizeof(*result))) return RIN_ERROR_ABI_MISMATCH;
    rin_sdk_zero_bytes(result, sizeof(*result));
    result->struct_size = sizeof(*result);
    result->version = RIN_SDK_STRUCT_VERSION_1;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = wait_set;
    request.value[1] = timeout_ns;
    wait_result = rin_sdk_invoke_v1(RIN_SDK_LIBRARY_IPC, IPC_WAIT_SET_WAIT,
                                    &request, sizeof(request), result,
                                    sizeof(*result));
    wait_result = finalize_versioned_output(wait_result, result,
                                            sizeof(*result));
    return wait_result;
}
RinResult rin_service_register_v1(RinStringV1 name, uint32_t flags, RinService* service) {
    if (!service) return RIN_ERROR_INVALID_ARGUMENT;
    HANDLE_OUTPUT_SIMPLE_CALL(
        RIN_SDK_LIBRARY_IPC, IPC_SERVICE_REGISTER, service,
        name.address,name.size,flags,0,0,0);
}
RinResult rin_service_accept_v1(RinService service, uint64_t timeout_ns, RinChannel* channel) {
    if (!channel) return RIN_ERROR_INVALID_ARGUMENT;
    HANDLE_OUTPUT_SIMPLE_CALL(
        RIN_SDK_LIBRARY_IPC, IPC_SERVICE_ACCEPT, channel,
        service,timeout_ns,0,0,0,0);
}
RinResult rin_posix_mq_open_v1(
    RinStringV1 name, uint32_t flags, uint32_t mode,
    const RinPosixMqAttrV1* attributes, RinObjectHandle* queue_out) {
    RinSdkArgsV1 request;
    if (!queue_out || !name.address || name.size == 0u ||
        name.size > RIN_POSIX_MQ_NAME_MAX ||
        ((flags & RIN_POSIX_MQ_OPEN_CREATE) == 0u && attributes != NULL) ||
        (attributes && !versioned_exact(attributes, sizeof(*attributes))))
        return RIN_ERROR_INVALID_ARGUMENT;
    *queue_out = RIN_OBJECT_HANDLE_INVALID;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = name.address;
    request.value[1] = name.size;
    request.value[2] = flags;
    request.value[3] = mode;
    request.value[4] = (uint64_t)(uintptr_t)attributes;
    return rin_sdk_invoke_handle_output_v1(
        RIN_SDK_LIBRARY_IPC, IPC_POSIX_MQ_OPEN, &request, sizeof(request),
        queue_out);
}
RinResult rin_posix_mq_unlink_v1(RinStringV1 name) {
    if (!name.address || name.size == 0u ||
        name.size > RIN_POSIX_MQ_NAME_MAX)
        return RIN_ERROR_INVALID_ARGUMENT;
    SIMPLE_CALL(RIN_SDK_LIBRARY_IPC, IPC_POSIX_MQ_UNLINK, (RinResult*)0,
                name.address, name.size, 0u, 0u, 0u, 0u);
}
RinResult rin_posix_mq_send_v1(
    RinObjectHandle queue, RinSliceV1 message, uint32_t priority) {
    SIMPLE_CALL(RIN_SDK_LIBRARY_IPC, IPC_POSIX_MQ_SEND, (RinResult*)0,
                queue, message.address, message.size, priority, 0u, 0u);
}
RinResult rin_posix_mq_receive_v1(
    RinObjectHandle queue, RinSliceV1 buffer,
    RinPosixMqReceiveResultV1* result_out) {
    RinSdkArgsV1 request;
    RinResult result;
    if (!result_out)
        return RIN_ERROR_INVALID_ARGUMENT;
    rin_sdk_zero_bytes(result_out, sizeof(*result_out));
    result_out->struct_size = sizeof(*result_out);
    result_out->version = RIN_POSIX_MQ_ABI_VERSION;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = queue;
    request.value[1] = buffer.address;
    request.value[2] = buffer.size;
    result = rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_IPC, IPC_POSIX_MQ_RECEIVE, &request, sizeof(request),
        result_out, sizeof(*result_out));
    if (result == RIN_SUCCESS &&
        (!versioned_exact(result_out, sizeof(*result_out)) ||
         result_out->reserved0 != 0u || result_out->reserved[0] != 0u ||
         result_out->reserved[1] != 0u ||
         result_out->length > buffer.size))
        result = RIN_ERROR_ABI_MISMATCH;
    if (result != RIN_SUCCESS) {
        rin_sdk_zero_bytes(result_out, sizeof(*result_out));
        result_out->struct_size = sizeof(*result_out);
        result_out->version = RIN_POSIX_MQ_ABI_VERSION;
    }
    return result;
}
RinResult rin_posix_mq_getattr_v1(
    RinObjectHandle queue, RinPosixMqAttrV1* attributes_out) {
    RinSdkArgsV1 request;
    RinResult result;
    if (!attributes_out) return RIN_ERROR_INVALID_ARGUMENT;
    rin_sdk_zero_bytes(attributes_out, sizeof(*attributes_out));
    attributes_out->struct_size = sizeof(*attributes_out);
    attributes_out->version = RIN_POSIX_MQ_ABI_VERSION;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = queue;
    result = rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_IPC, IPC_POSIX_MQ_GETATTR, &request, sizeof(request),
        attributes_out, sizeof(*attributes_out));
    if (result == RIN_SUCCESS &&
        !versioned_exact(attributes_out, sizeof(*attributes_out)))
        result = RIN_ERROR_ABI_MISMATCH;
    if (result != RIN_SUCCESS) {
        rin_sdk_zero_bytes(attributes_out, sizeof(*attributes_out));
        attributes_out->struct_size = sizeof(*attributes_out);
        attributes_out->version = RIN_POSIX_MQ_ABI_VERSION;
    }
    return result;
}
RinResult rin_posix_mq_setattr_v1(
    RinObjectHandle queue, const RinPosixMqAttrV1* attributes,
    RinPosixMqAttrV1* old_attributes_out) {
    RinSdkArgsV1 request;
    RinResult result;
    if (!attributes || !old_attributes_out ||
        !versioned_exact(attributes, sizeof(*attributes)))
        return RIN_ERROR_INVALID_ARGUMENT;
    rin_sdk_zero_bytes(old_attributes_out, sizeof(*old_attributes_out));
    old_attributes_out->struct_size = sizeof(*old_attributes_out);
    old_attributes_out->version = RIN_POSIX_MQ_ABI_VERSION;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = queue;
    request.value[1] = (uint64_t)(uintptr_t)attributes;
    result = rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_IPC, IPC_POSIX_MQ_SETATTR, &request, sizeof(request),
        old_attributes_out, sizeof(*old_attributes_out));
    if (result == RIN_SUCCESS &&
        !versioned_exact(old_attributes_out, sizeof(*old_attributes_out)))
        result = RIN_ERROR_ABI_MISMATCH;
    if (result != RIN_SUCCESS) {
        rin_sdk_zero_bytes(old_attributes_out, sizeof(*old_attributes_out));
        old_attributes_out->struct_size = sizeof(*old_attributes_out);
        old_attributes_out->version = RIN_POSIX_MQ_ABI_VERSION;
    }
    return result;
}
RinResult rin_posix_mq_notify_v1(
    RinObjectHandle queue, RinEvent event, uint32_t action) {
    SIMPLE_CALL(RIN_SDK_LIBRARY_IPC, IPC_POSIX_MQ_NOTIFY, (RinResult*)0,
                queue, event, action, 0u, 0u, 0u);
}
RinResult rin_posix_mq_wait_receive_begin_v1(
    RinObjectHandle queue, RinPosixMqWaitTokenV1* token_out) {
    RinSdkArgsV1 request;
    if (!token_out) return RIN_ERROR_INVALID_ARGUMENT;
    *token_out = RIN_POSIX_MQ_WAIT_TOKEN_INVALID;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = queue;
    return rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_IPC, IPC_POSIX_MQ_WAIT_RECEIVE_BEGIN, &request,
        sizeof(request), token_out, sizeof(*token_out));
}
RinResult rin_posix_mq_wait_receive_end_v1(
    RinPosixMqWaitTokenV1 token) {
    if (token == RIN_POSIX_MQ_WAIT_TOKEN_INVALID) return RIN_SUCCESS;
    SIMPLE_CALL(RIN_SDK_LIBRARY_IPC, IPC_POSIX_MQ_WAIT_RECEIVE_END,
                (RinResult*)0, token, 0u, 0u, 0u, 0u, 0u);
}
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_FS)
RinResult rin_file_open_v1(const RinFileOpenV1* request, RinFile* file) {
    if (!versioned(request, sizeof(*request)) || !file) return RIN_ERROR_INVALID_ARGUMENT;
    return rin_sdk_invoke_handle_output_v1(
        RIN_SDK_LIBRARY_FS, FS_FILE_OPEN, request, sizeof(*request), file);
}
RinResult rin_file_read_v1(RinFileIoV1* request, uint64_t* transferred) {
    if (!versioned(request, sizeof(*request))) return RIN_ERROR_ABI_MISMATCH;
    return rin_sdk_invoke_scalar_output_v1(
        RIN_SDK_LIBRARY_FS, FS_FILE_READ, request, sizeof(*request),
        transferred, transferred ? sizeof(*transferred) : 0u);
}
RinResult rin_file_write_v1(RinFileIoV1* request, uint64_t* transferred) {
    if (!versioned(request, sizeof(*request))) return RIN_ERROR_ABI_MISMATCH;
    return rin_sdk_invoke_scalar_output_v1(
        RIN_SDK_LIBRARY_FS, FS_FILE_WRITE, request, sizeof(*request),
        transferred, transferred ? sizeof(*transferred) : 0u);
}
RinResult rin_file_flush_v1(RinFile file) { SIMPLE_CALL(RIN_SDK_LIBRARY_FS, FS_FILE_FLUSH, (RinResult*)0, file,0,0,0,0,0); }
RinResult rin_directory_next_v1(RinDirectory directory, RinDirectoryEntryV1* entry) {
    RinStringV1 name;
    RinSdkArgsV1 request;
    RinResult next_result;
    if (!versioned(entry, sizeof(*entry))) return RIN_ERROR_ABI_MISMATCH;
    name = entry->name;
    rin_sdk_zero_bytes(entry, sizeof(*entry));
    entry->struct_size = sizeof(*entry);
    entry->version = RIN_SDK_STRUCT_VERSION_1;
    entry->name = name;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = directory;
    next_result = rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_FS, FS_DIRECTORY_NEXT, &request, sizeof(request),
        entry, sizeof(*entry));
    if (next_result == RIN_SUCCESS &&
        !versioned_exact(entry, sizeof(*entry)))
        next_result = RIN_ERROR_ABI_MISMATCH;
    if (next_result != RIN_SUCCESS) {
        rin_sdk_zero_bytes(entry, sizeof(*entry));
        entry->struct_size = sizeof(*entry);
        entry->version = RIN_SDK_STRUCT_VERSION_1;
        entry->name = name;
    }
    return next_result;
}
RinResult rin_directory_next_batch_v1(const RinDirectoryBatchV1* request,
                                      uint32_t* count) {
    if (!versioned(request, sizeof(*request)) || !count)
        return RIN_ERROR_INVALID_ARGUMENT;
    return rin_sdk_invoke_scalar_output_v1(
        RIN_SDK_LIBRARY_FS, FS_DIRECTORY_NEXT_BATCH, request, sizeof(*request),
        count, sizeof(*count));
}
RinResult rin_path_normalize_v1(RinStringV1 input, RinSliceV1 output, uint64_t* required) {
    SCALAR_OUTPUT_SIMPLE_CALL(
        RIN_SDK_LIBRARY_FS, FS_PATH_NORMALIZE, required,
        input.address,input.size,output.address,output.size,0,0);
}
RinResult rin_file_watch_v1(RinStringV1 path, uint32_t events, RinFileWatch* watch) {
    HANDLE_OUTPUT_SIMPLE_CALL(
        RIN_SDK_LIBRARY_FS, FS_FILE_WATCH, watch,
        path.address,path.size,events,0,0,0);
}
RinResult rin_file_read_async_v1(const RinFileIoV1* request, RinIoRequest* operation) {
    if (!versioned(request, sizeof(*request)) || !operation) return RIN_ERROR_INVALID_ARGUMENT;
    return rin_sdk_invoke_handle_output_v1(
        RIN_SDK_LIBRARY_FS, FS_FILE_READ_ASYNC, request, sizeof(*request),
        operation);
}
RinResult rin_file_write_async_v1(const RinFileIoV1* request, RinIoRequest* operation) {
    if (!versioned(request, sizeof(*request)) || !operation) return RIN_ERROR_INVALID_ARGUMENT;
    return rin_sdk_invoke_handle_output_v1(
        RIN_SDK_LIBRARY_FS, FS_FILE_WRITE_ASYNC, request, sizeof(*request),
        operation);
}
RinResult rin_file_io_result_v1(RinIoRequest operation, uint64_t* transferred) {
    SCALAR_OUTPUT_SIMPLE_CALL(
        RIN_SDK_LIBRARY_FS, FS_FILE_IO_RESULT, transferred,
        operation,0,0,0,0,0);
}
RinResult rin_file_io_cancel_v1(RinIoRequest operation) {
    SIMPLE_CALL(RIN_SDK_LIBRARY_FS, FS_FILE_IO_CANCEL, (RinResult*)0, operation,0,0,0,0,0);
}
RinResult rin_directory_open_v1(const RinDirectoryOpenV1* request, RinDirectory* directory) {
    if (!versioned(request, sizeof(*request)) || !directory) return RIN_ERROR_INVALID_ARGUMENT;
    return rin_sdk_invoke_handle_output_v1(
        RIN_SDK_LIBRARY_FS, FS_DIRECTORY_OPEN, request, sizeof(*request),
        directory);
}
RinResult rin_directory_rewind_v1(RinDirectory directory) {
    SIMPLE_CALL(RIN_SDK_LIBRARY_FS, FS_DIRECTORY_REWIND, (RinResult*)0, directory,0,0,0,0,0);
}
RinResult rin_file_watch_next_v1(RinFileWatch watch, uint64_t timeout_ns, RinFileWatchEventV1* event) {
    RinStringV1 name;
    RinSdkArgsV1 request;
    RinResult watch_result;
    if (!versioned(event, sizeof(*event))) return RIN_ERROR_ABI_MISMATCH;
    name = event->name;
    rin_sdk_zero_bytes(event, sizeof(*event));
    event->struct_size = sizeof(*event);
    event->version = RIN_SDK_STRUCT_VERSION_1;
    event->name = name;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = watch;
    request.value[1] = timeout_ns;
    watch_result = rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_FS, FS_FILE_WATCH_NEXT, &request, sizeof(request),
        event, sizeof(*event));
    if (watch_result == RIN_SUCCESS &&
        !versioned_exact(event, sizeof(*event)))
        watch_result = RIN_ERROR_ABI_MISMATCH;
    if (watch_result != RIN_SUCCESS) {
        rin_sdk_zero_bytes(event, sizeof(*event));
        event->struct_size = sizeof(*event);
        event->version = RIN_SDK_STRUCT_VERSION_1;
        event->name = name;
    }
    return watch_result;
}
RinResult rin_file_close_v1(RinFile file) {
    SIMPLE_CALL(RIN_SDK_LIBRARY_FS, FS_FILE_CLOSE, (RinResult*)0, file,0,0,0,0,0);
}
RinResult rin_directory_close_v1(RinDirectory directory) {
    SIMPLE_CALL(RIN_SDK_LIBRARY_FS, FS_DIRECTORY_CLOSE, (RinResult*)0, directory,0,0,0,0,0);
}
RinResult rin_file_seek_v1(const RinFileSeekV1* request, uint64_t* position) {
    if (!versioned(request, sizeof(*request)) || !position)
        return RIN_ERROR_INVALID_ARGUMENT;
    return rin_sdk_invoke_scalar_output_v1(
        RIN_SDK_LIBRARY_FS, FS_FILE_SEEK, request, sizeof(*request), position,
        sizeof(*position));
}
RinResult rin_file_stat_v1(RinFile file, RinFileStatV1* stat) {
    RinSdkArgsV1 request = {0};
    RinResult stat_result;
    if (file == RIN_HANDLE_INVALID || !stat)
        return RIN_ERROR_INVALID_ARGUMENT;
    if (!versioned(stat, sizeof(*stat))) return RIN_ERROR_ABI_MISMATCH;
    rin_sdk_zero_bytes(stat, sizeof(*stat));
    stat->struct_size = sizeof(*stat);
    stat->version = RIN_SDK_STRUCT_VERSION_1;
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = file;
    stat_result = rin_sdk_invoke_v1(RIN_SDK_LIBRARY_FS, FS_FILE_STAT,
                                    &request, sizeof(request), stat,
                                    sizeof(*stat));
    stat_result = finalize_versioned_output(stat_result, stat, sizeof(*stat));
    return stat_result;
}
RinResult rin_file_truncate_v1(const RinFileTruncateV1* request) {
    if (!versioned(request, sizeof(*request))) return RIN_ERROR_ABI_MISMATCH;
    return rin_sdk_invoke_v1(RIN_SDK_LIBRARY_FS, FS_FILE_TRUNCATE,
                             request, sizeof(*request), NULL, 0u);
}
RinResult rin_directory_sync_v1(RinDirectory directory) {
    SIMPLE_CALL(RIN_SDK_LIBRARY_FS, FS_DIRECTORY_SYNC, (RinResult*)0, directory,0,0,0,0,0);
}
RinResult rin_path_stat_v1(const RinFsPathRequestV1* request, RinFileStatV1* stat) {
    RinResult stat_result;
    if (!versioned(request, sizeof(*request)) || !stat)
        return RIN_ERROR_INVALID_ARGUMENT;
    if (!versioned(stat, sizeof(*stat))) return RIN_ERROR_ABI_MISMATCH;
    rin_sdk_zero_bytes(stat, sizeof(*stat));
    stat->struct_size = sizeof(*stat);
    stat->version = RIN_SDK_STRUCT_VERSION_1;
    if (request->secondary_path.address != 0u ||
        request->secondary_path.size != 0u || request->flags != 0u ||
        request->mode != 0u || request->reserved[0] != 0u ||
        request->reserved[1] != 0u ||
        request->secondary_directory != RIN_HANDLE_INVALID)
        return RIN_ERROR_INVALID_ARGUMENT;
    stat_result = rin_sdk_invoke_v1(RIN_SDK_LIBRARY_FS, FS_PATH_STAT,
                                    request, sizeof(*request), stat,
                                    sizeof(*stat));
    stat_result = finalize_versioned_output(stat_result, stat, sizeof(*stat));
    return stat_result;
}
RinResult rin_path_mkdir_v1(const RinFsPathRequestV1* request) {
    if (!versioned(request, sizeof(*request)) ||
        request->secondary_directory != RIN_HANDLE_INVALID ||
        request->secondary_path.address != 0u ||
        request->secondary_path.size != 0u ||
        request->flags != 0u || request->reserved[0] != 0u ||
        request->reserved[1] != 0u)
        return RIN_ERROR_INVALID_ARGUMENT;
    return rin_sdk_invoke_v1(RIN_SDK_LIBRARY_FS, FS_PATH_MKDIR,
                             request, sizeof(*request), NULL, 0u);
}
RinResult rin_path_unlink_v1(const RinFsPathRequestV1* request) {
    if (!versioned(request, sizeof(*request)) ||
        request->secondary_directory != RIN_HANDLE_INVALID ||
        request->secondary_path.address != 0u ||
        request->secondary_path.size != 0u || request->mode != 0u ||
        request->reserved[0] != 0u || request->reserved[1] != 0u ||
        (request->flags & ~RIN_FS_PATH_UNLINK_DIRECTORY) != 0u)
        return RIN_ERROR_INVALID_ARGUMENT;
    return rin_sdk_invoke_v1(RIN_SDK_LIBRARY_FS, FS_PATH_UNLINK,
                             request, sizeof(*request), NULL, 0u);
}
RinResult rin_path_rename_v1(const RinFsPathRequestV1* request) {
    if (!versioned(request, sizeof(*request)) ||
        request->path.address == 0u || request->path.size == 0u ||
        request->secondary_path.address == 0u ||
        request->secondary_path.size == 0u || request->flags != 0u ||
        request->mode != 0u || request->reserved[0] != 0u ||
        request->reserved[1] != 0u)
        return RIN_ERROR_INVALID_ARGUMENT;
    return rin_sdk_invoke_v1(RIN_SDK_LIBRARY_FS, FS_PATH_RENAME,
                             request, sizeof(*request), NULL, 0u);
}
RinResult rin_fs_portal_open_v1(const RinFsPortalOpenV1* request,
                                uint64_t* handle) {
    if (!versioned(request, sizeof(*request)) || handle == NULL)
        return RIN_ERROR_INVALID_ARGUMENT;
    return rin_sdk_invoke_handle_output_v1(
        RIN_SDK_LIBRARY_FS, FS_PORTAL_OPEN, request, sizeof(*request), handle);
}
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_NET)
RinResult rin_socket_create_v1(uint32_t domain, uint32_t type, uint32_t protocol, RinSocket* socket) { HANDLE_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_NET, NET_SOCKET_CREATE, socket, domain,type,protocol,0,0,0); }
RinResult rin_socket_connect_v1(RinSocket socket, const RinSocketAddressV1* address) {
    struct { RinVersionedV1 header; RinSocket socket; RinSocketAddressV1 address; } request;
    if (!versioned(address, sizeof(*address))) return RIN_ERROR_ABI_MISMATCH;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.header.struct_size = sizeof(request);
    request.header.version = RIN_SDK_STRUCT_VERSION_1;
    request.socket = socket;
    rin_sdk_copy_bytes(&request.address, address, sizeof(request.address));
    return rin_sdk_invoke_v1(RIN_SDK_LIBRARY_NET, NET_SOCKET_CONNECT,
                             &request, sizeof(request), NULL, 0u);
}
RinResult rin_socket_send_v1(RinSocket socket, RinSliceV1 data, uint32_t flags, uint64_t* sent) { SCALAR_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_NET, NET_SOCKET_SEND, sent, socket,data.address,data.size,flags,0,0); }
RinResult rin_socket_receive_v1(RinSocket socket, RinSliceV1 data, uint32_t flags, uint64_t* received) { SCALAR_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_NET, NET_SOCKET_RECEIVE, received, socket,data.address,data.size,flags,0,0); }
RinResult rin_dns_resolve_v1(RinStringV1 host, RinStringV1 service, RinSliceV1 results, uint64_t* count) { SCALAR_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_NET, NET_DNS_RESOLVE, count, host.address,host.size,service.address,service.size,results.address,results.size); }
RinResult rin_tls_connect_v1(RinSocket socket, RinStringV1 server_name, uint32_t flags, RinTlsSession* session) { HANDLE_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_NET, NET_TLS_CONNECT, session, socket,server_name.address,server_name.size,flags,0,0); }
RinResult rin_http_execute_v1(const RinHttpRequestV1* request, RinChannel* response_stream) {
    if (!versioned(request, sizeof(*request))) return RIN_ERROR_ABI_MISMATCH;
    return rin_sdk_invoke_handle_output_v1(
        RIN_SDK_LIBRARY_NET, NET_HTTP_EXECUTE, request, sizeof(*request),
        response_stream);
}
RinResult rin_tls_send_v1(RinTlsSession session, RinSliceV1 data, uint32_t flags, uint64_t* sent) { SCALAR_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_NET, NET_TLS_SEND, sent, session,data.address,data.size,flags,0,0); }
RinResult rin_tls_receive_v1(RinTlsSession session, RinSliceV1 data, uint32_t flags, uint64_t* received) { SCALAR_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_NET, NET_TLS_RECEIVE, received, session,data.address,data.size,flags,0,0); }
RinResult rin_tls_shutdown_v1(RinTlsSession session, uint32_t flags) { SIMPLE_CALL(RIN_SDK_LIBRARY_NET, NET_TLS_SHUTDOWN, (RinResult*)0, session,flags,0,0,0,0); }
RinResult rin_socket_bind_v1(RinSocket socket, const RinSocketAddressV1* address) {
    struct { RinVersionedV1 header; RinSocket socket; RinSocketAddressV1 address; } request;
    if (!versioned(address, sizeof(*address))) return RIN_ERROR_ABI_MISMATCH;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.header.struct_size = sizeof(request);
    request.header.version = RIN_SDK_STRUCT_VERSION_1;
    request.socket = socket;
    rin_sdk_copy_bytes(&request.address, address, sizeof(request.address));
    return rin_sdk_invoke_v1(RIN_SDK_LIBRARY_NET, NET_SOCKET_BIND, &request, sizeof(request), NULL, 0u);
}
RinResult rin_socket_listen_v1(RinSocket socket, uint32_t backlog) { SIMPLE_CALL(RIN_SDK_LIBRARY_NET, NET_SOCKET_LISTEN, (RinResult*)0, socket,backlog,0,0,0,0); }
RinResult rin_socket_accept_v1(RinSocket socket, uint64_t timeout_ns, RinSocketAcceptV1* accepted) {
    RinSdkArgsV1 request;
    RinResult result;
    if (!versioned(accepted, sizeof(*accepted))) return RIN_ERROR_ABI_MISMATCH;
    rin_sdk_zero_bytes(accepted, sizeof(*accepted));
    accepted->struct_size = sizeof(*accepted);
    accepted->version = RIN_SDK_STRUCT_VERSION_1;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = socket;
    request.value[1] = timeout_ns;
    result = rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_NET, NET_SOCKET_ACCEPT, &request, sizeof(request),
        accepted, sizeof(*accepted));
    result = finalize_versioned_output(result, accepted, sizeof(*accepted));
    if (result == RIN_SUCCESS && accepted->socket == RIN_HANDLE_INVALID) {
        reset_versioned_output(accepted, sizeof(*accepted));
        result = RIN_ERROR_ABI_MISMATCH;
    }
    return result;
}
RinResult rin_socket_shutdown_v1(RinSocket socket, uint32_t flags) { SIMPLE_CALL(RIN_SDK_LIBRARY_NET, NET_SOCKET_SHUTDOWN, (RinResult*)0, socket,flags,0,0,0,0); }
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_GUI)
static void rin_sdk_gui_copy(uint8_t* destination, const uint8_t* source,
                             uint64_t size);
static int rin_sdk_gui_utf8_valid(const uint8_t* bytes, uint64_t size);
static int rin_sdk_gui_text_snapshot(RinStringV1 input, uint64_t max_bytes,
                                     char* output);

RinResult rin_clipboard_set_v1(uint32_t format,RinSliceV1 data) { SIMPLE_CALL(RIN_SDK_LIBRARY_GUI,GUI_CLIPBOARD_SET,(RinResult*)0,format,data.address,data.size,0,0,0); }
RinResult rin_clipboard_get_v1(uint32_t format,RinSliceV1 data,uint64_t* required) { SCALAR_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_GUI,GUI_CLIPBOARD_GET,required,format,data.address,data.size,0,0,0); }
static void rin_sdk_gui_copy(uint8_t* destination, const uint8_t* source,
                             uint64_t size) {
    while (size-- != 0u) *destination++ = *source++;
}

static int rin_sdk_gui_utf8_valid(const uint8_t* bytes, uint64_t size) {
    uint64_t offset = 0u;
    while (offset < size) {
        const uint8_t lead = bytes[offset++];
        uint32_t codepoint;
        uint32_t continuation_count;
        uint8_t second = 0u;
        if (lead == 0u) return 0;
        if (lead < 0x80u) continue;
        if (lead >= 0xc2u && lead <= 0xdfu) {
            codepoint = (uint32_t)(lead & 0x1fu);
            continuation_count = 1u;
        } else if (lead >= 0xe0u && lead <= 0xefu) {
            codepoint = (uint32_t)(lead & 0x0fu);
            continuation_count = 2u;
        } else if (lead >= 0xf0u && lead <= 0xf4u) {
            codepoint = (uint32_t)(lead & 0x07u);
            continuation_count = 3u;
        } else {
            return 0;
        }
        if (continuation_count > size - offset) return 0;
        second = bytes[offset];
        if ((second & 0xc0u) != 0x80u ||
            (lead == 0xe0u && second < 0xa0u) ||
            (lead == 0xedu && second >= 0xa0u) ||
            (lead == 0xf0u && second < 0x90u) ||
            (lead == 0xf4u && second > 0x8fu))
            return 0;
        while (continuation_count-- != 0u) {
            const uint8_t continuation = bytes[offset++];
            if ((continuation & 0xc0u) != 0x80u) return 0;
            codepoint = (codepoint << 6u) | (continuation & 0x3fu);
        }
        if (codepoint > 0x10ffffu ||
            (codepoint >= 0xd800u && codepoint <= 0xdfffu))
            return 0;
    }
    return 1;
}

static int rin_sdk_gui_text_snapshot(RinStringV1 input, uint64_t max_bytes,
                                     char* output) {
    if (!output || input.size > max_bytes ||
        (input.size != 0u &&
         (input.address == 0u ||
          input.address > (uint64_t)UINTPTR_MAX ||
          input.size > (uint64_t)UINTPTR_MAX - input.address)))
        return 0;
    if (input.size != 0u) {
        rin_sdk_gui_copy((uint8_t*)output,
                         (const uint8_t*)(uintptr_t)input.address,
                         input.size);
        if (!rin_sdk_gui_utf8_valid((const uint8_t*)output, input.size))
            return 0;
    }
    output[input.size] = '\0';
    return 1;
}

static RinResult rin_sdk_gui_syscall_result(intptr_t result) {
    if (result == RIN_SUCCESS) return RIN_SUCCESS;
    if (result <= RIN_ERROR_INVALID_ARGUMENT &&
        result >= RIN_ERROR_WRONG_TYPE)
        return (RinResult)result;
    return RIN_ERROR_IO;
}

RinResult rin_notification_show_v1(RinStringV1 title, RinStringV1 message,
                                   uint32_t flags) {
    RinNotificationPostV1 request;
    char title_copy[RIN_NOTIFICATION_TITLE_MAX_BYTES + 1u];
    char message_copy[RIN_NOTIFICATION_MESSAGE_MAX_BYTES + 1u];
    uintptr_t title_address;
    uintptr_t message_address;

    if (flags > RIN_NOTIFICATION_KIND_ERROR || title.size == 0u ||
        title.size > RIN_NOTIFICATION_TITLE_MAX_BYTES ||
        message.size == 0u ||
        message.size > RIN_NOTIFICATION_MESSAGE_MAX_BYTES ||
        title.address == 0u || message.address == 0u ||
        title.address > (uint64_t)UINTPTR_MAX ||
        message.address > (uint64_t)UINTPTR_MAX)
        return RIN_ERROR_INVALID_ARGUMENT;
    title_address = (uintptr_t)title.address;
    message_address = (uintptr_t)message.address;
    if (title.size > (uint64_t)UINTPTR_MAX - title_address ||
        message.size > (uint64_t)UINTPTR_MAX - message_address)
        return RIN_ERROR_INVALID_ARGUMENT;

    rin_sdk_gui_copy((uint8_t*)title_copy, (const uint8_t*)title_address,
                     title.size);
    rin_sdk_gui_copy((uint8_t*)message_copy,
                     (const uint8_t*)message_address, message.size);
    if (!rin_sdk_gui_utf8_valid((const uint8_t*)title_copy, title.size) ||
        !rin_sdk_gui_utf8_valid((const uint8_t*)message_copy, message.size))
        return RIN_ERROR_INVALID_ARGUMENT;
    title_copy[title.size] = '\0';
    message_copy[message.size] = '\0';

    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = (uint32_t)sizeof(request);
    request.version = RIN_NOTIFICATION_V1_VERSION;
    request.kind = (uint16_t)flags;
    request.title = (uint64_t)(uintptr_t)title_copy;
    request.title_bytes = (uint32_t)title.size;
    request.message = (uint64_t)(uintptr_t)message_copy;
    request.message_bytes = (uint32_t)message.size;
    return rin_sdk_gui_syscall_result(_syscall1(
        (uintptr_t)RIN_SYS_GUI_NOTIFICATION_POST,
        (uintptr_t)&request));
}
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_MEDIA)
static int rin_media_range_valid_v1(uint64_t address, uint64_t size) {
    if (address == 0u || size == 0u || address > (uint64_t)UINTPTR_MAX)
        return 0;
    return size <= (uint64_t)UINTPTR_MAX - address;
}

RinResult rin_media_open_v1(const RinMediaOpenV1* request,RinMediaObject* object) {
    RinMediaOpenV1 request_snapshot;
    RinMediaObject kernel_object = RIN_HANDLE_INVALID;
    if (object != NULL) *object = RIN_HANDLE_INVALID;
    if (object == NULL) return RIN_ERROR_INVALID_ARGUMENT;
    if (!versioned(request, sizeof(*request)))
        return RIN_ERROR_ABI_MISMATCH;
    request_snapshot = *request;
    if (!versioned(&request_snapshot, sizeof(request_snapshot)))
        return RIN_ERROR_ABI_MISMATCH;
    if (request_snapshot.media_type != RIN_MEDIA_TYPE_IMAGE &&
        request_snapshot.media_type != RIN_MEDIA_TYPE_AUDIO &&
        request_snapshot.media_type != RIN_MEDIA_TYPE_VIDEO) {
        return RIN_ERROR_INVALID_ARGUMENT;
    }
    if (request_snapshot.flags != 0u ||
        request_snapshot.mime_type.size == 0u ||
        request_snapshot.mime_type.size > RIN_MEDIA_MAX_MIME_TYPE_BYTES_V1 ||
        !rin_media_range_valid_v1(request_snapshot.mime_type.address,
                                  request_snapshot.mime_type.size) ||
        request_snapshot.source.size > RIN_MEDIA_MAX_SOURCE_BYTES_V1 ||
        !rin_media_range_valid_v1(request_snapshot.source.address,
                                  request_snapshot.source.size)) {
        return RIN_ERROR_INVALID_ARGUMENT;
    }
    for (uint32_t index = 0u; index < 4u; ++index) {
        if (request_snapshot.reserved[index] != 0u)
            return RIN_ERROR_INVALID_ARGUMENT;
    }
    RinResult result = rin_sdk_register_object_close_hook_v1(
        rin_media_sdk_client_object_close_hook_v1);
    if (result != RIN_SUCCESS) return result;
    result = rin_sdk_invoke_handle_output_v1(
        RIN_SDK_LIBRARY_MEDIA, MEDIA_OPEN, &request_snapshot,
        sizeof(request_snapshot), &kernel_object);
    if (result != RIN_SUCCESS) return result;
    result = rin_media_sdk_client_open_v1(kernel_object, &request_snapshot);
    if (result != RIN_SUCCESS) {
        (void)rin_object_close_v1(kernel_object);
        return result;
    }
    *object = kernel_object;
    return RIN_SUCCESS;
}
RinResult rin_image_get_info_v1(RinMediaObject object, RinImageInfoV1* info) {
    RinResult result;
    if (!versioned(info, sizeof(*info))) return RIN_ERROR_ABI_MISMATCH;
    reset_versioned_output(info, sizeof(*info));
    result = rin_media_sdk_client_image_info_v1(object, info);
    if (result != RIN_SUCCESS) reset_versioned_output(info, sizeof(*info));
    return result;
}
RinResult rin_image_decode_v1(RinMediaObject object, uint32_t format,
                              RinSliceV1 output, uint64_t* required) {
    if (required != NULL) *required = 0u;
    if (required == NULL) return RIN_ERROR_INVALID_ARGUMENT;
    return rin_media_sdk_client_image_decode_v1(object, format, output,
                                                 required);
}
RinResult rin_audio_get_info_v1(RinMediaObject object,RinAudioInfoV1* info) {
    RinResult result;
    if (!versioned(info, sizeof(*info))) return RIN_ERROR_ABI_MISMATCH;
    reset_versioned_output(info, sizeof(*info));
    result = rin_media_sdk_client_audio_info_v1(object, info);
    if (result != RIN_SUCCESS) reset_versioned_output(info, sizeof(*info));
    return result;
}
RinResult rin_audio_decode_v1(RinMediaObject object,RinSliceV1 output,uint64_t* frames) {
    if (frames != NULL) *frames = 0u;
    if (frames == NULL) return RIN_ERROR_INVALID_ARGUMENT;
    return rin_media_sdk_client_audio_decode_v1(object, output, frames);
}
RinResult rin_video_decode_v1(RinMediaObject object, RinSurface target,
                              uint64_t* timestamp_ns) {
    RinGuiSurfaceMapV1 mapping = {0};
    RinRenderTarget render_target = {0};
    RinVideoFrameInfoV1 frame_info = {0};
    RinResult result;
    RinResult unmap_result;

    if (timestamp_ns != NULL) *timestamp_ns = 0u;
    if (timestamp_ns == NULL || target == RIN_HANDLE_INVALID)
        return RIN_ERROR_INVALID_ARGUMENT;

    result = rin_surface_map_v1(target, &mapping);
    if (result != RIN_SUCCESS) return result;
    if (mapping.struct_size != sizeof(mapping) ||
        mapping.version != RIN_SDK_STRUCT_VERSION_1 ||
        mapping.address == 0u || mapping.address > (uint64_t)UINTPTR_MAX ||
        mapping.width == 0u || mapping.height == 0u ||
        mapping.width > UINT32_MAX / 4u ||
        mapping.stride_bytes < mapping.width * 4u ||
        mapping.generation == 0u ||
        (mapping.format != RIN_GUI_PIXEL_FORMAT_BGRA32 &&
         mapping.format != RIN_GUI_PIXEL_FORMAT_RGBA32) ||
        mapping.reserved[0] != 0u || mapping.reserved[1] != 0u) {
        (void)rin_surface_unmap_v1(target, &mapping);
        return RIN_ERROR_INTEGRITY;
    }

    render_target.struct_size = sizeof(render_target);
    render_target.version = RIN_RENDER_TARGET_VERSION;
    render_target.pixels = (void*)(uintptr_t)mapping.address;
    render_target.width = mapping.width;
    render_target.height = mapping.height;
    render_target.pitch = mapping.stride_bytes;
    render_target.format = mapping.format;
    render_target.generation = mapping.generation;
    render_target.buffer_slot = 0u;
    render_target.flags = RIN_RENDER_TARGET_FLAG_WRITABLE |
                          RIN_RENDER_TARGET_FLAG_EXTERNAL;

    result = rin_media_sdk_client_video_decode_frame_v1(
        object, &render_target, NULL, &frame_info);
    unmap_result = rin_surface_unmap_v1(target, &mapping);
    if (unmap_result != RIN_SUCCESS) return unmap_result;
    if (result != RIN_SUCCESS) return result;
    if ((frame_info.flags & RIN_VIDEO_FRAME_FLAG_END_OF_STREAM) != 0u)
        return RIN_ERROR_NOT_FOUND;
    *timestamp_ns =
        (frame_info.flags & RIN_VIDEO_FRAME_FLAG_TIMESTAMP_KNOWN) != 0u
            ? frame_info.timestamp_ns
            : 0u;
    return RIN_SUCCESS;
}
RinResult rin_video_get_info_v1(RinMediaObject object, RinVideoInfoV1* info) {
    RinResult result;
    if (!versioned(info, sizeof(*info))) return RIN_ERROR_ABI_MISMATCH;
    reset_versioned_output(info, sizeof(*info));
    result = rin_media_sdk_client_video_info_v1(object, info);
    if (result != RIN_SUCCESS) reset_versioned_output(info, sizeof(*info));
    return result;
}
RinResult rin_video_decode_frame_v1(RinMediaObject object,
                                     RinRenderTarget* target,
                                     RinVideoFrameInfoV1* frame_info) {
    return rin_video_decode_frame_with_options_v1(object, target, NULL,
                                                   frame_info);
}
RinResult rin_video_decode_frame_with_options_v1(
    RinMediaObject object, RinRenderTarget* target,
    const RinVideoDecodeOptionsV1* options,
    RinVideoFrameInfoV1* frame_info) {
    RinRenderTarget target_snapshot;
    RinVideoDecodeOptionsV1 options_snapshot;
    RinResult result;
    if (!versioned(frame_info, sizeof(*frame_info)))
        return RIN_ERROR_ABI_MISMATCH;
    if (target == NULL) {
        reset_versioned_output(frame_info, sizeof(*frame_info));
        return RIN_ERROR_INVALID_ARGUMENT;
    }
    target_snapshot = *target;
    if (options != NULL) {
        options_snapshot = *options;
        if (!versioned(&options_snapshot, sizeof(options_snapshot)) ||
            options_snapshot.reserved[0] != 0u ||
            options_snapshot.reserved[1] != 0u) {
            reset_versioned_output(frame_info, sizeof(*frame_info));
            return RIN_ERROR_ABI_MISMATCH;
        }
    }
    reset_versioned_output(frame_info, sizeof(*frame_info));
    result = rin_media_sdk_client_video_decode_frame_v1(
        object, &target_snapshot,
        options != NULL ? &options_snapshot : NULL, frame_info);
    if (result != RIN_SUCCESS) reset_versioned_output(frame_info,
                                                       sizeof(*frame_info));
    return result;
}
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_CONFIG)
RinResult rin_config_open_v1(RinStringV1 domain,uint32_t flags,RinConfigStore* store) { HANDLE_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_CONFIG,CONFIG_OPEN,store,domain.address,domain.size,flags,0,0,0); }
RinResult rin_config_get_v1(RinConfigStore store,RinStringV1 key,RinSliceV1 value,uint64_t* required) { SCALAR_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_CONFIG,CONFIG_GET,required,store,key.address,key.size,value.address,value.size,0); }
RinResult rin_config_set_v1(RinConfigStore store,RinStringV1 key,RinSliceV1 value,uint32_t flags) { SIMPLE_CALL(RIN_SDK_LIBRARY_CONFIG,CONFIG_SET,(RinResult*)0,store,key.address,key.size,value.address,value.size,flags); }
RinResult rin_config_commit_v1(RinConfigStore store) { SIMPLE_CALL(RIN_SDK_LIBRARY_CONFIG,CONFIG_COMMIT,(RinResult*)0,store,0,0,0,0,0); }
RinResult rin_config_remove_v1(RinConfigStore store,RinStringV1 key,uint32_t flags) { SIMPLE_CALL(RIN_SDK_LIBRARY_CONFIG,CONFIG_REMOVE,(RinResult*)0,store,key.address,key.size,flags,0,0); }
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_PKG)
RinResult rin_package_open_v1(RinStringV1 name,RinPackage* package) { HANDLE_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_PKG,PKG_OPEN,package,name.address,name.size,0,0,0,0); }
RinResult rin_package_query_v1(RinPackage package,RinPackageInfoV1* info) { VERSIONED_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_PKG,PKG_QUERY,info,package,0,0,0,0,0); }
RinResult rin_package_install_v1(RinStringV1 source,uint32_t flags,RinEvent completion) { SIMPLE_CALL(RIN_SDK_LIBRARY_PKG,PKG_INSTALL,(RinResult*)0,source.address,source.size,flags,completion,0,0); }
RinResult rin_package_remove_v1(RinStringV1 name,uint32_t flags,RinEvent completion) { SIMPLE_CALL(RIN_SDK_LIBRARY_PKG,PKG_REMOVE,(RinResult*)0,name.address,name.size,flags,completion,0,0); }
RinResult rin_package_enumerate_v1(uint64_t index,RinPackageInfoV1* info) { VERSIONED_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_PKG,PKG_ENUMERATE,info,index,0,0,0,0,0); }
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_DEVICE)
RinResult rin_device_enumerate_v1(uint32_t device_class,uint64_t index,RinDeviceInfoV1* info) { VERSIONED_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_DEVICE,DEVICE_ENUMERATE,info,device_class,index,0,0,0,0); }
RinResult rin_device_open_v1(uint64_t device_id,uint32_t rights,RinDevice* device) { HANDLE_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_DEVICE,DEVICE_OPEN,device,device_id,rights,0,0,0,0); }
RinResult rin_device_control_v1(RinDevice device,uint32_t operation,RinSliceV1 input,RinSliceV1 output,uint64_t* transferred) { SCALAR_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_DEVICE,DEVICE_CONTROL,transferred,device,operation,input.address,input.size,output.address,output.size); }
RinResult rin_device_query_v1(RinDevice device,RinDeviceInfoV1* info) { VERSIONED_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_DEVICE,DEVICE_QUERY,info,device,0,0,0,0,0); }
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_GPU)
static int gpu_allocation_info_valid(
    const RinGpuAllocationInfoV1* info, RinGpuAllocationV1 allocation) {
    const uint32_t known_flags = RIN_GPU_ALLOCATION_GPU_READ |
                                 RIN_GPU_ALLOCATION_GPU_WRITE |
                                 RIN_GPU_ALLOCATION_CPU_VISIBLE |
                                 RIN_GPU_ALLOCATION_ZEROED;

    if (!info || info->struct_size != sizeof(*info) ||
        info->version != RIN_GPU_ALLOCATION_INFO_VERSION_V1 ||
        (info->heap != RIN_GPU_HEAP_LOCAL &&
         info->heap != RIN_GPU_HEAP_SYSTEM) ||
        (info->flags & ~known_flags) != 0u ||
        (info->flags & (RIN_GPU_ALLOCATION_GPU_READ |
                        RIN_GPU_ALLOCATION_GPU_WRITE)) == 0u ||
        info->allocation != allocation || info->reserved != 0u ||
        info->requested_size_bytes == 0u ||
        info->allocation_size_bytes < info->requested_size_bytes ||
        info->alignment == 0u ||
        (info->alignment & (info->alignment - 1u)) != 0u ||
        info->gpu_virtual_address % info->alignment != 0u ||
        info->heap_offset % info->alignment != 0u ||
        info->iommu_map_generation == 0u || info->device_epoch == 0u ||
        info->lease_count > RIN_GPU_ALLOCATION_MAX_LEASES_V1 ||
        info->state != RIN_GPU_ALLOCATION_STATE_ACTIVE_V1)
        return 0;
    return 1;
}

RinResult rin_gpu_memory_allocate_v1(
    uint64_t device_id, uint64_t device_generation,
    const RinGpuAllocationDescV1* descriptor,
    RinGpuAllocationV1* allocation_out) {
    RinSdkArgsV1 request;
    RinResult result;

    if (allocation_out) *allocation_out = 0u;
    if (device_id == 0u || device_generation == 0u || !descriptor ||
        descriptor->struct_size != sizeof(*descriptor) ||
        descriptor->version != RIN_GPU_ALLOCATION_DESC_VERSION_V1 ||
        descriptor->heap < RIN_GPU_HEAP_LOCAL ||
        descriptor->heap > RIN_GPU_HEAP_SYSTEM ||
        (descriptor->flags & ~(RIN_GPU_ALLOCATION_GPU_READ |
                               RIN_GPU_ALLOCATION_GPU_WRITE |
                               RIN_GPU_ALLOCATION_CPU_VISIBLE |
                               RIN_GPU_ALLOCATION_ZEROED)) != 0u ||
        (descriptor->flags & (RIN_GPU_ALLOCATION_GPU_READ |
                              RIN_GPU_ALLOCATION_GPU_WRITE)) == 0u ||
        descriptor->size_bytes == 0u ||
        !allocation_out)
        return RIN_ERROR_INVALID_ARGUMENT;
    for (uint32_t index = 0u; index < 4u; ++index)
        if (descriptor->reserved[index] != 0u)
            return RIN_ERROR_INVALID_ARGUMENT;

    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = device_id;
    request.value[1] = device_generation;
    request.value[2] = descriptor->heap;
    request.value[3] = descriptor->flags;
    request.value[4] = descriptor->size_bytes;
    request.value[5] = descriptor->alignment;
    result = rin_sdk_invoke_scalar_output_v1(
        RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_MEMORY_ALLOCATE,
        &request, sizeof(request), allocation_out, sizeof(*allocation_out));
    if (result == RIN_SUCCESS && *allocation_out == 0u)
        result = RIN_ERROR_ABI_MISMATCH;
    if (result != RIN_SUCCESS) *allocation_out = 0u;
    return result;
}

RinResult rin_gpu_memory_query_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuAllocationV1 allocation, RinGpuAllocationInfoV1* info_out) {
    RinSdkArgsV1 request;
    RinResult result;
    if (!info_out || device_id == 0u || device_generation == 0u ||
        allocation == 0u)
        return RIN_ERROR_INVALID_ARGUMENT;
    if (!versioned(info_out, sizeof(*info_out)))
        return RIN_ERROR_ABI_MISMATCH;

    reset_versioned_output(info_out, sizeof(*info_out));
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = device_id;
    request.value[1] = device_generation;
    request.value[2] = allocation;
    result = rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_MEMORY_QUERY, &request,
        sizeof(request), info_out, sizeof(*info_out));
    if (result == RIN_SUCCESS &&
        !gpu_allocation_info_valid(info_out, allocation))
        result = RIN_ERROR_ABI_MISMATCH;
    return finalize_versioned_output(result, info_out, sizeof(*info_out));
}

RinResult rin_gpu_memory_destroy_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuAllocationV1 allocation) {
    if (device_id == 0u || device_generation == 0u || allocation == 0u)
        return RIN_ERROR_INVALID_ARGUMENT;
    SIMPLE_CALL(RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_MEMORY_DESTROY,
                (RinResult*)0, device_id, device_generation, allocation,
                0, 0, 0);
}

RinResult rin_gpu_memory_sync_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuAllocationV1 allocation, uint32_t action, uint64_t offset,
    uint64_t length) {
    if (device_id == 0u || device_generation == 0u || allocation == 0u ||
        (action != RIN_GPU_SYNC_CPU_TO_DEVICE &&
         action != RIN_GPU_SYNC_DEVICE_TO_CPU) ||
        length == 0u || length - 1u > UINT64_MAX - offset)
        return RIN_ERROR_INVALID_ARGUMENT;
    SIMPLE_CALL(RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_MEMORY_SYNC,
                (RinResult*)0, device_id, device_generation, allocation,
                action, offset, length);
}

RinResult rin_gpu_memory_upload_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuAllocationV1 allocation, uint64_t offset, const void* source,
    uint64_t length) {
    uintptr_t address = (uintptr_t)source;
    if (device_id == 0u || device_generation == 0u || allocation == 0u ||
        !source || length == 0u ||
        length > RIN_GPU_MEMORY_MAX_TRANSFER_BYTES ||
        length - 1u > UINT64_MAX - offset ||
        length - 1u > (uint64_t)UINTPTR_MAX - address)
        return RIN_ERROR_INVALID_ARGUMENT;
    SIMPLE_CALL(RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_MEMORY_UPLOAD,
                (RinResult*)0, device_id, device_generation, allocation,
                offset, length, (uint64_t)address);
}

RinResult rin_gpu_memory_readback_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuAllocationV1 allocation, uint64_t offset, void* destination,
    uint64_t length) {
    uintptr_t address = (uintptr_t)destination;
    if (device_id == 0u || device_generation == 0u || allocation == 0u ||
        !destination || length == 0u ||
        length > RIN_GPU_MEMORY_MAX_TRANSFER_BYTES ||
        length - 1u > UINT64_MAX - offset ||
        length - 1u > (uint64_t)UINTPTR_MAX - address)
        return RIN_ERROR_INVALID_ARGUMENT;
    SIMPLE_CALL(RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_MEMORY_READBACK,
                (RinResult*)0, device_id, device_generation, allocation,
                offset, length, (uint64_t)address);
}

RinResult rin_gpu_memory_map_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuAllocationV1 allocation, uint32_t access,
    RinGpuMemoryMappingV1* mapping_out) {
    RinSdkArgsV1 request;
    RinResult result;
    uint64_t mapping_lease;

    if (!mapping_out || device_id == 0u || device_generation == 0u ||
        allocation == 0u ||
        (access & ~(RIN_GPU_MAP_CPU_READ | RIN_GPU_MAP_CPU_WRITE)) != 0u ||
        access == 0u)
        return RIN_ERROR_INVALID_ARGUMENT;
    if (!versioned(mapping_out, sizeof(*mapping_out)))
        return RIN_ERROR_ABI_MISMATCH;
    rin_sdk_zero_bytes(mapping_out, sizeof(*mapping_out));
    mapping_out->struct_size = sizeof(*mapping_out);
    mapping_out->version = RIN_SDK_STRUCT_VERSION_1;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = device_id;
    request.value[1] = device_generation;
    request.value[2] = allocation;
    request.value[3] = access;
    result = rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_MEMORY_MAP, &request,
        sizeof(request), mapping_out, sizeof(*mapping_out));
    if (result == RIN_SUCCESS &&
        (mapping_out->struct_size != sizeof(*mapping_out) ||
         mapping_out->version != RIN_SDK_STRUCT_VERSION_1 ||
         mapping_out->access != access || mapping_out->reserved0 != 0u ||
         mapping_out->allocation != allocation || mapping_out->address == 0u ||
         mapping_out->address > (uint64_t)UINTPTR_MAX ||
         mapping_out->size_bytes == 0u ||
         mapping_out->size_bytes - 1u >
             (uint64_t)UINTPTR_MAX - mapping_out->address ||
         mapping_out->mapping == 0u ||
         mapping_out->device_generation != device_generation ||
         mapping_out->reserved != 0u)) {
        mapping_lease = mapping_out->mapping;
        if (mapping_lease != 0u) {
            RinResult cleanup_result = rin_gpu_memory_unmap_v1(
                device_id, device_generation, allocation, mapping_lease);
            if (cleanup_result != RIN_SUCCESS) {
                /* Never expose a possibly invalid CPU address. Keep only the
                 * owner-bound token so the caller can retry cleanup. */
                rin_sdk_zero_bytes(mapping_out, sizeof(*mapping_out));
                mapping_out->struct_size = sizeof(*mapping_out);
                mapping_out->version = RIN_SDK_STRUCT_VERSION_1;
                mapping_out->access = access;
                mapping_out->allocation = allocation;
                mapping_out->mapping = mapping_lease;
                mapping_out->device_generation = device_generation;
                return cleanup_result;
            }
        }
        result = RIN_ERROR_ABI_MISMATCH;
    }
    return finalize_versioned_output(result, mapping_out,
                                     sizeof(*mapping_out));
}

RinResult rin_gpu_memory_unmap_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuAllocationV1 allocation, uint64_t mapping) {
    if (device_id == 0u || device_generation == 0u || allocation == 0u ||
        mapping == 0u)
        return RIN_ERROR_INVALID_ARGUMENT;
    SIMPLE_CALL(RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_MEMORY_UNMAP,
                (RinResult*)0, device_id, device_generation, allocation,
                mapping, 0, 0);
}

static int gpu_process_slice_valid(RinSliceV1 slice) {
    return slice.address != 0u && slice.size != 0u &&
           slice.address <= (uint64_t)UINTPTR_MAX &&
           slice.size - 1u <= (uint64_t)UINTPTR_MAX - slice.address;
}

/* The submit receipt is written before the syscall and again on failure. Do
 * not clear it until every caller-owned input range is proven disjoint; an
 * aliased output would otherwise corrupt the descriptor or command/resource
 * bytes before the kernel has copied them. Treat malformed/overflowing ranges
 * as unsafe to write through, leaving the caller's memory untouched. */
static int gpu_process_submit_output_disjoint(
    const RinGpuProcessSubmitReceiptV1* receipt,
    const void* descriptor, size_t descriptor_size,
    RinSliceV1 commands, RinSliceV1 resources, int has_resources) {
    const uintptr_t output_begin = (uintptr_t)receipt;
    const uintptr_t descriptor_begin = (uintptr_t)descriptor;
    uintptr_t output_end;
    uintptr_t descriptor_end;

    if (!receipt || !descriptor || descriptor_size == 0u ||
        sizeof(*receipt) - 1u > UINTPTR_MAX - output_begin ||
        descriptor_size - 1u > UINTPTR_MAX - descriptor_begin)
        return 0;
    output_end = output_begin + sizeof(*receipt) - 1u;
    descriptor_end = descriptor_begin + descriptor_size - 1u;
    if (output_begin <= descriptor_end && descriptor_begin <= output_end)
        return 0;

    if (!gpu_process_slice_valid(commands)) return 0;
    {
        const uintptr_t input_begin = (uintptr_t)commands.address;
        const uintptr_t input_end = input_begin + (uintptr_t)commands.size - 1u;
        if (output_begin <= input_end && input_begin <= output_end) return 0;
    }
    if (has_resources) {
        if (!gpu_process_slice_valid(resources)) return 0;
        {
            const uintptr_t input_begin = (uintptr_t)resources.address;
            const uintptr_t input_end =
                input_begin + (uintptr_t)resources.size - 1u;
            if (output_begin <= input_end && input_begin <= output_end)
                return 0;
        }
    }
    return 1;
}

RinResult rin_gpu_process_queue_bind_v1(
    uint64_t device_id, uint64_t device_generation, uint32_t queue_id,
    RinGpuProcessQueueV1* queue_out) {
    RinSdkArgsV1 request;
    RinResult result;

    if (queue_out) *queue_out = 0u;
    if (!queue_out || device_id == 0u || device_generation == 0u)
        return RIN_ERROR_INVALID_ARGUMENT;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = device_id;
    request.value[1] = device_generation;
    request.value[2] = queue_id;
    result = rin_sdk_invoke_scalar_output_v1(
        RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_PROCESS_QUEUE_BIND,
        &request, sizeof(request), queue_out, sizeof(*queue_out));
    if (result == RIN_SUCCESS && *queue_out == 0u)
        result = RIN_ERROR_ABI_MISMATCH;
    if (result != RIN_SUCCESS) *queue_out = 0u;
    return result;
}

RinResult rin_gpu_process_queue_release_v1(RinGpuProcessQueueV1 queue) {
    if (queue == 0u) return RIN_ERROR_INVALID_ARGUMENT;
    SIMPLE_CALL(RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_PROCESS_QUEUE_RELEASE,
                (RinResult*)0, queue, 0, 0, 0, 0, 0);
}

RinResult rin_gpu_process_submit_v1(
    RinGpuProcessQueueV1 queue,
    const RinGpuProcessSubmitDescV1* descriptor,
    RinGpuProcessSubmitReceiptV1* receipt_out) {
    RinGpuProcessSubmitDescV1 request_desc;
    RinSdkArgsV1 request;
    RinResult result;
    uint64_t expected_commands_size;
    uint64_t expected_resources_size;

    if (!receipt_out || !descriptor ||
        descriptor->struct_size != sizeof(*descriptor) ||
        descriptor->version != RIN_GPU_PROCESS_SUBMIT_DESC_VERSION_V1 ||
        !gpu_process_submit_output_disjoint(
            receipt_out, descriptor, sizeof(*descriptor),
            descriptor->commands, descriptor->resources,
            descriptor->resource_count != 0u))
        return RIN_ERROR_INVALID_ARGUMENT;
    rin_sdk_zero_bytes(receipt_out, sizeof(*receipt_out));
    receipt_out->struct_size = sizeof(*receipt_out);
    receipt_out->version = RIN_GPU_PROCESS_SUBMIT_RECEIPT_VERSION_V1;
    if (queue == 0u ||
        descriptor->reserved != 0u || descriptor->command_count == 0u ||
        descriptor->command_count > RIN_GPU_PROCESS_MAX_COMMANDS_V1 ||
        descriptor->command_record_size !=
            RIN_GPU_PROCESS_COMMAND_RECORD_SIZE_V1 ||
        descriptor->resource_count > RIN_GPU_PROCESS_MAX_RESOURCES_V1 ||
        descriptor->command_count >
            UINT64_MAX / descriptor->command_record_size)
        return RIN_ERROR_INVALID_ARGUMENT;
    expected_commands_size = (uint64_t)descriptor->command_count *
                             descriptor->command_record_size;
    if (!gpu_process_slice_valid(descriptor->commands) ||
        descriptor->commands.size != expected_commands_size)
        return RIN_ERROR_INVALID_ARGUMENT;
    if (descriptor->resource_count == 0u) {
        if (descriptor->resources.address != 0u ||
            descriptor->resources.size != 0u)
            return RIN_ERROR_INVALID_ARGUMENT;
    } else {
        expected_resources_size =
            (uint64_t)descriptor->resource_count *
            sizeof(RinGpuProcessResourceV1);
        if (!gpu_process_slice_valid(descriptor->resources) ||
            descriptor->resources.size != expected_resources_size)
            return RIN_ERROR_INVALID_ARGUMENT;
    }

    request_desc = *descriptor;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = queue;
    request.value[1] = (uint64_t)(uintptr_t)&request_desc;
    result = rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_PROCESS_SUBMIT,
        &request, sizeof(request), receipt_out, sizeof(*receipt_out));
    if (result == RIN_SUCCESS &&
        (receipt_out->struct_size != sizeof(*receipt_out) ||
         receipt_out->version != RIN_GPU_PROCESS_SUBMIT_RECEIPT_VERSION_V1 ||
         receipt_out->queue_id >= RIN_GPU_PROCESS_MAX_QUEUES_V1 ||
         receipt_out->reserved0 != 0u || receipt_out->sequence == 0u ||
         receipt_out->completion_value == 0u ||
         receipt_out->device_epoch == 0u ||
         receipt_out->iommu_map_generation == 0u ||
         receipt_out->reserved[0] != 0u ||
         receipt_out->reserved[1] != 0u))
        result = RIN_ERROR_ABI_MISMATCH;
    if (result != RIN_SUCCESS) {
        rin_sdk_zero_bytes(receipt_out, sizeof(*receipt_out));
        receipt_out->struct_size = sizeof(*receipt_out);
        receipt_out->version = RIN_GPU_PROCESS_SUBMIT_RECEIPT_VERSION_V1;
    }
    return result;
}

RinResult rin_gpu_process_submit_v2(
    RinGpuProcessQueueV1 queue,
    const RinGpuProcessSubmitDescV2* descriptor,
    RinGpuProcessSubmitReceiptV1* receipt_out) {
    RinGpuProcessSubmitDescV2 request_desc;
    RinSdkArgsV1 request;
    const RinSliceV1 no_resources = {0u, 0u};
    RinResult result;
    uint64_t expected_commands_size;

    if (!receipt_out || !descriptor ||
        descriptor->struct_size != sizeof(*descriptor) ||
        descriptor->version != RIN_GPU_PROCESS_SUBMIT_DESC_VERSION_V2 ||
        !gpu_process_submit_output_disjoint(
            receipt_out, descriptor, sizeof(*descriptor),
            descriptor->commands, no_resources, 0))
        return RIN_ERROR_INVALID_ARGUMENT;
    rin_sdk_zero_bytes(receipt_out, sizeof(*receipt_out));
    receipt_out->struct_size = sizeof(*receipt_out);
    receipt_out->version = RIN_GPU_PROCESS_SUBMIT_RECEIPT_VERSION_V1;
    if (queue == 0u ||
        descriptor->reserved[0] != 0u || descriptor->reserved[1] != 0u ||
        descriptor->command_count == 0u ||
        descriptor->command_count > RIN_GPU_PROCESS_MAX_COMMANDS_V1 ||
        descriptor->command_record_size !=
            RIN_GPU_PROCESS_COMMAND_RECORD_SIZE_V1 ||
        descriptor->command_count >
            UINT64_MAX / descriptor->command_record_size)
        return RIN_ERROR_INVALID_ARGUMENT;
    expected_commands_size = (uint64_t)descriptor->command_count *
                             descriptor->command_record_size;
    if (!gpu_process_slice_valid(descriptor->commands) ||
        descriptor->commands.size != expected_commands_size)
        return RIN_ERROR_INVALID_ARGUMENT;

    request_desc = *descriptor;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = queue;
    request.value[1] = (uint64_t)(uintptr_t)&request_desc;
    result = rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_PROCESS_SUBMIT_V2,
        &request, sizeof(request), receipt_out, sizeof(*receipt_out));
    if (result == RIN_SUCCESS &&
        (receipt_out->struct_size != sizeof(*receipt_out) ||
         receipt_out->version != RIN_GPU_PROCESS_SUBMIT_RECEIPT_VERSION_V1 ||
         receipt_out->queue_id >= RIN_GPU_PROCESS_MAX_QUEUES_V1 ||
         receipt_out->reserved0 != 0u || receipt_out->sequence == 0u ||
         receipt_out->completion_value == 0u ||
         receipt_out->device_epoch == 0u ||
         receipt_out->iommu_map_generation == 0u ||
         receipt_out->reserved[0] != 0u ||
         receipt_out->reserved[1] != 0u))
        result = RIN_ERROR_ABI_MISMATCH;
    if (result != RIN_SUCCESS) {
        rin_sdk_zero_bytes(receipt_out, sizeof(*receipt_out));
        receipt_out->struct_size = sizeof(*receipt_out);
        receipt_out->version = RIN_GPU_PROCESS_SUBMIT_RECEIPT_VERSION_V1;
    }
    return result;
}

RinResult rin_gpu_process_queue_query_timeline_v1(
    RinGpuProcessQueueV1 queue, uint64_t expected_device_epoch,
    uint64_t expected_iommu_map_generation,
    RinGpuProcessQueueTimelineV1* timeline_out) {
    RinSdkArgsV1 request;
    RinResult result;

    if (timeline_out) {
        rin_sdk_zero_bytes(timeline_out, sizeof(*timeline_out));
        timeline_out->struct_size = sizeof(*timeline_out);
        timeline_out->version = RIN_GPU_PROCESS_QUEUE_TIMELINE_VERSION_V1;
    }
    if (!timeline_out || queue == 0u || expected_device_epoch == 0u ||
        expected_iommu_map_generation == 0u)
        return RIN_ERROR_INVALID_ARGUMENT;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = queue;
    request.value[1] = expected_device_epoch;
    request.value[2] = expected_iommu_map_generation;
    result = rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_PROCESS_QUEUE_TIMELINE_QUERY,
        &request, sizeof(request), timeline_out, sizeof(*timeline_out));
    if (result == RIN_SUCCESS &&
        (timeline_out->struct_size != sizeof(*timeline_out) ||
         timeline_out->version !=
             RIN_GPU_PROCESS_QUEUE_TIMELINE_VERSION_V1 ||
         timeline_out->queue_id >= RIN_GPU_PROCESS_MAX_QUEUES_V1 ||
         timeline_out->reserved0 != 0u ||
         timeline_out->device_epoch != expected_device_epoch ||
         timeline_out->iommu_map_generation !=
             expected_iommu_map_generation))
        result = RIN_ERROR_ABI_MISMATCH;
    if (result != RIN_SUCCESS) {
        rin_sdk_zero_bytes(timeline_out, sizeof(*timeline_out));
        timeline_out->struct_size = sizeof(*timeline_out);
        timeline_out->version = RIN_GPU_PROCESS_QUEUE_TIMELINE_VERSION_V1;
    }
    return result;
}

RinResult rin_gpu_process_resource_bind_v1(
    uint64_t device_id, uint64_t device_generation,
    const RinGpuProcessResourceBindingV1* binding) {
    RinGpuProcessResourceBindingV1 request_binding;
    RinSdkArgsV1 request;

    if (device_id == 0u || device_generation == 0u || !binding ||
        binding->struct_size != sizeof(*binding) ||
        binding->version != RIN_GPU_PROCESS_RESOURCE_BINDING_VERSION_V1 ||
        binding->resource_cookie == 0u || binding->allocation == 0u ||
        binding->size_bytes == 0u || binding->reserved != 0u ||
        binding->allocation_offset > UINT64_MAX - binding->size_bytes ||
        binding->required_gpu_access == 0u ||
        (binding->required_gpu_access &
         ~(RIN_GPU_ALLOCATION_GPU_READ | RIN_GPU_ALLOCATION_GPU_WRITE)) != 0u)
        return RIN_ERROR_INVALID_ARGUMENT;
    request_binding = *binding;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = device_id;
    request.value[1] = device_generation;
    request.value[2] = (uint64_t)(uintptr_t)&request_binding;
    return rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_PROCESS_RESOURCE_BIND,
        &request, sizeof(request), (void*)0, 0u);
}

RinResult rin_gpu_process_resource_unbind_v1(
    uint64_t device_id, uint64_t device_generation,
    uint64_t resource_cookie) {
    if (device_id == 0u || device_generation == 0u || resource_cookie == 0u)
        return RIN_ERROR_INVALID_ARGUMENT;
    SIMPLE_CALL(RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_PROCESS_RESOURCE_UNBIND,
                (RinResult*)0, device_id, device_generation,
                resource_cookie, 0, 0, 0);
}

static int gpu_process_buffer_desc_valid(
    const RinGpuProcessBufferDescV1* descriptor) {
    return descriptor && descriptor->struct_size == sizeof(*descriptor) &&
           descriptor->version == RIN_GPU_PROCESS_BUFFER_DESC_VERSION_V1 &&
           descriptor->device_id != 0u &&
           descriptor->device_generation != 0u &&
           descriptor->resource_cookie != 0u &&
           descriptor->allocation != 0u && descriptor->size_bytes != 0u &&
           descriptor->allocation_offset <=
               UINT64_MAX - descriptor->size_bytes &&
           descriptor->required_gpu_access != 0u &&
           (descriptor->required_gpu_access &
            ~(RIN_GPU_ALLOCATION_GPU_READ | RIN_GPU_ALLOCATION_GPU_WRITE)) ==
               0u &&
           descriptor->reserved == 0u;
}

static int gpu_process_buffer_range_valid(
    const RinGpuProcessBufferV1* buffer, uint64_t offset, uint64_t length) {
    return buffer && buffer->struct_size == sizeof(*buffer) &&
           buffer->version == RIN_GPU_PROCESS_BUFFER_VERSION_V1 &&
           buffer->state == RIN_GPU_PROCESS_BUFFER_STATE_BOUND_V1 &&
           buffer->reserved0 == 0u &&
           gpu_process_buffer_desc_valid(&buffer->desc) && length != 0u &&
           offset <= buffer->desc.size_bytes &&
           length <= buffer->desc.size_bytes - offset;
}

RinResult rin_gpu_process_buffer_bind_v1(
    const RinGpuProcessBufferDescV1* descriptor,
    RinGpuProcessBufferV1* buffer_out) {
    RinGpuProcessBufferDescV1 descriptor_copy;
    RinGpuProcessResourceBindingV1 binding;
    RinGpuProcessBufferV1 buffer;
    RinResult result;

    if (!descriptor || !buffer_out)
        return RIN_ERROR_INVALID_ARGUMENT;
    descriptor_copy = *descriptor;
    if (!gpu_process_buffer_desc_valid(&descriptor_copy))
        return RIN_ERROR_INVALID_ARGUMENT;

    rin_sdk_zero_bytes(&binding, sizeof(binding));
    binding.struct_size = sizeof(binding);
    binding.version = RIN_GPU_PROCESS_RESOURCE_BINDING_VERSION_V1;
    binding.resource_cookie = descriptor_copy.resource_cookie;
    binding.allocation = descriptor_copy.allocation;
    binding.allocation_offset = descriptor_copy.allocation_offset;
    binding.size_bytes = descriptor_copy.size_bytes;
    binding.required_gpu_access = descriptor_copy.required_gpu_access;
    result = rin_gpu_process_resource_bind_v1(
        descriptor_copy.device_id, descriptor_copy.device_generation,
        &binding);
    if (result != RIN_SUCCESS)
        return result;

    rin_sdk_zero_bytes(&buffer, sizeof(buffer));
    buffer.struct_size = sizeof(buffer);
    buffer.version = RIN_GPU_PROCESS_BUFFER_VERSION_V1;
    buffer.state = RIN_GPU_PROCESS_BUFFER_STATE_BOUND_V1;
    buffer.desc = descriptor_copy;
    *buffer_out = buffer;
    return RIN_SUCCESS;
}

RinResult rin_gpu_process_buffer_unbind_v1(RinGpuProcessBufferV1* buffer) {
    RinGpuProcessBufferV1 buffer_copy;
    RinResult result;

    if (!buffer) return RIN_ERROR_INVALID_ARGUMENT;
    buffer_copy = *buffer;
    if (buffer_copy.struct_size != sizeof(buffer_copy) ||
        buffer_copy.version != RIN_GPU_PROCESS_BUFFER_VERSION_V1 ||
        buffer_copy.state != RIN_GPU_PROCESS_BUFFER_STATE_BOUND_V1 ||
        buffer_copy.reserved0 != 0u ||
        !gpu_process_buffer_desc_valid(&buffer_copy.desc))
        return RIN_ERROR_INVALID_ARGUMENT;

    result = rin_gpu_process_resource_unbind_v1(
        buffer_copy.desc.device_id, buffer_copy.desc.device_generation,
        buffer_copy.desc.resource_cookie);
    if (result == RIN_SUCCESS)
        rin_sdk_zero_bytes(buffer, sizeof(*buffer));
    return result;
}

static int gpu_process_buffer_mapping_valid(
    const RinGpuProcessBufferMappingV1* mapping) {
    if (!mapping || mapping->struct_size != sizeof(*mapping) ||
        mapping->version != RIN_GPU_PROCESS_BUFFER_MAPPING_VERSION_V1 ||
        mapping->device_id == 0u || mapping->device_generation == 0u ||
        mapping->allocation == 0u || mapping->mapping == 0u ||
        (mapping->access & ~(RIN_GPU_MAP_CPU_READ |
                             RIN_GPU_MAP_CPU_WRITE)) != 0u ||
        mapping->access == 0u || mapping->reserved != 0u)
        return 0;

    if (mapping->state ==
        RIN_GPU_PROCESS_BUFFER_MAPPING_STATE_UNMAP_PENDING_V1)
        return mapping->address == 0u && mapping->allocation_offset == 0u &&
               mapping->size_bytes == 0u;

    return mapping->state ==
               RIN_GPU_PROCESS_BUFFER_MAPPING_STATE_MAPPED_V1 &&
           mapping->address != 0u && mapping->size_bytes != 0u &&
           mapping->allocation_offset <=
               UINT64_MAX - mapping->size_bytes &&
           mapping->address <= (uint64_t)UINTPTR_MAX &&
           mapping->size_bytes - 1u <=
               (uint64_t)UINTPTR_MAX - mapping->address;
}

static void gpu_process_buffer_mapping_set_pending(
    RinGpuProcessBufferMappingV1* destination,
    const RinGpuProcessBufferDescV1* buffer, uint32_t access,
    uint64_t mapping_token) {
    RinGpuProcessBufferMappingV1 pending;

    rin_sdk_zero_bytes(&pending, sizeof(pending));
    pending.struct_size = sizeof(pending);
    pending.version = RIN_GPU_PROCESS_BUFFER_MAPPING_VERSION_V1;
    pending.state = RIN_GPU_PROCESS_BUFFER_MAPPING_STATE_UNMAP_PENDING_V1;
    pending.access = access;
    pending.device_id = buffer->device_id;
    pending.device_generation = buffer->device_generation;
    pending.allocation = buffer->allocation;
    pending.mapping = mapping_token;
    *destination = pending;
}

static int gpu_process_buffer_mapping_output_empty(
    const RinGpuProcessBufferMappingV1* mapping) {
    const unsigned char* bytes = (const unsigned char*)mapping;

    for (size_t index = 0u; index < sizeof(*mapping); ++index)
        if (bytes[index] != 0u) return 0;
    return 1;
}

RinResult rin_gpu_process_buffer_map_v1(
    const RinGpuProcessBufferV1* buffer,
    uint32_t access,
    RinGpuProcessBufferMappingV1* mapping_out) {
    RinGpuProcessBufferV1 buffer_copy;
    RinGpuMemoryMappingV1 allocation_mapping;
    RinGpuProcessBufferMappingV1 mapping;
    uint64_t view_end;
    uint64_t view_address;
    RinResult result;

    if (!buffer || !mapping_out ||
        (access & ~(RIN_GPU_MAP_CPU_READ | RIN_GPU_MAP_CPU_WRITE)) != 0u ||
        access == 0u)
        return RIN_ERROR_INVALID_ARGUMENT;
    if (!gpu_process_buffer_mapping_output_empty(mapping_out))
        return gpu_process_buffer_mapping_valid(mapping_out)
                   ? RIN_ERROR_BUSY
                   : RIN_ERROR_INVALID_ARGUMENT;
    buffer_copy = *buffer;
    if (!gpu_process_buffer_range_valid(&buffer_copy, 0u,
                                       buffer_copy.desc.size_bytes) ||
        buffer_copy.desc.allocation_offset >
            UINT64_MAX - buffer_copy.desc.size_bytes)
        return RIN_ERROR_INVALID_ARGUMENT;

    rin_sdk_zero_bytes(&allocation_mapping, sizeof(allocation_mapping));
    allocation_mapping.struct_size = sizeof(allocation_mapping);
    allocation_mapping.version = RIN_SDK_STRUCT_VERSION_1;
    result = rin_gpu_memory_map_v1(
        buffer_copy.desc.device_id, buffer_copy.desc.device_generation,
        buffer_copy.desc.allocation, access, &allocation_mapping);
    if (result != RIN_SUCCESS) {
        if (allocation_mapping.mapping != 0u &&
            allocation_mapping.struct_size == sizeof(allocation_mapping) &&
            allocation_mapping.version == RIN_SDK_STRUCT_VERSION_1 &&
            allocation_mapping.access == access &&
            allocation_mapping.reserved0 == 0u &&
            allocation_mapping.allocation == buffer_copy.desc.allocation &&
            allocation_mapping.address == 0u &&
            allocation_mapping.size_bytes == 0u &&
            allocation_mapping.device_generation ==
                buffer_copy.desc.device_generation &&
            allocation_mapping.reserved == 0u) {
            gpu_process_buffer_mapping_set_pending(
                mapping_out, &buffer_copy.desc, access,
                allocation_mapping.mapping);
        }
        return result;
    }

    view_end = buffer_copy.desc.allocation_offset +
               buffer_copy.desc.size_bytes;
    if (allocation_mapping.allocation != buffer_copy.desc.allocation ||
        allocation_mapping.device_generation !=
            buffer_copy.desc.device_generation ||
        allocation_mapping.access != access ||
        allocation_mapping.address == 0u ||
        allocation_mapping.address > (uint64_t)UINTPTR_MAX ||
        view_end > allocation_mapping.size_bytes ||
        buffer_copy.desc.allocation_offset > UINT64_MAX -
            allocation_mapping.address) {
        RinResult cleanup_result = rin_gpu_memory_unmap_v1(
            buffer_copy.desc.device_id, buffer_copy.desc.device_generation,
            buffer_copy.desc.allocation, allocation_mapping.mapping);
        if (cleanup_result != RIN_SUCCESS) {
            gpu_process_buffer_mapping_set_pending(
                mapping_out, &buffer_copy.desc, access,
                allocation_mapping.mapping);
            return cleanup_result;
        }
        return RIN_ERROR_ABI_MISMATCH;
    }

    view_address = allocation_mapping.address +
                   buffer_copy.desc.allocation_offset;
    if (view_address > (uint64_t)UINTPTR_MAX ||
        buffer_copy.desc.size_bytes - 1u >
            (uint64_t)UINTPTR_MAX - view_address) {
        RinResult cleanup_result = rin_gpu_memory_unmap_v1(
            buffer_copy.desc.device_id, buffer_copy.desc.device_generation,
            buffer_copy.desc.allocation, allocation_mapping.mapping);
        if (cleanup_result != RIN_SUCCESS) {
            gpu_process_buffer_mapping_set_pending(
                mapping_out, &buffer_copy.desc, access,
                allocation_mapping.mapping);
            return cleanup_result;
        }
        return RIN_ERROR_ABI_MISMATCH;
    }

    rin_sdk_zero_bytes(&mapping, sizeof(mapping));
    mapping.struct_size = sizeof(mapping);
    mapping.version = RIN_GPU_PROCESS_BUFFER_MAPPING_VERSION_V1;
    mapping.state = RIN_GPU_PROCESS_BUFFER_MAPPING_STATE_MAPPED_V1;
    mapping.access = access;
    mapping.device_id = buffer_copy.desc.device_id;
    mapping.device_generation = buffer_copy.desc.device_generation;
    mapping.allocation = buffer_copy.desc.allocation;
    mapping.mapping = allocation_mapping.mapping;
    mapping.address = view_address;
    mapping.allocation_offset = buffer_copy.desc.allocation_offset;
    mapping.size_bytes = buffer_copy.desc.size_bytes;
    *mapping_out = mapping;
    return RIN_SUCCESS;
}

RinResult rin_gpu_process_buffer_unmap_v1(
    RinGpuProcessBufferMappingV1* mapping) {
    RinGpuProcessBufferMappingV1 mapping_copy;
    RinResult result;

    if (!mapping) return RIN_ERROR_INVALID_ARGUMENT;
    mapping_copy = *mapping;
    if (!gpu_process_buffer_mapping_valid(&mapping_copy))
        return RIN_ERROR_INVALID_ARGUMENT;

    result = rin_gpu_memory_unmap_v1(
        mapping_copy.device_id, mapping_copy.device_generation,
        mapping_copy.allocation, mapping_copy.mapping);
    if (result == RIN_SUCCESS)
        rin_sdk_zero_bytes(mapping, sizeof(*mapping));
    return result;
}

static int gpu_process_allocated_buffer_desc_valid(
    const RinGpuProcessAllocatedBufferDescV1* descriptor) {
    const uint32_t known_allocation_flags =
        RIN_GPU_ALLOCATION_GPU_READ | RIN_GPU_ALLOCATION_GPU_WRITE |
        RIN_GPU_ALLOCATION_CPU_VISIBLE | RIN_GPU_ALLOCATION_ZEROED;

    if (!descriptor || descriptor->struct_size != sizeof(*descriptor) ||
        descriptor->version !=
            RIN_GPU_PROCESS_ALLOCATED_BUFFER_DESC_VERSION_V1 ||
        descriptor->device_id == 0u || descriptor->device_generation == 0u ||
        descriptor->resource_cookie == 0u ||
        descriptor->allocation.struct_size !=
            sizeof(descriptor->allocation) ||
        descriptor->allocation.version !=
            RIN_GPU_ALLOCATION_DESC_VERSION_V1 ||
        (descriptor->allocation.heap != RIN_GPU_HEAP_LOCAL &&
         descriptor->allocation.heap != RIN_GPU_HEAP_SYSTEM) ||
        (descriptor->allocation.flags & ~known_allocation_flags) != 0u ||
        (descriptor->allocation.flags &
         (RIN_GPU_ALLOCATION_GPU_READ | RIN_GPU_ALLOCATION_GPU_WRITE)) == 0u ||
        descriptor->allocation.size_bytes == 0u ||
        (descriptor->allocation.alignment != 0u &&
         (descriptor->allocation.alignment &
          (descriptor->allocation.alignment - 1u)) != 0u) ||
        descriptor->required_gpu_access == 0u ||
        (descriptor->required_gpu_access &
         ~(RIN_GPU_ALLOCATION_GPU_READ | RIN_GPU_ALLOCATION_GPU_WRITE)) !=
            0u ||
        (descriptor->required_gpu_access &
         descriptor->allocation.flags) != descriptor->required_gpu_access ||
        descriptor->reserved != 0u)
        return 0;
    for (uint32_t index = 0u; index < 4u; ++index)
        if (descriptor->allocation.reserved[index] != 0u) return 0;
    return 1;
}

static int gpu_process_allocated_buffer_valid(
    const RinGpuProcessAllocatedBufferV1* buffer) {
    return buffer && buffer->struct_size == sizeof(*buffer) &&
           buffer->version == RIN_GPU_PROCESS_ALLOCATED_BUFFER_VERSION_V1 &&
           (buffer->state ==
                RIN_GPU_PROCESS_ALLOCATED_BUFFER_STATE_BOUND_V1 ||
            buffer->state == RIN_GPU_PROCESS_ALLOCATED_BUFFER_STATE_DESTROY_PENDING_V1) &&
           buffer->reserved0 == 0u &&
           gpu_process_buffer_desc_valid(&buffer->desc) &&
           buffer->desc.allocation_offset == 0u;
}

static int gpu_process_allocated_buffer_output_empty(
    const RinGpuProcessAllocatedBufferV1* buffer) {
    const unsigned char* bytes = (const unsigned char*)buffer;

    for (size_t index = 0u; index < sizeof(*buffer); ++index)
        if (bytes[index] != 0u) return 0;
    return 1;
}

RinResult rin_gpu_process_allocated_buffer_create_v1(
    const RinGpuProcessAllocatedBufferDescV1* descriptor,
    RinGpuProcessAllocatedBufferV1* buffer_out) {
    RinGpuProcessAllocatedBufferDescV1 descriptor_copy;
    RinGpuProcessBufferDescV1 process_buffer_desc;
    RinGpuProcessBufferV1 process_buffer;
    RinGpuProcessAllocatedBufferV1 allocated_buffer;
    RinGpuAllocationV1 allocation = 0u;
    RinResult result;

    if (!descriptor || !buffer_out) return RIN_ERROR_INVALID_ARGUMENT;
    descriptor_copy = *descriptor;
    if (!gpu_process_allocated_buffer_desc_valid(&descriptor_copy))
        return RIN_ERROR_INVALID_ARGUMENT;
    if (!gpu_process_allocated_buffer_output_empty(buffer_out))
        return gpu_process_allocated_buffer_valid(buffer_out)
                   ? RIN_ERROR_BUSY
                   : RIN_ERROR_INVALID_ARGUMENT;

    result = rin_gpu_memory_allocate_v1(
        descriptor_copy.device_id, descriptor_copy.device_generation,
        &descriptor_copy.allocation, &allocation);
    if (result != RIN_SUCCESS) return result;

    rin_sdk_zero_bytes(&process_buffer_desc, sizeof(process_buffer_desc));
    process_buffer_desc.struct_size = sizeof(process_buffer_desc);
    process_buffer_desc.version = RIN_GPU_PROCESS_BUFFER_DESC_VERSION_V1;
    process_buffer_desc.device_id = descriptor_copy.device_id;
    process_buffer_desc.device_generation =
        descriptor_copy.device_generation;
    process_buffer_desc.resource_cookie = descriptor_copy.resource_cookie;
    process_buffer_desc.allocation = allocation;
    process_buffer_desc.size_bytes = descriptor_copy.allocation.size_bytes;
    process_buffer_desc.required_gpu_access =
        descriptor_copy.required_gpu_access;
    result = rin_gpu_process_buffer_bind_v1(&process_buffer_desc,
                                            &process_buffer);
    if (result != RIN_SUCCESS) {
        RinResult cleanup_result = rin_gpu_memory_destroy_v1(
            descriptor_copy.device_id, descriptor_copy.device_generation,
            allocation);
        if (cleanup_result != RIN_SUCCESS) {
            rin_sdk_zero_bytes(&allocated_buffer, sizeof(allocated_buffer));
            allocated_buffer.struct_size = sizeof(allocated_buffer);
            allocated_buffer.version =
                RIN_GPU_PROCESS_ALLOCATED_BUFFER_VERSION_V1;
            allocated_buffer.state =
                RIN_GPU_PROCESS_ALLOCATED_BUFFER_STATE_DESTROY_PENDING_V1;
            allocated_buffer.desc = process_buffer_desc;
            *buffer_out = allocated_buffer;
            return cleanup_result;
        }
        return result;
    }

    rin_sdk_zero_bytes(&allocated_buffer, sizeof(allocated_buffer));
    allocated_buffer.struct_size = sizeof(allocated_buffer);
    allocated_buffer.version = RIN_GPU_PROCESS_ALLOCATED_BUFFER_VERSION_V1;
    allocated_buffer.state =
        RIN_GPU_PROCESS_ALLOCATED_BUFFER_STATE_BOUND_V1;
    allocated_buffer.desc = process_buffer.desc;
    *buffer_out = allocated_buffer;
    return RIN_SUCCESS;
}

RinResult rin_gpu_process_allocated_buffer_destroy_v1(
    RinGpuProcessAllocatedBufferV1* buffer) {
    RinGpuProcessAllocatedBufferV1 buffer_copy;
    RinResult result;

    if (!buffer) return RIN_ERROR_INVALID_ARGUMENT;
    buffer_copy = *buffer;
    if (!gpu_process_allocated_buffer_valid(&buffer_copy))
        return RIN_ERROR_INVALID_ARGUMENT;

    if (buffer_copy.state ==
        RIN_GPU_PROCESS_ALLOCATED_BUFFER_STATE_BOUND_V1) {
        RinGpuProcessBufferV1 process_buffer;
        rin_sdk_zero_bytes(&process_buffer, sizeof(process_buffer));
        process_buffer.struct_size = sizeof(process_buffer);
        process_buffer.version = RIN_GPU_PROCESS_BUFFER_VERSION_V1;
        process_buffer.state = RIN_GPU_PROCESS_BUFFER_STATE_BOUND_V1;
        process_buffer.desc = buffer_copy.desc;
        result = rin_gpu_process_buffer_unbind_v1(&process_buffer);
        if (result != RIN_SUCCESS) return result;
        buffer_copy.state =
            RIN_GPU_PROCESS_ALLOCATED_BUFFER_STATE_DESTROY_PENDING_V1;
    }

    result = rin_gpu_memory_destroy_v1(
        buffer_copy.desc.device_id, buffer_copy.desc.device_generation,
        buffer_copy.desc.allocation);
    if (result != RIN_SUCCESS) {
        *buffer = buffer_copy;
        return result;
    }
    rin_sdk_zero_bytes(buffer, sizeof(*buffer));
    return RIN_SUCCESS;
}

RinResult rin_gpu_process_buffer_sync_v1(
    const RinGpuProcessBufferV1* buffer, uint32_t action,
    uint64_t offset, uint64_t length) {
    if (!gpu_process_buffer_range_valid(buffer, offset, length))
        return RIN_ERROR_INVALID_ARGUMENT;
    return rin_gpu_memory_sync_v1(
        buffer->desc.device_id, buffer->desc.device_generation,
        buffer->desc.allocation, action,
        buffer->desc.allocation_offset + offset, length);
}

RinResult rin_gpu_process_buffer_upload_v1(
    const RinGpuProcessBufferV1* buffer, uint64_t offset,
    const void* source, uint64_t length) {
    if (!source || !gpu_process_buffer_range_valid(buffer, offset, length))
        return RIN_ERROR_INVALID_ARGUMENT;
    return rin_gpu_memory_upload_v1(
        buffer->desc.device_id, buffer->desc.device_generation,
        buffer->desc.allocation, buffer->desc.allocation_offset + offset,
        source, length);
}

RinResult rin_gpu_process_buffer_readback_v1(
    const RinGpuProcessBufferV1* buffer, uint64_t offset,
    void* destination, uint64_t length) {
    if (!destination || !gpu_process_buffer_range_valid(buffer, offset, length))
        return RIN_ERROR_INVALID_ARGUMENT;
    return rin_gpu_memory_readback_v1(
        buffer->desc.device_id, buffer->desc.device_generation,
        buffer->desc.allocation, buffer->desc.allocation_offset + offset,
        destination, length);
}

RinResult rin_gpu_process_shader_module_create_v1(
    uint64_t device_id, uint64_t device_generation,
    uint64_t shader_module_token, const void* rin_shader_ir,
    uint64_t shader_ir_size) {
    RinSdkArgsV1 request;

    if (device_id == 0u || device_generation == 0u ||
        shader_module_token == 0u || !rin_shader_ir ||
        shader_ir_size == 0u ||
        shader_ir_size > RIN_GPU_PROCESS_SHADER_MAX_BYTES_V1)
        return RIN_ERROR_INVALID_ARGUMENT;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = device_id;
    request.value[1] = device_generation;
    request.value[2] = shader_module_token;
    request.value[3] = (uint64_t)(uintptr_t)rin_shader_ir;
    request.value[4] = shader_ir_size;
    return rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_PROCESS_SHADER_MODULE_CREATE,
        &request, sizeof(request), (void*)0, 0u);
}

RinResult rin_gpu_process_object_destroy_v1(
    uint64_t device_id, uint64_t device_generation, uint64_t object_token) {
    if (device_id == 0u || device_generation == 0u || object_token == 0u)
        return RIN_ERROR_INVALID_ARGUMENT;
    SIMPLE_CALL(RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_PROCESS_OBJECT_DESTROY,
                (RinResult*)0, device_id, device_generation, object_token,
                0, 0, 0);
}

RinResult rin_gpu_process_compute_pipeline_create_v1(
    uint64_t device_id, uint64_t device_generation,
    uint64_t pipeline_token, uint64_t shader_module_token) {
    if (device_id == 0u || device_generation == 0u || pipeline_token == 0u ||
        shader_module_token == 0u || pipeline_token == shader_module_token)
        return RIN_ERROR_INVALID_ARGUMENT;
    SIMPLE_CALL(RIN_SDK_LIBRARY_GPU,
                RIN_GPU_SDK_PROCESS_COMPUTE_PIPELINE_CREATE,
                (RinResult*)0, device_id, device_generation, pipeline_token,
                shader_module_token, 0, 0);
}

RinResult rin_gpu_process_graphics_pipeline_create_v1(
    uint64_t device_id, uint64_t device_generation,
    uint64_t pipeline_token, uint64_t vertex_shader_token,
    uint64_t fragment_shader_token,
    const RinGpuProcessGraphicsPipelineDescV1* descriptor) {
    RinGpuProcessGraphicsPipelineDescV1 descriptor_copy;
    RinSdkArgsV1 request;

    if (device_id == 0u || device_generation == 0u || pipeline_token == 0u ||
        vertex_shader_token == 0u || fragment_shader_token == 0u ||
        pipeline_token == vertex_shader_token ||
        pipeline_token == fragment_shader_token ||
        vertex_shader_token == fragment_shader_token || !descriptor ||
        descriptor->struct_size != sizeof(*descriptor) ||
        descriptor->version !=
            RIN_GPU_PROCESS_GRAPHICS_PIPELINE_DESC_VERSION_V1 ||
        descriptor->color_format == 0u ||
        descriptor->primitive_topology == 0u)
        return RIN_ERROR_INVALID_ARGUMENT;
    descriptor_copy = *descriptor;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = device_id;
    request.value[1] = device_generation;
    request.value[2] = pipeline_token;
    request.value[3] = vertex_shader_token;
    request.value[4] = fragment_shader_token;
    request.value[5] = (uint64_t)(uintptr_t)&descriptor_copy;
    return rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_GPU,
        RIN_GPU_SDK_PROCESS_GRAPHICS_PIPELINE_CREATE, &request,
        sizeof(request), (void*)0, 0u);
}

RinResult rin_gpu_process_graphics_pipeline_create_v2(
    uint64_t device_id, uint64_t device_generation,
    uint64_t pipeline_token, uint64_t vertex_shader_token,
    uint64_t fragment_shader_token,
    const RinGpuProcessGraphicsPipelineDescV2* descriptor) {
    RinGpuProcessGraphicsPipelineDescV2 descriptor_copy;
    RinSdkArgsV1 request;

    if (device_id == 0u || device_generation == 0u || pipeline_token == 0u ||
        vertex_shader_token == 0u || fragment_shader_token == 0u ||
        pipeline_token == vertex_shader_token ||
        pipeline_token == fragment_shader_token ||
        vertex_shader_token == fragment_shader_token || !descriptor ||
        !rin_gpu_process_graphics_pipeline_desc_v2_valid(descriptor))
        return RIN_ERROR_INVALID_ARGUMENT;
    descriptor_copy = *descriptor;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = device_id;
    request.value[1] = device_generation;
    request.value[2] = pipeline_token;
    request.value[3] = vertex_shader_token;
    request.value[4] = fragment_shader_token;
    request.value[5] = (uint64_t)(uintptr_t)&descriptor_copy;
    return rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_GPU,
        RIN_GPU_SDK_PROCESS_GRAPHICS_PIPELINE_CREATE_V2, &request,
        sizeof(request), (void*)0, 0u);
}

static int gpu_process_sampler_filter_valid(uint32_t filter) {
    return filter == RIN_GPU_PROCESS_SAMPLER_FILTER_NEAREST_V1 ||
           filter == RIN_GPU_PROCESS_SAMPLER_FILTER_LINEAR_V1;
}

static int gpu_process_sampler_address_valid(uint32_t address) {
    return address == RIN_GPU_PROCESS_SAMPLER_ADDRESS_CLAMP_TO_EDGE_V1 ||
           address == RIN_GPU_PROCESS_SAMPLER_ADDRESS_REPEAT_V1 ||
           address == RIN_GPU_PROCESS_SAMPLER_ADDRESS_MIRRORED_REPEAT_V1;
}

static int gpu_process_sampler_desc_valid(
    const RinGpuProcessSamplerDescV1* descriptor) {
    return descriptor && descriptor->struct_size == sizeof(*descriptor) &&
           descriptor->version == RIN_GPU_PROCESS_SAMPLER_DESC_VERSION_V1 &&
           gpu_process_sampler_filter_valid(descriptor->min_filter) &&
           gpu_process_sampler_filter_valid(descriptor->mag_filter) &&
           (descriptor->mip_filter ==
                RIN_GPU_PROCESS_SAMPLER_MIP_FILTER_NONE_V1 ||
            gpu_process_sampler_filter_valid(descriptor->mip_filter)) &&
           gpu_process_sampler_address_valid(descriptor->address_u) &&
           gpu_process_sampler_address_valid(descriptor->address_v) &&
           gpu_process_sampler_address_valid(descriptor->address_w) &&
           descriptor->mip_lod_bias == descriptor->mip_lod_bias &&
           descriptor->mip_lod_bias >= -16.0f &&
           descriptor->mip_lod_bias <= 16.0f &&
           descriptor->min_lod == descriptor->min_lod &&
           descriptor->min_lod >= 0.0f && descriptor->min_lod <= 32.0f &&
           descriptor->max_lod == descriptor->max_lod &&
           descriptor->max_lod >= descriptor->min_lod &&
           descriptor->max_lod <= 32.0f &&
           descriptor->max_anisotropy != 0u &&
           descriptor->max_anisotropy <=
               RIN_GPU_PROCESS_SAMPLER_MAX_ANISOTROPY_V1 &&
           descriptor->flags == 0u &&
           (descriptor->compare_op ==
                RIN_GPU_PROCESS_SAMPLER_COMPARE_NONE_V1 ||
            (descriptor->compare_op >=
                 RIN_GPU_PROCESS_SAMPLER_COMPARE_LESS_V1 &&
             descriptor->compare_op <=
                 RIN_GPU_PROCESS_SAMPLER_COMPARE_GREATER_EQUAL_V1));
}

RinResult rin_gpu_process_sampler_create_v1(
    uint64_t device_id, uint64_t device_generation, uint64_t sampler_token,
    const RinGpuProcessSamplerDescV1* descriptor) {
    RinGpuProcessSamplerDescV1 descriptor_copy;
    RinSdkArgsV1 request;

    if (device_id == 0u || device_generation == 0u || sampler_token == 0u ||
        !gpu_process_sampler_desc_valid(descriptor))
        return RIN_ERROR_INVALID_ARGUMENT;
    descriptor_copy = *descriptor;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = device_id;
    request.value[1] = device_generation;
    request.value[2] = sampler_token;
    request.value[3] = (uint64_t)(uintptr_t)&descriptor_copy;
    return rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_PROCESS_SAMPLER_CREATE, &request,
        sizeof(request), (void*)0, 0u);
}

RinResult rin_gpu_process_image_create_v1(
    uint64_t device_id, uint64_t device_generation,
    const RinGpuProcessImageViewV1* image)
{
    RinGpuProcessImageViewV1 image_copy;
    RinSdkArgsV1 request;

    if (device_id == 0u || device_generation == 0u || !image ||
        !rin_gpu_process_image_create_desc_v1_valid(image))
        return RIN_ERROR_INVALID_ARGUMENT;
    image_copy = *image;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = device_id;
    request.value[1] = device_generation;
    request.value[2] = (uint64_t)(uintptr_t)&image_copy;
    return rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_PROCESS_IMAGE_CREATE, &request,
        sizeof(request), (void*)0, 0u);
}

RinResult rin_gpu_process_compute_bind_group_create_v1(
    uint64_t device_id, uint64_t device_generation,
    uint64_t bind_group_token, uint64_t pipeline_token,
    const RinGpuProcessComputeBindingV1* bindings, uint32_t binding_count) {
    RinGpuProcessComputeBindingV1 binding_copy[
        RIN_GPU_PROCESS_MAX_COMPUTE_BINDINGS_V1];
    RinSdkArgsV1 request;

    if (device_id == 0u || device_generation == 0u ||
        bind_group_token == 0u || pipeline_token == 0u ||
        bind_group_token == pipeline_token ||
        ((binding_count == 0u) != (bindings == NULL)) ||
        binding_count > RIN_GPU_PROCESS_MAX_COMPUTE_BINDINGS_V1)
        return RIN_ERROR_INVALID_ARGUMENT;
    for (uint32_t index = 0u; index < binding_count; ++index) {
        const RinGpuProcessComputeBindingV1* binding = &bindings[index];
        if (binding->struct_size != sizeof(*binding) ||
            binding->version != RIN_GPU_PROCESS_COMPUTE_BINDING_VERSION_V1 ||
            binding->resource_cookie == 0u || binding->size_bytes == 0u ||
            binding->offset > UINT64_MAX - binding->size_bytes ||
            binding->access == 0u ||
            (binding->access & ~(RIN_GPU_ALLOCATION_GPU_READ |
                                 RIN_GPU_ALLOCATION_GPU_WRITE)) != 0u ||
            binding->reserved != 0u)
            return RIN_ERROR_INVALID_ARGUMENT;
        binding_copy[index] = *binding;
    }
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = device_id;
    request.value[1] = device_generation;
    request.value[2] = bind_group_token;
    request.value[3] = pipeline_token;
    request.value[4] = binding_count == 0u
        ? 0u : (uint64_t)(uintptr_t)binding_copy;
    request.value[5] = binding_count;
    return rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_GPU,
        RIN_GPU_SDK_PROCESS_COMPUTE_BIND_GROUP_CREATE, &request,
        sizeof(request), (void*)0, 0u);
}

RinResult rin_gpu_process_graphics_bind_group_create_v1(
    uint64_t device_id, uint64_t device_generation,
    uint64_t bind_group_token, uint64_t pipeline_token,
    const RinGpuProcessGraphicsBindingV1* bindings, uint32_t binding_count) {
    RinGpuProcessGraphicsBindingV1 binding_copy[
        RIN_GPU_PROCESS_MAX_GRAPHICS_BINDINGS_V1];
    uint64_t seen_bindings = 0u;
    RinSdkArgsV1 request;

    if (device_id == 0u || device_generation == 0u ||
        bind_group_token == 0u || pipeline_token == 0u ||
        bind_group_token == pipeline_token || !bindings ||
        binding_count == 0u ||
        binding_count > RIN_GPU_PROCESS_MAX_GRAPHICS_BINDINGS_V1)
        return RIN_ERROR_INVALID_ARGUMENT;
    for (uint32_t index = 0u; index < binding_count; ++index) {
        const RinGpuProcessGraphicsBindingV1* binding = &bindings[index];
        if (binding->struct_size != sizeof(*binding) ||
            binding->version != RIN_GPU_PROCESS_GRAPHICS_BINDING_VERSION_V1 ||
            binding->resource_cookie == 0u || binding->size_bytes == 0u ||
            binding->offset > UINT64_MAX - binding->size_bytes ||
            binding->binding >= binding_count ||
            (seen_bindings & (UINT64_C(1) << binding->binding)) != 0u ||
            (binding->offset & 3u) != 0u ||
            (binding->size_bytes & 3u) != 0u || binding->access == 0u ||
            (binding->access & ~(RIN_GPU_ALLOCATION_GPU_READ |
                                 RIN_GPU_ALLOCATION_GPU_WRITE)) != 0u)
            return RIN_ERROR_INVALID_ARGUMENT;
        seen_bindings |= UINT64_C(1) << binding->binding;
        binding_copy[index] = *binding;
    }
    if (seen_bindings !=
        (binding_count == 64u
             ? UINT64_MAX
             : ((UINT64_C(1) << binding_count) - UINT64_C(1))))
        return RIN_ERROR_INVALID_ARGUMENT;

    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = device_id;
    request.value[1] = device_generation;
    request.value[2] = bind_group_token;
    request.value[3] = pipeline_token;
    request.value[4] = (uint64_t)(uintptr_t)binding_copy;
    request.value[5] = binding_count;
    return rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_GPU,
        RIN_GPU_SDK_PROCESS_GRAPHICS_BIND_GROUP_CREATE, &request,
        sizeof(request), (void*)0, 0u);
}

RinResult rin_gpu_process_graphics_bind_group_create_v2(
    uint64_t device_id, uint64_t device_generation,
    uint64_t bind_group_token, uint64_t pipeline_token,
    const RinGpuProcessGraphicsBindingV2* bindings, uint32_t binding_count) {
    RinGpuProcessGraphicsBindingV2 binding_copy[
        RIN_GPU_PROCESS_GRAPHICS_BINDING_V2_MAX_COUNT];
    uint64_t seen_bindings = 0u;
    RinSdkArgsV1 request;

    if (device_id == 0u || device_generation == 0u ||
        bind_group_token == 0u || pipeline_token == 0u ||
        bind_group_token == pipeline_token || !bindings ||
        binding_count == 0u ||
        binding_count > RIN_GPU_PROCESS_GRAPHICS_BINDING_V2_MAX_COUNT)
        return RIN_ERROR_INVALID_ARGUMENT;
    for (uint32_t index = 0u; index < binding_count; ++index) {
        const RinGpuProcessGraphicsBindingV2* binding = &bindings[index];
        if (!rin_gpu_process_graphics_binding_v2_valid(binding) ||
            binding->binding >= binding_count ||
            (seen_bindings & (UINT64_C(1) << binding->binding)) != 0u)
            return RIN_ERROR_INVALID_ARGUMENT;
        seen_bindings |= UINT64_C(1) << binding->binding;
        binding_copy[index] = *binding;
    }
    if (seen_bindings !=
        (binding_count == 64u
             ? UINT64_MAX
             : ((UINT64_C(1) << binding_count) - UINT64_C(1))))
        return RIN_ERROR_INVALID_ARGUMENT;

    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = device_id;
    request.value[1] = device_generation;
    request.value[2] = bind_group_token;
    request.value[3] = pipeline_token;
    request.value[4] = (uint64_t)(uintptr_t)binding_copy;
    request.value[5] = binding_count;
    return rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_GPU,
        RIN_GPU_SDK_PROCESS_GRAPHICS_BIND_GROUP_CREATE_V2, &request,
        sizeof(request), (void*)0, 0u);
}

static int gpu_process_query_type_valid(uint32_t query_type) {
    return query_type == RIN_GPU_PROCESS_QUERY_TIMESTAMP_V1 ||
           query_type == RIN_GPU_PROCESS_QUERY_OCCLUSION_V1 ||
           query_type == RIN_GPU_PROCESS_QUERY_PIPELINE_STATISTICS_V1;
}

RinResult rin_gpu_process_query_create_v1(
    uint64_t device_id, uint64_t device_generation, uint64_t query_token,
    const RinGpuProcessQueryDescV1* descriptor) {
    RinGpuProcessQueryDescV1 descriptor_copy;
    RinSdkArgsV1 request;

    if (device_id == 0u || device_generation == 0u || query_token == 0u ||
        !descriptor || descriptor->struct_size != sizeof(*descriptor) ||
        descriptor->version != RIN_GPU_PROCESS_QUERY_DESC_VERSION_V1 ||
        !gpu_process_query_type_valid(descriptor->query_type) ||
        descriptor->flags != 0u || descriptor->reserved0 != 0u ||
        descriptor->reserved1 != 0u)
        return RIN_ERROR_INVALID_ARGUMENT;
    descriptor_copy = *descriptor;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = device_id;
    request.value[1] = device_generation;
    request.value[2] = query_token;
    request.value[3] = (uint64_t)(uintptr_t)&descriptor_copy;
    return rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_PROCESS_QUERY_CREATE, &request,
        sizeof(request), (void*)0, 0u);
}

RinResult rin_gpu_process_query_result_v1(
    uint64_t device_id, uint64_t device_generation, uint64_t query_token,
    RinGpuProcessQueryResultV1* result_out) {
    RinSdkArgsV1 request;
    RinResult result;

    if (!result_out || device_id == 0u || device_generation == 0u ||
        query_token == 0u)
        return RIN_ERROR_INVALID_ARGUMENT;
    if (!versioned(result_out, sizeof(*result_out)))
        return RIN_ERROR_ABI_MISMATCH;
    rin_sdk_zero_bytes(result_out, sizeof(*result_out));
    result_out->struct_size = sizeof(*result_out);
    result_out->version = RIN_GPU_PROCESS_QUERY_RESULT_VERSION_V1;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = device_id;
    request.value[1] = device_generation;
    request.value[2] = query_token;
    result = rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_PROCESS_QUERY_RESULT, &request,
        sizeof(request), result_out, sizeof(*result_out));
    if (result == RIN_SUCCESS &&
        (result_out->struct_size != sizeof(*result_out) ||
         result_out->version != RIN_GPU_PROCESS_QUERY_RESULT_VERSION_V1 ||
         !gpu_process_query_type_valid(result_out->query_type) ||
         result_out->available > 1u || result_out->reserved0 != 0u ||
         result_out->reserved1 != 0u))
        result = RIN_ERROR_ABI_MISMATCH;
    if (result != RIN_SUCCESS) {
        rin_sdk_zero_bytes(result_out, sizeof(*result_out));
        result_out->struct_size = sizeof(*result_out);
        result_out->version = RIN_GPU_PROCESS_QUERY_RESULT_VERSION_V1;
    }
    return result;
}

RinResult rin_gpu_process_timestamp_period_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuProcessTimestampPeriodV1* period_out) {
    RinSdkArgsV1 request;
    RinResult result;

    if (!period_out || device_id == 0u || device_generation == 0u)
        return RIN_ERROR_INVALID_ARGUMENT;
    if (!versioned(period_out, sizeof(*period_out)))
        return RIN_ERROR_ABI_MISMATCH;
    rin_sdk_zero_bytes(period_out, sizeof(*period_out));
    period_out->struct_size = sizeof(*period_out);
    period_out->version = RIN_GPU_PROCESS_TIMESTAMP_PERIOD_VERSION_V1;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = device_id;
    request.value[1] = device_generation;
    result = rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_PROCESS_TIMESTAMP_PERIOD, &request,
        sizeof(request), period_out, sizeof(*period_out));
    if (result == RIN_SUCCESS &&
        (period_out->struct_size != sizeof(*period_out) ||
         period_out->version !=
             RIN_GPU_PROCESS_TIMESTAMP_PERIOD_VERSION_V1 ||
         period_out->device_generation != device_generation ||
         period_out->clock_domain_cookie == 0u ||
         (period_out->fractional_numerator == 0u
              ? period_out->fractional_denominator != 0u ||
                    period_out->whole_nanoseconds == 0u
              : period_out->fractional_denominator == 0u ||
                    period_out->fractional_numerator >=
                        period_out->fractional_denominator) ||
         period_out->reserved[0] != 0u || period_out->reserved[1] != 0u))
        result = RIN_ERROR_ABI_MISMATCH;
    if (result != RIN_SUCCESS) {
        rin_sdk_zero_bytes(period_out, sizeof(*period_out));
        period_out->struct_size = sizeof(*period_out);
        period_out->version = RIN_GPU_PROCESS_TIMESTAMP_PERIOD_VERSION_V1;
    }
    return result;
}

RinResult rin_gpu_process_product_get_status_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuProcessProductStatusV1* status_out)
{
    RinSdkArgsV1 request;
    RinResult result;

    if (!status_out || device_id == 0u || device_generation == 0u)
        return RIN_ERROR_INVALID_ARGUMENT;
    if (!versioned(status_out, sizeof(*status_out)))
        return RIN_ERROR_ABI_MISMATCH;
    rin_sdk_zero_bytes(status_out, sizeof(*status_out));
    status_out->struct_size = sizeof(*status_out);
    status_out->version = RIN_GPU_PROCESS_PRODUCT_STATUS_VERSION_V1;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = device_id;
    request.value[1] = device_generation;
    result = rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_PROCESS_PRODUCT_GET_STATUS,
        &request, sizeof(request), status_out, sizeof(*status_out));
    if (result == RIN_SUCCESS &&
        (status_out->struct_size != sizeof(*status_out) ||
         status_out->version != RIN_GPU_PROCESS_PRODUCT_STATUS_VERSION_V1 ||
         status_out->flags == 0u ||
         (status_out->flags & ~RIN_GPU_PROCESS_PRODUCT_STATUS_KNOWN_V1) != 0u ||
         status_out->queue_count == 0u ||
         status_out->queue_count > RIN_GPU_PROCESS_MAX_QUEUES_V1 ||
         status_out->iommu_domain_cookie == 0u ||
         status_out->iommu_map_generation == 0u ||
         status_out->device_epoch == 0u ||
         status_out->max_resources_per_submission !=
             RIN_GPU_PROCESS_MAX_RESOURCES_V1))
        result = RIN_ERROR_ABI_MISMATCH;
    if (result != RIN_SUCCESS) {
        rin_sdk_zero_bytes(status_out, sizeof(*status_out));
        status_out->struct_size = sizeof(*status_out);
        status_out->version = RIN_GPU_PROCESS_PRODUCT_STATUS_VERSION_V1;
    }
    return result;
}

RinResult rin_gpu_process_product_poll_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuProcessProductReportV1* report_out)
{
    RinSdkArgsV1 request;
    RinResult result;

    if (!report_out || device_id == 0u || device_generation == 0u)
        return RIN_ERROR_INVALID_ARGUMENT;
    if (!versioned(report_out, sizeof(*report_out)))
        return RIN_ERROR_ABI_MISMATCH;
    rin_sdk_zero_bytes(report_out, sizeof(*report_out));
    report_out->struct_size = sizeof(*report_out);
    report_out->version = RIN_GPU_PROCESS_PRODUCT_REPORT_VERSION_V1;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.value[0] = device_id;
    request.value[1] = device_generation;
    result = rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_PROCESS_PRODUCT_POLL, &request,
        sizeof(request), report_out, sizeof(*report_out));
    if (result == RIN_SUCCESS &&
        (report_out->struct_size != sizeof(*report_out) ||
         report_out->version != RIN_GPU_PROCESS_PRODUCT_REPORT_VERSION_V1 ||
         (report_out->flags &
          ~RIN_GPU_PROCESS_PRODUCT_REPORT_KNOWN_V1) != 0u ||
         report_out->device_epoch == 0u ||
         report_out->iommu_domain_cookie == 0u ||
         report_out->iommu_map_generation == 0u))
        result = RIN_ERROR_ABI_MISMATCH;
    if (result != RIN_SUCCESS) {
        rin_sdk_zero_bytes(report_out, sizeof(*report_out));
        report_out->struct_size = sizeof(*report_out);
        report_out->version = RIN_GPU_PROCESS_PRODUCT_REPORT_VERSION_V1;
    }
    return result;
}

static int gpu_direct_display_bytes_zero(const void* value, uint64_t size) {
    const uint8_t* bytes = (const uint8_t*)value;
    uint64_t index;
    if (!value) return 0;
    for (index = 0u; index < size; ++index)
        if (bytes[index] != 0u) return 0;
    return 1;
}

static int gpu_direct_display_range_valid(const void* pointer, size_t size) {
    const uintptr_t start = (uintptr_t)pointer;
    return pointer && size != 0u && size - 1u <= UINTPTR_MAX - start;
}

static int gpu_direct_display_ranges_overlap(
    const void* first, size_t first_size, const void* second,
    size_t second_size) {
    const uintptr_t first_start = (uintptr_t)first;
    const uintptr_t second_start = (uintptr_t)second;
    if (!gpu_direct_display_range_valid(first, first_size) ||
        !gpu_direct_display_range_valid(second, second_size))
        return 1;
    return first_start <= second_start
               ? second_start - first_start < first_size
               : first_start - second_start < second_size;
}

static int gpu_direct_display_request_valid(
    const RinGpuProcessDirectDisplayRequestV1* request) {
    uint32_t operation;
    uint32_t maximum_capacity = 0u;

    if (!request) return 0;
    operation = request->operation;
    if (request->struct_size != sizeof(*request) ||
        request->version !=
            RIN_GPU_PROCESS_DIRECT_DISPLAY_REQUEST_VERSION_V1 ||
        operation < RIN_GPU_PROCESS_DIRECT_DISPLAY_QUERY_DISPLAYS_V1 ||
        operation > RIN_GPU_PROCESS_DIRECT_DISPLAY_CANCEL_V1 ||
        request->device_id == 0u || request->device_generation == 0u ||
        !gpu_direct_display_bytes_zero(request->reserved,
                                       sizeof(request->reserved)))
        return 0;

    switch (operation) {
    case RIN_GPU_PROCESS_DIRECT_DISPLAY_QUERY_DISPLAYS_V1:
        maximum_capacity = RIN_GPU_PROCESS_DIRECT_DISPLAY_MAX_DISPLAYS_V1;
        if (request->display_cookie != 0u ||
            request->output_generation != 0u || request->mode_cookie != 0u ||
            request->present_token != 0u || request->plane_index != 0u ||
            request->queue_family_index != 0u || request->queue_flags != 0u ||
            request->queue_count != 0u ||
            !gpu_direct_display_bytes_zero(&request->present,
                                            sizeof(request->present)))
            return 0;
        break;
    case RIN_GPU_PROCESS_DIRECT_DISPLAY_QUERY_MODES_V1:
        maximum_capacity = RIN_GPU_PROCESS_DIRECT_DISPLAY_MAX_MODES_V1;
        if (request->display_cookie == 0u ||
            request->output_generation == 0u || request->mode_cookie != 0u ||
            request->present_token != 0u || request->plane_index != 0u ||
            request->queue_family_index != 0u || request->queue_flags != 0u ||
            request->queue_count != 0u ||
            !gpu_direct_display_bytes_zero(&request->present,
                                            sizeof(request->present)))
            return 0;
        break;
    case RIN_GPU_PROCESS_DIRECT_DISPLAY_QUERY_PLANES_V1:
        maximum_capacity = RIN_GPU_PROCESS_DIRECT_DISPLAY_MAX_PLANES_V1;
        if (request->display_cookie != 0u ||
            request->output_generation != 0u || request->mode_cookie != 0u ||
            request->present_token != 0u || request->plane_index != 0u ||
            request->queue_family_index != 0u || request->queue_flags != 0u ||
            request->queue_count != 0u ||
            !gpu_direct_display_bytes_zero(&request->present,
                                            sizeof(request->present)))
            return 0;
        break;
    case RIN_GPU_PROCESS_DIRECT_DISPLAY_QUERY_PLANE_DISPLAYS_V1:
        maximum_capacity = RIN_GPU_PROCESS_DIRECT_DISPLAY_MAX_DISPLAYS_V1;
        if (request->display_cookie != 0u ||
            request->output_generation != 0u || request->mode_cookie != 0u ||
            request->present_token != 0u ||
            request->queue_family_index != 0u || request->queue_flags != 0u ||
            request->queue_count != 0u ||
            !gpu_direct_display_bytes_zero(&request->present,
                                            sizeof(request->present)))
            return 0;
        break;
    case RIN_GPU_PROCESS_DIRECT_DISPLAY_QUERY_PLANE_CAPABILITIES_V1:
    case RIN_GPU_PROCESS_DIRECT_DISPLAY_QUERY_SURFACE_PROPERTIES_V1:
        if (request->capacity != 0u || request->display_cookie == 0u ||
            request->output_generation == 0u || request->mode_cookie == 0u ||
            request->present_token != 0u ||
            request->queue_family_index != 0u || request->queue_flags != 0u ||
            request->queue_count != 0u ||
            !gpu_direct_display_bytes_zero(&request->present,
                                            sizeof(request->present)))
            return 0;
        break;
    case RIN_GPU_PROCESS_DIRECT_DISPLAY_QUERY_QUEUE_SUPPORT_V1:
        if (request->capacity != 0u || request->display_cookie == 0u ||
            request->output_generation == 0u || request->mode_cookie == 0u ||
            request->present_token != 0u || request->queue_count == 0u ||
            !gpu_direct_display_bytes_zero(&request->present,
                                            sizeof(request->present)))
            return 0;
        break;
    case RIN_GPU_PROCESS_DIRECT_DISPLAY_PRESENT_V1: {
        const RinGpuProcessDirectDisplayPresentRequestV1* present =
            &request->present;
        if (request->capacity != 0u || request->display_cookie != 0u ||
            request->output_generation != 0u || request->mode_cookie != 0u ||
            request->present_token != 0u || request->plane_index != 0u ||
            request->queue_family_index != 0u || request->queue_flags != 0u ||
            request->queue_count != 0u ||
            present->struct_size != sizeof(*present) ||
            !rin_gpu_process_direct_display_present_version_valid_v1(
                present->version) ||
            present->display_cookie == 0u || present->mode_cookie == 0u ||
            present->allocation == 0u || present->allocation_size == 0u ||
            present->allocation_offset >
                UINT64_MAX - present->allocation_size ||
            present->output_generation == 0u ||
            present->device_generation != request->device_generation ||
            present->frame_id == 0u || present->width == 0u ||
            present->height == 0u || present->pixel_format == 0u ||
            present->present_schedule !=
                RIN_GPU_PROCESS_DIRECT_DISPLAY_PRESENT_FIFO_V1 ||
            present->flags != 0u ||
            !gpu_direct_display_bytes_zero(present->reserved,
                                            sizeof(present->reserved)) ||
            !rin_gpu_process_direct_display_present_image_layout_valid_v1(
                present))
            return 0;
        break;
    }
    case RIN_GPU_PROCESS_DIRECT_DISPLAY_POLL_V1:
    case RIN_GPU_PROCESS_DIRECT_DISPLAY_CANCEL_V1:
        if (request->capacity != 0u || request->display_cookie == 0u ||
            request->present_token == 0u ||
            request->output_generation != 0u || request->mode_cookie != 0u ||
            request->plane_index != 0u ||
            request->queue_family_index != 0u || request->queue_flags != 0u ||
            request->queue_count != 0u ||
            !gpu_direct_display_bytes_zero(&request->present,
                                            sizeof(request->present)))
            return 0;
        break;
    default:
        return 0;
    }
    return request->capacity <= maximum_capacity;
}

static int gpu_direct_display_response_valid(
    const RinGpuProcessDirectDisplayRequestV1* request,
    const RinGpuProcessDirectDisplayResponseV1* response) {
    if (!response || response->struct_size != sizeof(*response) ||
        response->version !=
            RIN_GPU_PROCESS_DIRECT_DISPLAY_RESPONSE_VERSION_V1 ||
        response->operation != request->operation ||
        response->result > RIN_GPU_PROCESS_DIRECT_DISPLAY_LIMIT_V1 ||
        response->reserved0 != 0u ||
        !gpu_direct_display_bytes_zero(response->reserved,
                                       sizeof(response->reserved)))
        return 0;
    switch (request->operation) {
    case RIN_GPU_PROCESS_DIRECT_DISPLAY_QUERY_DISPLAYS_V1:
    case RIN_GPU_PROCESS_DIRECT_DISPLAY_QUERY_MODES_V1:
    case RIN_GPU_PROCESS_DIRECT_DISPLAY_QUERY_PLANES_V1:
    case RIN_GPU_PROCESS_DIRECT_DISPLAY_QUERY_PLANE_DISPLAYS_V1:
        if (response->count > request->capacity) return 0;
        break;
    case RIN_GPU_PROCESS_DIRECT_DISPLAY_PRESENT_V1:
        if (response->count != 0u ||
            (response->result == RIN_GPU_PROCESS_DIRECT_DISPLAY_OK_V1
                 ? response->data.present_token == 0u
                 : response->data.present_token != 0u))
            return 0;
        break;
    case RIN_GPU_PROCESS_DIRECT_DISPLAY_POLL_V1:
        if (response->count != 0u ||
            (response->result == RIN_GPU_PROCESS_DIRECT_DISPLAY_OK_V1 &&
             (response->data.present_status.struct_size !=
                  sizeof(response->data.present_status) ||
              response->data.present_status.version !=
                  RIN_GPU_PROCESS_DIRECT_DISPLAY_RESPONSE_VERSION_V1 ||
              (response->data.present_status.state !=
                   RIN_GPU_PROCESS_DIRECT_DISPLAY_PRESENT_PENDING_V1 &&
               response->data.present_status.state !=
                   RIN_GPU_PROCESS_DIRECT_DISPLAY_PRESENT_COMPLETE_V1) ||
              response->data.present_status.output_generation == 0u ||
              response->data.present_status.device_generation !=
                  request->device_generation ||
              response->data.present_status.reserved0 != 0u ||
              !gpu_direct_display_bytes_zero(
                  response->data.present_status.reserved,
                  sizeof(response->data.present_status.reserved)))))
            return 0;
        break;
    case RIN_GPU_PROCESS_DIRECT_DISPLAY_QUERY_QUEUE_SUPPORT_V1:
        if (response->count != 0u || response->data.queue_supported > 1u)
            return 0;
        break;
    case RIN_GPU_PROCESS_DIRECT_DISPLAY_QUERY_PLANE_CAPABILITIES_V1:
    case RIN_GPU_PROCESS_DIRECT_DISPLAY_QUERY_SURFACE_PROPERTIES_V1:
    case RIN_GPU_PROCESS_DIRECT_DISPLAY_CANCEL_V1:
        if (response->count != 0u) return 0;
        break;
    default:
        return 0;
    }
    return 1;
}

RinResult rin_gpu_process_direct_display_dispatch_v1(
    const RinGpuProcessDirectDisplayRequestV1* request,
    RinGpuProcessDirectDisplayResponseV1* response_out) {
    RinSdkArgsV1 syscall_request;
    RinResult result;

    if (!request || !response_out ||
        !gpu_direct_display_range_valid(request, sizeof(*request)) ||
        !gpu_direct_display_range_valid(response_out, sizeof(*response_out)) ||
        gpu_direct_display_ranges_overlap(request, sizeof(*request),
                                          response_out,
                                          sizeof(*response_out)) ||
        !gpu_direct_display_request_valid(request))
        return RIN_ERROR_INVALID_ARGUMENT;

    rin_sdk_zero_bytes(response_out, sizeof(*response_out));
    response_out->struct_size = sizeof(*response_out);
    response_out->version =
        RIN_GPU_PROCESS_DIRECT_DISPLAY_RESPONSE_VERSION_V1;
    response_out->operation = request->operation;
    rin_sdk_zero_bytes(&syscall_request, sizeof(syscall_request));
    syscall_request.struct_size = sizeof(syscall_request);
    syscall_request.version = RIN_SDK_STRUCT_VERSION_1;
    syscall_request.value[0] = (uint64_t)(uintptr_t)request;
    syscall_request.value[1] = sizeof(*request);
    result = rin_sdk_invoke_v1(
        RIN_SDK_LIBRARY_GPU, RIN_GPU_SDK_PROCESS_DIRECT_DISPLAY_DISPATCH,
        &syscall_request, sizeof(syscall_request), response_out,
        sizeof(*response_out));
    if (result == RIN_SUCCESS &&
        !gpu_direct_display_response_valid(request, response_out))
        result = RIN_ERROR_ABI_MISMATCH;
    if (result != RIN_SUCCESS) {
        rin_sdk_zero_bytes(response_out, sizeof(*response_out));
        response_out->struct_size = sizeof(*response_out);
        response_out->version =
            RIN_GPU_PROCESS_DIRECT_DISPLAY_RESPONSE_VERSION_V1;
        response_out->operation = request->operation;
    }
    return result;
}
#endif

#undef SIMPLE_CALL
#undef VERSIONED_OUTPUT_SIMPLE_CALL
#undef SCALAR_OUTPUT_SIMPLE_CALL
#undef HANDLE_OUTPUT_SIMPLE_CALL
