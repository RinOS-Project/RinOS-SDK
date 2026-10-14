/* SPDX-License-Identifier: MIT */
/* Public v1 control ABI for the authenticated RinShare service. */
#ifndef RIN_SDK_RINSHARE_CONTROL_H
#define RIN_SDK_RINSHARE_CONTROL_H

#include <stddef.h>
#include <stdint.h>

#include "file_portal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_SHARE_CONTROL_VERSION 1u
#define RIN_SHARE_CONTROL_VERSION_2 2u
#define RIN_SHARE_CONTROL_VERSION_3 3u
#define RIN_SHARE_CONTROL_VERSION_4 4u
#define RIN_SHARE_CONTROL_VERSION_5 5u
#define RIN_SHARE_CONTROL_VERSION_6 6u
#define RIN_SHARE_CONTROL_SERVICE_ID "rinshare-control"
#define RIN_SHARE_CONTROL_SOCKET_PATH "/run/rin/rinshare-control.sock"
#define RIN_SHARE_CONTROL_MAGIC UINT32_C(0x31435352) /* RSC1 */

enum RinShareControlOpcode {
    RIN_SHARE_CONTROL_SET_DIRECTORY_GRANT = 1u,
    RIN_SHARE_CONTROL_SET_APPROVAL = 2u,
    RIN_SHARE_CONTROL_REVOKE_DIRECTORY = 3u,
    RIN_SHARE_CONTROL_GET_PENDING_OFFER = 4u,
    RIN_SHARE_CONTROL_CLEAR_PENDING_OFFER = 5u,
    RIN_SHARE_CONTROL_SET_POLICY = 6u,
    RIN_SHARE_CONTROL_GET_POLICY = 7u,
    RIN_SHARE_CONTROL_LIST_DEVICE = 8u,
    RIN_SHARE_CONTROL_GET_HISTORY = 9u,
    RIN_SHARE_CONTROL_LIST_PEERS = 10u,
    RIN_SHARE_CONTROL_SEND_FILES = 11u,
    RIN_SHARE_CONTROL_PROVISION_IDENTITY = 12u
};

enum RinShareControlVisibilityV1 {
    RIN_SHARE_CONTROL_VISIBILITY_OFF = 0u,
    RIN_SHARE_CONTROL_VISIBILITY_LIMITED = 1u,
    RIN_SHARE_CONTROL_VISIBILITY_ALL = 2u
};

enum RinShareControlHistoryResultV1 {
    RIN_SHARE_CONTROL_HISTORY_PENDING = 1,
    RIN_SHARE_CONTROL_HISTORY_APPROVED = 2,
    RIN_SHARE_CONTROL_HISTORY_REJECTED = 3
};

#define RIN_SHARE_CONTROL_MAX_DEVICES 16u
#define RIN_SHARE_CONTROL_MAX_HISTORY 32u
#define RIN_SHARE_CONTROL_MAX_DISCOVERED_PEERS 64u
#define RIN_SHARE_CONTROL_DISCOVERY_FLAG_TRUNCATED UINT32_C(1)
#define RIN_SHARE_CONTROL_MAX_SEND_FILES 32u

typedef struct RinShareControlPolicyV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t visibility;
    uint32_t flags;
    uint64_t expires_at_ms;
    uint64_t generation;
    uint64_t reserved[2];
} RinShareControlPolicyV1;

typedef struct RinShareControlDeviceEntryV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    uint32_t reserved0;
    uint64_t generation;
    uint64_t last_seen_at_ms;
    uint8_t peer_fingerprint[32];
    char display_name[64];
    uint64_t reserved[2];
} RinShareControlDeviceEntryV1;

typedef struct RinShareControlHistoryEntryV1 {
    uint32_t struct_size;
    uint32_t version;
    int32_t result;
    uint32_t reserved0;
    uint64_t recorded_at_ms;
    uint64_t transfer_id;
    uint8_t peer_fingerprint[32];
    char display_name[64];
    uint32_t file_count;
    uint32_t reserved1;
    uint64_t total_bytes;
    uint64_t reserved[2];
} RinShareControlHistoryEntryV1;

