/* SPDX-License-Identifier: MIT */
/* Pointer-free USB Audio UAC2 catalog ABI. */
#ifndef RINOS_PUBLIC_BASE_RINOS_SDK_INCLUDE_RIN_DRIVER_USB_AUDIO_UAC2_CATALOG_V2_H
#define RINOS_PUBLIC_BASE_RINOS_SDK_INCLUDE_RIN_DRIVER_USB_AUDIO_UAC2_CATALOG_V2_H

#include <stdint.h>

#define RIN_USB_AUDIO_UAC2_ABI_VERSION UINT32_C(0x00010000)
#define RIN_USB_AUDIO_UAC2_MAX_CONFIGURATION_BYTES 4096u
#define RIN_USB_AUDIO_UAC2_MAX_INTERFACE_DESCRIPTORS 64u
#define RIN_USB_AUDIO_UAC2_MAX_PROFILES 8u
#define RIN_USB_AUDIO_UAC2_MAX_SAMPLE_RATES 8u

#define RIN_USB_AUDIO_UAC2_PROFILE_PLAYBACK UINT32_C(0x00000001)
#define RIN_USB_AUDIO_UAC2_PROFILE_CAPTURE UINT32_C(0x00000002)
#define RIN_USB_AUDIO_UAC2_PROFILE_CONTINUOUS_RATE UINT32_C(0x00000004)
#define RIN_USB_AUDIO_UAC2_PROFILE_EXPLICIT_FEEDBACK UINT32_C(0x00000008)
#define RIN_USB_AUDIO_UAC2_PROFILE_HIGH_BANDWIDTH UINT32_C(0x00000010)
#define RIN_USB_AUDIO_UAC2_PROFILE_FREQUENCY_CONTROL UINT32_C(0x00000020)
#define RIN_USB_AUDIO_UAC2_PROFILE_FLAGS                                      \
    (RIN_USB_AUDIO_UAC2_PROFILE_PLAYBACK |                                   \
     RIN_USB_AUDIO_UAC2_PROFILE_CAPTURE |                                    \
     RIN_USB_AUDIO_UAC2_PROFILE_CONTINUOUS_RATE |                             \
     RIN_USB_AUDIO_UAC2_PROFILE_EXPLICIT_FEEDBACK |                           \
     RIN_USB_AUDIO_UAC2_PROFILE_HIGH_BANDWIDTH |                              \
     RIN_USB_AUDIO_UAC2_PROFILE_FREQUENCY_CONTROL)

#define RIN_USB_AUDIO_UAC2_CATALOG_PLAYBACK UINT32_C(0x00000001)
#define RIN_USB_AUDIO_UAC2_CATALOG_CAPTURE UINT32_C(0x00000002)
#define RIN_USB_AUDIO_UAC2_CATALOG_FLAGS                                      \
    (RIN_USB_AUDIO_UAC2_CATALOG_PLAYBACK |                                   \
     RIN_USB_AUDIO_UAC2_CATALOG_CAPTURE)

typedef struct RinUsbAudioUac2ProfileV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t descriptor_offset;
    uint32_t descriptor_length;
    uint32_t flags;
    uint8_t clock_source_id;
    uint8_t clock_attributes;
    uint8_t clock_controls;
    uint8_t terminal_link;
    uint32_t sample_rate_minimum;
    uint32_t sample_rate_maximum;
    uint32_t sample_rates[RIN_USB_AUDIO_UAC2_MAX_SAMPLE_RATES];
    uint8_t interface_number;
    uint8_t alternate_setting;
    uint8_t endpoint_address;
    uint8_t feedback_endpoint_address;
    uint8_t endpoint_attributes;
    uint8_t endpoint_interval;
    uint8_t channels;
    uint8_t subslot_size;
    uint8_t bit_resolution;
    uint8_t sample_rate_count;
    uint16_t maximum_packet_size;
    uint16_t maximum_payload_per_interval;
    uint16_t format_type;
    uint16_t control_interface_number;
    uint32_t reserved[4];
} RinUsbAudioUac2ProfileV1;

typedef struct RinUsbAudioUac2CatalogV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t configuration_length;
    uint32_t profile_count;
    uint32_t flags;
    uint32_t reserved[3];
    RinUsbAudioUac2ProfileV1 profiles[RIN_USB_AUDIO_UAC2_MAX_PROFILES];
} RinUsbAudioUac2CatalogV1;

#if defined(__cplusplus)
static_assert(sizeof(RinUsbAudioUac2ProfileV1) == 100u,
              "USB Audio UAC2 profile ABI drift");
static_assert(sizeof(RinUsbAudioUac2CatalogV1) == 832u,
              "USB Audio UAC2 catalog ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinUsbAudioUac2ProfileV1) == 100u,
               "USB Audio UAC2 profile ABI drift");
_Static_assert(sizeof(RinUsbAudioUac2CatalogV1) == 832u,
               "USB Audio UAC2 catalog ABI drift");
#endif

#endif /* RINOS_PUBLIC_BASE_RINOS_SDK_INCLUDE_RIN_DRIVER_USB_AUDIO_UAC2_CATALOG_V2_H */
