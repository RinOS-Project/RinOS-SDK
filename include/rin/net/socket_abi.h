/* SPDX-License-Identifier: MIT */
/* Rin-native fixed-width local-socket identity contracts. */
#ifndef RIN_SDK_SOCKET_ABI_H
#define RIN_SDK_SOCKET_ABI_H

#ifndef RIN_SOCKET_ABI_TYPES_PROVIDED
#include <stddef.h>
#include <stdint.h>
#endif

#ifndef MIDL_PASS
static inline int rin_sdk_nonzero_bytes_valid(const void* value, size_t size)
{
    const unsigned char* bytes = (const unsigned char*)value;
    unsigned char aggregate = 0u;
    size_t index;
    if (size != 0u && bytes == NULL) return 0;
    for (index = 0u; index < size; ++index) aggregate |= bytes[index];
    return size != 0u && aggregate != 0u;
}

static inline int rin_sdk_live_identity(uint64_t value)
{
    return value != 0u && value != UINT64_MAX;
}

/* Session identity is a transport-visible, versioned value rather than a
 * POSIX credential.  Keep its exact layout available to SDK clients that use
 * peer-session socket metadata without importing the OS-Core API header. */
#define RIN_SESSION_IDENTITY_VERSION_1 1u
#ifndef RIN_SESSION_IDENTITY_V1_DEFINED
#define RIN_SESSION_IDENTITY_V1_DEFINED 1
typedef struct RinSessionIdentityV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t session_id;
    uint32_t uid;
    uint64_t instance_cookie;
    uint64_t reserved[2];
} RinSessionIdentityV1;
#if defined(__cplusplus)
static_assert(sizeof(RinSessionIdentityV1) == 40u,
              "RinSessionIdentityV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinSessionIdentityV1) == 40u,
               "RinSessionIdentityV1 ABI drift");
#endif
#endif

#define SO_RIN_UNIX_PUBLISH_SERVICE  0x5001
#define SO_RIN_UNIX_SERVICE_IDENTITY 0x5002
#define SO_RIN_UNIX_PEER_IDENTITY    0x5004
#define SO_RIN_UNIX_PEER_APP_IDENTITY 0x5005
#define SO_RIN_UNIX_PEER_SESSION_IDENTITY 0x5006
#define SO_RIN_UNIX_PEER_SESSION_STATE    0x5007
#define SO_RIN_UNIX_PEER_PACKAGE_IDENTITY 0x5008
#define SO_RIN_UNIX_PEER_APPLICATION_METADATA 0x5009
#define SO_RIN_UNIX_PEER_APPLICATION_IMAGE_PATH 0x500A
#define SO_RIN_UNIX_PEER_ACCOUNT_IDENTITY 0x500B
#define SO_RIN_UNIX_PEER_SERVICE_IDENTITY 0x500C
#define SO_RIN_UNIX_PEER_SECURITY_CONTEXT 0x500E
#define SO_RIN_UNIX_PEER_WINCOMPAT_TOKEN  0x500F

#define RIN_UNIX_SCOPE_SYSTEM UINT16_C(1)
#define RIN_UNIX_SCOPE_USER UINT16_C(2)

#define RIN_UNIX_SERVICE_IDENTITY_FLAG_PUBLISHED UINT16_C(0x0001)
#define RIN_UNIX_PEER_IDENTITY_VERSION 1u
#define RIN_UNIX_PEER_APP_IDENTITY_VERSION 1u
#define RIN_UNIX_PEER_SESSION_IDENTITY_VERSION 1u
#define RIN_UNIX_PEER_SESSION_STATE_VERSION_1 1u
#define RIN_UNIX_PEER_SESSION_STATE_VERSION_2 2u
#define RIN_UNIX_PEER_SESSION_STATE_VERSION \
    RIN_UNIX_PEER_SESSION_STATE_VERSION_2