typedef struct RinShareControlResponseV2 {
    uint32_t struct_size;
    uint32_t version;
    int32_t result;
    uint32_t reserved0;
    uint64_t generation;
    RinShareControlPolicyV1 policy;
    uint32_t count;
    uint32_t next_cursor;
    uint64_t reserved1;
    RinShareControlDeviceEntryV1 device;
    RinShareControlHistoryEntryV1 history;
    uint64_t reserved[2];
} RinShareControlResponseV2;

/* DNS-SD endpoints remain untrusted until the RinTLS certificate is
 * verified. interface_id and generations bind a link-local connect attempt
 * to the local interface that received the advertisement; they are not
 * authenticated peer identities. */
typedef struct RinShareControlDiscoveredPeerV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t interface_id;
    uint32_t port;
    uint64_t interface_generation;
    uint64_t device_generation;
    uint64_t expires_at_ms;
    uint8_t address[16];
    char instance[64];
    uint64_t reserved[2];
} RinShareControlDiscoveredPeerV1;

typedef struct RinShareControlResponseV3 {
    uint32_t struct_size;
    uint32_t version;
    int32_t result;
    uint32_t reserved0;
    uint64_t generation;
    uint32_t count;
    uint32_t flags;
    uint64_t reserved2;
    RinShareControlDiscoveredPeerV1 peers[
        RIN_SHARE_CONTROL_MAX_DISCOVERED_PEERS];
    uint64_t reserved[2];
} RinShareControlResponseV3;

static inline int rin_share_control_discovery_flags_valid(uint32_t flags)
{
    return (flags & ~RIN_SHARE_CONTROL_DISCOVERY_FLAG_TRUNCATED) == 0u;
}

static inline int rin_share_control_discovered_peer_valid(
    const RinShareControlDiscoveredPeerV1* peer)
{
    uint8_t address_nonzero = 0u;
    size_t index;
    if (peer == NULL || peer->struct_size != sizeof(*peer) ||
        peer->version != RIN_SHARE_CONTROL_VERSION ||
        peer->interface_id == 0u || peer->port == 0u || peer->port > 65535u ||
        peer->interface_generation == 0u ||
        peer->interface_generation == UINT64_MAX ||
        peer->device_generation == 0u ||
        peer->device_generation == UINT64_MAX || peer->expires_at_ms == 0u ||
        peer->expires_at_ms == UINT64_MAX || peer->address[0] != 0xfeu ||
        (peer->address[1] & 0xc0u) != 0x80u || peer->instance[0] == '\0' ||
        peer->instance[sizeof(peer->instance) - 1u] != '\0' ||
        peer->reserved[0] != 0u || peer->reserved[1] != 0u)
        return 0;
    for (index = 0u; index < sizeof(peer->address); ++index)
        address_nonzero |= peer->address[index];
    if (address_nonzero == 0u) return 0;
    for (index = 0u; index + 1u < sizeof(peer->instance) &&
         peer->instance[index] != '\0'; ++index) {
        const unsigned char value = (unsigned char)peer->instance[index];
        if (value < 0x20u || value > 0x7eu) return 0;
    }
    for (; index < sizeof(peer->instance); ++index)
        if (peer->instance[index] != '\0') return 0;
    return 1;
}

static inline int rin_share_control_response_v3_valid(
    const RinShareControlResponseV3* response)
{
    uint32_t index;
    if (response == NULL || response->struct_size != sizeof(*response) ||
        response->version != RIN_SHARE_CONTROL_VERSION_3 ||
        response->result != 0 || response->reserved0 != 0u ||
        response->generation == 0u || response->generation == UINT64_MAX ||
        response->count > RIN_SHARE_CONTROL_MAX_DISCOVERED_PEERS ||
        !rin_share_control_discovery_flags_valid(response->flags) ||
        response->reserved2 != 0u || response->reserved[0] != 0u ||
        response->reserved[1] != 0u)
        return 0;
    for (index = 0u; index < response->count; ++index)
        if (!rin_share_control_discovered_peer_valid(&response->peers[index]))
            return 0;
    for (; index < RIN_SHARE_CONTROL_MAX_DISCOVERED_PEERS; ++index) {
        const uint8_t* bytes = (const uint8_t*)&response->peers[index];
        size_t byte_index;
        for (byte_index = 0u; byte_index < sizeof(response->peers[index]);
             ++byte_index)
            if (bytes[byte_index] != 0u) return 0;
    }
    return 1;
}

