/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Portable CTAP-HID broker runtime for USB FIDO drivers. */

#ifndef RIN_DRIVER_USB_FIDO_BROKER_V1_H
#define RIN_DRIVER_USB_FIDO_BROKER_V1_H

#include <stdint.h>

#include "rin/driver/usb/fido/ctaphid.h"

#define RIN_USB_FIDO_BROKER_VERSION UINT32_C(0x00010000)
#define RIN_USB_FIDO_DEVICE_MAX UINT32_C(8)
#define RIN_USB_FIDO_REPORT_DESCRIPTOR_MAX UINT32_C(512)
#define RIN_USB_FIDO_USAGE_PAGE UINT32_C(0xf1d0)
#define RIN_USB_FIDO_USAGE_CTAPHID UINT32_C(0x01)
#define RIN_USB_FIDO_USAGE_INPUT UINT32_C(0x20)
#define RIN_USB_FIDO_USAGE_OUTPUT UINT32_C(0x21)

typedef struct RinUsbFidoReportLayout {
    uint32_t input_report_size;
    uint32_t output_report_size;
    uint32_t usage_page;
    uint32_t usage;
    uint32_t reserved[4];
} RinUsbFidoReportLayout;

typedef RinResultCode (*RinUsbFidoStartInputFn)(
    void* context, uint64_t transport_cookie, uint32_t port_generation);
/* start_input and submit_output queue work only; they must not re-enter the
 * broker synchronously. A failed start_input leaves no pending transfer. */
typedef RinResultCode (*RinUsbFidoSubmitOutputFn)(
    void* context, uint64_t transport_cookie, uint32_t port_generation,
    const uint8_t report[RIN_CTAPHID_REPORT_SIZE], uint64_t deadline_ms);
/* Returning OK guarantees that no older transfer can access broker buffers or
 * publish another completion for this port generation. */
typedef RinResultCode (*RinUsbFidoCancelTransfersFn)(
    void* context, uint64_t transport_cookie, uint32_t port_generation);

typedef struct RinUsbFidoTransportV1 {
    uint32_t struct_size;
    uint32_t version;
    RinUsbFidoStartInputFn start_input;
    RinUsbFidoSubmitOutputFn submit_output;
    RinUsbFidoCancelTransfersFn cancel_transfers;
    uint64_t reserved[4];
} RinUsbFidoTransportV1;

struct RinUsbFidoBroker;

typedef struct RinUsbFidoDevice {
    RinCtapHidClient client;
    struct RinUsbFidoBroker* broker;
    uint64_t device_id;
    uint64_t transport_cookie;
    uint64_t emit_deadline_ms;
    uint32_t port_generation;
    uint32_t active;
    uint32_t transport_started;
    uint32_t quarantined;
} RinUsbFidoDevice;

typedef struct RinUsbFidoBroker {
    RinUsbFidoDevice devices[RIN_USB_FIDO_DEVICE_MAX];
    RinUsbFidoTransportV1 transport;
    void* transport_context;
    uint64_t next_device_id;
    uint32_t initialized;
    uint32_t lock;
} RinUsbFidoBroker;

typedef struct RinUsbFidoDeviceSnapshot {
    uint64_t device_id;
    uint64_t transport_cookie;
    uint32_t port_generation;
    uint32_t client_state;
    uint32_t capabilities;
    uint32_t ready;
} RinUsbFidoDeviceSnapshot;

RinResultCode rin_usb_fido_parse_report_descriptor(
    const uint8_t* descriptor, uint32_t descriptor_size,
    RinUsbFidoReportLayout* layout);
RinResultCode rin_usb_fido_broker_init(
    RinUsbFidoBroker* broker, const RinUsbFidoTransportV1* transport,
    void* transport_context);
RinResultCode rin_usb_fido_attach(
    RinUsbFidoBroker* broker, uint64_t transport_cookie,
    uint32_t port_generation, uint32_t interrupt_in_packet_size,
    uint32_t interrupt_out_packet_size, const uint8_t* report_descriptor,
    uint32_t report_descriptor_size, uint64_t* device_id);
RinResultCode rin_usb_fido_detach(RinUsbFidoBroker* broker,
                                  uint64_t device_id,
                                  uint32_t port_generation);
RinResultCode rin_usb_fido_begin_init(
    RinUsbFidoBroker* broker, uint64_t device_id,
    uint32_t port_generation, const uint8_t nonce[8], uint64_t now_ms,
    uint32_t timeout_ms);
RinResultCode rin_usb_fido_begin_cbor(
    RinUsbFidoBroker* broker, uint64_t device_id,
    uint32_t port_generation, const uint8_t* payload, uint32_t payload_size,
    uint64_t now_ms, uint32_t timeout_ms);
RinResultCode rin_usb_fido_cancel(RinUsbFidoBroker* broker,
                                  uint64_t device_id,
                                  uint32_t port_generation);
RinResultCode rin_usb_fido_feed_report(
    RinUsbFidoBroker* broker, uint64_t device_id,
    uint32_t port_generation,
    const uint8_t report[RIN_CTAPHID_REPORT_SIZE], uint64_t now_ms,
    RinCtapHidMessage* message, uint32_t* event);
RinResultCode rin_usb_fido_poll(RinUsbFidoBroker* broker,
                                uint64_t device_id,
                                uint32_t port_generation,
                                uint64_t now_ms);
RinResultCode rin_usb_fido_first_ready(
    RinUsbFidoBroker* broker, RinUsbFidoDeviceSnapshot* snapshot);

#endif /* RIN_DRIVER_USB_FIDO_BROKER_V1_H */
