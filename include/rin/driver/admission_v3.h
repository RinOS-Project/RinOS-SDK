/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_DRIVER_ADMISSION_V3_H
#define RIN_DRIVER_ADMISSION_V3_H

#include <stdint.h>

#include <rin/formats_v3.h>

/* This contract describes device identity and evaluates signed-image match
 * records against facts supplied by the OS. It does not authorize a driver or
 * grant hardware resources; those decisions and grants remain OS-Core-owned. */
#define RIN_DRIVER_ADMISSION_V3_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_ADMISSION_V3_MAX_MATCHES UINT32_C(64)

typedef enum RinDriverAdmissionV3Result {
    RIN_DRIVER_ADMISSION_V3_OK = 0,
    RIN_DRIVER_ADMISSION_V3_INVALID_ARGUMENT = -1,
    RIN_DRIVER_ADMISSION_V3_ARCHITECTURE_MISMATCH = -2,
    RIN_DRIVER_ADMISSION_V3_EXPERIMENTAL_DENIED = -3,
    RIN_DRIVER_ADMISSION_V3_REQUIREMENTS_UNMET = -4,
    RIN_DRIVER_ADMISSION_V3_NO_MATCH = -5
} RinDriverAdmissionV3Result;

typedef struct RinDriverDeviceIdentityV3 {
    uint32_t struct_size;
    uint32_t version;
    /* Opaque live-owner cookie. PCI cookies may encode a device-incarnation
     * counter independently of generation, which is the catalog snapshot
     * generation. Consumers must compare the complete cookie for ownership. */
    uint64_t device_cookie;
    uint16_t bus_type;
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_device_id;
    uint16_t reserved0;
    uint32_t class_code;
    uint32_t revision;
    uint32_t reserved1;
    uint64_t available_resources;
    /* Snapshot/catalog generation; it may differ from a PCI cookie's
     * upper-word endpoint-incarnation generation. */
    uint64_t generation;
    uint64_t reserved;
} RinDriverDeviceIdentityV3;

typedef struct RinDriverAdmissionPolicyV3 {
    uint32_t struct_size;
    uint32_t version;
    uint16_t architecture;
    uint16_t allow_experimental;
    uint32_t reserved0;
    uint64_t available_cpu_features;
    uint64_t reserved[2];
} RinDriverAdmissionPolicyV3;

RinDriverAdmissionV3Result rin_driver_admission_v3_match_image(
    const RinDriverHeaderV3* image_header,
    const RinDriverMatchV3* image_matches,
    const RinDriverDeviceIdentityV3* device,
    const RinDriverAdmissionPolicyV3* policy,
    uint32_t* matched_index);

#if defined(__cplusplus)
static_assert(sizeof(RinDriverDeviceIdentityV3) == 64u,
              "RinDriverDeviceIdentityV3 ABI drift");
static_assert(sizeof(RinDriverAdmissionPolicyV3) == 40u,
              "RinDriverAdmissionPolicyV3 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinDriverDeviceIdentityV3) == 64u,
               "RinDriverDeviceIdentityV3 ABI drift");
_Static_assert(sizeof(RinDriverAdmissionPolicyV3) == 40u,
               "RinDriverAdmissionPolicyV3 ABI drift");
#endif

#endif /* RIN_DRIVER_ADMISSION_V3_H */
