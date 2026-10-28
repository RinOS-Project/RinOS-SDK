/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <rin/driver/usb/fido/broker_v1.h>

#include <stddef.h>

typedef struct HidGlobalState {
    uint32_t usage_page;
    uint32_t report_size;
    uint32_t report_count;
} HidGlobalState;

static void fido_zero(void* value, uint32_t size)
{
    volatile uint8_t* bytes = (volatile uint8_t*)value;
    if (!value) return;
    while (size-- != 0u) *bytes++ = 0u;
}

static void fido_copy(void* output_value, const void* input_value,
                      uint32_t size)
{
    uint8_t* output = (uint8_t*)output_value;
    const uint8_t* input = (const uint8_t*)input_value;
    while (size-- != 0u) *output++ = *input++;
}

static uint32_t item_value(const uint8_t* data, uint32_t size)
{
    uint32_t value = 0u;
    uint32_t index;
    for (index = 0u; index < size; ++index)
        value |= (uint32_t)data[index] << (index * 8u);
    return value;
}

static void local_clear(uint32_t* usage, uint32_t* has_usage)
{
    *usage = 0u;
    *has_usage = 0u;
}

static void effective_usage(uint32_t global_page, uint32_t raw_usage,
                            uint32_t* page, uint32_t* usage)
{
    if ((raw_usage >> 16u) != 0u) {
        *page = raw_usage >> 16u;
        *usage = raw_usage & UINT32_C(0xffff);
    } else {
        *page = global_page;
        *usage = raw_usage;
    }
}

RinResultCode rin_usb_fido_parse_report_descriptor(
    const uint8_t* descriptor, uint32_t descriptor_size,
    RinUsbFidoReportLayout* layout)
{
    HidGlobalState global = {0u, 0u, 0u};
    HidGlobalState stack[4];
    RinUsbFidoReportLayout parsed;
    uint32_t offset = 0u;
    uint32_t stack_size = 0u;
    uint32_t depth = 0u;
    uint32_t fido_depth = 0u;
    uint32_t raw_usage = 0u;
    uint32_t has_usage = 0u;
    uint32_t fido_collection_seen = 0u;
    if (layout) fido_zero(layout, sizeof(*layout));
    if (!descriptor || !layout || descriptor_size == 0u ||
        descriptor_size > RIN_USB_FIDO_REPORT_DESCRIPTOR_MAX)
        return RIN_RESULT_INVALID_ARGUMENT;
    fido_zero(&parsed, sizeof(parsed));
    fido_zero(stack, sizeof(stack));
    while (offset < descriptor_size) {
        uint8_t prefix = descriptor[offset++];
        uint32_t encoded_size;
        uint32_t type;
        uint32_t tag;
        uint32_t value;
        if (prefix == 0xfeu) return RIN_RESULT_NOT_SUPPORTED;
        encoded_size = prefix & 3u;
        encoded_size = encoded_size == 3u ? 4u : encoded_size;
        type = ((uint32_t)prefix >> 2u) & 3u;
        tag = (uint32_t)prefix >> 4u;
        if (encoded_size > descriptor_size - offset)
            return RIN_RESULT_CORRUPT_DATA;
        value = item_value(descriptor + offset, encoded_size);
        offset += encoded_size;
        if (type == 3u) return RIN_RESULT_NOT_SUPPORTED;
        if (type == 1u) {
            if (tag == 0u) global.usage_page = value;
            else if (tag == 7u) global.report_size = value;
            else if (tag == 8u) return RIN_RESULT_NOT_SUPPORTED;
            else if (tag == 9u) global.report_count = value;
            else if (tag == 10u) {
                if (stack_size >= 4u) return RIN_RESULT_LIMIT_EXCEEDED;
                stack[stack_size++] = global;
            } else if (tag == 11u) {
                if (stack_size == 0u) return RIN_RESULT_CORRUPT_DATA;
                global = stack[--stack_size];
            }
        } else if (type == 2u) {
            if (tag == 0u) {
                raw_usage = value;
                has_usage = 1u;
            }
        } else if (type == 0u) {
            uint32_t page = 0u;
            uint32_t usage = 0u;
            if (has_usage)
                effective_usage(global.usage_page, raw_usage, &page, &usage);
            if (tag == 10u) {
                if (depth == UINT32_MAX) return RIN_RESULT_LIMIT_EXCEEDED;
                ++depth;
                if (value == 1u && has_usage &&
                    page == RIN_USB_FIDO_USAGE_PAGE &&
                    usage == RIN_USB_FIDO_USAGE_CTAPHID) {
                    if (fido_collection_seen)
                        return RIN_RESULT_CORRUPT_DATA;
                    fido_collection_seen = 1u;
                    fido_depth = depth;
                }
            } else if (tag == 12u) {
                if (encoded_size != 0u || depth == 0u)
                    return RIN_RESULT_CORRUPT_DATA;
                if (fido_depth == depth) fido_depth = 0u;
                --depth;
            } else if (tag == 8u || tag == 9u) {
                uint64_t bits;
                uint32_t expected_usage =
                    tag == 8u ? RIN_USB_FIDO_USAGE_INPUT
                              : RIN_USB_FIDO_USAGE_OUTPUT;
                if (fido_depth != 0u && depth >= fido_depth && has_usage &&
                    page == RIN_USB_FIDO_USAGE_PAGE &&
                    usage == expected_usage) {
                    if ((value & 3u) != 2u || global.report_size == 0u ||
                        global.report_count == 0u)
                        return RIN_RESULT_CORRUPT_DATA;
                    bits = (uint64_t)global.report_size *
                           (uint64_t)global.report_count;
                    if (bits != RIN_CTAPHID_REPORT_SIZE * 8u)
                        return RIN_RESULT_NOT_SUPPORTED;
                    if (tag == 8u) {
                        if (parsed.input_report_size != 0u)
                            return RIN_RESULT_CORRUPT_DATA;
                        parsed.input_report_size = (uint32_t)(bits / 8u);
                    } else {
                        if (parsed.output_report_size != 0u)
                            return RIN_RESULT_CORRUPT_DATA;
                        parsed.output_report_size = (uint32_t)(bits / 8u);
                    }
                }
            }
            local_clear(&raw_usage, &has_usage);
        }
    }
    if (depth != 0u || stack_size != 0u || fido_depth != 0u ||
        !fido_collection_seen ||
        parsed.input_report_size != RIN_CTAPHID_REPORT_SIZE ||
        parsed.output_report_size != RIN_CTAPHID_REPORT_SIZE)
        return RIN_RESULT_CORRUPT_DATA;
    parsed.usage_page = RIN_USB_FIDO_USAGE_PAGE;
    parsed.usage = RIN_USB_FIDO_USAGE_CTAPHID;
    *layout = parsed;
    return RIN_RESULT_OK;
}