/* Version 4 carries the DNS-SD SRV target separately from the untrusted
 * service-instance label. RinTLS hostname verification must use this target,
 * never a display string or a guessed hostname. */
typedef struct __attribute__((packed)) RinShareControlDiscoveredPeerV2 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t interface_id;
    uint32_t port;
    uint64_t interface_generation;
    uint64_t device_generation;
    uint64_t expires_at_ms;
    uint8_t address[16];
    char instance[256];
    char hostname[256];
    uint64_t reserved[2];
} RinShareControlDiscoveredPeerV2;

typedef struct __attribute__((packed)) RinShareControlResponseV4 {
    uint32_t struct_size;
    uint32_t version;
    int32_t result;
    uint32_t reserved0;
    uint64_t generation;
    uint32_t count;
    uint32_t flags;
    uint64_t reserved2;
    RinShareControlDiscoveredPeerV2 peers[
        RIN_SHARE_CONTROL_MAX_DISCOVERED_PEERS];
    uint64_t reserved[2];
} RinShareControlResponseV4;

typedef struct __attribute__((packed)) RinShareControlSendFileV1 {
    RinFilePortalTokenV1 token;
    uint32_t display_name_size;
    uint32_t reserved0;
    char display_name[256];
} RinShareControlSendFileV1;

/* Appends to the fixed RinShareControlMessageV1 header for SEND_FILES. File
 * tokens must be issued by the chooser to the live rinshared process identity
 * on this authenticated control connection. */
typedef struct __attribute__((packed)) RinShareControlSendRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    RinShareControlDiscoveredPeerV2 peer;
    uint32_t file_count;
    uint32_t reserved0;
    RinShareControlSendFileV1 files[RIN_SHARE_CONTROL_MAX_SEND_FILES];
    uint64_t reserved[2];
} RinShareControlSendRequestV1;

/* User-consented first-run provisioning. Each token must be a distinct,
 * read-only File Portal object issued to the exact authenticated rinshared
 * process. The private-key descriptor is forwarded directly to keyringd;
 * this request carries no private-key bytes. */
typedef struct __attribute__((packed)) RinShareControlProvisionIdentityV1 {
    uint32_t struct_size;
    uint32_t version;
    RinFilePortalTokenV1 private_key;
    RinFilePortalTokenV1 certificate_list;
    RinFilePortalTokenV1 trust_bundle;
    uint64_t reserved[2];
} RinShareControlProvisionIdentityV1;

typedef struct __attribute__((packed)) RinShareControlResponseV5 {
    uint32_t struct_size;
    uint32_t version;
    int32_t result;
    uint32_t reserved0;
    uint64_t generation;
    uint64_t reserved[2];
} RinShareControlResponseV5;

