/* SPDX-License-Identifier: GPL-2.0-or-later */
/* NDRV v3 interrupt-IN owner for bounded USB HID profiles. */

#ifndef RIN_DRIVER_USB_HID_DEVICE_V3_H
#define RIN_DRIVER_USB_HID_DEVICE_V3_H

#include <stdint.h>

#include <rin/driver/module_api_v3.h>
#include <rin/driver/usb/hid/gamepad_v1.h>
#include <rin/driver/usb/hid/pen_v1.h"
#include <rin/driver/usb/hid/boot_report.h"
#include <rin/contract_abi.h>
#define RIN_DRIVER_USB_HID_V3_VERSION UINT32_C(0x00010002)
#define RIN_DRIVER_USB_HID_V3_MAGIC UINT32_C(0x33444948)
#define RIN_DRIVER_USB_HID_V3_MAX_REPORT_SIZE UINT32_C(64)
#define RIN_DRIVER_USB_HID_V3_MAX_BOOT_EVENTS UINT32_C(24)
#define RIN_DRIVER_USB_HID_V3_DEFAULT_INTERVAL_MS UINT32_C(8)

#define RIN_DRIVER_USB_HID_V3_BOOT_KEYBOARD UINT32_C(1)
#define RIN_DRIVER_USB_HID_V3_BOOT_MOUSE UINT32_C(2)
#define RIN_DRIVER_USB_HID_V3_GAMEPAD UINT32_C(3)
#define RIN_DRIVER_USB_HID_V3_PEN UINT32_C(4)

typedef struct RinDriverUsbHidMemoryV3 {
    uint32_t struct_size;
    uint32_t version;
    int (*publish)(void* context, const void* address, uint64_t length,
                   uint32_t dma_flags, uint64_t* memory_lease);
    int (*unpublish)(void* context, uint64_t memory_lease);
    uint64_t reserved[4];
} RinDriverUsbHidMemoryV3;

typedef struct RinDriverUsbHidPublicationV3 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t kind;
    uint32_t event_count;
    uint64_t device_id;
    uint32_t port_generation;
    uint32_t report_sequence;
    RinUsbHidEvent boot_events[RIN_DRIVER_USB_HID_V3_MAX_BOOT_EVENTS];
    RinUsbHidGamepadSampleV1 gamepad;
    RinUsbHidPenSampleV1 pen;
    uint64_t reserved[4];
} RinDriverUsbHidPublicationV3;

#if defined(__cplusplus)
static_assert(sizeof(RinDriverUsbHidPublicationV3) == 384u,
              "USB HID publication ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinDriverUsbHidPublicationV3) == 384u,
               "USB HID publication ABI drift");
#endif

typedef RinResultCode (*RinDriverUsbHidPublishV3)(
    void* context, const RinDriverUsbHidPublicationV3* publication);

typedef struct RinDriverUsbHidConfigV3 {
    uint32_t struct_size;
    uint32_t version;
    const RinDriverV3DeviceRegistration* registration;
    const RinDriverUsbHidMemoryV3* memory;
    void* memory_context;
    uint64_t (*monotonic_ns)(void* context);
    RinDriverUsbHidPublishV3 publish;
    void* callback_context;
    uint64_t reserved[4];
} RinDriverUsbHidConfigV3;

typedef struct RinDriverUsbHidAttachmentV3 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t kind;
    uint32_t port_index;
    uint32_t port_generation;
    uint32_t device_address;
    uint32_t interface_number;
    uint32_t interrupt_in_endpoint;
    uint32_t interrupt_in_packet_size;
    uint32_t interrupt_in_interval;
    uint32_t interrupt_out_endpoint;
    uint32_t interrupt_out_packet_size;
    uint32_t report_descriptor_size;
    uint64_t device_id;
    const uint8_t* report_descriptor;
    uint64_t reserved[2];
} RinDriverUsbHidAttachmentV3;

typedef struct RinDriverUsbHidV3 {
    uint32_t magic;
    volatile uint32_t lock;
    uint32_t callback_active;
    uint32_t attached;
    uint32_t quarantined;
    uint32_t kind;
    RinDriverV3UsbControllerOps operations;
    uint64_t driver_context;
    RinDriverUsbHidMemoryV3 memory;
    void* memory_context;
    uint64_t (*monotonic_ns)(void* context);
    RinDriverUsbHidPublishV3 publish;
    void* callback_context;
    uint64_t device_id;
    uint64_t next_transfer_id;
    uint64_t memory_lease;
    uint64_t output_memory_lease;
    uint32_t port_index;
    uint32_t port_generation;
    uint32_t device_address;
    uint32_t interface_number;
    uint32_t interrupt_in_endpoint;
    uint32_t interrupt_in_packet_size;
    uint32_t interrupt_in_interval;
    uint32_t interrupt_out_endpoint;
    uint32_t interrupt_out_packet_size;
    uint32_t report_sequence;
    uint32_t transfer_active;
    uint32_t output_transfer_active;
    RinDriverV3UsbTransfer request;
    RinDriverV3UsbTransfer output_request;
    uint8_t report[RIN_DRIVER_USB_HID_V3_MAX_REPORT_SIZE];
    uint8_t output_report[RIN_DRIVER_USB_HID_V3_MAX_REPORT_SIZE];
    RinUsbHidKeyboardState keyboard;
    RinUsbHidMouseState mouse;
    RinUsbHidGamepadProfileV1 gamepad_profile;
    RinUsbHidGamepadOutputProfileV1 gamepad_output_profile;
    RinUsbHidPenProfileV1 pen_profile;
} RinDriverUsbHidV3;

RinResultCode rin_driver_usb_hid_v3_init(
    RinDriverUsbHidV3* adapter,
    const RinDriverUsbHidConfigV3* config);
RinResultCode rin_driver_usb_hid_v3_attach(
    RinDriverUsbHidV3* adapter,
    const RinDriverUsbHidAttachmentV3* attachment);
RinResultCode rin_driver_usb_hid_v3_poll(
    RinDriverUsbHidV3* adapter, uint32_t* published);
/* Queues exactly one descriptor-admitted generic Gamepad Output report. The
 * caller must keep polling to reap its DMA lease before another is accepted. */
RinResultCode rin_driver_usb_hid_v3_submit_gamepad_output(
    RinDriverUsbHidV3* adapter, uint64_t device_id,
    uint32_t port_generation, const uint8_t* report,
    uint32_t report_size);
RinResultCode rin_driver_usb_hid_v3_detach(
    RinDriverUsbHidV3* adapter, uint64_t deadline_ns);

#endif /* RIN_DRIVER_USB_HID_DEVICE_V3_H */
