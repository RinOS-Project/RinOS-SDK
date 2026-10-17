/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RINOS_PUBLIC_BASE_RINOS_SDK_INCLUDE_RIN_DRIVER_USB_HID_BOOT_REPORT_H
#define RINOS_PUBLIC_BASE_RINOS_SDK_INCLUDE_RIN_DRIVER_USB_HID_BOOT_REPORT_H

#include <stdint.h>

#define RIN_USB_HID_BOOT_ABI_VERSION UINT32_C(0x00010000)

typedef enum RinUsbHidBootResult {
    RIN_USB_HID_BOOT_OK = 0,
    RIN_USB_HID_BOOT_INVALID = -1,
    RIN_USB_HID_BOOT_MALFORMED = -2,
    RIN_USB_HID_BOOT_ROLLOVER = -3
} RinUsbHidBootResult;

typedef enum RinUsbHidEventType {
    RIN_USB_HID_EVENT_KEY = 1,
    RIN_USB_HID_EVENT_RELATIVE = 2,
    RIN_USB_HID_EVENT_SYNC = 3
} RinUsbHidEventType;

#define RIN_USB_HID_REL_X     0u
#define RIN_USB_HID_REL_Y     1u
#define RIN_USB_HID_REL_WHEEL 8u

#define RIN_USB_HID_BTN_LEFT   0x110u
#define RIN_USB_HID_BTN_RIGHT  0x111u
#define RIN_USB_HID_BTN_MIDDLE 0x112u
#define RIN_USB_HID_BTN_SIDE   0x113u
#define RIN_USB_HID_BTN_EXTRA  0x114u

typedef struct RinUsbHidEvent {
    uint16_t type;
    uint16_t code;
    int32_t value;
} RinUsbHidEvent;

typedef void (*RinUsbHidEmitFn)(void* context,
                                const RinUsbHidEvent* event);

typedef struct RinUsbHidKeyboardState {
    uint8_t modifiers;
    uint8_t keys[6];
    uint8_t initialized;
} RinUsbHidKeyboardState;

typedef struct RinUsbHidMouseState {
    uint8_t buttons;
    uint8_t initialized;
} RinUsbHidMouseState;

void rin_usb_hid_keyboard_init(RinUsbHidKeyboardState* state);
void rin_usb_hid_mouse_init(RinUsbHidMouseState* state);

int rin_usb_hid_parse_keyboard(RinUsbHidKeyboardState* state,
                               const uint8_t* report, uint32_t report_size,
                               RinUsbHidEmitFn emit, void* emit_context);

int rin_usb_hid_parse_mouse(RinUsbHidMouseState* state,
                            const uint8_t* report, uint32_t report_size,
                            RinUsbHidEmitFn emit, void* emit_context);

#endif /* RINOS_PUBLIC_BASE_RINOS_SDK_INCLUDE_RIN_DRIVER_USB_HID_BOOT_REPORT_H */
