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
#define RIN_GPU_CAPABILITY_IPC_MAX_REQUEST_BYTES 96u
#define RIN_GPU_CAPABILITY_IPC_MAX_RESPONSE_BYTES 112u

#define RIN_GPU_CROSS_PROCESS_RIGHT_READ    UINT32_C(0x00000001)
#define RIN_GPU_CROSS_PROCESS_RIGHT_WRITE   UINT32_C(0x00000002)
#define RIN_GPU_CROSS_PROCESS_RIGHT_PRESENT UINT32_C(0x00000004)
#define RIN_GPU_CROSS_PROCESS_RIGHT_MASK \
    (RIN_GPU_CROSS_PROCESS_RIGHT_READ | RIN_GPU_CROSS_PROCESS_RIGHT_WRITE | \
     RIN_GPU_CROSS_PROCESS_RIGHT_PRESENT)

/* The capability service can distinguish an unsupported mapping from a
 * malformed request or transport failure. */
#define RIN_GPU_CROSS_PROCESS_UNSUPPORTED INT32_C(-8)

enum RinGpuCapabilityIpcOpcode {
    RIN_GPU_CAPABILITY_IPC_ISSUE = 1,
    RIN_GPU_CAPABILITY_IPC_VALIDATE = 2,
    RIN_GPU_CAPABILITY_IPC_RELEASE = 3,
    RIN_GPU_CAPABILITY_IPC_REVOKE_PROCESS = 4,
    RIN_GPU_CAPABILITY_IPC_ISSUE_V2 = 5,
    RIN_GPU_CAPABILITY_IPC_ACQUIRE_V2 = 6,
    RIN_GPU_CAPABILITY_IPC_RELEASE_LEASE_V2 = 7,
    RIN_GPU_CAPABILITY_IPC_REVOKE_V2 = 8,
    RIN_GPU_CAPABILITY_IPC_MAP_READABLE_LEASE_V1 = 9
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

/* V2 grants bind an allocation to the recipient process instance. The
 * issuing service obtains the owner identity from kernel-authenticated IPC;
 * callers must not supply an owner identity in this descriptor. */
typedef struct RinGpuCrossProcessCapabilityGrantDescV2 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t rights;
    uint32_t flags;
    uint64_t device_generation;
    uint64_t resource_id;
    uint64_t recipient_process_id;
    uint64_t recipient_process_instance_cookie;
    uint64_t reserved[2];
} RinGpuCrossProcessCapabilityGrantDescV2;

typedef struct RinGpuCrossProcessCapabilityTokenV2 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t token;
    uint64_t device_generation;
    uint32_t rights;
    uint32_t reserved[3];
} RinGpuCrossProcessCapabilityTokenV2;

typedef struct RinGpuCrossProcessCapabilityAcquireRequestV2 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t token;
    uint64_t device_generation;
    uint32_t required_rights;
    uint32_t reserved[3];
} RinGpuCrossProcessCapabilityAcquireRequestV2;

/* A successful acquire creates one independently releasable consumer lease.
 * The resource identity is returned only after matching the kernel peer to
 * the recipient bound when the grant was issued. */
typedef struct RinGpuCrossProcessCapabilityLeaseV2 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t token;
    uint64_t lease_id;
    uint64_t resource_id;
    uint64_t device_generation;
    uint32_t rights;
    uint32_t reserved[3];
} RinGpuCrossProcessCapabilityLeaseV2;

typedef struct RinGpuCrossProcessCapabilityReleaseLeaseV2 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t token;
    uint64_t lease_id;
    uint64_t device_generation;
    uint32_t reserved[2];
} RinGpuCrossProcessCapabilityReleaseLeaseV2;

/* Explicit CPU-readable import for a consumer that composes an image in its
 * own address space. This is not a GPUVA/IOVA export or a scanout mapping.
 * The request names an already acquired V2 lease; the kernel derives both
 * process identities from the authenticated channel message. The mapping is
 * released together with that same V2 lease. Allocations that are not
 * CPU-visible return UNSUPPORTED and remain unmapped. */
#define RIN_GPU_CROSS_PROCESS_MAPPED_READABLE_V1_VERSION UINT32_C(1)
#define RIN_GPU_CROSS_PROCESS_MAPPED_ACCESS_CPU_READ UINT32_C(1)
typedef struct RinGpuCrossProcessCapabilityMapReadableLeaseRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t token;
    uint64_t lease_id;
    uint64_t device_generation;
    uint64_t reserved[2];
} RinGpuCrossProcessCapabilityMapReadableLeaseRequestV1;

