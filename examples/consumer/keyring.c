/* SPDX-License-Identifier: MIT */
/* Read-only SDK consumer for the authenticated Keyring service status ABI. */
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>

#include <rin/keyring.h>
#include <rin/net/socket_abi.h>
#include <rin/service_directory.h>

#define KEYRING_IO_INTERRUPTION_LIMIT 32u

static int transfer_exact(int descriptor, void* buffer, uint32_t size,
                          int sending)
{
    uint8_t* bytes = (uint8_t*)buffer;
    uint32_t offset = 0u;
    uint32_t interrupted = 0u;

    while (offset < size) {
        ssize_t count = sending
            ? send(descriptor, bytes + offset, size - offset, MSG_NOSIGNAL)
            : recv(descriptor, bytes + offset, size - offset, 0);
        if (count > 0 && (uint64_t)count <= (uint64_t)(size - offset)) {
            offset += (uint32_t)count;
            interrupted = 0u;
            continue;
        }
        if (count < 0 && errno == EINTR &&
            ++interrupted < KEYRING_IO_INTERRUPTION_LIMIT)
            continue;
        return -1;
    }
    return 0;
}

static int keyring_identity_matches(
    const rin_unix_service_identity_v1* identity,
    uint32_t expected_slot_id)
{
    return identity != NULL && expected_slot_id != 0u &&
           identity->slot_id == expected_slot_id &&
           identity->owner_uid == RIN_KEYRING_SERVICE_OWNER_UID &&
           identity->scope == RIN_KEYRING_SERVICE_SCOPE &&
           identity->flags == RIN_UNIX_SERVICE_IDENTITY_FLAG_PUBLISHED;
}

static int known_keyring_status(int status)
{
    return status <= RIN_KEYRING_OK &&
           status >= RIN_KEYRING_BAD_BINDINGS;
}

static int query_keyring_status(void)
{
    static const char service_path[] = RIN_KEYRING_SOCKET_PATH;
    RinKeyringMessageHeaderV1 request;
    RinKeyringMessageHeaderV1 response;
    rin_unix_service_identity_v1 identity;
    struct sockaddr_un address;
    struct timeval timeout = {1, 0};
    socklen_t identity_size = (socklen_t)sizeof(identity);
    uint32_t expected_slot_id = 0u;
    int descriptor = -1;
    int result = RIN_KEYRING_STORAGE_FAILED;

    memset(&request, 0, sizeof(request));
    memset(&response, 0, sizeof(response));
    memset(&identity, 0, sizeof(identity));
    memset(&address, 0, sizeof(address));
    if (sizeof(service_path) > sizeof(address.sun_path) ||
        rin_service_find_system_slot(RIN_KEYRING_SERVICE_ID,
                                     &expected_slot_id) != 0 ||
        expected_slot_id == 0u)
        return RIN_KEYRING_LOCKED;

    descriptor = socket(AF_UNIX, SOCK_STREAM, 0);
    if (descriptor < 0) return RIN_KEYRING_LOCKED;
    address.sun_family = AF_UNIX;
    memcpy(address.sun_path, service_path, sizeof(service_path));
    if (setsockopt(descriptor, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                   (socklen_t)sizeof(timeout)) != 0 ||
        setsockopt(descriptor, SOL_SOCKET, SO_SNDTIMEO, &timeout,
                   (socklen_t)sizeof(timeout)) != 0 ||
        connect(descriptor, (const struct sockaddr*)&address,
                (socklen_t)sizeof(address)) != 0 ||
        getsockopt(descriptor, SOL_SOCKET, SO_RIN_UNIX_SERVICE_IDENTITY,
                   &identity, &identity_size) != 0 ||
        identity_size != (socklen_t)sizeof(identity) ||
        !keyring_identity_matches(&identity, expected_slot_id)) {
        (void)close(descriptor);
        return RIN_KEYRING_LOCKED;
    }

    request.struct_size = (uint32_t)sizeof(request);
    request.version = RIN_KEYRING_MESSAGE_VERSION;
    request.opcode = RIN_KEYRING_OP_STATUS;
    request.request_id = 1u;
    if (transfer_exact(descriptor, &request, (uint32_t)sizeof(request), 1) ==
            0 &&
        transfer_exact(descriptor, &response, (uint32_t)sizeof(response), 0) ==
            0 &&
        response.struct_size == (uint32_t)sizeof(response) &&
        response.version == RIN_KEYRING_MESSAGE_VERSION &&
        response.opcode == RIN_KEYRING_OP_STATUS &&
        response.request_id == request.request_id &&
        response.payload_size == 0u && response.flags == 0u &&
        response.reserved == 0u && known_keyring_status(response.status))
        result = response.status;

    if (close(descriptor) != 0 && result == RIN_KEYRING_OK)
        result = RIN_KEYRING_STORAGE_FAILED;
    return result;
}

int main(void)
{
    const int status = query_keyring_status();
    if (status == RIN_KEYRING_OK) {
        puts("Authenticated Keyring service is ready.");
        return 0;
    }
    printf("Keyring status unavailable or rejected (status=%d).\n", status);
    return 1;
}
