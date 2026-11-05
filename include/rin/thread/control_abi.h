/* SPDX-License-Identifier: MIT */
#ifndef RIN_THREAD_CONTROL_ABI_H
#define RIN_THREAD_CONTROL_ABI_H

#include <stdint.h>

#define RIN_THREAD_CONTROL_V1_VERSION 1u
#define RIN_THREAD_CONTROL_PRIORITY_IDLE (-15)
#define RIN_THREAD_CONTROL_PRIORITY_LOWEST (-2)
#define RIN_THREAD_CONTROL_PRIORITY_BELOW_NORMAL (-1)
#define RIN_THREAD_CONTROL_PRIORITY_NORMAL 0
#define RIN_THREAD_CONTROL_PRIORITY_ABOVE_NORMAL 1
#define RIN_THREAD_CONTROL_PRIORITY_HIGHEST 2
#define RIN_THREAD_CONTROL_PRIORITY_TIME_CRITICAL 15
#define RIN_THREAD_CONTROL_MAX_SUSPEND_COUNT UINT32_C(0x7fffffff)

typedef enum RinThreadControlOperationV1 {
    RIN_THREAD_CONTROL_RESOLVE = 1u,
    RIN_THREAD_CONTROL_QUERY = 2u,
    RIN_THREAD_CONTROL_TERMINATE = 3u,
    RIN_THREAD_CONTROL_SUSPEND = 4u,
    RIN_THREAD_CONTROL_RESUME = 5u,
    RIN_THREAD_CONTROL_SET_PRIORITY = 6u,
    /* Resolve the caller-visible thread and return both generations without
     * changing WinCompat state. Zero process identity means caller process. */
    RIN_THREAD_CONTROL_RESOLVE_GENERATION = 7u,
    RIN_THREAD_CONTROL_GET_AFFINITY = 8u,
    RIN_THREAD_CONTROL_SET_AFFINITY = 9u
} RinThreadControlOperationV1;

typedef enum RinThreadControlStateKindV1 {
    RIN_THREAD_CONTROL_STATE_ACTIVE = 1u,
    RIN_THREAD_CONTROL_STATE_TERMINATED = 2u
} RinThreadControlStateKindV1;

/* V1 affinity operations use reserved[0] for a caller pointer to a CPU mask
 * and reserved[1] for its byte size. All other operations require both fields
 * to be zero. A successful RESOLVE_GENERATION fills the process/thread
 * generations without marking the target as a WinCompat thread. */
typedef struct RinThreadControlV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved_header;
    uint32_t operation;
    uint32_t thread_id;
    uint32_t process_slot;
    uint32_t reserved0;
    uint64_t process_generation;
    uint64_t thread_generation;
    uint32_t exit_code;
    int32_t priority;
    uint32_t state;
    uint32_t suspend_count;
    uint32_t previous_suspend_count;
    uint32_t reserved1;
    uint64_t reserved[2];
} RinThreadControlV1;

#endif /* RIN_THREAD_CONTROL_ABI_H */
