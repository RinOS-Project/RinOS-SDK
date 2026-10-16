/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_DRIVER_VIRTIO_PCI_TRANSPORT_V3_H
#define RIN_DRIVER_VIRTIO_PCI_TRANSPORT_V3_H

#include <rin/driver/module_api_v3.h>

#define RIN_VIRTIO_PCI_VENDOR_ID UINT16_C(0x1af4)
#define RIN_VIRTIO_PCI_CAP_COMMON_CFG UINT8_C(1)
#define RIN_VIRTIO_PCI_CAP_NOTIFY_CFG UINT8_C(2)
#define RIN_VIRTIO_PCI_CAP_ISR_CFG UINT8_C(3)
#define RIN_VIRTIO_PCI_CAP_DEVICE_CFG UINT8_C(4)

typedef struct RinVirtioPciCapabilityV3 {
    uint32_t bar_index;
    uint32_t offset;
    uint32_t length;
    uint32_t notify_multiplier;
} RinVirtioPciCapabilityV3;

typedef struct RinVirtioPciLayoutV3 {
    uint16_t vendor_id;
    uint16_t device_id;
    RinVirtioPciCapabilityV3 common;
    RinVirtioPciCapabilityV3 notify;
    RinVirtioPciCapabilityV3 isr;
    RinVirtioPciCapabilityV3 device;
} RinVirtioPciLayoutV3;

RinDriverV3Result rin_virtio_pci_discover_v3(
    const RinDriverApiV3* api, const RinDriverContextV3* context,
    RinVirtioPciLayoutV3* output);

#endif /* RIN_DRIVER_VIRTIO_PCI_TRANSPORT_V3_H */
