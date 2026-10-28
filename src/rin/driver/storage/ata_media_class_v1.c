/* SPDX-License-Identifier: MIT */

#include <rin/driver/storage/ata_media_class_v1.h>

int rin_ata_identify_media_is_removable(uint16_t general_configuration)
{
    return (general_configuration & RIN_ATA_IDENTIFY_REMOVABLE_MEDIA) != 0u;
}
