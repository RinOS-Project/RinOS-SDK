# Storage Driver SDK helpers

`ata_media_class_v1.h` and `src/rin/driver/storage/ata_media_class_v1.c`
provide a versioned, portable projection of ATA IDENTIFY word 0. The helper
only classifies metadata already read by a caller. It performs no PCI
enumeration, port I/O, MMIO mapping, DMA, interrupt routing, or allocation.

Hardware access belongs behind the bounded leases and services in the RinOS
Driver SDK. Legacy ATA/ATAPI transport and controller discovery helpers remain
Kernel-owned until they can consume those resource handles directly.
