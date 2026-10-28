/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <rin/driver/usb/hid/pen_mapping_v1.h>

static void pen_map_zero(void* value, uint32_t size)
{
    volatile uint8_t* bytes = (volatile uint8_t*)value;
    if (!value) return;
    while (size-- != 0u) *bytes++ = 0u;
}

static int pen_map_all_zero(const void* value, uint32_t size)
{
    const volatile uint8_t* bytes = (const volatile uint8_t*)value;
    uint8_t combined = 0u;
    if (!value) return 0;
    while (size-- != 0u) combined |= *bytes++;
    return combined == 0u;
}

static int pen_map_valid(const RinUsbHidPenMappingV1* mapping)
{
    return mapping && mapping->version == RIN_USB_HID_PEN_MAPPING_VERSION &&
           mapping->struct_size == sizeof(*mapping) &&
           (mapping->flags & ~(RIN_USB_HID_PEN_MAPPING_SWAP_AXES |
                               RIN_USB_HID_PEN_MAPPING_INVERT_X |
                               RIN_USB_HID_PEN_MAPPING_INVERT_Y)) == 0u &&
           mapping->canvas_width >= 1u && mapping->canvas_width <= 65536u &&
           mapping->canvas_height >= 1u && mapping->canvas_height <= 65536u &&
           pen_map_all_zero(mapping->reserved, sizeof(mapping->reserved));
}

static int pen_map_sample_valid(const RinUsbHidPenSampleV1* sample)
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
           sample->reserved0[0] == 0u && sample->reserved0[1] == 0u &&
           sample->reserved0[2] == 0u &&
           pen_map_all_zero(sample->reserved, sizeof(sample->reserved));
}

static uint32_t pen_map_scale(uint32_t value, uint32_t input_maximum,
                              uint32_t output_maximum)
{
    return input_maximum == 0u ? 0u : (uint32_t)((uint64_t)value *
                                                  output_maximum /
                                                  input_maximum);
}

RinUsbHidPenResult rin_usb_hid_pen_mapping_default(
    RinUsbHidPenMappingV1* mapping, uint32_t canvas_width,
    uint32_t canvas_height)
{
    if (!mapping || canvas_width == 0u || canvas_width > 65536u ||
        canvas_height == 0u || canvas_height > 65536u)
        return RIN_USB_HID_PEN_INVALID_ARGUMENT;
    pen_map_zero(mapping, sizeof(*mapping));
    mapping->version = RIN_USB_HID_PEN_MAPPING_VERSION;
    mapping->struct_size = sizeof(*mapping);
    mapping->canvas_width = canvas_width;
    mapping->canvas_height = canvas_height;
    return RIN_USB_HID_PEN_OK;
}

RinUsbHidPenResult rin_usb_hid_pen_mapping_validate(
    const RinUsbHidPenMappingV1* mapping)
{
    return pen_map_valid(mapping) ? RIN_USB_HID_PEN_OK
                                  : RIN_USB_HID_PEN_INVALID_ARGUMENT;
}

RinUsbHidPenResult rin_usb_hid_pen_mapping_apply(
    const RinUsbHidPenMappingV1* mapping,
    const RinUsbHidPenSampleV1* sample,
    RinUsbHidPenMappedSampleV1* mapped_out)
{
    RinUsbHidPenMappedSampleV1 result;
    uint32_t x;
    uint32_t y;
    if (!mapped_out || !sample || (const void*)mapped_out ==
                                    (const void*)sample)
        return RIN_USB_HID_PEN_INVALID_ARGUMENT;
    pen_map_zero(mapped_out, sizeof(*mapped_out));
    if (!pen_map_valid(mapping) || !pen_map_sample_valid(sample))
        return RIN_USB_HID_PEN_INVALID_ARGUMENT;
    if (sample->x < 0 || sample->y < 0 ||
        (uint32_t)sample->x >= mapping->canvas_width ||
        (uint32_t)sample->y >= mapping->canvas_height)
        return RIN_USB_HID_PEN_MALFORMED;
    pen_map_zero(&result, sizeof(result));
    x = (uint32_t)sample->x;
    y = (uint32_t)sample->y;
    if ((mapping->flags & RIN_USB_HID_PEN_MAPPING_SWAP_AXES) != 0u) {
        uint32_t swapped_x = pen_map_scale(y, mapping->canvas_height - 1u,
                                            mapping->canvas_width - 1u);
        uint32_t swapped_y = pen_map_scale(x, mapping->canvas_width - 1u,
                                            mapping->canvas_height - 1u);
        x = swapped_x;
        y = swapped_y;
    }
    if ((mapping->flags & RIN_USB_HID_PEN_MAPPING_INVERT_X) != 0u)
        x = mapping->canvas_width - 1u - x;
    if ((mapping->flags & RIN_USB_HID_PEN_MAPPING_INVERT_Y) != 0u)
        y = mapping->canvas_height - 1u - y;
    result.version = RIN_USB_HID_PEN_MAPPING_VERSION;
    result.struct_size = sizeof(result);
    result.flags = sample->flags;
    result.x = (int32_t)x;
    result.y = (int32_t)y;
    result.pressure = sample->pressure;
    result.x_tilt = sample->x_tilt;
    result.y_tilt = sample->y_tilt;
    result.report_id = sample->report_id;
    if ((sample->flags & RIN_USB_HID_PEN_SAMPLE_TIP_SWITCH) != 0u)
        result.buttons |= RIN_USB_HID_PEN_MAPPING_TIP;
    if ((sample->flags & RIN_USB_HID_PEN_SAMPLE_BARREL_SWITCH) != 0u)
        result.buttons |= RIN_USB_HID_PEN_MAPPING_BARREL;
    if ((sample->flags & RIN_USB_HID_PEN_SAMPLE_ERASER) != 0u)
        result.buttons |= RIN_USB_HID_PEN_MAPPING_ERASER;
    *mapped_out = result;
    return RIN_USB_HID_PEN_OK;
}
