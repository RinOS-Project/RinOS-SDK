/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_RTC_BACKEND_V1_H
#define RIN_DRIVER_RTC_BACKEND_V1_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_RTC_CORRECTION_VERSION UINT32_C(0x00010001)

/* Evidence bits carried by RinRtcWriteRequestV1.  The RTC backend validates
 * these values but does not create or upgrade trusted-time evidence. */
#define RIN_RTC_TRUSTED_TIME_EVIDENCE_AUTHENTICATED UINT32_C(0x00000001)
#define RIN_RTC_TRUSTED_TIME_EVIDENCE_REPLAY_PROTECTED UINT32_C(0x00000002)
#define RIN_RTC_TRUSTED_TIME_EVIDENCE_NTS UINT32_C(0x00000004)
#define RIN_RTC_TRUSTED_TIME_EVIDENCE_BOOT_ANCHOR UINT32_C(0x00000008)
#define RIN_RTC_TRUSTED_TIME_EVIDENCE_REQUIRED                         \
    (RIN_RTC_TRUSTED_TIME_EVIDENCE_AUTHENTICATED |                    \
     RIN_RTC_TRUSTED_TIME_EVIDENCE_REPLAY_PROTECTED)
#define RIN_RTC_TRUSTED_TIME_EVIDENCE_ALL                             \
    (RIN_RTC_TRUSTED_TIME_EVIDENCE_REQUIRED |                         \
     RIN_RTC_TRUSTED_TIME_EVIDENCE_NTS |                              \
     RIN_RTC_TRUSTED_TIME_EVIDENCE_BOOT_ANCHOR)

#define RIN_RTC_SNAPSHOT_VALID UINT32_C(0x00000001)
#define RIN_RTC_SNAPSHOT_ALL RIN_RTC_SNAPSHOT_VALID

typedef enum RinRtcCorrectionStatus {
    RIN_RTC_CORRECTION_OK = 0,
    RIN_RTC_CORRECTION_INVALID = -1,
    RIN_RTC_CORRECTION_TRUSTED_TIME_UNAVAILABLE = -2,
    RIN_RTC_CORRECTION_TRUSTED_TIME_EXPIRED = -3,
    RIN_RTC_CORRECTION_RTC_UNAVAILABLE = -4,
    RIN_RTC_CORRECTION_RTC_CONFLICT = -5,
    RIN_RTC_CORRECTION_WRITE_FAILED = -6,
    RIN_RTC_CORRECTION_COMMIT_UNKNOWN = -7,
    RIN_RTC_CORRECTION_READBACK_MISMATCH = -8,
    RIN_RTC_CORRECTION_CLOCK_ROLLBACK = -9,
    RIN_RTC_CORRECTION_BUSY = -10,
    RIN_RTC_CORRECTION_OVERFLOW = -11
} RinRtcCorrectionStatus;

typedef struct RinRtcSnapshotV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t unix_time;
    uint32_t flags;
    uint32_t reserved0;
    uint8_t opaque[16];
    uint64_t reserved[2];
} RinRtcSnapshotV1;

typedef struct RinRtcWriteRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    RinRtcSnapshotV1 expected;
    uint64_t target_unix_time;
    uint64_t trusted_time_sequence;
    uint32_t trusted_time_evidence_flags;
    uint32_t reserved0;
    uint64_t reserved[2];
} RinRtcWriteRequestV1;

typedef int (*RinRtcReadV1)(void* context, RinRtcSnapshotV1* snapshot);
typedef int (*RinRtcCompareWriteV1)(
    void* context, const RinRtcWriteRequestV1* request);

/* The Kernel calls this backend only after acquiring trusted time and
 * captures the expected snapshot before compare_write. Drivers provide the
 * hardware operation; the OS retains time-trust policy and correction state.
 */
typedef struct RinRtcBackendV1 {
    uint32_t struct_size;
    uint32_t version;
    void* context;
    RinRtcReadV1 read;
    RinRtcCompareWriteV1 compare_write;
    uint64_t reserved[2];
} RinRtcBackendV1;

#if defined(__cplusplus)
static_assert(sizeof(RinRtcCorrectionStatus) == 4u,
              "RTC correction result ABI drift");
static_assert(sizeof(RinRtcSnapshotV1) == 56u,
              "RTC snapshot ABI drift");
static_assert(sizeof(RinRtcWriteRequestV1) == 104u,
              "RTC write request ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinRtcCorrectionStatus) == 4u,
               "RTC correction result ABI drift");
_Static_assert(sizeof(RinRtcSnapshotV1) == 56u,
               "RTC snapshot ABI drift");
_Static_assert(sizeof(RinRtcWriteRequestV1) == 104u,
               "RTC write request ABI drift");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RIN_DRIVER_RTC_BACKEND_V1_H */
