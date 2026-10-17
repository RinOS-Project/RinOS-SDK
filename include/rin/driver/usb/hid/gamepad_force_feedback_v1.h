/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Bounded, permission-gated USB HID PID force-feedback owner. */

#ifndef RIN_KERNEL_RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_V1_H
#define RIN_KERNEL_RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_V1_H

#include <stdint.h>

#define RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_VERSION 1u
#define RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAGIC UINT32_C(0x31464647)
#define RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAX_DESCRIPTOR_BYTES 512u
#define RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAX_REPORT_BYTES 64u
#define RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAX_DURATION_MS 60000u

#define RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_OPERATION_START 1u
#define RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_OPERATION_STOP 2u
#define RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_PERMISSION_USE UINT32_C(1)
#define RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_PERMISSION_STOP UINT32_C(2)

typedef enum RinUsbHidGamepadForceFeedbackResult {
    RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_OK = 0,
    RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_INVALID_ARGUMENT = -1,
    RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MALFORMED = -2,
    RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_UNSUPPORTED = -3,
    RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_ACCESS_DENIED = -4,
    RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_BUSY = -5,
    RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_IO = -6,
    RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_DEVICE_LOST = -7,
    RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_RECOVERY_REQUIRED = -8
} RinUsbHidGamepadForceFeedbackResult;

typedef struct RinUsbHidGamepadForceFeedbackReportV1 {
    uint32_t report_size;
    uint32_t report_bit_count;
    uint8_t report_id;
    uint8_t reserved0[3];
    uint16_t index_bit_offset;
    uint8_t index_bit_size;
    uint8_t reserved1;
    uint16_t value_bit_offset;
    uint8_t value_bit_size;
    uint8_t reserved2;
    int32_t value_minimum;
    int32_t value_maximum;
    uint32_t value_start;
    uint32_t value_stop;
} RinUsbHidGamepadForceFeedbackReportV1;

typedef struct RinUsbHidGamepadForceFeedbackProfileV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t descriptor_size;
    uint32_t flags;
    RinUsbHidGamepadForceFeedbackReportV1 set_effect;
    RinUsbHidGamepadForceFeedbackReportV1 set_constant;
    RinUsbHidGamepadForceFeedbackReportV1 effect_operation;
    RinUsbHidGamepadForceFeedbackReportV1 device_control;
    uint32_t reserved[4];
} RinUsbHidGamepadForceFeedbackProfileV1;

typedef int (*RinUsbHidGamepadForceFeedbackAuthorizeFn)(
    void* context, uint64_t application_token, uint64_t device_id,
    uint32_t port_generation, uint32_t permission);

typedef int (*RinUsbHidGamepadForceFeedbackSubmitFn)(
    void* context, uint64_t device_id, uint32_t port_generation,
    const uint8_t* report, uint32_t report_size);

typedef struct RinUsbHidGamepadForceFeedbackConfigV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint64_t device_id;
    uint32_t port_generation;
    const RinUsbHidGamepadForceFeedbackProfileV1* profile;
    RinUsbHidGamepadForceFeedbackAuthorizeFn authorize;
    void* authorize_context;
    RinUsbHidGamepadForceFeedbackSubmitFn submit;
    void* submit_context;
    uint32_t reserved;
} RinUsbHidGamepadForceFeedbackConfigV1;

typedef struct RinUsbHidGamepadForceFeedbackRequestV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint64_t application_token;
    int16_t magnitude;
    uint16_t duration_ms;
    uint32_t reserved[3];
} RinUsbHidGamepadForceFeedbackRequestV1;

typedef struct RinUsbHidGamepadForceFeedbackOwnerV1 {
    uint32_t magic;
    volatile uint32_t lock;
    uint32_t callback_active;
    uint32_t attached;
    uint32_t active;
    uint32_t recovery_required;
    uint64_t device_id;
    uint32_t port_generation;
    uint32_t reserved0;
    RinUsbHidGamepadForceFeedbackProfileV1 profile;
    RinUsbHidGamepadForceFeedbackAuthorizeFn authorize;
    void* authorize_context;
    RinUsbHidGamepadForceFeedbackSubmitFn submit;
    void* submit_context;
    uint8_t report[RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAX_REPORT_BYTES];
} RinUsbHidGamepadForceFeedbackOwnerV1;

RinUsbHidGamepadForceFeedbackResult
rin_usb_hid_gamepad_force_feedback_admit_descriptor(
    const uint8_t* descriptor, uint32_t descriptor_size,
    RinUsbHidGamepadForceFeedbackProfileV1* profile);

RinUsbHidGamepadForceFeedbackResult
rin_usb_hid_gamepad_force_feedback_init(
    RinUsbHidGamepadForceFeedbackOwnerV1* owner,
    const RinUsbHidGamepadForceFeedbackConfigV1* config);

RinUsbHidGamepadForceFeedbackResult
rin_usb_hid_gamepad_force_feedback_start(
    RinUsbHidGamepadForceFeedbackOwnerV1* owner,
    const RinUsbHidGamepadForceFeedbackRequestV1* request);

RinUsbHidGamepadForceFeedbackResult
rin_usb_hid_gamepad_force_feedback_stop(
    RinUsbHidGamepadForceFeedbackOwnerV1* owner,
    uint64_t application_token);

/* Privileged retry after an uncertain submit. It clears recovery_required only
 * after both stop reports are accepted. */
RinUsbHidGamepadForceFeedbackResult
rin_usb_hid_gamepad_force_feedback_recover(
    RinUsbHidGamepadForceFeedbackOwnerV1* owner,
    uint64_t device_id, uint32_t port_generation);

/* Privileged detach path. It never needs application permission, but it does
 * not forget an active effect unless the physical stop reports were accepted. */
RinUsbHidGamepadForceFeedbackResult
rin_usb_hid_gamepad_force_feedback_detach(
    RinUsbHidGamepadForceFeedbackOwnerV1* owner,
    uint64_t device_id, uint32_t port_generation);

#if defined(__cplusplus)
static_assert(sizeof(RinUsbHidGamepadForceFeedbackReportV1) == 36u,
              "USB HID force-feedback report ABI drift");
static_assert(sizeof(RinUsbHidGamepadForceFeedbackProfileV1) == 176u,
              "USB HID force-feedback profile ABI drift");
static_assert(sizeof(RinUsbHidGamepadForceFeedbackRequestV1) == 32u,
              "USB HID force-feedback request ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinUsbHidGamepadForceFeedbackReportV1) == 36u,
               "USB HID force-feedback report ABI drift");
_Static_assert(sizeof(RinUsbHidGamepadForceFeedbackProfileV1) == 176u,
               "USB HID force-feedback profile ABI drift");
_Static_assert(sizeof(RinUsbHidGamepadForceFeedbackRequestV1) == 32u,
               "USB HID force-feedback request ABI drift");
#endif

#endif /* RIN_KERNEL_RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_V1_H */
