# RinOS Driver SDK

These headers are the public C interface for NDRV v3 modules. They use fixed-width records, opaque handles, and image-relative callback addresses so a Driver can build against the SDK without including OS-Core headers. Cross-architecture operation tables store callbacks in eight-byte slots; 32-bit consumers must keep each slot's high word zero.

## Core interfaces

| Header | Purpose |
| --- | --- |
| `image_abi_v3.h` | Image ABI values, result codes, opaque handles, Driver context, resource descriptors, and PCI match records. |
| `resource_contract_v3.h` | Fixed-width resource and operation records for BAR, DMA, IRQ, timers, registration, block, network, input, and logging. |
| `module_api_v3.h` | The NDRV v3 callback table and core Block, Network, Input, Display, Audio, and USB Controller contracts. |
| `services_v3.h` | Checked capability brokers for port I/O, IRQ events, authenticated firmware, child devices, and typed device properties. |
| `resource_runtime_v3.h` | Header-only checked wrappers for resource callbacks and generation-bound leases. |
| `validation_v3.h` | Inline validation for API tables, DMA/BAR/IRQ leases, timers, Block requests/completions, USB transfers/topology, and safe utility functions. |
| `serial.h` | Serial/TTY line configuration, memory-lease transfers, completion, and modem-line contracts. |
| `bluetooth/hci.h` | Bluetooth HCI command/data packet, controller, event, and completion contracts. |
| `wifi.h` | Wi-Fi scanning, regulatory channels, secure association, events, and the shared Network data path. |

`module_api_v3.h` is the canonical OS-Core callback table. `resource_runtime_v3.h` supplies checked wrappers for the BAR, DMA, IRQ, timer, and registration records; call `rin_driver_runtime_init` before `rin_driver_runtime_start`, then release every acquired lease before stopping. Other callbacks, including DMA scatter/gather synchronization, PCI configuration, and extended service brokers, are provided by the canonical table and `services_v3.h`. The record layouts and numeric ABI values remain defined by their original types and macros.

## Shared components

| Header and source | Purpose |
| --- | --- |
| `virtio/pci_transport_v3.h` and `src/rin/driver/virtio/pci_transport_v3.c` | VirtIO PCI capability discovery, transport access, and queue setup shared by device Drivers. |
| `virtio/virtqueue_v3.h` and `src/rin/driver/virtio/virtqueue_v3.c` | Virtqueue descriptor management and completion processing. |
| `display/edid.h` and `src/rin/driver/display/edid.c` | EDID validation and display-mode parsing. |

The shared VirtIO and EDID sources are executable SDK implementations intended for direct use by external Driver packages. Header-only broker and device-class APIs validate requests, results, and operation tables, then call the granted OS-Core operation table. A capability is usable only when OS-Core grants it with a complete provider. OS-Core policy, kernel object layouts, and NDRV v1/v2 compatibility interfaces remain outside this public include tree.
