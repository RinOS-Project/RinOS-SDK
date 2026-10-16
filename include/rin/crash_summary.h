/* SPDX-License-Identifier: MIT */

#ifndef RIN_API_RIN_CRASH_SUMMARY_H
#define RIN_API_RIN_CRASH_SUMMARY_H

#include <stdint.h>

#include <rin/crash_record.h>

#define RIN_CRASH_SUMMARY_VERSION UINT16_C(2)
#define RIN_CRASH_SUMMARY_POLICY_LOCAL_ONLY UINT32_C(1)
#define RIN_CRASH_HISTORY_QUERY_VERSION UINT16_C(2)
#define RIN_CRASH_HISTORY_ACK_VERSION UINT16_C(1)
#define RIN_CRASH_HISTORY_MAX_RECORDS UINT16_C(2)
#define RIN_CRASH_PENDING_QUERY_VERSION UINT16_C(2)
#define RIN_CRASH_PENDING_MAX_RECORDS UINT16_C(1)

/* Pointer-free, bounded model for a future local diagnostic viewer. This
 * structure grants no authority to transmit or delete its source record. */
typedef struct RinCrashSummaryV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved;
    uint32_t disclosure_policy;
    uint32_t kind;
    uint32_t architecture;
    uint32_t process_id;
    uint32_t thread_id;
    uint32_t exception_code;
    uint32_t message_size;
    uint64_t sequence;
    uint64_t timestamp_ns;
    uint64_t boot_id_high;
    uint64_t boot_id_low;
    char message[RIN_CRASH_RECORD_MAX_MESSAGE + 1u];
    uint32_t stack_frame_count;
    uint32_t has_process;
    RinCrashRegistersX64 registers;
    uint64_t stack_frames[RIN_CRASH_RECORD_MAX_FRAMES];
    RinCrashProcessInfo process;
    uint32_t has_scheduler;
    RinCrashSchedulerInfo scheduler;
} RinCrashSummaryV1;

/* Cursor-based, bounded history page. The kernel returns up to capacity
 * retained summaries strictly newer than after_sequence in ascending order.
 * To read a larger retained history, repeat with the last returned sequence. */
typedef struct RinCrashHistoryQueryV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t capacity;
    uint32_t record_count;
    uint32_t reserved;
    uint64_t after_sequence;
    RinCrashSummaryV1 records[RIN_CRASH_HISTORY_MAX_RECORDS];
} RinCrashHistoryQueryV1;

/* Durable cursor update issued only after crashd has committed its recovery
 * registry. This acknowledges retained history; it does not erase raw data. */
typedef struct RinCrashHistoryAckV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved;
    uint64_t sequence;
} RinCrashHistoryAckV1;

/* Read-only view of snapshots retained in the volatile pre-drain queue after
 * a storage failure. queue_sequence belongs only to this queue; summary.sequence
 * is zero and must never be sent to the durable history ACK interface. The
 * boot ID plus queue_sequence identifies an entry while crashd moves it to
 * durable local recovery storage. */
typedef struct RinCrashPendingMinidumpV1 {
    uint64_t queue_sequence;
    RinCrashSummaryV1 summary;
} RinCrashPendingMinidumpV1;

typedef struct RinCrashPendingQueryV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t capacity;
    uint32_t record_count;
    uint32_t reserved;
    uint64_t after_queue_sequence;
    RinCrashPendingMinidumpV1 records[RIN_CRASH_PENDING_MAX_RECORDS];
} RinCrashPendingQueryV1;

#ifdef __cplusplus
extern "C" {
#endif

RinCrashRecordResult rin_crash_summary_decode(
    const void* record, uint32_t record_size, RinCrashSummaryV1* summary);

/* RinRuntime entry point. The kernel admits only a signed system
 * service holding RIN_CAP_CRASH_DIAGNOSTIC and clears summary on failure. */
int rin_crash_summary_get_local(RinCrashSummaryV1* summary);

/* Available to the same kernel-authorized crash diagnostic service as the
 * single-summary query. Fails closed if any retained slot is unreadable or
 * corrupt; output records are cleared on failure. */
int rin_crash_history_get_local(RinCrashHistoryQueryV1* query);

/* Read pending volatile snapshots after an ambiguous/failed drain. Only the
 * signed crash diagnostic system service can call this API. It does not ACK,
 * mutate, or clear the queue. Output is cleared on failure. */
int rin_crash_pending_minidumps_get_local(RinCrashPendingQueryV1* query);

/* Persist the highest sequence durably handled by the authorized diagnostic
 * service. The kernel revalidates the caller and rejects future sequences. */
int rin_crash_history_ack_local(uint64_t sequence);

#ifdef __cplusplus
}
#endif

#if UINTPTR_MAX == UINT64_MAX && defined(__cplusplus)
static_assert(sizeof(RinCrashSummaryV1) == 2216u,
              "RinCrashSummaryV1 ABI drift");
static_assert(sizeof(RinCrashHistoryQueryV1) == 4456u,
              "RinCrashHistoryQueryV1 ABI drift");
static_assert(sizeof(RinCrashHistoryAckV1) == 16u,
              "RinCrashHistoryAckV1 ABI drift");
static_assert(sizeof(RinCrashPendingMinidumpV1) == 2224u,
              "RinCrashPendingMinidumpV1 ABI drift");
static_assert(sizeof(RinCrashPendingQueryV1) == 2248u,
              "RinCrashPendingQueryV1 ABI drift");
#elif UINTPTR_MAX == UINT64_MAX && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinCrashSummaryV1) == 2216u,
               "RinCrashSummaryV1 ABI drift");
_Static_assert(sizeof(RinCrashHistoryQueryV1) == 4456u,
               "RinCrashHistoryQueryV1 ABI drift");
_Static_assert(sizeof(RinCrashHistoryAckV1) == 16u,
               "RinCrashHistoryAckV1 ABI drift");
_Static_assert(sizeof(RinCrashPendingMinidumpV1) == 2224u,
               "RinCrashPendingMinidumpV1 ABI drift");
_Static_assert(sizeof(RinCrashPendingQueryV1) == 2248u,
               "RinCrashPendingQueryV1 ABI drift");
#endif

#endif /* RIN_API_RIN_CRASH_SUMMARY_H */
