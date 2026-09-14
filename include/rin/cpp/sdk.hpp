/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_CPP_SDK_HPP
#define RIN_SDK_CPP_SDK_HPP

#include <rin/sdk.h>

/* Header-only C++20 conveniences.  These types never cross the public .rll
 * boundary: every call below is reduced to the stable fixed-width C ABI. */
namespace rin {

class status final {
public:
    constexpr status() noexcept : code_(RIN_SUCCESS) {}
    constexpr explicit status(RinResult code) noexcept : code_(code) {}

    [[nodiscard]] constexpr bool ok() const noexcept {
        return code_ == RIN_SUCCESS;
    }
    [[nodiscard]] constexpr RinResult code() const noexcept { return code_; }
    constexpr explicit operator bool() const noexcept { return ok(); }

private:
    RinResult code_;
};

template<typename T>
class outcome final {
public:
    constexpr outcome(RinResult code, const T& value) noexcept
        : code_(code), value_(value) {}

    [[nodiscard]] constexpr bool ok() const noexcept {
        return code_ == RIN_SUCCESS;
    }
    [[nodiscard]] constexpr RinResult code() const noexcept { return code_; }
    [[nodiscard]] constexpr const T& value() const noexcept { return value_; }
    [[nodiscard]] constexpr T& value() noexcept { return value_; }
    constexpr explicit operator bool() const noexcept { return ok(); }

private:
    RinResult code_;
    T value_;
};

template<typename T>
[[nodiscard]] constexpr T versioned() noexcept {
    T value{};
    value.struct_size = sizeof(T);
    value.version = RIN_SDK_STRUCT_VERSION_1;
    return value;
}

[[nodiscard]] inline RinStringV1 string(const char* data,
                                        uint64_t size) noexcept {
    return RinStringV1{static_cast<uint64_t>(reinterpret_cast<uintptr_t>(data)),
                       size};
}

[[nodiscard]] inline RinSliceV1 slice(void* data, uint64_t size) noexcept {
    return RinSliceV1{static_cast<uint64_t>(reinterpret_cast<uintptr_t>(data)),
                      size};
}

[[nodiscard]] inline RinSliceV1 slice(const void* data,
                                      uint64_t size) noexcept {
    return RinSliceV1{static_cast<uint64_t>(reinterpret_cast<uintptr_t>(data)),
                      size};
}

template<typename Handle = RinObject>
class unique_handle final {
public:
    constexpr unique_handle() noexcept : handle_(RIN_HANDLE_INVALID) {}
    constexpr explicit unique_handle(Handle handle) noexcept : handle_(handle) {}
    unique_handle(const unique_handle&) = delete;
    unique_handle& operator=(const unique_handle&) = delete;

    constexpr unique_handle(unique_handle&& other) noexcept
        : handle_(other.release()) {}

    unique_handle& operator=(unique_handle&& other) noexcept {
        if (this != &other) {
            (void)close();
            handle_ = other.release();
        }
        return *this;
    }

    ~unique_handle() {
        if (handle_ != RIN_HANDLE_INVALID) {
            (void)rin_object_close_v1(static_cast<RinObject>(handle_));
        }
    }

