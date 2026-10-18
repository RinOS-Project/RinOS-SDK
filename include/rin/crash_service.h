/* SPDX-License-Identifier: MIT */

#ifndef RIN_SDK_CRASH_SERVICE_H
#define RIN_SDK_CRASH_SERVICE_H

#include <stdint.h>

#include <rin/crash_summary.h>


#define RIN_CRASH_SERVICE_ABI_VERSION UINT16_C(1)
#define RIN_CRASH_SERVICE_ID "crashd"
#define RIN_CRASH_SERVICE_SOCKET_PATH "/run/rin/crashd.sock"

#define RIN_CRASH_SERVICE_OP_STATUS UINT16_C(1)
#define RIN_CRASH_SERVICE_OP_READ   UINT16_C(2)
#define RIN_CRASH_SERVICE_OP_REGISTER_RECOVERY UINT16_C(3)
#define RIN_CRASH_SERVICE_OP_APPEND_DIAGNOSTIC UINT16_C(4)
#define RIN_CRASH_SERVICE_OP_HANDOFF_REPORT UINT16_C(5)
#define RIN_CRASH_SERVICE_OP_QUERY_RECOVERY_CRASH UINT16_C(6)
#define RIN_CRASH_SERVICE_OP_EXPORT_MINIDUMP UINT16_C(7)
#define RIN_CRASH_SERVICE_OP_READ_HISTORY UINT16_C(8)
#define RIN_CRASH_SERVICE_OP_PURGE_HISTORY UINT16_C(9)
#define RIN_CRASH_SERVICE_OP_LEGACY_HISTORY_STATUS UINT16_C(10)
#define RIN_CRASH_SERVICE_OP_PURGE_LEGACY_HISTORY UINT16_C(11)
#define RIN_CRASH_SERVICE_OP_DESKTOP_NOTICE UINT16_C(12)
#define RIN_CRASH_SERVICE_OP_READ_NOTICE UINT16_C(13)
#define RIN_CRASH_SERVICE_OP_EXPORT_NOTICE UINT16_C(14)
#define RIN_CRASH_SERVICE_MINIDUMP_MAX_BYTES UINT32_C(638)

#define RIN_CRASH_SERVICE_STATUS_CONSENT_REQUIRED INT32_C(-1000)

#define RIN_CRASH_SERVICE_FLAG_READ_REQUIRES_CONSENT UINT32_C(0x00000001)
#define RIN_CRASH_SERVICE_FLAG_CONSENT_BROKER_READY  UINT32_C(0x00000002)
#define RIN_CRASH_SERVICE_FLAG_RECOVERY_REGISTRATION UINT32_C(0x00000004)
#define RIN_CRASH_SERVICE_FLAG_DIAGNOSTIC_APPEND UINT32_C(0x00000008)
#define RIN_CRASH_SERVICE_FLAG_REPORT_HANDOFF UINT32_C(0x00000010)
#define RIN_CRASH_SERVICE_FLAG_MASK \
    (RIN_CRASH_SERVICE_FLAG_READ_REQUIRES_CONSENT | \
     RIN_CRASH_SERVICE_FLAG_CONSENT_BROKER_READY | \
     RIN_CRASH_SERVICE_FLAG_RECOVERY_REGISTRATION | \
     RIN_CRASH_SERVICE_FLAG_DIAGNOSTIC_APPEND | \
     RIN_CRASH_SERVICE_FLAG_REPORT_HANDOFF)

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

/* Session-scoped, content-free hint for the signed Desktop service. The boot
 * identity and sequence let Desktop deduplicate without receiving any crash
 * contents. A READ_NOTICE request must return this exact record after fresh
 * physical consent. */
#define RIN_CRASH_SERVICE_NOTICE_VERSION UINT16_C(1)
typedef struct RinCrashServiceNoticeV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint64_t boot_id_high;
    uint64_t boot_id_low;
    uint64_t sequence;
    uint64_t reserved[2];
} RinCrashServiceNoticeV1;

typedef struct RinCrashServiceNoticeReadV1 {
    RinCrashConsentProofV1 proof;
    RinCrashServiceNoticeV1 notice;
} RinCrashServiceNoticeReadV1;

/* Each history page consumes a separate one-shot local physical-consent
 * proof. The sequence cursor is stable across pages if the archive changes. */
