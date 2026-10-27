/* SPDX-License-Identifier: MIT */
/* Minimal signed user-service example for the public IME Provider wire ABI. */
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <rin/gui/text_input_provider.h>
#include <rin/net/socket_abi.h>
#include <rin/runtime.h>
#include <rin/service_directory.h>

#define IME_PROVIDER_SOCKET_PREFIX "/run/rin/ime-"
#define IME_PROVIDER_POLL_SLICE_MS 100
#define IME_PROVIDER_INTERRUPTION_LIMIT 64u

typedef union ImeProviderMessageBuffer {
    uint64_t alignment;
    uint8_t bytes[RIN_IME_WIRE_MAX_MESSAGE_BYTES];
    RinImeWireHeaderV1 header;
    RinImeProviderHelloV1 hello;
    RinImeProviderContextV1 context;
    RinImeProviderContextV2 context_v2;
    RinImeKeyEventV1 key;
    RinImeCandidateSelectionV1 selection;
    RinImeCaretRectV1 caret;
    RinImeSurroundingTextV1 surrounding;
    RinImeControlV1 control;
    RinImeReplyV1 reply;
} ImeProviderMessageBuffer;

typedef struct ImeProviderState {
    RinSessionIdentityV1 session;
    uint64_t context_generation;
    uint64_t candidate_generation;
    uint64_t next_commit_id;
    char socket_path[64];
    int identity_socket;
} ImeProviderState;

static ImeProviderState g_state;

static int service_stopping(void)
{
    return rin_service_should_stop() != 0;
}

static int wait_for(int descriptor, short events)
{
    struct pollfd item;
    memset(&item, 0, sizeof(item));
    item.fd = descriptor;
    item.events = events;
    for (;;) {
        int result;
        if (service_stopping()) return -1;
        item.revents = 0;
        result = poll(&item, 1u, IME_PROVIDER_POLL_SLICE_MS);
        if (result < 0 && errno == EINTR) continue;
        if (result < 0) return -1;
        if (result == 0) continue;
        if ((item.revents & (POLLERR | POLLNVAL)) != 0) return -1;
        if ((item.revents & (events | POLLHUP)) != 0) return 0;
    }
}

static int transfer_exact(int descriptor, const void *buffer, uint32_t size,
                          int sending)
{
    uint8_t *read_bytes = (uint8_t *)buffer;
    const uint8_t *write_bytes = (const uint8_t *)buffer;
    uint32_t offset = 0u;
    uint32_t interruptions = 0u;
    while (offset < size) {
        ssize_t count;
        if (wait_for(descriptor, sending ? POLLOUT : POLLIN) != 0)
            return -1;
        count = sending
            ? send(descriptor, write_bytes + offset, size - offset,
                   MSG_DONTWAIT | MSG_NOSIGNAL)
            : recv(descriptor, read_bytes + offset, size - offset,
                   MSG_DONTWAIT);
        if (count > 0 && (uint64_t)count <= (uint64_t)(size - offset)) {
            offset += (uint32_t)count;
            interruptions = 0u;
            continue;
        }
        if (count < 0 && (errno == EINTR || errno == EAGAIN ||
                          errno == EWOULDBLOCK)) {
            if (++interruptions < IME_PROVIDER_INTERRUPTION_LIMIT) continue;
        }
        return -1;
    }
    return 0;
}

static uint32_t message_size(uint32_t type)
{
    switch (type) {
    case RIN_IME_WIRE_HELLO_V1:
    case RIN_IME_WIRE_HELLO_ACK_V1:
        return (uint32_t)sizeof(RinImeProviderHelloV1);
    case RIN_IME_WIRE_CONTEXT_V1:
        return (uint32_t)sizeof(RinImeProviderContextV1);
    case RIN_IME_WIRE_CONTEXT_V2:
        return (uint32_t)sizeof(RinImeProviderContextV2);
    case RIN_IME_WIRE_KEY_EVENT_V1:
        return (uint32_t)sizeof(RinImeKeyEventV1);
    case RIN_IME_WIRE_PREEDIT_V1:
        return (uint32_t)sizeof(RinImePreeditV1);
    case RIN_IME_WIRE_COMMIT_V1:
        return (uint32_t)sizeof(RinImeCommitV1);
    case RIN_IME_WIRE_CANDIDATES_V1:
        return (uint32_t)sizeof(RinImeCandidatesV1);
    case RIN_IME_WIRE_CANDIDATE_SELECTION_V1:
        return (uint32_t)sizeof(RinImeCandidateSelectionV1);
    case RIN_IME_WIRE_CARET_RECT_V1:
        return (uint32_t)sizeof(RinImeCaretRectV1);
    case RIN_IME_WIRE_SURROUNDING_TEXT_V1:
        return (uint32_t)sizeof(RinImeSurroundingTextV1);
    case RIN_IME_WIRE_CANCEL_V1:
    case RIN_IME_WIRE_RESET_V1:
        return (uint32_t)sizeof(RinImeControlV1);
    case RIN_IME_WIRE_REPLY_V1:
        return (uint32_t)sizeof(RinImeReplyV1);
    default:
        return 0u;
    }
}