typedef struct RinGpuCrossProcessCapabilityMappedReadableLeaseV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t token;
    uint64_t lease_id;
    uint64_t resource_id;
    uint64_t device_generation;
    uint64_t address_in_recipient;
    uint64_t allocation_size;
    uint32_t rights;
    uint32_t access;
    uint64_t reserved[2];
} RinGpuCrossProcessCapabilityMappedReadableLeaseV1;

typedef struct RinGpuCrossProcessCapabilityRevokeResponseV2 {
    uint32_t outstanding_lease_count;
    uint32_t reserved[3];
} RinGpuCrossProcessCapabilityRevokeResponseV2;

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
static_assert(sizeof(RinGpuCrossProcessCapabilityGrantDescV2) == 64u,
              "RinGpuCrossProcessCapabilityGrantDescV2 ABI drift");
static_assert(sizeof(RinGpuCrossProcessCapabilityTokenV2) == 40u,
              "RinGpuCrossProcessCapabilityTokenV2 ABI drift");
static_assert(sizeof(RinGpuCrossProcessCapabilityAcquireRequestV2) == 40u,
              "RinGpuCrossProcessCapabilityAcquireRequestV2 ABI drift");
static_assert(sizeof(RinGpuCrossProcessCapabilityLeaseV2) == 56u,
              "RinGpuCrossProcessCapabilityLeaseV2 ABI drift");
static_assert(sizeof(RinGpuCrossProcessCapabilityReleaseLeaseV2) == 40u,
              "RinGpuCrossProcessCapabilityReleaseLeaseV2 ABI drift");
static_assert(sizeof(RinGpuCrossProcessCapabilityMapReadableLeaseRequestV1) == 48u,
              "RinGpuCrossProcessCapabilityMapReadableLeaseRequestV1 ABI drift");
static_assert(sizeof(RinGpuCrossProcessCapabilityMappedReadableLeaseV1) == 80u,
              "RinGpuCrossProcessCapabilityMappedReadableLeaseV1 ABI drift");
static_assert(sizeof(RinGpuCrossProcessCapabilityRevokeResponseV2) == 16u,
              "RinGpuCrossProcessCapabilityRevokeResponseV2 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinGpuCapabilityIpcHeaderV1) == 32u,
               "RinGpuCapabilityIpcHeaderV1 ABI drift");
_Static_assert(sizeof(RinGpuCrossProcessCapabilityDescV1) == 48u,
               "RinGpuCrossProcessCapabilityDescV1 ABI drift");
_Static_assert(sizeof(RinGpuCrossProcessCapabilityTokenV1) == 48u,
               "RinGpuCrossProcessCapabilityTokenV1 ABI drift");
_Static_assert(sizeof(RinGpuCrossProcessCapabilityRequestV1) == 48u,
               "RinGpuCrossProcessCapabilityRequestV1 ABI drift");
_Static_assert(sizeof(RinGpuCrossProcessCapabilityGrantDescV2) == 64u,
               "RinGpuCrossProcessCapabilityGrantDescV2 ABI drift");
_Static_assert(sizeof(RinGpuCrossProcessCapabilityTokenV2) == 40u,
               "RinGpuCrossProcessCapabilityTokenV2 ABI drift");
_Static_assert(sizeof(RinGpuCrossProcessCapabilityAcquireRequestV2) == 40u,
               "RinGpuCrossProcessCapabilityAcquireRequestV2 ABI drift");
_Static_assert(sizeof(RinGpuCrossProcessCapabilityLeaseV2) == 56u,
               "RinGpuCrossProcessCapabilityLeaseV2 ABI drift");
_Static_assert(sizeof(RinGpuCrossProcessCapabilityReleaseLeaseV2) == 40u,
               "RinGpuCrossProcessCapabilityReleaseLeaseV2 ABI drift");
_Static_assert(sizeof(RinGpuCrossProcessCapabilityMapReadableLeaseRequestV1) == 48u,
               "RinGpuCrossProcessCapabilityMapReadableLeaseRequestV1 ABI drift");
_Static_assert(sizeof(RinGpuCrossProcessCapabilityMappedReadableLeaseV1) == 80u,
               "RinGpuCrossProcessCapabilityMappedReadableLeaseV1 ABI drift");
_Static_assert(sizeof(RinGpuCrossProcessCapabilityRevokeResponseV2) == 16u,
               "RinGpuCrossProcessCapabilityRevokeResponseV2 ABI drift");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_GPU_CAPABILITY_H */
