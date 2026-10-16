/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <rin/driver/virtio/pci_transport_v3.h>

#define PCI_STATUS_CAPABILITIES UINT16_C(0x0010)
#define PCI_CAPABILITY_LIST UINT16_C(0x0034)
#define PCI_CAP_ID_VENDOR UINT8_C(0x09)
#define VIRTIO_COMMON_MINIMUM_SIZE UINT32_C(0x38)
#define VIRTIO_CAPABILITY_VISIT_LIMIT 48u

static void virtio_pci_zero(void* memory, uint32_t size)
{
    uint8_t* bytes = (uint8_t*)memory;
    for (uint32_t index = 0u; index < size; ++index) bytes[index] = 0u;
}

static RinDriverV3Result config_read(
    const RinDriverApiV3* api, const RinDriverContextV3* context,
    uint16_t offset, uint8_t width, uint32_t* value)
{
    if (!api || !context || !value ||
        api->version != RIN_DRIVER_V3_ABI_VERSION ||
        context->version != RIN_DRIVER_V3_ABI_VERSION ||
        context->session == 0u ||
        (api->capabilities & RIN_DRIVER_V3_CAP_PCI_CONFIG) == 0u ||
        !api->pci_config_read) {
        if (value) *value = 0u;
        return RIN_DRIVER_V3_DENIED;
    }
    return api->pci_config_read(api->api_context, context->session,
                                offset, width, value);
}

static int capability_range_valid(uint32_t offset, uint32_t length,
                                  uint32_t minimum_length)
{
    return length >= minimum_length && offset <= UINT32_MAX - length;
}

static RinDriverV3Result read_capability(
    const RinDriverApiV3* api, const RinDriverContextV3* context,
    uint8_t capability, uint8_t capability_length, uint8_t config_type,
    RinVirtioPciCapabilityV3* output)
{
    uint32_t bar = 0u;
    uint32_t offset = 0u;
    uint32_t length = 0u;
    uint32_t multiplier = 0u;
    uint32_t minimum_length = config_type == RIN_VIRTIO_PCI_CAP_COMMON_CFG
        ? VIRTIO_COMMON_MINIMUM_SIZE : 1u;
    RinDriverV3Result result;

    if (!output || capability_length < 16u || capability > 0xf0u) {
        return RIN_DRIVER_V3_INVALID;
    }
    result = config_read(api, context, (uint16_t)(capability + 4u), 1u, &bar);
    if (result != RIN_DRIVER_V3_OK) return result;
    result = config_read(api, context, (uint16_t)(capability + 8u), 4u,
                         &offset);
    if (result != RIN_DRIVER_V3_OK) return result;
    result = config_read(api, context, (uint16_t)(capability + 12u), 4u,
                         &length);
    if (result != RIN_DRIVER_V3_OK) return result;
    if (bar >= 6u || !capability_range_valid(offset, length, minimum_length)) {
        return RIN_DRIVER_V3_INVALID;
    }
    if (config_type == RIN_VIRTIO_PCI_CAP_NOTIFY_CFG) {
        if (capability_length < 20u || capability > 0xecu) {
            return RIN_DRIVER_V3_INVALID;
        }
        result = config_read(api, context, (uint16_t)(capability + 16u), 4u,
                             &multiplier);
        if (result != RIN_DRIVER_V3_OK || multiplier == 0u) {
            return result == RIN_DRIVER_V3_OK ? RIN_DRIVER_V3_INVALID : result;
        }
    }
    output->bar_index = bar;
    output->offset = offset;
    output->length = length;
    output->notify_multiplier = multiplier;
    return RIN_DRIVER_V3_OK;
}

RinDriverV3Result rin_virtio_pci_discover_v3(
    const RinDriverApiV3* api, const RinDriverContextV3* context,
    RinVirtioPciLayoutV3* output)
{
    RinVirtioPciLayoutV3 layout;
    uint8_t visited[256];
    uint32_t identity = 0u;
    uint32_t status = 0u;
    uint32_t capability_value = 0u;
    uint8_t capability;
    uint32_t visits = 0u;
    uint32_t found = 0u;
    RinDriverV3Result result;

    if (!output) return RIN_DRIVER_V3_INVALID;
    virtio_pci_zero(output, sizeof(*output));
    virtio_pci_zero(&layout, sizeof(layout));
    virtio_pci_zero(visited, sizeof(visited));

    result = config_read(api, context, 0u, 4u, &identity);
    if (result != RIN_DRIVER_V3_OK) return result;
    layout.vendor_id = (uint16_t)identity;
    layout.device_id = (uint16_t)(identity >> 16u);
    if (layout.vendor_id != RIN_VIRTIO_PCI_VENDOR_ID ||
        layout.device_id < UINT16_C(0x1040) ||
        layout.device_id > UINT16_C(0x107f)) {
        return RIN_DRIVER_V3_DENIED;
    }
    result = config_read(api, context, 0x06u, 2u, &status);
    if (result != RIN_DRIVER_V3_OK) return result;
    if (((uint16_t)status & PCI_STATUS_CAPABILITIES) == 0u) {
        return RIN_DRIVER_V3_INVALID;
    }
    result = config_read(api, context, PCI_CAPABILITY_LIST, 1u,
                         &capability_value);
    if (result != RIN_DRIVER_V3_OK) return result;
    capability = (uint8_t)capability_value;

    while (capability != 0u) {
        uint32_t header = 0u;
        uint8_t capability_id;
        uint8_t next;
        uint8_t capability_length;
        uint8_t config_type;
        RinVirtioPciCapabilityV3* destination = NULL;
        uint32_t found_bit = 0u;

        if (capability < 0x40u || (capability & 3u) != 0u ||
            visited[capability] != 0u ||
            visits++ >= VIRTIO_CAPABILITY_VISIT_LIMIT) {
            return RIN_DRIVER_V3_INVALID;
        }
        visited[capability] = 1u;
        result = config_read(api, context, capability, 4u, &header);
        if (result != RIN_DRIVER_V3_OK) return result;
        capability_id = (uint8_t)header;
        next = (uint8_t)(header >> 8u);
        capability_length = (uint8_t)(header >> 16u);
        config_type = (uint8_t)(header >> 24u);

        if (capability_id == PCI_CAP_ID_VENDOR) {
            if (config_type == RIN_VIRTIO_PCI_CAP_COMMON_CFG) {
                destination = &layout.common;
                found_bit = UINT32_C(1) << 0;
            } else if (config_type == RIN_VIRTIO_PCI_CAP_NOTIFY_CFG) {
                destination = &layout.notify;
                found_bit = UINT32_C(1) << 1;
            } else if (config_type == RIN_VIRTIO_PCI_CAP_ISR_CFG) {
                destination = &layout.isr;
                found_bit = UINT32_C(1) << 2;
            } else if (config_type == RIN_VIRTIO_PCI_CAP_DEVICE_CFG) {
                destination = &layout.device;
                found_bit = UINT32_C(1) << 3;
            }
            if (destination) {
                if ((found & found_bit) != 0u) return RIN_DRIVER_V3_INVALID;
                result = read_capability(api, context, capability,
                                         capability_length, config_type,
                                         destination);
                if (result != RIN_DRIVER_V3_OK) return result;
                found |= found_bit;
            }
        }
        if (next != 0u && (next < 0x40u || (next & 3u) != 0u)) {
            return RIN_DRIVER_V3_INVALID;
        }
        capability = next;
    }
    if (found != UINT32_C(0x0f)) return RIN_DRIVER_V3_INVALID;
    *output = layout;
    return RIN_DRIVER_V3_OK;
}
