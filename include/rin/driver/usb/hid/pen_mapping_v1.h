/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Canonical canvas/tool mapping for calibrated USB HID pen samples. */

#ifndef RIN_DRIVER_USB_HID_PEN_MAPPING_V1_H
#define RIN_DRIVER_USB_HID_PEN_MAPPING_V1_H

#include <stdint.h>

#include <rin/driver/usb/hid/pen_v1.h>

#define RIN_USB_HID_PEN_MAPPING_VERSION 1u
#define RIN_USB_HID_PEN_MAPPING_SWAP_AXES UINT32_C(1)
#define RIN_USB_HID_PEN_MAPPING_INVERT_X UINT32_C(2)
#define RIN_USB_HID_PEN_MAPPING_INVERT_Y UINT32_C(4)

#define RIN_USB_HID_PEN_MAPPING_TIP UINT32_C(1)
#define RIN_USB_HID_PEN_MAPPING_BARREL UINT32_C(2)
#define RIN_USB_HID_PEN_MAPPING_ERASER UINT32_C(4)

typedef struct RinUsbHidPenMappingV1 {
    uint32_t version;
    uint32_t struct_size;
    uint32_t flags;
    uint32_t canvas_width;
    uint32_t canvas_height;
    uint32_t reserved[4];
} RinUsbHidPenMappingV1;

typedef struct RinUsbHidPenMappedSampleV1 {
    uint32_t version;
    uint32_t struct_size;
    uint32_t flags;
    uint32_t buttons;
    int32_t x;
    int32_t y;
    int32_t pressure;
    int32_t x_tilt;
    int32_t y_tilt;
    uint8_t report_id;
    uint8_t reserved0[3];
    uint32_t reserved[6];
} RinUsbHidPenMappedSampleV1;

RinUsbHidPenResult rin_usb_hid_pen_mapping_default(
    RinUsbHidPenMappingV1* mapping, uint32_t canvas_width,
    uint32_t canvas_height);

RinUsbHidPenResult rin_usb_hid_pen_mapping_validate(
    const RinUsbHidPenMappingV1* mapping);

RinUsbHidPenResult rin_usb_hid_pen_mapping_apply(
    const RinUsbHidPenMappingV1* mapping,
    const RinUsbHidPenSampleV1* sample,
    RinUsbHidPenMappedSampleV1* mapped_out);

#if defined(__cplusplus)
static_assert(sizeof(RinUsbHidPenMappingV1) == 36u,
              "USB HID pen mapping ABI drift");
static_assert(sizeof(RinUsbHidPenMappedSampleV1) == 64u,
              "USB HID mapped pen ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinUsbHidPenMappingV1) == 36u,
               "USB HID pen mapping ABI drift");
_Static_assert(sizeof(RinUsbHidPenMappedSampleV1) == 64u,
               "USB HID mapped pen ABI drift");
#endif

#endif /* RIN_DRIVER_USB_HID_PEN_MAPPING_V1_H */
