/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <rin/driver/usb/hid/gamepad_mapping_v1.h>

static void mapping_zero(void* value, uint32_t size)
{
    volatile uint8_t* bytes = (volatile uint8_t*)value;
    while (size-- != 0u) *bytes++ = 0u;
}

static int mapping_all_zero(const void* value, uint32_t size)
{
    const volatile uint8_t* bytes = (const volatile uint8_t*)value;
    uint8_t combined = 0u;
    while (size-- != 0u) combined |= *bytes++;
    return combined == 0u;
}

static int mapping_axis_valid(uint8_t axis)
{
    return axis == RIN_USB_HID_GAMEPAD_MAPPING_AXIS_ABSENT ||
           axis < RIN_USB_HID_GAMEPAD_MAX_AXES;
}

static int mapping_valid(const RinUsbHidGamepadMappingV1* mapping)
{
    uint16_t known_buttons =
        RIN_USB_HID_GAMEPAD_MAPPING_DPAD_UP |
        RIN_USB_HID_GAMEPAD_MAPPING_DPAD_DOWN |
        RIN_USB_HID_GAMEPAD_MAPPING_DPAD_LEFT |
        RIN_USB_HID_GAMEPAD_MAPPING_DPAD_RIGHT |
        RIN_USB_HID_GAMEPAD_MAPPING_START |
        RIN_USB_HID_GAMEPAD_MAPPING_BACK |
        RIN_USB_HID_GAMEPAD_MAPPING_LEFT_THUMB |
        RIN_USB_HID_GAMEPAD_MAPPING_RIGHT_THUMB |
        RIN_USB_HID_GAMEPAD_MAPPING_LEFT_SHOULDER |
        RIN_USB_HID_GAMEPAD_MAPPING_RIGHT_SHOULDER |
        RIN_USB_HID_GAMEPAD_MAPPING_A |
        RIN_USB_HID_GAMEPAD_MAPPING_B |
        RIN_USB_HID_GAMEPAD_MAPPING_X |
        RIN_USB_HID_GAMEPAD_MAPPING_Y;
    if (!mapping || mapping->version != RIN_USB_HID_GAMEPAD_MAPPING_V1 ||
        mapping->struct_size != sizeof(*mapping) ||
        !mapping_axis_valid(mapping->thumb_lx_axis) ||
        !mapping_axis_valid(mapping->thumb_ly_axis) ||
        !mapping_axis_valid(mapping->thumb_rx_axis) ||
        !mapping_axis_valid(mapping->thumb_ry_axis) ||
        !mapping_axis_valid(mapping->left_trigger_axis) ||
        !mapping_axis_valid(mapping->right_trigger_axis) ||
        (mapping->invert_axis_mask & ~UINT8_C(0x3f)) != 0u ||
        mapping->reserved0 != 0u ||
        !mapping_all_zero(mapping->reserved, sizeof(mapping->reserved)))
        return 0;
    for (uint32_t index = 0u;
         index < RIN_USB_HID_GAMEPAD_MAPPING_MAX_BUTTONS; ++index) {
        if ((mapping->button_masks[index] & ~known_buttons) != 0u)
            return 0;
    }
    return 1;
}

