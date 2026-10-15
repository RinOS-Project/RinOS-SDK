/* SPDX-License-Identifier: MIT */
/* Deny-by-default admission for privacy-sensitive application resources. */

#ifndef RIN_API_RIN_RESOURCE_CAPABILITY_H
#define RIN_API_RIN_RESOURCE_CAPABILITY_H

#include <stdint.h>

#include <rin/abi.h>
#include <rin/resource/subject.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_RESOURCE_CAPABILITY_VERSION UINT32_C(0x00010000)
#define RIN_DEVICE_CAPABILITY_BROKER_VERSION RIN_RESOURCE_SUBJECT_VERSION
#define RIN_DEVICE_CAPABILITY_APPLICATION_ID_SIZE \
    RIN_RESOURCE_SUBJECT_APPLICATION_ID_SIZE
#define RIN_DEVICE_CAPABILITY_MAX_HANDLES 64u
#define RIN_DEVICE_CAPABILITY_HANDLE_SLOT_BITS 8u
#define RIN_DEVICE_CAPABILITY_HANDLE_GENERATION_MAX \
    (UINT64_MAX >> RIN_DEVICE_CAPABILITY_HANDLE_SLOT_BITS)

#define RIN_DEVICE_RIGHT_READ UINT32_C(0x00000001)
#define RIN_DEVICE_RIGHT_WRITE UINT32_C(0x00000002)
#define RIN_DEVICE_RIGHT_CONTROL UINT32_C(0x00000004)
#define RIN_DEVICE_RIGHT_MAP UINT32_C(0x00000008)
#define RIN_DEVICE_RIGHT_ALL                                              \
    (RIN_DEVICE_RIGHT_READ | RIN_DEVICE_RIGHT_WRITE |                    \
     RIN_DEVICE_RIGHT_CONTROL | RIN_DEVICE_RIGHT_MAP)

typedef enum RinAppResource {
    RIN_APP_RESOURCE_NETWORK = 1,
    RIN_APP_RESOURCE_CLIPBOARD = 2,
    RIN_APP_RESOURCE_NOTIFICATION = 3,
    RIN_APP_RESOURCE_GPU = 4,
    RIN_APP_RESOURCE_MICROPHONE = 5,
    RIN_APP_RESOURCE_CAMERA = 6,
    RIN_APP_RESOURCE_MIDI = 7,
    RIN_APP_RESOURCE_LOCATION = 8
} RinAppResource;

typedef struct RinResourceCapabilityRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    RinCapabilitySet granted_capabilities;
    uint32_t resource;
    uint32_t flags;
    uint64_t reserved[3];
} RinResourceCapabilityRequestV1;

typedef struct RinResourceCapabilityResultV1 {
    uint32_t struct_size;
    uint32_t version;
    RinCapabilitySet required_capability;
    uint32_t allowed;
    uint32_t reserved0;
    uint64_t reserved[1];
} RinResourceCapabilityResultV1;

typedef enum RinResourceCapabilityStatus {
    RIN_RESOURCE_CAPABILITY_OK = 0,
    RIN_RESOURCE_CAPABILITY_INVALID_ARGUMENT = -1,
    RIN_RESOURCE_CAPABILITY_DENIED = -2
} RinResourceCapabilityStatus;

typedef RinResourceSubjectV1 RinDeviceCapabilitySubjectV1;

typedef struct RinDeviceCapabilityHandleRecordV1 {
    uint64_t generation;
    uint64_t owner_instance_cookie;
    uint8_t owner_application_id[RIN_DEVICE_CAPABILITY_APPLICATION_ID_SIZE];
    uint32_t owner_uid;
    uint32_t owner_process_id;
    uint32_t resource;
    uint32_t rights;
    uint32_t use_count;
    uint8_t active;
    uint8_t closing;
    uint8_t reserved[2];
} RinDeviceCapabilityHandleRecordV1;

typedef struct RinDeviceCapabilityBrokerV1 {
    volatile uint32_t lock;
    uint32_t reserved0;
    RinDeviceCapabilityHandleRecordV1
        handles[RIN_DEVICE_CAPABILITY_MAX_HANDLES];
} RinDeviceCapabilityBrokerV1;

typedef struct RinDeviceCapabilityLeaseV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t handle;
    uint32_t resource;
    uint32_t rights;
} RinDeviceCapabilityLeaseV1;

