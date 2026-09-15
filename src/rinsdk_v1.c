#include <rin/sdk.h>

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
    defined(RINSDK_BUILD_PKG) || defined(RINSDK_BUILD_DEVICE)
static void rin_sdk_zero_bytes(void* destination, uint64_t size) {
    uint8_t* out = (uint8_t*)destination;
    while (size-- > 0u) *out++ = 0u;
}
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_CORE)
static RinSdkBackendV1 g_backend;
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
    IPC_SERVICE_ACCEPT,
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
    NET_SOCKET_CREATE = 1, NET_SOCKET_CONNECT, NET_SOCKET_SEND,
    NET_SOCKET_RECEIVE, NET_DNS_RESOLVE, NET_TLS_CONNECT, NET_HTTP_EXECUTE,
    NET_TLS_SEND, NET_TLS_RECEIVE, NET_TLS_SHUTDOWN,
    NET_SOCKET_BIND, NET_SOCKET_LISTEN, NET_SOCKET_ACCEPT,
    NET_SOCKET_SHUTDOWN,
    GUI_WINDOW_CREATE = 1, GUI_WINDOW_NEXT_EVENT, GUI_SURFACE_CREATE,
    GUI_WINDOW_PRESENT, GUI_CLIPBOARD_SET, GUI_CLIPBOARD_GET,
    GUI_DIALOG_SHOW, GUI_NOTIFICATION_SHOW,
    MEDIA_OPEN = 1, MEDIA_IMAGE_DECODE, MEDIA_AUDIO_DECODE, MEDIA_VIDEO_DECODE,
    CONFIG_OPEN = 1, CONFIG_GET, CONFIG_SET, CONFIG_COMMIT, CONFIG_REMOVE,
    PKG_OPEN = 1, PKG_QUERY, PKG_INSTALL, PKG_REMOVE, PKG_ENUMERATE,
    DEVICE_ENUMERATE = 1, DEVICE_OPEN, DEVICE_CONTROL, DEVICE_QUERY
};

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_BASE) || \
    defined(RINSDK_BUILD_IPC) || defined(RINSDK_BUILD_FS) || \
    defined(RINSDK_BUILD_NET) || defined(RINSDK_BUILD_GUI) || \
    defined(RINSDK_BUILD_MEDIA) || defined(RINSDK_BUILD_PKG) || \
    defined(RINSDK_BUILD_DEVICE)
static int versioned(const void* value, uint32_t minimum_size) {
    const RinVersionedV1* header = (const RinVersionedV1*)value;
    return header && header->struct_size >= minimum_size &&
           header->version == RIN_SDK_STRUCT_VERSION_1;
}
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_CORE)
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

