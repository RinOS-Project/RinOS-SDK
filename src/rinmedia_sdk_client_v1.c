/* SPDX-License-Identifier: MIT */
/* Public SDK media-object adapters for authenticated media services. */
#include "rinmedia_sdk_client_v1.h"
#include <rin/internal/imagedecoder_ipc_v1.h>

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <rin/base.h>
#include <rin/service.h>
#include <rin/socket_abi.h>

#define RIN_MEDIA_MDC1_MAGIC UINT32_C(0x3143444d)
#define RIN_MEDIA_MDC1_VERSION UINT32_C(2)
#define RIN_MEDIA_MDC1_TEXT_CAPACITY UINT32_C(4096)
#define RIN_MEDIA_MDC1_MAX_FRAMES UINT32_C(1024)
#define RIN_MEDIA_MDC1_SAMPLE_FORMAT_S16LE UINT32_C(1)
#define RIN_MEDIA_MDC1_SOCKET_PATH "/run/rin/mediad.sock"
#define RIN_MEDIA_MDC1_SOURCE_CHUNK UINT32_C(65536)
#define RIN_MEDIA_MDC1_IO_TIMEOUT_MS 250
#define RIN_MEDIA_MDC1_IO_RETRY_LIMIT UINT32_C(16)
#define RIN_MEDIA_MDC1_MAX_SESSIONS UINT32_C(64)
#define RIN_MEDIA_IMAGE_DECODE_DEADLINE_MS UINT64_C(5000)
#define RIN_MEDIA_IMAGE_IO_TIMEOUT_MS 5000
#define RIN_MEDIA_IMAGE_MAX_FRAMES UINT32_C(1024)
#define RIN_MEDIA_IMAGE_SOURCE_CHUNK UINT32_C(8192)

enum {
    RIN_MEDIA_MDC1_OPEN = 1,
    RIN_MEDIA_MDC1_READ = 2,
    RIN_MEDIA_MDC1_SEEK = 3,
    RIN_MEDIA_MDC1_CLOSE = 4,
    RIN_MEDIA_MDC1_CONFIGURE_OUTPUT = 5
};

typedef struct RinMediaMdc1HeaderV1 {
    uint32_t magic;
    uint32_t version;
    uint16_t operation;
    uint16_t reserved0;
    uint32_t request_id;
    uint32_t payload_bytes;
    int32_t status;
    uint64_t session_id;
} RinMediaMdc1HeaderV1;

typedef struct RinMediaMdc1OutputFormatV2 {
    uint32_t sample_rate;
    uint32_t channels;
    uint32_t sample_format;
    uint32_t reserved0;
} RinMediaMdc1OutputFormatV2;

typedef struct RinMediaMdc1OpenReplyV2 {
    uint64_t session_id;
    int64_t duration_ms;
    uint32_t output_sample_rate;
    uint32_t output_channels;
    uint32_t sample_format;
    uint32_t reserved0;
    uint32_t title_bytes;
    uint32_t artist_bytes;
    uint32_t album_bytes;
    uint32_t reserved1;
    char title[RIN_MEDIA_MDC1_TEXT_CAPACITY];
    char artist[RIN_MEDIA_MDC1_TEXT_CAPACITY];
    char album[RIN_MEDIA_MDC1_TEXT_CAPACITY];
} RinMediaMdc1OpenReplyV2;

typedef struct RinMediaMdc1ReadRequestV1 {
    uint32_t maximum_frames;
    uint32_t reserved0;
} RinMediaMdc1ReadRequestV1;

typedef struct RinMediaMdc1ReadReplyV1 {
    uint32_t frames;
    uint32_t reserved0;
} RinMediaMdc1ReadReplyV1;

typedef struct RinMediaMdc1SeekRequestV1 {
    int64_t position_ms;
} RinMediaMdc1SeekRequestV1;

typedef struct RinMediaSdkSessionV1 {
    volatile uint32_t state; /* 0 free, 1 published, 2 reserved/closing */
    volatile uint32_t lock;
    RinObject object;
    int32_t socket_fd;
    uint32_t next_request_id;
    uint32_t sample_rate;
    uint32_t channels;
    uint32_t media_type;
    uint32_t image_decoded;
    uint64_t session_id;
    uint64_t owner_pid;
    uint64_t image_connection_id;
    uint64_t image_source_size;
    uint64_t image_source_capability;
    uint64_t image_source_generation;
    uint64_t image_decode_request_id;
    RinImageDecoderDecodeResponseV1 image_info;
} RinMediaSdkSessionV1;

static RinMediaSdkSessionV1 g_media_sessions[RIN_MEDIA_MDC1_MAX_SESSIONS];

static void rin_media_session_reset_image(RinMediaSdkSessionV1* session)
{
    if (session == NULL) return;
    session->media_type = 0u;
    session->image_decoded = 0u;
    session->image_connection_id = 0u;
    session->image_source_size = 0u;
    session->image_source_capability = 0u;
    session->image_source_generation = 0u;
    session->image_decode_request_id = 0u;
    memset(&session->image_info, 0, sizeof(session->image_info));
}

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinMediaMdc1HeaderV1) == 32u,
               "MDC1 header ABI drift");
_Static_assert(sizeof(RinMediaMdc1OutputFormatV2) == 16u,
               "MDC1 output format ABI drift");
_Static_assert(sizeof(RinMediaMdc1OpenReplyV2) == 12336u,
               "MDC1 open reply ABI drift");
_Static_assert(sizeof(RinMediaMdc1ReadRequestV1) == 8u,
               "MDC1 read request ABI drift");
_Static_assert(sizeof(RinMediaMdc1ReadReplyV1) == 8u,
               "MDC1 read reply ABI drift");
_Static_assert(sizeof(RinMediaMdc1SeekRequestV1) == 8u,
               "MDC1 seek request ABI drift");
#endif

static uint64_t rin_media_current_pid(void)
{
    const intptr_t result = _syscall0(RIN_SYS_GETPID);
    return result > 0 ? (uint64_t)result : 0u;
}

static int rin_media_range_valid(uint64_t address, uint64_t size,
                                 uint64_t maximum_size, int allow_empty)
{
    if (size == 0u && allow_empty) return address == 0u ||
                                             address <= (uint64_t)UINTPTR_MAX;
    if (address == 0u || size == 0u || size > maximum_size ||
        address > (uint64_t)UINTPTR_MAX)
        return 0;
    return size <= (uint64_t)UINTPTR_MAX - address;
}