    [[nodiscard]] constexpr Handle get() const noexcept { return handle_; }
    [[nodiscard]] constexpr explicit operator bool() const noexcept {
        return handle_ != RIN_HANDLE_INVALID;
    }
    [[nodiscard]] constexpr Handle release() noexcept {
        Handle value = handle_;
        handle_ = RIN_HANDLE_INVALID;
        return value;
    }
    status close() noexcept {
        if (handle_ == RIN_HANDLE_INVALID) return status{};
        RinResult result = rin_object_close_v1(static_cast<RinObject>(handle_));
        if (result == RIN_SUCCESS) handle_ = RIN_HANDLE_INVALID;
        return status{result};
    }
    status reset() noexcept { return close(); }

private:
    Handle handle_;
};

struct channel_pair final {
    RinChannel first;
    RinChannel second;
};

[[nodiscard]] inline outcome<RinObjectInfoV1>
object_query(RinObject object) noexcept {
    auto info = versioned<RinObjectInfoV1>();
    return {rin_object_query_v1(object, &info), info};
}

[[nodiscard]] inline outcome<RinProcess>
process_spawn(const RinProcessSpawnV1& request) noexcept {
    RinProcess process = RIN_HANDLE_INVALID;
    return {rin_process_spawn_v1(&request, &process), process};
}

[[nodiscard]] inline outcome<int32_t>
process_wait(RinProcess process, uint64_t timeout_ns = RIN_SDK_INFINITE) noexcept {
    int32_t exit_code = 0;
    return {rin_process_wait_v1(process, timeout_ns, &exit_code), exit_code};
}

[[nodiscard]] inline outcome<uint64_t>
memory_map(const RinMemoryMapV1& request) noexcept {
    uint64_t address = 0u;
    return {rin_memory_map_v1(&request, &address), address};
}

[[nodiscard]] inline status memory_unmap(uint64_t address,
                                         uint64_t size) noexcept {
    return status{rin_memory_unmap_v1(address, size)};
}

[[nodiscard]] inline outcome<RinTimeV1> time_get(uint32_t clock_id) noexcept {
    auto value = versioned<RinTimeV1>();
    return {rin_time_get_v1(clock_id, &value), value};
}

[[nodiscard]] inline status random_fill(RinSliceV1 buffer) noexcept {
    return status{rin_random_fill_v1(buffer)};
}

[[nodiscard]] inline status log_write(uint32_t level,
                                      RinStringV1 message) noexcept {
    return status{rin_log_write_v1(level, message)};
}

[[nodiscard]] inline outcome<RinThread>
thread_create(const RinThreadCreateV1& request) noexcept {
    RinThread thread = RIN_HANDLE_INVALID;
    return {rin_thread_create_v1(&request, &thread), thread};
}

[[nodiscard]] inline outcome<int32_t>
thread_join(RinThread thread,
            uint64_t timeout_ns = RIN_SDK_INFINITE) noexcept {
    int32_t exit_code = 0;
    return {rin_thread_join_v1(thread, timeout_ns, &exit_code), exit_code};
}

[[nodiscard]] inline outcome<channel_pair> channel_create() noexcept {
    channel_pair pair{RIN_HANDLE_INVALID, RIN_HANDLE_INVALID};
    return {rin_channel_create_v1(&pair.first, &pair.second), pair};
}

[[nodiscard]] inline status channel_send(
    RinChannel channel, const RinIpcMessageV1& message) noexcept {
    return status{rin_channel_send_v1(channel, &message)};
}

[[nodiscard]] inline outcome<RinIpcMessageV1>
channel_receive(RinChannel channel) noexcept {
    auto message = versioned<RinIpcMessageV1>();
    return {rin_channel_receive_v1(channel, &message), message};
}

[[nodiscard]] inline outcome<RinEvent> event_create(uint32_t flags) noexcept {
    RinEvent event = RIN_HANDLE_INVALID;
    return {rin_event_create_v1(flags, &event), event};
}

[[nodiscard]] inline status event_signal(RinEvent event,
                                         uint64_t value) noexcept {
    return status{rin_event_signal_v1(event, value)};
}

[[nodiscard]] inline outcome<uint32_t>
wait_many(RinSliceV1 items, uint64_t timeout_ns = RIN_SDK_INFINITE) noexcept {
    uint32_t index = 0u;
    return {rin_wait_many_v1(items, timeout_ns, &index), index};
}

[[nodiscard]] inline outcome<RinSharedMemory>
shared_memory_create(uint64_t size, uint32_t flags) noexcept {
    RinSharedMemory memory = RIN_HANDLE_INVALID;
    return {rin_shared_memory_create_v1(size, flags, &memory), memory};
}

[[nodiscard]] inline outcome<RinChannel>
service_connect(RinStringV1 name) noexcept {
    RinChannel channel = RIN_HANDLE_INVALID;
    return {rin_service_connect_v1(name, &channel), channel};
}

[[nodiscard]] inline outcome<RinService>
service_register(RinStringV1 name, uint32_t flags = 0u) noexcept {
    RinService service = RIN_HANDLE_INVALID;
    return {rin_service_register_v1(name, flags, &service), service};
}

[[nodiscard]] inline outcome<RinChannel>
service_accept(RinService service,
               uint64_t timeout_ns = RIN_SDK_INFINITE) noexcept {
    RinChannel channel = RIN_HANDLE_INVALID;
    return {rin_service_accept_v1(service, timeout_ns, &channel), channel};
}

[[nodiscard]] inline outcome<RinWaitSet>
wait_set_create(uint32_t flags = 0u) noexcept {
    RinWaitSet wait_set = RIN_HANDLE_INVALID;
    return {rin_wait_set_create_v1(flags, &wait_set), wait_set};
}

[[nodiscard]] inline status wait_set_set_items(
    RinWaitSet wait_set, RinSliceV1 items) noexcept {
    return status{rin_wait_set_set_items_v1(wait_set, items)};
}

[[nodiscard]] inline outcome<RinWaitResultV1>
wait_set_wait(RinWaitSet wait_set,
              uint64_t timeout_ns = RIN_SDK_INFINITE) noexcept {
    auto result = versioned<RinWaitResultV1>();
    return {rin_wait_set_wait_v1(wait_set, timeout_ns, &result), result};
}

[[nodiscard]] inline outcome<RinFile>
file_open(const RinFileOpenV1& request) noexcept {
    RinFile file = RIN_HANDLE_INVALID;
    return {rin_file_open_v1(&request, &file), file};
}

[[nodiscard]] inline outcome<uint64_t> file_read(RinFileIoV1& request) noexcept {
    uint64_t transferred = 0u;
    return {rin_file_read_v1(&request, &transferred), transferred};
}

[[nodiscard]] inline outcome<uint64_t>
file_write(RinFileIoV1& request) noexcept {
    uint64_t transferred = 0u;
    return {rin_file_write_v1(&request, &transferred), transferred};
}

[[nodiscard]] inline status file_flush(RinFile file) noexcept {
    return status{rin_file_flush_v1(file)};
}

[[nodiscard]] inline outcome<RinDirectoryEntryV1>
directory_next(RinDirectory directory) noexcept {
    auto entry = versioned<RinDirectoryEntryV1>();
    return {rin_directory_next_v1(directory, &entry), entry};
}

/* Directory names are caller-owned output.  Keeping the buffer explicit
 * prevents an opaque SDK handle from smuggling a pointer to a temporary
 * string across the ABI. */
[[nodiscard]] inline outcome<RinDirectoryEntryV1>
directory_next(RinDirectory directory, char* name,
               uint64_t name_capacity) noexcept {
    auto entry = versioned<RinDirectoryEntryV1>();
    entry.name = string(name, name_capacity);
    return {rin_directory_next_v1(directory, &entry), entry};
}

[[nodiscard]] inline outcome<RinDirectory>
directory_open(const RinDirectoryOpenV1& request) noexcept {
    RinDirectory directory = RIN_HANDLE_INVALID;
    return {rin_directory_open_v1(&request, &directory), directory};
}

[[nodiscard]] inline status directory_rewind(RinDirectory directory) noexcept {
    return status{rin_directory_rewind_v1(directory)};
}

[[nodiscard]] inline outcome<uint64_t>
path_normalize(RinStringV1 input, RinSliceV1 output) noexcept {
    uint64_t required = 0u;
    return {rin_path_normalize_v1(input, output, &required), required};
}

[[nodiscard]] inline outcome<RinFileWatch>
file_watch(RinStringV1 path, uint32_t events) noexcept {
    RinFileWatch watch = RIN_HANDLE_INVALID;
    return {rin_file_watch_v1(path, events, &watch), watch};
}

[[nodiscard]] inline outcome<RinFileWatchEventV1>
file_watch_next(RinFileWatch watch,
                uint64_t timeout_ns = RIN_SDK_INFINITE) noexcept {
    auto event = versioned<RinFileWatchEventV1>();
    return {rin_file_watch_next_v1(watch, timeout_ns, &event), event};
}

[[nodiscard]] inline outcome<RinIoRequest>
file_read_async(const RinFileIoV1& request) noexcept {
    RinIoRequest operation = RIN_HANDLE_INVALID;
    return {rin_file_read_async_v1(&request, &operation), operation};
}

[[nodiscard]] inline outcome<RinIoRequest>
file_write_async(const RinFileIoV1& request) noexcept {
    RinIoRequest operation = RIN_HANDLE_INVALID;
    return {rin_file_write_async_v1(&request, &operation), operation};
}

[[nodiscard]] inline outcome<uint64_t>
file_io_result(RinIoRequest operation) noexcept {
    uint64_t transferred = 0u;
    return {rin_file_io_result_v1(operation, &transferred), transferred};
}

[[nodiscard]] inline status file_io_cancel(RinIoRequest operation) noexcept {
    return status{rin_file_io_cancel_v1(operation)};
}

[[nodiscard]] inline outcome<RinSocket>
socket_create(uint32_t domain, uint32_t type, uint32_t protocol) noexcept {
    RinSocket socket = RIN_HANDLE_INVALID;
    return {rin_socket_create_v1(domain, type, protocol, &socket), socket};
}

[[nodiscard]] inline status socket_connect(
    RinSocket socket, const RinSocketAddressV1& address) noexcept {
    return status{rin_socket_connect_v1(socket, &address)};
}

[[nodiscard]] inline outcome<uint64_t>
socket_send(RinSocket socket, RinSliceV1 data, uint32_t flags = 0u) noexcept {
    uint64_t sent = 0u;
    return {rin_socket_send_v1(socket, data, flags, &sent), sent};
}

[[nodiscard]] inline outcome<uint64_t>
socket_receive(RinSocket socket, RinSliceV1 data,
               uint32_t flags = 0u) noexcept {
    uint64_t received = 0u;
    return {rin_socket_receive_v1(socket, data, flags, &received), received};
}

[[nodiscard]] inline status socket_bind(
    RinSocket socket, const RinSocketAddressV1& address) noexcept {
    return status{rin_socket_bind_v1(socket, &address)};
}

[[nodiscard]] inline status socket_listen(RinSocket socket,
                                          uint32_t backlog) noexcept {
    return status{rin_socket_listen_v1(socket, backlog)};
}

[[nodiscard]] inline outcome<RinSocketAcceptV1>
socket_accept(RinSocket socket,
              uint64_t timeout_ns = RIN_SDK_INFINITE) noexcept {
    auto accepted = versioned<RinSocketAcceptV1>();
    return {rin_socket_accept_v1(socket, timeout_ns, &accepted), accepted};
}

[[nodiscard]] inline status socket_shutdown(RinSocket socket,
                                            uint32_t flags) noexcept {
    return status{rin_socket_shutdown_v1(socket, flags)};
}

[[nodiscard]] inline outcome<uint64_t>
dns_resolve(RinStringV1 host, RinStringV1 service,
            RinSliceV1 results) noexcept {
    uint64_t count = 0u;
    return {rin_dns_resolve_v1(host, service, results, &count), count};
}

[[nodiscard]] inline outcome<RinTlsSession>
tls_connect(RinSocket socket, RinStringV1 server_name,
            uint32_t flags = 0u) noexcept {
    RinTlsSession session = RIN_HANDLE_INVALID;
    return {rin_tls_connect_v1(socket, server_name, flags, &session), session};
}

[[nodiscard]] inline outcome<uint64_t>
tls_send(RinTlsSession session, RinSliceV1 data,
         uint32_t flags = 0u) noexcept {
    uint64_t sent = 0u;
    return {rin_tls_send_v1(session, data, flags, &sent), sent};
}

[[nodiscard]] inline outcome<uint64_t>
tls_receive(RinTlsSession session, RinSliceV1 data,
            uint32_t flags = 0u) noexcept {
    uint64_t received = 0u;
    return {rin_tls_receive_v1(session, data, flags, &received), received};
}

[[nodiscard]] inline status tls_shutdown(RinTlsSession session,
                                         uint32_t flags = 0u) noexcept {
    return status{rin_tls_shutdown_v1(session, flags)};
}

[[nodiscard]] inline outcome<RinChannel>
http_execute(const RinHttpRequestV1& request) noexcept {
    RinChannel stream = RIN_HANDLE_INVALID;
    return {rin_http_execute_v1(&request, &stream), stream};
}

[[nodiscard]] inline outcome<RinWindow>
window_create(const RinWindowCreateV1& request) noexcept {
    RinWindow window = RIN_HANDLE_INVALID;
    return {rin_window_create_v1(&request, &window), window};
}

[[nodiscard]] inline outcome<RinGuiEventV1>
window_next_event(RinWindow window,
                  uint64_t timeout_ns = RIN_SDK_INFINITE) noexcept {
    auto event = versioned<RinGuiEventV1>();
    return {rin_window_next_event_v1(window, timeout_ns, &event), event};
}

[[nodiscard]] inline outcome<RinSurface>
surface_create(uint32_t width, uint32_t height, uint32_t format) noexcept {
    RinSurface surface = RIN_HANDLE_INVALID;
    return {rin_surface_create_v1(width, height, format, &surface), surface};
}

[[nodiscard]] inline status window_present(
    RinWindow window, RinSurface surface, RinSliceV1 damage) noexcept {
    return status{rin_window_present_v1(window, surface, damage)};
}

[[nodiscard]] inline status clipboard_set(uint32_t format,
                                          RinSliceV1 data) noexcept {
    return status{rin_clipboard_set_v1(format, data)};
}

[[nodiscard]] inline outcome<uint64_t>
clipboard_get(uint32_t format, RinSliceV1 data) noexcept {
    uint64_t required = 0u;
    return {rin_clipboard_get_v1(format, data, &required), required};
}

[[nodiscard]] inline outcome<uint32_t>
dialog_show(RinStringV1 title, RinStringV1 message, uint32_t flags) noexcept {
    uint32_t result = 0u;
    return {rin_dialog_show_v1(title, message, flags, &result), result};
}

[[nodiscard]] inline status notification_show(
    RinStringV1 title, RinStringV1 message, uint32_t flags) noexcept {
    return status{rin_notification_show_v1(title, message, flags)};
}

[[nodiscard]] inline outcome<RinMediaObject>
media_open(const RinMediaOpenV1& request) noexcept {
    RinMediaObject object = RIN_HANDLE_INVALID;
    return {rin_media_open_v1(&request, &object), object};
}

[[nodiscard]] inline outcome<uint64_t>
image_decode(RinMediaObject object, uint32_t format,
             RinSliceV1 output) noexcept {
    uint64_t required = 0u;
    return {rin_image_decode_v1(object, format, output, &required), required};
}

[[nodiscard]] inline outcome<uint64_t>
audio_decode(RinMediaObject object, RinSliceV1 output) noexcept {
    uint64_t frames = 0u;
    return {rin_audio_decode_v1(object, output, &frames), frames};
}

[[nodiscard]] inline outcome<uint64_t>
video_decode(RinMediaObject object, RinSurface target) noexcept {
    uint64_t timestamp_ns = 0u;
    return {rin_video_decode_v1(object, target, &timestamp_ns), timestamp_ns};
}

[[nodiscard]] inline outcome<RinConfigStore>
config_open(RinStringV1 domain, uint32_t flags) noexcept {
    RinConfigStore store = RIN_HANDLE_INVALID;
    return {rin_config_open_v1(domain, flags, &store), store};
}

[[nodiscard]] inline outcome<uint64_t>
config_get(RinConfigStore store, RinStringV1 key, RinSliceV1 value) noexcept {
    uint64_t required = 0u;
    return {rin_config_get_v1(store, key, value, &required), required};
}

[[nodiscard]] inline status config_set(
    RinConfigStore store, RinStringV1 key, RinSliceV1 value,
    uint32_t flags = 0u) noexcept {
    return status{rin_config_set_v1(store, key, value, flags)};
}

[[nodiscard]] inline status config_commit(RinConfigStore store) noexcept {
    return status{rin_config_commit_v1(store)};
}

[[nodiscard]] inline status config_remove(
    RinConfigStore store, RinStringV1 key, uint32_t flags = 0u) noexcept {
    return status{rin_config_remove_v1(store, key, flags)};
}

[[nodiscard]] inline outcome<RinPackage>
package_open(RinStringV1 name) noexcept {
    RinPackage package = RIN_HANDLE_INVALID;
    return {rin_package_open_v1(name, &package), package};
}

[[nodiscard]] inline outcome<RinPackageInfoV1>
package_query(RinPackage package) noexcept {
    auto info = versioned<RinPackageInfoV1>();
    return {rin_package_query_v1(package, &info), info};
}

[[nodiscard]] inline status package_install(
    RinStringV1 source, uint32_t flags, RinEvent completion) noexcept {
    return status{rin_package_install_v1(source, flags, completion)};
}

[[nodiscard]] inline status package_remove(
    RinStringV1 name, uint32_t flags, RinEvent completion) noexcept {
    return status{rin_package_remove_v1(name, flags, completion)};
}

[[nodiscard]] inline outcome<RinPackageInfoV1>
package_enumerate(uint64_t index) noexcept {
    auto info = versioned<RinPackageInfoV1>();
    return {rin_package_enumerate_v1(index, &info), info};
}

[[nodiscard]] inline outcome<RinDeviceInfoV1>
device_enumerate(uint32_t device_class, uint64_t index) noexcept {
    auto info = versioned<RinDeviceInfoV1>();
    return {rin_device_enumerate_v1(device_class, index, &info), info};
}

[[nodiscard]] inline outcome<RinDevice>
device_open(uint64_t device_id, uint32_t rights) noexcept {
    RinDevice device = RIN_HANDLE_INVALID;
    return {rin_device_open_v1(device_id, rights, &device), device};
}

[[nodiscard]] inline outcome<uint64_t>
device_control(RinDevice device, uint32_t operation,
               RinSliceV1 input, RinSliceV1 output) noexcept {
    uint64_t transferred = 0u;
    return {rin_device_control_v1(device, operation, input, output,
                                  &transferred), transferred};
}

[[nodiscard]] inline outcome<RinDeviceInfoV1>
device_query(RinDevice device) noexcept {
    auto info = versioned<RinDeviceInfoV1>();
    return {rin_device_query_v1(device, &info), info};
}

} /* namespace rin */

#endif /* RIN_SDK_CPP_SDK_HPP */