static int read_frame(int descriptor, ImeProviderMessageBuffer *buffer,
                      uint32_t *size_out)
{
    uint32_t size;
    if (buffer == NULL || size_out == NULL ||
        transfer_exact(descriptor, buffer->bytes,
                       (uint32_t)sizeof(RinImeWireHeaderV1), 0) != 0)
        return -1;
    size = buffer->header.message_bytes;
    if (size < (uint32_t)sizeof(RinImeWireHeaderV1) ||
        size > RIN_IME_WIRE_MAX_MESSAGE_BYTES ||
        size != message_size(buffer->header.message_type))
        return -1;
    if (size > (uint32_t)sizeof(RinImeWireHeaderV1) &&
        transfer_exact(descriptor,
                       buffer->bytes + sizeof(RinImeWireHeaderV1),
                       size - (uint32_t)sizeof(RinImeWireHeaderV1), 0) != 0)
        return -1;
    if (!rin_ime_wire_payload_v1(buffer->bytes, size)) return -1;
    *size_out = size;
    return 0;
}

static void init_header(RinImeWireHeaderV1 *header, uint32_t type,
                        uint32_t size, uint64_t request_id,
                        uint64_t context_generation)
{
    memset(header, 0, sizeof(*header));
    header->magic = RIN_IME_WIRE_MAGIC;
    header->version_major = RIN_IME_WIRE_VERSION_MAJOR;
    header->version_minor = RIN_IME_WIRE_VERSION_MINOR;
    header->message_type = type;
    header->header_bytes = (uint32_t)sizeof(*header);
    header->message_bytes = size;
    header->request_id = request_id;
    header->context_generation = context_generation;
}

static int write_frame(int descriptor, const void *message, uint32_t size)
{
    if (!rin_ime_wire_payload_v1(message, size)) return -1;
    return transfer_exact(descriptor, message, size, 1);
}

static int send_reply(int descriptor, uint64_t request_id,
                      uint64_t context_generation, uint32_t status)
{
    RinImeReplyV1 reply;
    memset(&reply, 0, sizeof(reply));
    init_header(&reply.header, RIN_IME_WIRE_REPLY_V1, sizeof(reply),
                request_id, context_generation);
    reply.status = status;
    return write_frame(descriptor, &reply, sizeof(reply));
}

static int send_empty_preedit(int descriptor, uint64_t request_id,
                              uint64_t context_generation)
{
    RinImePreeditV1 preedit;
    memset(&preedit, 0, sizeof(preedit));
    init_header(&preedit.header, RIN_IME_WIRE_PREEDIT_V1, sizeof(preedit),
                request_id, context_generation);
    return write_frame(descriptor, &preedit, sizeof(preedit));
}

static int send_empty_candidates(int descriptor, uint64_t request_id,
                                 uint64_t context_generation)
{
    RinImeCandidatesV1 candidates;
    memset(&candidates, 0, sizeof(candidates));
    init_header(&candidates.header, RIN_IME_WIRE_CANDIDATES_V1,
                sizeof(candidates), request_id, context_generation);
    candidates.selected_index = RIN_IME_NO_CANDIDATE;
    if (++g_state.candidate_generation == 0u)
        g_state.candidate_generation = 1u;
    candidates.candidate_generation = g_state.candidate_generation;
    return write_frame(descriptor, &candidates, sizeof(candidates));
}

static int send_state(int descriptor, uint64_t request_id,
                      uint64_t context_generation)
{
    if (send_empty_preedit(descriptor, request_id, context_generation) != 0)
        return -1;
    return send_empty_candidates(descriptor, request_id, context_generation);
}

