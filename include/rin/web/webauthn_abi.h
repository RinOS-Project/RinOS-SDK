/* SPDX-License-Identifier: MIT */
/* Public WebAuthn request record embedded by browser-facing protocols. */
#ifndef RIN_SDK_WEB_WEBAUTHN_ABI_H
#define RIN_SDK_WEB_WEBAUTHN_ABI_H

#include <stdint.h>

#define RIN_PASS_WEBAUTHN_RP_ID_STORAGE 256u
#define RIN_PASS_WEBAUTHN_RESULT_STREAM_MAX 0x1170u

#ifndef RIN_SDK_WEBAUTHN_ABI_DEFINED
#define RIN_SDK_WEBAUTHN_ABI_DEFINED
typedef struct RinPassWebAuthnRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    uint32_t authenticator;
    uint32_t user_verification;
    uint32_t resident_key;
    uint32_t attestation;
    uint32_t credential_count;
    uint32_t algorithm_count;
    uint32_t origin_size;
    uint32_t rp_id_size;
    uint32_t rp_name_size;
    uint32_t user_id_size;
    uint32_t user_name_size;
    uint32_t user_display_name_size;
    uint32_t reserved0;
    uint8_t client_data_hash[32];
    uint8_t origin[320];
    uint8_t rp_id[256];
    uint8_t rp_name[128];
    uint8_t user_id[64];
    uint8_t user_name[128];
    uint8_t user_display_name[128];
    uint16_t credential_sizes[8];
    uint16_t credential_transports[8];
    int32_t algorithms[8];
    uint8_t credential_ids[1024];
    uint64_t reserved[4];
} RinPassWebAuthnRequestV1;

#if defined(__cplusplus)
static_assert(sizeof(RinPassWebAuthnRequestV1) == 2240u,
              "RinPassWebAuthnRequestV1 ABI drift");
#else
_Static_assert(sizeof(RinPassWebAuthnRequestV1) == 2240u,
               "RinPassWebAuthnRequestV1 ABI drift");
#endif
#endif /* RIN_SDK_WEBAUTHN_ABI_DEFINED */

#endif
