/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_DRIVER_BUS_PCI_CATALOG_V3_H
#define RIN_DRIVER_BUS_PCI_CATALOG_V3_H

#include <stddef.h>
#include <stdint.h>

#include <rin/driver/admission_v3.h>

#define RIN_DRIVER_PCI_CATALOG_V3_VERSION UINT32_C(0x00010000)

#define RIN_DRIVER_PCI_V3_CAP_MSI  UINT32_C(0x00000001)
#define RIN_DRIVER_PCI_V3_CAP_MSIX UINT32_C(0x00000002)

#define RIN_DRIVER_PCI_V3_FLAG_CAPABILITIES_MALFORMED UINT32_C(0x00000001)
#define RIN_DRIVER_PCI_V3_FLAG_CONFIG_MALFORMED       UINT32_C(0x00000002)

#define RIN_DRIVER_PCI_V3_PROVABLE_RESOURCES                         \
    (RIN_DRIVER_RESOURCE_MMIO | RIN_DRIVER_RESOURCE_IOPORT |        \
     RIN_DRIVER_RESOURCE_DMA | RIN_DRIVER_RESOURCE_IRQ)

typedef enum RinDriverPciCatalogV3Result {
    RIN_DRIVER_PCI_CATALOG_V3_OK = 0,
    RIN_DRIVER_PCI_CATALOG_V3_INVALID = -1,
    RIN_DRIVER_PCI_CATALOG_V3_IO = -2,
    RIN_DRIVER_PCI_CATALOG_V3_LIMIT = -3
} RinDriverPciCatalogV3Result;

typedef enum RinDriverPciMatchV3Result {
    RIN_DRIVER_PCI_MATCH_V3_OK = 0,
    RIN_DRIVER_PCI_MATCH_V3_NO_DEVICE = 1,
    RIN_DRIVER_PCI_MATCH_V3_REQUIREMENTS_UNMET = 2,
    RIN_DRIVER_PCI_MATCH_V3_LIMIT = 3,
    RIN_DRIVER_PCI_MATCH_V3_INVALID = -1
} RinDriverPciMatchV3Result;

typedef struct RinDriverPciAddressV3 {
    uint16_t segment;
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint8_t reserved[3];
} RinDriverPciAddressV3;

typedef int (*RinDriverPciRead32V3)(
    void* context, uint16_t segment, uint8_t bus, uint8_t device,
    uint8_t function, uint16_t offset, uint32_t* value);

typedef struct RinDriverPciCatalogConfigV3 {
    uint32_t struct_size;
    uint32_t version;
    uint16_t segment;
    uint8_t first_bus;
    uint8_t last_bus;
    uint32_t generation;
    /* Only resources implemented by the platform backend may be supplied.
     * Hardware discovery records BAR/IRQ presence separately and never makes
     * those resources admissible on its own. DMA additionally requires an
     * operational isolated domain for every catalogued device. */
    uint64_t proven_resources;
    RinDriverPciRead32V3 read32;
    void* read_context;
    uint64_t reserved[2];
} RinDriverPciCatalogConfigV3;

typedef struct RinDriverPciDeviceV3 {
    RinDriverDeviceIdentityV3 identity;
    RinDriverPciAddressV3 address;
    uint32_t capabilities;
    uint32_t flags;
    uint8_t memory_bar_mask;
    uint8_t io_bar_mask;
    uint8_t interrupt_pin;
    uint8_t header_type;
    uint32_t reserved;
} RinDriverPciDeviceV3;

typedef struct RinDriverPciCatalogV3 {
    uint32_t struct_size;
    uint32_t version;
    RinDriverPciDeviceV3* devices;
    uint32_t capacity;
    uint32_t count;
    uint32_t generation;
    uint32_t complete;
} RinDriverPciCatalogV3;

RinDriverPciCatalogV3Result rin_driver_pci_catalog_v3_scan(
    RinDriverPciCatalogV3* catalog,
    const RinDriverPciCatalogConfigV3* config);

const RinDriverPciDeviceV3* rin_driver_pci_catalog_v3_find(
    const RinDriverPciCatalogV3* catalog, uint64_t device_cookie,
    uint64_t generation);

RinDriverPciMatchV3Result rin_driver_pci_catalog_v3_match(
    const RinDriverPciCatalogV3* catalog,
    const RinDriverHeaderV3* image_header,
    const RinDriverMatchV3* image_matches,
    const RinDriverAdmissionPolicyV3* policy,
    const RinDriverPciDeviceV3** matched_device, uint32_t* matched_index);

/* Enumerate every admissible device for one signed image.  The output arrays
 * are caller-owned and are never retained.  A REQUIREMENTS_UNMET result is
 * returned only when no device is admissible but at least one exact device
 * identity was found with unavailable resources. */
RinDriverPciMatchV3Result rin_driver_pci_catalog_v3_match_all(
    const RinDriverPciCatalogV3* catalog,
    const RinDriverHeaderV3* image_header,
    const RinDriverMatchV3* image_matches,
    const RinDriverAdmissionPolicyV3* policy,
    const RinDriverPciDeviceV3** matched_devices,
    uint32_t* matched_indices, uint32_t capacity, uint32_t* matched_count);

#endif /* RIN_DRIVER_BUS_PCI_CATALOG_V3_H */