#define RIN_UNIX_PEER_PACKAGE_IDENTITY_VERSION 1u
#define RIN_UNIX_PEER_APPLICATION_METADATA_VERSION 1u
#define RIN_UNIX_PEER_APPLICATION_IMAGE_PATH_VERSION 1u
#define RIN_UNIX_PEER_ACCOUNT_IDENTITY_VERSION 1u
#define RIN_UNIX_PEER_SERVICE_IDENTITY_VERSION 1u
#define RIN_UNIX_PEER_SECURITY_CONTEXT_VERSION 1u
#define RIN_UNIX_PEER_WINCOMPAT_TOKEN_VERSION 1u
#define RIN_UNIX_PEER_WINCOMPAT_TOKEN_FLAG_ACTIVE UINT16_C(0x0001)
#define RIN_UNIX_PEER_WINCOMPAT_TOKEN_FLAG_RESTRICTED UINT16_C(0x0002)
#define RIN_UNIX_PEER_WINCOMPAT_TOKEN_FLAG_MASK \
    (RIN_UNIX_PEER_WINCOMPAT_TOKEN_FLAG_ACTIVE | \
     RIN_UNIX_PEER_WINCOMPAT_TOKEN_FLAG_RESTRICTED)
#define RIN_UNIX_PEER_WINCOMPAT_TOKEN_MAX_GROUPS 12u
#define RIN_UNIX_PEER_WINCOMPAT_TOKEN_MAX_RESTRICTED_SIDS 16u
#define RIN_UNIX_PEER_WINCOMPAT_TOKEN_MAX_SID_SIZE 68u
#define RIN_UNIX_PEER_WINCOMPAT_TOKEN_MAX_PRIVILEGES 13u
#define RIN_UNIX_ACCOUNT_IDENTITY_SIZE 15u
#define RIN_UNIX_SESSION_STATE_NONE UINT32_C(0)
#define RIN_UNIX_SESSION_STATE_CREATING UINT32_C(1)
#define RIN_UNIX_SESSION_STATE_AUTHENTICATING UINT32_C(2)
#define RIN_UNIX_SESSION_STATE_ACTIVE_FOREGROUND UINT32_C(3)
#define RIN_UNIX_SESSION_STATE_ACTIVE_BACKGROUND UINT32_C(4)
#define RIN_UNIX_SESSION_STATE_LOCKED UINT32_C(5)
#define RIN_UNIX_SESSION_STATE_LOGGING_OUT UINT32_C(6)
#define RIN_UNIX_SESSION_STATE_TERMINATING UINT32_C(7)
#define RIN_UNIX_SESSION_STATE_DEAD UINT32_C(8)
#define RIN_UNIX_PEER_SESSION_STATE_FLAG_ADMINISTRATOR UINT16_C(0x0001)
#define RIN_UNIX_PEER_SESSION_STATE_FLAG_MASK \
    RIN_UNIX_PEER_SESSION_STATE_FLAG_ADMINISTRATOR
#define RIN_UNIX_PEER_IDENTITY_FLAG_CONNECTED UINT32_C(0x00000001)
#define RIN_UNIX_PEER_APP_IDENTITY_FLAG_CONNECTED       UINT32_C(0x00000001)
#define RIN_UNIX_PEER_APP_IDENTITY_FLAG_AUTHENTICATED   UINT32_C(0x00000002)
#define RIN_UNIX_APPLICATION_ID_SIZE 32u
#define RIN_UNIX_APPLICATION_NAME_MAX 64u
#define RIN_UNIX_APPLICATION_IMAGE_PATH_MAX 96u
#define RIN_UNIX_SERVICE_ID_MAX 64u

typedef struct rin_unix_service_identity_v1 {
    uint32_t slot_id;
    uint32_t owner_uid;
    uint16_t scope;
    uint16_t flags;
} rin_unix_service_identity_v1;

/* Authenticated service identity for the connected peer. service_id is
 * resolved by the kernel from the service-manager slot attached to the peer
 * socket; callers must still check the expected ID, scope, owner, and session. */
typedef struct rin_unix_peer_service_identity_v1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t scope;
    uint32_t flags;
    uint32_t service_slot_id;
    uint32_t owner_uid;
    uint32_t reserved;
    uint64_t process_id;
    uint64_t process_instance_cookie;
    char service_id[RIN_UNIX_SERVICE_ID_MAX];
} rin_unix_peer_service_identity_v1;

