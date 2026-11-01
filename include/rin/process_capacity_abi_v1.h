/* SPDX-License-Identifier: MIT */
#ifndef RIN_PROCESS_CAPACITY_ABI_V1_H
#define RIN_PROCESS_CAPACITY_ABI_V1_H

#include "abi.h"

#define RIN_PROCESS_CAPACITY_ABI_VERSION_1 UINT32_C(1)
#define RIN_PROCESS_MAX_OBJECTS_PER_PROCESS UINT32_C(65535)
#define RIN_PROCESS_MAX_READINESS_EVENTS_PER_PROCESS UINT32_C(32)
#define RIN_PROCESS_MAX_SOCKET_READINESS_PRODUCERS_PER_PROCESS UINT32_C(32)
#define RIN_PROCESS_MAX_GPU_QUEUES_PER_PROCESS UINT32_C(16)
#define RIN_PROCESS_MAX_SERVICE_PUBLISHERS_PER_PROCESS UINT32_C(8)

/* These are per-kind ceilings. All live objects also share the process
 * object-table limit, so the maxima cannot all be consumed independently. */
typedef struct RinProcessCapacitiesV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t max_objects_per_process;
    uint32_t max_readiness_events_per_process;
    uint32_t max_socket_readiness_producers_per_process;
    uint32_t max_gpu_queues_per_process;
    uint32_t max_service_publishers_per_process;
    uint32_t reserved;
} RinProcessCapacitiesV1;

RIN_SDK_API int32_t rin_process_capacities_get_v1(
    RinProcessCapacitiesV1* capacities);

#if defined(__cplusplus)
static_assert(sizeof(RinProcessCapacitiesV1) == 32u,
              "RinProcessCapacitiesV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinProcessCapacitiesV1) == 32u,
               "RinProcessCapacitiesV1 ABI drift");
#endif

#endif /* RIN_PROCESS_CAPACITY_ABI_V1_H */