RinResultCode rin_usb_hid_gamepad_mapping_default(
    RinUsbHidGamepadMappingV1* mapping)
{
    static const uint16_t button_masks[10] = {
        RIN_USB_HID_GAMEPAD_MAPPING_A,
        RIN_USB_HID_GAMEPAD_MAPPING_B,
        RIN_USB_HID_GAMEPAD_MAPPING_X,
        RIN_USB_HID_GAMEPAD_MAPPING_Y,
        RIN_USB_HID_GAMEPAD_MAPPING_LEFT_SHOULDER,
        RIN_USB_HID_GAMEPAD_MAPPING_RIGHT_SHOULDER,
        RIN_USB_HID_GAMEPAD_MAPPING_BACK,
        RIN_USB_HID_GAMEPAD_MAPPING_START,
        RIN_USB_HID_GAMEPAD_MAPPING_LEFT_THUMB,
        RIN_USB_HID_GAMEPAD_MAPPING_RIGHT_THUMB
    };
    if (!mapping) return RIN_RESULT_INVALID_ARGUMENT;
    mapping_zero(mapping, sizeof(*mapping));
    mapping->version = RIN_USB_HID_GAMEPAD_MAPPING_V1;
    mapping->struct_size = sizeof(*mapping);
    for (uint32_t index = 0u; index < 10u; ++index)
        mapping->button_masks[index] = button_masks[index];
    mapping->thumb_lx_axis = RIN_USB_HID_GAMEPAD_AXIS_X;
    mapping->thumb_ly_axis = RIN_USB_HID_GAMEPAD_AXIS_Y;
    mapping->thumb_rx_axis = RIN_USB_HID_GAMEPAD_AXIS_RX;
    mapping->thumb_ry_axis = RIN_USB_HID_GAMEPAD_AXIS_RY;
    mapping->left_trigger_axis = RIN_USB_HID_GAMEPAD_AXIS_Z;
    mapping->right_trigger_axis = RIN_USB_HID_GAMEPAD_AXIS_RZ;
    mapping->invert_axis_mask = UINT8_C(0x0a);
    return RIN_RESULT_OK;
}

RinResultCode rin_usb_hid_gamepad_mapping_validate(
    const RinUsbHidGamepadMappingV1* mapping)
{
    return mapping_valid(mapping) ? RIN_RESULT_OK
                                   : RIN_RESULT_INVALID_ARGUMENT;
}

static int16_t mapped_axis(const RinUsbHidGamepadSampleV1* sample,
                           uint8_t axis, uint8_t invert)
{
    int16_t value;
    if (axis == RIN_USB_HID_GAMEPAD_MAPPING_AXIS_ABSENT ||
        (sample->axis_mask & (UINT32_C(1) << axis)) == 0u)
        return 0;
    value = sample->axes[axis];
    return invert ? (int16_t)-value : value;
}

static uint8_t mapped_trigger(const RinUsbHidGamepadSampleV1* sample,
                              uint8_t axis)
{
    int32_t shifted;
    if (axis == RIN_USB_HID_GAMEPAD_MAPPING_AXIS_ABSENT ||
        (sample->axis_mask & (UINT32_C(1) << axis)) == 0u)
        return 0u;
    shifted = (int32_t)sample->axes[axis] + 32767;
    if (shifted < 0) shifted = 0;
    if (shifted > 65534) shifted = 65534;
    return (uint8_t)(((uint32_t)shifted * UINT32_C(255)) / UINT32_C(65534));
}

