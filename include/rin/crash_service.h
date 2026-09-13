/* SPDX-License-Identifier: MIT */

#ifndef RIN_SDK_CRASH_SERVICE_H
#define RIN_SDK_CRASH_SERVICE_H

#include <stdint.h>


#define RIN_CRASH_SERVICE_ABI_VERSION UINT16_C(1)
#define RIN_CRASH_SERVICE_ID "crashd"
#define RIN_CRASH_SERVICE_SOCKET_PATH "/run/rin/crashd.sock"

#define RIN_CRASH_SERVICE_OP_STATUS UINT16_C(1)
#define RIN_CRASH_SERVICE_OP_READ   UINT16_C(2)
#define RIN_CRASH_SERVICE_OP_REGISTER_RECOVERY UINT16_C(3)
#define RIN_CRASH_SERVICE_OP_APPEND_DIAGNOSTIC UINT16_C(4)

#define RIN_CRASH_SERVICE_STATUS_CONSENT_REQUIRED INT32_C(-1000)

#define RIN_CRASH_SERVICE_FLAG_READ_REQUIRES_CONSENT UINT32_C(0x00000001)
#define RIN_CRASH_SERVICE_FLAG_CONSENT_BROKER_READY  UINT32_C(0x00000002)
#define RIN_CRASH_SERVICE_FLAG_RECOVERY_REGISTRATION UINT32_C(0x00000004)
#define RIN_CRASH_SERVICE_FLAG_DIAGNOSTIC_APPEND UINT32_C(0x00000008)
#define RIN_CRASH_SERVICE_FLAG_MASK \
    (RIN_CRASH_SERVICE_FLAG_READ_REQUIRES_CONSENT | \
     RIN_CRASH_SERVICE_FLAG_CONSENT_BROKER_READY | \
     RIN_CRASH_SERVICE_FLAG_RECOVERY_REGISTRATION | \
     RIN_CRASH_SERVICE_FLAG_DIAGNOSTIC_APPEND)

#define RIN_CRASH_CONSENT_FLAG_LOCAL_PHYSICAL_GESTURE UINT16_C(0x0001)
#define RIN_CRASH_CONSENT_FLAG_MASK \
    RIN_CRASH_CONSENT_FLAG_LOCAL_PHYSICAL_GESTURE

typedef struct RinCrashServiceMessageHeaderV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t opcode;
    uint64_t request_id;
    uint32_t payload_size;
    uint32_t flags;
    int32_t status;
    uint32_t reserved;
} RinCrashServiceMessageHeaderV1;

/* STATUS reveals protocol readiness only. In particular, it does not reveal
 * whether a crash record exists. */
typedef struct RinCrashServiceStatusV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint32_t flags;
    uint32_t max_summary_size;
    uint64_t reserved[2];
} RinCrashServiceStatusV1;

/* Opaque proof minted by a future kernel consent broker after a trusted local
 * physical gesture. crashd binds it to the exact signed peer connection and
 * request before asking the broker to authenticate and consume it. */
typedef struct RinCrashConsentProofV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t request_id;
    uint64_t peer_process_id;
    uint64_t peer_process_instance_cookie;
    uint64_t peer_connection_id;
    uint64_t expires_at_ns;
    uint8_t nonce[32];
    uint8_t authenticator[32];
    uint64_t reserved[2];
} RinCrashConsentProofV1;

/* Registration stores only authenticated identity and opaque document
 * metadata. Recovery plaintext and filesystem paths remain in the owning
 * application's private state directory. */
#define RIN_CRASH_RECOVERY_REGISTRATION_VERSION UINT16_C(1)
#define RIN_CRASH_RECOVERY_APPLICATION_ID_SIZE 32u
#define RIN_CRASH_RECOVERY_PACKAGE_DIGEST_SIZE 32u
#define RIN_CRASH_RECOVERY_DOCUMENT_ID_SIZE 32u
#define RIN_CRASH_RECOVERY_REASON_CRASH UINT16_C(1)
#define RIN_CRASH_RECOVERY_REASON_LOGOUT_OR_REBOOT UINT16_C(2)

typedef struct RinCrashRecoveryRegistrationV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reason;
    uint64_t package_generation;
    uint64_t document_generation;
    uint64_t snapshot_generation;
    uint64_t saved_at_ns;
    uint8_t application_id[RIN_CRASH_RECOVERY_APPLICATION_ID_SIZE];
    uint8_t package_digest[RIN_CRASH_RECOVERY_PACKAGE_DIGEST_SIZE];
    uint8_t document_id[RIN_CRASH_RECOVERY_DOCUMENT_ID_SIZE];
    uint64_t reserved[2];
} RinCrashRecoveryRegistrationV1;

/* Application diagnostics are intentionally a separate, redaction-marked
 * record. No path, descriptor, credential, or arbitrary binary payload can
 * cross the crashd boundary. */
#define RIN_CRASH_DIAGNOSTIC_VERSION UINT16_C(1)
#define RIN_CRASH_DIAGNOSTIC_FLAG_REDACTED UINT16_C(0x0001)
#define RIN_CRASH_DIAGNOSTIC_FLAG_MASK RIN_CRASH_DIAGNOSTIC_FLAG_REDACTED
#define RIN_CRASH_DIAGNOSTIC_MAX_MESSAGE UINT32_C(256)

typedef struct RinCrashDiagnosticLogV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t message_size;
    char message[RIN_CRASH_DIAGNOSTIC_MAX_MESSAGE + 1u];
    uint64_t reserved[2];
} RinCrashDiagnosticLogV1;

#if defined(__cplusplus)
static_assert(sizeof(RinCrashServiceMessageHeaderV1) == 32u,
              "RinCrashServiceMessageHeaderV1 ABI drift");
static_assert(sizeof(RinCrashServiceStatusV1) == 32u,
              "RinCrashServiceStatusV1 ABI drift");
static_assert(sizeof(RinCrashConsentProofV1) == 128u,
              "RinCrashConsentProofV1 ABI drift");
static_assert(sizeof(RinCrashRecoveryRegistrationV1) == 152u,
              "RinCrashRecoveryRegistrationV1 ABI drift");
static_assert(sizeof(RinCrashDiagnosticLogV1) == 288u,
              "RinCrashDiagnosticLogV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinCrashServiceMessageHeaderV1) == 32u,
               "RinCrashServiceMessageHeaderV1 ABI drift");
_Static_assert(sizeof(RinCrashServiceStatusV1) == 32u,
               "RinCrashServiceStatusV1 ABI drift");
_Static_assert(sizeof(RinCrashConsentProofV1) == 128u,
               "RinCrashConsentProofV1 ABI drift");
_Static_assert(sizeof(RinCrashRecoveryRegistrationV1) == 152u,
               "RinCrashRecoveryRegistrationV1 ABI drift");
_Static_assert(sizeof(RinCrashDiagnosticLogV1) == 288u,
               "RinCrashDiagnosticLogV1 ABI drift");
#endif

#endif /* RIN_SDK_CRASH_SERVICE_H */