#define RIN_CRASH_SERVICE_HISTORY_PAGE_MAX_RECORDS UINT16_C(8)
#define RIN_CRASH_SERVICE_HISTORY_READ_VERSION UINT16_C(1)
#define RIN_CRASH_SERVICE_HISTORY_PAGE_VERSION UINT16_C(1)
#define RIN_CRASH_SERVICE_HISTORY_PURGE_VERSION UINT16_C(1)
#define RIN_CRASH_SERVICE_HISTORY_PAGE_FLAG_GAP UINT32_C(0x00000001)
#define RIN_CRASH_SERVICE_HISTORY_PAGE_FLAG_ARCHIVE_FULL UINT32_C(0x00000002)
#define RIN_CRASH_SERVICE_HISTORY_PAGE_FLAG_MASK \
    (RIN_CRASH_SERVICE_HISTORY_PAGE_FLAG_GAP | \
     RIN_CRASH_SERVICE_HISTORY_PAGE_FLAG_ARCHIVE_FULL)

typedef struct RinCrashServiceHistoryReadV1 {
    RinCrashConsentProofV1 proof;
    uint64_t after_sequence;
    uint16_t capacity;
    uint16_t version;
    uint32_t reserved;
} RinCrashServiceHistoryReadV1;

typedef struct RinCrashServiceHistoryPageV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t record_count;
    uint32_t flags;
    uint32_t reserved;
    uint64_t after_sequence;
    uint64_t gap_before_sequence;
    uint64_t next_sequence;
    RinCrashSummaryV1 records[
        RIN_CRASH_SERVICE_HISTORY_PAGE_MAX_RECORDS];
} RinCrashServiceHistoryPageV1;

/* Purge removes only records authorized for the authenticated owner and
 * requires a fresh physical-consent proof for this exact request. */
typedef struct RinCrashServiceHistoryPurgeV1 {
    RinCrashConsentProofV1 proof;
    uint64_t through_sequence;
    uint16_t version;
    uint16_t reserved0;
    uint32_t reserved1;
} RinCrashServiceHistoryPurgeV1;

/* The status contains a count and sequence high-water only; it never exposes
 * crash contents. A system administrator displays this exact count before
 * invoking the separate, physically consented purge operation. */
#define RIN_CRASH_SERVICE_LEGACY_HISTORY_STATUS_VERSION UINT16_C(1)
#define RIN_CRASH_SERVICE_LEGACY_HISTORY_PURGE_VERSION UINT16_C(1)
#define RIN_CRASH_SERVICE_LEGACY_HISTORY_MAX_RECORDS UINT32_C(64)
typedef struct RinCrashServiceLegacyHistoryStatusQueryV1 {
    uint16_t version;
    uint16_t reserved0;
    uint32_t reserved1;
} RinCrashServiceLegacyHistoryStatusQueryV1;

typedef struct RinCrashServiceLegacyHistoryStatusV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint32_t inaccessible_record_count;
    uint32_t reserved1;
    uint64_t through_sequence;
} RinCrashServiceLegacyHistoryStatusV1;

/* The expected count and sequence bind the user's confirmation to the status
 * shown immediately before purge. The kernel consent proof is fresh and bound
 * to this exact request and authenticated connection. */
typedef struct RinCrashServiceLegacyHistoryPurgeV1 {
    RinCrashConsentProofV1 proof;
    uint64_t through_sequence;
    uint32_t expected_inaccessible_record_count;
    uint16_t version;
    uint16_t reserved;
} RinCrashServiceLegacyHistoryPurgeV1;

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

/* A fatal signal handler may not connect, allocate, wait, or cross an
 * untrusted path boundary.  CoreCLR therefore opens the authenticated crashd
 * stream during startup and writes this fixed-size, path-free notice after
 * its signal-safe report has been finalized.  The peer identity is captured
 * again by the kernel on the service connection; the duplicated process
 * identity below is an anti-confusion check, not an authority. */
#define RIN_CRASH_HANDOFF_VERSION UINT16_C(1)
#define RIN_CRASH_HANDOFF_FLAG_REPORT_FILE_READY UINT16_C(0x0001)
#define RIN_CRASH_HANDOFF_FLAG_KERNEL_SUMMARY_EXPECTED UINT16_C(0x0002)
#define RIN_CRASH_HANDOFF_FLAG_MASK \
    (RIN_CRASH_HANDOFF_FLAG_REPORT_FILE_READY | \
     RIN_CRASH_HANDOFF_FLAG_KERNEL_SUMMARY_EXPECTED)

typedef struct RinCrashReportHandoffV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t signal;
    uint32_t reserved0;
    uint64_t process_id;
    uint64_t process_instance_cookie;
    uint64_t reserved[2];
} RinCrashReportHandoffV1;

#if defined(__cplusplus)
static_assert(sizeof(RinCrashServiceMessageHeaderV1) == 32u,
              "RinCrashServiceMessageHeaderV1 ABI drift");