/* Returned by SO_RIN_UNIX_PEER_IDENTITY on one connected local socket. The
 * kernel, not the remote endpoint, supplies every field. */
typedef struct rin_unix_peer_identity_v1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t scope;
    uint64_t process_id;
    uint64_t process_instance_cookie;
    uint64_t connection_id;
    uint64_t capabilities;
    uint32_t owner_uid;
    uint32_t flags;
} rin_unix_peer_identity_v1;

/* Kernel-captured credentials for the connected peer. Kept separate from
 * SO_RIN_UNIX_PEER_IDENTITY so existing peer identity consumers retain their
 * fixed ABI while ACL consumers can query groups and login-session identity. */
typedef struct rin_unix_peer_security_context_v1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved;
    uint32_t effective_uid;
    uint32_t effective_gid;
    uint32_t group_count;
    uint32_t session_id;
    uint64_t session_cookie;
    uint32_t groups[8];
    uint32_t reserved2[2];
} rin_unix_peer_security_context_v1;

typedef struct rin_unix_wincompat_sid_entry_v1 {
    uint32_t sid_size;
    uint32_t attributes;
    uint8_t sid[RIN_UNIX_PEER_WINCOMPAT_TOKEN_MAX_SID_SIZE];
} rin_unix_wincompat_sid_entry_v1;

#if defined(_MSC_VER)
#define RIN_UNIX_PEER_ABI_ALIGN8 __declspec(align(8))
#elif defined(__GNUC__)
#define RIN_UNIX_PEER_ABI_ALIGN8 __attribute__((aligned(8)))
#else
#define RIN_UNIX_PEER_ABI_ALIGN8
#endif

/* Kernel-snapshotted effective WinCompat thread token, attached to the
 * connected Unix peer. Applications cannot submit this value as credentials. */
typedef struct RIN_UNIX_PEER_ABI_ALIGN8 rin_unix_peer_wincompat_token_v1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t token_id;
    uint64_t token_process_id;
    uint64_t token_process_cookie;
    uint32_t impersonation_level;
    uint32_t user_sid_size;
    uint32_t group_count;
    uint32_t restricted_sid_count;
    uint32_t privilege_mask;
    uint32_t reserved;
    uint8_t user_sid[RIN_UNIX_PEER_WINCOMPAT_TOKEN_MAX_SID_SIZE];
    rin_unix_wincompat_sid_entry_v1
        groups[RIN_UNIX_PEER_WINCOMPAT_TOKEN_MAX_GROUPS];
    rin_unix_wincompat_sid_entry_v1
        restricted_sids[RIN_UNIX_PEER_WINCOMPAT_TOKEN_MAX_RESTRICTED_SIDS];
} rin_unix_peer_wincompat_token_v1;
#define RIN_UNIX_PEER_WINCOMPAT_TOKEN_V1_SIZE 2256u
#if defined(__cplusplus)
static_assert(sizeof(rin_unix_wincompat_sid_entry_v1) == 76u,
              "WinCompat SID entry ABI drift");
static_assert(sizeof(rin_unix_peer_wincompat_token_v1) ==
                  RIN_UNIX_PEER_WINCOMPAT_TOKEN_V1_SIZE,
              "WinCompat token context ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(rin_unix_wincompat_sid_entry_v1) == 76u,
               "WinCompat SID entry ABI drift");
_Static_assert(sizeof(rin_unix_peer_wincompat_token_v1) ==
                   RIN_UNIX_PEER_WINCOMPAT_TOKEN_V1_SIZE,
               "WinCompat token context ABI drift");
#endif
#undef RIN_UNIX_PEER_ABI_ALIGN8
#undef RIN_UNIX_PEER_WINCOMPAT_TOKEN_V1_SIZE

/* Extended peer identity for security services. The kernel returns this only
 * when the peer was admitted from a verified signed RIN v3 image. */