static int encode_utf8(uint32_t codepoint, uint8_t text[4], uint32_t *size_out)
{
    if (text == NULL || size_out == NULL || codepoint == 0u ||
        codepoint > UINT32_C(0x10ffff) ||
        (codepoint >= UINT32_C(0xd800) && codepoint <= UINT32_C(0xdfff)))
        return -1;
    if (codepoint <= 0x7fu) {
        text[0] = (uint8_t)codepoint;
        *size_out = 1u;
    } else if (codepoint <= 0x7ffu) {
        text[0] = (uint8_t)(0xc0u | (codepoint >> 6u));
        text[1] = (uint8_t)(0x80u | (codepoint & 0x3fu));
        *size_out = 2u;
    } else if (codepoint <= 0xffffu) {
        text[0] = (uint8_t)(0xe0u | (codepoint >> 12u));
        text[1] = (uint8_t)(0x80u | ((codepoint >> 6u) & 0x3fu));
        text[2] = (uint8_t)(0x80u | (codepoint & 0x3fu));
        *size_out = 3u;
    } else {
        text[0] = (uint8_t)(0xf0u | (codepoint >> 18u));
        text[1] = (uint8_t)(0x80u | ((codepoint >> 12u) & 0x3fu));
        text[2] = (uint8_t)(0x80u | ((codepoint >> 6u) & 0x3fu));
        text[3] = (uint8_t)(0x80u | (codepoint & 0x3fu));
        *size_out = 4u;
    }
    return 0;
}

static int send_passthrough_commit(int descriptor, uint64_t context_generation,
                                   uint32_t codepoint)
{
    RinImeCommitV1 commit;
    ImeProviderMessageBuffer buffer;
    uint32_t size = 0u;
    uint64_t delivery_id;
    uint32_t text_bytes = 0u;
    if (g_state.next_commit_id == UINT64_MAX) return -1;
    delivery_id = g_state.next_commit_id++;
    memset(&commit, 0, sizeof(commit));
    if (encode_utf8(codepoint, commit.text, &text_bytes) != 0) return -1;
    init_header(&commit.header, RIN_IME_WIRE_COMMIT_V1, sizeof(commit),
                delivery_id, context_generation);
    commit.text_bytes = text_bytes;
    if (write_frame(descriptor, &commit, sizeof(commit)) != 0 ||
        read_frame(descriptor, &buffer, &size) != 0 ||
        size != sizeof(RinImeReplyV1) ||
        buffer.header.message_type != RIN_IME_WIRE_REPLY_V1 ||
        buffer.reply.header.request_id != delivery_id ||
        buffer.reply.header.context_generation != context_generation ||
        buffer.reply.status != RIN_IME_REPLY_OK)
        return -1;
    return 0;
}

static int authenticate_compositor(int descriptor)
{
    rin_unix_peer_service_identity_v1 service;
    rin_unix_peer_app_identity_v1 app;
    socklen_t service_size = (socklen_t)sizeof(service);
    socklen_t app_size = (socklen_t)sizeof(app);
    uint32_t compositor_slot = 0u;
    memset(&service, 0, sizeof(service));
    memset(&app, 0, sizeof(app));
    if (rin_service_find_system_slot("compositor", &compositor_slot) != 0 ||
        compositor_slot == 0u ||
        getsockopt(descriptor, SOL_SOCKET,
                   SO_RIN_UNIX_PEER_SERVICE_IDENTITY, &service,
                   &service_size) != 0 || service_size != sizeof(service) ||
        !rin_unix_peer_service_identity_valid(&service) ||
        service.scope != RIN_UNIX_SCOPE_SYSTEM || service.owner_uid != 0u ||
        service.service_slot_id != compositor_slot ||
        strcmp(service.service_id, "compositor") != 0 ||
        getsockopt(descriptor, SOL_SOCKET, SO_RIN_UNIX_PEER_APP_IDENTITY,
                   &app, &app_size) != 0 || app_size != sizeof(app) ||
        !rin_unix_peer_app_identity_valid(&app) ||
        app.scope != RIN_UNIX_SCOPE_SYSTEM || app.owner_uid != 0u ||
        app.process_id != service.process_id ||
        app.process_instance_cookie != service.process_instance_cookie)
        return -1;
    return 0;
}

