/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Bounded, descriptor-independent calibration for generic USB HID gamepads. */

#ifndef RIN_DRIVER_USB_HID_GAMEPAD_CALIBRATION_V1_H
#define RIN_DRIVER_USB_HID_GAMEPAD_CALIBRATION_V1_H

#include <stdint.h>

#include <rin/driver/usb/hid/gamepad_v1.h>

#define RIN_USB_HID_GAMEPAD_CALIBRATION_V1 1u

typedef struct RinUsbHidGamepadCalibrationV1 {
    uint32_t version;
    uint32_t struct_size;
    int16_t minimum[RIN_USB_HID_GAMEPAD_MAX_AXES];
    int16_t maximum[RIN_USB_HID_GAMEPAD_MAX_AXES];
    int16_t center[RIN_USB_HID_GAMEPAD_MAX_AXES];
    int16_t deadzone[RIN_USB_HID_GAMEPAD_MAX_AXES];
    uint32_t reserved[4];
} RinUsbHidGamepadCalibrationV1;

RinUsbHidGamepadResult rin_usb_hid_gamepad_calibration_default(
    RinUsbHidGamepadCalibrationV1* calibration);

/* Validates a caller-provided calibration without publishing or mutating it. */
RinUsbHidGamepadResult rin_usb_hid_gamepad_calibration_validate(
    const RinUsbHidGamepadCalibrationV1* calibration);

/* Copy a validated sample and normalize each present axis around its
 * calibrated center.  The result remains a normal gamepad sample and can be
 * handed to the existing mapping owner.  No sample state is changed on
 * malformed input or calibration. */
RinUsbHidGamepadResult rin_usb_hid_gamepad_calibration_apply(
    const RinUsbHidGamepadCalibrationV1* calibration,
    const RinUsbHidGamepadSampleV1* sample,
    RinUsbHidGamepadSampleV1* calibrated_out);

#if defined(__cplusplus)
static_assert(sizeof(RinUsbHidGamepadCalibrationV1) == 96u,
              "USB HID gamepad calibration ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinUsbHidGamepadCalibrationV1) == 96u,
               "USB HID gamepad calibration ABI drift");
#endif

#endif /* RIN_DRIVER_USB_HID_GAMEPAD_CALIBRATION_V1_H */
