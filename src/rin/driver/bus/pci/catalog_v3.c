/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <rin/driver/bus/pci/catalog_v3.h>

#define PCI_VENDOR_INVALID UINT16_C(0xffff)
#define PCI_STATUS_CAPABILITIES UINT32_C(0x00100000)
#define PCI_HEADER_MULTIFUNCTION UINT8_C(0x80)
#define PCI_HEADER_TYPE_MASK UINT8_C(0x7f)
#define PCI_HEADER_DEVICE UINT8_C(0x00)
#define PCI_HEADER_BRIDGE UINT8_C(0x01)
#define PCI_CAPABILITY_MSI UINT8_C(0x05)
#define PCI_CAPABILITY_MSIX UINT8_C(0x11)

static void catalog_zero(void* pointer, size_t size)
{
    uint8_t* bytes = (uint8_t*)pointer;
    while (size-- != 0u) *bytes++ = 0u;
}

static int read_config(const RinDriverPciCatalogConfigV3* config,
                       uint8_t bus, uint8_t device, uint8_t function,
                       uint16_t offset, uint32_t* value)
{
    if (!value || (offset & 3u) != 0u || offset > UINT16_C(0x00fc)) {
        return -1;
    }
    *value = UINT32_MAX;
    return config->read32(config->read_context, config->segment, bus,
                          device, function, offset, value);
}

static uint64_t class_resources(uint32_t class_code)
{
    uint8_t base_class = (uint8_t)(class_code >> 16u);
    uint8_t subclass = (uint8_t)(class_code >> 8u);
    if (base_class == 0x01u) return RIN_DRIVER_RESOURCE_BLOCK;
    if (base_class == 0x02u) return RIN_DRIVER_RESOURCE_NETWORK;
    if (base_class == 0x03u) return RIN_DRIVER_RESOURCE_DISPLAY;
    if (base_class == 0x04u) return RIN_DRIVER_RESOURCE_AUDIO;
    if (base_class == 0x09u) return RIN_DRIVER_RESOURCE_INPUT;
    if (base_class == 0x0cu && subclass == 0x03u) {
        return RIN_DRIVER_RESOURCE_USB;
    }
    return 0u;
}

static int scan_capabilities(const RinDriverPciCatalogConfigV3* config,
                             RinDriverPciDeviceV3* output,
                             uint32_t command_status)
{
    uint32_t pointer_value;
    uint64_t visited = 0u;
    uint8_t offset;
    uint32_t capabilities = 0u;
    if ((command_status & PCI_STATUS_CAPABILITIES) == 0u) return 0;
    if (read_config(config, output->address.bus, output->address.device,
                    output->address.function, 0x34u, &pointer_value) != 0) {
        return -1;
    }
    offset = (uint8_t)(pointer_value & 0xffu);
    for (uint32_t count = 0u; offset != 0u && count < 48u; ++count) {
        uint64_t bit;
        uint32_t capability;
        uint8_t next;
        if (offset < 0x40u || offset > 0xfcu || (offset & 3u) != 0u) {
            output->flags |= RIN_DRIVER_PCI_V3_FLAG_CAPABILITIES_MALFORMED;
            output->capabilities = 0u;
            return 0;
        }
        bit = UINT64_C(1) << (offset >> 2u);
        if ((visited & bit) != 0u) {
            output->flags |= RIN_DRIVER_PCI_V3_FLAG_CAPABILITIES_MALFORMED;
            output->capabilities = 0u;
            return 0;
        }
        visited |= bit;
        if (read_config(config, output->address.bus, output->address.device,
                        output->address.function, offset, &capability) != 0) {
            return -1;
        }
        if ((uint8_t)capability == PCI_CAPABILITY_MSI) {
            capabilities |= RIN_DRIVER_PCI_V3_CAP_MSI;
        } else if ((uint8_t)capability == PCI_CAPABILITY_MSIX) {
            capabilities |= RIN_DRIVER_PCI_V3_CAP_MSIX;
        }
        next = (uint8_t)(capability >> 8u);
        if (next != 0u &&
            (next < 0x40u || next > 0xfcu || (next & 3u) != 0u)) {
            output->flags |= RIN_DRIVER_PCI_V3_FLAG_CAPABILITIES_MALFORMED;
            output->capabilities = 0u;
            return 0;
        }
        offset = next;
    }
    if (offset != 0u) {
        output->flags |= RIN_DRIVER_PCI_V3_FLAG_CAPABILITIES_MALFORMED;
        output->capabilities = 0u;
        return 0;
    }
    output->capabilities = capabilities;
    return 0;
}

