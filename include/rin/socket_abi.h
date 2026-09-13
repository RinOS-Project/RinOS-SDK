/* SPDX-License-Identifier: MIT */
/* Rin-native fixed-width local-socket identity contracts. */
#ifndef RIN_SDK_SOCKET_ABI_H
#define RIN_SDK_SOCKET_ABI_H

#ifndef RIN_SOCKET_ABI_TYPES_PROVIDED
#include <stddef.h>
#include <stdint.h>
#endif

#include "contract_abi.h"

static inline int rin_sdk_nonzero_bytes_valid(const void* value, size_t size)
{
    const unsigned char* bytes = (const unsigned char*)value;
    unsigned char aggregate = 0u;
    size_t index;
    if (size != 0u && bytes == NULL) return 0;
    for (index = 0u; index < size; ++index) aggregate |= bytes[index];
    return size != 0u && aggregate != 0u;
}

#define SO_RIN_UNIX_PUBLISH_SERVICE  0x5001
#define SO_RIN_UNIX_SERVICE_IDENTITY 0x5002
#define SO_RIN_UNIX_PEER_IDENTITY    0x5004
#define SO_RIN_UNIX_PEER_APP_IDENTITY 0x5005
#define SO_RIN_UNIX_PEER_SESSION_IDENTITY 0x5006
#define SO_RIN_UNIX_PEER_SESSION_STATE    0x5007
#define SO_RIN_UNIX_PEER_PACKAGE_IDENTITY 0x5008
#define SO_RIN_UNIX_PEER_APPLICATION_METADATA 0x5009
#define SO_RIN_UNIX_PEER_APPLICATION_IMAGE_PATH 0x500A

#define RIN_UNIX_SERVICE_IDENTITY_FLAG_PUBLISHED UINT16_C(0x0001)
#define RIN_UNIX_PEER_IDENTITY_VERSION 1u
#define RIN_UNIX_PEER_APP_IDENTITY_VERSION 1u
#define RIN_UNIX_PEER_SESSION_IDENTITY_VERSION 1u
#define RIN_UNIX_PEER_SESSION_STATE_VERSION 1u
#define RIN_UNIX_PEER_PACKAGE_IDENTITY_VERSION 1u
#define RIN_UNIX_PEER_APPLICATION_METADATA_VERSION 1u
#define RIN_UNIX_PEER_APPLICATION_IMAGE_PATH_VERSION 1u
#define RIN_UNIX_PEER_IDENTITY_FLAG_CONNECTED UINT32_C(0x00000001)
#define RIN_UNIX_PEER_APP_IDENTITY_FLAG_CONNECTED       UINT32_C(0x00000001)
#define RIN_UNIX_PEER_APP_IDENTITY_FLAG_AUTHENTICATED   UINT32_C(0x00000002)
#define RIN_UNIX_APPLICATION_ID_SIZE 32u
#define RIN_UNIX_APPLICATION_NAME_MAX 64u
#define RIN_UNIX_APPLICATION_IMAGE_PATH_MAX 96u

typedef struct rin_unix_service_identity_v1 {
    uint32_t slot_id;
    uint32_t owner_uid;
    uint16_t scope;
    uint16_t flags;
} rin_unix_service_identity_v1;

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
           identity->process_id != 0u &&
           identity->process_instance_cookie != 0u &&
           identity->connection_id != 0u &&
           identity->flags == RIN_UNIX_PEER_IDENTITY_FLAG_CONNECTED;
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
           identity->process_id != 0u &&
           identity->process_instance_cookie != 0u &&
           identity->connection_id != 0u &&
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
           identity->instance_cookie != 0u;
}

static inline int rin_unix_peer_session_state_valid(
    const rin_unix_peer_session_state_v1* state)
{
    return state != NULL && state->struct_size == sizeof(*state) &&
           state->version == RIN_UNIX_PEER_SESSION_STATE_VERSION &&
           state->flags == 0u && state->state != 0u &&
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
        identity->process_id == 0u || identity->process_instance_cookie == 0u ||
        identity->package_generation == 0u ||
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
           metadata->process_id != 0u &&
           metadata->process_instance_cookie != 0u &&
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
           image->process_id != 0u &&
           image->process_instance_cookie != 0u &&
           image->package_generation != 0u &&
           rin_unix_application_id_valid(image->application_id) &&
           rin_unix_application_image_path_valid(image->image_path) &&
           image->reserved[0] == 0u && image->reserved[1] == 0u;
}

#if defined(__cplusplus)
static_assert(sizeof(rin_unix_service_identity_v1) == 12u,
              "Rin Unix service identity ABI drift");
static_assert(sizeof(rin_unix_peer_identity_v1) == 48u,
              "Rin Unix peer identity ABI drift");
static_assert(sizeof(rin_unix_peer_app_identity_v1) == 80u,
              "Rin Unix peer application identity ABI drift");
static_assert(sizeof(rin_unix_peer_session_identity_v1) == 40u,
              "Rin Unix peer session identity ABI drift");
static_assert(sizeof(rin_unix_peer_session_state_v1) == 72u,
              "Rin Unix peer session state ABI drift");
static_assert(sizeof(rin_unix_peer_package_identity_v1) == 112u,
              "Rin Unix peer package identity ABI drift");
static_assert(sizeof(rin_unix_peer_application_metadata_v1) == 144u,
              "Rin Unix peer application metadata ABI drift");
static_assert(sizeof(rin_unix_peer_application_image_path_v1) == 176u,
              "Rin Unix peer application image path ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(rin_unix_service_identity_v1) == 12u,
               "Rin Unix service identity ABI drift");
_Static_assert(sizeof(rin_unix_peer_identity_v1) == 48u,
               "Rin Unix peer identity ABI drift");
_Static_assert(sizeof(rin_unix_peer_app_identity_v1) == 80u,
               "Rin Unix peer application identity ABI drift");
_Static_assert(sizeof(rin_unix_peer_session_identity_v1) == 40u,
               "Rin Unix peer session identity ABI drift");
_Static_assert(sizeof(rin_unix_peer_session_state_v1) == 72u,
               "Rin Unix peer session state ABI drift");
_Static_assert(sizeof(rin_unix_peer_package_identity_v1) == 112u,
               "Rin Unix peer package identity ABI drift");
_Static_assert(sizeof(rin_unix_peer_application_metadata_v1) == 144u,
               "Rin Unix peer application metadata ABI drift");
_Static_assert(sizeof(rin_unix_peer_application_image_path_v1) == 176u,
               "Rin Unix peer application image path ABI drift");
#endif

#endif /* RIN_SDK_SOCKET_ABI_H */

