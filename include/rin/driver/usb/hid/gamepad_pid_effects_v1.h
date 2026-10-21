/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Generation-bound normalized PID effect pool and scheduler. */

#ifndef RIN_KERNEL_RIN_USB_HID_GAMEPAD_PID_EFFECTS_V1_H
#define RIN_KERNEL_RIN_USB_HID_GAMEPAD_PID_EFFECTS_V1_H

#include <stdint.h>

#include "gamepad_force_feedback_v1.h"

#define RIN_USB_HID_GAMEPAD_PID_EFFECTS_VERSION 1u
#define RIN_USB_HID_GAMEPAD_PID_EFFECTS_MAGIC UINT32_C(0x31504547)
#define RIN_USB_HID_GAMEPAD_PID_EFFECTS_MAX_EFFECTS 8u
#define RIN_USB_HID_GAMEPAD_PID_EFFECTS_MAX_DURATION_MS 60000u

#define RIN_USB_HID_GAMEPAD_PID_EFFECT_CONSTANT UINT32_C(1)
#define RIN_USB_HID_GAMEPAD_PID_EFFECT_RAMP UINT32_C(2)
#define RIN_USB_HID_GAMEPAD_PID_EFFECT_PERIODIC UINT32_C(4)
#define RIN_USB_HID_GAMEPAD_PID_EFFECT_CONDITIONAL UINT32_C(8)

#define RIN_USB_HID_GAMEPAD_PID_EFFECT_PERMISSION_USE UINT32_C(1)
#define RIN_USB_HID_GAMEPAD_PID_EFFECT_PERMISSION_STOP UINT32_C(2)

typedef enum RinUsbHidGamepadPidEffectsResult {
    RIN_USB_HID_GAMEPAD_PID_EFFECTS_OK = 0,
    RIN_USB_HID_GAMEPAD_PID_EFFECTS_INVALID_ARGUMENT = -1,
    RIN_USB_HID_GAMEPAD_PID_EFFECTS_MALFORMED = -2,
    RIN_USB_HID_GAMEPAD_PID_EFFECTS_UNSUPPORTED = -3,
    RIN_USB_HID_GAMEPAD_PID_EFFECTS_ACCESS_DENIED = -4,
    RIN_USB_HID_GAMEPAD_PID_EFFECTS_BUSY = -5,
    RIN_USB_HID_GAMEPAD_PID_EFFECTS_STALE = -6,
    RIN_USB_HID_GAMEPAD_PID_EFFECTS_IO = -7,
    RIN_USB_HID_GAMEPAD_PID_EFFECTS_RECOVERY_REQUIRED = -8,
    RIN_USB_HID_GAMEPAD_PID_EFFECTS_NOT_FOUND = -9,
    RIN_USB_HID_GAMEPAD_PID_EFFECTS_LIMIT_EXCEEDED = -10
} RinUsbHidGamepadPidEffectsResult;

typedef struct RinUsbHidGamepadPidEffectsProfileV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t descriptor_size;
    uint32_t supported_types;
    RinUsbHidGamepadForceFeedbackProfileV1 safety;
    uint32_t max_effects;
    uint32_t reserved[3];
} RinUsbHidGamepadPidEffectsProfileV1;

typedef struct RinUsbHidGamepadPidEffectSpecV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t type;
    uint32_t flags;
    uint32_t duration_ms;
    uint32_t period_ms;
    uint32_t phase_ms;
    int16_t start_value;
    int16_t end_value;
    int16_t offset;
    int16_t positive;
    int16_t negative;
    uint16_t reserved0;
    uint32_t reserved[4];
} RinUsbHidGamepadPidEffectSpecV1;

typedef struct RinUsbHidGamepadPidEffectOutputV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    int16_t force;
    uint16_t active_count;
    uint32_t sequence;
    uint32_t reserved[4];
} RinUsbHidGamepadPidEffectOutputV1;

typedef int (*RinUsbHidGamepadPidEffectsAuthorizeFn)(
    void* context, uint64_t application_token, uint64_t device_id,
    uint32_t port_generation, uint32_t permission);

typedef int (*RinUsbHidGamepadPidEffectsSubmitFn)(
    void* context, uint64_t device_id, uint32_t port_generation,
    uint32_t effect_id, const RinUsbHidGamepadPidEffectSpecV1* spec,
    uint32_t running);

