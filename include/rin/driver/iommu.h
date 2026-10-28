/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_IOMMU_H
#define RIN_DRIVER_IOMMU_H

/*
 * General Drivers use the DMA lease API. IOMMU domains, page tables,
 * invalidation, and physical-page pinning are OS-Core owned and deliberately
 * have no general Driver callback in this ABI.
 */
#include <rin/driver/dma.h>

#endif /* RIN_DRIVER_IOMMU_H */
