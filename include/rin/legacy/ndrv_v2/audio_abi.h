/* SPDX-License-Identifier: MIT */
#ifndef RIN_API_LEGACY_DRIVER_V2_AUDIO_ABI_H
#define RIN_API_LEGACY_DRIVER_V2_AUDIO_ABI_H

#include <stdint.h>

/* Canonical legacy SDK contract; retained only for the legacy namespace. */

/* The v2 audio device is intentionally a single-owner PCM registration. */
#define RIN_LEGACY_DRIVER_V2_AUDIO_MAGIC UINT32_C(0x32445541) /* AUD2 */
#define RIN_LEGACY_DRIVER_V2_AUDIO_VERSION UINT16_C(1)
#define RIN_LEGACY_DRIVER_V2_AUDIO_NAME_SIZE UINT32_C(32)
#define RIN_LEGACY_DRIVER_V2_AUDIO_MAX_ENTRIES UINT32_C(4)

#define RIN_AUDIO_V2_CONTROL_PLAY       UINT32_C(1)
#define RIN_AUDIO_V2_CONTROL_STOP       UINT32_C(2)
#define RIN_AUDIO_V2_CONTROL_FLUSH      UINT32_C(3)
#define RIN_AUDIO_V2_CONTROL_SET_VOLUME UINT32_C(4)
#define RIN_AUDIO_V2_CONTROL_SET_MUTE   UINT32_C(5)
#define RIN_AUDIO_V2_CONTROL_SET_RATE   UINT32_C(6)

typedef struct RinDriverDeviceV2 {
    uint32_t id;
    uint32_t type;
    uint32_t bus;
    uint32_t device;
    uint32_t function;
    uint32_t class_code;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t irq;
    uint8_t reserved8[7];
    uint64_t mmio_phys;
    uint64_t mmio_size;
    uint32_t io_base;
    uint32_t io_size;
    void* private_data;
} RinDriverDeviceV2;

typedef struct RinAudioOpsV2 {
    int (*open)(RinDriverDeviceV2* dev, uint32_t sample_rate,
                uint16_t channels, uint16_t bits_per_sample);
    int (*close)(RinDriverDeviceV2* dev);
    int (*write)(RinDriverDeviceV2* dev, const void* data, uint32_t len);
    int (*control)(RinDriverDeviceV2* dev, uint32_t cmd, uint64_t arg);
} RinAudioOpsV2;

typedef struct __attribute__((packed)) {
    char name[RIN_LEGACY_DRIVER_V2_AUDIO_NAME_SIZE];
    uint64_t device;
    uint64_t ops;
    uint8_t valid;
    uint8_t reserved[7];
} RinLegacyDriverV2AudioPendingEntry;

#define RIN_LEGACY_DRIVER_V2_AUDIO_PENDING_ENTRY_SIZE UINT32_C(56)

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinDriverDeviceV2) == 72u,
               "NDRV v2 device ABI drift");
_Static_assert(sizeof(RinAudioOpsV2) == 32u,
               "NDRV v2 audio ops ABI drift");
_Static_assert(sizeof(RinLegacyDriverV2AudioPendingEntry) ==
                   RIN_LEGACY_DRIVER_V2_AUDIO_PENDING_ENTRY_SIZE,
               "NDRV v2 audio pending ABI drift");
#endif

#endif /* RIN_API_LEGACY_DRIVER_V2_AUDIO_ABI_H */
