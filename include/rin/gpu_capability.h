/* SPDX-License-Identifier: MIT */
/* RinGPU cross-process capability wire contract carried by RinOS IPC. */
#ifndef RIN_SDK_GPU_CAPABILITY_H
#define RIN_SDK_GPU_CAPABILITY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_GPU_CAPABILITY_IPC_VERSION UINT16_C(1)
#define RIN_GPU_CROSS_PROCESS_CAPABILITY_V2_VERSION UINT32_C(2)
#define RIN_GPU_CAPABILITY_SERVICE_NAME "ringpu-capability"
#define RIN_GPU_CROSS_PROCESS_CAPABILITY_MAX 64u
#define RIN_GPU_CAPABILITY_IPC_MAX_REQUEST_BYTES 96u
#define RIN_GPU_CAPABILITY_IPC_MAX_RESPONSE_BYTES 1024u

/* The authenticated capability service also transports the admitted
 * OS-Core Vulkan physical profile to a normal process. The profile is an
 * opaque, pointer-free V2 ABI record; Vulkan consumers validate its size and
 * version before interpreting it. Queue routes are explicit and are never
 * reconstructed from queue counts. */
#define RIN_GPU_VULKAN_CATALOG_QUERY_V1_VERSION UINT32_C(1)
#define RIN_GPU_VULKAN_CATALOG_SNAPSHOT_V1_VERSION UINT32_C(1)
#define RIN_GPU_VULKAN_CATALOG_PROFILE_V2_SIZE UINT32_C(576)
#define RIN_GPU_VULKAN_CATALOG_MAX_QUEUE_ROUTES UINT32_C(8)
#define RIN_GPU_CAPABILITY_PRESENT_RESOURCE_BIND_V1_VERSION UINT32_C(1)

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
    RIN_GPU_CAPABILITY_IPC_MAP_READABLE_LEASE_V1 = 9,
    RIN_GPU_CAPABILITY_IPC_QUERY_VULKAN_CATALOG_V1 = 10,
    RIN_GPU_CAPABILITY_IPC_BIND_PRESENT_RESOURCE_V1 = 11
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

/* Bind an exact allocation-relative range into the authenticated recipient's
 * GPU resource namespace under an acquired PRESENT lease. Only GPU_READ is
 * admitted by this operation. The resource cookie is process-local; no GPU
 * virtual, I/O virtual, or scanout address is carried by this wire record.
 * Releasing the same capability lease retires the binding. */
typedef struct RinGpuCapabilityPresentResourceBindV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t token;
    uint64_t lease_id;
    uint64_t device_generation;
    uint64_t resource_cookie;
    uint64_t allocation_offset;
    uint64_t size_bytes;
    uint32_t required_gpu_access;
    uint32_t reserved;
} RinGpuCapabilityPresentResourceBindV1;

typedef struct RinGpuVulkanCatalogQueryRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t device_id;
    uint64_t device_generation;
    uint64_t reserved[2];
} RinGpuVulkanCatalogQueryRequestV1;

typedef struct RinGpuVulkanQueueRouteIpcV1 {
    uint32_t queue_family_index;
    uint32_t queue_index;
    uint32_t physical_queue_id;
    uint32_t reserved;
} RinGpuVulkanQueueRouteIpcV1;

typedef struct RinGpuVulkanCatalogSnapshotV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t device_id;
    uint64_t device_generation;
    uint64_t device_epoch;
    uint32_t profile_size;
    uint32_t profile_version;
    uint32_t route_count;
    uint32_t route_struct_size;
    uint64_t reserved[2];
    uint8_t profile[RIN_GPU_VULKAN_CATALOG_PROFILE_V2_SIZE];
    RinGpuVulkanQueueRouteIpcV1
        routes[RIN_GPU_VULKAN_CATALOG_MAX_QUEUE_ROUTES];
} RinGpuVulkanCatalogSnapshotV1;

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
static_assert(sizeof(RinGpuCapabilityPresentResourceBindV1) == 64u,
              "RinGpuCapabilityPresentResourceBindV1 ABI drift");
static_assert(sizeof(RinGpuCrossProcessCapabilityRevokeResponseV2) == 16u,
              "RinGpuCrossProcessCapabilityRevokeResponseV2 ABI drift");
static_assert(sizeof(RinGpuVulkanCatalogQueryRequestV1) == 40u,
              "RinGpuVulkanCatalogQueryRequestV1 ABI drift");
static_assert(sizeof(RinGpuVulkanQueueRouteIpcV1) == 16u,
              "RinGpuVulkanQueueRouteIpcV1 ABI drift");
static_assert(sizeof(RinGpuVulkanCatalogSnapshotV1) == 768u,
              "RinGpuVulkanCatalogSnapshotV1 ABI drift");
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
_Static_assert(sizeof(RinGpuCapabilityPresentResourceBindV1) == 64u,
               "RinGpuCapabilityPresentResourceBindV1 ABI drift");
_Static_assert(sizeof(RinGpuCrossProcessCapabilityRevokeResponseV2) == 16u,
               "RinGpuCrossProcessCapabilityRevokeResponseV2 ABI drift");
_Static_assert(sizeof(RinGpuVulkanCatalogQueryRequestV1) == 40u,
               "RinGpuVulkanCatalogQueryRequestV1 ABI drift");
_Static_assert(sizeof(RinGpuVulkanQueueRouteIpcV1) == 16u,
               "RinGpuVulkanQueueRouteIpcV1 ABI drift");
_Static_assert(sizeof(RinGpuVulkanCatalogSnapshotV1) == 768u,
               "RinGpuVulkanCatalogSnapshotV1 ABI drift");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_GPU_CAPABILITY_H */
