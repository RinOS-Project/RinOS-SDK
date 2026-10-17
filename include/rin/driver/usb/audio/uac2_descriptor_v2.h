/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_DRIVERS_USB_AUDIO_UAC2_V1_H
#define RIN_DRIVERS_USB_AUDIO_UAC2_V1_H

#include <stdint.h>

#include "uac2_catalog_v2.h"

typedef enum RinUsbAudioUac2Result {
    RIN_USB_AUDIO_UAC2_OK = 0,
    RIN_USB_AUDIO_UAC2_INVALID_ARGUMENT = -1,
    RIN_USB_AUDIO_UAC2_MALFORMED = -2,
    RIN_USB_AUDIO_UAC2_UNSUPPORTED = -3,
    RIN_USB_AUDIO_UAC2_LIMIT_EXCEEDED = -4
} RinUsbAudioUac2Result;

/*
 * Admit UAC2 PCM AudioStreaming alternate settings from one immutable,
 * complete USB configuration descriptor. The output is zero on any malformed
 * or unsupported graph; no partial profile is ever published. UAC3 and
 * non-PCM formats remain outside this ABI.
 */
RinUsbAudioUac2Result rin_usb_audio_uac2_admit_configuration(
    const uint8_t* descriptor, uint32_t descriptor_size,
    RinUsbAudioUac2CatalogV1* catalog);

RinUsbAudioUac2Result rin_usb_audio_uac2_validate_profile(
    const RinUsbAudioUac2ProfileV1* profile,
    uint32_t configuration_length);

#endif /* RIN_DRIVERS_USB_AUDIO_UAC2_V1_H */
