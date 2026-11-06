/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_COMPAT_HID_INPUT_PROVIDER_V1_H
#define RIN_SDK_COMPAT_HID_INPUT_PROVIDER_V1_H

#include <stdint.h>

#include <rin/compat/nt_service_abi.h>
#include <rin/contract_abi.h>
#include <rin/syscall.h>
#include <rin/syscall_abi.h>
#include <rin/input/gamepad_state_v1.h>

#define RIN_HID_INPUT_PROVIDER_ABI_VERSION 1u
#define RIN_HID_INPUT_PROVIDER_MAX_BYTES 288u

typedef enum RinHidInputProviderOperationV1 {
    RIN_HID_INPUT_PROVIDER_QUERY_GAMEPAD = 1,
    RIN_HID_INPUT_PROVIDER_SET_VIBRATION = 2,
    RIN_HID_INPUT_PROVIDER_READ_RAW = 3,
    RIN_HID_INPUT_PROVIDER_REVOKE_PROCESS = 4,
    RIN_HID_INPUT_PROVIDER_SENSOR_SET_POWER = 5,
    RIN_HID_INPUT_PROVIDER_SENSOR_GET_POWER_STATUS = 6
} RinHidInputProviderOperationV1;

typedef enum RinHidInputProviderResultV1 {
    RIN_HID_INPUT_PROVIDER_OK = 0,
    RIN_HID_INPUT_PROVIDER_NOT_FOUND = 1,
    RIN_HID_INPUT_PROVIDER_TEMPORARY = 2,
    RIN_HID_INPUT_PROVIDER_IO_ERROR = 3,
    RIN_HID_INPUT_PROVIDER_ACCESS_DENIED = 4,
    RIN_HID_INPUT_PROVIDER_NOT_SUPPORTED = 5
} RinHidInputProviderResultV1;

/* This subject is copied from the authenticated RinNT peer by wincompatregd.
 * The syscall accepts it only from that exact trusted system service. */
typedef struct RinHidInputProviderSubjectV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t process_id;
    uint64_t process_start_cookie;
    uint64_t connection_id;
    uint64_t process_cookie;
    uint32_t uid;
    uint16_t scope;
    uint16_t reserved;
    uint32_t flags;
    uint32_t reserved2;
    uint64_t reserved3;
} RinHidInputProviderSubjectV1;

typedef struct RinHidInputProviderNativeEventV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t event_type;
    uint32_t flags;
    uint64_t sequence;
    uint64_t timestamp_ns;
    uint64_t device_id;
    uint64_t device_generation;
    int32_t data[6];
    uint64_t reserved;
} RinHidInputProviderNativeEventV1;

/* One fixed-size in/out request keeps user pointers out of provider payloads.
 * Only the result and matching state/event output are written by the kernel. */
typedef struct RinHidInputProviderCallV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t operation;
    uint32_t flags;
    RinHidInputProviderSubjectV1 subject;
    uint32_t user_index;
    uint32_t class_mask;
    uint16_t left_motor_speed;
    uint16_t right_motor_speed;
    uint32_t reserved0;
    uint64_t device_id;
    uint64_t device_generation;
    uint64_t after_sequence;
    int32_t provider_result;
    uint32_t reserved1;
    RinGamepadStateV1 gamepad;
    RinHidInputProviderNativeEventV1 event;
    uint64_t reserved2[2];
} RinHidInputProviderCallV1;

/* SENSOR_SET_POWER uses user_index for the requested power state, class_mask
 * for the HID sensor type, and device_generation for the live attachment.
 * SENSOR_GET_POWER_STATUS has those inputs zero except class_mask and device
 * generation; on success class_mask is preserved, user_index receives the
 * live power state, reserved0 receives the status, and reserved1 the driver's
 * RinResultCode completion result. All unrelated request fields stay zero. */

static inline RinResultCode rin_hid_input_provider_call_v1(
    RinHidInputProviderCallV1* request)
{
    long result;
    if (request == (RinHidInputProviderCallV1*)0) {
        return RIN_RESULT_INVALID_ARGUMENT;
    }
    result = syscall((long)RIN_SYS_HID_INPUT_PROVIDER_CALL,
                     (uintptr_t)request);
    return (RinResultCode)result;
}

#if defined(__cplusplus)
static_assert(sizeof(RinHidInputProviderSubjectV1) == 64u,
              "HID input provider subject ABI drift");
static_assert(sizeof(RinHidInputProviderNativeEventV1) == 80u,
              "HID input provider event ABI drift");
static_assert(sizeof(RinHidInputProviderCallV1) ==
                  RIN_HID_INPUT_PROVIDER_MAX_BYTES,
              "HID input provider call ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinHidInputProviderSubjectV1) == 64u,
               "HID input provider subject ABI drift");
_Static_assert(sizeof(RinHidInputProviderNativeEventV1) == 80u,
               "HID input provider event ABI drift");
_Static_assert(sizeof(RinHidInputProviderCallV1) ==
                   RIN_HID_INPUT_PROVIDER_MAX_BYTES,
               "HID input provider call ABI drift");
#endif

#endif /* RIN_SDK_COMPAT_HID_INPUT_PROVIDER_V1_H */
