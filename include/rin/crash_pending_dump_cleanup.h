/* SPDX-License-Identifier: MIT */

#ifndef RIN_SDK_CRASH_PENDING_DUMP_CLEANUP_H
#define RIN_SDK_CRASH_PENDING_DUMP_CLEANUP_H

#include <stdint.h>

#include <rin/crash_service.h>

#define RIN_CRASH_SERVICE_OP_QUERY_PENDING_DUMP_CLEANUP UINT16_C(20)
#define RIN_CRASH_SERVICE_OP_PURGE_PENDING_DUMP_CLEANUP UINT16_C(21)

#define RIN_CRASH_SERVICE_PENDING_DUMP_CLEANUP_VERSION UINT16_C(1)
#define RIN_CRASH_SERVICE_PENDING_DUMP_CLEANUP_MAX_CANDIDATES UINT16_C(32)
#define RIN_CRASH_SERVICE_PENDING_DUMP_CLEANUP_FLAG_MORE UINT32_C(0x00000001)
#define RIN_CRASH_SERVICE_PENDING_DUMP_CLEANUP_FLAG_MASK \
    RIN_CRASH_SERVICE_PENDING_DUMP_CLEANUP_FLAG_MORE

/* Identifies one exact root-private dump inode and its canonical queue tuple.
 * This is an opaque cleanup token to callers; the runtime never displays the
 * filesystem identity fields. */
typedef struct RinCrashServicePendingDumpCleanupCandidateV1 {
    uint64_t boot_id_high;
    uint64_t boot_id_low;
    uint64_t queue_sequence;
    uint64_t file_device;
    uint64_t file_inode;
    uint64_t file_size;
    uint64_t file_mtime_seconds;
    uint64_t file_ctime_seconds;
    uint32_t reserved0;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t reserved3;
} RinCrashServicePendingDumpCleanupCandidateV1;

/* A consented, bounded snapshot of purgeable files. The first 32 candidates
 * are ordered by boot identity and queue sequence. When FLAG_MORE is set, a
 * successful purge removes this batch; the caller must review a new snapshot
 * before another batch. unattributed_count includes files which are unsafe
 * to purge automatically as well. */
typedef struct RinCrashServicePendingDumpCleanupStatusV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t candidate_count;
    uint32_t total_candidate_count;
    uint32_t unattributed_count;
    uint32_t flags;
    uint32_t reserved0;
    RinCrashServicePendingDumpCleanupCandidateV1 candidates[
        RIN_CRASH_SERVICE_PENDING_DUMP_CLEANUP_MAX_CANDIDATES];
    uint64_t reserved[2];
} RinCrashServicePendingDumpCleanupStatusV1;

/* Purge is bound to the complete snapshot shown to the administrator and
 * consumes a fresh kernel-issued physical-consent proof. */
#define RIN_CRASH_SERVICE_PENDING_DUMP_CLEANUP_REQUEST_VERSION UINT16_C(1)
typedef struct RinCrashServicePendingDumpCleanupRequestV1 {
    RinCrashConsentProofV1 proof;
    uint16_t version;
    uint16_t reserved0;
    uint32_t reserved1;
    RinCrashServicePendingDumpCleanupStatusV1 snapshot;
} RinCrashServicePendingDumpCleanupRequestV1;

typedef struct RinCrashServicePendingDumpCleanupResultV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint32_t removed_count;
    uint32_t reserved1;
} RinCrashServicePendingDumpCleanupResultV1;

#if defined(__cplusplus)
static_assert(sizeof(RinCrashServicePendingDumpCleanupCandidateV1) == 80u,
              "Pending dump cleanup candidate ABI drift");
static_assert(sizeof(RinCrashServicePendingDumpCleanupStatusV1) == 2600u,
              "Pending dump cleanup status ABI drift");
#else
_Static_assert(sizeof(RinCrashServicePendingDumpCleanupCandidateV1) == 80u,
               "Pending dump cleanup candidate ABI drift");
_Static_assert(sizeof(RinCrashServicePendingDumpCleanupStatusV1) == 2600u,
               "Pending dump cleanup status ABI drift");
#endif

#endif /* RIN_SDK_CRASH_PENDING_DUMP_CLEANUP_H */