static int scan_bars(const RinDriverPciCatalogConfigV3* config,
                     RinDriverPciDeviceV3* output)
{
    uint8_t type = output->header_type & PCI_HEADER_TYPE_MASK;
    uint8_t bar_count = type == PCI_HEADER_DEVICE ? 6u :
                        type == PCI_HEADER_BRIDGE ? 2u : 0u;
    for (uint8_t index = 0u; index < bar_count; ++index) {
        uint32_t value;
        if (read_config(config, output->address.bus, output->address.device,
                        output->address.function,
                        (uint16_t)(0x10u + index * 4u), &value) != 0) {
            return -1;
        }
        if (value == 0u || value == UINT32_MAX) continue;
        if ((value & 1u) != 0u) {
            if ((value & UINT32_C(2)) != 0u) {
                output->flags |= RIN_DRIVER_PCI_V3_FLAG_CONFIG_MALFORMED;
            } else if ((value & ~UINT32_C(3)) != 0u) {
                output->io_bar_mask |= (uint8_t)(1u << index);
            }
        } else {
            uint32_t memory_type = (value >> 1u) & 3u;
            if (memory_type == 1u || memory_type == 3u) {
                output->flags |= RIN_DRIVER_PCI_V3_FLAG_CONFIG_MALFORMED;
                continue;
            }
            if (memory_type == 2u) {
                if (index + 1u >= bar_count) {
                    output->flags |= RIN_DRIVER_PCI_V3_FLAG_CONFIG_MALFORMED;
                } else {
                    uint32_t high = 0u;
                    if (read_config(config, output->address.bus,
                                    output->address.device,
                                    output->address.function,
                                    (uint16_t)(0x10u +
                                               (index + 1u) * 4u),
                                    &high) != 0) {
                        return -1;
                    }
                    /* A 64-bit BAR may be placed above 4 GiB. In that case
                     * its low address dword is zero and only the upper dword
                     * proves that the BAR is assigned. */
                    if ((value & ~UINT32_C(15)) != 0u || high != 0u) {
                        output->memory_bar_mask |= (uint8_t)(1u << index);
                    }
                    ++index;
                }
            } else if ((value & ~UINT32_C(15)) != 0u) {
                output->memory_bar_mask |= (uint8_t)(1u << index);
            }
        }
    }
    return 0;
}

static uint64_t device_cookie(uint32_t generation, uint16_t segment,
                              uint8_t bus, uint8_t device, uint8_t function)
{
    uint32_t address = ((uint32_t)segment << 16u) |
                       ((uint32_t)bus << 8u) |
                       ((uint32_t)device << 3u) | function;
    return ((uint64_t)generation << 32u) | address;
}

