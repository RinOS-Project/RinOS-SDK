/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Generation-bound PID force actuator and report serializer. */

#ifndef RIN_KERNEL_RIN_USB_HID_GAMEPAD_PID_ACTUATOR_V1_H
#define RIN_KERNEL_RIN_USB_HID_GAMEPAD_PID_ACTUATOR_V1_H

#include <stdint.h>

#include "gamepad_force_feedback_v1.h"
#include "gamepad_pid_effects_v1.h"

#define RIN_USB_HID_GAMEPAD_PID_ACTUATOR_VERSION 1u
#define RIN_USB_HID_GAMEPAD_PID_ACTUATOR_MAGIC UINT32_C(0x31544147)
#define RIN_USB_HID_GAMEPAD_PID_ACTUATOR_MAX_REPORT_BYTES \
    RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAX_REPORT_BYTES

#define RIN_USB_HID_GAMEPAD_PID_ACTUATOR_PERMISSION_USE UINT32_C(1)
#define RIN_USB_HID_GAMEPAD_PID_ACTUATOR_PERMISSION_STOP UINT32_C(2)

typedef enum RinUsbHidGamepadPidActuatorResult {
    RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK = 0,
    RIN_USB_HID_GAMEPAD_PID_ACTUATOR_INVALID_ARGUMENT = -1,
    RIN_USB_HID_GAMEPAD_PID_ACTUATOR_MALFORMED = -2,
    RIN_USB_HID_GAMEPAD_PID_ACTUATOR_UNSUPPORTED = -3,
    RIN_USB_HID_GAMEPAD_PID_ACTUATOR_ACCESS_DENIED = -4,
    RIN_USB_HID_GAMEPAD_PID_ACTUATOR_BUSY = -5,
    RIN_USB_HID_GAMEPAD_PID_ACTUATOR_IO = -6,
    RIN_USB_HID_GAMEPAD_PID_ACTUATOR_STALE = -7,
    RIN_USB_HID_GAMEPAD_PID_ACTUATOR_RECOVERY_REQUIRED = -8
} RinUsbHidGamepadPidActuatorResult;

/* The serializer emits the report ID byte followed by the HID payload.  All
 * HID fields are little-endian bit streams as required by the USB HID
 * report format; no host-word-size casts are used. */
RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_serialize_set_effect(
    const RinUsbHidGamepadForceFeedbackProfileV1* profile,
    uint32_t effect_index, uint32_t duration_ms, uint8_t* report,
    uint32_t report_capacity, uint32_t* report_size_out);

RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_serialize_constant(
    const RinUsbHidGamepadForceFeedbackProfileV1* profile,
    uint32_t effect_index, int16_t magnitude, uint8_t* report,
    uint32_t report_capacity, uint32_t* report_size_out);

RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_serialize_operation(
    const RinUsbHidGamepadForceFeedbackProfileV1* profile,
    uint32_t effect_index, uint32_t operation, uint8_t* report,
    uint32_t report_capacity, uint32_t* report_size_out);

RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_serialize_stop_all(
    const RinUsbHidGamepadForceFeedbackProfileV1* profile,
    uint32_t effect_index, uint8_t* report, uint32_t report_capacity,
    uint32_t* report_size_out);

typedef int (*RinUsbHidGamepadPidActuatorAuthorizeFn)(
    void* context, uint64_t application_token, uint64_t device_id,
    uint32_t port_generation, uint32_t permission);

typedef int (*RinUsbHidGamepadPidActuatorSubmitFn)(
    void* context, uint64_t device_id, uint32_t port_generation,
    const uint8_t* report, uint32_t report_size);

typedef struct RinUsbHidGamepadPidActuatorConfigV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint64_t device_id;
    uint32_t port_generation;
    const RinUsbHidGamepadForceFeedbackProfileV1* profile;
    RinUsbHidGamepadPidActuatorAuthorizeFn authorize;
    void* authorize_context;
    RinUsbHidGamepadPidActuatorSubmitFn submit;
    void* submit_context;
    uint32_t reserved[4];
} RinUsbHidGamepadPidActuatorConfigV1;

typedef struct RinUsbHidGamepadPidActuatorOwnerV1 {
    uint32_t magic;
    volatile uint32_t lock;
    volatile uint32_t callback_active;
    uint32_t attached;
    uint32_t recovery_required;
    uint64_t device_id;
    uint32_t port_generation;
    uint32_t reserved0;
    uint32_t active_effect;
    RinUsbHidGamepadForceFeedbackProfileV1 profile;
    RinUsbHidGamepadPidActuatorAuthorizeFn authorize;
    void* authorize_context;
    RinUsbHidGamepadPidActuatorSubmitFn submit;
    void* submit_context;
    uint8_t report[RIN_USB_HID_GAMEPAD_PID_ACTUATOR_MAX_REPORT_BYTES];
} RinUsbHidGamepadPidActuatorOwnerV1;

RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_init(
    RinUsbHidGamepadPidActuatorOwnerV1* owner,
    const RinUsbHidGamepadPidActuatorConfigV1* config);

/* Send one normalized scheduler sample to the controller's Constant Force
 * report.  The effect block index is explicit and must be nonzero. */
RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_start(
    RinUsbHidGamepadPidActuatorOwnerV1* owner, uint64_t application_token,
    uint32_t effect_index, uint32_t duration_ms, int16_t magnitude);

RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_submit(
    RinUsbHidGamepadPidActuatorOwnerV1* owner, uint64_t application_token,
    uint32_t effect_index, int16_t magnitude);

/* Consume one scheduler output without exposing a second report-formatting
 * path.  The output ABI and reserved bytes are validated before its force is
 * routed through submit(). */
RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_submit_output(
    RinUsbHidGamepadPidActuatorOwnerV1* owner, uint64_t application_token,
    uint32_t effect_index,
    const RinUsbHidGamepadPidEffectOutputV1* output);

/* Synchronize one scheduler output with the hardware lifecycle.  A non-empty
 * output starts an idle actuator or updates the same active effect; an empty
 * output stops that effect.  The actuator admits only one effect at a time. */
RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_sync_output(
    RinUsbHidGamepadPidActuatorOwnerV1* owner, uint64_t application_token,
    uint32_t effect_index, uint32_t duration_ms,
    const RinUsbHidGamepadPidEffectOutputV1* output);

/* Stop an effect and then issue Device Control Stop All.  A failure leaves
 * recovery_required set so callers cannot continue emitting force. */
RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_stop(
    RinUsbHidGamepadPidActuatorOwnerV1* owner, uint64_t application_token,
    uint32_t effect_index);

RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_recover(
    RinUsbHidGamepadPidActuatorOwnerV1* owner, uint64_t device_id,
    uint32_t port_generation);

RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_detach(
    RinUsbHidGamepadPidActuatorOwnerV1* owner, uint64_t device_id,
    uint32_t port_generation);

#if defined(__cplusplus)
static_assert(sizeof(RinUsbHidGamepadPidActuatorConfigV1) ==
                  (__SIZEOF_POINTER__ == 4 ? 56u : 80u),
              "PID actuator config ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinUsbHidGamepadPidActuatorConfigV1) ==
                   (__SIZEOF_POINTER__ == 4 ? 56u : 80u),
               "PID actuator config ABI drift");
#endif

#endif /* RIN_KERNEL_RIN_USB_HID_GAMEPAD_PID_ACTUATOR_V1_H */