RinResult rin_sdk_invoke_v1(uint32_t library_id, uint32_t operation,
                            const void* request, uint32_t request_size,
                            void* response, uint32_t response_size) {
    RinSdkInvokeV1 invoke;
    if (__atomic_load_n(&g_backend_state, __ATOMIC_ACQUIRE) != 2u) {
        return RIN_ERROR_NOT_SUPPORTED;
    }
    if (library_id < RIN_SDK_LIBRARY_BASE || library_id > RIN_SDK_LIBRARY_DEVICE ||
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
    defined(RINSDK_BUILD_PKG) || defined(RINSDK_BUILD_DEVICE)
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
    if (call_result != RIN_SUCCESS) { \
        rin_sdk_zero_bytes((out), sizeof(*(out))); \
        ((RinVersionedV1 *)(out))->struct_size = sizeof(*(out)); \
        ((RinVersionedV1 *)(out))->version = RIN_SDK_STRUCT_VERSION_1; \
    } \
    return call_result; \
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

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_BASE)
RinResult rin_object_close_v1(RinObject object) {
    SIMPLE_CALL(RIN_SDK_LIBRARY_BASE, BASE_OBJECT_CLOSE, (RinResult*)0, object,0,0,0,0,0);
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
    if (query_result != RIN_SUCCESS) {
        rin_sdk_zero_bytes(info, sizeof(*info));
        info->struct_size = sizeof(*info);
        info->version = (uint16_t)RIN_SDK_STRUCT_VERSION_1;
    }
    return query_result;
}
RinResult rin_process_spawn_v1(const RinProcessSpawnV1* request, RinProcess* process) {
    if (!versioned(request, sizeof(*request)) || !process) return RIN_ERROR_INVALID_ARGUMENT;
    return rin_sdk_invoke_scalar_output_v1(
        RIN_SDK_LIBRARY_BASE, BASE_PROCESS_SPAWN, request, sizeof(*request),
        process, sizeof(*process));
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
    return rin_sdk_invoke_scalar_output_v1(
        RIN_SDK_LIBRARY_BASE, BASE_THREAD_CREATE, request, sizeof(*request),
        thread, sizeof(*thread));
}
RinResult rin_thread_join_v1(RinThread thread, uint64_t timeout_ns, int32_t* exit_code) {
    SCALAR_OUTPUT_SIMPLE_CALL(
        RIN_SDK_LIBRARY_BASE, BASE_THREAD_JOIN, exit_code,
        thread,timeout_ns,0,0,0,0);
}
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_IPC)
RinResult rin_channel_create_v1(RinChannel* first, RinChannel* second) {
    RinChannel pair[2] = {0, 0}; RinResult result;
    if (!first || !second) return RIN_ERROR_INVALID_ARGUMENT;
    result = rin_sdk_invoke_scalar_output_v1(
        RIN_SDK_LIBRARY_IPC, IPC_CHANNEL_CREATE, NULL, 0u, pair, sizeof(pair));
    if (result == RIN_SUCCESS) { *first = pair[0]; *second = pair[1]; }
    return result;
}
RinResult rin_channel_send_v1(RinChannel channel, const RinIpcMessageV1* message) {
    struct { RinVersionedV1 header; RinChannel channel; RinIpcMessageV1 message; } request;
    if (!versioned(message, sizeof(*message))) return RIN_ERROR_ABI_MISMATCH;
    rin_sdk_zero_bytes(&request, sizeof(request));
    request.header.struct_size = sizeof(request);
    request.header.version = RIN_SDK_STRUCT_VERSION_1;
    request.channel = channel;
    rin_sdk_copy_bytes(&request.message, message, sizeof(request.message));
    return rin_sdk_invoke_v1(RIN_SDK_LIBRARY_IPC, IPC_CHANNEL_SEND,
                             &request, sizeof(request), NULL, 0u);
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
    if (receive_result != RIN_SUCCESS) {
        rin_sdk_zero_bytes(message, sizeof(*message));
        message->struct_size = sizeof(*message);
        message->version = RIN_SDK_STRUCT_VERSION_1;
        message->bytes = bytes;
        message->handles = handles;
    }
    return receive_result;
}
RinResult rin_event_create_v1(uint32_t flags, RinEvent* event) {
    SCALAR_OUTPUT_SIMPLE_CALL(
        RIN_SDK_LIBRARY_IPC, IPC_EVENT_CREATE, event, flags,0,0,0,0,0);
}
RinResult rin_event_signal_v1(RinEvent event, uint64_t value) {
    SIMPLE_CALL(RIN_SDK_LIBRARY_IPC, IPC_EVENT_SIGNAL, (RinResult*)0, event,value,0,0,0,0);
}
RinResult rin_wait_many_v1(RinSliceV1 items, uint64_t timeout_ns, uint32_t* index) {
    SCALAR_OUTPUT_SIMPLE_CALL(
        RIN_SDK_LIBRARY_IPC, IPC_WAIT_MANY, index,
        items.address,items.size,timeout_ns,0,0,0);
}
RinResult rin_shared_memory_create_v1(uint64_t size, uint32_t flags, RinSharedMemory* memory) {
    SCALAR_OUTPUT_SIMPLE_CALL(
        RIN_SDK_LIBRARY_IPC, IPC_SHARED_MEMORY_CREATE, memory,
        size,flags,0,0,0,0);
}
RinResult rin_service_connect_v1(RinStringV1 name, RinChannel* channel) {
    SCALAR_OUTPUT_SIMPLE_CALL(
        RIN_SDK_LIBRARY_IPC, IPC_SERVICE_CONNECT, channel,
        name.address,name.size,0,0,0,0);
}
RinResult rin_wait_set_create_v1(uint32_t flags, RinWaitSet* wait_set) {
    SCALAR_OUTPUT_SIMPLE_CALL(
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
    if (wait_result != RIN_SUCCESS) {
        rin_sdk_zero_bytes(result, sizeof(*result));
        result->struct_size = sizeof(*result);
        result->version = RIN_SDK_STRUCT_VERSION_1;
    }
    return wait_result;
}
RinResult rin_service_register_v1(RinStringV1 name, uint32_t flags, RinService* service) {
    if (!service) return RIN_ERROR_INVALID_ARGUMENT;
    SCALAR_OUTPUT_SIMPLE_CALL(
        RIN_SDK_LIBRARY_IPC, IPC_SERVICE_REGISTER, service,
        name.address,name.size,flags,0,0,0);
}
RinResult rin_service_accept_v1(RinService service, uint64_t timeout_ns, RinChannel* channel) {
    if (!channel) return RIN_ERROR_INVALID_ARGUMENT;
    SCALAR_OUTPUT_SIMPLE_CALL(
        RIN_SDK_LIBRARY_IPC, IPC_SERVICE_ACCEPT, channel,
        service,timeout_ns,0,0,0,0);
}
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_FS)
RinResult rin_file_open_v1(const RinFileOpenV1* request, RinFile* file) {
    if (!versioned(request, sizeof(*request)) || !file) return RIN_ERROR_INVALID_ARGUMENT;
    return rin_sdk_invoke_scalar_output_v1(
        RIN_SDK_LIBRARY_FS, FS_FILE_OPEN, request, sizeof(*request), file,
        sizeof(*file));
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
    SCALAR_OUTPUT_SIMPLE_CALL(
        RIN_SDK_LIBRARY_FS, FS_FILE_WATCH, watch,
        path.address,path.size,events,0,0,0);
}
RinResult rin_file_read_async_v1(const RinFileIoV1* request, RinIoRequest* operation) {
    if (!versioned(request, sizeof(*request)) || !operation) return RIN_ERROR_INVALID_ARGUMENT;
    return rin_sdk_invoke_scalar_output_v1(
        RIN_SDK_LIBRARY_FS, FS_FILE_READ_ASYNC, request, sizeof(*request),
        operation, sizeof(*operation));
}
RinResult rin_file_write_async_v1(const RinFileIoV1* request, RinIoRequest* operation) {
    if (!versioned(request, sizeof(*request)) || !operation) return RIN_ERROR_INVALID_ARGUMENT;
    return rin_sdk_invoke_scalar_output_v1(
        RIN_SDK_LIBRARY_FS, FS_FILE_WRITE_ASYNC, request, sizeof(*request),
        operation, sizeof(*operation));
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
    return rin_sdk_invoke_scalar_output_v1(
        RIN_SDK_LIBRARY_FS, FS_DIRECTORY_OPEN, request, sizeof(*request),
        directory, sizeof(*directory));
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
    if (stat_result != RIN_SUCCESS) {
        rin_sdk_zero_bytes(stat, sizeof(*stat));
        stat->struct_size = sizeof(*stat);
        stat->version = RIN_SDK_STRUCT_VERSION_1;
    }
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
    return rin_sdk_invoke_scalar_output_v1(
        RIN_SDK_LIBRARY_FS, FS_PORTAL_OPEN, request, sizeof(*request), handle,
        sizeof(*handle));
}
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_NET)
RinResult rin_socket_create_v1(uint32_t domain, uint32_t type, uint32_t protocol, RinSocket* socket) { SCALAR_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_NET, NET_SOCKET_CREATE, socket, domain,type,protocol,0,0,0); }
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
RinResult rin_tls_connect_v1(RinSocket socket, RinStringV1 server_name, uint32_t flags, RinTlsSession* session) { SCALAR_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_NET, NET_TLS_CONNECT, session, socket,server_name.address,server_name.size,flags,0,0); }
RinResult rin_http_execute_v1(const RinHttpRequestV1* request, RinChannel* response_stream) {
    if (!versioned(request, sizeof(*request))) return RIN_ERROR_ABI_MISMATCH;
    return rin_sdk_invoke_scalar_output_v1(
        RIN_SDK_LIBRARY_NET, NET_HTTP_EXECUTE, request, sizeof(*request),
        response_stream, response_stream ? sizeof(*response_stream) : 0u);
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
    VERSIONED_OUTPUT_SIMPLE_CALL(
        RIN_SDK_LIBRARY_NET, NET_SOCKET_ACCEPT, accepted,
        socket,timeout_ns,0,0,0,0);
}
RinResult rin_socket_shutdown_v1(RinSocket socket, uint32_t flags) { SIMPLE_CALL(RIN_SDK_LIBRARY_NET, NET_SOCKET_SHUTDOWN, (RinResult*)0, socket,flags,0,0,0,0); }
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_GUI)
RinResult rin_window_create_v1(const RinWindowCreateV1* request, RinWindow* window) { if (!versioned(request,sizeof(*request))) return RIN_ERROR_ABI_MISMATCH; return rin_sdk_invoke_scalar_output_v1(RIN_SDK_LIBRARY_GUI,GUI_WINDOW_CREATE,request,sizeof(*request),window,window?sizeof(*window):0u); }
RinResult rin_window_next_event_v1(RinWindow window, uint64_t timeout_ns, RinGuiEventV1* event) { VERSIONED_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_GUI,GUI_WINDOW_NEXT_EVENT,event,window,timeout_ns,0,0,0,0); }
RinResult rin_surface_create_v1(uint32_t width,uint32_t height,uint32_t format,RinSurface* surface) { SCALAR_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_GUI,GUI_SURFACE_CREATE,surface,width,height,format,0,0,0); }
RinResult rin_window_present_v1(RinWindow window,RinSurface surface,RinSliceV1 damage) { SIMPLE_CALL(RIN_SDK_LIBRARY_GUI,GUI_WINDOW_PRESENT,(RinResult*)0,window,surface,damage.address,damage.size,0,0); }
RinResult rin_clipboard_set_v1(uint32_t format,RinSliceV1 data) { SIMPLE_CALL(RIN_SDK_LIBRARY_GUI,GUI_CLIPBOARD_SET,(RinResult*)0,format,data.address,data.size,0,0,0); }
RinResult rin_clipboard_get_v1(uint32_t format,RinSliceV1 data,uint64_t* required) { SCALAR_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_GUI,GUI_CLIPBOARD_GET,required,format,data.address,data.size,0,0,0); }
RinResult rin_dialog_show_v1(RinStringV1 title,RinStringV1 message,uint32_t flags,uint32_t* result) { SCALAR_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_GUI,GUI_DIALOG_SHOW,result,title.address,title.size,message.address,message.size,flags,0); }
RinResult rin_notification_show_v1(RinStringV1 title,RinStringV1 message,uint32_t flags) { SIMPLE_CALL(RIN_SDK_LIBRARY_GUI,GUI_NOTIFICATION_SHOW,(RinResult*)0,title.address,title.size,message.address,message.size,flags,0); }
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_MEDIA)
RinResult rin_media_open_v1(const RinMediaOpenV1* request,RinMediaObject* object) { if (!versioned(request,sizeof(*request))) return RIN_ERROR_ABI_MISMATCH; return rin_sdk_invoke_scalar_output_v1(RIN_SDK_LIBRARY_MEDIA,MEDIA_OPEN,request,sizeof(*request),object,object?sizeof(*object):0u); }
RinResult rin_image_decode_v1(RinMediaObject object,uint32_t format,RinSliceV1 output,uint64_t* required) { SCALAR_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_MEDIA,MEDIA_IMAGE_DECODE,required,object,format,output.address,output.size,0,0); }
RinResult rin_audio_decode_v1(RinMediaObject object,RinSliceV1 output,uint64_t* frames) { SCALAR_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_MEDIA,MEDIA_AUDIO_DECODE,frames,object,output.address,output.size,0,0,0); }
RinResult rin_video_decode_v1(RinMediaObject object,RinSurface target,uint64_t* timestamp_ns) { SCALAR_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_MEDIA,MEDIA_VIDEO_DECODE,timestamp_ns,object,target,0,0,0,0); }
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_CONFIG)
RinResult rin_config_open_v1(RinStringV1 domain,uint32_t flags,RinConfigStore* store) { SCALAR_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_CONFIG,CONFIG_OPEN,store,domain.address,domain.size,flags,0,0,0); }
RinResult rin_config_get_v1(RinConfigStore store,RinStringV1 key,RinSliceV1 value,uint64_t* required) { SCALAR_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_CONFIG,CONFIG_GET,required,store,key.address,key.size,value.address,value.size,0); }
RinResult rin_config_set_v1(RinConfigStore store,RinStringV1 key,RinSliceV1 value,uint32_t flags) { SIMPLE_CALL(RIN_SDK_LIBRARY_CONFIG,CONFIG_SET,(RinResult*)0,store,key.address,key.size,value.address,value.size,flags); }
RinResult rin_config_commit_v1(RinConfigStore store) { SIMPLE_CALL(RIN_SDK_LIBRARY_CONFIG,CONFIG_COMMIT,(RinResult*)0,store,0,0,0,0,0); }
RinResult rin_config_remove_v1(RinConfigStore store,RinStringV1 key,uint32_t flags) { SIMPLE_CALL(RIN_SDK_LIBRARY_CONFIG,CONFIG_REMOVE,(RinResult*)0,store,key.address,key.size,flags,0,0); }
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_PKG)
RinResult rin_package_open_v1(RinStringV1 name,RinPackage* package) { SCALAR_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_PKG,PKG_OPEN,package,name.address,name.size,0,0,0,0); }
RinResult rin_package_query_v1(RinPackage package,RinPackageInfoV1* info) { VERSIONED_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_PKG,PKG_QUERY,info,package,0,0,0,0,0); }
RinResult rin_package_install_v1(RinStringV1 source,uint32_t flags,RinEvent completion) { SIMPLE_CALL(RIN_SDK_LIBRARY_PKG,PKG_INSTALL,(RinResult*)0,source.address,source.size,flags,completion,0,0); }
RinResult rin_package_remove_v1(RinStringV1 name,uint32_t flags,RinEvent completion) { SIMPLE_CALL(RIN_SDK_LIBRARY_PKG,PKG_REMOVE,(RinResult*)0,name.address,name.size,flags,completion,0,0); }
RinResult rin_package_enumerate_v1(uint64_t index,RinPackageInfoV1* info) { VERSIONED_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_PKG,PKG_ENUMERATE,info,index,0,0,0,0,0); }
#endif

