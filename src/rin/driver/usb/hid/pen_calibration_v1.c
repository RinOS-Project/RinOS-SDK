/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <rin/driver/usb/hid/pen_calibration_v1.h>

static void pen_cal_zero(void* value, uint32_t size)
{
    volatile uint8_t* bytes = (volatile uint8_t*)value;
    if (!value) return;
    while (size-- != 0u) *bytes++ = 0u;
}

static int pen_cal_all_zero(const void* value, uint32_t size)
{
    const volatile uint8_t* bytes = (const volatile uint8_t*)value;
    uint8_t combined = 0u;
    if (!value) return 0;
    while (size-- != 0u) combined |= *bytes++;
    return combined == 0u;
}

static int32_t pen_cal_clamp(int32_t value, int32_t minimum, int32_t maximum)
{
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

static uint32_t pen_cal_scale(int32_t value, int32_t minimum,
                              int32_t maximum, uint32_t output_maximum)
{
    int64_t delta;
    int64_t span;
    uint64_t numerator;
    value = pen_cal_clamp(value, minimum, maximum);
    delta = (int64_t)value - (int64_t)minimum;
    span = (int64_t)maximum - (int64_t)minimum;
    numerator = (uint64_t)delta * output_maximum;
    return (uint32_t)(numerator / (uint64_t)span);
}

static int pen_cal_valid(const RinUsbHidPenCalibrationV1* calibration)
{
    return calibration && calibration->version ==
               RIN_USB_HID_PEN_CALIBRATION_VERSION &&
           calibration->struct_size == sizeof(*calibration) &&
           calibration->x_minimum < calibration->x_maximum &&
           calibration->y_minimum < calibration->y_maximum &&
           calibration->pressure_minimum < calibration->pressure_maximum &&
           calibration->x_tilt_minimum < calibration->x_tilt_maximum &&
           calibration->y_tilt_minimum < calibration->y_tilt_maximum &&
           calibration->output_width >= 1u && calibration->output_width <= 65536u &&
           calibration->output_height >= 1u && calibration->output_height <= 65536u &&
           calibration->flags == 0u &&
           pen_cal_all_zero(calibration->reserved, sizeof(calibration->reserved));
}

static int pen_cal_sample_valid(const RinUsbHidPenSampleV1* sample)
{
    return sample && sample->abi_version == RIN_USB_HID_PEN_ABI_VERSION &&
           sample->struct_size == sizeof(*sample) &&
           (sample->flags & ~(RIN_USB_HID_PEN_SAMPLE_TIP_SWITCH |
                              RIN_USB_HID_PEN_SAMPLE_IN_RANGE |
                              RIN_USB_HID_PEN_SAMPLE_BARREL_SWITCH |
                              RIN_USB_HID_PEN_SAMPLE_ERASER |
                              RIN_USB_HID_PEN_SAMPLE_PRESSURE_VALID |
                              RIN_USB_HID_PEN_SAMPLE_X_TILT_VALID |
                              RIN_USB_HID_PEN_SAMPLE_Y_TILT_VALID)) == 0u &&
           pen_cal_all_zero(sample->reserved, sizeof(sample->reserved)) &&
           sample->reserved0[0] == 0u && sample->reserved0[1] == 0u &&
           sample->reserved0[2] == 0u;
}

RinUsbHidPenResult rin_usb_hid_pen_calibration_default(
    RinUsbHidPenCalibrationV1* calibration)
{
    if (!calibration) return RIN_USB_HID_PEN_INVALID_ARGUMENT;
    pen_cal_zero(calibration, sizeof(*calibration));
    calibration->version = RIN_USB_HID_PEN_CALIBRATION_VERSION;
    calibration->struct_size = sizeof(*calibration);
    calibration->x_minimum = 0;
    calibration->x_maximum = 32767;
    calibration->y_minimum = 0;
    calibration->y_maximum = 32767;
    calibration->pressure_minimum = 0;
    calibration->pressure_maximum = 65535;
    calibration->x_tilt_minimum = -90;
    calibration->x_tilt_maximum = 90;
    calibration->y_tilt_minimum = -90;
    calibration->y_tilt_maximum = 90;
    calibration->output_width = 65536u;
    calibration->output_height = 65536u;
    return RIN_USB_HID_PEN_OK;
}

RinUsbHidPenResult rin_usb_hid_pen_calibration_validate(
    const RinUsbHidPenCalibrationV1* calibration)
{
    return pen_cal_valid(calibration) ? RIN_USB_HID_PEN_OK
                                      : RIN_USB_HID_PEN_INVALID_ARGUMENT;
}

RinUsbHidPenResult rin_usb_hid_pen_calibration_apply(
    const RinUsbHidPenCalibrationV1* calibration,
    const RinUsbHidPenSampleV1* sample,
    RinUsbHidPenSampleV1* calibrated_out)
{
    RinUsbHidPenSampleV1 result;
    if (!calibrated_out || !sample || (const void*)calibrated_out ==
                                      (const void*)sample)
        return RIN_USB_HID_PEN_INVALID_ARGUMENT;
    pen_cal_zero(calibrated_out, sizeof(*calibrated_out));
    if (!pen_cal_valid(calibration) || !pen_cal_sample_valid(sample))
        return RIN_USB_HID_PEN_INVALID_ARGUMENT;
    result = *sample;
    result.x = (int32_t)pen_cal_scale(sample->x, calibration->x_minimum,
                                      calibration->x_maximum,
                                      calibration->output_width - 1u);
    result.y = (int32_t)pen_cal_scale(sample->y, calibration->y_minimum,
                                      calibration->y_maximum,
                                      calibration->output_height - 1u);
    if ((sample->flags & RIN_USB_HID_PEN_SAMPLE_PRESSURE_VALID) != 0u)
        result.pressure = (int32_t)pen_cal_scale(
            sample->pressure, calibration->pressure_minimum,
            calibration->pressure_maximum, UINT32_C(65535));
    else
        result.pressure = 0;
    if ((sample->flags & RIN_USB_HID_PEN_SAMPLE_X_TILT_VALID) != 0u)
        result.x_tilt = pen_cal_clamp(sample->x_tilt,
                                      calibration->x_tilt_minimum,
                                      calibration->x_tilt_maximum);
    else
        result.x_tilt = 0;
    if ((sample->flags & RIN_USB_HID_PEN_SAMPLE_Y_TILT_VALID) != 0u)
        result.y_tilt = pen_cal_clamp(sample->y_tilt,
                                      calibration->y_tilt_minimum,
                                      calibration->y_tilt_maximum);
    else
        result.y_tilt = 0;
    *calibrated_out = result;
    return RIN_USB_HID_PEN_OK;
}