static_assert(sizeof(RinCrashServiceStatusV1) == 32u,
              "RinCrashServiceStatusV1 ABI drift");
static_assert(sizeof(RinCrashConsentProofV1) == 128u,
              "RinCrashConsentProofV1 ABI drift");
static_assert(sizeof(RinCrashServiceNoticeV1) == 48u,
              "RinCrashServiceNoticeV1 ABI drift");
static_assert(sizeof(RinCrashServiceNoticeReadV1) == 176u,
              "RinCrashServiceNoticeReadV1 ABI drift");
static_assert(sizeof(RinCrashServiceHistoryReadV1) == 144u,
              "RinCrashServiceHistoryReadV1 ABI drift");
static_assert(sizeof(RinCrashServiceHistoryPageV1) ==
                  40u + sizeof(RinCrashSummaryV1) *
                            RIN_CRASH_SERVICE_HISTORY_PAGE_MAX_RECORDS,
              "RinCrashServiceHistoryPageV1 ABI drift");
static_assert(sizeof(RinCrashServiceHistoryPurgeV1) == 144u,
              "RinCrashServiceHistoryPurgeV1 ABI drift");
static_assert(sizeof(RinCrashServiceLegacyHistoryStatusQueryV1) == 8u,
              "RinCrashServiceLegacyHistoryStatusQueryV1 ABI drift");
static_assert(sizeof(RinCrashServiceLegacyHistoryStatusV1) == 24u,
              "RinCrashServiceLegacyHistoryStatusV1 ABI drift");
static_assert(sizeof(RinCrashServiceLegacyHistoryPurgeV1) == 144u,
              "RinCrashServiceLegacyHistoryPurgeV1 ABI drift");
static_assert(sizeof(RinCrashRecoveryRegistrationV1) == 152u,
              "RinCrashRecoveryRegistrationV1 ABI drift");
static_assert(sizeof(RinCrashDiagnosticLogV1) == 288u,
              "RinCrashDiagnosticLogV1 ABI drift");
static_assert(sizeof(RinCrashReportHandoffV1) == 48u,
              "RinCrashReportHandoffV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinCrashServiceMessageHeaderV1) == 32u,
               "RinCrashServiceMessageHeaderV1 ABI drift");
_Static_assert(sizeof(RinCrashServiceStatusV1) == 32u,
               "RinCrashServiceStatusV1 ABI drift");
_Static_assert(sizeof(RinCrashConsentProofV1) == 128u,
               "RinCrashConsentProofV1 ABI drift");
_Static_assert(sizeof(RinCrashServiceNoticeV1) == 48u,
               "RinCrashServiceNoticeV1 ABI drift");
_Static_assert(sizeof(RinCrashServiceNoticeReadV1) == 176u,
               "RinCrashServiceNoticeReadV1 ABI drift");
_Static_assert(sizeof(RinCrashServiceHistoryReadV1) == 144u,
               "RinCrashServiceHistoryReadV1 ABI drift");
_Static_assert(sizeof(RinCrashServiceHistoryPageV1) ==
                   40u + sizeof(RinCrashSummaryV1) *
                             RIN_CRASH_SERVICE_HISTORY_PAGE_MAX_RECORDS,
               "RinCrashServiceHistoryPageV1 ABI drift");
_Static_assert(sizeof(RinCrashServiceHistoryPurgeV1) == 144u,
               "RinCrashServiceHistoryPurgeV1 ABI drift");
_Static_assert(sizeof(RinCrashServiceLegacyHistoryStatusQueryV1) == 8u,
               "RinCrashServiceLegacyHistoryStatusQueryV1 ABI drift");
_Static_assert(sizeof(RinCrashServiceLegacyHistoryStatusV1) == 24u,
               "RinCrashServiceLegacyHistoryStatusV1 ABI drift");
_Static_assert(sizeof(RinCrashServiceLegacyHistoryPurgeV1) == 144u,
               "RinCrashServiceLegacyHistoryPurgeV1 ABI drift");
_Static_assert(sizeof(RinCrashRecoveryRegistrationV1) == 152u,
               "RinCrashRecoveryRegistrationV1 ABI drift");
_Static_assert(sizeof(RinCrashDiagnosticLogV1) == 288u,
               "RinCrashDiagnosticLogV1 ABI drift");
_Static_assert(sizeof(RinCrashReportHandoffV1) == 48u,
               "RinCrashReportHandoffV1 ABI drift");
#endif

#endif /* RIN_SDK_CRASH_SERVICE_H */