typedef struct RinUsbHidGamepadPidEffectsConfigV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint64_t device_id;
    uint32_t port_generation;
    const RinUsbHidGamepadPidEffectsProfileV1* profile;
    RinUsbHidGamepadPidEffectsAuthorizeFn authorize;
    void* authorize_context;
    RinUsbHidGamepadPidEffectsSubmitFn submit;
    void* submit_context;
    uint32_t reserved[4];
} RinUsbHidGamepadPidEffectsConfigV1;

typedef struct RinUsbHidGamepadPidEffectSlotV1 {
    uint32_t effect_id;
    uint32_t running;
    uint64_t started_ms;
    RinUsbHidGamepadPidEffectSpecV1 spec;
} RinUsbHidGamepadPidEffectSlotV1;

typedef struct RinUsbHidGamepadPidEffectsOwnerV1 {
    uint32_t magic;
    volatile uint32_t lock;
    volatile uint32_t callback_active;
    uint32_t attached;
    uint32_t recovery_required;
    uint32_t sequence;
    uint64_t device_id;
    uint32_t port_generation;
    uint32_t reserved0;
    RinUsbHidGamepadPidEffectsProfileV1 profile;
    RinUsbHidGamepadPidEffectsAuthorizeFn authorize;
    void* authorize_context;
    RinUsbHidGamepadPidEffectsSubmitFn submit;
    void* submit_context;
    RinUsbHidGamepadPidEffectSlotV1 slots[RIN_USB_HID_GAMEPAD_PID_EFFECTS_MAX_EFFECTS];
} RinUsbHidGamepadPidEffectsOwnerV1;

RinUsbHidGamepadPidEffectsResult
rin_usb_hid_gamepad_pid_effects_admit_descriptor(
    const uint8_t* descriptor, uint32_t descriptor_size,
    RinUsbHidGamepadPidEffectsProfileV1* profile);

RinUsbHidGamepadPidEffectsResult rin_usb_hid_gamepad_pid_effects_init(
    RinUsbHidGamepadPidEffectsOwnerV1* owner,
    const RinUsbHidGamepadPidEffectsConfigV1* config);

RinUsbHidGamepadPidEffectsResult rin_usb_hid_gamepad_pid_effects_create(
    RinUsbHidGamepadPidEffectsOwnerV1* owner, uint64_t application_token,
    const RinUsbHidGamepadPidEffectSpecV1* spec, uint32_t* effect_id_out);

RinUsbHidGamepadPidEffectsResult rin_usb_hid_gamepad_pid_effects_start(
    RinUsbHidGamepadPidEffectsOwnerV1* owner, uint64_t application_token,
    uint32_t effect_id, uint64_t now_ms);

RinUsbHidGamepadPidEffectsResult rin_usb_hid_gamepad_pid_effects_stop(
    RinUsbHidGamepadPidEffectsOwnerV1* owner, uint64_t application_token,
    uint32_t effect_id);

RinUsbHidGamepadPidEffectsResult rin_usb_hid_gamepad_pid_effects_destroy(
    RinUsbHidGamepadPidEffectsOwnerV1* owner, uint64_t application_token,
    uint32_t effect_id);

RinUsbHidGamepadPidEffectsResult rin_usb_hid_gamepad_pid_effects_tick(
    RinUsbHidGamepadPidEffectsOwnerV1* owner, uint64_t now_ms,
    int16_t condition, RinUsbHidGamepadPidEffectOutputV1* output);

RinUsbHidGamepadPidEffectsResult rin_usb_hid_gamepad_pid_effects_recover(
    RinUsbHidGamepadPidEffectsOwnerV1* owner, uint64_t device_id,
    uint32_t port_generation);

RinUsbHidGamepadPidEffectsResult rin_usb_hid_gamepad_pid_effects_detach(
    RinUsbHidGamepadPidEffectsOwnerV1* owner, uint64_t device_id,
    uint32_t port_generation);

#if defined(__cplusplus)
static_assert(sizeof(RinUsbHidGamepadPidEffectSpecV1) == 56u,
              "PID effect spec ABI drift");
static_assert(sizeof(RinUsbHidGamepadPidEffectOutputV1) == 32u,
              "PID effect output ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinUsbHidGamepadPidEffectSpecV1) == 56u,
               "PID effect spec ABI drift");
_Static_assert(sizeof(RinUsbHidGamepadPidEffectOutputV1) == 32u,
               "PID effect output ABI drift");
#endif

#endif /* RIN_KERNEL_RIN_USB_HID_GAMEPAD_PID_EFFECTS_V1_H */