typedef struct rin_unix_peer_app_identity_v1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t scope;
    uint64_t process_id;
    uint64_t process_instance_cookie;
    uint64_t connection_id;
    uint64_t capabilities;
    uint32_t owner_uid;
    uint32_t flags;
    uint8_t application_id[RIN_UNIX_APPLICATION_ID_SIZE];
} rin_unix_peer_app_identity_v1;

/* Returned by SO_RIN_UNIX_PEER_SESSION_IDENTITY.  Unlike a UID-only peer
 * credential this is a complete, kernel-captured session generation handle.
 * It is never accepted as authentication without revalidation by the session
 * manager. */
typedef RinSessionIdentityV1 rin_unix_peer_session_identity_v1;

typedef struct rin_unix_peer_session_state_v1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t state;
    uint32_t foreground;
    RinSessionIdentityV1 identity;
    uint64_t reserved[2];
} rin_unix_peer_session_state_v1;

/* Restricted to the signed crash diagnostic service. The opaque account key
 * is returned with the exact live peer session so consumers cannot substitute
 * a UID or a caller-supplied session token for persistent ownership. */
typedef struct rin_unix_peer_account_identity_v1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved;
    RinSessionIdentityV1 identity;
    uint8_t account_identity[RIN_UNIX_ACCOUNT_IDENTITY_SIZE];
    uint8_t reserved1;
} rin_unix_peer_account_identity_v1;

/* Returned by SO_RIN_UNIX_PEER_PACKAGE_IDENTITY.  Every field is captured
 * from the kernel-owned launch record; the application cannot select or
 * rewrite the package digest/generation used for recovery ownership. */
typedef struct rin_unix_peer_package_identity_v1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t process_id;
    uint64_t process_instance_cookie;
    uint64_t package_generation;
    uint8_t application_id[32];
    uint8_t package_digest[32];
    uint64_t reserved[2];
} rin_unix_peer_package_identity_v1;

/* Returned by SO_RIN_UNIX_PEER_APPLICATION_METADATA.  The display name is
 * copied from the kernel-owned sandbox launch identity, which is populated
 * from the signed system catalog or installed package record.  It is not
 * derived from a PID, executable basename, or caller-provided socket data. */
typedef struct rin_unix_peer_application_metadata_v1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t process_id;
    uint64_t process_instance_cookie;
    uint64_t package_generation;
    uint8_t application_id[RIN_UNIX_APPLICATION_ID_SIZE];
    char display_name[RIN_UNIX_APPLICATION_NAME_MAX];
    uint64_t reserved[2];
} rin_unix_peer_application_metadata_v1;

/* Returned by SO_RIN_UNIX_PEER_APPLICATION_IMAGE_PATH.  The image path is
 * copied from the kernel launch record after the signed image was admitted;
 * consumers must never reconstruct it from a PID, basename, or display name. */
typedef struct rin_unix_peer_application_image_path_v1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t process_id;
    uint64_t process_instance_cookie;
    uint64_t package_generation;
    uint8_t application_id[RIN_UNIX_APPLICATION_ID_SIZE];
    char image_path[RIN_UNIX_APPLICATION_IMAGE_PATH_MAX];
    uint64_t reserved[2];
} rin_unix_peer_application_image_path_v1;

/* Validate the transport-owned portion of a peer identity. Scope and
 * capability policy belongs to the consuming service. */
static inline int rin_unix_peer_identity_valid(
    const rin_unix_peer_identity_v1* identity)
{
    return identity != NULL && identity->struct_size == sizeof(*identity) &&
           identity->version == RIN_UNIX_PEER_IDENTITY_VERSION &&
           rin_sdk_live_identity(identity->process_id) &&
           rin_sdk_live_identity(identity->process_instance_cookie) &&
           rin_sdk_live_identity(identity->connection_id) &&
           identity->flags == RIN_UNIX_PEER_IDENTITY_FLAG_CONNECTED;
}