typedef enum RinDeviceCapabilityStatus {
    RIN_DEVICE_CAPABILITY_OK = 0,
    RIN_DEVICE_CAPABILITY_INVALID_ARGUMENT = -1,
    RIN_DEVICE_CAPABILITY_DENIED = -2,
    RIN_DEVICE_CAPABILITY_NOT_FOUND = -3,
    RIN_DEVICE_CAPABILITY_LIMIT_EXCEEDED = -4
} RinDeviceCapabilityStatus;

/* The result is initialized on every call. Unknown resources, unknown
 * capability bits, flags and reserved fields fail closed. A caller must use
 * only the kernel-owned current capability set recorded on the process; this
 * structure is not a user-provided grant mechanism. */
RinResourceCapabilityStatus rin_resource_capability_authorize(
    const RinResourceCapabilityRequestV1* request,
    RinResourceCapabilityResultV1* result);

/* Initialize a broker before it becomes visible to callers. This is not a
 * runtime revocation operation; use revoke_process for published brokers. */
void rin_device_capability_broker_reset(RinDeviceCapabilityBrokerV1* broker);

/* Only GPU, microphone, camera, MIDI and location handles are issued here.
 * The subject must come from the current kernel process record, never from
 * user memory. A handle remains bound to the exact signed process identity. */
RinDeviceCapabilityStatus rin_device_capability_handle_open(
    RinDeviceCapabilityBrokerV1* broker,
    const RinDeviceCapabilitySubjectV1* subject,
    uint32_t resource,
    uint32_t rights,
    uint64_t* handle_out);

/* Device data paths acquire a lease before touching hardware or copying
 * device data. Current capability and process identity are rechecked on every
 * acquire. close/revoke prevents new leases and waits for existing leases to
 * be released before the slot can be reused. */
RinDeviceCapabilityStatus rin_device_capability_handle_acquire(
    RinDeviceCapabilityBrokerV1* broker,
    const RinDeviceCapabilitySubjectV1* subject,
    uint64_t handle,
    uint32_t resource,
    uint32_t rights,
    RinDeviceCapabilityLeaseV1* lease_out);

RinDeviceCapabilityStatus rin_device_capability_handle_release(
    RinDeviceCapabilityBrokerV1* broker,
    const RinDeviceCapabilitySubjectV1* subject,
    const RinDeviceCapabilityLeaseV1* lease);

RinDeviceCapabilityStatus rin_device_capability_handle_close(
    RinDeviceCapabilityBrokerV1* broker,
    const RinDeviceCapabilitySubjectV1* subject,
    uint64_t handle);

uint32_t rin_device_capability_revoke_process(
    RinDeviceCapabilityBrokerV1* broker,
    uint32_t owner_uid,
    uint32_t process_id,
    uint64_t process_instance_cookie);

RinDeviceCapabilityBrokerV1* rin_device_capability_product_broker(void);

#if defined(__cplusplus)
static_assert(sizeof(RinResourceCapabilityRequestV1) == 48u,
              "RinResourceCapabilityRequestV1 ABI drift");
static_assert(sizeof(RinResourceCapabilityResultV1) == 32u,
              "RinResourceCapabilityResultV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinResourceCapabilityRequestV1) == 48u,
               "RinResourceCapabilityRequestV1 ABI drift");
_Static_assert(sizeof(RinResourceCapabilityResultV1) == 32u,
               "RinResourceCapabilityResultV1 ABI drift");
#endif

#if defined(__cplusplus)
static_assert(sizeof(RinDeviceCapabilitySubjectV1) == 80u,
              "RinDeviceCapabilitySubjectV1 ABI drift");
static_assert(sizeof(RinDeviceCapabilityHandleRecordV1) == 72u,
              "RinDeviceCapabilityHandleRecordV1 ABI drift");
static_assert(sizeof(RinDeviceCapabilityLeaseV1) == 24u,
              "RinDeviceCapabilityLeaseV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinDeviceCapabilitySubjectV1) == 80u,
               "RinDeviceCapabilitySubjectV1 ABI drift");
_Static_assert(sizeof(RinDeviceCapabilityHandleRecordV1) == 72u,
               "RinDeviceCapabilityHandleRecordV1 ABI drift");
_Static_assert(sizeof(RinDeviceCapabilityLeaseV1) == 24u,
               "RinDeviceCapabilityLeaseV1 ABI drift");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RIN_API_RIN_RESOURCE_CAPABILITY_H */
