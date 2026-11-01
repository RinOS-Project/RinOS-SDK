/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_PERF_EVENT_H
#define RIN_SDK_PERF_EVENT_H

#include <stdint.h>
#include <rin/contract_abi.h>

typedef uint32_t RinPerfEventControlOperation;
typedef uint32_t RinPerfEventId;

/* Request for RIN_SYS_PERF_EVENT_CONTROL_V1.  Handles belong to the calling
 * task generation, are not descriptors, and are released by CLOSE or task
 * exit.  OPEN currently accepts TASK_CLOCK and flags == 0. */
typedef struct RinPerfEventControlV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t operation;
    uint32_t event_id;
    uint32_t flags;
    uint32_t reserved;
    uint64_t handle;
    uint64_t value;
} RinPerfEventControlV1;

#if defined(__cplusplus)
static_assert(sizeof(RinPerfEventControlV1) == 40u,
              "RinPerfEventControlV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinPerfEventControlV1) == 40u,
               "RinPerfEventControlV1 ABI drift");
#endif

#endif