static inline int rin_share_control_file_portal_token_valid(
    const RinFilePortalTokenV1* token, uint32_t required_rights)
{
    uint8_t nonce_nonzero = 0u;
    uint8_t tag_nonzero = 0u;
    uint8_t publisher_nonzero = 0u;
    size_t name_length = 0u;
    size_t index;
    if (required_rights == 0u ||
        (required_rights & ~RIN_FILE_PORTAL_RIGHT_ALL) != 0u ||
        token == NULL || token->magic != RIN_FILE_PORTAL_TOKEN_MAGIC ||
        token->version != RIN_FILE_PORTAL_TOKEN_VERSION ||
        token->token_size != RIN_FILE_PORTAL_TOKEN_SIZE ||
        token->rights != required_rights || token->flags != 0u ||
        token->file_object_id == 0u ||
        token->file_object_id == UINT64_MAX ||
        token->not_before_epoch == UINT64_MAX ||
        token->expires_at_epoch == 0u ||
        token->expires_at_epoch == UINT64_MAX ||
        token->expires_at_epoch <= token->not_before_epoch ||
        token->policy_generation == 0u ||
        token->policy_generation == UINT64_MAX ||
        token->identity.publisher_key_generation == 0u)
        return 0;
    for (index = 0u; index < sizeof(token->identity.package_name); ++index) {
        const unsigned char value =
            (unsigned char)token->identity.package_name[index];
        if (value == '\0') break;
        if (value < 0x20u || value > 0x7eu || value == '/' ||
            value == '\\' || value == ':')
            return 0;
        ++name_length;
    }
    if (index == 0u || index == sizeof(token->identity.package_name) ||
        (name_length == 1u && token->identity.package_name[0] == '.') ||
        (name_length == 2u && token->identity.package_name[0] == '.' &&
         token->identity.package_name[1] == '.'))
        return 0;
    for (++index; index < sizeof(token->identity.package_name); ++index)
        if (token->identity.package_name[index] != '\0') return 0;
    for (index = 0u; index < sizeof(token->identity.publisher_key_id); ++index)
        publisher_nonzero |= token->identity.publisher_key_id[index];
    if (publisher_nonzero == 0u) return 0;
    for (index = 0u; index < sizeof(token->reserved); ++index)
        if (token->reserved[index] != 0u) return 0;
    for (index = 0u; index < sizeof(token->nonce); ++index)
        nonce_nonzero |= token->nonce[index];
    for (index = 0u; index < sizeof(token->authentication_tag); ++index)
        tag_nonzero |= token->authentication_tag[index];
    return nonce_nonzero != 0u && tag_nonzero != 0u;
}

/* Version 6 consumes the one-shot File Portal token itself.  It deliberately
 * does not carry a durable-grant ID: the token is bound to the exact live
 * rinshared process and OPEN returns a directory descriptor capability. */
typedef struct __attribute__((packed)) RinShareControlSetDirectoryGrantV1 {
    uint32_t struct_size;
    uint32_t version;
    RinFilePortalTokenV1 token;
    uint64_t reserved[2];
} RinShareControlSetDirectoryGrantV1;

typedef struct __attribute__((packed)) RinShareControlResponseV6 {
    uint32_t struct_size;
    uint32_t version;
    int32_t result;
    uint32_t reserved0;
    uint64_t generation;
    uint64_t reserved[2];
} RinShareControlResponseV6;

static inline int rin_share_control_set_directory_grant_v1_valid(
    const RinShareControlSetDirectoryGrantV1* request)
{
    if (request == NULL || request->struct_size != sizeof(*request) ||
        request->version != 1u || request->reserved[0] != 0u ||
        request->reserved[1] != 0u)
        return 0;
    return rin_share_control_file_portal_token_valid(
        &request->token, RIN_FILE_PORTAL_RIGHT_ALL);
}

static inline int rin_share_control_response_v6_valid(
    const RinShareControlResponseV6* response)
{
    return response != NULL && response->struct_size == sizeof(*response) &&
           response->version == RIN_SHARE_CONTROL_VERSION_6 &&
           response->result == 0 && response->reserved0 == 0u &&
           response->generation != 0u &&
           response->generation != UINT64_MAX &&
           response->reserved[0] == 0u && response->reserved[1] == 0u;
}

