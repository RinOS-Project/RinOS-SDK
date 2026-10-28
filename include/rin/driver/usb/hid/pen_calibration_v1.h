/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Bounded calibration for descriptor-admitted USB HID pen samples. */

#ifndef RIN_DRIVER_USB_HID_PEN_CALIBRATION_V1_H
#define RIN_DRIVER_USB_HID_PEN_CALIBRATION_V1_H

#include <stdint.h>

#include <rin/driver/usb/hid/pen_v1.h>

#define RIN_USB_HID_PEN_CALIBRATION_VERSION 1u

typedef struct RinUsbHidPenCalibrationV1 {
    uint32_t version;
    uint32_t struct_size;
    int32_t x_minimum;
    int32_t x_maximum;
    int32_t y_minimum;
    int32_t y_maximum;
    int32_t pressure_minimum;
    int32_t pressure_maximum;
    int32_t x_tilt_minimum;
    int32_t x_tilt_maximum;
    int32_t y_tilt_minimum;
    int32_t y_tilt_maximum;
    uint32_t output_width;
    uint32_t output_height;
    uint32_t flags;
    uint32_t reserved[4];
} RinUsbHidPenCalibrationV1;

RinUsbHidPenResult rin_usb_hid_pen_calibration_default(
    RinUsbHidPenCalibrationV1* calibration);

RinUsbHidPenResult rin_usb_hid_pen_calibration_validate(
    const RinUsbHidPenCalibrationV1* calibration);

/* Maps absolute X/Y to [0,width-1]/[0,height-1], pressure to 0..65535, and
 * clamps optional tilt fields. The sample remains generation/report-ID aware. */
RinUsbHidPenResult rin_usb_hid_pen_calibration_apply(
    const RinUsbHidPenCalibrationV1* calibration,
    const RinUsbHidPenSampleV1* sample,
    RinUsbHidPenSampleV1* calibrated_out);

#if defined(__cplusplus)
static_assert(sizeof(RinUsbHidPenCalibrationV1) == 76u,
              "USB HID pen calibration ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinUsbHidPenCalibrationV1) == 76u,
               "USB HID pen calibration ABI drift");
#endif

#endif /* RIN_DRIVER_USB_HID_PEN_CALIBRATION_V1_H */
