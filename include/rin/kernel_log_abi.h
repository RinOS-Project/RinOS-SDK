/* SPDX-License-Identifier: MIT */
#ifndef RIN_KERNEL_LOG_ABI_H
#define RIN_KERNEL_LOG_ABI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_KERNEL_LOG_ABI_VERSION 1u
#define RIN_KERNEL_LOG_MESSAGE_MAX 192u
#define RIN_KERNEL_LOG_RECORDS_MAX 8u
#define RIN_KERNEL_LOG_CONTROL_V1_SIZE 1808u

enum RinKernelLogOperationV1 {
    RIN_KERNEL_LOG_OP_GET_INFO = 1u,
    RIN_KERNEL_LOG_OP_READ = 2u,
    RIN_KERNEL_LOG_OP_CLEAR = 3u,
    RIN_KERNEL_LOG_OP_SET_CONSOLE_LEVEL = 4u,
    RIN_KERNEL_LOG_OP_GET_CONSOLE_LEVEL = 5u
};

enum RinKernelLogSeverityV1 {
    RIN_KERNEL_LOG_EMERGENCY = 0u,
    RIN_KERNEL_LOG_ALERT = 1u,
    RIN_KERNEL_LOG_CRITICAL = 2u,
    RIN_KERNEL_LOG_ERROR = 3u,
    RIN_KERNEL_LOG_WARNING = 4u,
    RIN_KERNEL_LOG_NOTICE = 5u,
    RIN_KERNEL_LOG_INFO = 6u,
    RIN_KERNEL_LOG_DEBUG = 7u
};

enum RinKernelLogRecordFlagsV1 {
    RIN_KERNEL_LOG_RECORD_TRUNCATED = 1u << 0,
    RIN_KERNEL_LOG_RECORD_CONTINUATION = 1u << 1,
    RIN_KERNEL_LOG_RECORD_TIMESTAMP_VALID = 1u << 2
};

typedef struct RinKernelLogRecordV1 {
    uint64_t sequence;
    uint64_t monotonic_time_ns;
    uint32_t cpu_id;
    uint16_t message_size;
    uint8_t severity;
    uint8_t flags;
    char message[RIN_KERNEL_LOG_MESSAGE_MAX];
} RinKernelLogRecordV1;

/* One bounded inline copy validates the whole request without a nested user
 * pointer. READ uses after_sequence as an exclusive cursor and returns the
 * number of records copied; next_sequence is the cursor for the next call.
 * GET_INFO reports the visible range and cumulative overwrite count. CLEAR
 * hides all records written before the call. SET_CONSOLE_LEVEL returns the
 * old level; GET_CONSOLE_LEVEL returns the current level as the syscall value.
 * Console levels use an inclusive maximum severity (0 is emergency only,
 * 7 includes debug). */
typedef struct RinKernelLogControlV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    uint32_t record_capacity;
    uint64_t after_sequence;
    uint64_t next_sequence;
    uint64_t lost_records;
    uint64_t oldest_sequence;
    uint64_t newest_sequence;
    uint32_t record_count;
    uint32_t console_level;
    uint32_t reserved[4];
    RinKernelLogRecordV1 records[RIN_KERNEL_LOG_RECORDS_MAX];
} RinKernelLogControlV1;

/* The fixed size is part of the syscall copy contract on all three ABIs. */

#ifdef __cplusplus
}
#endif

#endif /* RIN_KERNEL_LOG_ABI_H */
