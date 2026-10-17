/* SPDX-License-Identifier: MIT */
/* Pointer-free USB Audio UAC1 catalog ABI. */
#ifndef RINOS_PUBLIC_BASE_RINOS_SDK_INCLUDE_RIN_DRIVER_USB_AUDIO_UAC1_CATALOG_V1_H
#define RINOS_PUBLIC_BASE_RINOS_SDK_INCLUDE_RIN_DRIVER_USB_AUDIO_UAC1_CATALOG_V1_H

#include <stdint.h>

#define RIN_USB_AUDIO_UAC1_ABI_VERSION UINT32_C(0x00010000)
#define RIN_USB_AUDIO_UAC1_MAX_CONFIGURATION_BYTES 4096u
#define RIN_USB_AUDIO_UAC1_MAX_INTERFACE_DESCRIPTORS 64u
#define RIN_USB_AUDIO_UAC1_MAX_PROFILES 8u
#define RIN_USB_AUDIO_UAC1_MAX_SAMPLE_RATES 8u

#define RIN_USB_AUDIO_UAC1_PROFILE_PLAYBACK UINT32_C(0x00000001)
#define RIN_USB_AUDIO_UAC1_PROFILE_CAPTURE UINT32_C(0x00000002)
#define RIN_USB_AUDIO_UAC1_PROFILE_CONTINUOUS_RATE UINT32_C(0x00000004)
#define RIN_USB_AUDIO_UAC1_PROFILE_EXPLICIT_FEEDBACK UINT32_C(0x00000008)
#define RIN_USB_AUDIO_UAC1_PROFILE_FREQUENCY_CONTROL UINT32_C(0x00000010)
#define RIN_USB_AUDIO_UAC1_PROFILE_PITCH_CONTROL UINT32_C(0x00000020)
#define RIN_USB_AUDIO_UAC1_PROFILE_HIGH_BANDWIDTH UINT32_C(0x00000040)
#define RIN_USB_AUDIO_UAC1_PROFILE_FLAGS                                      \
    (RIN_USB_AUDIO_UAC1_PROFILE_PLAYBACK |                                   \
     RIN_USB_AUDIO_UAC1_PROFILE_CAPTURE |                                    \
     RIN_USB_AUDIO_UAC1_PROFILE_CONTINUOUS_RATE |                            \
     RIN_USB_AUDIO_UAC1_PROFILE_EXPLICIT_FEEDBACK |                          \
     RIN_USB_AUDIO_UAC1_PROFILE_FREQUENCY_CONTROL |                          \
     RIN_USB_AUDIO_UAC1_PROFILE_PITCH_CONTROL |                              \
     RIN_USB_AUDIO_UAC1_PROFILE_HIGH_BANDWIDTH)

#define RIN_USB_AUDIO_UAC1_CATALOG_PLAYBACK UINT32_C(0x00000001)
#define RIN_USB_AUDIO_UAC1_CATALOG_CAPTURE UINT32_C(0x00000002)
#define RIN_USB_AUDIO_UAC1_CATALOG_FLAGS                                     \
    (RIN_USB_AUDIO_UAC1_CATALOG_PLAYBACK |                                  \
     RIN_USB_AUDIO_UAC1_CATALOG_CAPTURE)

typedef struct RinUsbAudioUac1ProfileV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t descriptor_offset;
    uint32_t descriptor_length;
    uint32_t flags;
    uint32_t sample_rate_minimum;
    uint32_t sample_rate_maximum;
    uint32_t reserved0;
    uint32_t sample_rates[RIN_USB_AUDIO_UAC1_MAX_SAMPLE_RATES];
    uint8_t interface_number;
    uint8_t alternate_setting;
    uint8_t terminal_link;
    uint8_t endpoint_address;
    uint8_t feedback_endpoint_address;
    uint8_t endpoint_attributes;
    uint8_t endpoint_interval;
    uint8_t endpoint_refresh;
    uint8_t endpoint_sync_address;
    uint8_t channels;
    uint8_t subframe_size;
    uint8_t bit_resolution;
    uint8_t sample_rate_count;
    uint8_t lock_delay_units;
    uint8_t class_endpoint_attributes;
    uint8_t reserved1;
    uint16_t maximum_packet_size;
    uint16_t maximum_payload_per_interval;
    uint16_t format_tag;
    uint16_t lock_delay;
    uint32_t reserved[2];
} RinUsbAudioUac1ProfileV1;

typedef struct RinUsbAudioUac1CatalogV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t configuration_length;
    uint32_t profile_count;
    uint32_t flags;
    uint32_t reserved[3];
    RinUsbAudioUac1ProfileV1 profiles[RIN_USB_AUDIO_UAC1_MAX_PROFILES];
} RinUsbAudioUac1CatalogV1;

#if defined(__cplusplus)
static_assert(sizeof(RinUsbAudioUac1ProfileV1) == 96u,
              "USB Audio UAC1 profile ABI drift");
static_assert(sizeof(RinUsbAudioUac1CatalogV1) == 800u,
              "USB Audio UAC1 catalog ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinUsbAudioUac1ProfileV1) == 96u,
               "USB Audio UAC1 profile ABI drift");
_Static_assert(sizeof(RinUsbAudioUac1CatalogV1) == 800u,
               "USB Audio UAC1 catalog ABI drift");
#endif

#endif /* RINOS_PUBLIC_BASE_RINOS_SDK_INCLUDE_RIN_DRIVER_USB_AUDIO_UAC1_CATALOG_V1_H */
