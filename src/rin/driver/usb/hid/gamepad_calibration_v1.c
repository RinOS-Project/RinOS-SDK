/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <rin/driver/usb/hid/gamepad_calibration_v1.h>

static void calibration_zero(void* value, uint32_t size)
{
    volatile uint8_t* bytes = (volatile uint8_t*)value;
    if (!value) return;
    while (size-- != 0u) *bytes++ = 0u;
}

static int calibration_all_zero(const void* value, uint32_t size)
{
    const volatile uint8_t* bytes = (const volatile uint8_t*)value;
    uint8_t combined = 0u;
    if (!value) return 0;
    while (size-- != 0u) combined |= *bytes++;
    return combined == 0u;
}

static int calibration_valid(const RinUsbHidGamepadCalibrationV1* calibration)
{
    uint32_t index;
    if (!calibration ||
        calibration->version != RIN_USB_HID_GAMEPAD_CALIBRATION_V1 ||
        calibration->struct_size != sizeof(*calibration) ||
        !calibration_all_zero(calibration->reserved,
                               sizeof(calibration->reserved))) return 0;
    for (index = 0u; index < RIN_USB_HID_GAMEPAD_MAX_AXES; ++index) {
        int32_t minimum = calibration->minimum[index];
        int32_t maximum = calibration->maximum[index];
        int32_t center = calibration->center[index];
        int32_t deadzone = calibration->deadzone[index];
        int32_t left_span;
        int32_t right_span;
        if (minimum >= maximum || center < minimum || center > maximum ||
            deadzone < 0) return 0;
        left_span = center - minimum;
        right_span = maximum - center;
        if ((left_span != 0 && deadzone >= left_span) ||
            (right_span != 0 && deadzone >= right_span)) return 0;
    }
    return 1;
}

static int sample_valid(const RinUsbHidGamepadSampleV1* sample)
{
    uint32_t index;
    if (!sample || sample->abi_version != RIN_USB_HID_GAMEPAD_ABI_VERSION ||
        sample->struct_size != sizeof(*sample) ||
        (sample->axis_mask &
         ~((UINT32_C(1) << RIN_USB_HID_GAMEPAD_MAX_AXES) - 1u)) != 0u ||
        (sample->flags & ~RIN_USB_HID_GAMEPAD_SAMPLE_HAS_HAT) != 0u ||
        ((sample->flags & RIN_USB_HID_GAMEPAD_SAMPLE_HAS_HAT) != 0u &&
         sample->hat > RIN_USB_HID_GAMEPAD_HAT_NEUTRAL) ||
        !calibration_all_zero(sample->reserved, sizeof(sample->reserved)))
        return 0;
    for (index = 0u; index < RIN_USB_HID_GAMEPAD_MAX_AXES; ++index) {
        if ((sample->axis_mask & (UINT32_C(1) << index)) == 0u &&
            sample->axes[index] != 0) return 0;
        /* INT16_MIN is reserved by the mapping owner for malformed input. */
        if (sample->axes[index] == INT16_MIN) return 0;
    }
    return 1;
}

static int16_t calibration_axis(int16_t input, int16_t minimum,
                                int16_t maximum, int16_t center,
                                int16_t deadzone)
{
    int32_t value = input;
    int32_t delta;
    int32_t span;
    int32_t magnitude;
    int64_t scaled;
    if (value < minimum) value = minimum;
    if (value > maximum) value = maximum;
    delta = value - center;
    magnitude = delta < 0 ? -delta : delta;
    if (magnitude <= deadzone) return 0;
    span = delta < 0 ? center - minimum : maximum - center;
    scaled = (int64_t)(magnitude - deadzone) * INT32_C(32767) /
             (span - deadzone);
    if (scaled > INT32_C(32767)) scaled = INT32_C(32767);
    return (int16_t)(delta < 0 ? -scaled : scaled);
}

RinUsbHidGamepadResult rin_usb_hid_gamepad_calibration_default(
    RinUsbHidGamepadCalibrationV1* calibration)
{
    uint32_t index;
    if (!calibration) return RIN_USB_HID_GAMEPAD_INVALID_ARGUMENT;
    calibration_zero(calibration, sizeof(*calibration));
    calibration->version = RIN_USB_HID_GAMEPAD_CALIBRATION_V1;
    calibration->struct_size = sizeof(*calibration);
    for (index = 0u; index < RIN_USB_HID_GAMEPAD_MAX_AXES; ++index) {
        calibration->minimum[index] = -32767;
        calibration->maximum[index] = 32767;
    }
    return RIN_USB_HID_GAMEPAD_OK;
}

RinUsbHidGamepadResult rin_usb_hid_gamepad_calibration_validate(
    const RinUsbHidGamepadCalibrationV1* calibration)
{
    return calibration_valid(calibration)
        ? RIN_USB_HID_GAMEPAD_OK
        : RIN_USB_HID_GAMEPAD_INVALID_ARGUMENT;
}

RinUsbHidGamepadResult rin_usb_hid_gamepad_calibration_apply(
    const RinUsbHidGamepadCalibrationV1* calibration,
    const RinUsbHidGamepadSampleV1* sample,
    RinUsbHidGamepadSampleV1* calibrated_out)
{
    RinUsbHidGamepadSampleV1 result;
    uint32_t index;
    if (!calibrated_out || (const void*)calibrated_out ==
                               (const void*)sample)
        return RIN_USB_HID_GAMEPAD_INVALID_ARGUMENT;
    calibration_zero(calibrated_out, sizeof(*calibrated_out));
    if (!calibration_valid(calibration) || !sample_valid(sample))
        return RIN_USB_HID_GAMEPAD_INVALID_ARGUMENT;
    result = *sample;
    for (index = 0u; index < RIN_USB_HID_GAMEPAD_MAX_AXES; ++index) {
        if ((sample->axis_mask & (UINT32_C(1) << index)) != 0u) {
            result.axes[index] = calibration_axis(
                sample->axes[index], calibration->minimum[index],
                calibration->maximum[index], calibration->center[index],
                calibration->deadzone[index]);
        }
    }
    *calibrated_out = result;
    return RIN_USB_HID_GAMEPAD_OK;
}
