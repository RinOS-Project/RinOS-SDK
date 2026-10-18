/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Public list and revoke ABI for per-user platform WebAuthn credentials. */

#ifndef RIN_PASS_PLATFORM_ABI_H
#define RIN_PASS_PLATFORM_ABI_H

#include <stdint.h>

#define RIN_PASS_OPERATION_REMOVE_PLATFORM_PASSKEY 67u
#define RIN_PASS_OPERATION_LIST_PLATFORM_PASSKEYS  68u
#define RIN_PASS_INPUT_PLATFORM_PASSKEY_ID_V1      2u
#define RIN_PASS_PLATFORM_PASSKEY_ID_VERSION       1u
#define RIN_PASS_PLATFORM_PASSKEY_PAGE_VERSION     1u
#define RIN_PASS_PLATFORM_PASSKEY_PAGE_CAPACITY    8u
#define RIN_PASS_PLATFORM_PASSKEY_RP_ID_MAX       253u
#define RIN_PASS_PLATFORM_PASSKEY_CREDENTIAL_ID_MAX 128u

typedef struct RinPassPlatformPasskeyIdV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t credential_id_size;
    uint8_t credential_id[RIN_PASS_PLATFORM_PASSKEY_CREDENTIAL_ID_MAX];
    uint8_t reserved[8];
} RinPassPlatformPasskeyIdV1;

typedef struct RinPassPlatformPasskeyInfoV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t sign_count;
    uint16_t credential_id_size;
    uint16_t rp_id_size;
    uint8_t rp_id_hash[32];
    char rp_id[RIN_PASS_PLATFORM_PASSKEY_RP_ID_MAX + 1u];
    uint8_t credential_id[RIN_PASS_PLATFORM_PASSKEY_CREDENTIAL_ID_MAX];
} RinPassPlatformPasskeyInfoV1;

typedef struct RinPassPlatformPasskeyPageV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t entry_count;
    uint32_t first_ordinal;
    uint32_t total_count;
    RinPassPlatformPasskeyInfoV1
        entries[RIN_PASS_PLATFORM_PASSKEY_PAGE_CAPACITY];
} RinPassPlatformPasskeyPageV1;

#if defined(__cplusplus)
static_assert(sizeof(RinPassPlatformPasskeyIdV1) == 144,
              "Rin Pass platform passkey ID ABI drift");
static_assert(sizeof(RinPassPlatformPasskeyInfoV1) == 432,
              "Rin Pass platform passkey info ABI drift");
static_assert(sizeof(RinPassPlatformPasskeyPageV1) == 3472,
              "Rin Pass platform passkey page ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinPassPlatformPasskeyIdV1) == 144,
               "Rin Pass platform passkey ID ABI drift");
_Static_assert(sizeof(RinPassPlatformPasskeyInfoV1) == 432,
               "Rin Pass platform passkey info ABI drift");
_Static_assert(sizeof(RinPassPlatformPasskeyPageV1) == 3472,
               "Rin Pass platform passkey page ABI drift");
#endif

#endif /* RIN_PASS_PLATFORM_ABI_H */
