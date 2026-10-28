/* SPDX-License-Identifier: MIT */
/* Pointer-free Wi-Fi firmware provenance; presence does not grant authority. */

#ifndef RIN_DRIVER_WIFI_FIRMWARE_IDENTITY_V1_H
#define RIN_DRIVER_WIFI_FIRMWARE_IDENTITY_V1_H

#include <stdint.h>

#define RIN_WIFI_FIRMWARE_ADMISSION_V1_MAX_IMAGE_BYTES UINT32_C(4194304)
#define RIN_WIFI_VENDOR_PRODUCT_FIRMWARE_IDENTITY_V1_VERSION \
    UINT32_C(0x00010000)
#define RIN_WIFI_VENDOR_PRODUCT_FIRMWARE_IDENTITY_FLAG_VERSION_VALID \
    UINT32_C(1)
#define RIN_WIFI_VENDOR_PRODUCT_FIRMWARE_IDENTITY_FLAG_DIGEST_VALID \
    UINT32_C(2)
#define RIN_WIFI_VENDOR_PRODUCT_FIRMWARE_IDENTITY_FLAG_AUTHENTICATED \
    UINT32_C(4)
#define RIN_WIFI_VENDOR_PRODUCT_FIRMWARE_IDENTITY_FLAG_REQUIRED \
    (RIN_WIFI_VENDOR_PRODUCT_FIRMWARE_IDENTITY_FLAG_VERSION_VALID | \
     RIN_WIFI_VENDOR_PRODUCT_FIRMWARE_IDENTITY_FLAG_DIGEST_VALID | \
     RIN_WIFI_VENDOR_PRODUCT_FIRMWARE_IDENTITY_FLAG_AUTHENTICATED)

/* Pointer-free provenance retained only after signed firmware admission. */
typedef struct RinWifiVendorProductFirmwareIdentityV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    uint32_t family;
    uint64_t device_cookie;
    uint64_t device_generation;
    uint64_t firmware_version;
    uint64_t firmware_generation;
    uint32_t image_size;
    uint32_t reserved0;
    uint8_t content_hash[32];
    uint8_t trusted_key_id[32];
    uint64_t reserved[2];
} RinWifiVendorProductFirmwareIdentityV1;

#if defined(__cplusplus)
static_assert(sizeof(RinWifiVendorProductFirmwareIdentityV1) == 136u,
              "Wi-Fi product firmware identity ABI drift");
#else
_Static_assert(sizeof(RinWifiVendorProductFirmwareIdentityV1) == 136u,
               "Wi-Fi product firmware identity ABI drift");
#endif

#endif /* RIN_DRIVER_WIFI_FIRMWARE_IDENTITY_V1_H */
