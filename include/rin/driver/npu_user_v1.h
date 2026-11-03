/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_NPU_USER_V1_H
#define RIN_DRIVER_NPU_USER_V1_H

#include <stdint.h>

#include <rin/driver/npu_v1.h>
#include <rin/syscall.h>
#include <rin/syscall_abi.h>

#define RIN_DRIVER_V3_NPU_USER_ABI_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_NPU_USER_MAX_BUFFER UINT32_C(65536)

typedef enum RinDriverV3NpuUserOperationV1 {
    RIN_DRIVER_V3_NPU_USER_ENUMERATE = 1,
    RIN_DRIVER_V3_NPU_USER_QUERY_DEVICE = 2,
    RIN_DRIVER_V3_NPU_USER_CONTEXT_CREATE = 3,
    RIN_DRIVER_V3_NPU_USER_CONTEXT_DESTROY = 4,
    RIN_DRIVER_V3_NPU_USER_BUFFER_CREATE = 5,
    RIN_DRIVER_V3_NPU_USER_BUFFER_DESTROY = 6,
    RIN_DRIVER_V3_NPU_USER_QUEUE_CREATE = 7,
    RIN_DRIVER_V3_NPU_USER_QUEUE_DESTROY = 8,
    RIN_DRIVER_V3_NPU_USER_SUBMIT = 9,
    RIN_DRIVER_V3_NPU_USER_POLL_EVENTS = 10,
    RIN_DRIVER_V3_NPU_USER_CANCEL = 11,
    RIN_DRIVER_V3_NPU_USER_RESET_CONTEXT = 12,
    RIN_DRIVER_V3_NPU_USER_REGISTER_MEMORY = 13,
    RIN_DRIVER_V3_NPU_USER_UNREGISTER_MEMORY = 14
} RinDriverV3NpuUserOperationV1;

/* The request and response are fixed-width across native x86, x86_64, and
 * x86_64 compat32. `argument0/1` are operation-specific scalars; user virtual
 * addresses are accepted only by REGISTER_MEMORY and are pinned by the
 * kernel before a token is returned. */
typedef struct RinDriverV3NpuUserCallV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t operation;
    uint32_t flags;
    uint64_t device_generation;
    uint64_t context_handle;
    uint64_t context_generation;
    uint64_t object_handle;
    uint64_t object_generation;
    uint64_t argument0;
    uint64_t argument1;
    uint32_t access;
    uint32_t reserved0;
    uint64_t reserved[2];
} RinDriverV3NpuUserCallV1;

typedef struct RinDriverV3NpuUserDeviceInfoV1 {
    uint32_t struct_size;
    uint32_t version;
    RinDriverV3NpuIdentityV1 identity;
    RinDriverV3NpuCapabilitiesV1 capabilities;
} RinDriverV3NpuUserDeviceInfoV1;

typedef struct RinDriverV3NpuUserCallResultV1 {
    uint32_t struct_size;
    uint32_t version;
    int32_t result;
    uint32_t flags;
    uint64_t handle;
    uint64_t generation;
    uint32_t count;
    uint32_t data_bytes;
    union {
        RinDriverV3NpuUserDeviceInfoV1 device;
        RinDriverV3NpuIdentityV1 identity;
        RinDriverV3NpuCapabilitiesV1 capabilities;
        RinDriverV3NpuBufferInfoV1 buffer;
        RinDriverV3NpuFenceV1 fence;
        uint64_t token_pair[2];
    } value;
    uint64_t reserved[2];
} RinDriverV3NpuUserCallResultV1;

/* Variable-length data is passed in one bounded buffer. SUBMIT uses a
 * RinDriverV3NpuSubmitV1 followed by resource_count buffer references;
 * POLL_EVENTS writes an array of fixed-size event records; ENUMERATE writes
 * an array of device records. Other operations require their exact payload
 * record size or an empty buffer. The response and data buffer are zeroed on
 * kernel-side failure wherever copyout remains possible. */
static inline long rin_driver_v3_npu_user_call_v1(
    const RinDriverV3NpuUserCallV1* request,
    RinDriverV3NpuUserCallResultV1* result,
    void* data, uint32_t data_bytes)
{
    if (request == 0 || result == 0 ||
        data_bytes > RIN_DRIVER_V3_NPU_USER_MAX_BUFFER ||
        ((data == 0) != (data_bytes == 0u)))
        return -1;
    return syscall((long)RIN_SYS_NPU_CALL,
                   (uintptr_t)request, (uintptr_t)result,
                   (uintptr_t)data, (uintptr_t)data_bytes);
}

#if defined(__cplusplus)
static_assert(sizeof(RinDriverV3NpuUserCallV1) == 96u,
              "NPU user-call request ABI drift");
static_assert(sizeof(RinDriverV3NpuUserDeviceInfoV1) == 184u,
              "NPU user device-info ABI drift");
static_assert(sizeof(RinDriverV3NpuUserCallResultV1) == 240u,
              "NPU user-call response ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinDriverV3NpuUserCallV1) == 96u,
               "NPU user-call request ABI drift");
_Static_assert(sizeof(RinDriverV3NpuUserDeviceInfoV1) == 184u,
               "NPU user device-info ABI drift");
_Static_assert(sizeof(RinDriverV3NpuUserCallResultV1) == 240u,
               "NPU user-call response ABI drift");
#endif

#endif /* RIN_DRIVER_NPU_USER_V1_H */