static inline int rin_unix_peer_security_context_valid(
    const rin_unix_peer_security_context_v1* context)
{
    return context != NULL &&
           context->struct_size == sizeof(*context) &&
           context->version == RIN_UNIX_PEER_SECURITY_CONTEXT_VERSION &&
           context->reserved == 0u && context->group_count <= 8u &&
           ((context->session_id == 0u && context->session_cookie == 0u) ||
            (context->session_id != 0u && context->session_cookie != 0u)) &&
           context->reserved2[0] == 0u && context->reserved2[1] == 0u;
}

static inline int rin_unix_wincompat_sid_valid(
    const uint8_t* sid, uint32_t sid_size) {
    return sid != NULL && sid_size >= 8u && sid_size <= 68u &&
           sid[0] == 1u && sid[1] <= 15u &&
           sid_size == 8u + (uint32_t)sid[1] * 4u;
}

static inline int rin_unix_peer_wincompat_token_valid(
    const rin_unix_peer_wincompat_token_v1* context) {
    uint32_t index;
    uint32_t byte_index;
    if (!context || context->struct_size != sizeof(*context) ||
        context->version != RIN_UNIX_PEER_WINCOMPAT_TOKEN_VERSION ||
        (context->flags & ~RIN_UNIX_PEER_WINCOMPAT_TOKEN_FLAG_MASK) != 0u ||
        context->reserved != 0u ||
        context->group_count > RIN_UNIX_PEER_WINCOMPAT_TOKEN_MAX_GROUPS ||
        context->restricted_sid_count >
            RIN_UNIX_PEER_WINCOMPAT_TOKEN_MAX_RESTRICTED_SIDS ||
        (context->privilege_mask &
         ~((UINT32_C(1) << RIN_UNIX_PEER_WINCOMPAT_TOKEN_MAX_PRIVILEGES) -
           UINT32_C(1))) != 0u)
        return 0;
    if ((context->flags & RIN_UNIX_PEER_WINCOMPAT_TOKEN_FLAG_ACTIVE) == 0u) {
        const uint8_t* bytes = (const uint8_t*)context;
        for (byte_index = 8u; byte_index < sizeof(*context); ++byte_index)
            if (bytes[byte_index] != 0u) return 0;
        return 1;
    }
    if (context->token_id == 0u || context->token_process_id == 0u ||
        context->token_process_cookie == 0u ||
        context->impersonation_level < 2u ||
        context->impersonation_level > 3u ||
        !rin_unix_wincompat_sid_valid(context->user_sid,
                                       context->user_sid_size) ||
        (((context->flags & RIN_UNIX_PEER_WINCOMPAT_TOKEN_FLAG_RESTRICTED) !=
          0u) != (context->restricted_sid_count != 0u)))
        return 0;
    for (index = context->user_sid_size;
         index < RIN_UNIX_PEER_WINCOMPAT_TOKEN_MAX_SID_SIZE; ++index)
        if (context->user_sid[index] != 0u) return 0;
    for (index = 0u; index < RIN_UNIX_PEER_WINCOMPAT_TOKEN_MAX_GROUPS;
         ++index) {
        const rin_unix_wincompat_sid_entry_v1* entry =
            &context->groups[index];
        if (index >= context->group_count) {
            const uint8_t* bytes = (const uint8_t*)entry;
            for (byte_index = 0u; byte_index < sizeof(*entry); ++byte_index)
                if (bytes[byte_index] != 0u) return 0;
            continue;
        }
        if (!rin_unix_wincompat_sid_valid(entry->sid, entry->sid_size) ||
            (entry->attributes & ~UINT32_C(0xe000007f)) != 0u)
            return 0;
        for (byte_index = entry->sid_size;
             byte_index < RIN_UNIX_PEER_WINCOMPAT_TOKEN_MAX_SID_SIZE;
             ++byte_index)
            if (entry->sid[byte_index] != 0u) return 0;
    }
    for (index = 0u;
         index < RIN_UNIX_PEER_WINCOMPAT_TOKEN_MAX_RESTRICTED_SIDS; ++index) {
        const rin_unix_wincompat_sid_entry_v1* entry =
            &context->restricted_sids[index];
        if (index >= context->restricted_sid_count) {
            const uint8_t* bytes = (const uint8_t*)entry;
            for (byte_index = 0u; byte_index < sizeof(*entry); ++byte_index)
                if (bytes[byte_index] != 0u) return 0;
            continue;
        }
        if (!rin_unix_wincompat_sid_valid(entry->sid, entry->sid_size) ||
            (entry->attributes & ~UINT32_C(0xe000007f)) != 0u)
            return 0;
        for (byte_index = entry->sid_size;
             byte_index < RIN_UNIX_PEER_WINCOMPAT_TOKEN_MAX_SID_SIZE;
             ++byte_index)
            if (entry->sid[byte_index] != 0u) return 0;
    }
    return 1;
}

