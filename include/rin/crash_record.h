/* SPDX-License-Identifier: MIT */

#ifndef RIN_API_RIN_CRASH_RECORD_H
#define RIN_API_RIN_CRASH_RECORD_H

#include <stdint.h>
#include <stddef.h>

#define RIN_CRASH_RECORD_MAGIC UINT32_C(0x31524352) /* "RCR1" */
#define RIN_CRASH_RECORD_VERSION UINT16_C(1)
#define RIN_CRASH_RECORD_MAX_SIZE UINT32_C(65536)
#define RIN_CRASH_RECORD_MAX_SECTIONS UINT32_C(16)
#define RIN_CRASH_RECORD_MAX_MESSAGE UINT32_C(256)
#define RIN_CRASH_RECORD_MAX_FRAMES UINT32_C(64)
#define RIN_CRASH_SCHEDULER_INFO_VERSION UINT16_C(2)
#define RIN_CRASH_SCHEDULER_MAX_CPUS UINT32_C(16)

#define RIN_CRASH_RECORD_FLAG_LOCAL_ONLY UINT32_C(0x00000001)
#define RIN_CRASH_RECORD_FLAG_USER_MODE  UINT32_C(0x00000002)
#define RIN_CRASH_RECORD_FLAG_MASK       UINT32_C(0x00000003)

typedef enum RinCrashRecordState {
    RIN_CRASH_RECORD_DRAFT = 0,
    RIN_CRASH_RECORD_PREPARED = 1,
    RIN_CRASH_RECORD_COMMITTED = 2
} RinCrashRecordState;

typedef enum RinCrashRecordKind {
    RIN_CRASH_RECORD_KERNEL_PANIC = 1,
    RIN_CRASH_RECORD_USER_MINIDUMP = 2
} RinCrashRecordKind;

typedef enum RinCrashRecordArchitecture {
    RIN_CRASH_ARCH_X86_64 = 1,
    RIN_CRASH_ARCH_X86_32 = 2
} RinCrashRecordArchitecture;

typedef enum RinCrashSectionType {
    RIN_CRASH_SECTION_MESSAGE = 1,
    RIN_CRASH_SECTION_REGISTERS_X64 = 2,
    RIN_CRASH_SECTION_STACKTRACE_X64 = 3,
    RIN_CRASH_SECTION_PROCESS = 4,
    RIN_CRASH_SECTION_REGISTERS_X86 = 5,
    RIN_CRASH_SECTION_SCHEDULER = 6
} RinCrashSectionType;

/* Stable order for RinCrashRegistersX64.gpr. */
typedef enum RinCrashX64RegisterIndex {
    RIN_CRASH_X64_RAX = 0,
    RIN_CRASH_X64_RBX = 1,
    RIN_CRASH_X64_RCX = 2,
    RIN_CRASH_X64_RDX = 3,
    RIN_CRASH_X64_RSI = 4,
    RIN_CRASH_X64_RDI = 5,
    RIN_CRASH_X64_RBP = 6,
    RIN_CRASH_X64_RSP = 7,
    RIN_CRASH_X64_R8 = 8,
    RIN_CRASH_X64_R9 = 9,
    RIN_CRASH_X64_R10 = 10,
    RIN_CRASH_X64_R11 = 11,
    RIN_CRASH_X64_R12 = 12,
    RIN_CRASH_X64_R13 = 13,
    RIN_CRASH_X64_R14 = 14,
    RIN_CRASH_X64_R15 = 15
} RinCrashX64RegisterIndex;

typedef enum RinCrashRecordResult {
    RIN_CRASH_RECORD_OK = 0,
    RIN_CRASH_RECORD_INVALID = -1,
    RIN_CRASH_RECORD_CORRUPT = -2,
    RIN_CRASH_RECORD_FULL = -3,
    RIN_CRASH_RECORD_STATE_ERROR = -4,
    RIN_CRASH_RECORD_IO = -5,
    RIN_CRASH_RECORD_NOT_FOUND = -6,
    RIN_CRASH_RECORD_EXHAUSTED = -7
} RinCrashRecordResult;

typedef struct RinCrashRecordHeader {
    uint32_t magic;
    uint16_t version;
    uint16_t header_size;
    uint32_t total_size;
    uint32_t state;
    uint64_t sequence;
    uint64_t timestamp_ns;
    uint64_t boot_id_high;
    uint64_t boot_id_low;
    uint32_t kind;
    uint32_t architecture;
    uint32_t process_id;
    uint32_t thread_id;
    uint32_t exception_code;
    uint32_t flags;
    uint32_t section_count;
    uint32_t payload_crc32;
    uint32_t header_crc32;
    uint32_t reserved;
} RinCrashRecordHeader;

typedef struct RinCrashSectionHeader {
    uint32_t type;
    uint32_t flags;
    uint32_t size;
    uint32_t reserved;
} RinCrashSectionHeader;

typedef struct RinCrashRegistersX64 {
    uint64_t gpr[16];
    uint64_t rip;
    uint64_t rflags;
    uint64_t cr2;
} RinCrashRegistersX64;

/* IA-32 register snapshot. Values are kept at their native width so a
 * crashd consumer cannot mistake a truncated native64 register for a
 * legacy process register. The stable GPR order mirrors the CPURegs frame. */
typedef enum RinCrashX86RegisterIndex {
    RIN_CRASH_X86_EAX = 0,
    RIN_CRASH_X86_EBX = 1,
    RIN_CRASH_X86_ECX = 2,
    RIN_CRASH_X86_EDX = 3,
    RIN_CRASH_X86_ESI = 4,
    RIN_CRASH_X86_EDI = 5,
    RIN_CRASH_X86_EBP = 6,
    RIN_CRASH_X86_ESP = 7
} RinCrashX86RegisterIndex;

