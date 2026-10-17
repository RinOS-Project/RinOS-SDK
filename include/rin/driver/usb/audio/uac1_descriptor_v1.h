/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_DRIVERS_USB_AUDIO_UAC1_V1_H
#define RIN_DRIVERS_USB_AUDIO_UAC1_V1_H

#include <stdint.h>

#include "uac1_catalog_v1.h"

typedef enum RinUsbAudioUac1Result {
    RIN_USB_AUDIO_UAC1_OK = 0,
    RIN_USB_AUDIO_UAC1_INVALID_ARGUMENT = -1,
    RIN_USB_AUDIO_UAC1_MALFORMED = -2,
    RIN_USB_AUDIO_UAC1_UNSUPPORTED = -3,
    RIN_USB_AUDIO_UAC1_LIMIT_EXCEEDED = -4
} RinUsbAudioUac1Result;

/*
 * Admit the UAC1 PCM AudioStreaming alternate settings from one immutable,
 * complete USB configuration descriptor. The output is zero on any
 * well-formed non-aliasing failure. UAC2/UAC3 and compressed formats are not
 * accepted by this version.
 */
RinUsbAudioUac1Result rin_usb_audio_uac1_admit_configuration(
    const uint8_t* descriptor, uint32_t descriptor_size,
    RinUsbAudioUac1CatalogV1* catalog);

RinUsbAudioUac1Result rin_usb_audio_uac1_validate_profile(
    const RinUsbAudioUac1ProfileV1* profile,
    uint32_t configuration_length);

#endif /* RIN_DRIVERS_USB_AUDIO_UAC1_V1_H */
