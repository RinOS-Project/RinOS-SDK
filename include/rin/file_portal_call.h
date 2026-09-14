/* SPDX-License-Identifier: MIT */
/* Public syscall ABI for the privileged file chooser portal. */

#ifndef RIN_SDK_FILE_PORTAL_CALL_H
#define RIN_SDK_FILE_PORTAL_CALL_H

#include <stdint.h>

#include "file_portal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_FILE_PORTAL_CALL_VERSION UINT16_C(1)

#define RIN_FILE_PORTAL_OPERATION_ISSUE UINT16_C(1)
#define RIN_FILE_PORTAL_OPERATION_OPEN  UINT16_C(2)
/* Reauthorizes one durable RFPG record directly into the current sandbox
 * process.  It never accepts or returns a short-lived portal token. */
#define RIN_FILE_PORTAL_OPERATION_DURABLE_OPEN UINT16_C(3)
/* Lists one durable RFPG record belonging to the current sandbox identity. */
#define RIN_FILE_PORTAL_OPERATION_DURABLE_LIST UINT16_C(4)
/* Privileged Settings-only listing of durable records across identities. */
#define RIN_FILE_PORTAL_OPERATION_SETTINGS_LIST UINT16_C(5)
/* Revokes a just-issued ephemeral object before its recipient consumes it.
 * The caller supplies only the kernel-issued object ID and receives no
 * token, descriptor, or object metadata in return. */
#define RIN_FILE_PORTAL_OPERATION_REVOKE UINT16_C(6)
/* Settings starts a kernel-owned physical-confirmation revoke prompt. */
#define RIN_FILE_PORTAL_OPERATION_SETTINGS_REVOKE_REQUEST UINT16_C(7)
#define RIN_FILE_PORTAL_OPERATION_SETTINGS_REVOKE_STATUS  UINT16_C(8)
#define RIN_FILE_PORTAL_OPERATION_SETTINGS_REVOKE_FINISH  UINT16_C(9)
#define RIN_FILE_PORTAL_OPERATION_PAYLOAD_READ     UINT16_C(10)
#define RIN_FILE_PORTAL_OPERATION_PAYLOAD_WRITE    UINT16_C(11)
#define RIN_FILE_PORTAL_OPERATION_PAYLOAD_SYNC     UINT16_C(12)
#define RIN_FILE_PORTAL_OPERATION_PAYLOAD_TRUNCATE UINT16_C(13)

#define RIN_FILE_PORTAL_CALL_FD_CLOEXEC UINT32_C(1)
#define RIN_FILE_PORTAL_TOKEN_LIFETIME_SECONDS UINT64_C(120)

#define RIN_FILE_PORTAL_DURABLE_OBJECT_REGULAR UINT32_C(1)
#define RIN_FILE_PORTAL_DURABLE_OBJECT_DIRECTORY UINT32_C(2)
#define RIN_FILE_PORTAL_DURABLE_SCOPE_EXACT UINT32_C(1)
#define RIN_FILE_PORTAL_DURABLE_SCOPE_DESCENDANTS UINT32_C(2)
#define RIN_FILE_PORTAL_DURABLE_STATE_ACTIVE UINT32_C(1)
#define RIN_FILE_PORTAL_DURABLE_STATE_EXPIRED UINT32_C(2)
#define RIN_FILE_PORTAL_PAYLOAD_VERSION UINT16_C(1)
#define RIN_FILE_PORTAL_PAYLOAD_DATA_SIZE 176u

/* Public listing never exposes an identity (it is always the current
 * sandbox), a pathname, a file descriptor, an object cookie or a portal
 * token.  It shares the token-sized call storage so the v1 call layout remains
 * fixed across ISSUE, OPEN and durable operations. */
typedef struct __attribute__((packed)) RinFilePortalDurableEntryV1 {
    uint64_t grant_id;
    uint64_t database_generation;
    uint64_t expires_at_epoch;
    uint64_t mount_id;
    uint64_t namespace_id;
    uint64_t object_id;
    uint64_t object_generation;
    uint32_t object_type;
    uint32_t scope;
    uint32_t rights;
    uint32_t state;
    uint8_t reserved[136];
} RinFilePortalDurableEntryV1;

/* Settings may display the authenticated package identity alongside the
 * stable object metadata. This member is accepted only by the privileged
 * SETTINGS_LIST operation; ordinary durable listing keeps identity private. */
typedef struct __attribute__((packed)) RinFilePortalSettingsEntryV1 {
    uint64_t grant_id;
    uint64_t database_generation;
    uint64_t expires_at_epoch;
    uint64_t mount_id;
    uint64_t namespace_id;
    uint64_t object_id;
    uint64_t object_generation;
    uint32_t object_type;
    uint32_t scope;
    uint32_t rights;
    uint32_t state;
    RinFilePortalIdentityV1 identity;
    uint8_t reserved[32];
} RinFilePortalSettingsEntryV1;

/* A bounded inline chunk. Larger payloads are streamed through repeated
 * offset-addressed calls so the syscall copies the complete request/result
 * atomically and never trusts a user pointer embedded in the portal ABI. */
typedef struct __attribute__((packed)) RinFilePortalPayloadV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t payload_size;
    uint32_t reserved0;
    uint64_t offset;
    uint64_t result_size;
    uint8_t bytes[RIN_FILE_PORTAL_PAYLOAD_DATA_SIZE];
} RinFilePortalPayloadV1;