static int scan_function(const RinDriverPciCatalogConfigV3* config,
                         uint8_t bus, uint8_t device, uint8_t function,
                         uint32_t identity, RinDriverPciDeviceV3* output)
{
    uint32_t class_revision;
    uint32_t header;
    uint32_t command_status;
    uint32_t interrupt;
    uint32_t subsystem = 0u;
    uint16_t vendor = (uint16_t)identity;
    if (vendor == 0u || vendor == PCI_VENDOR_INVALID) return 1;
    catalog_zero(output, sizeof(*output));
    output->address.segment = config->segment;
    output->address.bus = bus;
    output->address.device = device;
    output->address.function = function;
    if (read_config(config, bus, device, function, 0x08u,
                    &class_revision) != 0 ||
        read_config(config, bus, device, function, 0x0cu, &header) != 0 ||
        read_config(config, bus, device, function, 0x04u,
                    &command_status) != 0 ||
        read_config(config, bus, device, function, 0x3cu, &interrupt) != 0) {
        return -1;
    }
    output->header_type = (uint8_t)(header >> 16u);
    if ((output->header_type & PCI_HEADER_TYPE_MASK) > PCI_HEADER_BRIDGE) {
        output->flags |= RIN_DRIVER_PCI_V3_FLAG_CONFIG_MALFORMED;
    }
    if ((output->header_type & PCI_HEADER_TYPE_MASK) == PCI_HEADER_DEVICE &&
        read_config(config, bus, device, function, 0x2cu, &subsystem) != 0) {
        return -1;
    }
    output->interrupt_pin = (uint8_t)(interrupt >> 8u);
    if (output->interrupt_pin > 4u) {
        output->flags |= RIN_DRIVER_PCI_V3_FLAG_CONFIG_MALFORMED;
    }
    if (scan_bars(config, output) != 0 ||
        scan_capabilities(config, output, command_status) != 0) {
        return -1;
    }
    output->identity.struct_size = sizeof(output->identity);
    output->identity.version = RIN_DRIVER_ADMISSION_V3_VERSION;
    output->identity.device_cookie = device_cookie(
        config->generation, config->segment, bus, device, function);
    output->identity.bus_type = RIN_DRIVER_BUS_PCI;
    output->identity.vendor_id = vendor;
    output->identity.device_id = (uint16_t)(identity >> 16u);
    output->identity.subsystem_vendor_id = (uint16_t)subsystem;
    output->identity.subsystem_device_id = (uint16_t)(subsystem >> 16u);
    output->identity.class_code = class_revision >> 8u;
    output->identity.revision = class_revision & 0xffu;
    output->identity.generation = config->generation;
    output->identity.available_resources =
        RIN_DRIVER_RESOURCE_PCI |
        (config->proven_resources & RIN_DRIVER_RESOURCE_DMA) |
        class_resources(output->identity.class_code);
    if (output->memory_bar_mask != 0u &&
        (config->proven_resources & RIN_DRIVER_RESOURCE_MMIO) != 0u) {
        output->identity.available_resources |= RIN_DRIVER_RESOURCE_MMIO;
    }
    if (output->io_bar_mask != 0u &&
        (config->proven_resources & RIN_DRIVER_RESOURCE_IOPORT) != 0u) {
        output->identity.available_resources |= RIN_DRIVER_RESOURCE_IOPORT;
    }
    if ((output->interrupt_pin != 0u || output->capabilities != 0u) &&
        (config->proven_resources & RIN_DRIVER_RESOURCE_IRQ) != 0u) {
        output->identity.available_resources |= RIN_DRIVER_RESOURCE_IRQ;
    }
    return 0;
}

static int pci_image_has_admissible_matches(
    const RinDriverHeaderV3* image_header,
    const RinDriverMatchV3* image_matches)
{
    for (uint32_t index = 0u; index < image_header->match_count; ++index) {
        const RinDriverMatchV3* match = &image_matches[index];
        if (match->bus_type != RIN_DRIVER_BUS_PCI ||
            (match->flags & RIN_DRIVER_MATCH_VENDOR_DEVICE) != 0u) continue;
        /* NVMe is a standardized PCI programming interface. Permit its exact
         * class predicate so one protocol driver can support conforming
         * controllers from different vendors. Other broad PCI predicates
         * still require an explicit vendor/device profile. */
        if (match->flags != RIN_DRIVER_MATCH_CLASS ||
            match->class_code != UINT32_C(0x010802) ||
            match->class_mask != UINT32_C(0x00ffffff)) return 0;
    }
    return 1;
}

static int pci_catalog_shape_valid(const RinDriverPciCatalogV3* catalog)
{
    return catalog && catalog->struct_size >= sizeof(*catalog) &&
           catalog->version == RIN_DRIVER_PCI_CATALOG_V3_VERSION &&
           catalog->devices && catalog->capacity != 0u &&
           catalog->count <= catalog->capacity && catalog->complete == 1u &&
           catalog->generation != 0u;
}