static inline int rin_share_control_provision_token_valid(
    const RinFilePortalTokenV1* token)
{
    uint8_t nonce_nonzero = 0u;
    uint8_t tag_nonzero = 0u;
    uint8_t publisher_nonzero = 0u;
    uint8_t name_terminated = 0u;
    size_t index;
    if (token == NULL || token->magic != RIN_FILE_PORTAL_TOKEN_MAGIC ||
        token->version != RIN_FILE_PORTAL_TOKEN_VERSION ||
        token->token_size != RIN_FILE_PORTAL_TOKEN_SIZE ||
        token->rights != RIN_FILE_PORTAL_RIGHT_READ || token->flags != 0u ||
        token->file_object_id == 0u ||
        token->file_object_id == UINT64_MAX ||
        token->not_before_epoch == UINT64_MAX ||
        token->expires_at_epoch == 0u ||
        token->expires_at_epoch == UINT64_MAX ||
        token->expires_at_epoch <= token->not_before_epoch ||
        token->policy_generation == 0u ||
        token->policy_generation == UINT64_MAX ||
        token->identity.publisher_key_generation == 0u ||
        token->identity.package_name[0] == '\0')
        return 0;
    for (index = 0u; index < sizeof(token->identity.package_name); ++index) {
        const unsigned char value =
            (unsigned char)token->identity.package_name[index];
        if (value == '\0') {
            if (index == 0u) return 0;
            name_terminated = 1u;
            for (++index; index < sizeof(token->identity.package_name); ++index)
                if (token->identity.package_name[index] != '\0') return 0;
            break;
        }
        if (value < 0x20u || value == 0x7fu || value == '/' ||
            value == '\\' || value == ':')
            return 0;
    }
    if (name_terminated == 0u) return 0;
    for (index = 0u; index < sizeof(token->identity.publisher_key_id); ++index)
        publisher_nonzero |= token->identity.publisher_key_id[index];
    if (publisher_nonzero == 0u) return 0;
    for (index = 0u; index < sizeof(token->reserved); ++index)
        if (token->reserved[index] != 0u) return 0;
    for (index = 0u; index < sizeof(token->nonce); ++index)
        nonce_nonzero |= token->nonce[index];
    for (index = 0u; index < sizeof(token->authentication_tag); ++index)
        tag_nonzero |= token->authentication_tag[index];
    return nonce_nonzero != 0u && tag_nonzero != 0u;
}

static inline int rin_share_control_provision_identity_v1_valid(
    const RinShareControlProvisionIdentityV1* request)
{
    const uint8_t* first_identity;
    const uint8_t* certificate_identity;
    const uint8_t* trust_identity;
    uint8_t identity_difference = 0u;
    size_t index;
    if (request == NULL || request->struct_size != sizeof(*request) ||
        request->version != 1u ||
        !rin_share_control_provision_token_valid(&request->private_key) ||
        !rin_share_control_provision_token_valid(&request->certificate_list) ||
        !rin_share_control_provision_token_valid(&request->trust_bundle) ||
        request->private_key.file_object_id ==
            request->certificate_list.file_object_id ||
        request->private_key.file_object_id ==
            request->trust_bundle.file_object_id ||
        request->certificate_list.file_object_id ==
            request->trust_bundle.file_object_id ||
        request->reserved[0] != 0u || request->reserved[1] != 0u)
        return 0;
    first_identity = (const uint8_t*)&request->private_key.identity;
    certificate_identity =
        (const uint8_t*)&request->certificate_list.identity;
    trust_identity = (const uint8_t*)&request->trust_bundle.identity;
    for (index = 0u; index < sizeof(request->private_key.identity); ++index)
        identity_difference |=
            (first_identity[index] ^ certificate_identity[index]) |
            (first_identity[index] ^ trust_identity[index]);
    if (identity_difference != 0u) return 0;
    return 1;
}

static inline int rin_share_control_response_v5_valid(
    const RinShareControlResponseV5* response)
{
    return response != NULL && response->struct_size == sizeof(*response) &&
           response->version == RIN_SHARE_CONTROL_VERSION_5 &&
           response->result == 0 && response->reserved0 == 0u &&
           response->generation != 0u &&
           response->generation != UINT64_MAX &&
           response->reserved[0] == 0u && response->reserved[1] == 0u;
}

