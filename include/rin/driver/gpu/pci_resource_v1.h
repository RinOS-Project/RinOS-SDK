/* SPDX-License-Identifier: MIT */
/* OS-Core-granted BAR and PCI configuration callbacks for GPU owners. */
#ifndef RIN_DRIVER_GPU_PCI_RESOURCE_V1_H
#define RIN_DRIVER_GPU_PCI_RESOURCE_V1_H

#include <stdint.h>

#define RIN_GPU_PCI_RESOURCE_OWNER_V1_VERSION UINT32_C(0x00010001)

/*
 * The caller initializes all outputs to zero. On every return, a nonzero
 * lease identifies an outstanding mapping that bar_unmap must be able to
 * release; this remains true when bar_map reports failure or cannot provide
 * valid address/length metadata. A zero lease guarantees that no mapping is
 * retained. On success, lease, address, and length must describe the complete
 * requested mapping.
 */
typedef int (*RinGpuPciResourceOwnerV1BarMapFn)(
    void* context, uint64_t device_cookie, uint8_t bar_index,
    uint64_t length, uint32_t flags, uint64_t* lease,
    uint64_t* driver_virtual_address, uint64_t* mapped_length);
/* On failure, the lease remains owned and a retry with the same lease is safe. */
typedef int (*RinGpuPciResourceOwnerV1BarUnmapFn)(
    void* context, uint64_t device_cookie, uint64_t lease);
typedef int (*RinGpuPciResourceOwnerV1PciReadFn)(
    void* context, uint64_t device_cookie, uint16_t offset,
    uint8_t width, uint32_t* value);
typedef int (*RinGpuPciResourceOwnerV1PciWriteFn)(
    void* context, uint64_t device_cookie, uint16_t offset,
    uint8_t width, uint32_t value);

typedef struct RinGpuPciResourceOwnerV1Ops {
    uint32_t struct_size;
    uint32_t version;
    RinGpuPciResourceOwnerV1BarMapFn bar_map;
    RinGpuPciResourceOwnerV1BarUnmapFn bar_unmap;
    RinGpuPciResourceOwnerV1PciReadFn pci_config_read;
    RinGpuPciResourceOwnerV1PciWriteFn pci_config_write;
    void* context;
    uint64_t reserved[2];
} RinGpuPciResourceOwnerV1Ops;

#endif /* RIN_DRIVER_GPU_PCI_RESOURCE_V1_H */