RinResultCode rin_usb_hid_gamepad_mapping_apply(
    const RinUsbHidGamepadMappingV1* mapping,
    const RinUsbHidGamepadSampleV1* sample,
    uint32_t user_index, uint64_t device_id, uint64_t device_generation,
    uint64_t sample_sequence, RinGamepadStateV1* state_out)
{
    if (state_out) mapping_zero(state_out, sizeof(*state_out));
    if (!mapping_valid(mapping) || !sample || !state_out ||
        sample->abi_version != RIN_USB_HID_GAMEPAD_ABI_VERSION ||
        sample->struct_size != sizeof(*sample) ||
        (sample->axis_mask &
         ~((UINT32_C(1) << RIN_USB_HID_GAMEPAD_MAX_AXES) - 1u)) != 0u ||
        (sample->flags & ~RIN_USB_HID_GAMEPAD_SAMPLE_HAS_HAT) != 0u ||
        (((sample->flags & RIN_USB_HID_GAMEPAD_SAMPLE_HAS_HAT) != 0u) &&
         sample->hat > RIN_USB_HID_GAMEPAD_HAT_NEUTRAL) ||
        !mapping_all_zero(sample->reserved, sizeof(sample->reserved)) ||
        device_id == 0u || device_generation == 0u || sample_sequence == 0u)
        return RIN_RESULT_INVALID_ARGUMENT;
    for (uint32_t index = 0u;
         index < RIN_USB_HID_GAMEPAD_MAX_AXES; ++index) {
        if ((sample->axis_mask & (UINT32_C(1) << index)) == 0u &&
            sample->axes[index] != 0)
            return RIN_RESULT_CORRUPT_DATA;
        if (sample->axes[index] == INT16_MIN)
            return RIN_RESULT_CORRUPT_DATA;
    }
    state_out->struct_size = sizeof(*state_out);
    state_out->version = RIN_GAMEPAD_STATE_VERSION;
    state_out->user_index = user_index;
    state_out->flags = RIN_GAMEPAD_STATE_FLAG_CONNECTED;
    state_out->device_id = device_id;
    state_out->device_generation = device_generation;
    state_out->sample_sequence = sample_sequence;
    for (uint32_t index = 0u; index <
         RIN_USB_HID_GAMEPAD_MAPPING_MAX_BUTTONS; ++index) {
        if ((sample->buttons & (UINT32_C(1) << index)) != 0u)
            state_out->buttons |= mapping->button_masks[index];
    }
    if ((sample->flags & RIN_USB_HID_GAMEPAD_SAMPLE_HAS_HAT) != 0u) {
        switch (sample->hat) {
            case 0u: state_out->buttons |= RIN_USB_HID_GAMEPAD_MAPPING_DPAD_UP; break;
            case 1u: state_out->buttons |= RIN_USB_HID_GAMEPAD_MAPPING_DPAD_UP | RIN_USB_HID_GAMEPAD_MAPPING_DPAD_RIGHT; break;
            case 2u: state_out->buttons |= RIN_USB_HID_GAMEPAD_MAPPING_DPAD_RIGHT; break;
            case 3u: state_out->buttons |= RIN_USB_HID_GAMEPAD_MAPPING_DPAD_RIGHT | RIN_USB_HID_GAMEPAD_MAPPING_DPAD_DOWN; break;
            case 4u: state_out->buttons |= RIN_USB_HID_GAMEPAD_MAPPING_DPAD_DOWN; break;
            case 5u: state_out->buttons |= RIN_USB_HID_GAMEPAD_MAPPING_DPAD_DOWN | RIN_USB_HID_GAMEPAD_MAPPING_DPAD_LEFT; break;
            case 6u: state_out->buttons |= RIN_USB_HID_GAMEPAD_MAPPING_DPAD_LEFT; break;
            case 7u: state_out->buttons |= RIN_USB_HID_GAMEPAD_MAPPING_DPAD_LEFT | RIN_USB_HID_GAMEPAD_MAPPING_DPAD_UP; break;
            default: break;
        }
    }
    state_out->thumb_lx = mapped_axis(
        sample, mapping->thumb_lx_axis,
        (mapping->invert_axis_mask & UINT8_C(0x01)) != 0u);
    state_out->thumb_ly = mapped_axis(
        sample, mapping->thumb_ly_axis,
        (mapping->invert_axis_mask & UINT8_C(0x02)) != 0u);
    state_out->thumb_rx = mapped_axis(
        sample, mapping->thumb_rx_axis,
        (mapping->invert_axis_mask & UINT8_C(0x04)) != 0u);
    state_out->thumb_ry = mapped_axis(
        sample, mapping->thumb_ry_axis,
        (mapping->invert_axis_mask & UINT8_C(0x08)) != 0u);
    state_out->left_trigger = mapped_trigger(sample,
                                              mapping->left_trigger_axis);
    state_out->right_trigger = mapped_trigger(sample,
                                               mapping->right_trigger_axis);
    return RIN_RESULT_OK;
}