static void broker_lock(RinUsbFidoBroker* broker)
{
    while (__atomic_exchange_n(&broker->lock, 1u, __ATOMIC_ACQUIRE) != 0u) {
    }
}

static void broker_unlock(RinUsbFidoBroker* broker)
{
    __atomic_store_n(&broker->lock, 0u, __ATOMIC_RELEASE);
}

static uint64_t next_id(RinUsbFidoBroker* broker)
{
    ++broker->next_device_id;
    if (broker->next_device_id == 0u) ++broker->next_device_id;
    return broker->next_device_id;
}

static RinUsbFidoDevice* find_device(RinUsbFidoBroker* broker,
                                     uint64_t device_id)
{
    uint32_t index;
    for (index = 0u; index < RIN_USB_FIDO_DEVICE_MAX; ++index) {
        if (broker->devices[index].active &&
            broker->devices[index].device_id == device_id)
            return &broker->devices[index];
    }
    return NULL;
}

static RinResultCode validate_device(RinUsbFidoBroker* broker,
                                     uint64_t device_id,
                                     uint32_t port_generation,
                                     RinUsbFidoDevice** output)
{
    RinUsbFidoDevice* device;
    if (!broker || !broker->initialized || device_id == 0u ||
        port_generation == 0u || !output)
        return RIN_RESULT_INVALID_ARGUMENT;
    device = find_device(broker, device_id);
    if (!device) return RIN_RESULT_NOT_FOUND;
    if (device->port_generation != port_generation)
        return RIN_RESULT_DEVICE_LOST;
    if (!device->transport_started || device->quarantined)
        return RIN_RESULT_DEVICE_LOST;
    *output = device;
    return RIN_RESULT_OK;
}

static RinResultCode emit_report(
    void* context, const uint8_t report[RIN_CTAPHID_REPORT_SIZE])
{
    RinUsbFidoDevice* device = (RinUsbFidoDevice*)context;
    if (!device || !device->active || !device->broker)
        return RIN_RESULT_DEVICE_LOST;
    return device->broker->transport.submit_output(
        device->broker->transport_context, device->transport_cookie,
        device->port_generation, report, device->emit_deadline_ms);
}

