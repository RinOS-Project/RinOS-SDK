/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_API_RIN_CRYPT_H
#define RIN_API_RIN_CRYPT_H

#include <stdint.h>

#define RIN_CRYPT_ABI_VERSION UINT32_C(0x00010000)
#define RIN_CRYPT_VOLUME_ID_SIZE 16u
#define RIN_CRYPT_KEYSLOT_ID_SIZE 16u
#define RIN_CRYPT_XTS_256_KEY_SIZE 64u
#define RIN_CRYPT_MIN_DATA_UNIT_SIZE 512u
#define RIN_CRYPT_MAX_DATA_UNIT_SIZE 65536u

typedef enum RinCryptResult {
    RIN_CRYPT_OK = 0,
    RIN_CRYPT_INVALID = -1,
    RIN_CRYPT_LOCKED = -2,
    RIN_CRYPT_KEY_REJECTED = -3,
    RIN_CRYPT_NOT_SUPPORTED = -4,
    RIN_CRYPT_RANGE = -5,
    RIN_CRYPT_INACTIVE = -6
} RinCryptResult;

typedef enum RinCryptPurpose {
    RIN_CRYPT_PURPOSE_VOLUME = 1,
    RIN_CRYPT_PURPOSE_SWAP = 2
} RinCryptPurpose;

/* Flags describe evidence produced by the trusted key provider, not a request
 * that a caller may self-assert. */
#define RIN_CRYPT_KEY_ARGON2ID_DERIVED       UINT32_C(0x00000001)
#define RIN_CRYPT_KEY_KEYSLOT_AUTHENTICATED UINT32_C(0x00000002)
#define RIN_CRYPT_KEY_EPHEMERAL_BOOT         UINT32_C(0x00000004)
#define RIN_CRYPT_KEY_CSPRNG_GENERATED       UINT32_C(0x00000008)
#define RIN_CRYPT_KEY_TPM2_POLICY_AUTHENTICATED UINT32_C(0x00000010)
#define RIN_CRYPT_KEY_FLAG_MASK                                      \
    (RIN_CRYPT_KEY_ARGON2ID_DERIVED |                                \
     RIN_CRYPT_KEY_KEYSLOT_AUTHENTICATED |                           \
     RIN_CRYPT_KEY_EPHEMERAL_BOOT | RIN_CRYPT_KEY_CSPRNG_GENERATED | \
     RIN_CRYPT_KEY_TPM2_POLICY_AUTHENTICATED)

#define RIN_CRYPT_KEY_REQUEST_VERSION UINT16_C(1)

typedef struct RinCryptKeyRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t purpose;
    uint8_t volume_id[RIN_CRYPT_VOLUME_ID_SIZE];
    uint8_t keyslot_id[RIN_CRYPT_KEYSLOT_ID_SIZE];
} RinCryptKeyRequestV1;

#if defined(__cplusplus)
static_assert(sizeof(RinCryptKeyRequestV1) == 40u,
              "RinCrypt key request ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinCryptKeyRequestV1) == 40u,
               "RinCrypt key request ABI drift");
#endif

#endif /* RIN_API_RIN_CRYPT_H */