#if !defined(RINSDK_SPLIT_BUILD) || defined(RINSDK_BUILD_DEVICE)
RinResult rin_device_enumerate_v1(uint32_t device_class,uint64_t index,RinDeviceInfoV1* info) { VERSIONED_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_DEVICE,DEVICE_ENUMERATE,info,device_class,index,0,0,0,0); }
RinResult rin_device_open_v1(uint64_t device_id,uint32_t rights,RinDevice* device) { SCALAR_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_DEVICE,DEVICE_OPEN,device,device_id,rights,0,0,0,0); }
RinResult rin_device_control_v1(RinDevice device,uint32_t operation,RinSliceV1 input,RinSliceV1 output,uint64_t* transferred) { SCALAR_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_DEVICE,DEVICE_CONTROL,transferred,device,operation,input.address,input.size,output.address,output.size); }
RinResult rin_device_query_v1(RinDevice device,RinDeviceInfoV1* info) { VERSIONED_OUTPUT_SIMPLE_CALL(RIN_SDK_LIBRARY_DEVICE,DEVICE_QUERY,info,device,0,0,0,0,0); }
#endif

#undef SIMPLE_CALL
#undef VERSIONED_OUTPUT_SIMPLE_CALL
#undef SCALAR_OUTPUT_SIMPLE_CALL