static int rin_media_wait_fd_timeout(int fd, short events, int timeout_ms)
{
    struct pollfd item;
    uint32_t attempt;

    memset(&item, 0, sizeof(item));
    item.fd = fd;
    item.events = events;
    for (attempt = 0u; attempt < RIN_MEDIA_MDC1_IO_RETRY_LIMIT; ++attempt) {
        const int result = poll(&item, 1u, timeout_ms);
        if (result < 0) {
            if (errno == EINTR) continue;
            return 0;
        }
        if (result == 0) continue;
        if ((item.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0)
            return 0;
        if ((item.revents & events) != 0) return 1;
        item.revents = 0;
    }
    return 0;
}

static int rin_media_transfer_exact_timeout(int fd, void* buffer,
                                            uint32_t size, int receiving,
                                            int timeout_ms)
{
    uint8_t* cursor = (uint8_t*)buffer;
    uint32_t offset = 0u;

    if (fd < 0 || (size != 0u && buffer == NULL)) return 0;
    while (offset < size) {
        ssize_t count;
        uint32_t attempt = 0u;
        const short readiness = receiving ? POLLIN : POLLOUT;
        if (!rin_media_wait_fd_timeout(fd, readiness, timeout_ms)) return 0;
        do {
            count = receiving
                ? recv(fd, cursor + offset, (size_t)(size - offset), 0)
                : send(fd, cursor + offset, (size_t)(size - offset),
                       MSG_NOSIGNAL);
            if (count < 0 && (errno == EINTR || errno == EAGAIN ||
                              errno == EWOULDBLOCK)) {
                if (++attempt >= RIN_MEDIA_MDC1_IO_RETRY_LIMIT) return 0;
                continue;
            }
            break;
        } while (1);
        if (count <= 0 || (uint64_t)count > (uint64_t)(size - offset))
            return 0;
        offset += (uint32_t)count;
    }
    return 1;
}

static int rin_media_wait_fd(int fd, short events)
{
    return rin_media_wait_fd_timeout(fd, events,
                                     RIN_MEDIA_MDC1_IO_TIMEOUT_MS);
}

static int rin_media_transfer_exact(int fd, void* buffer, uint32_t size,
                                    int receiving)
{
    return rin_media_transfer_exact_timeout(
        fd, buffer, size, receiving, RIN_MEDIA_MDC1_IO_TIMEOUT_MS);
}

static int rin_media_send_open_with_descriptor(
    int socket_fd, const RinMediaMdc1HeaderV1* header,
    const RinMediaMdc1OutputFormatV2* format, int source_fd)
{
    uint8_t frame[sizeof(*header) + sizeof(*format)];
    uint8_t control[CMSG_SPACE(sizeof(int))];
    struct iovec vector;
    struct msghdr message;
    struct cmsghdr* control_header;
    uint32_t offset = 0u;
    int rights_sent = 0;
    uint32_t retries = 0u;

    if (socket_fd < 0 || source_fd < 0 || header == NULL || format == NULL)
        return 0;
    memcpy(frame, header, sizeof(*header));
    memcpy(frame + sizeof(*header), format, sizeof(*format));
    memset(control, 0, sizeof(control));
    memset(&vector, 0, sizeof(vector));
    memset(&message, 0, sizeof(message));
    vector.iov_base = frame;
    vector.iov_len = sizeof(frame);
    message.msg_iov = &vector;
    message.msg_iovlen = 1u;
    message.msg_control = control;
    message.msg_controllen = sizeof(control);
    control_header = CMSG_FIRSTHDR(&message);
    if (control_header == NULL) return 0;
    control_header->cmsg_len = CMSG_LEN(sizeof(source_fd));
    control_header->cmsg_level = SOL_SOCKET;
    control_header->cmsg_type = SCM_RIGHTS;
    memcpy(CMSG_DATA(control_header), &source_fd, sizeof(source_fd));

    while (offset < sizeof(frame)) {
        ssize_t count;
        if (!rin_media_wait_fd(socket_fd, POLLOUT)) return 0;
        if (!rights_sent) {
            vector.iov_base = frame + offset;
            vector.iov_len = sizeof(frame) - offset;
            count = sendmsg(socket_fd, &message, MSG_NOSIGNAL);
        } else {
            count = send(socket_fd, frame + offset, sizeof(frame) - offset,
                         MSG_NOSIGNAL);
        }
        if (count < 0 && (errno == EINTR || errno == EAGAIN ||
                          errno == EWOULDBLOCK)) {
            if (++retries >= RIN_MEDIA_MDC1_IO_RETRY_LIMIT) return 0;
            continue;
        }
        if (count <= 0 || (uint64_t)count > sizeof(frame) - offset) return 0;
        offset += (uint32_t)count;
        rights_sent = 1;
        retries = 0u;
        message.msg_control = NULL;
        message.msg_controllen = 0u;
    }
    return rights_sent;
}

static int rin_media_service_identity_valid(int fd)
{
    rin_unix_service_identity_v1 identity;
    socklen_t size = (socklen_t)sizeof(identity);
    uint32_t expected_slot = 0u;

    memset(&identity, 0, sizeof(identity));
    if (rin_service_find_system_slot("mediad", &expected_slot) != 0 ||
        expected_slot == 0u)
        return 0;
    if (getsockopt(fd, SOL_SOCKET, SO_RIN_UNIX_SERVICE_IDENTITY,
                   &identity, &size) != 0 || size != sizeof(identity))
        return 0;
    return identity.slot_id == expected_slot && identity.owner_uid == 0u &&
           identity.scope == RIN_UNIX_SCOPE_SYSTEM &&
           identity.flags == RIN_UNIX_SERVICE_IDENTITY_FLAG_PUBLISHED;
}

static int rin_media_connect_once(void)
{
    struct sockaddr_un address;
    int fd;

    fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (fd < 0) return -1;
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    memcpy(address.sun_path, RIN_MEDIA_MDC1_SOCKET_PATH,
           sizeof(RIN_MEDIA_MDC1_SOCKET_PATH));
    if (connect(fd, (const struct sockaddr*)&address,
                (socklen_t)sizeof(address)) != 0 ||
        !rin_media_service_identity_valid(fd)) {
        (void)close(fd);
        return -1;
    }
    return fd;
}

static int rin_media_connect_service(void)
{
    int fd = rin_media_connect_once();
    if (fd >= 0) return fd;

    (void)rin_service_start(RIN_SERVICE_SCOPE_SYSTEM, "mediad");
    for (uint32_t elapsed = 0u; elapsed < 500u; elapsed += 25u) {
        (void)poll(NULL, 0u, 25);
        fd = rin_media_connect_once();
        if (fd >= 0) return fd;
    }
    return -1;
}

static RinResult rin_media_image_status_result(int32_t status)
{
    switch (status) {
    case RIN_IMAGE_DECODER_TRANSPORT_OK:
        return RIN_SUCCESS;
    case RIN_IMAGE_DECODER_TRANSPORT_INVALID_ARGUMENT:
        return RIN_ERROR_INVALID_ARGUMENT;
    case RIN_IMAGE_DECODER_TRANSPORT_UNAVAILABLE:
        return RIN_ERROR_NOT_SUPPORTED;
    case RIN_IMAGE_DECODER_TRANSPORT_UNAUTHORIZED:
        return RIN_ERROR_ACCESS_DENIED;
    case RIN_IMAGE_DECODER_TRANSPORT_TIMED_OUT:
        return RIN_ERROR_TIMEOUT;
    case RIN_IMAGE_DECODER_TRANSPORT_CANCELLED:
        return RIN_ERROR_CANCELLED;
    case RIN_IMAGE_DECODER_TRANSPORT_NOT_FOUND:
        return RIN_ERROR_STALE_HANDLE;
    case RIN_IMAGE_DECODER_TRANSPORT_PROTOCOL:
        return RIN_ERROR_INTEGRITY;
    case RIN_IMAGE_DECODER_TRANSPORT_UNSUPPORTED:
        return RIN_ERROR_NOT_SUPPORTED;
    case RIN_IMAGE_DECODER_TRANSPORT_LIMIT_EXCEEDED:
    case RIN_IMAGE_DECODER_TRANSPORT_OVERFLOW:
        return RIN_ERROR_LIMIT_EXCEEDED;
    case RIN_IMAGE_DECODER_TRANSPORT_MALFORMED:
        return RIN_ERROR_INTEGRITY;
    default:
        return RIN_ERROR_INTEGRITY;
    }
}

static void rin_media_image_header_init(
    RinImageDecoderServiceListenerHeaderV1* header, uint16_t operation,
    uint64_t request_id, uint64_t connection_id, uint32_t payload_size)
{
    memset(header, 0, sizeof(*header));
    header->magic = RIN_IMAGE_DECODER_SERVICE_PROTOCOL_MAGIC;
    header->version = RIN_IMAGE_DECODER_SERVICE_PROTOCOL_VERSION;
    header->operation = operation;
    header->request_id = request_id;
    header->payload_size = payload_size;
    header->connection_id = connection_id;
}

static int rin_media_image_service_identity_valid(int fd)
{
    rin_unix_service_identity_v1 identity;
    socklen_t size = (socklen_t)sizeof(identity);
    uint32_t expected_slot = 0u;

    if (fd < 0 || rin_service_find_system_slot(
                      RIN_IMAGE_DECODER_SERVICE_ID, &expected_slot) != 0 ||
        expected_slot == 0u)
        return 0;
    memset(&identity, 0, sizeof(identity));
    if (getsockopt(fd, SOL_SOCKET, SO_RIN_UNIX_SERVICE_IDENTITY,
                   &identity, &size) != 0 || size != sizeof(identity))
        return 0;
    return identity.slot_id == expected_slot && identity.owner_uid == 0u &&
           identity.scope == RIN_UNIX_SCOPE_SYSTEM &&
           identity.flags == RIN_UNIX_SERVICE_IDENTITY_FLAG_PUBLISHED;
}

static int rin_media_image_connect_once(uint64_t* connection_id_out)
{
    RinImageDecoderServiceListenerHeaderV1 request;
    RinImageDecoderServiceListenerHeaderV1 response;
    struct sockaddr_un address;
    int fd;

    if (connection_id_out != NULL) *connection_id_out = 0u;
    if (connection_id_out == NULL) return -1;
    fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (fd < 0) return -1;
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    if (strlen(RIN_IMAGE_DECODER_SERVICE_SOCKET_PATH) >=
            sizeof(address.sun_path)) {
        (void)close(fd);
        return -1;
    }
    memcpy(address.sun_path, RIN_IMAGE_DECODER_SERVICE_SOCKET_PATH,
           strlen(RIN_IMAGE_DECODER_SERVICE_SOCKET_PATH) + 1u);
    if (connect(fd, (const struct sockaddr*)&address,
                (socklen_t)sizeof(address)) != 0 ||
        !rin_media_image_service_identity_valid(fd)) {
        (void)close(fd);
        return -1;
    }
    rin_media_image_header_init(
        &request, RIN_IMAGE_DECODER_SERVICE_LISTENER_HELLO, 0u, 0u, 0u);
    memset(&response, 0, sizeof(response));
    if (!rin_media_transfer_exact_timeout(
            fd, &request, (uint32_t)sizeof(request), 0,
            RIN_MEDIA_IMAGE_IO_TIMEOUT_MS) ||
        !rin_media_transfer_exact_timeout(
            fd, &response, (uint32_t)sizeof(response), 1,
            RIN_MEDIA_IMAGE_IO_TIMEOUT_MS) ||
        response.magic != RIN_IMAGE_DECODER_SERVICE_PROTOCOL_MAGIC ||
        response.version != RIN_IMAGE_DECODER_SERVICE_PROTOCOL_VERSION ||
        response.operation != RIN_IMAGE_DECODER_SERVICE_LISTENER_HELLO ||
        response.flags != 0u || response.request_id != 0u ||
        response.payload_size != 0u || response.status != 0 ||
        response.connection_id == 0u || response.reserved != 0u) {
        (void)close(fd);
        return -1;
    }
    *connection_id_out = response.connection_id;
    return fd;
}

static int rin_media_connect_image_service(uint64_t* connection_id_out)
{
    int fd;
    uint32_t elapsed;

    if (connection_id_out != NULL) *connection_id_out = 0u;
    if (connection_id_out == NULL) return -1;
    fd = rin_media_image_connect_once(connection_id_out);
    if (fd >= 0) return fd;
    (void)rin_service_start(RIN_SERVICE_SCOPE_SYSTEM,
                            RIN_IMAGE_DECODER_SERVICE_ID);
    for (elapsed = 0u; elapsed < 500u; elapsed += 25u) {
        (void)poll(NULL, 0u, 25);
        fd = rin_media_image_connect_once(connection_id_out);
        if (fd >= 0) return fd;
    }
    return -1;
}

static int rin_media_image_response_header(
    RinMediaSdkSessionV1* session, uint16_t operation, uint64_t request_id,
    RinImageDecoderServiceListenerHeaderV1* response)
{
    if (session == NULL || response == NULL || session->socket_fd < 0 ||
        session->image_connection_id == 0u || request_id == 0u ||
        !rin_media_transfer_exact_timeout(
            session->socket_fd, response, (uint32_t)sizeof(*response), 1,
            RIN_MEDIA_IMAGE_IO_TIMEOUT_MS))
        return 0;
    return response->magic == RIN_IMAGE_DECODER_SERVICE_PROTOCOL_MAGIC &&
           response->version == RIN_IMAGE_DECODER_SERVICE_PROTOCOL_VERSION &&
           response->operation == operation && response->flags == 0u &&
           response->request_id == request_id &&
           response->connection_id == session->image_connection_id &&
           response->reserved == 0u &&
           response->status <= RIN_IMAGE_DECODER_TRANSPORT_OK &&
           response->status >= RIN_IMAGE_DECODER_TRANSPORT_MALFORMED &&
           (response->status == RIN_IMAGE_DECODER_TRANSPORT_OK ||
            response->payload_size == 0u);
}

static RinResult rin_media_image_begin_request(
    RinMediaSdkSessionV1* session, uint16_t operation, uint64_t request_id,
    const void* payload, uint32_t payload_size,
    RinImageDecoderServiceListenerHeaderV1* response)
{
    RinImageDecoderServiceListenerHeaderV1 header;

    if (session == NULL || session->socket_fd < 0 ||
        session->image_connection_id == 0u || request_id == 0u ||
        (payload_size != 0u && payload == NULL) || response == NULL)
        return RIN_ERROR_INVALID_ARGUMENT;
    rin_media_image_header_init(&header, operation, request_id,
                                session->image_connection_id, payload_size);
    if (!rin_media_transfer_exact_timeout(
            session->socket_fd, &header, (uint32_t)sizeof(header), 0,
            RIN_MEDIA_IMAGE_IO_TIMEOUT_MS) ||
        (payload_size != 0u && !rin_media_transfer_exact_timeout(
             session->socket_fd, (void*)payload, payload_size, 0,
             RIN_MEDIA_IMAGE_IO_TIMEOUT_MS)) ||
        !rin_media_image_response_header(session, operation, request_id,
                                         response))
        return RIN_ERROR_IO;
    return rin_media_image_status_result(response->status);
}

static int rin_media_header_valid(const RinMediaMdc1HeaderV1* header,
                                  uint16_t operation, uint32_t request_id,
                                  uint64_t session_id)
{
    return header != NULL && header->magic == RIN_MEDIA_MDC1_MAGIC &&
           header->version == RIN_MEDIA_MDC1_VERSION &&
           header->operation == operation && header->reserved0 == 0u &&
           header->request_id == request_id &&
           header->session_id == session_id;
}

static int rin_media_rpc(RinMediaSdkSessionV1* session, uint16_t operation,
                         const void* payload, uint32_t payload_bytes,
                         RinMediaMdc1HeaderV1* reply)
{
    RinMediaMdc1HeaderV1 request;
    RinMediaMdc1HeaderV1 response;

    if (session == NULL || reply == NULL || session->socket_fd < 0 ||
        session->session_id == 0u ||
        (payload_bytes != 0u && payload == NULL))
        return 0;
    if (session->next_request_id == 0u) session->next_request_id = 1u;
    memset(&request, 0, sizeof(request));
    request.magic = RIN_MEDIA_MDC1_MAGIC;
    request.version = RIN_MEDIA_MDC1_VERSION;
    request.operation = operation;
    request.request_id = session->next_request_id++;
    request.payload_bytes = payload_bytes;
    request.session_id = session->session_id;
    if (!rin_media_transfer_exact(session->socket_fd, &request,
                                  (uint32_t)sizeof(request), 0) ||
        (payload_bytes != 0u &&
         !rin_media_transfer_exact(session->socket_fd, (void*)payload,
                                   payload_bytes, 0)) ||
        !rin_media_transfer_exact(session->socket_fd, &response,
                                  (uint32_t)sizeof(response), 1) ||
        !rin_media_header_valid(&response, operation, request.request_id,
                                session->session_id))
        return 0;
    *reply = response;
    return 1;
}

static int rin_media_snapshot_source(const RinMediaOpenV1* request,
                                     int* descriptor_out)
{
    char path[] = "/tmp/rinmedia-XXXXXX";
    const uint8_t* source;
    uint64_t remaining;
    int fd;

    if (descriptor_out == NULL) return 0;
    *descriptor_out = -1;
    if (request == NULL || request->source.size == 0u ||
        request->source.size > RIN_MEDIA_MAX_SOURCE_BYTES_V1 ||
        !rin_media_range_valid(request->source.address,
                               request->source.size,
                               RIN_MEDIA_MAX_SOURCE_BYTES_V1, 0))
        return 0;
    fd = mkstemp(path);
    if (fd < 0) return 0;
    if (fcntl(fd, F_SETFD, FD_CLOEXEC) != 0 || unlink(path) != 0) {
        (void)close(fd);
        (void)unlink(path);
        return 0;
    }
    source = (const uint8_t*)(uintptr_t)request->source.address;
    remaining = request->source.size;
    while (remaining != 0u) {
        const size_t chunk = remaining < RIN_MEDIA_MDC1_SOURCE_CHUNK
                                 ? (size_t)remaining
                                 : (size_t)RIN_MEDIA_MDC1_SOURCE_CHUNK;
        size_t written = 0u;
        while (written < chunk) {
            const ssize_t count = write(fd, source + written, chunk - written);
            if (count < 0 && errno == EINTR) continue;
            if (count <= 0 || (size_t)count > chunk - written) {
                (void)close(fd);
                return 0;
            }
            written += (size_t)count;
        }
        source += chunk;
        remaining -= chunk;
    }
    if (lseek(fd, 0, SEEK_SET) < 0) {
        (void)close(fd);
        return 0;
    }
    *descriptor_out = fd;
    return 1;
}

/* The image service receives a byte stream rather than an SCM_RIGHTS source
 * descriptor. Send from our unlinked snapshot so a caller modifying its
 * original slice concurrently cannot change bytes midway through OPEN. */
static int rin_media_transfer_snapshot_timeout(int source_fd, int socket_fd,
                                               uint64_t size)
{
    uint8_t buffer[RIN_MEDIA_IMAGE_SOURCE_CHUNK];
    uint64_t remaining = size;
    int success = 0;

    if (source_fd < 0 || socket_fd < 0 || size == 0u) return 0;
    while (remaining != 0u) {
        const uint32_t chunk = remaining < sizeof(buffer)
            ? (uint32_t)remaining : (uint32_t)sizeof(buffer);
        uint32_t received = 0u;
        uint32_t interrupted = 0u;
        while (received < chunk) {
            const ssize_t count = read(source_fd, buffer + received,
                                      (size_t)(chunk - received));
            if (count < 0 && errno == EINTR) {
                if (++interrupted < RIN_MEDIA_MDC1_IO_RETRY_LIMIT) continue;
                goto done;
            }
            if (count <= 0 || (uint64_t)count > (uint64_t)(chunk - received))
                goto done;
            received += (uint32_t)count;
            interrupted = 0u;
        }
        if (!rin_media_transfer_exact_timeout(
                socket_fd, buffer, chunk, 0, RIN_MEDIA_IMAGE_IO_TIMEOUT_MS))
            goto done;
        remaining -= chunk;
    }
    success = 1;

done:
    memset(buffer, 0, sizeof(buffer));
    return success;
}

static int rin_media_session_lock(RinMediaSdkSessionV1* session)
{
    while (__sync_lock_test_and_set(&session->lock, 1u) != 0u)
        (void)poll(NULL, 0u, 1);
    return 1;
}

static void rin_media_session_unlock(RinMediaSdkSessionV1* session)
{
    __sync_lock_release(&session->lock);
}

static RinMediaSdkSessionV1* rin_media_session_reserve(void)
{
    uint32_t index;
    for (index = 0u; index < RIN_MEDIA_MDC1_MAX_SESSIONS; ++index) {
        RinMediaSdkSessionV1* session = &g_media_sessions[index];
        if (__sync_bool_compare_and_swap(&session->state, 0u, 2u)) {
            session->lock = 0u;
            session->object = RIN_OBJECT_HANDLE_INVALID;
            session->socket_fd = -1;
            session->next_request_id = 1u;
            session->sample_rate = 0u;
            session->channels = 0u;
            rin_media_session_reset_image(session);
            session->session_id = 0u;
            session->owner_pid = 0u;
            return session;
        }
    }
    return NULL;
}

static void rin_media_session_discard_reserved(
    RinMediaSdkSessionV1* session)
{
    if (session == NULL) return;
    if (session->socket_fd >= 0) (void)close(session->socket_fd);
    session->socket_fd = -1;
    session->session_id = 0u;
    session->object = RIN_OBJECT_HANDLE_INVALID;
    session->next_request_id = 1u;
    session->sample_rate = 0u;
    session->channels = 0u;
    rin_media_session_reset_image(session);
    session->owner_pid = 0u;
    __atomic_store_n(&session->state, 0u, __ATOMIC_RELEASE);
}

/* Called with the session lock held. State remains non-free until the caller
 * unlocks, so another open cannot reuse this slot while the old lock is held. */
static void rin_media_session_drop_locked(RinMediaSdkSessionV1* session,
                                          int send_close)
{
    __atomic_store_n(&session->state, 2u, __ATOMIC_RELEASE);
    if (send_close && session->media_type == RIN_MEDIA_TYPE_AUDIO &&
        session->socket_fd >= 0 && session->session_id != 0u) {
        RinMediaMdc1HeaderV1 reply;
        (void)rin_media_rpc(session, RIN_MEDIA_MDC1_CLOSE, NULL, 0u, &reply);
    }
    if (session->socket_fd >= 0) (void)close(session->socket_fd);
    session->socket_fd = -1;
    session->session_id = 0u;
    session->object = RIN_OBJECT_HANDLE_INVALID;
    session->next_request_id = 1u;
    session->sample_rate = 0u;
    session->channels = 0u;
    rin_media_session_reset_image(session);
    session->owner_pid = 0u;
}

static void rin_media_session_unlock_free(RinMediaSdkSessionV1* session)
{
    rin_media_session_unlock(session);
    __atomic_store_n(&session->state, 0u, __ATOMIC_RELEASE);
}

/* A forked child has a private copy of the session table but shares the
 * inherited socket description with its parent.  The copied lock may have
 * been held by a thread that does not exist in the child, so retire a
 * foreign-owner row without acquiring that lock and without sending CLOSE. */
static void rin_media_session_discard_inherited(
    RinMediaSdkSessionV1* session)
{
    uint32_t expected_state = 1u;
    int socket_fd;

    if (session == NULL ||
        !__atomic_compare_exchange_n(&session->state, &expected_state, 2u, 0,
                                     __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
        return;
    socket_fd = session->socket_fd;
    session->socket_fd = -1;
    session->session_id = 0u;
    session->object = RIN_OBJECT_HANDLE_INVALID;
    session->next_request_id = 1u;
    session->sample_rate = 0u;
    session->channels = 0u;
    rin_media_session_reset_image(session);
    session->owner_pid = 0u;
    session->lock = 0u;
    if (socket_fd >= 0) (void)close(socket_fd);
    __atomic_store_n(&session->state, 0u, __ATOMIC_RELEASE);
}

static RinMediaSdkSessionV1* rin_media_session_find_locked(RinObject object)
{
    const uint64_t caller_pid = rin_media_current_pid();
    uint32_t index;
    if (caller_pid == 0u) return NULL;
    for (index = 0u; index < RIN_MEDIA_MDC1_MAX_SESSIONS; ++index) {
        RinMediaSdkSessionV1* session = &g_media_sessions[index];
        if (__atomic_load_n(&session->state, __ATOMIC_ACQUIRE) != 1u)
            continue;
        if (session->owner_pid != caller_pid) {
            rin_media_session_discard_inherited(session);
            continue;
        }
        rin_media_session_lock(session);
        if (__atomic_load_n(&session->state, __ATOMIC_ACQUIRE) == 1u &&
            session->object == object && session->owner_pid == caller_pid)
            return session;
        rin_media_session_unlock(session);
    }
    return NULL;
}

static RinResult rin_media_image_register_source(
    RinMediaSdkSessionV1* session, const RinMediaOpenV1* request)
{
    RinImageDecoderSourceRegisterRequestV1 register_request;
    RinImageDecoderSourceRegisterResponseV1 register_response;
    RinImageDecoderServiceListenerHeaderV1 header;
    RinImageDecoderServiceListenerHeaderV1 response_header;
    uint64_t connection_id = 0u;
    uint64_t payload_size;
    int source_fd = -1;
    int fd;
    RinResult result;

    if (session == NULL || request == NULL ||
        request->media_type != RIN_MEDIA_TYPE_IMAGE ||
        request->source.size == 0u)
        return RIN_ERROR_INVALID_ARGUMENT;
    if (request->source.size >
        RIN_IMAGE_DECODER_TRANSPORT_MAX_SOURCE_BYTES)
        return RIN_ERROR_LIMIT_EXCEEDED;
    if (!rin_media_snapshot_source(request, &source_fd))
        return RIN_ERROR_IO;
    fd = rin_media_connect_image_service(&connection_id);
    if (fd < 0 || connection_id == 0u) {
        (void)close(source_fd);
        return RIN_ERROR_NOT_SUPPORTED;
    }

    session->socket_fd = fd;
    session->image_connection_id = connection_id;
    session->media_type = RIN_MEDIA_TYPE_IMAGE;
    session->image_source_size = request->source.size;
    session->image_decode_request_id = 1u;

    memset(&register_request, 0, sizeof(register_request));
    register_request.struct_size = sizeof(register_request);
    register_request.version = RIN_IMAGE_DECODER_TRANSPORT_VERSION;
    register_request.operation = RIN_IMAGE_DECODER_TRANSPORT_REGISTER_SOURCE;
    register_request.request_id = 1u;
    register_request.source_size = request->source.size;
    register_request.max_width = RIN_IMAGE_DECODER_TRANSPORT_MAX_DIMENSION;
    register_request.max_height = RIN_IMAGE_DECODER_TRANSPORT_MAX_DIMENSION;
    register_request.max_pixels = RIN_IMAGE_DECODER_TRANSPORT_MAX_PIXELS;
    register_request.max_frames = RIN_MEDIA_IMAGE_MAX_FRAMES;
    register_request.max_output_bytes =
        RIN_IMAGE_DECODER_TRANSPORT_MAX_OUTPUT_BYTES;
    payload_size = sizeof(register_request) + request->source.size;
    if (payload_size > UINT32_MAX) {
        (void)close(source_fd);
        return RIN_ERROR_LIMIT_EXCEEDED;
    }

    rin_media_image_header_init(
        &header, RIN_IMAGE_DECODER_SERVICE_LISTENER_REGISTER_SOURCE,
        register_request.request_id, connection_id, (uint32_t)payload_size);
    if (!rin_media_transfer_exact_timeout(
            fd, &header, (uint32_t)sizeof(header), 0,
            RIN_MEDIA_IMAGE_IO_TIMEOUT_MS) ||
        !rin_media_transfer_exact_timeout(
            fd, &register_request, (uint32_t)sizeof(register_request), 0,
            RIN_MEDIA_IMAGE_IO_TIMEOUT_MS) ||
        !rin_media_transfer_snapshot_timeout(
            source_fd, fd, request->source.size)) {
        (void)close(source_fd);
        return RIN_ERROR_IO;
    }
    if (close(source_fd) != 0) return RIN_ERROR_IO;
    source_fd = -1;
    if (!rin_media_image_response_header(
            session, RIN_IMAGE_DECODER_SERVICE_LISTENER_REGISTER_SOURCE,
            register_request.request_id, &response_header))
        return RIN_ERROR_IO;
    result = rin_media_image_status_result(response_header.status);
    if (result != RIN_SUCCESS) return result;
    if (response_header.payload_size != sizeof(register_response) ||
        !rin_media_transfer_exact_timeout(
            fd, &register_response, (uint32_t)sizeof(register_response), 1,
            RIN_MEDIA_IMAGE_IO_TIMEOUT_MS) ||
        register_response.struct_size != sizeof(register_response) ||
        register_response.version != RIN_IMAGE_DECODER_TRANSPORT_VERSION ||
        register_response.status != RIN_IMAGE_DECODER_TRANSPORT_OK ||
        register_response.flags != 0u ||
        register_response.request_id != register_request.request_id ||
        register_response.source_size != request->source.size ||
        register_response.source_capability == 0u ||
        register_response.source_generation == 0u ||
        register_response.reserved1[0] != 0u ||
        register_response.reserved1[1] != 0u)
        return RIN_ERROR_INTEGRITY;

    session->image_source_capability = register_response.source_capability;
    session->image_source_generation = register_response.source_generation;
    return RIN_SUCCESS;
}

static RinResult rin_media_image_decode_locked(
    RinMediaSdkSessionV1* session)
{
    RinImageDecoderDecodeRequestV1 request;
    RinImageDecoderDecodeResponseV1 response;
    RinImageDecoderServiceListenerHeaderV1 response_header;
    uint64_t request_id;
    uint64_t pixel_count;
    uint64_t pixel_bytes;
    RinResult result;

    if (session == NULL || session->media_type != RIN_MEDIA_TYPE_IMAGE ||
        session->socket_fd < 0 || session->image_source_capability == 0u ||
        session->image_source_generation == 0u ||
        session->image_source_size == 0u)
        return RIN_ERROR_STALE_HANDLE;
    if (session->image_decoded == 1u) return RIN_SUCCESS;
    if (session->image_decode_request_id == UINT64_MAX)
        return RIN_ERROR_LIMIT_EXCEEDED;

    request_id = session->image_decode_request_id + 1u;
    memset(&request, 0, sizeof(request));
    request.struct_size = sizeof(request);
    request.version = RIN_IMAGE_DECODER_TRANSPORT_VERSION;
    request.operation = RIN_IMAGE_DECODER_TRANSPORT_DECODE;
    request.request_id = request_id;
    request.source_capability = session->image_source_capability;
    request.source_generation = session->image_source_generation;
    request.source_size = session->image_source_size;
    request.deadline_ms = RIN_MEDIA_IMAGE_DECODE_DEADLINE_MS;
    request.max_width = RIN_IMAGE_DECODER_TRANSPORT_MAX_DIMENSION;
    request.max_height = RIN_IMAGE_DECODER_TRANSPORT_MAX_DIMENSION;
    request.max_pixels = RIN_IMAGE_DECODER_TRANSPORT_MAX_PIXELS;
    request.max_frames = RIN_MEDIA_IMAGE_MAX_FRAMES;
    request.max_output_bytes =
        RIN_IMAGE_DECODER_TRANSPORT_MAX_OUTPUT_BYTES;
    result = rin_media_image_begin_request(
        session, RIN_IMAGE_DECODER_SERVICE_LISTENER_DECODE, request_id,
        &request, (uint32_t)sizeof(request), &response_header);
    if (result != RIN_SUCCESS) return result;
    if (response_header.payload_size != sizeof(response))
        return RIN_ERROR_INTEGRITY;
    memset(&response, 0, sizeof(response));
    if (!rin_media_transfer_exact_timeout(
            session->socket_fd, &response, (uint32_t)sizeof(response), 1,
            RIN_MEDIA_IMAGE_IO_TIMEOUT_MS))
        return RIN_ERROR_IO;
    if (response.struct_size != sizeof(response) ||
        response.version != RIN_IMAGE_DECODER_TRANSPORT_VERSION ||
        response.status != RIN_IMAGE_DECODER_TRANSPORT_OK ||
        response.flags != 0u || response.request_id != request_id ||
        response.source_generation != session->image_source_generation ||
        response.width == 0u || response.height == 0u ||
        response.width > RIN_IMAGE_DECODER_TRANSPORT_MAX_DIMENSION ||
        response.height > RIN_IMAGE_DECODER_TRANSPORT_MAX_DIMENSION ||
        response.stride_bytes != response.width * 4u ||
        response.frame_count != 1u ||
        response.pixel_format !=
            RIN_IMAGE_DECODER_TRANSPORT_PIXEL_FORMAT_ARGB8888 ||
        response.reserved0 != 0u || response.reserved1[0] != 0u ||
        response.reserved1[1] != 0u)
        return RIN_ERROR_INTEGRITY;
    pixel_count = (uint64_t)response.width * response.height;
    pixel_bytes = pixel_count * 4u;
    if (pixel_count == 0u ||
        pixel_count > RIN_IMAGE_DECODER_TRANSPORT_MAX_PIXELS ||
        pixel_bytes > RIN_IMAGE_DECODER_TRANSPORT_MAX_OUTPUT_BYTES ||
        response.pixel_bytes != pixel_bytes)
        return RIN_ERROR_INTEGRITY;

    response.status = RIN_IMAGE_DECODER_TRANSPORT_OK;
    session->image_info = response;
    session->image_decode_request_id = request_id;
    session->image_decoded = 1u;
    return RIN_SUCCESS;
}

static RinResult rin_media_image_read_pixels_locked(
    RinMediaSdkSessionV1* session, uint8_t* output)
{
    uint64_t offset = 0u;
    const uint64_t total = session->image_info.pixel_bytes;

    if (output == NULL || total == 0u ||
        session->image_decode_request_id == 0u)
        return RIN_ERROR_INVALID_ARGUMENT;
    while (offset < total) {
        RinImageDecoderPixelReadRequestV1 request;
        RinImageDecoderPixelReadResponseV1 response;
        RinImageDecoderServiceListenerHeaderV1 response_header;
        uint64_t remaining = total - offset;
        const uint32_t capacity = remaining <
                RIN_IMAGE_DECODER_TRANSPORT_MAX_PIXEL_CHUNK
            ? (uint32_t)remaining
            : RIN_IMAGE_DECODER_TRANSPORT_MAX_PIXEL_CHUNK;
        RinResult result;

        memset(&request, 0, sizeof(request));
        request.struct_size = sizeof(request);
        request.version = RIN_IMAGE_DECODER_TRANSPORT_VERSION;
        request.operation = RIN_IMAGE_DECODER_TRANSPORT_READ_PIXELS;
        request.request_id = session->image_decode_request_id;
        request.source_generation = session->image_source_generation;
        request.offset = offset;
        request.capacity = capacity;
        result = rin_media_image_begin_request(
            session, RIN_IMAGE_DECODER_SERVICE_LISTENER_READ_PIXELS,
            request.request_id, &request, (uint32_t)sizeof(request),
            &response_header);
        if (result != RIN_SUCCESS) return result;
        if (response_header.payload_size < sizeof(response) ||
            response_header.payload_size > sizeof(response) + capacity)
            return RIN_ERROR_INTEGRITY;
        memset(&response, 0, sizeof(response));
        if (!rin_media_transfer_exact_timeout(
                session->socket_fd, &response, (uint32_t)sizeof(response), 1,
                RIN_MEDIA_IMAGE_IO_TIMEOUT_MS))
            return RIN_ERROR_IO;
        if (response.struct_size != sizeof(response) ||
            response.version != RIN_IMAGE_DECODER_TRANSPORT_VERSION ||
            response.status != RIN_IMAGE_DECODER_TRANSPORT_OK ||
            response.flags != 0u ||
            response.request_id != request.request_id ||
            response.source_generation != request.source_generation ||
            response.offset != offset || response.total_bytes != total ||
            response.bytes_read == 0u || response.bytes_read > capacity ||
            response_header.payload_size != sizeof(response) +
                response.bytes_read || response.reserved0 != 0u ||
            response.reserved1[0] != 0u || response.reserved1[1] != 0u)
            return RIN_ERROR_INTEGRITY;
        if (!rin_media_transfer_exact_timeout(
                session->socket_fd, output + (size_t)offset,
                response.bytes_read, 1, RIN_MEDIA_IMAGE_IO_TIMEOUT_MS))
            return RIN_ERROR_IO;
        offset += response.bytes_read;
    }
    return RIN_SUCCESS;
}

static int rin_media_session_pid_valid(RinMediaSdkSessionV1* session)
{
    const uint64_t pid = rin_media_current_pid();
    /* A forked child inherits the numeric kernel handle and a duplicate of
     * this descriptor, but must not issue requests on the parent's stream. */
    return pid != 0u && pid == session->owner_pid;
}

static RinResult rin_media_validate_object(RinObject object)
{
    RinObjectInfoV1 info;
    RinResult result;

    memset(&info, 0, sizeof(info));
    info.struct_size = sizeof(info);
    info.version = RIN_SDK_STRUCT_VERSION_1;
    result = rin_object_query_v1(object, &info);
    if (result == RIN_ERROR_STALE_HANDLE) {
        /* Raw kernel close paths cannot notify the SDK hook. Retire any
         * matching service-side session when its typed token is already
         * stale, so a descriptor does not stay open until process exit. */
        rin_media_sdk_client_object_close_hook_v1(object);
        return result;
    }
    if (result != RIN_SUCCESS) return result;
    return info.type == RIN_OBJECT_MEDIA ? RIN_SUCCESS
                                         : RIN_ERROR_WRONG_TYPE;
}

RinResult rin_media_sdk_client_open_v1(RinObject object,
                                       const RinMediaOpenV1* request)
{
    RinMediaSdkSessionV1* session;
    RinMediaMdc1HeaderV1 request_header;
    RinMediaMdc1HeaderV1 reply_header;
    RinMediaMdc1OutputFormatV2 output;
    RinMediaMdc1OpenReplyV2 metadata;
    uint64_t owner_pid;
    int source_fd = -1;
    int socket_fd = -1;
    uint32_t index;

    if (object == RIN_OBJECT_HANDLE_INVALID || request == NULL)
        return RIN_ERROR_INVALID_ARGUMENT;
    if (request->media_type != RIN_MEDIA_TYPE_AUDIO &&
        request->media_type != RIN_MEDIA_TYPE_IMAGE)
        return RIN_ERROR_NOT_SUPPORTED;
    if (request->struct_size < sizeof(*request) ||
        request->version != RIN_SDK_STRUCT_VERSION_1 ||
        request->flags != 0u ||
        request->mime_type.size == 0u ||
        request->mime_type.size > RIN_MEDIA_MAX_MIME_TYPE_BYTES_V1 ||
        !rin_media_range_valid(request->mime_type.address,
                               request->mime_type.size, UINT64_MAX, 0) ||
        request->reserved[0] != 0u || request->reserved[1] != 0u ||
        request->reserved[2] != 0u || request->reserved[3] != 0u ||
        request->source.size == 0u ||
        request->source.size > RIN_MEDIA_MAX_SOURCE_BYTES_V1 ||
        !rin_media_range_valid(request->source.address, request->source.size,
                               RIN_MEDIA_MAX_SOURCE_BYTES_V1, 0))
        return RIN_ERROR_INVALID_ARGUMENT;

    owner_pid = rin_media_current_pid();
    if (owner_pid == 0u) return RIN_ERROR_IO;
    session = rin_media_session_reserve();
    if (session == NULL) return RIN_ERROR_TABLE_FULL;
    if (request->media_type == RIN_MEDIA_TYPE_IMAGE) {
        RinResult image_result;
        session->object = object;
        session->owner_pid = owner_pid;
        image_result = rin_media_image_register_source(session, request);
        if (image_result != RIN_SUCCESS) {
            rin_media_session_discard_reserved(session);
            return image_result;
        }
        __atomic_store_n(&session->state, 1u, __ATOMIC_RELEASE);
        return RIN_SUCCESS;
    }
    if (!rin_media_snapshot_source(request, &source_fd)) {
        rin_media_session_discard_reserved(session);
        return RIN_ERROR_IO;
    }
    socket_fd = rin_media_connect_service();
    if (socket_fd < 0) {
        (void)close(source_fd);
        rin_media_session_discard_reserved(session);
        return RIN_ERROR_NOT_SUPPORTED;
    }

    memset(&request_header, 0, sizeof(request_header));
    request_header.magic = RIN_MEDIA_MDC1_MAGIC;
    request_header.version = RIN_MEDIA_MDC1_VERSION;
    request_header.operation = RIN_MEDIA_MDC1_OPEN;
    request_header.request_id = 1u;
    request_header.payload_bytes = sizeof(output);
    memset(&output, 0, sizeof(output));
    output.sample_rate = 48000u;
    output.channels = 2u;
    output.sample_format = RIN_MEDIA_MDC1_SAMPLE_FORMAT_S16LE;
    if (!rin_media_send_open_with_descriptor(socket_fd, &request_header,
                                             &output, source_fd)) {
        (void)close(source_fd);
        (void)close(socket_fd);
        rin_media_session_discard_reserved(session);
        return RIN_ERROR_IO;
    }
    (void)close(source_fd);
    source_fd = -1;
    memset(&reply_header, 0, sizeof(reply_header));
    memset(&metadata, 0, sizeof(metadata));
    if (!rin_media_transfer_exact(socket_fd, &reply_header,
                                  (uint32_t)sizeof(reply_header), 1) ||
        !rin_media_header_valid(&reply_header, RIN_MEDIA_MDC1_OPEN,
                                request_header.request_id,
                                reply_header.session_id) ||
        reply_header.status != 0 || reply_header.session_id == 0u ||
        reply_header.payload_bytes != sizeof(metadata) ||
        !rin_media_transfer_exact(socket_fd, &metadata,
                                  (uint32_t)sizeof(metadata), 1) ||
        metadata.session_id != reply_header.session_id ||
        metadata.duration_ms < 0 || metadata.reserved0 != 0u ||
        metadata.reserved1 != 0u ||
        metadata.output_sample_rate != output.sample_rate ||
        metadata.output_channels != output.channels ||
        metadata.sample_format != RIN_MEDIA_MDC1_SAMPLE_FORMAT_S16LE ||
        metadata.title_bytes > RIN_MEDIA_MDC1_TEXT_CAPACITY ||
        metadata.artist_bytes > RIN_MEDIA_MDC1_TEXT_CAPACITY ||
        metadata.album_bytes > RIN_MEDIA_MDC1_TEXT_CAPACITY) {
        (void)close(socket_fd);
        rin_media_session_discard_reserved(session);
        return RIN_ERROR_IO;
    }
    for (index = 0u; index < RIN_MEDIA_MDC1_TEXT_CAPACITY; ++index) {
        if ((index >= metadata.title_bytes && metadata.title[index] != '\0') ||
            (index >= metadata.artist_bytes && metadata.artist[index] != '\0') ||
            (index >= metadata.album_bytes && metadata.album[index] != '\0')) {
            (void)close(socket_fd);
            rin_media_session_discard_reserved(session);
            return RIN_ERROR_INTEGRITY;
        }
    }

    session->object = object;
    session->socket_fd = socket_fd;
    session->next_request_id = 2u;
    session->sample_rate = output.sample_rate;
    session->channels = output.channels;
    session->media_type = RIN_MEDIA_TYPE_AUDIO;
    session->session_id = reply_header.session_id;
    session->owner_pid = owner_pid;
    __atomic_store_n(&session->state, 1u, __ATOMIC_RELEASE);
    return RIN_SUCCESS;
}

RinResult rin_media_sdk_client_audio_info_v1(RinObject object,
                                              RinAudioInfoV1* info)
{
    RinMediaSdkSessionV1* session;
    RinResult result;
    if (info == NULL) return RIN_ERROR_INVALID_ARGUMENT;
    result = rin_media_validate_object(object);
    if (result != RIN_SUCCESS) return result;
    session = rin_media_session_find_locked(object);
    if (session == NULL) return RIN_ERROR_STALE_HANDLE;
    if (__atomic_load_n(&session->state, __ATOMIC_ACQUIRE) != 1u ||
        session->object != object ||
        session->media_type != RIN_MEDIA_TYPE_AUDIO) {
        const RinResult invalid_result =
            session->object == object &&
                    session->media_type != RIN_MEDIA_TYPE_AUDIO
                ? RIN_ERROR_WRONG_TYPE
                : RIN_ERROR_STALE_HANDLE;
        rin_media_session_unlock(session);
        return invalid_result;
    }
    if (!rin_media_session_pid_valid(session)) {
        rin_media_session_drop_locked(session, 0);
        rin_media_session_unlock_free(session);
        return RIN_ERROR_STALE_HANDLE;
    }
    memset(info, 0, sizeof(*info));
    info->struct_size = sizeof(*info);
    info->version = RIN_SDK_STRUCT_VERSION_1;
    info->sample_rate_hz = session->sample_rate;
    info->channels = session->channels;
    info->sample_format = RIN_MEDIA_AUDIO_SAMPLE_FORMAT_S16LE;
    info->bytes_per_frame = session->channels * 2u;
    /* MDC1 v2 reports duration but not an exact decoded frame count. */
    info->frame_count = 0u;
    info->flags = 0u;
    rin_media_session_unlock(session);
    return RIN_SUCCESS;
}

RinResult rin_media_sdk_client_image_info_v1(RinObject object,
                                              RinImageInfoV1* info)
{
    RinMediaSdkSessionV1* session;
    RinResult result;

    if (info == NULL) return RIN_ERROR_INVALID_ARGUMENT;
    result = rin_media_validate_object(object);
    if (result != RIN_SUCCESS) return result;
    session = rin_media_session_find_locked(object);
    if (session == NULL) return RIN_ERROR_STALE_HANDLE;
    if (__atomic_load_n(&session->state, __ATOMIC_ACQUIRE) != 1u ||
        session->object != object) {
        rin_media_session_unlock(session);
        return RIN_ERROR_STALE_HANDLE;
    }
    if (session->media_type != RIN_MEDIA_TYPE_IMAGE) {
        rin_media_session_unlock(session);
        return RIN_ERROR_WRONG_TYPE;
    }
    if (!rin_media_session_pid_valid(session)) {
        rin_media_session_drop_locked(session, 0);
        rin_media_session_unlock_free(session);
        return RIN_ERROR_STALE_HANDLE;
    }
    result = rin_media_image_decode_locked(session);
    if (result != RIN_SUCCESS) {
        if (result == RIN_ERROR_IO || result == RIN_ERROR_INTEGRITY ||
            result == RIN_ERROR_STALE_HANDLE)
            rin_media_session_drop_locked(session, 0);
        if (__atomic_load_n(&session->state, __ATOMIC_ACQUIRE) == 2u)
            rin_media_session_unlock_free(session);
        else
            rin_media_session_unlock(session);
        return result;
    }
    memset(info, 0, sizeof(*info));
    info->struct_size = sizeof(*info);
    info->version = RIN_SDK_STRUCT_VERSION_1;
    info->width = session->image_info.width;
    info->height = session->image_info.height;
    info->format = RIN_IMAGE_DECODE_FORMAT_ARGB8888;
    info->row_stride_bytes = session->image_info.stride_bytes;
    /* The internal first-frame wire response does not carry total animation
     * frame count, so report unknown instead of claiming every source is still. */
    info->frame_count = 0u;
    info->decoded_size_bytes = session->image_info.pixel_bytes;
    rin_media_session_unlock(session);
    return RIN_SUCCESS;
}

RinResult rin_media_sdk_client_image_decode_v1(RinObject object,
                                                uint32_t format,
                                                RinSliceV1 output,
                                                uint64_t* required)
{
    RinMediaSdkSessionV1* session;
    RinResult result;
    uint64_t needed;

    if (required != NULL) *required = 0u;
    if (required == NULL ||
        !rin_media_range_valid(output.address, output.size, UINT64_MAX, 1))
        return RIN_ERROR_INVALID_ARGUMENT;
    if (format != RIN_IMAGE_DECODE_FORMAT_ARGB8888)
        return RIN_ERROR_NOT_SUPPORTED;
    result = rin_media_validate_object(object);
    if (result != RIN_SUCCESS) return result;
    session = rin_media_session_find_locked(object);
    if (session == NULL) return RIN_ERROR_STALE_HANDLE;
    if (__atomic_load_n(&session->state, __ATOMIC_ACQUIRE) != 1u ||
        session->object != object) {
        rin_media_session_unlock(session);
        return RIN_ERROR_STALE_HANDLE;
    }
    if (session->media_type != RIN_MEDIA_TYPE_IMAGE) {
        rin_media_session_unlock(session);
        return RIN_ERROR_WRONG_TYPE;
    }
    if (!rin_media_session_pid_valid(session)) {
        rin_media_session_drop_locked(session, 0);
        rin_media_session_unlock_free(session);
        return RIN_ERROR_STALE_HANDLE;
    }
    result = rin_media_image_decode_locked(session);
    if (result != RIN_SUCCESS) {
        if (result == RIN_ERROR_IO || result == RIN_ERROR_INTEGRITY ||
            result == RIN_ERROR_STALE_HANDLE)
            rin_media_session_drop_locked(session, 0);
        if (__atomic_load_n(&session->state, __ATOMIC_ACQUIRE) == 2u)
            rin_media_session_unlock_free(session);
        else
            rin_media_session_unlock(session);
        return result;
    }
    needed = session->image_info.pixel_bytes;
    *required = needed;
    if (output.size < needed) {
        rin_media_session_unlock(session);
        return RIN_ERROR_BUFFER_TOO_SMALL;
    }
    if (!rin_media_range_valid(output.address, needed, UINT64_MAX, 0)) {
        rin_media_session_unlock(session);
        return RIN_ERROR_INVALID_ARGUMENT;
    }
    memset((void*)(uintptr_t)output.address, 0, (size_t)needed);
    result = rin_media_image_read_pixels_locked(
        session, (uint8_t*)(uintptr_t)output.address);
    if (result != RIN_SUCCESS) {
        memset((void*)(uintptr_t)output.address, 0, (size_t)needed);
        if (result == RIN_ERROR_IO || result == RIN_ERROR_INTEGRITY ||
            result == RIN_ERROR_STALE_HANDLE)
            rin_media_session_drop_locked(session, 0);
        if (__atomic_load_n(&session->state, __ATOMIC_ACQUIRE) == 2u)
            rin_media_session_unlock_free(session);
        else
            rin_media_session_unlock(session);
        return result;
    }
    rin_media_session_unlock(session);
    return RIN_SUCCESS;
}

RinResult rin_media_sdk_client_audio_decode_v1(RinObject object,
                                                RinSliceV1 output,
                                                uint64_t* frames_out)
{
    RinMediaSdkSessionV1* session;
    RinMediaMdc1ReadRequestV1 request;
    RinMediaMdc1ReadReplyV1 response;
    RinMediaMdc1HeaderV1 reply;
    uint64_t maximum_frames;
    uint64_t maximum_bytes;
    uint32_t frame_bytes;
    RinResult result = RIN_ERROR_IO;

    if (frames_out != NULL) *frames_out = 0u;
    if (frames_out == NULL ||
        !rin_media_range_valid(output.address, output.size, UINT64_MAX, 1))
        return RIN_ERROR_INVALID_ARGUMENT;
    result = rin_media_validate_object(object);
    if (result != RIN_SUCCESS) return result;
    session = rin_media_session_find_locked(object);
    if (session == NULL) return RIN_ERROR_STALE_HANDLE;
    if (__atomic_load_n(&session->state, __ATOMIC_ACQUIRE) != 1u ||
        session->object != object ||
        session->media_type != RIN_MEDIA_TYPE_AUDIO) {
        const RinResult invalid_result =
            session->object == object &&
                    session->media_type != RIN_MEDIA_TYPE_AUDIO
                ? RIN_ERROR_WRONG_TYPE
                : RIN_ERROR_STALE_HANDLE;
        rin_media_session_unlock(session);
        return invalid_result;
    }
    if (!rin_media_session_pid_valid(session)) {
        rin_media_session_drop_locked(session, 0);
        rin_media_session_unlock_free(session);
        return RIN_ERROR_STALE_HANDLE;
    }
    if (session->channels == 0u || session->channels > 8u ||
        session->channels > UINT32_MAX / 2u) {
        rin_media_session_unlock(session);
        return RIN_ERROR_INTEGRITY;
    }
    frame_bytes = session->channels * 2u;
    maximum_frames = output.size / frame_bytes;
    if (maximum_frames > RIN_MEDIA_MDC1_MAX_FRAMES)
        maximum_frames = RIN_MEDIA_MDC1_MAX_FRAMES;
    if (maximum_frames == 0u) {
        rin_media_session_unlock(session);
        return output.size == 0u ? RIN_SUCCESS : RIN_ERROR_BUFFER_TOO_SMALL;
    }
    maximum_bytes = maximum_frames * frame_bytes;
    memset((void*)(uintptr_t)output.address, 0, (size_t)maximum_bytes);
    memset(&request, 0, sizeof(request));
    request.maximum_frames = (uint32_t)maximum_frames;
    memset(&reply, 0, sizeof(reply));
    memset(&response, 0, sizeof(response));
    if (!rin_media_rpc(session, RIN_MEDIA_MDC1_READ, &request,
                       sizeof(request), &reply) || reply.status != 0 ||
        reply.payload_bytes < sizeof(response) ||
        reply.payload_bytes > sizeof(response) + maximum_bytes ||
        !rin_media_transfer_exact(session->socket_fd, &response,
                                  (uint32_t)sizeof(response), 1) ||
        response.reserved0 != 0u || response.frames > maximum_frames ||
        reply.payload_bytes != sizeof(response) +
            (uint64_t)response.frames * frame_bytes ||
        (response.frames != 0u &&
         !rin_media_transfer_exact(session->socket_fd,
                                   (void*)(uintptr_t)output.address,
                                   response.frames * frame_bytes, 1))) {
        memset((void*)(uintptr_t)output.address, 0, (size_t)maximum_bytes);
        rin_media_session_drop_locked(session, 0);
        rin_media_session_unlock_free(session);
        return RIN_ERROR_IO;
    }
    *frames_out = response.frames;
    result = RIN_SUCCESS;
    rin_media_session_unlock(session);
    return result;
}

void rin_media_sdk_client_object_close_hook_v1(RinObject object)
{
    RinMediaSdkSessionV1* session = rin_media_session_find_locked(object);
    if (session == NULL) return;
    if (__atomic_load_n(&session->state, __ATOMIC_ACQUIRE) == 1u &&
        session->object == object)
        rin_media_session_drop_locked(
            session, rin_media_current_pid() == session->owner_pid);
    else {
        rin_media_session_unlock(session);
        return;
    }
    rin_media_session_unlock_free(session);
}