static int negotiate(int descriptor)
{
    ImeProviderMessageBuffer buffer;
    uint32_t size = 0u;
    RinImeProviderHelloV1 ack;
    const uint64_t supported = RIN_IME_FEATURE_PREEDIT |
                               RIN_IME_FEATURE_CANDIDATES |
                               RIN_IME_FEATURE_CANCEL_RESET |
                               RIN_IME_FEATURE_ASYNC_KEY_RELEASES |
                               RIN_IME_FEATURE_SURFACE_CONTEXT;
    if (read_frame(descriptor, &buffer, &size) != 0 ||
        size != sizeof(RinImeProviderHelloV1) ||
        buffer.header.message_type != RIN_IME_WIRE_HELLO_V1 ||
        (buffer.hello.required_features & supported) !=
            buffer.hello.required_features ||
        buffer.hello.max_message_bytes < sizeof(RinImeCandidatesV1))
        return -1;
    memset(&ack, 0, sizeof(ack));
    init_header(&ack.header, RIN_IME_WIRE_HELLO_ACK_V1, sizeof(ack),
                buffer.hello.header.request_id, 0u);
    ack.min_version = (RIN_IME_WIRE_VERSION_MAJOR << 16) |
                      RIN_IME_WIRE_VERSION_MINOR;
    ack.max_version = ack.min_version;
    ack.features = buffer.hello.features & supported;
    ack.required_features = buffer.hello.required_features;
    ack.max_message_bytes = buffer.hello.max_message_bytes;
    ack.request_timeout_ms = buffer.hello.request_timeout_ms;
    if (ack.max_message_bytes < sizeof(RinImeCandidatesV1))
        ack.max_message_bytes = sizeof(RinImeCandidatesV1);
    if (ack.max_message_bytes > RIN_IME_WIRE_MAX_MESSAGE_BYTES)
        ack.max_message_bytes = RIN_IME_WIRE_MAX_MESSAGE_BYTES;
    return write_frame(descriptor, &ack, sizeof(ack));
}

static int handle_key(int descriptor, const RinImeKeyEventV1 *key)
{
    uint64_t request_id = key->header.request_id;
    uint64_t generation = key->header.context_generation;
    if (key->state == RIN_IME_KEY_RELEASED &&
        key->header.flags == RIN_IME_WIRE_FLAG_NO_REPLY)
        return 0;
    if (key->state != RIN_IME_KEY_PRESSED ||
        generation != g_state.context_generation)
        return send_reply(descriptor, request_id, generation,
                          RIN_IME_REPLY_REJECTED);
    if (send_reply(descriptor, request_id, generation, RIN_IME_REPLY_OK) != 0 ||
        send_empty_preedit(descriptor, request_id, generation) != 0)
        return -1;
    if (key->unicode_scalar >= 0x20u && key->unicode_scalar != 0x7fu &&
        (key->modifiers & (RIN_IME_MODIFIER_CONTROL | RIN_IME_MODIFIER_ALT |
                           RIN_IME_MODIFIER_SUPER)) == 0u &&
        send_passthrough_commit(descriptor, generation,
                                key->unicode_scalar) != 0)
        return -1;
    /* The compositor expects candidates after any commit acknowledgement. */
    return send_empty_candidates(descriptor, request_id, generation);
}

static int handle_frame(int descriptor, ImeProviderMessageBuffer *buffer)
{
    const uint64_t request_id = buffer->header.request_id;
    const uint64_t generation = buffer->header.context_generation;
    switch (buffer->header.message_type) {
    case RIN_IME_WIRE_CONTEXT_V1:
        g_state.context_generation = generation;
        return send_reply(descriptor, request_id, generation, RIN_IME_REPLY_OK) != 0
            ? -1 : send_state(descriptor, request_id, generation);
    case RIN_IME_WIRE_CONTEXT_V2:
        g_state.context_generation = generation;
        return send_reply(descriptor, request_id, generation, RIN_IME_REPLY_OK) != 0
            ? -1 : send_state(descriptor, request_id, generation);
    case RIN_IME_WIRE_KEY_EVENT_V1:
        return handle_key(descriptor, &buffer->key);
    case RIN_IME_WIRE_CANDIDATE_SELECTION_V1:
        return send_reply(descriptor, request_id, generation,
                          RIN_IME_REPLY_REJECTED);
    case RIN_IME_WIRE_CANCEL_V1:
    case RIN_IME_WIRE_RESET_V1:
        if (generation != g_state.context_generation)
            return send_reply(descriptor, request_id, generation,
                              RIN_IME_REPLY_REJECTED);
        return send_reply(descriptor, request_id, generation, RIN_IME_REPLY_OK) != 0
            ? -1 : send_state(descriptor, request_id, generation);
    case RIN_IME_WIRE_CARET_RECT_V1:
    case RIN_IME_WIRE_SURROUNDING_TEXT_V1:
        return send_reply(descriptor, request_id, generation,
                          RIN_IME_REPLY_UNSUPPORTED);
    default:
        return -1;
    }
}

