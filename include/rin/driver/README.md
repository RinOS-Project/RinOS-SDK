# RinOS Driver SDK

These headers are the public C interface for NDRV v3 modules. They use fixed-width records, opaque handles, and image-relative callback addresses so a Driver can build against the SDK without including OS-Core headers. Cross-architecture operation tables store callbacks in eight-byte slots; 32-bit consumers must keep each slot's high word zero.

## Core interfaces

| Header | Purpose |
| --- | --- |
| `framework_v3.h` | Convenience include for the public driver, device, resource, PCI, Port I/O, MMIO, IRQ, DMA, Block, Serial, and lifecycle contracts. |
| `driver.h`, `device.h` | Focused entrypoints for NDRV v3 callbacks, device identity/admission, and capability-gated services. |
| `resource.h`, `pci.h` | Checked generation-bound leases and the public PCI catalog/configuration contract. |
| `portio.h`, `mmio.h`, `irq.h`, `dma.h`, `iommu.h` | Focused resource APIs. Drivers request bounded leases; physical addresses, interrupt routing, DMA domains, and IOMMU page tables remain OS-Core owned. |
| `block.h`, `lifecycle.h`, `power.h` | Block operations and the versioned image lifecycle callback contracts. Kernel registration and suspend coordination remain OS-Core services. |
| `gpu/device.h`, `gpu/memory.h` | Public display/device resources and DMA-based memory operations for GPU drivers. GPU process mappings and hardware queue scheduling remain OS-Core owned. |
| `image_abi_v3.h` | Image ABI values, result codes, opaque handles, Driver context, resource descriptors, and PCI match records. |
| `admission_v3.h` and `src/rin/driver/admission_v3.c` | Public device identity and portable image-to-device matching. OS-Core retains admission policy and resource authorization. |
| `bus/pci/catalog_v3.h` and `src/rin/driver/bus/pci/catalog_v3.c` | Callback-driven PCI config-space cataloging and signed-image matching, with OS-authorized config reads supplied by the caller. |
| `resource_contract_v3.h` | Fixed-width resource and operation records for BAR, DMA, IRQ, timers, registration, block, network, input, and logging. |
| `module_api_v3.h` | The NDRV v3 callback table and core Block, Network, Input, Display, Audio, and USB Controller contracts. |
| `services_v3.h` | Checked capability brokers for port I/O, IRQ events, authenticated firmware, child devices, and typed device properties. |
| `resource_runtime_v3.h` | Header-only checked wrappers for resource callbacks and generation-bound leases. |
| `cpu_relax.h` | Inline architecture-aware processor wait hints for bounded driver polling loops. |
| `ps2/set1_decoder.h` and `src/rin/driver/ps2/set1_decoder.c` | Portable PS/2 Set 1 scancode decoding, key-repeat eligibility, and US ASCII translation. |
| `validation_v3.h` | Inline validation for API tables, DMA/BAR/IRQ leases, timers, Block requests/completions, USB transfers/topology, and safe utility functions. |
| `serial.h` | Serial/TTY line configuration, memory-lease transfers, completion, and modem-line contracts. |
| `capture/transport_service_v1.h` | Registers a permanent capture transport while OS-Core retains capability endpoint and session state. |
| `bluetooth/hci.h` | Bluetooth HCI command/data packet, controller, event, and completion contracts. |
| `bluetooth/hci_runtime_v1.h` and `src/rin/driver/bluetooth/hci_runtime_v1.c` | Portable HCI controller state machine and event processing over a caller-owned transport. |
| `bluetooth/hci_transport_v1.h` | Callback contract for HCI command transport and monotonic time. |
| `atomic_u64.h` | Aligned 64-bit storage type for shared driver records on 32-bit targets. |
| `gpu/pci_resource_v1.h` | OS-Core-granted BAR mapping and PCI configuration callbacks shared by GPU platform and driver owners. |
| `wifi.h` | Wi-Fi scanning, regulatory channels, secure association, events, and the shared Network data path. |
| `wifi/*.h` and `src/rin/driver/wifi/*.c` | Portable 802.11 discovery, association sequencing, WPA2/CCMP, and supplicant state machines. WPA2 crypto sources use RinTLS public crypto headers; OS-Core still owns regulatory authorization and key policy. |
| `usb/audio/*.h` | USB Audio Class 1/2 descriptors, runtime stream negotiation, isochronous packet planning, and transport contracts. |
| `usb/hid/*.h` | HID boot reports, pen and gamepad parsing, mapping/calibration support, and force-feedback records. |
| `usb/fido/ctaphid.h` | CTAP-HID packet framing and transaction state for FIDO authenticators. |
| `usb/fido/broker_v1.h` and `src/rin/driver/usb/fido/broker_v1.c` | Portable bounded CTAP-HID device broker and transfer lifecycle for USB FIDO drivers. |