static inline int rin_share_control_dns_name_valid(const char* name,
                                                    size_t capacity)
{
    size_t length = 0u;
    size_t label_length = 0u;
    unsigned char previous = 0u;
    if (name == NULL || capacity < 2u || name[0] == '\0') return 0;
    while (length < capacity && name[length] != '\0') {
        const unsigned char value = (unsigned char)name[length];
        if (value == '.') {
            if (label_length == 0u || label_length > 63u ||
                previous == '-')
                return 0;
            label_length = 0u;
        } else if ((value >= 'a' && value <= 'z') ||
                   (value >= 'A' && value <= 'Z') ||
                   (value >= '0' && value <= '9') || value == '-') {
            if (label_length == 0u && value == '-') return 0;
            ++label_length;
        } else {
            return 0;
        }
        previous = value;
        ++length;
    }
    if (length == 0u || length >= capacity || length > 253u ||
        label_length == 0u || label_length > 63u || previous == '-')
        return 0;
    for (size_t index = length + 1u; index < capacity; ++index)
        if (name[index] != '\0') return 0;
    return 1;
}

static inline int rin_share_control_discovered_peer_v2_valid(
    const RinShareControlDiscoveredPeerV2* peer)
{
    uint8_t address_nonzero = 0u;
    size_t index;
    if (peer == NULL || peer->struct_size != sizeof(*peer) ||
        peer->version != 2u || peer->interface_id == 0u || peer->port == 0u ||
        peer->port > 65535u || peer->interface_generation == 0u ||
        peer->interface_generation == UINT64_MAX ||
        peer->device_generation == 0u || peer->device_generation == UINT64_MAX ||
        peer->expires_at_ms == 0u || peer->expires_at_ms == UINT64_MAX ||
        peer->address[0] != 0xfeu || (peer->address[1] & 0xc0u) != 0x80u ||
        peer->instance[0] == '\0' || peer->reserved[0] != 0u ||
        peer->reserved[1] != 0u ||
        !rin_share_control_dns_name_valid(peer->hostname,
                                          sizeof(peer->hostname)))
        return 0;
    for (index = 0u; index < sizeof(peer->address); ++index)
        address_nonzero |= peer->address[index];
    if (address_nonzero == 0u) return 0;
    for (index = 0u; index < sizeof(peer->instance); ++index) {
        const unsigned char value = (unsigned char)peer->instance[index];
        if (value == '\0') break;
        if (value < 0x20u || value > 0x7eu) return 0;
    }
    if (index == 0u || index == sizeof(peer->instance)) return 0;
    for (++index; index < sizeof(peer->instance); ++index)
        if (peer->instance[index] != '\0') return 0;
    return 1;
}

static inline int rin_share_control_response_v4_valid(
    const RinShareControlResponseV4* response)
{
    uint32_t index;
    if (response == NULL || response->struct_size != sizeof(*response) ||
        response->version != RIN_SHARE_CONTROL_VERSION_4 ||
        response->result != 0 || response->reserved0 != 0u ||
        response->generation == 0u || response->generation == UINT64_MAX ||
        response->count > RIN_SHARE_CONTROL_MAX_DISCOVERED_PEERS ||
        !rin_share_control_discovery_flags_valid(response->flags) ||
        response->reserved2 != 0u || response->reserved[0] != 0u ||
        response->reserved[1] != 0u)
        return 0;
    for (index = 0u; index < response->count; ++index)
        if (!rin_share_control_discovered_peer_v2_valid(
                &response->peers[index]))
            return 0;
    for (; index < RIN_SHARE_CONTROL_MAX_DISCOVERED_PEERS; ++index) {
        const uint8_t* bytes = (const uint8_t*)&response->peers[index];
        for (size_t byte_index = 0u;
             byte_index < sizeof(response->peers[index]); ++byte_index)
            if (bytes[byte_index] != 0u) return 0;
    }
    return 1;
}