RinDriverPciCatalogV3Result rin_driver_pci_catalog_v3_scan(
    RinDriverPciCatalogV3* catalog,
    const RinDriverPciCatalogConfigV3* config)
{
    uint32_t count = 0u;
    if (catalog) {
        catalog->count = 0u;
        catalog->complete = 0u;
    }
    if (!catalog || !config || !catalog->devices || catalog->capacity == 0u ||
        config->struct_size < sizeof(*config) ||
        config->version != RIN_DRIVER_PCI_CATALOG_V3_VERSION ||
        config->first_bus > config->last_bus || config->generation == 0u ||
        !config->read32 ||
        (config->proven_resources &
         ~RIN_DRIVER_PCI_V3_PROVABLE_RESOURCES) != 0u ||
        config->reserved[0] != 0u || config->reserved[1] != 0u) {
        return RIN_DRIVER_PCI_CATALOG_V3_INVALID;
    }
    catalog->struct_size = sizeof(*catalog);
    catalog->version = RIN_DRIVER_PCI_CATALOG_V3_VERSION;
    catalog->generation = config->generation;
    for (uint32_t bus_value = config->first_bus;
         bus_value <= config->last_bus; ++bus_value) {
        uint8_t bus = (uint8_t)bus_value;
        for (uint8_t device = 0u; device < 32u; ++device) {
            uint32_t identity0;
            uint32_t header0;
            uint8_t function_limit = 1u;
            if (read_config(config, bus, device, 0u, 0u, &identity0) != 0) {
                goto io_failure;
            }
            if ((uint16_t)identity0 == 0u ||
                (uint16_t)identity0 == PCI_VENDOR_INVALID) continue;
            if (read_config(config, bus, device, 0u, 0x0cu, &header0) != 0) {
                goto io_failure;
            }
            if (((uint8_t)(header0 >> 16u) &
                 PCI_HEADER_MULTIFUNCTION) != 0u) function_limit = 8u;
            for (uint8_t function = 0u; function < function_limit; ++function) {
                uint32_t identity = identity0;
                int scanned;
                if (function != 0u &&
                    read_config(config, bus, device, function, 0u,
                                &identity) != 0) goto io_failure;
                if ((uint16_t)identity == 0u ||
                    (uint16_t)identity == PCI_VENDOR_INVALID) continue;
                if (count == catalog->capacity) {
                    catalog->count = count;
                    return RIN_DRIVER_PCI_CATALOG_V3_LIMIT;
                }
                scanned = scan_function(config, bus, device, function,
                                        identity, &catalog->devices[count]);
                if (scanned < 0) goto io_failure;
                if (scanned == 0) ++count;
            }
        }
        if (bus_value == UINT8_MAX) break;
    }
    catalog->count = count;
    catalog->complete = 1u;
    return RIN_DRIVER_PCI_CATALOG_V3_OK;

io_failure:
    catalog->count = 0u;
    catalog->complete = 0u;
    return RIN_DRIVER_PCI_CATALOG_V3_IO;
}

const RinDriverPciDeviceV3* rin_driver_pci_catalog_v3_find(
    const RinDriverPciCatalogV3* catalog, uint64_t cookie,
    uint64_t generation)
{
    if (!pci_catalog_shape_valid(catalog) || cookie == 0u ||
        generation == 0u || generation != catalog->generation) return NULL;
    for (uint32_t index = 0u; index < catalog->count; ++index) {
        if (catalog->devices[index].identity.device_cookie == cookie &&
            catalog->devices[index].identity.generation == generation) {
            return &catalog->devices[index];
        }
    }
    return NULL;
}

RinDriverPciMatchV3Result rin_driver_pci_catalog_v3_match(
    const RinDriverPciCatalogV3* catalog,
    const RinDriverHeaderV3* image_header,
    const RinDriverMatchV3* image_matches,
    const RinDriverAdmissionPolicyV3* policy,
    const RinDriverPciDeviceV3** matched_device, uint32_t* matched_index)
{
    const RinDriverPciDeviceV3* unmet = NULL;
    uint32_t unmet_index = UINT32_MAX;
    if (matched_device) *matched_device = NULL;
    if (matched_index) *matched_index = UINT32_MAX;
    if (!pci_catalog_shape_valid(catalog) || !image_header ||
        !image_matches || image_header->match_count == 0u ||
        image_header->match_count > RIN_DRIVER_ADMISSION_V3_MAX_MATCHES ||
        !policy || policy->struct_size < sizeof(*policy) ||
        policy->version != RIN_DRIVER_ADMISSION_V3_VERSION ||
        (policy->architecture != RIN_ARCH_X86 &&
         policy->architecture != RIN_ARCH_X86_64) ||
        policy->allow_experimental > 1u || policy->reserved0 != 0u ||
        policy->reserved[0] != 0u || policy->reserved[1] != 0u) {
        return RIN_DRIVER_PCI_MATCH_V3_INVALID;
    }
    /* Class-only PCI admission is limited to the exact standard NVMe class;
     * other broad predicates still need an explicit device profile. */
    if (!pci_image_has_admissible_matches(image_header, image_matches))
        return RIN_DRIVER_PCI_MATCH_V3_NO_DEVICE;
    for (uint32_t index = 0u; index < catalog->count; ++index) {
        if ((catalog->devices[index].flags &
             (RIN_DRIVER_PCI_V3_FLAG_CAPABILITIES_MALFORMED |
              RIN_DRIVER_PCI_V3_FLAG_CONFIG_MALFORMED)) != 0u) {
            continue;
        }
        uint32_t image_match = UINT32_MAX;
        RinDriverAdmissionV3Result result =
            rin_driver_admission_v3_match_image(
                image_header, image_matches,
                &catalog->devices[index].identity, policy, &image_match);
        if (result == RIN_DRIVER_ADMISSION_V3_OK) {
            if (matched_device) *matched_device = &catalog->devices[index];
            if (matched_index) *matched_index = image_match;
            return RIN_DRIVER_PCI_MATCH_V3_OK;
        }
        if (!unmet) {
            RinDriverDeviceIdentityV3 structural =
                catalog->devices[index].identity;
            structural.available_resources |= image_header->required_resources;
            if (rin_driver_admission_v3_match_image(
                    image_header, image_matches, &structural, policy,
                    &image_match) ==
                RIN_DRIVER_ADMISSION_V3_OK) {
                unmet = &catalog->devices[index];
                unmet_index = image_match;
            }
        }
    }
    if (unmet) {
        if (matched_device) *matched_device = unmet;
        if (matched_index) *matched_index = unmet_index;
        return RIN_DRIVER_PCI_MATCH_V3_REQUIREMENTS_UNMET;
    }
    return RIN_DRIVER_PCI_MATCH_V3_NO_DEVICE;
}