`module_api_v3.h` is the canonical OS-Core callback table. `resource_runtime_v3.h` supplies checked wrappers for the BAR, DMA, IRQ, timer, and registration records; call `rin_driver_runtime_init` before `rin_driver_runtime_start`, then release every acquired lease before stopping. Other callbacks, including DMA scatter/gather synchronization, PCI configuration, and extended service brokers, are provided by the canonical table and `services_v3.h`. The record layouts and numeric ABI values remain defined by their original types and macros.

The focused headers above are stable SDK include roots over those canonical
records; they do not expose OS-Core object layouts or grant a capability by
being included. A Driver gets a resource only when the signed image is admitted
and the OS supplies the corresponding callback and a live, generation-bound
lease. `portio.h` uses a measured I/O BAR lease, `mmio.h` uses an OS-mapped BAR
lease, and `dma.h` exposes IOVA segments without exposing physical pages. The
`iommu.h` contract intentionally presents DMA operations only: domain creation,
page-table writes, invalidation, and device isolation remain private to
OS-Core. PCI enumeration and resource proof are OS-Core operations; the public
PCI catalog is for consuming OS-authorized config reads and device identities.

## Shared components

| Header and source | Purpose |
| --- | --- |
| `virtio/pci_transport_v3.h` and `src/rin/driver/virtio/pci_transport_v3.c` | VirtIO PCI capability discovery, transport access, and queue setup shared by device Drivers. |
| `virtio/virtqueue_v3.h` and `src/rin/driver/virtio/virtqueue_v3.c` | Virtqueue descriptor management and completion processing. |
| `display/edid.h` and `src/rin/driver/display/edid.c` | EDID validation and display-mode parsing. |
| `ps2/set1_decoder.h` and `src/rin/driver/ps2/set1_decoder.c` | Stateful Set 1 scan-code parsing shared by kernel input and PS/2 drivers. |

The shared VirtIO and EDID sources are executable SDK implementations intended for direct use by external Driver packages. Header-only broker and device-class APIs validate requests, results, and operation tables, then call the granted OS-Core operation table. A capability is usable only when OS-Core grants it with a complete provider. OS-Core policy, kernel object layouts, and NDRV v1/v2 compatibility interfaces remain outside this public include tree.

The USB Audio, HID, and CTAP-HID parsers and packet helpers are also executable SDK sources under `src/rin/driver/usb/`; build them directly into the relevant Driver package or SDK library. USB descriptors, reports, and packet sizes are checked against fixed bounds before any data is exposed to a Driver.

Drivers that need a processor wait hint should include
`rin/driver/cpu_relax.h`. These inline helpers keep polling code independent
of OS-Core's private architecture headers and link-time symbols.

`services_v3.h` defines the public firmware and child-device records and checked calls. NDRV v3 packages may embed authenticated firmware with repeated `make_drv.py --firmware=NAME=PATH` options. The kernel verifies the signed resource catalog and payload digests, then returns bounded reads through a generation-bound lease. Child publication uses the kernel's bounded child catalog and returns a kernel-generated device identity. Neither capability is granted when its backing runtime service is unavailable.
