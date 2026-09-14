/* SPDX-License-Identifier: MIT */
/* Public keyring service wire ABI.  Master-key and audit-owner contracts
 * remain OS-Core private; this header contains only caller-visible frames. */
#ifndef RIN_SDK_KEYRING_H
#define RIN_SDK_KEYRING_H

#include <stdint.h>

#define RIN_KEYRING_ABI_VERSION UINT32_C(0x00010000)
#define RIN_KEYRING_SOCKET_PATH "/run/rin/keyring"
#define RIN_KEYRING_SERVICE_ID "keyring"
#define RIN_KEYRING_SERVICE_SCOPE UINT16_C(1)
#define RIN_KEYRING_SERVICE_OWNER_UID UINT32_C(0)

#define RIN_KEYRING_MAX_SECRET_NAME 96u
#define RIN_KEYRING_MAX_SECRET_SIZE 4096u

#define RIN_KEYRING_GENERATION_CREATE_ONLY UINT64_C(0)
#define RIN_KEYRING_GENERATION_ANY UINT64_MAX
#define RIN_KEYRING_SCOPE_MAX 64u

typedef enum RinKeyringSecretPurposeV1 {
    RIN_KEYRING_PURPOSE_GENERIC = 0,
    RIN_KEYRING_PURPOSE_LOGIN = 1,
    RIN_KEYRING_PURPOSE_OAUTH = 2,
    RIN_KEYRING_PURPOSE_API_TOKEN = 3,
    RIN_KEYRING_PURPOSE_CREDENTIAL = 4
} RinKeyringSecretPurposeV1;

typedef enum RinKeyringResult {
    RIN_KEYRING_OK = 0,
    RIN_KEYRING_INVALID = -1,
    RIN_KEYRING_DENIED = -2,
    RIN_KEYRING_LOCKED = -3,
    RIN_KEYRING_NOT_FOUND = -4,
    RIN_KEYRING_CONFLICT = -5,
    RIN_KEYRING_STORAGE_FAILED = -6,
    RIN_KEYRING_INTEGRITY_FAILED = -7,
    RIN_KEYRING_ENTROPY_UNAVAILABLE = -8,
    RIN_KEYRING_TOO_LARGE = -9,
    RIN_KEYRING_REPLAY = -10,
    RIN_KEYRING_NOT_SUPPORTED = -11
} RinKeyringResult;

typedef enum RinKeyringOpcode {
    RIN_KEYRING_OP_PUT = 1,
    RIN_KEYRING_OP_GET = 2,
    RIN_KEYRING_OP_REMOVE = 3,
    RIN_KEYRING_OP_STATUS = 4,
    RIN_KEYRING_OP_ACQUIRE_HANDLE = 5,
    RIN_KEYRING_OP_GET_HANDLE = 6,
    RIN_KEYRING_OP_REMOVE_HANDLE = 7
} RinKeyringOpcode;

#define RIN_KEYRING_HANDLE_SIZE 72u
typedef struct RinKeyringHandleV1 {
    uint8_t storage_id[32];
    uint64_t generation;
    uint8_t authenticator[32];
} RinKeyringHandleV1;

typedef struct RinKeyringHandleRequestV1 {
    uint32_t handle_size;
    uint32_t reserved;
} RinKeyringHandleRequestV1;

/* Fixed-width frame header. Variable payload bytes follow this structure. */
typedef struct RinKeyringMessageHeaderV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t opcode;
    uint64_t request_id;
    uint32_t payload_size;
    uint32_t flags;
    int32_t status;
    uint32_t reserved;
} RinKeyringMessageHeaderV1;

typedef struct RinKeyringPutRequestV1 {
    uint64_t expected_generation;
    uint32_t secret_name_size;
    uint32_t secret_size;
} RinKeyringPutRequestV1;

typedef struct RinKeyringGetRequestV1 {
    uint32_t secret_name_size;
    uint32_t reserved;
} RinKeyringGetRequestV1;

typedef struct RinKeyringRemoveRequestV1 {
    uint64_t expected_generation;
    uint32_t secret_name_size;
    uint32_t reserved;
} RinKeyringRemoveRequestV1;

typedef struct RinKeyringSecretResponseV1 {
    uint64_t generation;
    uint32_t secret_size;
    uint32_t reserved;
} RinKeyringSecretResponseV1;

#if defined(__cplusplus)
static_assert(sizeof(RinKeyringHandleV1) == 72u,
              "RinKeyring handle ABI drift");
static_assert(sizeof(RinKeyringHandleRequestV1) == 8u,
              "RinKeyring handle request ABI drift");
static_assert(sizeof(RinKeyringMessageHeaderV1) == 32u,
              "RinKeyring message header ABI drift");
static_assert(sizeof(RinKeyringPutRequestV1) == 16u,
              "RinKeyring put request ABI drift");
static_assert(sizeof(RinKeyringGetRequestV1) == 8u,
              "RinKeyring get request ABI drift");
static_assert(sizeof(RinKeyringRemoveRequestV1) == 16u,
              "RinKeyring remove request ABI drift");
static_assert(sizeof(RinKeyringSecretResponseV1) == 16u,
              "RinKeyring response ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinKeyringHandleV1) == 72u,
               "RinKeyring handle ABI drift");
_Static_assert(sizeof(RinKeyringHandleRequestV1) == 8u,
               "RinKeyring handle request ABI drift");
_Static_assert(sizeof(RinKeyringMessageHeaderV1) == 32u,
               "RinKeyring message header ABI drift");
_Static_assert(sizeof(RinKeyringPutRequestV1) == 16u,
               "RinKeyring put request ABI drift");
_Static_assert(sizeof(RinKeyringGetRequestV1) == 8u,
               "RinKeyring get request ABI drift");
_Static_assert(sizeof(RinKeyringRemoveRequestV1) == 16u,
               "RinKeyring remove request ABI drift");
_Static_assert(sizeof(RinKeyringSecretResponseV1) == 16u,
               "RinKeyring response ABI drift");
#endif

#endif /* RIN_SDK_KEYRING_H */