RinDriverPciMatchV3Result rin_driver_pci_catalog_v3_match_all(
    const RinDriverPciCatalogV3* catalog,
    const RinDriverHeaderV3* image_header,
    const RinDriverMatchV3* image_matches,
    const RinDriverAdmissionPolicyV3* policy,
    const RinDriverPciDeviceV3** matched_devices,
    uint32_t* matched_indices, uint32_t capacity, uint32_t* matched_count)
{
    uint32_t count = 0u;
    int requirements_unmet = 0;

    if (matched_count != NULL)
        *matched_count = 0u;
    if (!pci_catalog_shape_valid(catalog) || !image_header ||
        !image_matches || !policy || !matched_count || capacity == 0u ||
        !matched_devices || !matched_indices ||
        image_header->match_count == 0u ||
        image_header->match_count > RIN_DRIVER_ADMISSION_V3_MAX_MATCHES ||
        policy->struct_size < sizeof(*policy) ||
        policy->version != RIN_DRIVER_ADMISSION_V3_VERSION ||
        (policy->architecture != RIN_ARCH_X86 &&
         policy->architecture != RIN_ARCH_X86_64) ||
        policy->allow_experimental > 1u || policy->reserved0 != 0u ||
        policy->reserved[0] != 0u || policy->reserved[1] != 0u)
        return RIN_DRIVER_PCI_MATCH_V3_INVALID;

    /* Keep the same NVMe-only class exception for both single-device and
     * multi-device admission. Enumeration must not widen the class policy. */
    if (!pci_image_has_admissible_matches(image_header, image_matches))
        return RIN_DRIVER_PCI_MATCH_V3_NO_DEVICE;

    for (uint32_t index = 0u; index < catalog->count; ++index) {
        const RinDriverPciDeviceV3* device = &catalog->devices[index];
        uint32_t image_match = UINT32_MAX;
        RinDriverAdmissionV3Result result;

        if ((device->flags &
             (RIN_DRIVER_PCI_V3_FLAG_CAPABILITIES_MALFORMED |
              RIN_DRIVER_PCI_V3_FLAG_CONFIG_MALFORMED)) != 0u)
            continue;
        result = rin_driver_admission_v3_match_image(
            image_header, image_matches, &device->identity, policy,
            &image_match);
        if (result == RIN_DRIVER_ADMISSION_V3_OK) {
            if (count == capacity)
                return RIN_DRIVER_PCI_MATCH_V3_LIMIT;
            matched_devices[count] = device;
            matched_indices[count] = image_match;
            ++count;
            continue;
        }

        /* Preserve the existing REQUIREMENTS_UNMET distinction without
         * treating an unrelated device as a driver failure. */
        if (!requirements_unmet) {
            RinDriverDeviceIdentityV3 structural = device->identity;
            structural.available_resources |= image_header->required_resources;
            if (rin_driver_admission_v3_match_image(
                    image_header, image_matches, &structural, policy,
                    &image_match) ==
                RIN_DRIVER_ADMISSION_V3_OK)
                requirements_unmet = 1;
        }
    }

    *matched_count = count;
    if (count != 0u)
        return RIN_DRIVER_PCI_MATCH_V3_OK;
    return requirements_unmet ? RIN_DRIVER_PCI_MATCH_V3_REQUIREMENTS_UNMET
                              : RIN_DRIVER_PCI_MATCH_V3_NO_DEVICE;
}