static inline int rin_share_control_send_file_valid(
    const RinShareControlSendFileV1* file)
{
    size_t index;
    if (file == NULL || file->reserved0 != 0u ||
        file->token.magic != RIN_FILE_PORTAL_TOKEN_MAGIC ||
        file->token.version != RIN_FILE_PORTAL_TOKEN_VERSION ||
        file->token.token_size != RIN_FILE_PORTAL_TOKEN_SIZE ||
        (file->token.rights & RIN_FILE_PORTAL_RIGHT_READ) == 0u ||
        file->token.file_object_id == 0u ||
        file->token.file_object_id == UINT64_MAX ||
        !rin_file_portal_identity_valid(&file->token.identity) ||
        file->display_name_size == 0u ||
        file->display_name_size >= sizeof(file->display_name) ||
        file->display_name[file->display_name_size] != '\0')
        return 0;
    if ((file->display_name_size == 1u && file->display_name[0] == '.') ||
        (file->display_name_size == 2u && file->display_name[0] == '.' &&
         file->display_name[1] == '.'))
        return 0;
    for (index = 0u; index < file->display_name_size; ++index) {
        const unsigned char value = (unsigned char)file->display_name[index];
        if (value < 0x20u || value == 0x7fu || value == '/' || value == '\\')
            return 0;
    }
    for (index = (size_t)file->display_name_size + 1u;
         index < sizeof(file->display_name); ++index)
        if (file->display_name[index] != '\0') return 0;
    return 1;
}

static inline int rin_share_control_send_request_v1_valid(
    const RinShareControlSendRequestV1* request)
{
    uint32_t index;
    if (request == NULL || request->struct_size != sizeof(*request) ||
        request->version != 1u ||
        !rin_share_control_discovered_peer_v2_valid(&request->peer) ||
        request->file_count == 0u ||
        request->file_count > RIN_SHARE_CONTROL_MAX_SEND_FILES ||
        request->reserved0 != 0u || request->reserved[0] != 0u ||
        request->reserved[1] != 0u)
        return 0;
    for (index = 0u; index < request->file_count; ++index)
        if (!rin_share_control_send_file_valid(&request->files[index]))
            return 0;
    for (; index < RIN_SHARE_CONTROL_MAX_SEND_FILES; ++index) {
        const uint8_t* bytes = (const uint8_t*)&request->files[index];
        for (size_t byte_index = 0u;
             byte_index < sizeof(request->files[index]); ++byte_index)
            if (bytes[byte_index] != 0u) return 0;
    }
    return 1;
}

typedef struct RinShareControlMessageV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t magic;
    uint32_t opcode;
    uint64_t grant_id;
    uint32_t object_type;
    uint32_t rights;
    uint32_t flags;
    uint32_t reserved0;
    uint8_t peer_fingerprint[32];
    uint64_t transfer_id;
    uint64_t approval_generation;
    uint64_t approval_expires_at_ms;
    uint64_t reserved[2];
} RinShareControlMessageV1;

typedef struct RinShareControlPendingOfferV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t available;
    uint32_t reserved0;
    uint64_t generation;
    uint64_t transfer_id;
    uint64_t expires_at_ms;
    uint8_t peer_fingerprint[32];
    char display_name[64];
    uint32_t file_count;
    uint32_t reserved1;
    uint64_t total_bytes;
    uint64_t reserved[2];
} RinShareControlPendingOfferV1;

typedef struct RinShareControlResponseV1 {
    uint32_t struct_size;
    uint32_t version;
    int32_t result;
    uint32_t reserved0;
    uint64_t generation;
    RinShareControlPendingOfferV1 pending;
    uint64_t reserved[2];
} RinShareControlResponseV1;

#if defined(__cplusplus)
static_assert(sizeof(RinShareControlMessageV1) == 112u,
              "RinShareControlMessageV1 ABI drift");
static_assert(sizeof(RinShareControlPendingOfferV1) == 168u,
              "RinShareControlPendingOfferV1 ABI drift");
static_assert(sizeof(RinShareControlResponseV1) == 208u,
              "RinShareControlResponseV1 ABI drift");
static_assert(sizeof(RinShareControlPolicyV1) == 48u,
              "RinShareControlPolicyV1 ABI drift");
static_assert(sizeof(RinShareControlDeviceEntryV1) == 144u,
              "RinShareControlDeviceEntryV1 ABI drift");
