/* SPDX-License-Identifier: MIT */
/* Minimal ATA IDENTIFY DEVICE media-class projection. */
#ifndef RIN_DRIVER_STORAGE_ATA_MEDIA_CLASS_V1_H
#define RIN_DRIVER_STORAGE_ATA_MEDIA_CLASS_V1_H

#include <stdint.h>

#define RIN_ATA_MEDIA_CLASS_API_VERSION UINT32_C(1)

/* ATA IDENTIFY DEVICE word 0: Removable Media Device. */
#define RIN_ATA_IDENTIFY_REMOVABLE_MEDIA UINT16_C(0x0080)

int rin_ata_identify_media_is_removable(uint16_t general_configuration);

#endif /* RIN_DRIVER_STORAGE_ATA_MEDIA_CLASS_V1_H */
