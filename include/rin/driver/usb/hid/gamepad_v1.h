/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_DRIVERS_USB_HID_GAMEPAD_V1_H
#define RIN_DRIVERS_USB_HID_GAMEPAD_V1_H

#include <stdint.h>

#define RIN_USB_HID_GAMEPAD_ABI_VERSION 1u
#define RIN_USB_HID_GAMEPAD_MAX_DESCRIPTOR_BYTES 512u
#define RIN_USB_HID_GAMEPAD_MAX_REPORT_BYTES 64u
#define RIN_USB_HID_GAMEPAD_MAX_AXES 9u
#define RIN_USB_HID_GAMEPAD_MAX_BUTTONS 32u

#define RIN_USB_HID_GAMEPAD_APPLICATION_JOYSTICK 0x04u
#define RIN_USB_HID_GAMEPAD_APPLICATION_GAME_PAD 0x05u

#define RIN_USB_HID_GAMEPAD_AXIS_X 0u
#define RIN_USB_HID_GAMEPAD_AXIS_Y 1u
#define RIN_USB_HID_GAMEPAD_AXIS_Z 2u
#define RIN_USB_HID_GAMEPAD_AXIS_RX 3u
#define RIN_USB_HID_GAMEPAD_AXIS_RY 4u
#define RIN_USB_HID_GAMEPAD_AXIS_RZ 5u
#define RIN_USB_HID_GAMEPAD_AXIS_SLIDER 6u
#define RIN_USB_HID_GAMEPAD_AXIS_DIAL 7u
#define RIN_USB_HID_GAMEPAD_AXIS_WHEEL 8u

#define RIN_USB_HID_GAMEPAD_PROFILE_HAS_REPORT_ID (1u << 0)
#define RIN_USB_HID_GAMEPAD_PROFILE_HAS_HAT (1u << 1)

#define RIN_USB_HID_GAMEPAD_OUTPUT_HAS_REPORT_ID (1u << 0)

#define RIN_USB_HID_GAMEPAD_CONTROL_SIGNED (1u << 0)
#define RIN_USB_HID_GAMEPAD_CONTROL_NULL_STATE (1u << 1)

#define RIN_USB_HID_GAMEPAD_SAMPLE_HAS_HAT (1u << 0)
#define RIN_USB_HID_GAMEPAD_HAT_NEUTRAL 8u

typedef enum RinUsbHidGamepadResult {
    RIN_USB_HID_GAMEPAD_OK = 0,
    RIN_USB_HID_GAMEPAD_INVALID_ARGUMENT = -1,
    RIN_USB_HID_GAMEPAD_MALFORMED = -2,
    RIN_USB_HID_GAMEPAD_UNSUPPORTED = -3,
    RIN_USB_HID_GAMEPAD_LIMIT_EXCEEDED = -4
} RinUsbHidGamepadResult;

typedef struct RinUsbHidGamepadControlV1 {
    uint16_t bit_offset;
    uint8_t bit_size;
    uint8_t flags;
    int32_t logical_minimum;
    int32_t logical_maximum;
} RinUsbHidGamepadControlV1;

typedef struct RinUsbHidGamepadProfileV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t descriptor_size;
    uint32_t report_size;
    uint32_t report_bit_count;
    uint32_t application_usage;
    uint32_t flags;
    uint32_t axis_mask;
    uint32_t button_count;
    uint8_t report_id;
    uint8_t hat_positions;
    uint16_t reserved0;
    RinUsbHidGamepadControlV1 axes[RIN_USB_HID_GAMEPAD_MAX_AXES];
    /* UINT16_MAX denotes an absent Button-page usage. */
    uint16_t button_bit_offsets[RIN_USB_HID_GAMEPAD_MAX_BUTTONS];
    RinUsbHidGamepadControlV1 hat;
    uint32_t reserved[4];
} RinUsbHidGamepadProfileV1;

/* This is intentionally an opaque transport admission record: v1 validates
 * one exact byte-aligned Output report but does not assign HID PID or vendor
 * usages to a vibration/force-feedback policy. */
typedef struct RinUsbHidGamepadOutputProfileV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t descriptor_size;
    uint32_t report_size;
    uint32_t report_bit_count;
    uint32_t flags;
    uint8_t report_id;
    uint8_t reserved0[3];
    /* Explicitly occupy the 32-bit alignment gap so the public record is
     * 40 bytes on both i686 and x86_64. */
    uint32_t reserved1;
    uint64_t reserved[1];
} RinUsbHidGamepadOutputProfileV1;

typedef struct RinUsbHidGamepadSampleV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t flags;
    uint32_t axis_mask;
    uint32_t buttons;
    int16_t axes[RIN_USB_HID_GAMEPAD_MAX_AXES];
    uint8_t hat;
    uint8_t report_id;
    uint32_t reserved[6];
} RinUsbHidGamepadSampleV1;

RinUsbHidGamepadResult rin_usb_hid_gamepad_admit_descriptor(
    const uint8_t* descriptor, uint32_t descriptor_size,
    RinUsbHidGamepadProfileV1* profile);

RinUsbHidGamepadResult rin_usb_hid_gamepad_decode_report(
    const RinUsbHidGamepadProfileV1* profile,
    const uint8_t* report, uint32_t report_size,
    RinUsbHidGamepadSampleV1* sample);

/* Admits exactly one Data/Variable/Absolute, byte-aligned Output report in
 * the same Game Pad/Joystick application collection as the input profile. */
RinUsbHidGamepadResult rin_usb_hid_gamepad_admit_output_descriptor(
    const uint8_t* descriptor, uint32_t descriptor_size,
    RinUsbHidGamepadOutputProfileV1* profile);

RinUsbHidGamepadResult rin_usb_hid_gamepad_validate_output_report(
    const RinUsbHidGamepadOutputProfileV1* profile,
    const uint8_t* report, uint32_t report_size);

#if defined(__cplusplus)
static_assert(sizeof(RinUsbHidGamepadControlV1) == 12u,
              "USB HID gamepad control ABI drift");
static_assert(sizeof(RinUsbHidGamepadProfileV1) == 240u,
              "USB HID gamepad profile ABI drift");
static_assert(sizeof(RinUsbHidGamepadOutputProfileV1) == 40u,
              "USB HID gamepad output profile ABI drift");
static_assert(sizeof(RinUsbHidGamepadSampleV1) == 64u,
              "USB HID gamepad sample ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinUsbHidGamepadControlV1) == 12u,
               "USB HID gamepad control ABI drift");
_Static_assert(sizeof(RinUsbHidGamepadProfileV1) == 240u,
               "USB HID gamepad profile ABI drift");
_Static_assert(sizeof(RinUsbHidGamepadOutputProfileV1) == 40u,
               "USB HID gamepad output profile ABI drift");
_Static_assert(sizeof(RinUsbHidGamepadSampleV1) == 64u,
               "USB HID gamepad sample ABI drift");
#endif

#endif /* RIN_DRIVERS_USB_HID_GAMEPAD_V1_H */