static inline int rin_unix_application_id_valid(
    const uint8_t application_id[RIN_UNIX_APPLICATION_ID_SIZE])
{
    return rin_sdk_nonzero_bytes_valid(application_id,
                                        RIN_UNIX_APPLICATION_ID_SIZE);
}

static inline int rin_unix_peer_app_identity_valid(
    const rin_unix_peer_app_identity_v1* identity)
{
    return identity != NULL && identity->struct_size == sizeof(*identity) &&
           identity->version == RIN_UNIX_PEER_APP_IDENTITY_VERSION &&
           rin_sdk_live_identity(identity->process_id) &&
           rin_sdk_live_identity(identity->process_instance_cookie) &&
           rin_sdk_live_identity(identity->connection_id) &&
           identity->flags ==
               (RIN_UNIX_PEER_APP_IDENTITY_FLAG_CONNECTED |
                RIN_UNIX_PEER_APP_IDENTITY_FLAG_AUTHENTICATED) &&
           rin_unix_application_id_valid(identity->application_id);
}

static inline int rin_unix_peer_session_identity_valid(
    const rin_unix_peer_session_identity_v1* identity)
{
    return identity != NULL &&
           identity->struct_size == sizeof(*identity) &&
           identity->version == RIN_UNIX_PEER_SESSION_IDENTITY_VERSION &&
           identity->reserved[0] == 0u && identity->reserved[1] == 0u &&
           identity->session_id != 0u && identity->uid != 0u &&
           rin_sdk_live_identity(identity->instance_cookie);
}

static inline int rin_unix_peer_account_identity_valid(
    const rin_unix_peer_account_identity_v1* value)
{
    return value != NULL && value->struct_size == sizeof(*value) &&
           value->version == RIN_UNIX_PEER_ACCOUNT_IDENTITY_VERSION &&
           value->reserved == 0u && value->reserved1 == 0u &&
           rin_unix_peer_session_identity_valid(&value->identity) &&
           value->account_identity[0] == 1u &&
           rin_sdk_nonzero_bytes_valid(value->account_identity + 1u,
                                       RIN_UNIX_ACCOUNT_IDENTITY_SIZE - 1u);
}

static inline int rin_unix_peer_session_state_valid(
    const rin_unix_peer_session_state_v1* state)
{
    return state != NULL && state->struct_size == sizeof(*state) &&
           (state->version == RIN_UNIX_PEER_SESSION_STATE_VERSION_1 ||
            state->version == RIN_UNIX_PEER_SESSION_STATE_VERSION_2) &&
           (state->version == RIN_UNIX_PEER_SESSION_STATE_VERSION_1
                ? state->flags == 0u
                : (state->flags & (uint16_t)~
                       RIN_UNIX_PEER_SESSION_STATE_FLAG_MASK) == 0u) &&
           state->state != 0u &&
           state->foreground <= 1u &&
           rin_unix_peer_session_identity_valid(&state->identity) &&
           state->reserved[0] == 0u && state->reserved[1] == 0u;
}

