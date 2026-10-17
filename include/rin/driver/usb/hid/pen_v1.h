/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RINOS_PUBLIC_BASE_RINOS_SDK_INCLUDE_RIN_DRIVER_USB_HID_PEN_V1_H
#define RINOS_PUBLIC_BASE_RINOS_SDK_INCLUDE_RIN_DRIVER_USB_HID_PEN_V1_H

#include <stdint.h>

#define RIN_USB_HID_PEN_ABI_VERSION 1u
#define RIN_USB_HID_PEN_MAX_DESCRIPTOR_BYTES 512u
#define RIN_USB_HID_PEN_MAX_REPORT_BYTES 64u

#define RIN_USB_HID_PEN_PROFILE_HAS_REPORT_ID (1u << 0)
#define RIN_USB_HID_PEN_PROFILE_HAS_PRESSURE (1u << 1)
#define RIN_USB_HID_PEN_PROFILE_HAS_X_TILT (1u << 2)
#define RIN_USB_HID_PEN_PROFILE_HAS_Y_TILT (1u << 3)
#define RIN_USB_HID_PEN_PROFILE_HAS_BARREL_SWITCH (1u << 4)
#define RIN_USB_HID_PEN_PROFILE_HAS_ERASER (1u << 5)

#define RIN_USB_HID_PEN_CONTROL_SIGNED (1u << 0)

#define RIN_USB_HID_PEN_SAMPLE_TIP_SWITCH (1u << 0)
#define RIN_USB_HID_PEN_SAMPLE_IN_RANGE (1u << 1)
#define RIN_USB_HID_PEN_SAMPLE_BARREL_SWITCH (1u << 2)
#define RIN_USB_HID_PEN_SAMPLE_ERASER (1u << 3)
#define RIN_USB_HID_PEN_SAMPLE_PRESSURE_VALID (1u << 4)
#define RIN_USB_HID_PEN_SAMPLE_X_TILT_VALID (1u << 5)
#define RIN_USB_HID_PEN_SAMPLE_Y_TILT_VALID (1u << 6)

typedef enum RinUsbHidPenResult {
    RIN_USB_HID_PEN_OK = 0,
    RIN_USB_HID_PEN_INVALID_ARGUMENT = -1,
    RIN_USB_HID_PEN_MALFORMED = -2,
    RIN_USB_HID_PEN_UNSUPPORTED = -3,
    RIN_USB_HID_PEN_LIMIT_EXCEEDED = -4
} RinUsbHidPenResult;

typedef struct RinUsbHidPenControlV1 {
    uint16_t bit_offset;
    uint8_t bit_size;
    uint8_t flags;
    int32_t logical_minimum;
    int32_t logical_maximum;
} RinUsbHidPenControlV1;

typedef struct RinUsbHidPenProfileV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t descriptor_size;
    uint32_t report_size;
    uint32_t report_bit_count;
    uint32_t flags;
    uint8_t report_id;
    uint8_t reserved0[3];
    RinUsbHidPenControlV1 x;
    RinUsbHidPenControlV1 y;
    RinUsbHidPenControlV1 pressure;
    RinUsbHidPenControlV1 x_tilt;
    RinUsbHidPenControlV1 y_tilt;
    uint16_t tip_switch_bit;
    uint16_t in_range_bit;
    uint16_t barrel_switch_bit;
    uint16_t eraser_bit;
    uint32_t reserved[8];
} RinUsbHidPenProfileV1;

typedef struct RinUsbHidPenSampleV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t flags;
    int32_t x;
    int32_t y;
    int32_t pressure;
    int32_t x_tilt;
    int32_t y_tilt;
    uint8_t report_id;
    uint8_t reserved0[3];
    uint32_t reserved[7];
} RinUsbHidPenSampleV1;

RinUsbHidPenResult rin_usb_hid_pen_admit_descriptor(
    const uint8_t* descriptor, uint32_t descriptor_size,
    RinUsbHidPenProfileV1* profile);

RinUsbHidPenResult rin_usb_hid_pen_decode_report(
    const RinUsbHidPenProfileV1* profile,
    const uint8_t* report, uint32_t report_size,
    RinUsbHidPenSampleV1* sample);

#if defined(__cplusplus)
static_assert(sizeof(RinUsbHidPenControlV1) == 12u,
              "USB HID pen control ABI drift");
static_assert(sizeof(RinUsbHidPenProfileV1) == 128u,
              "USB HID pen profile ABI drift");
static_assert(sizeof(RinUsbHidPenSampleV1) == 64u,
              "USB HID pen sample ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinUsbHidPenControlV1) == 12u,
               "USB HID pen control ABI drift");
_Static_assert(sizeof(RinUsbHidPenProfileV1) == 128u,
               "USB HID pen profile ABI drift");
_Static_assert(sizeof(RinUsbHidPenSampleV1) == 64u,
               "USB HID pen sample ABI drift");
#endif

#endif /* RINOS_PUBLIC_BASE_RINOS_SDK_INCLUDE_RIN_DRIVER_USB_HID_PEN_V1_H */
