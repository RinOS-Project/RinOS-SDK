/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Public fixed-record request envelope for the system log service. */
#ifndef RIN_SDK_SERVICE_LOG_TRANSPORT_V1_H
#define RIN_SDK_SERVICE_LOG_TRANSPORT_V1_H

#include <stddef.h>
#include <stdint.h>

#define RIN_SERVICE_LOG_SERVICE_VERSION 1u
#define RIN_SERVICE_LOG_SERVICE_ID_MAX 64u
#define RIN_SERVICE_LOG_SERVICE_PATH "/run/rin/servicelogd.sock"
#define RIN_SERVICE_LOG_SERVICE_PAGE_MAX 16u
#define RIN_SERVICE_LOG_SERVICE_MODE_LATEST 1u
#define RIN_SERVICE_LOG_SERVICE_MODE_OLDER 2u
#define RIN_SERVICE_LOG_SERVICE_MODE_NEWER 3u
#define RIN_SERVICE_LOG_SERVICE_PAGE_HAS_OLDER 0x00000001u
#define RIN_SERVICE_LOG_SERVICE_PAGE_HAS_NEWER 0x00000002u
#define RIN_SERVICE_LOG_SERVICE_PAGE_FLAG_MASK 0x00000003u
#define RIN_SERVICE_LOG_SERVICE_SCOPE_SYSTEM 1u
#define RIN_SERVICE_LOG_SERVICE_STATUS_OK 0
#define RIN_SERVICE_LOG_SERVICE_STATUS_NOT_FOUND 1
#define RIN_SERVICE_LOG_SERVICE_STATUS_INVALID 2
#define RIN_SERVICE_LOG_SERVICE_CURSOR_CAPACITY 256u

typedef struct RinServiceLogServiceRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t mode;
    uint32_t limit;
    uint32_t reserved0;
    uint64_t request_id;
    uint64_t cursor;
    char service_id[RIN_SERVICE_LOG_SERVICE_ID_MAX];
    uint64_t reserved[2];
} RinServiceLogServiceRequestV1;

#if defined(__cplusplus)
static_assert(sizeof(RinServiceLogServiceRequestV1) == 112u,
              "RinServiceLogServiceRequestV1 ABI drift");
static_assert(offsetof(RinServiceLogServiceRequestV1, request_id) == 16u,
              "RinServiceLogServiceRequestV1.request_id ABI drift");
static_assert(offsetof(RinServiceLogServiceRequestV1, service_id) == 32u,
              "RinServiceLogServiceRequestV1.service_id ABI drift");
#else
_Static_assert(sizeof(RinServiceLogServiceRequestV1) == 112u,
               "RinServiceLogServiceRequestV1 ABI drift");
_Static_assert(offsetof(RinServiceLogServiceRequestV1, request_id) == 16u,
               "RinServiceLogServiceRequestV1.request_id ABI drift");
_Static_assert(offsetof(RinServiceLogServiceRequestV1, service_id) == 32u,
               "RinServiceLogServiceRequestV1.service_id ABI drift");
#endif

#endif /* RIN_SDK_SERVICE_LOG_TRANSPORT_V1_H */