static inline int rin_unix_peer_package_identity_valid(
    const rin_unix_peer_package_identity_v1* identity)
{
    if (identity == NULL ||
        identity->struct_size != sizeof(*identity) ||
        identity->version != RIN_UNIX_PEER_PACKAGE_IDENTITY_VERSION ||
        identity->flags != RIN_UNIX_PEER_APP_IDENTITY_FLAG_AUTHENTICATED ||
        !rin_sdk_live_identity(identity->process_id) ||
        !rin_sdk_live_identity(identity->process_instance_cookie) ||
        !rin_sdk_live_identity(identity->package_generation) ||
        identity->reserved[0] != 0u || identity->reserved[1] != 0u) {
        return 0;
    }
    return rin_sdk_nonzero_bytes_valid(identity->application_id,
                                        sizeof(identity->application_id)) &&
           rin_sdk_nonzero_bytes_valid(identity->package_digest,
                                        sizeof(identity->package_digest));
}

static inline int rin_unix_application_name_valid(
    const char name[RIN_UNIX_APPLICATION_NAME_MAX])
{
    size_t index;
    if (name == NULL || name[0] == '\0') return 0;
    for (index = 1u; index < RIN_UNIX_APPLICATION_NAME_MAX; ++index)
        if (name[index] == '\0') return 1;
    return 0;
}

static inline int rin_unix_application_image_path_valid(
    const char path[RIN_UNIX_APPLICATION_IMAGE_PATH_MAX])
{
    size_t index;
    if (path == NULL || path[0] != '/') return 0;
    for (index = 1u; index < RIN_UNIX_APPLICATION_IMAGE_PATH_MAX; ++index)
        if (path[index] == '\0') return index > 1u;
    return 0;
}

static inline int rin_unix_peer_application_metadata_valid(
    const rin_unix_peer_application_metadata_v1* metadata)
{
    return metadata != NULL &&
           metadata->struct_size == sizeof(*metadata) &&
           metadata->version == RIN_UNIX_PEER_APPLICATION_METADATA_VERSION &&
           metadata->flags ==
               (RIN_UNIX_PEER_APP_IDENTITY_FLAG_CONNECTED |
                RIN_UNIX_PEER_APP_IDENTITY_FLAG_AUTHENTICATED) &&
           rin_sdk_live_identity(metadata->process_id) &&
           rin_sdk_live_identity(metadata->process_instance_cookie) &&
           rin_unix_application_id_valid(metadata->application_id) &&
           rin_unix_application_name_valid(metadata->display_name) &&
           metadata->reserved[0] == 0u && metadata->reserved[1] == 0u;
}

static inline int rin_unix_peer_application_image_path_valid(
    const rin_unix_peer_application_image_path_v1* image)
{
    return image != NULL &&
           image->struct_size == sizeof(*image) &&
           image->version == RIN_UNIX_PEER_APPLICATION_IMAGE_PATH_VERSION &&
           image->flags ==
               (RIN_UNIX_PEER_APP_IDENTITY_FLAG_CONNECTED |
                RIN_UNIX_PEER_APP_IDENTITY_FLAG_AUTHENTICATED) &&
           rin_sdk_live_identity(image->process_id) &&
           rin_sdk_live_identity(image->process_instance_cookie) &&
           rin_sdk_live_identity(image->package_generation) &&
           rin_unix_application_id_valid(image->application_id) &&
           rin_unix_application_image_path_valid(image->image_path) &&
           image->reserved[0] == 0u && image->reserved[1] == 0u;
}