static int identify_session(void)
{
    int pair[2] = {-1, -1};
    socklen_t identity_size = (socklen_t)sizeof(g_state.session);
    memset(&g_state.session, 0, sizeof(g_state.session));
    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, pair) != 0)
        return -1;
    if (getsockopt(pair[0], SOL_SOCKET, SO_RIN_UNIX_PEER_SESSION_IDENTITY,
                   &g_state.session, &identity_size) != 0 ||
        identity_size != sizeof(g_state.session) ||
        !rin_unix_peer_session_identity_valid(&g_state.session) ||
        g_state.session.uid != (uint32_t)getuid()) {
        (void)close(pair[0]);
        (void)close(pair[1]);
        return -1;
    }
    (void)close(pair[1]);
    g_state.identity_socket = pair[0];
    return 0;
}

static int build_socket_path(void)
{
    int count = snprintf(g_state.socket_path, sizeof(g_state.socket_path),
                         IME_PROVIDER_SOCKET_PREFIX "%u.sock",
                         g_state.session.session_id);
    return count > 0 && (size_t)count < sizeof(g_state.socket_path) ? 0 : -1;
}

static int make_listener(void)
{
    struct sockaddr_un address;
    int descriptor;
    memset(&address, 0, sizeof(address));
    if (strlen(g_state.socket_path) >= sizeof(address.sun_path)) return -1;
    descriptor = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
    if (descriptor < 0) return -1;
    address.sun_family = AF_UNIX;
    memcpy(address.sun_path, g_state.socket_path,
           strlen(g_state.socket_path) + 1u);
    if (bind(descriptor, (struct sockaddr *)&address, sizeof(address)) != 0 ||
        listen(descriptor, 4) != 0) {
        (void)close(descriptor);
        return -1;
    }
    return descriptor;
}

static int serve_connection(int descriptor)
{
    ImeProviderMessageBuffer buffer;
    uint32_t size = 0u;
    if (authenticate_compositor(descriptor) != 0 || negotiate(descriptor) != 0)
        return -1;
    while (!service_stopping()) {
        if (read_frame(descriptor, &buffer, &size) != 0 ||
            !rin_ime_wire_payload_v1(buffer.bytes, size) ||
            handle_frame(descriptor, &buffer) != 0)
            return -1;
        if (rin_service_health(0u) != 0) return -1;
    }
    return 0;
}

int main(void)
{
    int listener = -1;
    int result = 1;
    memset(&g_state, 0, sizeof(g_state));
    g_state.identity_socket = -1;
    g_state.next_commit_id = 1u;
    g_state.candidate_generation = 1u;
    if (identify_session() != 0 || build_socket_path() != 0 ||
        (listener = make_listener()) < 0) {
        puts("IME Provider sample could not initialize its session socket.");
        goto done;
    }
    if (rin_service_health(0u) != 0) goto done;
    puts("IME Provider sample ready; it passes Unicode text through unchanged.");
    while (!service_stopping()) {
        struct pollfd item;
        int ready;
        memset(&item, 0, sizeof(item));
        item.fd = listener;
        item.events = POLLIN;
        ready = poll(&item, 1u, IME_PROVIDER_POLL_SLICE_MS);
        if (ready < 0) {
            if (errno == EINTR) continue;
            break;
        }
        if (ready == 0) continue;
        if ((item.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) break;
        if ((item.revents & POLLIN) != 0) {
            int client = accept(listener, NULL, NULL);
            int client_flags;
            if (client < 0) {
                if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
                    continue;
                break;
            }
            client_flags = fcntl(client, F_GETFL, 0);
            if (client_flags < 0 ||
                fcntl(client, F_SETFD, FD_CLOEXEC) != 0 ||
                fcntl(client, F_SETFL, client_flags | O_NONBLOCK) != 0) {
                (void)close(client);
                continue;
            }
            (void)serve_connection(client);
            (void)close(client);
            if (rin_service_health(0u) != 0) break;
        }
    }
    result = 0;
done:
    if (listener >= 0) (void)close(listener);
    if (g_state.identity_socket >= 0) (void)close(g_state.identity_socket);
    return result;
}
