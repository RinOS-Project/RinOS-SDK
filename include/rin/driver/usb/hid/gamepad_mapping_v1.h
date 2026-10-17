/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Versioned standard mapping owner for generic USB HID gamepads. */

#ifndef RIN_KERNEL_RIN_USB_HID_GAMEPAD_MAPPING_V1_H
#define RIN_KERNEL_RIN_USB_HID_GAMEPAD_MAPPING_V1_H

#include <stdint.h>

#include <rin/driver/usb/hid/gamepad_v1.h>
#include "subsystems/rinnt/service_input.h"
#include "rin/contract_abi.h"
#define RIN_USB_HID_GAMEPAD_MAPPING_V1 1u
#define RIN_USB_HID_GAMEPAD_MAPPING_AXIS_ABSENT UINT8_C(0xff)
#define RIN_USB_HID_GAMEPAD_MAPPING_MAX_BUTTONS 32u

/* XInput-compatible native button bits. */
#define RIN_USB_HID_GAMEPAD_MAPPING_DPAD_UP UINT16_C(0x0001)
#define RIN_USB_HID_GAMEPAD_MAPPING_DPAD_DOWN UINT16_C(0x0002)
#define RIN_USB_HID_GAMEPAD_MAPPING_DPAD_LEFT UINT16_C(0x0004)
#define RIN_USB_HID_GAMEPAD_MAPPING_DPAD_RIGHT UINT16_C(0x0008)
#define RIN_USB_HID_GAMEPAD_MAPPING_START UINT16_C(0x0010)
#define RIN_USB_HID_GAMEPAD_MAPPING_BACK UINT16_C(0x0020)
#define RIN_USB_HID_GAMEPAD_MAPPING_LEFT_THUMB UINT16_C(0x0040)
#define RIN_USB_HID_GAMEPAD_MAPPING_RIGHT_THUMB UINT16_C(0x0080)
#define RIN_USB_HID_GAMEPAD_MAPPING_LEFT_SHOULDER UINT16_C(0x0100)
#define RIN_USB_HID_GAMEPAD_MAPPING_RIGHT_SHOULDER UINT16_C(0x0200)
#define RIN_USB_HID_GAMEPAD_MAPPING_A UINT16_C(0x1000)
#define RIN_USB_HID_GAMEPAD_MAPPING_B UINT16_C(0x2000)
#define RIN_USB_HID_GAMEPAD_MAPPING_X UINT16_C(0x4000)
#define RIN_USB_HID_GAMEPAD_MAPPING_Y UINT16_C(0x8000)

typedef struct RinUsbHidGamepadMappingV1 {
    uint32_t version;
    uint32_t struct_size;
    uint16_t button_masks[RIN_USB_HID_GAMEPAD_MAPPING_MAX_BUTTONS];
    uint8_t thumb_lx_axis;
    uint8_t thumb_ly_axis;
    uint8_t thumb_rx_axis;
    uint8_t thumb_ry_axis;
    uint8_t left_trigger_axis;
    uint8_t right_trigger_axis;
    uint8_t invert_axis_mask;
    uint8_t reserved0;
    uint32_t reserved[4];
} RinUsbHidGamepadMappingV1;

RinResultCode rin_usb_hid_gamepad_mapping_default(
    RinUsbHidGamepadMappingV1* mapping);

/* Validates a caller-provided mapping without publishing or mutating it. */
RinResultCode rin_usb_hid_gamepad_mapping_validate(
    const RinUsbHidGamepadMappingV1* mapping);

RinResultCode rin_usb_hid_gamepad_mapping_apply(
    const RinUsbHidGamepadMappingV1* mapping,
    const RinUsbHidGamepadSampleV1* sample,
    uint32_t user_index, uint64_t device_id, uint64_t device_generation,
    uint64_t sample_sequence, RinNTServiceInputNativeGamepadV1* state_out);

#if defined(__cplusplus)
static_assert(sizeof(RinUsbHidGamepadMappingV1) == 96u,
              "USB HID gamepad mapping ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinUsbHidGamepadMappingV1) == 96u,
               "USB HID gamepad mapping ABI drift");
#endif

#endif /* RIN_KERNEL_RIN_USB_HID_GAMEPAD_MAPPING_V1_H */