static inline int rin_unix_peer_service_identity_valid(
    const rin_unix_peer_service_identity_v1* identity)
{
    size_t length = 0u;
    if (identity == NULL || identity->struct_size != sizeof(*identity) ||
        identity->version != RIN_UNIX_PEER_SERVICE_IDENTITY_VERSION ||
        (identity->scope != RIN_UNIX_SCOPE_SYSTEM &&
         identity->scope != RIN_UNIX_SCOPE_USER) ||
        identity->flags != RIN_UNIX_SERVICE_IDENTITY_FLAG_PUBLISHED ||
        identity->service_slot_id == 0u ||
        (identity->scope == RIN_UNIX_SCOPE_USER &&
         identity->owner_uid == 0u) ||
        identity->reserved != 0u ||
        !rin_sdk_live_identity(identity->process_id) ||
        !rin_sdk_live_identity(identity->process_instance_cookie) ||
        identity->service_id[0] == '\0')
        return 0;
    while (length < RIN_UNIX_SERVICE_ID_MAX &&
           identity->service_id[length] != '\0') {
        const unsigned char value =
            (unsigned char)identity->service_id[length];
        if (!((value >= 'a' && value <= 'z') ||
              (value >= '0' && value <= '9') || value == '-' ||
              value == '_' || value == '.'))
            return 0;
        ++length;
    }
    if (length == 0u || length >= RIN_UNIX_SERVICE_ID_MAX ||
        (length == 1u && identity->service_id[0] == '.') ||
        (length == 2u && identity->service_id[0] == '.' &&
         identity->service_id[1] == '.'))
        return 0;
    for (size_t index = length + 1u; index < RIN_UNIX_SERVICE_ID_MAX;
         ++index)
        if (identity->service_id[index] != '\0') return 0;
    return 1;
}

#endif /* !MIDL_PASS */

#if defined(__cplusplus) && !defined(MIDL_PASS)
static_assert(sizeof(rin_unix_service_identity_v1) == 12u,
              "Rin Unix service identity ABI drift");
static_assert(sizeof(rin_unix_peer_service_identity_v1) == 104u,
              "Rin Unix peer service identity ABI drift");
static_assert(sizeof(rin_unix_peer_identity_v1) == 48u,
              "Rin Unix peer identity ABI drift");
static_assert(sizeof(rin_unix_peer_security_context_v1) == 72u,
              "Rin Unix peer security context ABI drift");
static_assert(sizeof(rin_unix_peer_app_identity_v1) == 80u,
              "Rin Unix peer application identity ABI drift");
static_assert(sizeof(rin_unix_peer_session_identity_v1) == 40u,
              "Rin Unix peer session identity ABI drift");
static_assert(sizeof(rin_unix_peer_session_state_v1) == 72u,
              "Rin Unix peer session state ABI drift");
static_assert(sizeof(rin_unix_peer_account_identity_v1) == 64u,
              "Rin Unix peer account identity ABI drift");
static_assert(sizeof(rin_unix_peer_package_identity_v1) == 112u,
              "Rin Unix peer package identity ABI drift");
static_assert(sizeof(rin_unix_peer_application_metadata_v1) == 144u,
              "Rin Unix peer application metadata ABI drift");
static_assert(sizeof(rin_unix_peer_application_image_path_v1) == 176u,
              "Rin Unix peer application image path ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(MIDL_PASS)
_Static_assert(sizeof(rin_unix_service_identity_v1) == 12u,
               "Rin Unix service identity ABI drift");
_Static_assert(sizeof(rin_unix_peer_service_identity_v1) == 104u,
               "Rin Unix peer service identity ABI drift");
_Static_assert(sizeof(rin_unix_peer_identity_v1) == 48u,
               "Rin Unix peer identity ABI drift");
_Static_assert(sizeof(rin_unix_peer_security_context_v1) == 72u,
               "Rin Unix peer security context ABI drift");
_Static_assert(sizeof(rin_unix_peer_app_identity_v1) == 80u,
               "Rin Unix peer application identity ABI drift");
_Static_assert(sizeof(rin_unix_peer_session_identity_v1) == 40u,
               "Rin Unix peer session identity ABI drift");
_Static_assert(sizeof(rin_unix_peer_session_state_v1) == 72u,
               "Rin Unix peer session state ABI drift");
_Static_assert(sizeof(rin_unix_peer_account_identity_v1) == 64u,
               "Rin Unix peer account identity ABI drift");
_Static_assert(sizeof(rin_unix_peer_package_identity_v1) == 112u,
               "Rin Unix peer package identity ABI drift");
_Static_assert(sizeof(rin_unix_peer_application_metadata_v1) == 144u,
               "Rin Unix peer application metadata ABI drift");
_Static_assert(sizeof(rin_unix_peer_application_image_path_v1) == 176u,
               "Rin Unix peer application image path ABI drift");
#endif

#endif /* RIN_SDK_SOCKET_ABI_H */