RinResultCode rin_usb_fido_broker_init(
    RinUsbFidoBroker* broker, const RinUsbFidoTransportV1* transport,
    void* transport_context)
{
    uint32_t index;
    if (!broker || !transport ||
        transport->struct_size < sizeof(RinUsbFidoTransportV1) ||
        transport->version != RIN_USB_FIDO_BROKER_VERSION ||
        !transport->start_input || !transport->submit_output ||
        !transport->cancel_transfers)
        return RIN_RESULT_INVALID_ARGUMENT;
    fido_zero(broker, sizeof(*broker));
    fido_copy(&broker->transport, transport, sizeof(*transport));
    broker->transport_context = transport_context;
    broker->initialized = 1u;
    for (index = 0u; index < RIN_USB_FIDO_DEVICE_MAX; ++index)
        rin_ctaphid_client_init(&broker->devices[index].client);
    return RIN_RESULT_OK;
}

RinResultCode rin_usb_fido_attach(
    RinUsbFidoBroker* broker, uint64_t transport_cookie,
    uint32_t port_generation, uint32_t interrupt_in_packet_size,
    uint32_t interrupt_out_packet_size, const uint8_t* report_descriptor,
    uint32_t report_descriptor_size, uint64_t* device_id)
{
    RinUsbFidoReportLayout layout;
    RinUsbFidoDevice* device = NULL;
    RinResultCode result;
    uint32_t index;
    if (device_id) *device_id = 0u;
    if (!broker || !broker->initialized || !device_id ||
        transport_cookie == 0u || port_generation == 0u ||
        interrupt_in_packet_size != RIN_CTAPHID_REPORT_SIZE ||
        interrupt_out_packet_size != RIN_CTAPHID_REPORT_SIZE)
        return RIN_RESULT_INVALID_ARGUMENT;
    result = rin_usb_fido_parse_report_descriptor(
        report_descriptor, report_descriptor_size, &layout);
    if (result != RIN_RESULT_OK) return result;
    broker_lock(broker);
    for (index = 0u; index < RIN_USB_FIDO_DEVICE_MAX; ++index) {
        if (!broker->devices[index].active) {
            device = &broker->devices[index];
            break;
        }
    }
    if (!device) {
        broker_unlock(broker);
        return RIN_RESULT_TABLE_FULL;
    }
    fido_zero(device, sizeof(*device));
    rin_ctaphid_client_init(&device->client);
    device->broker = broker;
    device->device_id = next_id(broker);
    device->transport_cookie = transport_cookie;
    device->port_generation = port_generation;
    device->active = 1u;
    broker_unlock(broker);
    result = broker->transport.start_input(
        broker->transport_context, transport_cookie, port_generation);
    broker_lock(broker);
    if (result != RIN_RESULT_OK) {
        fido_zero(device, sizeof(*device));
        broker_unlock(broker);
        return result;
    }
    device->transport_started = 1u;
    *device_id = device->device_id;
    broker_unlock(broker);
    return RIN_RESULT_OK;
}

RinResultCode rin_usb_fido_detach(RinUsbFidoBroker* broker,
                                  uint64_t device_id,
                                  uint32_t port_generation)
{
    RinUsbFidoDevice* device;
    RinResultCode result;
    if (!broker || !broker->initialized) return RIN_RESULT_INVALID_ARGUMENT;
    broker_lock(broker);
    device = find_device(broker, device_id);
    if (!device) {
        broker_unlock(broker);
        return RIN_RESULT_NOT_FOUND;
    }
    if (device->port_generation != port_generation) {
        broker_unlock(broker);
        return RIN_RESULT_DEVICE_LOST;
    }
    device->transport_started = 0u;
    broker_unlock(broker);
    result = broker->transport.cancel_transfers(
        broker->transport_context, device->transport_cookie,
        device->port_generation);
    broker_lock(broker);
    if (result == RIN_RESULT_OK) {
        rin_ctaphid_client_disconnect(&device->client);
        fido_zero(device, sizeof(*device));
    } else if (device->active && device->device_id == device_id &&
               device->port_generation == port_generation) {
        device->transport_started = 1u;
        device->quarantined = 1u;
    }
    broker_unlock(broker);
    return result;
}

static uint64_t deadline(uint64_t now_ms, uint32_t timeout_ms)
{
    return now_ms > UINT64_MAX - timeout_ms ? UINT64_MAX
                                            : now_ms + timeout_ms;
}

RinResultCode rin_usb_fido_begin_init(
    RinUsbFidoBroker* broker, uint64_t device_id,
    uint32_t port_generation, const uint8_t nonce[8], uint64_t now_ms,
    uint32_t timeout_ms)
{
    RinUsbFidoDevice* device;
    RinResultCode result;
    if (!broker || !broker->initialized) return RIN_RESULT_INVALID_ARGUMENT;
    broker_lock(broker);
    result = validate_device(broker, device_id, port_generation, &device);
    if (result == RIN_RESULT_OK) {
        device->emit_deadline_ms = deadline(now_ms, timeout_ms);
        result = rin_ctaphid_client_begin_init(
            &device->client, nonce, now_ms, timeout_ms, emit_report, device);
        device->emit_deadline_ms = 0u;
    }
    broker_unlock(broker);
    return result;
}

