/* SPDX-License-Identifier: MIT */
/* Authenticated, identity-bound grants for user-selected file objects. */

#ifndef RIN_SDK_FILE_PORTAL_H
#define RIN_SDK_FILE_PORTAL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_FILE_PORTAL_TOKEN_MAGIC UINT32_C(0x54504652) /* "RFPT" */
#define RIN_FILE_PORTAL_VERSION UINT32_C(0x00010000)
#define RIN_FILE_PORTAL_TOKEN_VERSION UINT16_C(1)
#define RIN_FILE_PORTAL_TOKEN_SIZE UINT16_C(208)
#define RIN_FILE_PORTAL_PACKAGE_NAME_SIZE 64u
#define RIN_FILE_PORTAL_PUBLISHER_ID_SIZE 32u
#define RIN_FILE_PORTAL_NONCE_SIZE 16u
#define RIN_FILE_PORTAL_KEY_SIZE 32u
#define RIN_FILE_PORTAL_TAG_SIZE 32u

#define RIN_FILE_PORTAL_RIGHT_READ     (UINT32_C(1) << 0)
#define RIN_FILE_PORTAL_RIGHT_WRITE    (UINT32_C(1) << 1)
#define RIN_FILE_PORTAL_RIGHT_METADATA (UINT32_C(1) << 2)
#define RIN_FILE_PORTAL_RIGHT_ALL                                      \
    (RIN_FILE_PORTAL_RIGHT_READ | RIN_FILE_PORTAL_RIGHT_WRITE |        \
     RIN_FILE_PORTAL_RIGHT_METADATA)

typedef struct __attribute__((packed)) RinFilePortalIdentityV1 {
    uint32_t uid;
    uint32_t publisher_key_generation;
    char package_name[RIN_FILE_PORTAL_PACKAGE_NAME_SIZE];
    uint8_t publisher_key_id[RIN_FILE_PORTAL_PUBLISHER_ID_SIZE];
} RinFilePortalIdentityV1;

typedef struct __attribute__((packed)) RinFilePortalIssueRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    RinFilePortalIdentityV1 identity;
    uint32_t rights;
    uint32_t flags;
    uint64_t file_object_id;
    uint64_t not_before_epoch;
    uint64_t expires_at_epoch;
    uint64_t policy_generation;
    uint8_t nonce[RIN_FILE_PORTAL_NONCE_SIZE];
    uint8_t reserved[8];
} RinFilePortalIssueRequestV1;

typedef struct __attribute__((packed)) RinFilePortalTokenV1 {
    uint32_t magic;
    uint16_t version;
    uint16_t token_size;
    RinFilePortalIdentityV1 identity;
    uint32_t rights;
    uint32_t flags;
    uint64_t file_object_id;
    uint64_t not_before_epoch;
    uint64_t expires_at_epoch;
    uint64_t policy_generation;
    uint8_t nonce[RIN_FILE_PORTAL_NONCE_SIZE];
    uint8_t reserved[8];
    uint8_t authentication_tag[RIN_FILE_PORTAL_TAG_SIZE];
} RinFilePortalTokenV1;

typedef struct __attribute__((packed)) RinFilePortalOpenRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    RinFilePortalIdentityV1 identity;
    uint32_t requested_rights;
    uint32_t reserved0;
    uint64_t current_epoch;
    uint64_t minimum_policy_generation;
    uint8_t reserved[16];
} RinFilePortalOpenRequestV1;

typedef struct __attribute__((packed)) RinFilePortalOpenResultV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t granted_rights;
    uint32_t reserved0;
    uint64_t file_object_id;
    uint64_t expires_at_epoch;
} RinFilePortalOpenResultV1;

typedef enum RinFilePortalStatus {
    RIN_FILE_PORTAL_OK = 0,
    RIN_FILE_PORTAL_INVALID_ARGUMENT = -1,
    RIN_FILE_PORTAL_BAD_FORMAT = -2,
    RIN_FILE_PORTAL_INTEGRITY_FAILED = -3,
    RIN_FILE_PORTAL_IDENTITY_MISMATCH = -4,
    RIN_FILE_PORTAL_RIGHTS_DENIED = -5,
    RIN_FILE_PORTAL_NOT_YET_VALID = -6,
    RIN_FILE_PORTAL_EXPIRED = -7,
    RIN_FILE_PORTAL_REVOKED = -8
} RinFilePortalStatus;

/* Shared identity validation prevents sandbox and portal policy from
 * interpreting package identity differently. */
int rin_file_portal_identity_valid(const RinFilePortalIdentityV1* identity);
int rin_file_portal_identity_equal(const RinFilePortalIdentityV1* left,
                                   const RinFilePortalIdentityV1* right);

/* The portal must resolve a user-selected path to a stable file object before
 * issuing a token. The path itself is intentionally absent from this ABI. */
RinFilePortalStatus rin_file_portal_issue(
    const RinFilePortalIssueRequestV1* request,
    const uint8_t authentication_key[RIN_FILE_PORTAL_KEY_SIZE],
    RinFilePortalTokenV1* token);

/* Every failure initializes result to a zero grant. minimum_policy_generation
 * provides immediate revocation without accepting a rolled-back policy DB. */
RinFilePortalStatus rin_file_portal_authorize(
    const RinFilePortalTokenV1* token,
    const RinFilePortalOpenRequestV1* request,
    const uint8_t authentication_key[RIN_FILE_PORTAL_KEY_SIZE],
    RinFilePortalOpenResultV1* result);

#if defined(__cplusplus)
static_assert(sizeof(RinFilePortalIdentityV1) == 104u,
              "RinFilePortalIdentityV1 ABI drift");
static_assert(sizeof(RinFilePortalIssueRequestV1) == 176u,
              "RinFilePortalIssueRequestV1 ABI drift");
static_assert(sizeof(RinFilePortalTokenV1) == RIN_FILE_PORTAL_TOKEN_SIZE,
              "RinFilePortalTokenV1 ABI drift");
static_assert(sizeof(RinFilePortalOpenRequestV1) == 152u,
              "RinFilePortalOpenRequestV1 ABI drift");
static_assert(sizeof(RinFilePortalOpenResultV1) == 32u,
              "RinFilePortalOpenResultV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinFilePortalIdentityV1) == 104u,
               "RinFilePortalIdentityV1 ABI drift");
_Static_assert(sizeof(RinFilePortalIssueRequestV1) == 176u,
               "RinFilePortalIssueRequestV1 ABI drift");
_Static_assert(sizeof(RinFilePortalTokenV1) == RIN_FILE_PORTAL_TOKEN_SIZE,
               "RinFilePortalTokenV1 ABI drift");
_Static_assert(sizeof(RinFilePortalOpenRequestV1) == 152u,
               "RinFilePortalOpenRequestV1 ABI drift");
_Static_assert(sizeof(RinFilePortalOpenResultV1) == 32u,
               "RinFilePortalOpenResultV1 ABI drift");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_FILE_PORTAL_H */


