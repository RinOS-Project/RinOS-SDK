/* SPDX-License-Identifier: MIT */
/* RinGPU cross-process capability wire contract carried by RinOS IPC. */
#ifndef RIN_SDK_GPU_CAPABILITY_H
#define RIN_SDK_GPU_CAPABILITY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_GPU_CAPABILITY_IPC_VERSION UINT16_C(1)
#define RIN_GPU_CAPABILITY_SERVICE_NAME "ringpu-capability"
#define RIN_GPU_CROSS_PROCESS_CAPABILITY_MAX 64u

#define RIN_GPU_CROSS_PROCESS_RIGHT_READ    UINT32_C(0x00000001)
#define RIN_GPU_CROSS_PROCESS_RIGHT_WRITE   UINT32_C(0x00000002)
#define RIN_GPU_CROSS_PROCESS_RIGHT_PRESENT UINT32_C(0x00000004)
#define RIN_GPU_CROSS_PROCESS_RIGHT_MASK \
    (RIN_GPU_CROSS_PROCESS_RIGHT_READ | RIN_GPU_CROSS_PROCESS_RIGHT_WRITE | \
     RIN_GPU_CROSS_PROCESS_RIGHT_PRESENT)

enum RinGpuCapabilityIpcOpcode {
    RIN_GPU_CAPABILITY_IPC_ISSUE = 1,
    RIN_GPU_CAPABILITY_IPC_VALIDATE = 2,
    RIN_GPU_CAPABILITY_IPC_RELEASE = 3,
    RIN_GPU_CAPABILITY_IPC_REVOKE_PROCESS = 4
};

typedef struct RinGpuCapabilityIpcHeaderV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t opcode;
    uint64_t request_id;
    uint32_t payload_size;
    int32_t status;
    uint32_t flags;
    uint32_t reserved;
} RinGpuCapabilityIpcHeaderV1;

typedef struct RinGpuCrossProcessCapabilityDescV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t owner_process_id;
    uint64_t device_generation;
    uint64_t resource_id;
    uint32_t rights;
    uint32_t reserved[3];
} RinGpuCrossProcessCapabilityDescV1;

typedef struct RinGpuCrossProcessCapabilityTokenV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t token;
    uint64_t owner_process_id;
    uint64_t device_generation;
    uint32_t rights;
    uint32_t reserved[3];
} RinGpuCrossProcessCapabilityTokenV1;

typedef struct RinGpuCrossProcessCapabilityRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t token;
    uint64_t process_id;
    uint64_t device_generation;
    uint32_t required_rights;
    uint32_t reserved[3];
} RinGpuCrossProcessCapabilityRequestV1;

typedef struct RinGpuCapabilityValidateResponseV1 {
    uint64_t resource_id;
    uint32_t rights;
    uint32_t reserved[3];
} RinGpuCapabilityValidateResponseV1;

typedef struct RinGpuCapabilityRevokeProcessRequestV1 {
    uint64_t process_id;
} RinGpuCapabilityRevokeProcessRequestV1;

typedef struct RinGpuCapabilityRevokeProcessResponseV1 {
    uint32_t revoked_count;
    uint32_t reserved[3];
} RinGpuCapabilityRevokeProcessResponseV1;

#if defined(__cplusplus)
static_assert(sizeof(RinGpuCapabilityIpcHeaderV1) == 32u,
              "RinGpuCapabilityIpcHeaderV1 ABI drift");
static_assert(sizeof(RinGpuCrossProcessCapabilityDescV1) == 48u,
              "RinGpuCrossProcessCapabilityDescV1 ABI drift");
static_assert(sizeof(RinGpuCrossProcessCapabilityTokenV1) == 48u,
              "RinGpuCrossProcessCapabilityTokenV1 ABI drift");
static_assert(sizeof(RinGpuCrossProcessCapabilityRequestV1) == 48u,
              "RinGpuCrossProcessCapabilityRequestV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinGpuCapabilityIpcHeaderV1) == 32u,
               "RinGpuCapabilityIpcHeaderV1 ABI drift");
_Static_assert(sizeof(RinGpuCrossProcessCapabilityDescV1) == 48u,
               "RinGpuCrossProcessCapabilityDescV1 ABI drift");
_Static_assert(sizeof(RinGpuCrossProcessCapabilityTokenV1) == 48u,
               "RinGpuCrossProcessCapabilityTokenV1 ABI drift");
_Static_assert(sizeof(RinGpuCrossProcessCapabilityRequestV1) == 48u,
               "RinGpuCrossProcessCapabilityRequestV1 ABI drift");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_GPU_CAPABILITY_H */