typedef struct RinCrashRegistersX86 {
    uint32_t gpr[8];
    uint32_t eip;
    uint32_t eflags;
    uint32_t cr2;
} RinCrashRegistersX86;

typedef struct RinCrashOwnerSessionV1 {
    uint32_t session_id;
    uint32_t owner_uid;
    uint64_t instance_cookie;
} RinCrashOwnerSessionV1;

static inline int rin_crash_owner_session_valid(
    const RinCrashOwnerSessionV1* session, uint32_t owner_uid)
{
    return session != NULL &&
           ((session->session_id == 0u && session->owner_uid == 0u &&
             session->instance_cookie == 0u) ||
            (session->session_id != 0u && session->owner_uid != 0u &&
             session->owner_uid == owner_uid &&
             session->instance_cookie != 0u &&
             session->instance_cookie != UINT64_MAX));
}

typedef struct RinCrashProcessInfo {
    uint64_t image_base;
    uint64_t image_size;
    uint8_t image_sha256[32];
    char image_name[64];
    /* Authenticated launch package identity.  A zero pair is retained for
     * legacy records; otherwise both generation and digest are required. */
    uint64_t package_generation;
    uint8_t package_digest[32];
    /* Authenticated live-process identity captured by the kernel.  Older
     * RCR1 process sections end at package_digest and decode with these
     * fields cleared. */
    uint64_t process_instance_cookie;
    uint32_t owner_uid;
    uint32_t reserved;
    /* Exact authenticated login session captured with this process, when
     * the process was session-bound. A zero record is allowed for legacy
     * launches and kernel-generated summaries. */
    RinCrashOwnerSessionV1 owner_session;
} RinCrashProcessInfo;

#define RIN_CRASH_PROCESS_INFO_LEGACY_SIZE \
    ((uint32_t)offsetof(RinCrashProcessInfo, process_instance_cookie))
#define RIN_CRASH_PROCESS_INFO_PRE_SESSION_SIZE \
    ((uint32_t)offsetof(RinCrashProcessInfo, owner_session))

/* Pointer-free scheduler evidence captured at the crash boundary.  The
 * bounded CPU array mirrors the platform's supported scheduler CPU bound;
 * cpu_count says how many entries are valid. */
typedef struct RinCrashSchedulerCpuInfo {
    uint32_t cpu_index;
    uint32_t nr_running;
    uint64_t context_switches;
    uint64_t enqueue;
    uint64_t dequeue;
    uint64_t migrations;
    uint64_t steals;
    uint64_t idle_transitions;
} RinCrashSchedulerCpuInfo;

typedef struct RinCrashSchedulerInfo {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved;
    uint32_t cpu_count;
    uint32_t reserved2;
    uint64_t process_live;
    uint64_t process_high_watermark;
    uint64_t process_create_total;
    uint64_t process_create_fail_oom;
    uint64_t process_create_fail_policy;
    uint64_t thread_runnable_total;
    uint64_t reaper_pending;
    uint64_t reaper_deferred;
    /* Pointer-free current execution identity.  A valid sample contains the
     * complete Registry slot/generation and process-local thread generation;
     * zero valid means the CPU was idle or the owner was unavailable. */
    uint32_t current_valid;
    uint32_t current_cpu;
    uint32_t current_process_id;
    int32_t current_thread_id;
    uint32_t current_process_slot;
    uint32_t current_reserved;
    uint64_t current_process_generation;
    uint64_t current_thread_generation;
    RinCrashSchedulerCpuInfo cpu[RIN_CRASH_SCHEDULER_MAX_CPUS];
} RinCrashSchedulerInfo;

RinCrashRecordResult rin_crash_record_initialize(
    void* record, uint32_t capacity, uint32_t kind, uint32_t flags,
    uint64_t timestamp_ns, uint64_t boot_id_high, uint64_t boot_id_low,
    uint32_t process_id, uint32_t thread_id, uint32_t exception_code);
RinCrashRecordResult rin_crash_record_initialize_architecture(
    void* record, uint32_t capacity, uint32_t kind, uint32_t flags,
    uint64_t timestamp_ns, uint64_t boot_id_high, uint64_t boot_id_low,
    uint32_t process_id, uint32_t thread_id, uint32_t exception_code,
    uint32_t architecture);
RinCrashRecordResult rin_crash_record_add_section(
    void* record, uint32_t capacity, uint32_t type,
    const void* data, uint32_t size);
RinCrashRecordResult rin_crash_record_finalize(
    void* record, uint32_t capacity);
RinCrashRecordResult rin_crash_record_validate(
    const void* record, uint32_t available_size, int require_committed);

#if defined(__cplusplus)
static_assert(sizeof(RinCrashOwnerSessionV1) == 16u,
              "RinCrashOwnerSessionV1 ABI drift");
static_assert(sizeof(RinCrashProcessInfo) == 184u,
              "RinCrashProcessInfo ABI drift");
static_assert(sizeof(RinCrashRecordHeader) == 88u,
              "RinCrashRecordHeader ABI drift");
static_assert(sizeof(RinCrashSectionHeader) == 16u,
              "RinCrashSectionHeader ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinCrashOwnerSessionV1) == 16u,
               "RinCrashOwnerSessionV1 ABI drift");
_Static_assert(sizeof(RinCrashProcessInfo) == 184u,
               "RinCrashProcessInfo ABI drift");
_Static_assert(sizeof(RinCrashRecordHeader) == 88u,
               "RinCrashRecordHeader ABI drift");
_Static_assert(sizeof(RinCrashSectionHeader) == 16u,
               "RinCrashSectionHeader ABI drift");
#endif

#endif /* RIN_API_RIN_CRASH_RECORD_H */