RinResultCode rin_usb_fido_begin_cbor(
    RinUsbFidoBroker* broker, uint64_t device_id,
    uint32_t port_generation, const uint8_t* payload, uint32_t payload_size,
    uint64_t now_ms, uint32_t timeout_ms)
{
    RinUsbFidoDevice* device;
    RinResultCode result;
    if (!broker || !broker->initialized) return RIN_RESULT_INVALID_ARGUMENT;
    broker_lock(broker);
    result = validate_device(broker, device_id, port_generation, &device);
    if (result == RIN_RESULT_OK) {
        device->emit_deadline_ms = deadline(now_ms, timeout_ms);
        result = rin_ctaphid_client_begin_cbor(
            &device->client, payload, payload_size, now_ms, timeout_ms,
            emit_report, device);
        device->emit_deadline_ms = 0u;
    }
    broker_unlock(broker);
    return result;
}

RinResultCode rin_usb_fido_cancel(RinUsbFidoBroker* broker,
                                  uint64_t device_id,
                                  uint32_t port_generation)
{
    RinUsbFidoDevice* device;
    RinResultCode result;
    if (!broker || !broker->initialized) return RIN_RESULT_INVALID_ARGUMENT;
    broker_lock(broker);
    result = validate_device(broker, device_id, port_generation, &device);
    if (result == RIN_RESULT_OK) {
        device->emit_deadline_ms = device->client.deadline_ms;
        result = rin_ctaphid_client_cancel(&device->client, emit_report,
                                            device);
        device->emit_deadline_ms = 0u;
    }
    broker_unlock(broker);
    return result;
}

RinResultCode rin_usb_fido_feed_report(
    RinUsbFidoBroker* broker, uint64_t device_id,
    uint32_t port_generation,
    const uint8_t report[RIN_CTAPHID_REPORT_SIZE], uint64_t now_ms,
    RinCtapHidMessage* message, uint32_t* event)
{
    RinUsbFidoDevice* device;
    RinResultCode result;
    if (message) fido_zero(message, sizeof(*message));
    if (event) *event = RIN_CTAPHID_CLIENT_EVENT_NONE;
    if (!broker || !broker->initialized || !report || !message || !event)
        return RIN_RESULT_INVALID_ARGUMENT;
    broker_lock(broker);
    result = validate_device(broker, device_id, port_generation, &device);
    if (result == RIN_RESULT_OK)
        result = rin_ctaphid_client_feed(&device->client, report, now_ms,
                                         message, event);
    broker_unlock(broker);
    return result;
}

RinResultCode rin_usb_fido_poll(RinUsbFidoBroker* broker,
                                uint64_t device_id,
                                uint32_t port_generation,
                                uint64_t now_ms)
{
    RinUsbFidoDevice* device;
    RinResultCode result;
    if (!broker || !broker->initialized) return RIN_RESULT_INVALID_ARGUMENT;
    broker_lock(broker);
    result = validate_device(broker, device_id, port_generation, &device);
    if (result == RIN_RESULT_OK)
        result = rin_ctaphid_client_poll(&device->client, now_ms);
    broker_unlock(broker);
    return result;
}

RinResultCode rin_usb_fido_first_ready(
    RinUsbFidoBroker* broker, RinUsbFidoDeviceSnapshot* snapshot)
{
    uint32_t index;
    if (snapshot) fido_zero(snapshot, sizeof(*snapshot));
    if (!broker || !broker->initialized || !snapshot)
        return RIN_RESULT_INVALID_ARGUMENT;
    broker_lock(broker);
    for (index = 0u; index < RIN_USB_FIDO_DEVICE_MAX; ++index) {
        const RinUsbFidoDevice* device = &broker->devices[index];
        if (!device->active || !device->transport_started ||
            device->quarantined ||
            device->client.state != RIN_CTAPHID_CLIENT_IDLE ||
            (device->client.capabilities &
             RIN_CTAPHID_CAPABILITY_CBOR) == 0u)
            continue;
        snapshot->device_id = device->device_id;
        snapshot->transport_cookie = device->transport_cookie;
        snapshot->port_generation = device->port_generation;
        snapshot->client_state = device->client.state;
        snapshot->capabilities = device->client.capabilities;
        snapshot->ready = 1u;
        broker_unlock(broker);
        return RIN_RESULT_OK;
    }
    broker_unlock(broker);
    return RIN_RESULT_NOT_FOUND;
}