#if defined(__cplusplus)
static_assert(sizeof(RinFilePortalPayloadV1) == RIN_FILE_PORTAL_TOKEN_SIZE,
              "RinFilePortalPayloadV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinFilePortalPayloadV1) == RIN_FILE_PORTAL_TOKEN_SIZE,
               "RinFilePortalPayloadV1 ABI drift");
#endif

/* ISSUE input:
 *   descriptor = chooser-owned VFS descriptor
 *   requested_rights, target_process_id, target_process_cookie
 * OPEN input:
 *   descriptor = minimum result descriptor
 *   requested_rights, descriptor_flags, token
 * DURABLE_OPEN input:
 *   descriptor = minimum result descriptor
 *   requested_rights, descriptor_flags
 *   file_object_id = caller-owned durable grant ID
 *   target_process_id, target_process_cookie and token = zero
 * DURABLE_LIST input:
 *   file_object_id = zero-based cursor (0 for the first record)
 *   descriptor = -1; all token/open inputs and outputs = zero/-1
 *   output file_object_id = next cursor; durable_entry = one current record
 * SETTINGS_LIST input:
 *   file_object_id = zero-based global RFPG record cursor
 *   descriptor = -1; all token/open inputs and outputs = zero/-1
 *   output file_object_id = next cursor; expires_at_epoch and settings_entry
 *   carry the selected record's expiry/metadata
 * REVOKE input:
 *   file_object_id = kernel-issued ISSUE object ID; all other inputs and
 *   outputs use their canonical empty values. A successful revoke clears the
 *   object ID before returning to the caller.
 * SETTINGS_REVOKE_REQUEST input:
 *   file_object_id = durable grant ID; all identity, token, descriptor and
 *   output fields are empty. The kernel resolves the authenticated identity,
 *   prepares a private RFPG decision, and returns request_id plus its trusted
 *   expiry. STATUS and FINISH accept only request_id. STATUS returns the
 *   terminal permission status and committed database generation; mutation
 *   occurs only after the built-in physical prompt receives an exact click.
 * PAYLOAD_READ/PAYLOAD_WRITE:
 *   descriptor = current-process VFS file descriptor returned by OPEN or
 *   DURABLE_OPEN; payload.offset and payload_size select one bounded chunk.
 *   READ returns bytes/result_size, WRITE consumes bytes/result_size.
 * PAYLOAD_SYNC:
 *   descriptor = current-process VFS file descriptor; payload is empty.
 * PAYLOAD_TRUNCATE:
 *   descriptor = current-process VFS file descriptor; payload.offset is the
 *   new exact size and payload is otherwise empty.
 *
 * The kernel requires every output field to have its canonical empty value
 * before executing the operation. process_fd is -1 when empty. Listing
 * operations mirror the selected record's expiry into expires_at_epoch only
 * after a successful result. This prevents
 * partially initialized callers from accidentally treating stale data as a
 * successful grant.  For DURABLE_OPEN, file_object_id is intentionally the
 * grant ID both on entry and after success; the kernel does not disclose an
 * object cookie or a reusable token. The syscall return value is a
 * RinResultCode. */
typedef struct RinFilePortalCallV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    int32_t descriptor;
    uint32_t requested_rights;
    uint32_t target_process_id;
    uint32_t descriptor_flags;
    uint64_t target_process_cookie;
    union {
        RinFilePortalTokenV1 token;
        RinFilePortalDurableEntryV1 durable_entry;
        RinFilePortalSettingsEntryV1 settings_entry;
        RinFilePortalPayloadV1 payload;
    };
    uint32_t granted_rights;
    int32_t process_fd;
    uint64_t file_object_id;
    uint64_t expires_at_epoch;
    uint64_t request_id;
    uint64_t new_generation;
    int32_t request_status;
    uint32_t reserved_status;
    uint64_t reserved[1];
} RinFilePortalCallV1;

/* Common Rin user runtime entry. ISSUE callers pass the returned token to the
 * exact target over authenticated IPC; OPEN callers receive process_fd only
 * after the token has been consumed and an independent VFS reference exists. */
int rin_file_portal_call(RinFilePortalCallV1* call);

#if defined(__cplusplus)
static_assert(sizeof(RinFilePortalCallV1) == 296u,
              "RinFilePortalCallV1 ABI drift");
static_assert(sizeof(RinFilePortalDurableEntryV1) == RIN_FILE_PORTAL_TOKEN_SIZE,
              "RinFilePortalDurableEntryV1 ABI drift");
static_assert(sizeof(RinFilePortalSettingsEntryV1) == RIN_FILE_PORTAL_TOKEN_SIZE,
              "RinFilePortalSettingsEntryV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinFilePortalCallV1) == 296u,
               "RinFilePortalCallV1 ABI drift");
_Static_assert(sizeof(RinFilePortalDurableEntryV1) == RIN_FILE_PORTAL_TOKEN_SIZE,
               "RinFilePortalDurableEntryV1 ABI drift");
_Static_assert(sizeof(RinFilePortalSettingsEntryV1) == RIN_FILE_PORTAL_TOKEN_SIZE,
               "RinFilePortalSettingsEntryV1 ABI drift");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_FILE_PORTAL_CALL_H */