static_assert(sizeof(RinShareControlHistoryEntryV1) == 160u,
              "RinShareControlHistoryEntryV1 ABI drift");
static_assert(sizeof(RinShareControlResponseV2) == 408u,
              "RinShareControlResponseV2 ABI drift");
static_assert(sizeof(RinShareControlDiscoveredPeerV1) == 136u,
              "RinShareControlDiscoveredPeerV1 ABI drift");
static_assert(sizeof(RinShareControlResponseV3) == 8760u,
              "RinShareControlResponseV3 ABI drift");
static_assert(sizeof(RinShareControlDiscoveredPeerV2) == 584u,
              "RinShareControlDiscoveredPeerV2 ABI drift");
static_assert(sizeof(RinShareControlResponseV4) == 37432u,
              "RinShareControlResponseV4 ABI drift");
static_assert(sizeof(RinShareControlSendFileV1) == 472u,
              "RinShareControlSendFileV1 ABI drift");
static_assert(sizeof(RinShareControlSendRequestV1) == 15720u,
              "RinShareControlSendRequestV1 ABI drift");
static_assert(sizeof(RinShareControlProvisionIdentityV1) == 648u,
              "RinShareControlProvisionIdentityV1 ABI drift");
static_assert(sizeof(RinShareControlResponseV5) == 40u,
              "RinShareControlResponseV5 ABI drift");
static_assert(sizeof(RinShareControlSetDirectoryGrantV1) == 232u,
              "RinShareControlSetDirectoryGrantV1 ABI drift");
static_assert(sizeof(RinShareControlResponseV6) == 40u,
              "RinShareControlResponseV6 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinShareControlMessageV1) == 112u,
               "RinShareControlMessageV1 ABI drift");
_Static_assert(sizeof(RinShareControlPendingOfferV1) == 168u,
               "RinShareControlPendingOfferV1 ABI drift");
_Static_assert(sizeof(RinShareControlResponseV1) == 208u,
               "RinShareControlResponseV1 ABI drift");
_Static_assert(sizeof(RinShareControlPolicyV1) == 48u,
               "RinShareControlPolicyV1 ABI drift");
_Static_assert(sizeof(RinShareControlDeviceEntryV1) == 144u,
               "RinShareControlDeviceEntryV1 ABI drift");
_Static_assert(sizeof(RinShareControlHistoryEntryV1) == 160u,
               "RinShareControlHistoryEntryV1 ABI drift");
_Static_assert(sizeof(RinShareControlResponseV2) == 408u,
               "RinShareControlResponseV2 ABI drift");
_Static_assert(sizeof(RinShareControlDiscoveredPeerV1) == 136u,
               "RinShareControlDiscoveredPeerV1 ABI drift");
_Static_assert(sizeof(RinShareControlResponseV3) == 8760u,
               "RinShareControlResponseV3 ABI drift");
_Static_assert(sizeof(RinShareControlDiscoveredPeerV2) == 584u,
               "RinShareControlDiscoveredPeerV2 ABI drift");
_Static_assert(sizeof(RinShareControlResponseV4) == 37432u,
               "RinShareControlResponseV4 ABI drift");
_Static_assert(sizeof(RinShareControlSendFileV1) == 472u,
               "RinShareControlSendFileV1 ABI drift");
_Static_assert(sizeof(RinShareControlSendRequestV1) == 15720u,
               "RinShareControlSendRequestV1 ABI drift");
_Static_assert(sizeof(RinShareControlProvisionIdentityV1) == 648u,
               "RinShareControlProvisionIdentityV1 ABI drift");
_Static_assert(sizeof(RinShareControlResponseV5) == 40u,
               "RinShareControlResponseV5 ABI drift");
_Static_assert(sizeof(RinShareControlSetDirectoryGrantV1) == 232u,
               "RinShareControlSetDirectoryGrantV1 ABI drift");
_Static_assert(sizeof(RinShareControlResponseV6) == 40u,
               "RinShareControlResponseV6 ABI drift");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_RINSHARE_CONTROL_H */
